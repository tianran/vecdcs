package mylib.count_util

object CutOff {

  def main(args: Array[String]) {

    val fn = args(0)
    val min_count = args(1).toDouble

    val unknown_count = {
      var ret = 0.0
      val file = io.Source.fromFile(fn)
      for (s <- file.getLines()) {
        val sp = s.split("\t", 2)
        val count = sp(0).toDouble
        if (count < min_count) {
          ret += count
        } else {
          println(s)
        }
      }
      file.close()
      ret
    }

    println(unknown_count.toString + "\t*UNKNOWN*")
  }
}
