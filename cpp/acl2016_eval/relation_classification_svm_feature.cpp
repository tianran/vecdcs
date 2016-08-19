#include <iostream>
#include <fstream>
#include <limits>
#include "Eigen/Core"
#include "VecDCS_Model.h"
#include "Simple_VecDCS_Node.h"
#include "DCS_Tree_Reader.h"

using namespace std;
using namespace Eigen;

int main(int argc, char* argv[]) {

    if (argc == 1) {
        cout << "Calculate svm feature for relation classification." << endl;
        cout << "Usage: relation_classification_svm_feature NUM DATA VOCAB ROLE MODEL > OUT" << endl;
        cout << endl;
        cout << "Arguments:" << endl;
        cout << "\tNUM\tnumber of data points" << endl;
        cout << "\tDATA\tdata to calculate compositional similarity" << endl;
        cout << "\tVOCAB\tword list" << endl;
        cout << "\tROLE\trole list" << endl;
        cout << "\tMODEL\tmodel file" << endl;
        cout << "Example:" << endl;
        cout << "./relation_classification_svm_feature 16000 semeval10t8_train.txt vocab.txt roles.txt model-001 > semeval10t8_train_svm_feature.txt" << endl;
        cout << endl;
        return 0;
    }

    const long data_num = stol(argv[1]);
    const string data_fn(argv[2]);
    const string vocab_fn(argv[3]);
    const string role_fn(argv[4]);
    const string comp_model_fn(argv[5]);

    VecDCS_Model vm(vocab_fn, role_fn, comp_model_fn);

    RowVectorXi gold(data_num);

    MatrixXf feature_a(vm.dimension(), data_num);
    MatrixXf feature_b(vm.dimension(), data_num);
    MatrixXf feature_c(vm.dimension(), data_num);
    MatrixXf feature_d(vm.dimension(), data_num);

    ifstream data_file(data_fn);
    for (long i = 0; i != data_num; ++i) {
        string line;
        getline(data_file, line);
        gold(i) = stoi(line);
        int ln;

        getline(data_file, line);
        ln = stoi(line);
        auto a = read_dcs_tree<Simple_VecDCS_Node>(data_file, ln);
        a.front().calc_sub_tree_vec(vm);
        RowVectorXf a_vec = a.front().get_sub_tree_vec().normalized();
        feature_a.col(i) = a_vec.transpose();

        getline(data_file, line);
        ln = stoi(line);
        auto b = read_dcs_tree<Simple_VecDCS_Node>(data_file, ln);
        b.front().calc_sub_tree_vec(vm);
        RowVectorXf b_vec = b.front().get_sub_tree_vec().normalized();
        feature_b.col(i) = b_vec.transpose();

        getline(data_file, line);
        ln = stoi(line);
        auto c = read_dcs_tree<Simple_VecDCS_Node>(data_file, ln);
        Simple_VecDCS_Node* c_slot = nullptr;
        for (auto& x: c) {
            if (x.word == "**_1") c_slot = &x;
            if (x.word == "**_2") x.substitute(b_vec);
        }
        assert(c_slot);
        c_slot->calc_surrounding_vec(vm);
        RowVectorXf c_vec = c_slot->get_surrounding_vec();
        if (!c_vec.isZero()) c_vec.normalize();
        feature_c.col(i) = c_vec.transpose();

        getline(data_file, line);
        ln = stoi(line);
        auto d = read_dcs_tree<Simple_VecDCS_Node>(data_file, ln);
        Simple_VecDCS_Node* d_slot = nullptr;
        for (auto& x: d) {
            if (x.word == "**_1") x.substitute(a_vec);
            if (x.word == "**_2") d_slot = &x;
        }
        assert(d_slot);
        d_slot->calc_surrounding_vec(vm);
        RowVectorXf d_vec = d_slot->get_surrounding_vec();
        if (!d_vec.isZero()) d_vec.normalize();
        feature_d.col(i) = d_vec.transpose();
    }
    data_file.close();

    cout.precision(numeric_limits<float>::digits10);
    for (long i = 0; i != data_num; ++i) {
        cout << gold(i);
        for (long j = 0; j != vm.dimension(); ++j) {
            cout << ' ' << (j + 1) << ':' << feature_a(j, i);
        }
        for (long j = 0; j != vm.dimension(); ++j) {
            cout << ' ' << (j + 1 + vm.dimension()) << ':' << feature_b(j, i);
        }
        for (long j = 0; j != vm.dimension(); ++j) {
            cout << ' ' << (j + 1 + vm.dimension() * 2) << ':' << feature_c(j, i);
        }
        for (long j = 0; j != vm.dimension(); ++j) {
            cout << ' ' << (j + 1 + vm.dimension() * 3) << ':' << feature_d(j, i);
        }
        cout << '\n';
    }
}
