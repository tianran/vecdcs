package mylib.count_util

import java.io.{FileReader, BufferedReader}

import mylib.misc.FibonacciHeap

import scala.collection.mutable

object MergeCount {

  def main(args: Array[String]) {

    val brs = for (fn <- args) yield {
      new BufferedReader(new FileReader(fn))
    }

    val key_pool = mutable.Map.empty[String, Double]
    val key_heap = new FibonacciHeap[String]

    def process(br: BufferedReader): String = {
      val s = br.readLine()
      if (s == null) {
        br.close()
        null
      } else {
        val sp = s.split("\t", 2)
        val count = sp(0).toDouble
        val key = sp(1)
        if (key_pool.contains(key)) {
          key_pool(key) += count
        } else {
          key_pool(key) = count
          key_heap.insert(key)
        }
        key
      }
    }

    val top_keys = brs.map(process)

    def loop(): Unit = {

      if (key_heap.nonEmpty) {
        val key = key_heap.removeMin()
        println(key_pool(key).toString + "\t" + key)
        key_pool.remove(key)
        for (i <- top_keys.indices) {
          if (top_keys(i) != null && top_keys(i) <= key) {
            top_keys(i) = process(brs(i))
          }
        }
        loop()
      }
    }
    loop()

  }
}
