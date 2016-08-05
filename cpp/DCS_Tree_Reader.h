#ifndef DCS_TREE_READER_H
#define DCS_TREE_READER_H

#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <utility>

template <class DCS_Node_Type>
std::vector<DCS_Node_Type> read_dcs_tree(std::istream& is, int ln) {
    std::vector<DCS_Node_Type> ret;
    ret.reserve(ln);
    for (int i = 0; i != ln; ++i) {
        std::string idx_str, code_str, word, ol, ul, pid_str;
        std::getline(is, idx_str, '\t');
        std::getline(is, code_str, '\t');
        std::getline(is, word, '\t');
        std::getline(is, ol, '\t');
        std::getline(is, ul, '\t');
        std::getline(is, pid_str);
        assert(std::stoi(idx_str) == i);
        int pid = std::stoi(pid_str);
        assert(i != 0 || pid == -1);
        DCS_Node_Type* p = (i == 0)? nullptr : &ret[pid];
        ret.emplace_back(std::stoi(code_str), std::move(word), std::move(ol), std::move(ul), p);
    }
    std::string line;
    std::getline(is, line);
    assert(line.empty());

    ret[0].set_goes_down();
    for (auto& x: ret) x.set_goes_up();

    return ret;
}

#endif //DCS_TREE_READER_H
