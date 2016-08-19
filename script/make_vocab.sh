#!/bin/sh -e

MYLIB_CP=$HOME/mylib/scala/classes
BIN_PATH=cpp

WORDS_CUTOFF=$1
shift
ROLES_CUTOFF=$1
shift

ALL_WORDS=""
ALL_ROLES=""
for fn in $*
do
 echo $fn
 $BIN_PATH/count_words $fn > $fn.words
 $BIN_PATH/count_roles $fn > $fn.roles
 scala -cp $MYLIB_CP mylib.count_util.SortKey $fn.words > $fn.words.sortkey
 ALL_WORDS=$ALL_WORDS" "$fn.words.sortkey
 scala -cp $MYLIB_CP mylib.count_util.SortKey $fn.roles > $fn.roles.sortkey
 ALL_ROLES=$ALL_ROLES" "$fn.roles.sortkey
done

scala -cp $MYLIB_CP mylib.count_util.MergeCount $ALL_WORDS > words.merge
scala -cp $MYLIB_CP mylib.count_util.MergeCount $ALL_ROLES > roles.merge

scala -cp $MYLIB_CP mylib.count_util.CutOffWordsWithPOS words.merge $WORDS_CUTOFF > words.cutoff
scala -cp $MYLIB_CP mylib.count_util.CutOff roles.merge $ROLES_CUTOFF > roles.cutoff

scala -cp $MYLIB_CP mylib.count_util.SortCount words.cutoff > words.sort
scala -cp $MYLIB_CP mylib.count_util.SortCount roles.cutoff > roles.sort
