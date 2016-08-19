#!/usr/bin/env bash
#
# Runs the English PCFG parser, output readily to be converted to DCS trees

if [ ! $# -ge 1 ]; then
  echo Usage: `basename $0` 'file(s)'
  echo
  exit
fi

java -mx1g -cp "external/stanford-parser-full-2015-12-09/*" edu.stanford.nlp.parser.lexparser.LexicalizedParser \
 -outputFormat "wordsAndTags,typedDependencies" -outputFormatOptions "stem,basicDependencies" edu/stanford/nlp/models/lexparser/englishPCFG.ser.gz $*
