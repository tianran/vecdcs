package mylib.count_util

import scala.collection.mutable

object CutOffWordsWithPOS {

  def main(args: Array[String]) {

    val fn = args(0)
    val min_count = args(1).toDouble

    val unknown_count_by_POS = mutable.Map.empty[String, Double]

    val file = io.Source.fromFile(fn)
    for (s <- file.getLines()) {
      val sp = s.split("\t", 2)
      val count = sp(0).toDouble
      if (count < min_count) {
        val pos = sp(1).substring(sp(1).lastIndexOf('/') + 1, sp(1).length)
        if (!unknown_count_by_POS.contains(pos)) {
          unknown_count_by_POS(pos) = count
        } else {
          unknown_count_by_POS(pos) += count
        }
      } else {
        println(s)
      }
    }
    file.close()

    for ((x, v) <- unknown_count_by_POS) {
      println(v.toString + "\t*UNKNOWN*/" + x)
    }
  }
}
