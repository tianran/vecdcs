#include "Simple_VecDCS_Node.h"

#include <utility>

using namespace std;
using namespace Eigen;

Simple_VecDCS_Node::Simple_VecDCS_Node(int c, string&& w, string&& ol, string&& ul, Simple_VecDCS_Node* p):
        Simple_DCS_Node(c, move(w), move(ol), move(ul), p) {}

const RowVectorXf& Simple_VecDCS_Node::get_sub_tree_vec() const {
    return sub_tree_vec;
}

const RowVectorXf& Simple_VecDCS_Node::get_from_up_vec() const {
    return from_up_vec;
}

const RowVectorXf& Simple_VecDCS_Node::get_surrounding_vec() const {
    return surrounding_vec;
}

void Simple_VecDCS_Node::substitute(const Eigen::RowVectorXf& vec) {
    assert(string(word, 0, 3) == "**_");
    assert(children.empty());
    assert(sub_tree_vec.size() == 0);
    sub_tree_vec = vec;
}

void Simple_VecDCS_Node::calc_sub_tree_vec(const VecDCS_Model& vm) {
    if (sub_tree_vec.size() == 0) {
        vm.set_query_vec(word, sub_tree_vec);
        float ratio = 1.0f / children.size();
        for (auto x : children) {
            if (string(x->word, 0, 3) == "**_") {
                assert(static_cast<Simple_VecDCS_Node *>(x)->sub_tree_vec.size() > 0);
                assert(static_cast<Simple_VecDCS_Node *>(x)->children.empty());
            } else {
                static_cast<Simple_VecDCS_Node *>(x)->calc_sub_tree_vec(vm);
            }
            RowVectorXf tmp;
            vm.trans_query_vec(static_cast<Simple_VecDCS_Node *>(x)->sub_tree_vec, x->oLabel, x->uLabel, tmp);
            sub_tree_vec += tmp * ratio;
        }
    }
}

void Simple_VecDCS_Node::calc_from_up_vec(const VecDCS_Model& vm) {
    if (from_up_vec.size() == 0) {
        if (parent) {
            RowVectorXf tmp;
            if (string(parent->word, 0, 3) == "**_") {
                assert(static_cast<Simple_VecDCS_Node *>(parent)->sub_tree_vec.size() > 0);
                assert(parent->parent == nullptr);
                assert(static_cast<Simple_VecDCS_Node *>(parent)->children.size() == 1);
                tmp = static_cast<Simple_VecDCS_Node *>(parent)->sub_tree_vec;
            } else {
                int counter = (parent->parent) ? 1 : 0;
                static_cast<Simple_VecDCS_Node *>(parent)->calc_from_up_vec(vm);
                tmp = static_cast<Simple_VecDCS_Node *>(parent)->from_up_vec;
                for (auto x: static_cast<Simple_VecDCS_Node *>(parent)->children) {
                    if (x == this) continue;
                    if (code != 0 && code == x->code) continue;
                    static_cast<Simple_VecDCS_Node *>(x)->calc_sub_tree_vec(vm);
                    RowVectorXf tmptmp;
                    vm.trans_query_vec(static_cast<Simple_VecDCS_Node *>(x)->sub_tree_vec, x->oLabel, x->uLabel, tmptmp);
                    tmp += tmptmp;
                    ++counter;
                }
                if (counter > 0) tmp /= counter;
                RowVectorXf tmptmp;
                vm.set_query_vec(parent->word, tmptmp);
                tmp += tmptmp;
            }
            vm.trans_query_vec(tmp, uLabel, oLabel, from_up_vec);
        } else {
            from_up_vec = RowVectorXf::Zero(vm.dimension());
        }
    }
}

void Simple_VecDCS_Node::calc_surrounding_vec(const VecDCS_Model& vm) {
    if (surrounding_vec.size() == 0) {
        int counter = (parent) ? 0 : -1;
        calc_from_up_vec(vm);
        RowVectorXf res = from_up_vec;
        for (auto x: children) {
            static_cast<Simple_VecDCS_Node *>(x)->calc_sub_tree_vec(vm);
            RowVectorXf tmp;
            vm.trans_query_vec(static_cast<Simple_VecDCS_Node *>(x)->sub_tree_vec, x->oLabel, x->uLabel, tmp);
            res += tmp;
            ++counter;
        }
        if (string(word, 0, 3) == "**_") assert(counter == 0 || (counter == -1 && res.isZero()));
        if (counter > 0) res /= counter;
        surrounding_vec = res;
    }
}
