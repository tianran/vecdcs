package dcstree_simple

import scala.collection.{SortedMap, mutable}

class Tree {

  private[this] val ns = Seq.newBuilder[Node]
  lazy val nodes = ns.result()
  private[this] var sz = 0
  def size = sz

  private[this] var point_counter = 0
  def points = point_counter

  def addNodes(nl: List[Node]): Unit = {
    for (x <- nl) {
      assert(x.direct_parent == null || x.direct_parent.tree == this)
      x.setTree(this)

      val c = if (x.direct_parent == null) {
        0
      } else if (x.isConj) {
        x.direct_parent.code
      } else if (x.isConjEdge) {
        0
      } else if (x.hasConj) {
        point_counter += 1
        point_counter
      } else {
        0
      }
      x.setCode(c)

      x.setIndex(sz)
      sz += 1
    }
    ns ++= nl
  }

  def mkString = nodes.mkString(size.toString + "\n", "\n", "\n")

}
