#include <iostream>
#include <fstream>
#include "Eigen/Core"
#include "VecDCS_Model.h"
#include "VecDCS_NoRole_Model.h"
#include "Simple_VecDCS_Node.h"
#include "DCS_Tree_Reader.h"

using namespace std;
using namespace Eigen;

int main(int argc, char* argv[]) {

    if (argc == 1) {
        cout << "Test on sentence completion task." << endl;
        cout << "Usage: sentence_completion DATA VOCAB (norole|ROLE) MODEL OUT" << endl;
        cout << endl;
        cout << "Arguments:" << endl;
        cout << "\tDATA\tdata file" << endl;
        cout << "\tVOCAB\tword list" << endl;
        cout << "\tnorole\tif set, use a no-role model" << endl;
        cout << "\tROLE\trole list" << endl;
        cout << "\tMODEL\tmodel file" << endl;
        cout << "\tOUT\toutput file" << endl;
        cout << "Example 1:" << endl;
        cout << "./sentence_completion MSRSentComp.txt vocab.txt roles.txt model-001 MSRSentComp-out.txt" << endl;
        cout << "Example 2:" << endl;
        cout << "./sentence_completion MSRSentComp.txt vocab.txt norole norole-model-001 MSRSentComp-out.txt" << endl;
        cout << endl;
        return 0;
    }

    const string data_fn(argv[1]);
    const string vocab_fn(argv[2]);
    const string role_fn(argv[3]);
    const string comp_model_fn(argv[4]);
    const string answer_fn(argv[5]);

    VecDCS_Model* vm;
    if (role_fn == "norole") {
        vm = new VecDCS_NoRole_Model(vocab_fn, comp_model_fn);
    } else {
        vm = new VecDCS_Model(vocab_fn, role_fn, comp_model_fn);
    }

    ifstream data_file(data_fn);
    ifstream answer_file(answer_fn);

    int max_index = -1;
    float max_score = -2.0f;
    int counter = 0;
    int qnum = 0;
    int correctnum = 0;

    string line;
    while (getline(data_file, line)) {
        int idx = stoi(line);

        float score;
        if (idx == -1) {
            score = 0.0f;
        } else {
            getline(data_file, line);
            int ln = stoi(line);
            auto tree = read_dcs_tree<Simple_VecDCS_Node>(data_file, ln);

            VectorXf wvec;
            vm->set_answer_vec(tree[idx].word, wvec);

            tree[idx].calc_surrounding_vec(*vm);
            RowVectorXf survec = tree[idx].get_surrounding_vec();
            if (!survec.isZero()) survec.normalize();

            score = (survec * wvec)(0, 0);
        }
        cerr << score << endl;
        if (score > max_score) {
            max_score = score;
            max_index = counter;
        }
        ++counter;

        if (counter == 5) {
            string aline;
            getline(answer_file, aline);
            string ans(aline, 0, aline.find(')'));
            ++qnum;
            assert(stoi(string(ans, 0, ans.length() - 1)) == qnum);
            int ansidx = ans.at(ans.length() - 1) - 'a';
            assert(ansidx >= 0);
            assert(ansidx < 5);
            assert(max_index != -1);

            if (max_index == ansidx) ++correctnum;

            max_index = -1;
            max_score = -2.0f;
            counter = 0;
        }
    }

    delete vm;
    cout << (double)correctnum / (double)qnum << endl;
}
