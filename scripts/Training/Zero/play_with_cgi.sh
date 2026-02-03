#!/bin/bash

if [ "$REMOTE_ADDR" != "140.113.167.37" ]\
    && [ "$REMOTE_ADDR" != "140.113.167.47" ]\
    && [ "$REMOTE_ADDR" != "140.113.167.52" ]\
    && [ "$REMOTE_ADDR" != "140.113.167.57" ]\
    && [ "$REMOTE_ADDR" != "140.113.125.41" ]\
    && [ "$REMOTE_ADDR" != "123.195.24.26" ]; then
    echo "[Error]"
    exit
fi

DIR=$1
if [[ -z $DIR ]]; then
    DIR=$(ls -1td 2017* | head -n1)
fi

read SIMULATION
BEST_MODEL=$(grep "*" $DIR/Training.log | tail -n1 | awk '{ print $3; }')
./CGI_play -conf_str "USE_TIME_SEED=true:BOARD_SIZE=9:KOMI=7.5:NUM_THREAD=2:SIM_CONTROL=COUNT:SIM_COUNT_LIMIT=$SIMULATION:RESIGN_WINRATE_THRESHOLD=0.05:RESIGN_COUNT_THRESHOLD=0:EARLY_PASS=false:DCNN_NET+=NET|1|G51|Zero_B6.prototxt|../../"$DIR"/model/iter_"$BEST_MODEL".caffemodel|00:DISPLAY_MESSAGE=false" && kill $(pgrep -P $$)
