# vecdcs

This is a tool for training semantically compositional word vectors and syntactic transformation matrices. Please refer to the following paper for the underlying theory:

[Learning Semantically and Additively Compositional Distributional Representations](http://aclweb.org/anthology/P/P16/P16-1121.pdf), ACL2016

## Prerequisite

The tool requires shell, make, c++, java and scala for building. It trains vectors and matrices on dependency parses of corpora. 

## Quick Start

In this demo, we compile the source, download a sample corpus, train a model on that corpus and investigate the learned model. 

### Compile the source:

Assume the working directory is the root of this git repo. 

```shell
cd external/
./get_Eigen-3.2.9.sh
cd ../cpp/
make
cd ../mylib/scala/
mkdir classes
scalac -d classes $(find . -name '*.scala')
cd ../../scala/
mkdir classes
scalac -d classes $(find . -name '*.scala')
cd ../
```

### Download a sample corpus:

```shell
cd data/
./get_enwiki-partial-parsed.sh
cd ../
```

This corpus is a small portion extracted from [English Wikipedia](https://en.wikipedia.org/wiki/Main_Page) (using the tool [Wikipedia Extractor](http://medialab.di.unipi.it/wiki/Wikipedia_Extractor)), then parsed by [Stanford Parser](http://nlp.stanford.edu/software/lex-parser.shtml) (2015-12-09 release).

### Convert the parsed corpus to DCS trees:

```shell
for fn in $(ls data/enwiki-partial-parsed/parse-??.out); do echo $fn; scala -cp scala/classes/ ud_to_dcs.en_stanford_simple.DepTree $fn > $fn.dcs; done
```
### Make vocabulary:

```shell
script/make_vocab.sh 200 1000 data/enwiki-partial-parsed/parse-??.out.dcs
```

### Train model:

As an example, we train a model on the previously obtained DCS trees. We train 6 epochs, using 10 parallel threads, and set the dimension of vectors to 200, using 4 negative samples per data point during training. This will take about one hour. 

```shell
cpp/train words.sort roles.sort model- 0 6 10 200 4 data/enwiki-partial-parsed/parse-??.out.dcs
```

The output model file will be named `model-006` (one model per epoch).

### See trained model:

```shell
cpp/see_model words.sort roles.sort model-006
```
Then, use command such as 
`
M book/NOUN
`
or
`
M learn/VERB SUBJ ARG
`
or 
`
M learn/VERB COMP ARG
`
or 
`
M learn/VERB ARG about
`
etc. to investigate the model. 

## Pre-trained models

A 250-dim model trained on the whole English Wikipedia dump, which took about one week for parsing (by a 12core * 20 cluster) and three days for training (by a 24core computer), can be downloaded as follows.
```shell
cd acl2016_eval/
./get_enwiki-model.sh
cd ../
```

A 200-dim model trained on a collection of 19-th century novels (collected from [Project Gutenberg](https://www.gutenberg.org) by [Microsoft Sentence Completion Challenge](https://www.microsoft.com/en-us/research/project/msr-sentence-completion-challenge/)). 
```shell
cd acl2016_eval/
./get_MSRSentComp-model.sh
cd ../
```

## Reproduce the results in our ACL2016 paper:

### for phrase similarity:

```shell
cpp/acl2016_eval/phrase_similarity acl2016_eval/phrase_similarity/mitchell10-AN.txt acl2016_eval/enwiki-model/words.cut1000.sort acl2016_eval/enwiki-model/roles.cut10000.sort acl2016_eval/enwiki-model/model-dim250-001 /dev/null
```

### for relation classification:

The following will convert the training and testing data into features used by the [libsvm](https://www.csie.ntu.edu.tw/~cjlin/libsvm/) toolkit. 
```shell
cpp/acl2016_eval/relation_classification_svm_feature 16000 acl2016_eval/relation_classification/semeval10t8_train_converted.txt acl2016_eval/enwiki-model/words.cut1000.sort acl2016_eval/enwiki-model/roles.cut10000.sort acl2016_eval/enwiki-model/model-dim250-001 > semeval10t8_train.svm
cpp/acl2016_eval/relation_classification_svm_feature 2717 acl2016_eval/relation_classification/semeval10t8_test_converted.txt acl2016_eval/enwiki-model/words.cut1000.sort acl2016_eval/enwiki-model/roles.cut10000.sort acl2016_eval/enwiki-model/model-dim250-001 > semeval10t8_test.svm
```

Then, an SVM model can be trained and tested using the tool. 

```shell
svm-train -g 0.25 -c 2.0 semeval10t8_train.svm semeval10t8.model
svm-predict semeval10t8_test.svm semeval10t8.model semeval10t8.out
```

### for sentence completion:

```shell
cpp/acl2016_eval/sentence_completion acl2016_eval/sentence_completion/MSRSentComp-converted.txt acl2016_eval/sentence_completion/MSRSentComp-answers.txt acl2016_eval/MSRSentComp-model/words.cut50.sort acl2016_eval/MSRSentComp-model/roles.cut1000.sort acl2016_eval/MSRSentComp-model/model-dim200-018 /dev/null
cpp/acl2016_eval/sentence_completion acl2016_eval/sentence_completion/MSRSentComp-converted.txt acl2016_eval/sentence_completion/MSRSentComp-answers.txt acl2016_eval/MSRSentComp-model/words.cut50.sort norole acl2016_eval/MSRSentComp-model/model-dim200-norole-034 /dev/null
```

## Parse your own corpus:

You can use [Stanford Parser](http://nlp.stanford.edu/software/lex-parser.shtml) with option `-outputFormat "wordsAndTags,typedDependencies" -outputFormatOptions "stem,basicDependencies"` to parse your corpus. 

The following is an example to download the parser and then parse a simple sentence:
```shell
cd external/
./get_Stanford-Parser-2015-12-09.sh
cd ../
echo "I am happy." | script/lexparser.sh -
```

We also provide a tool to parse a large corpus by running several Stanford parsers in parallel JVMs. To compile:
```shell
cd stanford_parser_run/
mkdir classes
javac -cp "../external/stanford-parser-full-2015-12-09/*" -d classes $(find . -name '*.java')
cd ../
```

To parse, write the paths to all files you want to parse into a single file, say `fnlist.txt`, one path per line. Then, run the following command: 
```shell
script/parse_files.sh fnlist.txt output_dir 10
```
It will parse all files listed in `fnlist.txt` using 10 parallel parsers. The result will be output into `output_dir`. (WARNING: if `output_dir` already exists, this command will erase the existing contents.)
