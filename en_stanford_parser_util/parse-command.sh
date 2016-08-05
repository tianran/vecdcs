#!/bin/bash

DISPATCHER_CP=$1
shift
STANFORD_PARSER_CP=$1
shift
PARSER_MODEL_SER_GZ=$1
shift
WORKING_DIR=$1
shift
PROCESS_NUM=$1
shift

java -XX:+UseParallelOldGC -XX:ParallelGCThreads=12 -Xmx4g -Xms4g -XX:NewSize=3g -cp "$DISPATCHER_CP:$STANFORD_PARSER_CP" Dispatcher $WORKING_DIR/text.filelist $PROCESS_NUM $WORKING_DIR/pipe $WORKING_DIR/lock_file $WORKING_DIR/parsed/parse- $PARSER_MODEL_SER_GZ $* java -XX:+UseSerialGC -Xmx4g -Xms4g -XX:NewSize=3g -XX:MaxNewSize=3g -XX:SurvivorRatio=4 -cp "$DISPATCHER_CP:$STANFORD_PARSER_CP" ParserFailSafe > $WORKING_DIR/text_parsed.filelist
