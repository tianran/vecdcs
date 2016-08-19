#ifndef __VOCAB_AUX_H
#define __VOCAB_AUX_H

#include <unordered_map>
#include <string>
#include <fstream>
#include <vector>

#include "Weight_Sampler.h"

namespace Vocab_Aux {

    long get_index_unk_type(const std::unordered_map<std::string, long>& word_map, const std::string& word);
    long get_index_unk(const std::unordered_map<std::string, long>& word_map, const std::string& word);
    long get_index(const std::unordered_map<std::string, long>& word_map, const std::string& word);

    long init_vocab(const std::string& fn, std::unordered_map<std::string, long>& map, Weight_Sampler& ws);
    long init_vocab(const std::string& fn, std::unordered_map<std::string, long>& map, std::vector<std::string>& vs);
    long init_vocab(const std::string& fn, std::unordered_map<std::string, long>& map);
}

#endif //__VOCAB_AUX_H
