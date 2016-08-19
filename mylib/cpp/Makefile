OBJECTS=\
	Random_Generator.o\
	Weight_Sampler.o\
	Vocab_Aux.o\
	DAO.o\
	Matrix_Aux.o\


CC=g++
CFLAGS=-Ofast -std=c++11 -Wall

EIGEN_DIR=../external/Eigen-3.2.9

all: $(OBJECTS)

%.o: %.cpp %.h
	$(CC) -c $(CFLAGS) -I $(EIGEN_DIR) $<

clean:
	rm -f $(OBJECTS)

.PHONY: clean
