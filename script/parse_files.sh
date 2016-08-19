#!/bin/sh -e

FILENAME_LIST=$1
shift
WORKING_DIR=$1
shift
PARA=$1
shift

if [ -d $WORKING_DIR ]; then rm -rf $WORKING_DIR; fi
mkdir $WORKING_DIR
touch $WORKING_DIR/lock_file
mkfifo $WORKING_DIR/pipe

PARSE_CP="stanford_parser_run/classes:external/stanford-parser-full-2015-12-09/*"

java -cp $PARSE_CP Dispatcher $FILENAME_LIST $PARA $WORKING_DIR/pipe $WORKING_DIR/lock_file $WORKING_DIR/parse- edu/stanford/nlp/models/lexparser/englishFactored.ser.gz java -XX:+UseSerialGC -Xmx4g -Xms4g -XX:NewSize=3g -XX:MaxNewSize=3g -XX:SurvivorRatio=4 -cp $PARSE_CP ParserFailSafe
