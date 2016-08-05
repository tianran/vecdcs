#ifndef SIMPLE_VECDCS_NODE_H
#define SIMPLE_VECDCS_NODE_H

#include "Simple_DCS_Node.h"
#include "VecDCS_Model.h"

#include <string>
#include "Eigen/Core"

class Simple_VecDCS_Node: public Simple_DCS_Node {

    Eigen::RowVectorXf sub_tree_vec;
    Eigen::RowVectorXf from_up_vec;
    Eigen::RowVectorXf surrounding_vec;

public:
    Simple_VecDCS_Node(int c, std::string&& w, std::string&& ol, std::string&& ul, Simple_VecDCS_Node* p);

    const Eigen::RowVectorXf& get_sub_tree_vec() const;
    const Eigen::RowVectorXf& get_from_up_vec() const;
    const Eigen::RowVectorXf& get_surrounding_vec() const;

    void substitute(const Eigen::RowVectorXf& vec);
    void calc_sub_tree_vec(const VecDCS_Model& vm);
    void calc_from_up_vec(const VecDCS_Model& vm);
    void calc_surrounding_vec(const VecDCS_Model& vm);
};

#endif //SIMPLE_VECDCS_NODE_H
