import edu.stanford.nlp.ling.*;
import edu.stanford.nlp.parser.common.ParserQuery;
import edu.stanford.nlp.parser.lexparser.LexicalizedParser;
import edu.stanford.nlp.trees.*;

import java.io.*;
import java.nio.channels.FileChannel;
import java.nio.channels.FileLock;
import java.nio.file.StandardOpenOption;
import java.util.Arrays;
import java.util.List;
import java.util.zip.GZIPInputStream;

public class ParserFailSafe {

    private static boolean pipe_message(FileChannel fc, PrintWriter pwPipe, String msg, PrintWriter pwErr) {
        FileLock lc = null;
        try {
            lc = fc.lock();
            pwPipe.println(msg);
            pwPipe.flush();
        } catch (IOException e) {
            pwErr.println("Unable to send message: " + msg);
            pwErr.flush();
            System.exit(0);
        }
        try {
            lc.release();
        } catch (IOException e) {
            pwErr.println("Cannot release lock.");
            pwErr.flush();
            return true;
        }
        return false;
    }

    public static void main(String[] args) {

        String myID = args[0];
        String pipe_path = args[1];
        String lock_file_path = args[2];
        String log_path_prefix = args[3];
        String parser_model_file_path = args[4];
        String[] extraArgs = Arrays.copyOfRange(args, 5, args.length);

        PrintWriter pwOut;
        PrintWriter pwErr;
        try {
            pwOut = new PrintWriter(new FileWriter(log_path_prefix + myID + ".out", true));
            pwErr = new PrintWriter(new FileWriter(log_path_prefix + myID + ".err", true));
        } catch (IOException e) {
            return;
        }

        ObjectInputStream ois;
        try {
            ois = new ObjectInputStream(System.in);
        } catch (IOException e) {
            e.printStackTrace(pwErr);
            pwErr.flush();
            pwOut.close();
            pwErr.close();
            return;
        }

        FileChannel fc = null;
        PrintWriter pwPipe = null;
        try {
            fc = FileChannel.open(new File(lock_file_path).toPath(), StandardOpenOption.WRITE);
            pwPipe = new PrintWriter(pipe_path);
        } catch (IOException e) {
            e.printStackTrace(pwErr);
            pwErr.flush();
        }

        LexicalizedParser lp = null;

        int parsed = 0;
        while (fc != null && pwPipe != null) {

            if (lp == null) {
                try {
                    ObjectInputStream in = new ObjectInputStream(new GZIPInputStream(ClassLoader.getSystemResourceAsStream(parser_model_file_path)));
                    //ObjectInputStream in = new ObjectInputStream(new GZIPInputStream(new FileInputStream(parser_model_file_path)));
                    lp = LexicalizedParser.loadModel(in);
                    in.close();
                    if (extraArgs.length > 0) {
                        lp.setOptionFlags(extraArgs);
                    }
                } catch (Throwable e) {
                    e.printStackTrace(pwErr);
                    pwErr.flush();
                    break;
                }
            }

            if (pipe_message(fc, pwPipe, myID, pwErr)) {
                try {
                    fc.close();
                    pwPipe.close();
                    fc = FileChannel.open(new File(lock_file_path).toPath(), StandardOpenOption.WRITE);
                    pwPipe = new PrintWriter(pipe_path);
                } catch (IOException e) {
                    pwErr.println("IOException during chanel and pipe closing and reopening.");
                    pwErr.flush();
                    fc = null;
                    pwPipe = null;
                }
            }

            String para_sent_id;
            List<CoreLabel> sentence;
            try {
                para_sent_id = (String) ois.readObject();
                sentence = (List<CoreLabel>) ois.readObject();
            } catch (EOFException e) {
                break;
            } catch (Throwable e) {
                e.printStackTrace(pwErr);
                pwErr.flush();
                break;
            }

            pwErr.println(Sentence.listToString(sentence, true));
            pwOut.println(para_sent_id);
            boolean result_output_done = false;
            try {
                ParserQuery pq = lp.parserQuery();
                if (pq.parse(sentence)) {
                    Tree res = pq.getBestParse();

                    WordStemmer stemmer = new WordStemmer();
                    stemmer.visitTree(res);

                    List<LabeledWord> labels = res.labeledYield();
                    TreebankLanguagePack tlp = lp.treebankLanguagePack();
                    GrammaticalStructureFactory gsf = tlp.grammaticalStructureFactory(tlp.punctuationTagRejectFilter(), tlp.typedDependencyHeadFinder());
                    GrammaticalStructure gs = gsf.newGrammaticalStructure(res);

                    int rootNodeIndex = 0;
                    int cnums[] = new int[labels.size() + 1];
                    boolean auxbe[] = new boolean[labels.size() + 1];
                    boolean mwe[] = new boolean[labels.size() + 1];
                    for (TypedDependency x : gs.typedDependencies()) {
                        int govidx = x.gov().index();
                        int depidx = x.dep().index();
                        //String govtag = x.gov().tag();
                        String deptag = x.dep().tag();
                        if (govidx == 0) { //ROOT
                            rootNodeIndex = depidx;
                        } else {
                            String rel = x.reln().getShortName();
                            if (!(rel.matches("de[tp]") || deptag.equals("DT")
                                    || (deptag.equals("PRP$") && govidx > depidx))) {
                                ++cnums[govidx];
                            }
                            if (rel.equals("cop") || (rel.equals("aux") && x.dep().value().toLowerCase().equals("be"))) {
                                auxbe[govidx] = true;
                            }
                            if (rel.equals("mwe")) {
                                mwe[govidx] = true;
                            }
                        }
                    }
                    boolean changed = false;
                    for (TypedDependency x : gs.typedDependencies()) {
                        int govidx = x.gov().index();
                        int depidx = x.dep().index();
                        String govtag = x.gov().tag();
                        String deptag = x.dep().tag();
                        if (govidx == 0) continue;
                        String rel = x.reln().getShortName();
                        if (govidx == rootNodeIndex) {
                            if (cnums[govidx] == 1 && govtag.matches("NN.*") && rel.equals("acl") && deptag.equals("VBN")) {
                                CoreLabel w = sentence.get(depidx - 1);
                                w.setTag("VBD");
                                pwErr.println("01: " + w + " VBN -> VBD");
                                changed = true;
                            }
                            if (cnums[govidx] == 0 && govtag.matches("VB.*") && rel.equals("dep")
                                    && deptag.matches("VB.*") && govidx < depidx
                                    && !x.gov().value().toLowerCase().equals("be")) {
                                CoreLabel w = sentence.get(govidx - 1);
                                String ntag = govtag.equals("VBZ") ? "NNS" : "NN";
                                w.setTag(ntag);
                                pwErr.println("02: " + w + " " + govtag + " -> " + ntag);
                                changed = true;
                            }
                        }
                        if (rel.equals("dep") && cnums[depidx] == 0 && deptag.matches("VB.*")) {
                            int tmp = depidx - 2;
                            if (tmp >= 0 && labels.get(tmp).tag().value().matches("NN.*|JJ.*")) {
                                CoreLabel w = sentence.get(depidx - 1);
                                String ntag = deptag.equals("VBZ") ? "NNS" : "NN";
                                w.setTag(ntag);
                                pwErr.println("03: " + w + " " + deptag + " -> " + ntag);
                                changed = true;
                            }
                        }
                        if (rel.equals("case") && deptag.matches("JJ.*") && !mwe[depidx]) {
                            int tmp = depidx - 2;
                            if (tmp >= 0 && depidx < labels.size()
                                    && labels.get(tmp).tag().value().matches("NN.*")
                                    && labels.get(depidx).tag().value().equals("IN")) {
                                CoreLabel w = sentence.get(depidx - 1);
                                w.setTag("NN");
                                pwErr.println("04: " + w + " " + deptag + " -> NN");
                                changed = true;
                            }
                        }
                        if (rel.equals("conj") && !auxbe[depidx]) {
                            if (govtag.matches("NN.*") && ((deptag.matches("JJ.*") && !auxbe[govidx])
                                    || (deptag.equals("VBG") && cnums[depidx] == 0))
                                    && !(depidx < labels.size() && labels.get(depidx).tag().value().matches("JJ.*|NN.*"))) {
                                CoreLabel w = sentence.get(depidx - 1);
                                w.setTag("NN");
                                pwErr.println("05: " + w + " " + deptag + " -> NN");
                                changed = true;
                            }
                            if (deptag.matches("NN.*") && govtag.matches("VBG|JJ.*")) {
                                CoreLabel w = sentence.get(govidx - 1);
                                w.setTag("NN");
                                pwErr.println("06: " + w + " " + govtag + " -> NN");
                                changed = true;
                            }
                        }
                        if ((deptag.equals("PRP$") && govidx < depidx)
                                || (deptag.equals("DT")
                                    && (govidx < depidx || rel.matches("nsubj(pass)?|dobj"))
                                    && x.dep().value().toLowerCase().matches("a|the"))) { //Weird reversed construct
                            if (depidx < labels.size()) {
                                String tag = labels.get(depidx).tag().value();
                                if (tag.matches("VB.*")) { //probably caused by miss-tagging NN as VB
                                    CoreLabel w = sentence.get(depidx);
                                    String ntag;
                                    if (tag.matches("VB[DGN]") && depidx + 1 < labels.size()
                                            && labels.get(depidx + 1).tag().value().matches("JJ.*|NN.*")) {
                                        ntag = "JJ";
                                    } else if (tag.equals("VBZ")) {
                                        ntag = "NNS";
                                    } else {
                                        ntag = "NN";
                                    }
                                    w.setTag(ntag);
                                    pwErr.println("07: " + w + " " + tag + " -> " + ntag);
                                    changed = true;
                                }
                            }
                        }
                        if (govtag.equals("DT") && govidx == depidx - 1) {
                            if (deptag.matches("VB.*")) {
                                if (x.gov().value().toLowerCase().matches("a|the")
                                        || !deptag.matches("VB[NG]")) {
                                    CoreLabel w = sentence.get(govidx);
                                    String ntag = deptag.equals("VBZ") ? "NNS" : "NN";
                                    w.setTag(ntag);
                                    pwErr.println("08: " + w + " " + deptag + " -> " + ntag);
                                    changed = true;
                                }
                            }
                        }
                    }
                    if (changed) {
                        for (CoreLabel x : sentence) {
                            x.setValue(x.word()); //revert stem
                        }
                        if (pq.parse(sentence)) {
                            res = pq.getBestParse();
                            stemmer.visitTree(res);
                            labels = res.labeledYield();
                            gs = gsf.newGrammaticalStructure(res);
                        }
                    }
                    pwOut.println(Sentence.listToString(labels, false));
                    pwOut.println();
                    for (TypedDependency x : gs.typedDependencies()) {
                        pwOut.println(x);
                    }
                    pwOut.println();

                    pwErr.flush();
                    pwOut.flush();
                    result_output_done = true;
                }
            } catch (Throwable e) {
                lp = null;
                e.printStackTrace(pwErr);
                pwErr.flush();
            }
            if (!result_output_done) {
                pwOut.println("***###===SKIPPED===###***");
                pwOut.flush();
            }
            ++parsed;

            if (parsed >= 10000) {
                //lp = null;
                //parsed = 0;
                pwErr.println("Parsed 10000 Sentences. Restart.");
                pwErr.flush();
                if (fc != null) pipe_message(fc, pwPipe, "R" + myID, pwErr);
                break;
            }
        }

        if (fc != null && pwPipe != null) {
            try {
                fc.close();
                pwPipe.close();
                //fc = null;
                //pwPipe = null;
            } catch (IOException e) {
                pwErr.println("IOException during chanel and pipe final closing.");
                pwErr.flush();
            }
        }
        pwOut.close();
        pwErr.close();
    }
}
