#!/bin/bash

if [ "$1" == "" ]; then
	echo "$0 <log_file> <nn_komi>=7.5 <color_dependent>=0"
	exit 1
fi

if [ "$2" == "" ]; then
	nn_komi=7.5
else
	nn_komi=$2
fi

if [ "$3" == "" ]; then
	color_dependent=0
else
	color_dependent=$3
fi

echo $0 $1 $nn_komi $color_dependent

cat $1 | awk -v MAX_MOVE=285 -v KOMI="$nn_komi" -v COLOR_DEPENDENT="$color_dependent"  '
{
	# format: file_name line move_number game_komi turn_color winner_color min_komi ~~~~~
    
	#file_name = $1;
	#line = $2;
	move_number = ($3>=MAX_MOVE-1)? MAX_MOVE-1: $3;
	game_komi = $4;
	turn_color = $5;
	winner_color = $6;
	min_komi = $7;
	
	# value start from 8
	value_index = (KOMI-min_komi<0)? 8: KOMI-min_komi+8;
	value = $value_index*2-1;
	if (COLOR_DEPENDENT == 1){
		result_value = (winner_color=="B")? 1.0f: -1.0f;
	}
	else{
		result_value = (winner_color==turn_color)? 1.0f: -1.0f;
	}	
	loss = 0.5 * ((result_value-value)^2);

	MSE[game_komi][move_number] += loss;
	COUNT[game_komi][move_number]++;
}
END {
	for( game_komi=-361.5; game_komi<=361.5; game_komi++ ) {
		if( !(game_komi in MSE) ) { continue; }
		
		print "game_komi: "game_komi;
		total_mse = total_count = 0;
		for( move_number=0; move_number<MAX_MOVE; move_number++ ) {
			if( COUNT[game_komi][move_number]==0 ) { print "0"; }
			else { print MSE[game_komi][move_number]/COUNT[game_komi][move_number]; }
			total_mse += MSE[game_komi][move_number];
			total_count += COUNT[game_komi][move_number];
		}
		print "Total: "total_mse/total_count;
		print "============================================";
	}
}
'
