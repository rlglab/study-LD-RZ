#!/bin/bash

ITERATION=$1
DIR=$2
if [[ -z $DIR ]]; then
    DIR=$(ls -1td 2018* | head -n1)
fi
if [[ -z $ITERATION ]]; then
	ITERATION=$(grep "Iteration" $DIR/Training.log | tail -n1 | awk '{ print $3; }')
fi

echo "======= Iteration: $ITERATION, directory: $DIR ======="
echo "[Self-play progress]"
wc -l $DIR/$ITERATION/*Self-play.sgf | awk -v ITER=$ITERATION 'BEGIN {
}{
	machine_name = substr($2,index($2,ITER"/"));
	machine_name = substr(machine_name,index(machine_name,"/")+1);
	machine_name = substr(machine_name,0,index(machine_name,"@")-1);
	nGPU = length(machine_name)-index(machine_name,"_");
	if(machine_name!="") {
		if(index(machine_name,"caffe")>0 || index(machine_name,"CGI")>0) { printf("\033[32m%20s%4d\033[0m",machine_name,$1); num["caffe"]+=$1; gpu_num["caffe"]+=nGPU; }
		else if(index(machine_name,"cn")>0) { printf("\033[33m%20s%4d\033[0m",machine_name,$1); num["cn"]+=$1; gpu_num["cn"]+=nGPU; }
		else if(index(machine_name,"GTX")>0) { printf("\033[1;34m%20s%4d\033[0m",machine_name,$1); num["gtx"]+=$1; gpu_num["gtx"]+=nGPU; }
		else if(index(machine_name,"gps")>0) { printf("\033[1;35m%20s%4d\033[0m",machine_name,$1); num["ubitus"]+=$1; gpu_num["ubitus"]+=nGPU; }
		else if(index(machine_name,"ubuntu")>0) { printf("\033[36m%20s%4d\033[0m",machine_name,$1); num["itri"]+=$1; gpu_num["itri"]+=nGPU; }
		else if(index(machine_name,"icwu")>0) { printf("\033[1;37m%20s%4d\033[0m",machine_name,$1); num["icwu"]+=$1; gpu_num["icwu"]+=8; }
		else { printf("%20s%4d",machine_name,$1); num["other"]+=$1; gpu_num["other"]+=nGPU; }
	} else { total = $1; c--; }
	c++;
	if(c%8==0) { print ""; }
} END {
	total_gpu = 0;
	for(key in num) {
		name[num[key]] = key;
		gpu[num[key]] = gpu_num[key];
		total_gpu += gpu_num[key];
	}
	
	if(c%8!=0) { print "\n"; }
	machine_name = "Total";
	printf("\033[1;37m%10s (%3d)%6d\033[0m\n",machine_name,total_gpu,total);
	n = asort(num);
	for(i=n; i>=1; i--) {
		printf("\033[1;31m%10s (%3d)%6d (%.3f%%)\033[0m\n",name[num[i]],gpu[num[i]],num[i],num[i]*100/total);
	}
}'

echo "[Evaluator progress]"
cat $DIR/$ITERATION/*Evaluator.sgf | sed 's/ //g' | sed 's/PB\[iter_/#/g;s/.caffemodel\]PW\[iter_/#/g;s/.caffemodel\]DT/#/g;s/RE\[/#/g;s/\]C\[/#/g' | awk -F "#" '{
	iter = ($2>$3)? $2: $3;
	name[iter] = ($2>$3)? $2" vs "$3":": $3" vs "$2":";
	if(iter==$2) {
		# black
		if(index($5,"B+")>0) { win_black[iter]++; }
		total_black[iter]++;
	} else {
		# white
		if(index($5,"W+")>0) { win_white[iter]++; }
		total_white[iter]++;
	}
} END {
	for(key in name) {
		win = win_black[key] + win_white[key];
		total = total_black[key] + total_white[key];

		if(total>0) { printf "["name[key]" "win*100/total"% ("win"/"total") "; }
		if(total_black[key]>0) { printf "black: "win_black[key]*100/total_black[key]"% ("win_black[key]"/"total_black[key]") "; }
		if(total_white[key]>0) { printf "white: "win_white[key]*100/total_white[key]"% ("win_white[key]"/"total_white[key]")"; }
		if(total>0) { print "]"; }
	}
}'

cat $DIR/$ITERATION/*Evaluator.sgf | awk -F ";" '{ sum+=NF-2; c++; } END { if(c>0) { print "Avg. game length = "sum/c" / "c; } }'

