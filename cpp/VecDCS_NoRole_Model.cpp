#include "VecDCS_NoRole_Model.h"

#include <fstream>
#include "Vocab_Aux.h"
#include "DAO.h"
#include "Matrix_Aux.h"

using namespace std;
using namespace Eigen;

VecDCS_NoRole_Model::VecDCS_NoRole_Model(const std::string &vocab_fn, const std::string &model_fn) {

    const long nw = Vocab_Aux::init_vocab(vocab_fn, word_map, vec_word); //Number of words
    role_map["*UNKNOWN*"] = 0;
    vec_role.push_back("*UNKNOWN*");

    ifstream model_file(model_fn);
    dim = DAO::read_long(model_file); //Vector dimension
    n_neg = DAO::read_int(model_file); //Negative samples
    we = DAO::read_float(model_file); //Initial learning rate for word vectors
    wl = DAO::read_float(model_file); //Decaying rate for word vectors

    c_vecs.resize(dim, nw);
    Matrix_Aux::read_matrix(model_file, c_vecs);
    t_vecs.resize(nw, dim);
    Matrix_Aux::read_matrix(model_file, t_vecs);
    model_file.close();

    role_mats.push_back(MatrixXf::Identity(dim, dim));
    inv_mats.push_back(MatrixXf::Identity(dim, dim));

    c_vecs.colwise().normalize();
    t_vecs.rowwise().normalize();
}
