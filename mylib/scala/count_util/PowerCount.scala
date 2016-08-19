package mylib.count_util

object PowerCount {

  def main(args: Array[String]) {

    val fn = args(0)
    val power = args(1).toDouble

    val file = io.Source.fromFile(fn)
    for (s <- file.getLines()) {
      val sp = s.split("\t", 2)
      val count = sp(0).toDouble
      val pow = math.pow(count, power)
      println(pow.toString + "\t" + sp(1))
    }
    file.close()
  }

}
