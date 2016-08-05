package dcstree_simple

class Node(val word: Word) {

  private var dp = null:Node
  def direct_parent = dp

  private[this] var dol = null:String
  def direct_ol = dol
  private[this] var dul = null:String
  def direct_ul = dul
  def isConjEdge = dul == null && dp != null
  lazy val isConj: Boolean = isConjEdge && (dp.direct_ul != null || dp.isConj)

  def addEdge(ol: String, ul: String, p: Node): Unit = {
    assert(dp == null)
    dp = p
    dol = ol
    dul = ul
  }

  private var hcj = false
  def hasConj = hcj

  def addConj(p: Node, ol: String): Unit = {
    assert(dp == null)
    dp = p
    dol = ol
    p.hcj = true
  }

  private[this] var tr = null:Tree
  def tree = tr
  def setTree(t: Tree): Unit = {
    assert(tr == null)
    tr = t
  }

  private[this] var cd = -1
  def code = cd
  def setCode(c: Int): Unit = {
    assert(cd == -1)
    cd = c
  }

  private[this] var idx = -1
  def index = idx
  def setIndex(i: Int): Unit = {
    assert(idx == -1)
    idx = i
  }

  lazy val oLabel: String = if (direct_parent == null) {
    "Null"
  } else if (direct_ol == null) {
    assert(isConjEdge)
    if (isConj) {
      direct_parent.oLabel
    } else {
      "ARG"
    }
  } else {
    direct_ol
  }

  lazy val uLabel: String = if (direct_parent == null) {
    "Null"
  } else if (isConj) {
    direct_parent.uLabel
  } else if (isConjEdge) {
    "ARG"
  } else {
    direct_ul
  }

  lazy val parent: Node = if (direct_parent == null) {
    null
  } else if (isConj) {
    direct_parent.parent
  } else {
    direct_parent
  }

  override def toString = {
    val pidx = if (parent == null) -1 else parent.index
    assert(pidx < index)
    Seq(index, code, word, oLabel, uLabel, pidx).mkString("\t")
  }

  var workingLink = null:Any
}
