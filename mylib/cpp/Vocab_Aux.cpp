#include "Vocab_Aux.h"

using namespace std;

namespace Vocab_Aux {

    long get_index_unk_type(const unordered_map<string, long>& word_map, const string& word) {
        auto get = word_map.find(word);
        if (get != word_map.end()) {
            return get->second;
        } else {
            return word_map.at("*UNKNOWN*" + string(word, word.find_last_of('/')));
        }
    }

    long get_index_unk(const unordered_map<string, long>& word_map, const string& word) {
        auto get = word_map.find(word);
        if (get != word_map.end()) {
            return get->second;
        } else {
            return word_map.at("*UNKNOWN*");
        }
    }

    long get_index(const unordered_map<string, long>& word_map, const string& word) {
        auto get = word_map.find(word);
        if (get != word_map.end()) {
            return get->second;
        } else {
            return -1;
        }
    }

    long init_vocab(const string& fn, unordered_map<string, long>& map, Weight_Sampler& ws) {
        ifstream file(fn);
        vector<double> weights;
        long idx = 0;
        string line;
        while(getline(file, line)) {
            auto i = line.find('\t');
            map[string(line, i + 1)] = idx;
            weights.push_back(stod(string(line, 0, i)));
            ++idx;
        }
        ws.init(weights);
        return idx;
    }

    long init_vocab(const string& fn, unordered_map<string, long>& map, vector<string>& vs) {
        ifstream file(fn);
        long idx = 0;
        string line;
        while(getline(file, line)) {
            string str(line, line.find('\t') + 1);
            map[str] = idx;
            vs.push_back(str);
            ++idx;
        }
        return idx;
    }

    long init_vocab(const string& fn, unordered_map<string, long>& map) {
        ifstream file(fn);
        long idx = 0;
        string line;
        while(getline(file, line)) {
            string str(line, line.find('\t') + 1);
            map[str] = idx;
            ++idx;
        }
        return idx;
    }
}