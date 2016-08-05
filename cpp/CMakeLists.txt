cmake_minimum_required(VERSION 3.3)
project(vecdcs_cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(~/external/Eigen-3.2.8)

set(MYLIB_DIR ~/mylib/cpp)

include_directories(${MYLIB_DIR})
link_directories(${MYLIB_DIR})

set(MYLIB
        ${MYLIB_DIR}/Vocab_Aux.o
        ${MYLIB_DIR}/DAO.o
        ${MYLIB_DIR}/Matrix_Aux.o
        ${MYLIB_DIR}/Random_Generator.o
        ${MYLIB_DIR}/Weight_Sampler.o
        )

add_executable(count_words count_words.cpp ${MYLIB} Simple_DCS_Node.cpp)
add_executable(count_roles count_roles.cpp ${MYLIB} Simple_DCS_Node.cpp)
add_executable(train train.cpp ${MYLIB} Simple_DCS_Node.cpp Trainer_Aux.cpp Trainer.cpp)

add_executable(see_model see_model.cpp ${MYLIB} VecDCS_Model.cpp VecDCS_NoRole_Model.cpp)

include_directories(.)

add_executable(phrase_similarity acl2016_eval/phrase_similarity.cpp
        ${MYLIB} VecDCS_Model.cpp VecDCS_NoRole_Model.cpp)
add_executable(relation_classification_svm_feature acl2016_eval/relation_classification_svm_feature.cpp
        ${MYLIB} Simple_DCS_Node.cpp VecDCS_Model.cpp Simple_VecDCS_Node.cpp)
add_executable(sentence_completion acl2016_eval/sentence_completion.cpp
        ${MYLIB} Simple_DCS_Node.cpp VecDCS_Model.cpp
        Simple_VecDCS_Node.cpp VecDCS_NoRole_Model.cpp)
