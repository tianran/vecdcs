OBJECTS=\
	Simple_DCS_Node.o\
	Trainer_Aux.o\
	Trainer.o\
	VecDCS_Model.o\
	VecDCS_NoRole_Model.o\
	Simple_VecDCS_Node.o\


TARGETS=\
	count_words\
	count_roles\
	train\
	see_model\


EVAL_OBJECTS=\


EVAL_TARGETS=\
	acl2016_eval/phrase_similarity\
	acl2016_eval/relation_classification_svm_feature\
	acl2016_eval/sentence_completion\
	

CC=g++
CFLAGS=-Ofast -std=c++11 -Wall
LFLAGS=-pthread

EIGEN_DIR=~/external/Eigen-3.2.9

MYLIB_DIR=~/mylib/cpp
MYLIB=\
	$(MYLIB_DIR)/Vocab_Aux.o\
	$(MYLIB_DIR)/DAO.o\
	$(MYLIB_DIR)/Matrix_Aux.o\
	$(MYLIB_DIR)/Random_Generator.o\
	$(MYLIB_DIR)/Weight_Sampler.o\


all: $(MYLIB) $(OBJECTS) $(TARGETS) $(EVAL_OBJECTS) $(EVAL_TARGETS)

$(MYLIB):
	cd $(MYLIB_DIR) && $(MAKE)

acl2016_eval/%.o: acl2016_eval/%.cpp acl2016_eval/%.h
	$(CC) -c $(CFLAGS) -I $(EIGEN_DIR) -I $(MYLIB_DIR) -I . $< -o $@

acl2016_eval/%: acl2016_eval/%.cpp $(MYLIB) $(OBJECTS) $(EVAL_OBJECTS)
	$(CC) $(CFLAGS) -I $(EIGEN_DIR) -I $(MYLIB_DIR) -I . $^ $(LFLAGS) -o $@

%.o: %.cpp %.h
	$(CC) -c $(CFLAGS) -I $(EIGEN_DIR) -I $(MYLIB_DIR) $< -o $@

%: %.cpp $(MYLIB) $(OBJECTS)
	$(CC) $(CFLAGS) -I $(EIGEN_DIR) -I $(MYLIB_DIR) $^ $(LFLAGS) -o $@

clean:
	rm -f $(OBJECTS) $(TARGETS) $(EVAL_OBJECTS) $(EVAL_TARGETS)

.PHONY: clean
