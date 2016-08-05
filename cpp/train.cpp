#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>

#include "Trainer.h"

using namespace std;

int main(int argc, char* argv[]) {

    if (argc == 1) {
        cout << "Train vecDCS models." << endl;
        cout << "Usage: train VOCAB ROLE MODEL_PRE INDEX EPOCHS PARA [DIM NEG] file1 [file2 ...]" << endl;
        cout << endl;
        cout << "Arguments:" << endl;
        cout << "\tVOCAB\tList of words." << endl;
        cout << "\tROLE\tList of syntactic/semantic roles." << endl;
        cout << "\tMODEL_PRE\tPrefix of model file names." << endl;
        cout << "\tINDEX\tTrain new model if INDEX=0. Otherwise, initialize from 'MODEL_PRE-INDEX'" << endl;
        cout << "\tEPOCHS\tNumber of epochs to train." << endl;
        cout << "\tPARA\tNumber of parallel threads." << endl;
        cout << "\tDIM\tDimension." << endl;
        cout << "\tNEG\tNumber of negative samples." << endl;
        cout << endl;
        cout << "Example 1, train a new model, 1 epoch, 6 threads, 200 dim, 4 negative samples:" << endl;
        cout << "./train vocab.txt roles.txt model- 0 1 6 200 4 file1 file2" << endl;
        cout << "Example 2, starting from existing model 'model-001', train additional 2 epochs:" << endl;
        cout << "./train vocab.txt roles.txt model- 1 2 6 file1 file2" << endl;
        cout << endl;
        return 0;
    }

    const string vocab_fn(argv[1]);
    const string role_fn(argv[2]);
    const string model_fn_prefix(argv[3]);
    const int last_index = stoi(argv[4]);
    const int epochs = stoi(argv[5]);
    const int para = stoi(argv[6]);

    Trainer* trainer;
    if (last_index == 0) {
        const long dim = stol(argv[7]);
        const int n_neg = stoi(argv[8]);
        const float we = 0.1f; //stof(argv[9]);
        const float wl = 1e-4f; //stof(argv[10]);
        const float re = 0.0005f; //stof(argv[11]);
        const float rl = 1e-5f; //stof(argv[12]);
        const float rk = 1e-4f; //stof(argv[13]);
        const float rg = 1e-3f; //stof(argv[14]);

        //vector<string> fns(argv + 15, argv + argc);
        vector<string> fns(argv + 9, argv + argc);

        trainer = new Trainer(fns, vocab_fn, role_fn, dim, n_neg, we, wl, re, rl, rk, rg);
        if (epochs == 0) {
            trainer->save_model(model_fn_prefix + "001");
            return 0;
        }
    } else {
        vector<string> fns(argv + 7, argv + argc);

        ostringstream formatter;
        formatter << model_fn_prefix << setfill('0') << setw(3) << last_index;
        trainer = new Trainer(fns, vocab_fn, role_fn, ifstream(formatter.str()));
    }

    for (int i = 1; i <= epochs; ++i) {
        ostringstream formatter;
        formatter << model_fn_prefix << setfill('0') << setw(3) << last_index + i;
        trainer->train(1, para, formatter.str());
        cout << "trained: " << i << endl;
    }
    delete trainer;

    return 0;
}
