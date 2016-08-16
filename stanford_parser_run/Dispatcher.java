import edu.stanford.nlp.ling.CoreAnnotations;
import edu.stanford.nlp.ling.CoreLabel;
import edu.stanford.nlp.ling.HasWord;
import edu.stanford.nlp.parser.lexparser.Options;
import edu.stanford.nlp.process.DocumentPreprocessor;
import edu.stanford.nlp.trees.TreebankLanguagePack;

import java.io.*;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class Dispatcher {

    public static void main(String[] args) throws IOException {

        String fnlist_fn = args[0];
        int para = Integer.parseInt(args[1]);
        String formatter = "%0" + args[1].length() + "d";

        String pipe_path = args[2];
        String lock_file_path = args[3];
        String log_path_prefix = args[4];
        String parser_model_file_path = args[5];

        int argIndex = 6;
        Options op = new Options();
        List<String> optionArgs = new ArrayList<>();
        while (argIndex < args.length && args[argIndex].charAt(0) == '-') {
            int oldIndex = argIndex;
            argIndex = op.setOptionOrWarn(args, argIndex);
            optionArgs.addAll(Arrays.asList(args).subList(oldIndex, argIndex));
        }

        List<String> cmdstrs = new ArrayList<>(Arrays.asList(args).subList(argIndex, args.length));
        cmdstrs.add("id_AUTO");
        cmdstrs.add(pipe_path);
        cmdstrs.add(lock_file_path);
        cmdstrs.add(log_path_prefix);
        cmdstrs.add(parser_model_file_path);
        cmdstrs.addAll(optionArgs);
        ObjectOutputStream[] ooss = new ObjectOutputStream[para];
        for (int i = 0; i < para; ++i) {
            cmdstrs.set(args.length - argIndex, String.format(formatter, i));
            ProcessBuilder pb = new ProcessBuilder(cmdstrs);
            ooss[i] = new ObjectOutputStream(pb.start().getOutputStream());
            ooss[i].flush();
        }

        final String[] puncs = {".", ";", "?", "!"};

        BufferedReader pipe_input = new BufferedReader(new FileReader(pipe_path));

        long paragraph_id = 0;
        BufferedReader fnlist_br = new BufferedReader(new FileReader(fnlist_fn));
        String filename = fnlist_br.readLine();
        while (filename != null) {

            BufferedReader file_br = new BufferedReader(new FileReader(filename));
            String line = file_br.readLine();
            while (line != null) {
                if (!line.isEmpty() && !line.matches("<.*>")) {
                    line = line.replace("()", "");

                    StringReader sr = new StringReader(line);
                    DocumentPreprocessor documentPreprocessor = new DocumentPreprocessor(sr, DocumentPreprocessor.DocType.Plain);
                    documentPreprocessor.setSentenceFinalPuncWords(puncs);
                    TreebankLanguagePack tlp = op.tlpParams.treebankLanguagePack();
                    documentPreprocessor.setTokenizerFactory(tlp.getTokenizerFactory());

                    int sentence_id = 0;
                    for (List<HasWord> sentence : documentPreprocessor) {

                        String msg = pipe_input.readLine();
                        while (msg.charAt(0) == 'R') {
                            int i = Integer.parseInt(msg.substring(1));
                            cmdstrs.set(args.length - argIndex, String.format(formatter, i));
                            ProcessBuilder pb = new ProcessBuilder(cmdstrs);
                            ooss[i] = new ObjectOutputStream(pb.start().getOutputStream());
                            ooss[i].flush();
                            msg = pipe_input.readLine();
                        }
                        ObjectOutputStream oos;
                        oos = ooss[Integer.parseInt(msg)];

                        StringBuilder sb = new StringBuilder();
                        sb.append(paragraph_id);
                        sb.append('\t');
                        sb.append(sentence_id);
                        for (HasWord hw : sentence) {
                            CoreLabel cl = (CoreLabel)hw;
                            Integer cob = cl.get(CoreAnnotations.CharacterOffsetBeginAnnotation.class);
                            sb.append('\t');
                            sb.append(cob);
                            sb.append('\t');
                            sb.append(line.substring(cob, cl.get(CoreAnnotations.CharacterOffsetEndAnnotation.class)));
                        }
                        oos.writeObject(sb.toString());
                        oos.writeObject(sentence);
                        oos.reset();
                        oos.flush();
                        ++sentence_id;
                    }
                    ++paragraph_id;
                }
                line = file_br.readLine();
            }
            System.out.println(filename);

            filename = fnlist_br.readLine();
        }

        for (int i = 0; i < para; ++i) {
            ooss[i].close();
        }
    }
}
