package ud_to_dcs.en_stanford_simple

import java.io.{FileReader, BufferedReader}

import dcstree_simple.{Word, POS, Node, Tree}

import scala.collection.BitSet

class DepTree(taggedSent: String) {

  val tokens = {
    val ret = IndexedSeq.newBuilder[TokenNode]
    ret += new TokenNode("ROOT", "ROOT", 0)
    val sp = taggedSent.split(" ")
    for (i <- 0 until sp.length) {
      val s = sp(i)
      val li = s.lastIndexOf('/')
      ret += new TokenNode(s.substring(0, li), s.substring(li + 1), i + 1)
    }
    ret.result()
  }

  private[this] var prevDepIndex = 0
  private[this] var oneRoot = false
  private[this] val govSetBuilder = BitSet.newBuilder
  private[this] val depSetBuilder = BitSet.newBuilder

  def addDep(depstr: String): Unit = {
    val regex = """([a-z:]+)\((.+), (.+)\)""".r
    depstr match {
      case regex(rel, gov, dep) =>

        assert(DepTree.validRelations(rel))

        val gli = gov.lastIndexOf('-')
        val gi = gov.substring(gli + 1).toInt

        //we have to comment out this... but really, it should stand
        //assert(tokens(gi).lemma == gov.substring(0, gli))

        if (gi == 0) {//ROOT
          assert(!oneRoot)
          oneRoot = true
        } else {
          govSetBuilder += gi
        }

        val dli = dep.lastIndexOf('-')
        val di = dep.substring(dli + 1).toInt

        //we have to comment out this... but really, it should stand
        //assert(tokens(di).lemma == dep.substring(0, dli))

        assert(di > prevDepIndex)
        prevDepIndex = di
        depSetBuilder += di

        if (tokens(di).parent != null) {
          //really, this should not happen... just some quick fix
          if (rel == "parataxis") {
            if (tokens(gi).parent == null) {
              //try reverse
              tokens(gi).addParent(rel, tokens(di))
            } else {
              //ignore
            }
          } else if (rel == "dep") {
            //ignore
          }
        } else {
          tokens(di).addParent(rel, tokens(gi))
        }
      case _ =>
        throw new Exception("invalid depstr!")
    }
  }

  def validDepTree(): Unit = {
    assert(oneRoot)
    val govSet = govSetBuilder.result()
    val depSet = depSetBuilder.result()
    assert(govSet.subsetOf(depSet))
  }

  lazy val to_dcs = {
    val ret = Seq.newBuilder[Tree]
    for (x <- tokens) {
      if (x.pennPOS.matches("""NN.*|PRP.*|CD|\$|#""") || (x.pennPOS.matches("JJ.*|VB.*") && x.relation != null && x.relation.matches("nsubj(pass)?|dobj"))) {
        x.dcs_word_pos = POS.NOUN
      } else if (x.pennPOS matches "JJ.*") {
        x.dcs_word_pos = POS.ADJ
      } else if ((x.pennPOS matches "RB.*") && x.lemma.toLowerCase != "not") {
        x.dcs_word_pos = POS.ADV
      } else if (x.pennPOS == "DT" && Set("some", "any", "all", "either", "neither", "both")(x.lemma.toLowerCase)) {
        x.dcs_word_pos = POS.NOUN
      } else if (x.pennPOS matches "VB.*") {
        x.dcs_word_pos = POS.VERB
      }
      if (x.relation == "compound:prt" || x.relation == "mwe") {
        x.parent.dcs_word_lem = x.parent.dcs_word_lem + " " + x.lemma.toLowerCase
      } else {
        if ((x.pennPOS == "CD" || x.relation == "nummod" || x.relation == "compound") && x.lemma.matches("""[\+\-0-9,\.]+""")) {
          x.dcs_word_lem = "*NUMERAL*"
        } else if (x.relation == "nmod:poss" && x.lemma.toLowerCase == "my") {
          x.dcs_word_lem = "I"
        } else if (x.lemma.toUpperCase == x.lemma) {
          // all upper case
          x.dcs_word_lem = x.lemma
        } else {
          x.dcs_word_lem = x.lemma.toLowerCase
        }
        if (x.pennPOS matches "PRP.*") {
          x.dcs_word_lem = x.dcs_word_lem + "_Pr"
        }
      }
      if (x.has_whose && x.parent != null) x.parent.setWhDep(x)
    }

    for (x <- tokens) {
      if (x.dcs_word_pos != null && x.dcs_word_lem != null) x.dcs_node = new Node(Word(x.dcs_word_lem, x.dcs_word_pos))
      if (x.relation == "case") x.parent.setCase(x.dcs_word_lem)
      if (x.relation == "mark") x.parent.setMark(x.dcs_word_lem)
    }

    for (x <- tokens) {
      if (x.relation == "nmod" && x.parent.mark_label == "to" && x.case_label == null && x.dcs_word_pos == null) x.parent.setNmodTo(x.dcs_word_lem)
      if (x.relation == "conj" && x.parent.parent != null && x.parent.relation.matches("nmod(:npmod)?")
        && x.parent.pennPOS == "IN" && x.parent.case_label == null && x.dcs_node != null) {
        x.parent.setCase(x.parent.lemma.toLowerCase)
        x.parent.dcs_word_lem = x.dcs_word_lem
        x.parent.dcs_word_pos = x.dcs_word_pos
        x.parent.dcs_node = new Node(Word(x.dcs_word_lem, x.dcs_word_pos))
      }
    }

    def subj_label(p: TokenNode) = {
      if (p.has_cop) {
        if (p.case_label == null) {
          if (Set(POS.NOUN, POS.ADV)(p.dcs_word_pos)) {
            "ARG"
          } else {
            "SUBJ"
          }
        } else {
          p.case_label
        }
      } else {
        "SUBJ"
      }
    }

    for (x <- tokens; if x.dcs_node != null && x.parent != null && x.parent.dcs_node != null) {
      val rel = x.relation.split(":", 2)(0)
      if (rel == "nsubj") {
        x.dcs_node.addEdge("ARG", subj_label(x.parent), x.parent.dcs_node)
      } else if (rel == "csubj") {
        val ol = if (x.wh_dep == null || x.wh_dep.lemma.toLowerCase != "what") {
          "ARG"
        } else {
          if (x.wh_dep.relation == "nsubj") {
            "SUBJ"
          } else if (Set("nsubjpass", "dobj")(x.wh_dep.relation)) {
            "COMP"
          } else {
            "ARG"
          }
        }
        x.dcs_node.addEdge(ol, subj_label(x.parent), x.parent.dcs_node)
      } else if (rel == "nsubjpass") {
        x.dcs_node.addEdge("ARG", "COMP", x.parent.dcs_node)
      } else if (rel == "csubjpass") {
        val ol = if (x.wh_dep == null || x.wh_dep.lemma.toLowerCase != "what") {
          "ARG"
        } else {
          if (x.wh_dep.relation == "nsubj") {
            "SUBJ"
          } else if (Set("nsubjpass", "dobj")(x.wh_dep.relation)) {
            "COMP"
          } else {
            "ARG"
          }
        }
        x.dcs_node.addEdge(ol, "COMP", x.parent.dcs_node)
      } else if (rel == "dobj") {
        x.dcs_node.addEdge("ARG", "COMP", x.parent.dcs_node)
      } else if (rel == "ccomp") {
        val ol = if (x.wh_dep == null) {
          "ARG"
        } else {
          if (x.wh_dep.relation == "nsubj") {
            "SUBJ"
          } else if (Set("nsubjpass", "dobj")(x.wh_dep.relation)) {
            "COMP"
          } else {
            "ARG"
          }
        }
        x.dcs_node.addEdge(ol, "COMP", x.parent.dcs_node)
      } else if (rel == "xcomp") {
        val ol = if (Set(POS.VERB, POS.ADJ)(x.dcs_word_pos)) {
          if (x.pennPOS == "VBN") {
            "COMP"
          } else {
            "SUBJ"
          }
        } else {
          "ARG"
        }
        if (x.has_subj) {
          x.dcs_node.addEdge("ARG", "COMP", x.parent.dcs_node)
        } else if (x.parent.has_comp) {
          x.dcs_node.addEdge(ol, "COMP", x.parent.dcs_node)
        } else {
          x.dcs_node.addEdge(ol, "SUBJ", x.parent.dcs_node)
        }
      } else if (rel == "iobj") {
        x.dcs_node.addEdge("iobj", "ARG", x.parent.dcs_node)
      } else if (rel == "amod") {
        val ol = if (Set(POS.NOUN, POS.ADV)(x.dcs_word_pos)) {
          "ARG"
        } else if (x.pennPOS == "VBN") {
          "COMP"
        } else {
          "SUBJ"
        }
        x.dcs_node.addEdge(ol, "ARG", x.parent.dcs_node)
      } else if (rel == "appos") {
        x.dcs_node.addEdge("ARG", "ARG", x.parent.dcs_node)
      } else if (rel == "nmod") {
        if (x.relation == "nmod:tmod") {
          x.dcs_node.addEdge("when", "ARG", x.parent.dcs_node)
        } else if (x.relation == "nmod:poss") {
          x.dcs_node.addEdge("'s", "ARG", x.parent.dcs_node)
        } else if (x.parent.passive && x.case_label == "by") {
          x.dcs_node.addEdge("ARG", "SUBJ", x.parent.dcs_node)
        } else if (x.case_label != null) {
          x.dcs_node.addEdge(x.case_label, "ARG", x.parent.dcs_node)
        } else if (x.relation == "nmod:npmod") {
          x.dcs_node.addEdge("ARG", "ARG", x.parent.dcs_node)
        }
      } else if (rel == "dep") {
        if (x.parent.dcs_word_pos == POS.NOUN && x.dcs_word_pos == POS.NOUN) {
          x.dcs_node.addEdge("ARG", "ARG", x.parent.dcs_node)
        } else if (x.parent.dcs_word_pos == POS.VERB && x.pennPOS == "VBN" && !x.has_subj && !x.has_comp) {
          // "Overturned fishing skiffs lie abandoned on the shores."
          x.dcs_node.addEdge("COMP", "SUBJ", x.parent.dcs_node)
        }
      } else if (rel == "acl" || (rel == "advcl" && x.parent.dcs_word_pos == POS.NOUN && x.parent.has_cop)) {
        if (x.wh_dep == null) {
          if (x.dcs_word_pos == POS.VERB) {
            if (x.mark_label == "to") {
              if (x.nmod_to == null) {
                if (x.has_comp) {
                  x.dcs_node.addEdge("SUBJ", "ARG", x.parent.dcs_node)
                } else {
                  x.dcs_node.addEdge("COMP", "ARG", x.parent.dcs_node)
                }
              } else {
                x.dcs_node.addEdge("ARG", x.nmod_to, x.parent.dcs_node)
              }
            } else if (!x.has_subj && (x.pennPOS == "VBG" || (x.havingVBN && !x.passive))) {
              x.dcs_node.addEdge("SUBJ", "ARG", x.parent.dcs_node)
            } else if (!x.has_comp && (x.has_subj || x.pennPOS == "VBN")) {
              x.setPassiveFromAclVBN()
              x.dcs_node.addEdge("COMP", "ARG", x.parent.dcs_node)
            }
          } else if (x.has_cop) {
            x.dcs_node.addEdge(subj_label(x), "ARG", x.parent.dcs_node)
          }
        } else {
          if (x.wh_dep.relation == "nsubj") {
            val ul = if (x.wh_dep.has_whose) "'s" else "ARG"
            x.dcs_node.addEdge(subj_label(x), ul, x.parent.dcs_node)
          } else if (Set("nsubjpass", "dobj")(x.wh_dep.relation)) {
            val ul = if (x.wh_dep.has_whose) "'s" else "ARG"
            x.dcs_node.addEdge("COMP", ul, x.parent.dcs_node)
          } else if (!x.wh_dep.has_whose && x.wh_dep.relation == "advmod") {
            x.dcs_node.addEdge("ARG", x.wh_dep.lemma.toLowerCase, x.parent.dcs_node)
          } else if (!x.wh_dep.has_whose && x.wh_dep.relation == "nmod" && x.wh_dep.case_label != null) {
            x.dcs_node.addEdge("ARG", x.wh_dep.case_label, x.parent.dcs_node)
          }
        }
      } else if (rel == "advcl") {
        if (x.wh_dep == null) {
          if (x.parent.passive && x.mark_label == "by") {
            x.dcs_node.addEdge("ARG", "SUBJ", x.parent.dcs_node)
          } else if (x.mark_label != null) {
            x.dcs_node.addEdge(x.mark_label, "ARG", x.parent.dcs_node)
          } else if (x.index > x.parent.index) {
            x.dcs_node.addEdge("ARG", "ARG", x.parent.dcs_node)
          }
        } else if (!x.wh_dep.has_whose && x.wh_dep.relation == "advmod") {
          x.dcs_node.addEdge(x.wh_dep.lemma.toLowerCase, "ARG", x.parent.dcs_node)
        }
      } else if (rel == "advmod") {
        x.dcs_node.addEdge("ARG", "ARG", x.parent.dcs_node)
      } else if (rel == "conj") {
        if (x.parent.has_cop || (x.parent.dcs_word_pos == POS.VERB && (x.has_cop || x.dcs_word_pos == POS.VERB))) {
          if (x.has_subj) {
            x.dcs_node.addConj(x.parent.dcs_node, x.case_label)
          } else {
            def subjrole(n: TokenNode) = {
              if (n.passive) {
                "COMP"
              } else if (Set(POS.VERB, POS.ADJ)(n.dcs_word_pos)) {
                "SUBJ"
              } else if (n.has_cop && n.case_label != null) {
                n.case_label
              } else {
                "ARG"
              }
            }
            x.dcs_node.addEdge(subjrole(x), subjrole(x.parent), x.parent.dcs_node)
          }
        } else if (x.dcs_word_pos == x.parent.dcs_word_pos) {
          x.dcs_node.addConj(x.parent.dcs_node, x.case_label)
        }
      } else if (rel == "compound" || rel == "name") {
        x.dcs_node.addEdge("ARG", "ARG", x.parent.dcs_node)
      } else if (rel == "nummod") {
        x.dcs_node.addEdge("ARG", "ARG", x.parent.dcs_node)
      }
    }

    for (x <- tokens; if x.dcs_node != null && x.dcs_node.tree == null) {
      def loop_parent(y: Node, tr: List[Node]): Unit = {
        val tmp = y :: tr
        if (y.direct_parent == null) {
          val tree = new Tree
          ret += tree
          tree.addNodes(tmp)
        } else if (y.direct_parent.tree == null) {
          loop_parent(y.direct_parent, tmp)
        } else {
          y.direct_parent.tree.addNodes(tmp)
        }
      }
      loop_parent(x.dcs_node, Nil)
    }

    ret.result().filter(_.size >= 2)
  }

}

object DepTree {

  val validRelations = Set(
    "nmod",
    "case",
    "det",
    "nsubj",
    "compound",
    "dobj",
    "amod",
    "conj",
    "dep",
    "advmod",
    "cc",
    "advcl",
    "xcomp",
    "acl",
    "nummod",
    "nsubjpass",
    "nmod:poss",
    "mark",
    "appos",
    "acl:relcl",
    "auxpass",
    "ccomp",
    "cop",
    "aux",
    "nmod:tmod",
    "compound:prt",
    "neg",
    "parataxis",
    "mwe",
    "nmod:npmod",
    "expl",
    "iobj",
    "csubj",
    "det:predet",
    "cc:preconj",
    "csubjpass",
    "discourse",
    "list",
    "root"
  )

  def readFrom(input: BufferedReader): DepTree = {
    val sent = input.readLine()
    if (sent == "***###===SKIPPED===###***") {
      null
    } else {
      //System.err.println(sent)
      val ret = new DepTree(sent)
      //System.err.println()
      assert(input.readLine().isEmpty)
      def loop(): Unit = {
        val s = input.readLine()
        if (s.nonEmpty) {
          //System.err.println(s)
          ret.addDep(s)
          loop()
        }
      }
      loop()
      //System.err.println()
      //we have to comment out this... sometimes the output of stanford parser is not a valid tree
      //ret.validDepTree()
      ret
    }
  }

  def main(args: Array[String]) {
    val fn = args(0)

    val input = new BufferedReader(new FileReader(fn))
    def loop(): Unit = {
      val s = input.readLine()
      if (s != null) {
        val dt = readFrom(input)
        if (dt != null) {
          for (x <- dt.to_dcs) {
            println(x.mkString)
          }
        } else {
          //assert(s.split("\t").length >= 54)
        }
        loop()
      }
    }
    loop()
    input.close()
  }
}
