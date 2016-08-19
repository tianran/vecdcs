#include <iostream>
#include <unordered_map>
#include <vector>
#include <utility>
#include "Eigen/Core"

#include "VecDCS_Model.h"
#include "VecDCS_NoRole_Model.h"

using namespace std;
using namespace Eigen;

void display_list(const vector<pair<float, string>>& list) {
    for (auto x : list) {
        cout << fixed << x.first << '\t' << x.second << endl;
    }
}

int main(int argc, char** argv) {

    if (argc == 1) {
        cout << "Show information of a trained model." << endl;
        cout << "Usage: see_model VOCAB (norole|ROLE) MODEL" << endl;
        cout << endl;
        cout << "Arguments:" << endl;
        cout << "\tVOCAB\tList of words." << endl;
        cout << "\tnorole\tif set, use a no-role model" << endl;
        cout << "\tROLE\tList of syntactic/semantic roles." << endl;
        cout << "\tMODEL\tModel file name." << endl;
        cout << "Example 1:" << endl;
        cout << "./see_model vocab.txt roles.txt model-001" << endl;
        cout << "Example 2:" << endl;
        cout << "./see_model vocab.txt norole norole-model-001" << endl;
        cout << endl;
        return 0;
    }

    const string vocab_fn(argv[1]);
    const string role_fn(argv[2]);
    const string model_fn(argv[3]);

    VecDCS_Model* vm;
    if (role_fn == "norole") {
        vm = new VecDCS_NoRole_Model(vocab_fn, model_fn);
    } else {
        vm = new VecDCS_Model(vocab_fn, role_fn, model_fn);
    }

    int num = 20;
    while (true) {

        cout << endl;
        cout << "Ready. Enter a command ('H' for help):" << endl;

        string command;
        cin >> command;
        if (command == "H") {
            cout << "H: help" << endl;
            cout << "X: exit" << endl;
            cout << "N <num>: set k in kNN (default 20)" << endl;
            cout << "M <word> <role>...: show info of <word> transformed by <role>s" << endl;
            cout << "M <word> <role>... +/- <word> <role>... +/-...: additive composition" << endl;
        } else if (command == "X") {
            break;
        } else if (command == "N") {
            cin >> num;
        } else if (command == "M") {
            RowVectorXf vec = RowVectorXf::Zero(vm->dimension());
            RowVectorXf one_vec;
            bool plus = true;

            string word;
            cin >> word;
            replace(word.begin(), word.end(), '~', ' ');
            vm->set_query_vec(word, one_vec);
            do {
                while (cin.peek() == ' ') cin.get();
                if (cin.peek() == '\n') {
                    if (plus) vec += one_vec; else vec -= one_vec;
                    break;
                }
                string str;
                cin >> str;
                if (str == "+") {
                    if (plus) vec += one_vec; else vec -= one_vec;
                    plus = true;
                    string nword;
                    cin >> nword;
                    replace(nword.begin(), nword.end(), '~', ' ');
                    vm->set_query_vec(nword, one_vec);
                    continue;
                } else if (str == "-") {
                    if (plus) vec += one_vec; else vec -= one_vec;
                    plus = false;
                    string nword;
                    cin >> nword;
                    replace(nword.begin(), nword.end(), '~', ' ');
                    vm->set_query_vec(nword, one_vec);
                    continue;
                } else {
                    replace(str.begin(), str.end(), '~', ' ');
                    string str2;
                    cin >> str2;
                    replace(str2.begin(), str2.end(), '~', ' ');
                    vm->trans_query_vec(one_vec, str, str2, one_vec);
                }
            } while (true);

            cout << "Similar target: " << endl;
            display_list(vm->top_similar(vec, num));

            cout << "Strong context: " << endl;
            display_list(vm->top_answers(vec, num));
        } else {
            cout << "ERROR: unknown command" << endl;
        }

        cin.ignore (numeric_limits<streamsize>::max(), '\n');
    }

    delete vm;
    return 0;
}

