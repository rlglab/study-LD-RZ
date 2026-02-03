#!/bin/bash

input=$1
simulationCount=$2
Threshold=$3

echo "bv with playout log: "$input;
echo "simulationCount is set to "$simulationCount".";
echo "BV Threshold: "$Threshold;


cat $input | awk -v BVThreshold=$Threshold -v maxSim=$simulationCount -v MAXMOVE=285 '

BEGIN { sim=0;}
{
	if(sim==0) {               # first line which contains BV answer and some sgf info , parse it
		#print $0;
		moveNumber = $2;
		gameResult = ($6 == "B") ? 1 : -1;
		komi = $4;
		split($0, attr," ");
		for(i=0;i<361;++i) { BVAnswer[i] = attr[i+6]; }
		iBlackWin= 0;	
	}
	else{        #  next N simulation result, need or not to be replace by BV result according to the threshold we set
		split($0, territory, " ");
		fResult =-komi; 
		for(i=0;i<361;++i){
			if(BVAnswer[i] >= BVThreshold) { estiTerritory[i] = 1; }
			else if(BVAnswer[i] <= 0-BVThreshold) { estiTerritory[i] = -1; }
			else {estiTerritory[i] = (territory[i] == "B") ? 1 : -1 ; }
			#print estiTerritory[i]" ";
			fResult += estiTerritory[i];
			#print fResult;
		}
		#print fResult;
		if(fResult > 0) iBlackWin++;
		# counting error here

	}

	sim++; # how many simulation we have handled, if it equals to the set value, reset it, which means we come to the next position
	if(sim > maxSim) { #summarize the result

		winrate = (iBlackWin/ maxSim)*2 -1;
		if(moveNumber >= MAXMOVE){ 
			dError[MAXMOVE] += (gameResult-winrate)*(gameResult-winrate) / 2;
			moveCount[MAXMOVE]++;
		}
		else{
			dError[moveNumber] += (gameResult-winrate)*(gameResult-winrate) /2;
			moveCount[moveNumber]++;
		}
		#print moveNumber" "iBlackWin" "winrate" "dError[moveNumber];
		sim=0; 
		iBlackWin=0;
	
	}
}

END {
	for(i=1; i<=MAXMOVE; ++i){
		print i" "dError[i]/moveCount[i]" "moveCount[i];
	}


}
'
