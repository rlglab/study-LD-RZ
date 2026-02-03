#!/bin/bash

# ======================= Setting =====================================
LOG_MODE_NUMBER=8
TEST_SGF_PATH="/home/cgilab/GO/public/kgs6.5"
SAMPLE_PER_SGF=3

FEATURE_TYPE="G51"
TEST_MODEL_PATH="analysis"
TEST_OUTPUT_PATH="analysis_result"
NUM_THREAD=24
GPU_LIST=0123
# =====================================================================

CONFIGURE="NUM_THREAD=${NUM_THREAD}:DCNN_TRAIN_GPU_LIST=${GPU_LIST}:USE_TIME_SEED=true:ENABLE_DCNN_MESSAGE=flase"
lossFileName="net_loss.txt"
netType="NET"

if [ ! -d "$TEST_MODEL_PATH" ]; then
    echo "[ERROR] Please put this script at the root directory of CGI program"
    echo "[ERROR] Please make sure \"database/dcnn/${TEST_MODEL_PATH}/\" exist"
    exit 1
fi

# prepare train mode input
input_filename=.$(< /dev/urandom tr -dc _A-Za-z0-9 | head -c 8).txt
echo "$LOG_MODE_NUMBER" > $input_filename
echo "$TEST_SGF_PATH" >> $input_filename
echo "$SAMPLE_PER_SGF" >> $input_filename

echo "[INFO] log_mode_number: ${LOG_MODE_NUMBER}"
echo "[INFO] testing_sgf_path: ${TEST_SGF_PATH}"
echo "[INFO] sample_per_sgf: ${SAMPLE_PER_SGF}"

ARR=(${TEST_SGF_PATH//// })
TEST_DIRECTORY=${ARR[${#ARR[@]}-1]}

for directory in $TEST_MODEL_PATH/*
do
	prototxt_filename=$(ls $directory/*.prototxt)
	for snapshot in $directory/*.caffemodel
	do
		snapshot="${snapshot##*/}"
		log_filename="${snapshot%caffemodel}log"
		log_filename=${TEST_DIRECTORY}"_"${netType}"_loss_${log_filename}"
		if [ -f "${TEST_OUTPUT_PATH}/${log_filename}" ]; then
			echo "skip ${TEST_OUTPUT_PATH}/${log_filename}"
			continue
		fi
		CONF_STR="${CONFIGURE}:DCNN_NET+=${netType}|1|${FEATURE_TYPE}|../../${prototxt_filename}|../../${directory}/${snapshot}|0"
		COMMAND="Release/CGI -conf_str \"${CONF_STR}\" -mode train < $input_filename > /dev/null 2> /dev/null"
	    echo "[Runnung] ${log_filename}"
		echo "[Command] ${COMMAND}"
		Release/CGI -conf_str "${CONF_STR}" -mode train < $input_filename > /dev/null 2> /dev/null
		mv ${lossFileName} ${TEST_OUTPUT_PATH}/${log_filename}
	done
done

rm $input_filename
