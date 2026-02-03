#!/bin/zsh

# This bash will summarize the json files of Tsumego Solver

#if [ -z "$1" ]; then
#  echo "./solverResultParser.sh Directory"
#  exit 0
#fi

DIR=$1

# $2=ProblemSgfFilename
# $4=NumSimulations
# $5=Time
# $8=BestMove
# $9=BestMoveWinrate
# $13=WinStatus
for f in $DIR/*.json; do awk -F ",| " '{print $2}' $f | awk 'NR==2 || NR==4 || NR==5 || NR==8 || NR==9 || NR==13' | awk '{for(i=1;i<=NF;i++) printf "%s\t",$i }' ;echo ""; done >! $DIR/result.txt
sed -i "s/\"//g" $DIR/result.txt

# Remove the last tab
awk '{for(i=1;i<=NF;i++) { printf "%s",$i; if(i%6==0) {printf "\n";} else { printf "\t"} } }' $DIR/result.txt >! $DIR/result_refined.txt

# NR=2~20 include all the information in .tt file.
for f in $DIR/*.tt; do awk 'NR>=2 && NR<=20' $f | awk -F ",| " '{print $2}' | awk '{for(i=1;i<=NF;i++) printf "%s\t",$i }' ;echo ""; done >! $DIR/tt_result.txt

sed -i ':a;N;$!ba;s/\t\n/\n/g' $DIR/tt_result.txt

# Remove the last tab
awk '{for(i=1;i<=NF;i++) { printf "%s",$i; if(i%19==0) {printf "\n";} else { printf "\t"} } }' $DIR/tt_result.txt >! $DIR/tt_result_refined.txt

paste $DIR/result_refined.txt $DIR/tt_result_refined.txt >! $DIR/result_all.txt

echo "[Basic Result Done]"

# Parse TT summary
TT_OUTPUT_DIR=$2
if [[ $TT_OUTPUT_DIR = '' ]]
then
  TT_OUTPUT_DIR="tt_summary"
fi

OUTPUT_DIR=$DIR/$TT_OUTPUT_DIR
echo "[Parse TT Summary] to "$OUTPUT_DIR

if [ -d $OUTPUT_DIR ]
then
	echo $OUTPUT_DIR" exist!" 
else
	echo "create "$OUTPUT_DIR
	mkdir $OUTPUT_DIR
fi	

./parseSummary_both.sh $DIR $OUTPUT_DIR

# trav.txt store $8=nCompare, $12=nLeftIDSkipEdge, $13=nRightIDSkipEdge, $16=nLeftCompareEdge, $17=nRightCompareEdge
for f in $OUTPUT_DIR/*.both; do awk '{print $8"\t"$12"\t"$13"\t"$16"\t"$17}' $f; done >! $DIR/trav.txt

paste $DIR/result_all.txt $DIR/trav.txt >! $DIR/result_summary.txt

sort -n -k 1 $DIR/result_summary.txt >! $DIR/result_summary_reorder.txt

echo "Final Result in "$DIR/result_summary_reorder.txt
