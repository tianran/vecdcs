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
        cout << "Count weighted words in dcs trees." << endl;
        cout << "Usage: count_words FILE" << endl;
        cout << endl;
        cout << "Arguments:" << endl;
        cout << "\tFile\tdcs file name." << endl;
        cout << "Example:" << endl;
        cout << "./count_words sample.dcs > vocab.txt" << endl;
        cout << endl;
        return 0;
    }

    const string fn(argv[1]);

    unordered_map<string, double> result;

    string line;
    ifstream file(fn);
    while (getline(file, line)) {
        auto tree = read_dcs_tree<Simple_DCS_Node>(file, stoi(line));
        for (auto& x: tree) result[x.word] += x.word_weight();
    }

    cout.precision(numeric_limits<double>::digits10);
    for (auto& x: result) {
        cout << x.second << '\t' << x.first << endl;
    }
}
