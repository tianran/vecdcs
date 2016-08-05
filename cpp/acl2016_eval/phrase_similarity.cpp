#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "Eigen/Core"
#include "Spearmans_Rho.h"
#include "VecDCS_Model.h"

using namespace std;
using namespace Eigen;

int main(int argc, char** argv) {

    if (argc == 1) {
        cout << "Calculate similarity and output Spearman's rho." << endl;
        cout << "Usage: phrase_similarity DATA VOCAB ROLE MODEL OUT" << endl;
        cout << endl;
        cout << "Arguments:" << endl;
        cout << "\tDATA\tdata to calculate compositional similarity" << endl;
        cout << "\tVOCAB\tword list" << endl;
        cout << "\tROLE\trole list" << endl;
        cout << "\tMODEL\tmodel file" << endl;
        cout << "\tOUT\toutput file" << endl;
        cout << "Example:" << endl;
        cout << "./phrase_similarity mitchell10-VO.txt vocab.txt roles.txt model-001 mitchell10-VO-out.txt" << endl;
        cout << endl;
        return 0;
    }

    string data_fn(argv[1]);
    string vocab_fn(argv[2]);
    string role_fn(argv[3]);
    string model_fn(argv[4]);
    string out_fn(argv[5]);

    VecDCS_Model vm(vocab_fn, role_fn, model_fn);
    auto comp_vec = [&](const string& s, RowVectorXf& vec) {
        vec = RowVectorXf::Zero(vm.dimension());
        istringstream ss(s);

        string word;
        ss >> word;
        replace(word.begin(), word.end(), '~', ' ');
        RowVectorXf one_vec;
        vm.set_query_vec(word, one_vec);

        string rstr;
        while(ss >> rstr) {
            if (rstr == "+") {
                vec += one_vec;
                string nword;
                ss >> nword;
                replace(nword.begin(), nword.end(), '~', ' ');
                vm.set_query_vec(nword, one_vec);
            } else {
                replace(rstr.begin(), rstr.end(), '~', ' ');
                string rstr2;
                ss >> rstr2;
                replace(rstr2.begin(), rstr2.end(), '~', ' ');
                vm.trans_query_vec(one_vec, rstr, rstr2, one_vec);
            }
        }
        vec += one_vec;
        vec.normalize();
    };

    vector<float> gold;
    vector<float> comp;
    ifstream data_file(data_fn);
    ofstream out_file(out_fn);

    string str;
    while(getline(data_file, str, '\t')) {
        float gscore = stof(str);
        getline(data_file, str, '\t');
        RowVectorXf vec1;
        comp_vec(str, vec1);
        getline(data_file, str);
        RowVectorXf vec2;
        comp_vec(str, vec2);
        float cscore = vec1.dot(vec2);
        gold.push_back(gscore);
        comp.push_back(cscore);
        out_file << cscore << endl;
    }

    data_file.close();
    out_file.close();
    cout << spearmans_rho(gold, comp) << endl;

    return 0;
}