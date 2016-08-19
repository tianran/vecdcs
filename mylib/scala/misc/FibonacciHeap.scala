package mylib.misc

class FibonacciHeap[T >: Null <: AnyRef](implicit ord: math.Ordering[T]) {

  type Node = FibonacciHeap.Node[T]

  private[this] var min = null: Node
  /** The minimum node in the heap. */
  def minNode = min

  private[this] var n = 0
  /** Number of nodes in the heap. */
  def size = n

  /**
   * Removes all elements from this heap.
   */
  def clear() {
    min = null
    n = 0
  }

  /**
   * Deletes a node from the heap given the reference to the node.
   * The trees in the heap will be consolidated, if necessary.
   *
   * @param  x  node to remove from heap.
   */
  def delete(x: Node) {
    val y = x.parent
    if (y != null) {
      y.cut(x, min)
      y.cascadingCut(min)
    }
    min = x
    removeMin()
  }

  def isEmpty = min == null
  def nonEmpty = min != null

  /**
   * Inserts a new data element into the heap. No heap consolidation
   * is performed at this time, the new node is simply inserted into
   * the root list of this heap.
   *
   * @param  x    data object to insert into heap.
   * @return newly created heap node.
   */
  def insert(x: T) = {
    val node = new Node(x)
    if (min != null) {
      node.right = min
      node.left = min.left
      min.left = node
      node.left.right = node
      if (ord.lt(x, min.data)) {
        min = node
      }
    } else {
      min = node
    }
    n += 1
    node
  }

  /**
   * Removes the smallest element from the heap. This will cause
   * the trees in the heap to be consolidated, if necessary.
   *
   * @return  data object with the smallest key.
   */
  def removeMin() = {
    val z = min
    if (z == null) {
      null: T
    } else {
      if (z.child != null) {
        z.child.parent = null
        var x = z.child.right
        while (x != z.child) {
          x.parent = null
          x = x.right
        }
        val minleft = min.left
        val zchildleft = z.child.left
        min.left = zchildleft
        zchildleft.right = min
        z.child.left = minleft
        minleft.right = z.child
      }
      z.left.right = z.right
      z.right.left = z.left
      if (z == z.right) {
        min = null
      } else {
        min = z.right
        consolidate()
      }
      n -= 1
      z.data
    }
  }

  private[this] def consolidate() {
    val A = new Array[Node](45)

    var start = min
    var w = min
    do {
      var x = w
      var nextW = w.right
      var d = x.degree
      while (A(d) != null) {
        var y = A(d)
        if (ord.gt(x.data, y.data)) {
          val tmp = y
          y = x
          x = tmp
        }
        if (y == start) {
          start = start.right
        }
        if (y == nextW) {
          nextW = nextW.right
        }
        y.link(x)
        A(d) = null
        d += 1
      }
      A(d) = x
      w = nextW
    } while (w != start)

    min = start
    for (a <- A; if a != null) {
      if (ord.lt(a.data, min.data)) min = a
    }
  }

}

object FibonacciHeap {

  /** Implements a node of the Fibonacci heap. */
  class Node[T](val data: T) {

    private[FibonacciHeap] var parent = null: Node[T]
    private[FibonacciHeap] var child = null: Node[T]
    private[FibonacciHeap] var right = this
    private[FibonacciHeap] var left = this

    private[FibonacciHeap] var degree = 0
    private[FibonacciHeap] var mark = false

    private[FibonacciHeap] def cascadingCut(min: Node[T]) {
      val z = parent
      if (z != null) {
        if (mark) {
          z.cut(this, min)
          z.cascadingCut(min)
        } else {
          mark = true
        }
      }
    }

    private[FibonacciHeap] def cut(x: Node[T], min: Node[T]) {
      x.left.right = x.right
      x.right.left = x.left
      degree -= 1
      if (degree == 0) {
        child = null
      } else if (child == x) {
        child = x.right
      }
      x.right = min
      x.left = min.left
      min.left = x
      x.left.right = x
      x.parent = null
      x.mark = false
    }

    private[FibonacciHeap] def link(prt: Node[T]) {
      left.right = right
      right.left = left
      parent = prt
      if (prt.child == null) {
        prt.child = this
        right = this
        left = this
      } else {
        left = prt.child
        right = prt.child.right
        prt.child.right = this
        right.left = this
      }
      prt.degree += 1
      mark = false
    }
  }
}

