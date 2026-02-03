#!/bin/bash

if  [ "$#"  -ne 6  ] && [  "$#" -ne 7 ]; then
	echo "$0 <Snapshot_dir> <model_prefix> <prototxt_dir> <sgf_dir> <model_begin> <model_end> <step=50000>"
	exit 1
fi

step=50000
if  [ ${#1} -eq 7  ]; then
	step  = $7;
fi

modelDir=$1
prefix=$2
targetDir=$3
sgfSet=$4
begin=$5
end=$6

rm SL_prediction_rate.txt

# copy all the target models to the targetDir set by test_model.sh

for (( i=$begin ; i<=$end ; i=i+50000 )) { cp ${modelDir}/${prefix}${i}.caffemodel ${targetDir};   }
bash test_model.sh

# run summarize_SL.sh for all logfile outputed by previous step
prefixPlus=${sgfSet}_SL_loss_${prefix}
for ((i=$begin; i<=$end; i=i+50000)) {  bash summarize_SL.sh  ${prefixPlus}${i}.log > pr_${i}.log;  }

# output Result from summarize_SL.sh
for ((i=$begin; i<=$end; i=i+50000)) {  tail -n1 pr_${i}.log >> SL_prediction_rate.txt;   }




