#!/bin/bash

TEST_OUTPUT_PATH="analysis_result"
# =====================================================================

function summarize_net {
	cat $1 | awk 'BEGIN {
		sl = MSE = total = 0;
	}{
		# sl
		if($7==0) { sl++; }
		
		# vn
		if($5==$6) { ans=1; }
		else { ans=-1; }
		vn=$8*2-1;
		MSE += (ans-vn)*(ans-vn);

		total++;
	} END {
		print sl/total,MSE/(total*2);
	}'
}

for result_file in $TEST_OUTPUT_PATH/*
do
	SL_VN=$(echo $result_file; summarize_net $result_file)
	if [[ -z "${RESULT// }" ]]; then
		RESULT="${SL_VN}"
	else
		RESULT="${RESULT}\n${SL_VN}"
	fi
done

echo -e $RESULT | awk '{
	iter_start=index($1,"iter_")+5;
	iter_end=index($1,".log");
	iteration=substr($1,iter_start,iter_end-iter_start);
	name_start=index($1,"B15_")+4;
	name_end=index($1,"_iter");
	name=substr($1,name_start,name_end-name_start);
	name2=substr(name,0,index(name,"_")-1);
	
	name_set[name] = name2;					# just a flag
	iteration_set[iteration] = iteration;	# just a flag
	sl_pr[iteration][name] = $2;
	vn_mse[iteration][name] = $3;
} END {
	n=asorti(iteration_set,iteration_set_sort,"@val_num_asc");
	
	printf "\t";
	for(key in name_set) { printf key"\t"; }
	print "";
	
	for(i=1; i<=193; i++) { printf "="; }
	print "";
	printf "\t";
	for(key in name_set) { printf("%8s\t",name_set[key]); }
	print "";
	
	for(i=1; i<=193; i++) { printf "="; }
	print "";
	for(i=1; i<=n; i++) {
		printf iteration_set_sort[i]"\t";
		for(key in name_set) {
			if(sl_pr[iteration_set_sort[i]][key]=="") { printf("        \t"); }
			else { printf("%.6f\t",sl_pr[iteration_set_sort[i]][key]); }
		}
		print "";
	}
	
	for(i=1; i<=193; i++) { printf "="; }
	print "";
	for(i=1; i<=n; i++) {
		printf iteration_set_sort[i]"\t";
		for(key in name_set) {
			if(vn_mse[iteration_set_sort[i]][key]=="") { printf("        \t"); }
			else { printf("%.6f\t",vn_mse[iteration_set_sort[i]][key]); }
		}
		print "";
	}
}'
