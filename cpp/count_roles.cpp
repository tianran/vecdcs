#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <limits>

#include "Simple_DCS_Node.h"
#include "DCS_Tree_Reader.h"

using namespace std;

int main(int argc, char** argv) {

    if (argc == 1) {
        cout << "Count weighted roles in dcs trees." << endl;
        cout << "Usage: count_roles FILE" << endl;
        cout << endl;
        cout << "Arguments:" << endl;
        cout << "\tFile\tdcs file name." << endl;
        cout << "Example:" << endl;
        cout << "./count_roles sample.dcs > roles.txt" << endl;
        cout << endl;
        return 0;
    }

    const string fn(argv[1]);

    unordered_map<string, double> result;

    string line;
    ifstream file(fn);
    while (getline(file, line)) {
        auto tree = read_dcs_tree<Simple_DCS_Node>(file, stoi(line));
        for (auto x = ++tree.begin(); x != tree.end(); ++x) {
            double tmp = x->role_weight();
            result[x->oLabel] += tmp;
            result[x->uLabel] += tmp;
        }
    }

    cout.precision(numeric_limits<double>::digits10);
    for (auto& x: result) {
        cout << x.second << '\t' << x.first << endl;
    }
}

