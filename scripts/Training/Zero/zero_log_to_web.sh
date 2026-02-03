#!/bin/bash

now_time=$(date "+%Y/%m/%d %H:%M:%S")
echo "$REMOTE_ADDR $now_time" >> websocketd_log.log

DIR=$1
if [[ -z $DIR ]]; then
	DIR=$(ls -1td 2018* | head -n1)
fi

awk -v DIR_NAME=$DIR 'BEGIN {
	iteration = 0;
	elo_alive = 0;
	spBwinrate = "";
}{
	if($2=="[Training_Info]" || $2=="[Elo]") {
		for(i=2; i<=NF; i++) { printf $i" "; }
		print "";
		if($2=="[Elo]") {
			elo_alive++;
			elo_alive = ($4=="*")? 0: elo_alive;
		}
	} else if($2=="[Self-play_WinRate]") {
		spBwinrate = spBwinrate" "$4;
	} else if($2=="[Training_Time]") {
		time[$3] = time[$3]" "$5;
	} else if($2=="[Training_Stage]") {
		last_training_stage = $2" Iteration "iteration;
		for(i=3; i<=NF; i++) { last_training_stage = last_training_stage" "$i; }
	} else if($2=="[Iteration]") {
		iteration = $3;
	}
} END {
	print last_training_stage;
	for(key in time) { print "[Training_Time] "key time[key]; }
	if(spBwinrate!="") { print "[Self-play_WinRate]"spBwinrate; }
	print "[elo_alive] "elo_alive;
}' <(cat $DIR/Training.log)

awk -v DIR_NAME=$DIR 'BEGIN {
	iteration = 0;
}{
	if($3=="Iteration") { iteration = $4; }
	else if($5=="loss_vn") { loss[$5] = loss[$5]" "iteration" "$7; }
	else if($5=="sl_softmax_loss_1") { loss[$5] = loss[$5]" "iteration" "$7; }
} END {
	for(key in loss) {
		print "[Training_Loss] "key loss[key];
	}
}' <(grep ": loss_vn\|: sl_softmax_loss_1\|] Iteration" $DIR/TrainingLoss.log)

CURRENT_STAGE=$(tac $DIR/Training.log | grep "Training_Stage" | head -n1 | awk '{ print $3; }')
if [ $CURRENT_STAGE == "Evaluator" ]; then
	./view_progress.sh | grep "vs" | awk '{ print "[Evaluator_Progress] "$0; }'
fi

total_worker=0
total_gpu=0
total_sp=0
total_ev=0
WORKER=""
#for worker_name in $(ls $DIR/*/*.sgf | awk -F "/|_Self-play|_Evaluator" '{ name[$3]=$3; } END { for(key in name) { print name[key]; } }')
#do
#	sp_game=0
#	ev_game=0
#	if [[ $(ls $DIR/*/$worker_name*Self-play.sgf 2>/dev/null | wc -l) -ne 0 ]]; then
#		sp_game=$(tail -n1 $(ls $DIR/*/$worker_name*Self-play.sgf -t | head -n1) | grep ";FF\[4\]" | cut -d" " -f1 | awk '{ sum+=$1; } END { print sum; }')
#	fi
#	if [[ $(ls $DIR/*/$worker_name*Evaluator.sgf 2>/dev/null | wc -l) -ne 0 ]]; then
#		ev_game=$(tail -n1 $(ls $DIR/*/$worker_name*Evaluator.sgf -t | head -n1) | grep ";FF\[4\]" | cut -d" " -f2 | awk '{ sum+=$1; } END { print sum; }')
#	fi
#	date_time=$(date -d "$(stat -c%y $(ls $DIR/*/$worker_name*.sgf -t | head -n1))" +"%Y/%m/%d_%H:%M:%S")
#	nGPU=$(echo $worker_name | awk -F "_|@" '{ print length($2); }')
#	
#	let "total_worker++"
#	let "total_gpu+=$nGPU"
#	let "total_sp+=$sp_game"
#	let "total_ev+=$ev_game"
#	WORKER=$WORKER"[Worker] [Worker-Connection] $date_time $worker_name $nGPU $sp_game $ev_game 0\n"
#done
ITERATION=$(grep "Iteration" $DIR/Training.log | tail -n1 | awk '{ print $3; }')
WORKER=$(wc -l $DIR/$ITERATION/*Self-play.sgf | awk -v ITER=$ITERATION 'BEGIN {
}{
	worker_name = substr($2,index($2,ITER"/"));
	worker_name = substr(worker_name,index(worker_name,"/")+1);
	if(worker_name=="total") { next; }
	if(index(worker_name,"_Self-play")>0) {
		worker_name = substr(worker_name,0,index(worker_name,"_Self-play")-1);
		sp_game[worker_name] = $1;
	} else if(index(worker_name,"_Evaluator")>0) {
		worker_name = substr(worker_name,0,index(worker_name,"_Evaluator")-1);
		ev_game[worker_name] = $2;
	}
	nGPU[worker_name] = index(worker_name,"@") - index(worker_name,"_") - 1;
	if(nGPU[worker_name]<0) { nGPU[worker_name] = 8; } # hack code
} END {
	total_worker = 0;
	total_gpu = 0;
	total_sp = 0;
	total_ev = 0;
	for(name in nGPU) {
		if(sp_game[name]=="") { sp_game[name] = 0; }
		if(ev_game[name]=="") { ev_game[name] = 0; }
		total_worker++;
		total_gpu += nGPU[name];
		total_sp += sp_game[name];
		total_ev += ev_game[name];
		print "[Worker] [Worker-Connection] date_time "name,nGPU[name],sp_game[name],ev_game[name],"0";
	}
	print "[Worker_Info]",total_worker,total_gpu,total_sp,total_ev;
}')
echo -e "$WORKER" | grep "\[Worker_Info\]"
echo -e "$WORKER" | grep "\[Worker\]" | sort -k6 -n | tail -n15
echo -e "$WORKER" | grep "\[Worker\]" | sed 's/\[Worker\]/\[Worker_List\]/g' | sort -k4
echo "display"

BEST_MODEL=$(grep "*" $DIR/Training.log | tail -n1 | awk '{ print "iter_"$3".pb"; }')
LAST_ITERATION=$(($ITERATION-1))
SP_GAME_PATH=""
if [[ -d $DIR/$ITERATION && ! -f $DIR/$ITERATION/*Self-play.sgf && $(ls $DIR/$ITERATION/*Self-play.sgf 2>/dev/null | wc -l) -ne 0 ]]; then
	SP_GAME_PATH=$DIR/$ITERATION/*Self-play.sgf
else
	SP_GAME_PATH=$DIR/$LAST_ITERATION/*Self-play.sgf
fi
BOARD_SIZE=$(grep "Training_Info" $DIR/Training.log | awk '{ print $5; }')
tail -n10 $SP_GAME_PATH | grep "(;FF\[4\]" | shuf | head -n5 | sed 's/disable resign/disable-resign/g' | awk -v BEST_MODEL=$BEST_MODEL '{
	result_start=index($3,"RE[")+3;
	result_end=index($3,"]SZ");
	dt_start=index($3,"DT[")+3;
	dt_end=index($3,"]EV");
	print "selfplay",substr($3,result_start,result_end-result_start),$2,substr($3,dt_start,dt_end-dt_start),$3;
}' | ./parse.sh $BOARD_SIZE
EV_GAME_PATH=""
if [[ -d $DIR/$ITERATION && ! -f $DIR/$ITERATION/*Evaluator.sgf && $(ls $DIR/$ITERATION/*Evaluator.sgf 2>/dev/null | wc -l) -ne 0 ]]; then
	EV_GAME_PATH=$DIR/$ITERATION/*Evaluator.sgf
else
	EV_GAME_PATH=$DIR/$LAST_ITERATION/*Evaluator.sgf
fi
tail -n10 $EV_GAME_PATH | grep "(;FF\[4\]" | grep $BEST_MODEL | shuf | head -n10 | awk '{
	result_start=index($0,"RE[")+3;
	result_end=index($0,"] C");
	dt_start=index($0,", 2018")+2;
	dt_end=index($0,"] ;B");
	tag = substr($0,dt_start,dt_end-dt_start);
	gsub(/ /,"_",tag);
	print "evaluator",substr($0,result_start,result_end-result_start),gsub(/;/,";",$0)-1,tag,$0;
}'
echo "end"
