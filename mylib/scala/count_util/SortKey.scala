package mylib.count_util

import scala.util.Sorting

object SortKey {

  def main(args: Array[String]) {

    val fn = args(0)

    val to_sort = {
      val ret = Seq.newBuilder[(Double, String)]
      val file = io.Source.fromFile(fn)
      for (s <- file.getLines()) {
        val sp = s.split("\t", 2)
        ret += ((sp(0).toDouble, sp(1)))
      }
      file.close()
      ret.result()
    }

    val ord = math.Ordering[String].on[(Double, String)](_._2)
    val res = Sorting.stableSort[(Double, String)](to_sort, ord.lt _)

    for ((count, key) <- res) {
      println(count.toString + "\t" + key)
    }

  }
}
