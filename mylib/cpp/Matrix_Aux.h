#ifndef __MATRIX_AUX_H
#define __MATRIX_AUX_H

#include <iostream>

#include "Eigen/Core"

namespace Matrix_Aux {

    void write_matrix(std::ostream& os, const Eigen::MatrixXf& mat);

    void read_matrix(std::istream& is, Eigen::MatrixXf& mat);
    void read_matrix_resize(std::istream& is, Eigen::MatrixXf& mat);

}

#endif //__MATRIX_AUX_H
