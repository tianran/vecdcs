package ud_to_dcs.en_stanford_simple

import dcstree_simple.Node
import dcstree_simple.POS.POS

class TokenNode(val lemma: String, val pennPOS: String, val index: Int) {

  private[this] var par = null:TokenNode
  def parent = par

  private[this] var rel = null:String
  def relation = rel


  private var hws = false
  def has_whose = hws

  private[this] var whd = null:TokenNode
  def setWhDep(x: TokenNode): Unit = {
    //assert(whd == null)
    if (whd == null || (whd.lemma.toLowerCase == "that" && x.lemma.toLowerCase != "that")) {
      whd = x
    }
  }
  def wh_dep = whd

  private var hsbj = false
  def has_subj = hsbj

  private var hcmp = false
  def has_comp = hcmp

  private var hc = false
  def has_cop = hc

  private var pas = false
  def passive = pas
  def setPassiveFromAclVBN(): Unit = {
    pas = true
  }

  private var hvbn = false
  def havingVBN = hvbn

  def addParent(r: String, p: TokenNode): Unit = {
    assert(par == null)
    par = p
    rel = r

    if (pennPOS == "WP$") {
      p.hws = true
    } else if (pennPOS matches "W.+") {
      p.setWhDep(this)
    }

    if (r matches "[nc]subj(pass)?") {
      p.hsbj = true
    } else if (r matches "dobj|ccomp|[nc]subjpass") {
      p.hcmp = true
    } else if (r == "auxpass") {
      p.pas = true
    } else if (r == "cop") {
      p.hc = true
    } else if (r == "aux" && lemma.toLowerCase == "have" && pennPOS == "VBG" && p.pennPOS == "VBN") {
      p.hvbn = true
    }
    //println(rel + " " + p.lemma + " " + lemma)
  }

  var dcs_node = null:Node
  var dcs_word_lem = null:String
  var dcs_word_pos = null:POS
  private[this] var cs = null:String
  def setCase(x: String): Unit = {
    cs = x
  }
  def case_label = cs
  private[this] var ms = null:String
  def setMark(x: String): Unit = {
    ms = x
  }
  def mark_label = ms
  private[this] var nmt = null:String
  def setNmodTo(x: String): Unit = {
    nmt = x
  }
  def nmod_to = nmt

  var workingLink = null:Any
}
