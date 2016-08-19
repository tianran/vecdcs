#ifndef __FIBONACCI_HEAP_H
#define __FIBONACCI_HEAP_H

#include <utility>

template<class T>
class Fibonacci_Heap {
    
    struct Node {
        T data;
        const double key;
        
        Node* parent;
        Node* child;
        Node* right;
        Node* left;
        
        int degree;
        bool mark;
        
        Node(T&& d, double k)
        : data{std::move(d)}, key{k}, parent{}, child{}, right{this}, left{this}, degree{}, mark{} {}
        Node(const T& d, double k)
        : data{d}, key{k}, parent{}, child{}, right{this}, left{this}, degree{}, mark{} {}
        
        void cascadingCut(Node* min) {
            Node* z = parent;
            if (z) {
                if (mark) {
                    z->cut(this, min);
                    z->cascadingCut(min);
                } else {
                    mark = true;
                }
            }
        }
        
        void cut(Node* x, Node* min) {
            x->left->right = x->right;
            x->right->left = x->left;
            --degree;
            if (degree == 0) {
                child = nullptr;
            } else if (child == x) {
                child = x->right;
            }
            x->right = min;
            x->left = min->left;
            min->left = x;
            x->left->right = x;
            x->parent = nullptr;
            x->mark = false;
        }
        
        void link(Node* prt) {
            left->right = right;
            right->left = left;
            parent = prt;
            if (prt->child) {
                left = prt->child;
                right = prt->child->right;
                prt->child->right = this;
                right->left = this;
            } else {
                prt->child = this;
                right = this;
                left = this;
            }
            ++(prt->degree);
            mark = false;
        }
    };
    
    Node* min;
    int n;
    
    void insert_pointer(Node* node) {
        if (min) {
            node->right = min;
            node->left = min->left;
            min->left = node;
            node->left->right = node;
            if (node->key < min->key) min = node;
        } else {
            min = node;
        }
        ++n;
    }
    
    void recursive_delete(Node*);
    void consolidate();
    
public:
    Fibonacci_Heap(): min{}, n{} {}

    const T& min_data() const { return min->data; }
    double min_key() const { return min->key; }
    int size() const { return n; }
    bool non_empty() const { return static_cast<bool>(min); }
    
    void clear() { recursive_delete(min); min = nullptr; n = 0; }
    ~Fibonacci_Heap() { clear(); }
    
    void insert(T&& d, double k) { insert_pointer(new Node(std::move(d), k)); }
    void insert(const T& d, double k) { insert_pointer(new Node(d, k)); }
    T remove_min();
};


template<class T>
void Fibonacci_Heap<T>::recursive_delete(Node* z) {
    if (z) {
        recursive_delete(z->child);
        Node* x = z->right;
        delete z;
        while (x != z) {
            recursive_delete(x->child);
            Node* tmp = x->right;
            delete x;
            x = tmp;
        }
    }
}

template<class T>
T Fibonacci_Heap<T>::remove_min() {
    Node* z = min;
    if (z->child) {
        z->child->parent = nullptr;
        Node* x = z->child->right;
        while (x != z->child) {
            x->parent = nullptr;
            x = x->right;
        }
        Node* minleft = min->left;
        Node* zchildleft = z->child->left;
        min->left = zchildleft;
        zchildleft->right = min;
        z->child->left = minleft;
        minleft->right = z->child;
    }
    z->left->right = z->right;
    z->right->left = z->left;
    if (z == z->right) {
        min = nullptr;
    } else {
        min = z->right;
        consolidate();
    }
    --n;
    T tmp {std::move(z->data)};
    delete z;
    return std::move(tmp);
}

template<class T>
void Fibonacci_Heap<T>::consolidate() {
    Node* A[45] {};

    Node* start = min;
    Node* w = min;
    do {
        Node* x = w;
        Node* nextW = w->right;
        int d = x->degree;
        while (A[d]) {
            Node* y = A[d];
            if (x->key > y->key) {
                Node* tmp = y;
                y = x;
                x = tmp;
            }
            if (y == start) start = start->right;
            if (y == nextW) nextW = nextW->right;
            y->link(x);
            A[d] = nullptr;
            ++d;
        }
        A[d] = x;
        w = nextW;
    } while (w != start);

    min = start;
    for (int i = 0; i < 45; ++i) {
        if (A[i] && A[i]->key < min->key) min = A[i];
    }
}

#endif //__FIBONACCI_HEAP_H
