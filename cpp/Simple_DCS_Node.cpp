#include "Simple_DCS_Node.h"

#include "Trainer_Aux.h"

using namespace std;

Simple_DCS_Node::Simple_DCS_Node(int c, string &&w, string &&ol, string &&ul, Simple_DCS_Node* p)
        : code(c), word(move(w)), oLabel(move(ol)), uLabel(move(ul)), parent(p) {
    if (parent) parent->children.push_back(this);
}

double Simple_DCS_Node::set_goes_down() {
    double sum = 0.0;
    for (auto x: children) sum += x->set_goes_down();
    return (goes_down = (children.empty())? 1.0 : sum / children.size() + 1.0);
}

void Simple_DCS_Node::set_goes_up() {
    if (parent) {
        int counter = (parent->parent)? 1 : 0;
        double sum = parent->goes_up;
        for (auto x: parent->children) {
            if (x == this) continue;
            if (code != 0 && code == x->code) continue;
            sum += x->goes_down;
            ++counter;
        }
        goes_up = (counter == 0)? 1.0 : sum / counter + 1.0;
    } else {
        goes_up = 0.0;
    }
}

double Simple_DCS_Node::word_weight() const {
    double ret = goes_up;
    for (auto x: children) ret += x->goes_down;
    return ret;
}

double Simple_DCS_Node::role_weight() const {
    return goes_up * goes_down;
}

pair<vector<string>, string> Simple_DCS_Node::sample_path(Random_Generator &rand) const {
    vector<string> ret_path;

    vector<double> bd(children.size() + 1);
    bd[0] = goes_up;
    for (int i = 0; i != children.size(); ++i) {
        bd[i + 1] = bd[i] + children[i]->goes_down;
    }

    const Simple_DCS_Node* from;
    const Simple_DCS_Node* next;

    double dice = (rand() & 0xFFFFFFFFFFFFFULL) / double(0x10000000000000ULL) * bd.back();
    int choice = 0;
    while (dice >= bd[choice]) ++choice;
    if (choice == 0) {
        ret_path.push_back(oLabel);
        ret_path.push_back(uLabel);
        from = this;
        next = parent;
    } else {
        from = nullptr;
        next = children[choice - 1];
        ret_path.push_back(next->uLabel);
        ret_path.push_back(next->oLabel);
    }
    do {
        auto ncsz = next->children.size();
        vector<double> nbd(ncsz + 2);
        if (from) {
            int counter = (next->parent)? 1 : 0;
            nbd[0] = next->goes_up;
            for (int i = 0; i != ncsz; ++i) {
                auto nci = next->children[i];
                if (nci == from || (from->code != 0 && from->code == nci->code)) {
                    nbd[i + 1] = nbd[i];
                } else {
                    nbd[i + 1] = nbd[i] + nci->goes_down;
                    ++counter;
                }
            }
            nbd.back() = nbd[ncsz] + counter;
        } else {
            nbd[0] = 0.0;
            for (int i = 0; i != ncsz; ++i) {
                nbd[i + 1] = nbd[i] + next->children[i]->goes_down;
            }
            nbd.back() = nbd[ncsz] + ncsz;
        }
        if (nbd.back() == 0.0) {
            return make_pair(move(ret_path), string(next->word));
        }
        double ndice = (rand() & 0xFFFFFFFFFFFFFULL) / double(0x10000000000000ULL) * nbd.back();
        int nchoice = 0;
        while (ndice >= nbd[nchoice]) ++nchoice;
        if (nchoice == ncsz + 1) {
            return make_pair(move(ret_path), string(next->word));
        } else if (nchoice == 0) {
            ret_path.push_back(next->oLabel);
            ret_path.push_back(next->uLabel);
            from = next;
            next = next->parent;
        } else {
            from = nullptr;
            next = next->children[nchoice - 1];
            ret_path.push_back(next->uLabel);
            ret_path.push_back(next->oLabel);
        }
    } while(true);
}
