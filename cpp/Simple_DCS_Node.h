#ifndef SIMPLE_DCS_NODE_H
#define SIMPLE_DCS_NODE_H

#include <vector>
#include <string>
#include <iostream>

#include "Random_Generator.h"

class Simple_DCS_Node {

protected:
    double goes_up;
    double goes_down;

    std::vector<Simple_DCS_Node*> children;

public:
    int code;
    std::string word;
    std::string oLabel;
    std::string uLabel;
    Simple_DCS_Node* parent;

    Simple_DCS_Node(int c, std::string&& w, std::string&& ol, std::string&& ul, Simple_DCS_Node* p);

    double word_weight() const;
    double role_weight() const;
    std::pair<std::vector<std::string>, std::string> sample_path(Random_Generator& rand) const;

    double set_goes_down();
    void set_goes_up();

};


#endif //SIMPLE_DCS_NODE_H
