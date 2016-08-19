#include "Matrix_Aux.h"

#include <cassert>
#include "DAO.h"

using namespace std;
using namespace Eigen;

namespace Matrix_Aux {

    void write_matrix(ostream& os, const MatrixXf& mat) {
        DAO::write_magic(os, "MATf");
        DAO::write_long(os, mat.rows());
        DAO::write_long(os, mat.cols());
        for (long i = 0; i != mat.size(); ++i) {
            DAO::write_float(os, *(mat.data() + i));
        }
    }

    void read_matrix(istream& is, MatrixXf& mat) {
        assert(DAO::read_magic(is) == "MATf");
        assert(DAO::read_long(is) == mat.rows());
        assert(DAO::read_long(is) == mat.cols());
        for (float* p = mat.data(); p != mat.data() + mat.size(); ++p) {
            *p = DAO::read_float(is);
        }
    }

    void read_matrix_resize(istream& is, MatrixXf& mat) {
        assert(DAO::read_magic(is) == "MATf");
        assert(mat.size() == 0);
        long rows = DAO::read_long(is);
        long cols = DAO::read_long(is);
        mat.resize(rows, cols);
        for (float* p = mat.data(); p != mat.data() + mat.size(); ++p) {
            *p = DAO::read_float(is);
        }
    }
}
