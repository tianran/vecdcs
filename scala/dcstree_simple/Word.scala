package dcstree_simple

import POS.POS

case class Word(lem: String, pos: POS) {
  override def toString = lem + "/" + pos.toString
}
