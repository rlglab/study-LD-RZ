#!/bin/zsh

SOURCE_DIR=$1
DEST_DIR=$2

for f in $SOURCE_DIR/*.ttsummary
do               
  print $f
  FILENAME=${f##*/}
  FILE_TAG=$(echo $FILENAME | cut -d . -f 1)
  awk -f analyzeSummary_both.awk $SOURCE_DIR/$FILE_TAG.ttsummary >! $DEST_DIR/$FILE_TAG.both
  awk -f countnodes.awk $SOURCE_DIR/$FILE_TAG.nodes >! $DEST_DIR/$FILE_TAG.ratio
done  

echo "Check output in "$DEST_DIR
