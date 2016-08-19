#ifndef VECDCS_NOROLE_MODEL_H
#define VECDCS_NOROLE_MODEL_H

#include "VecDCS_Model.h"

class VecDCS_NoRole_Model: public VecDCS_Model {

public:
    VecDCS_NoRole_Model(const std::string& vocab_fn, const std::string& model_fn);
};

#endif //VECDCS_NOROLE_MODEL_H
