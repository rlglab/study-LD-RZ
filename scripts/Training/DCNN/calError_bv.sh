#!/bin/bash

if [ "$1" == "" ] ; then
	echo "./calError_bv.sh bv_loss.txt threshold=0(0~1)"
	exit
fi

input=$1
RATIO=$2

if [ "$2" == "" ]; then
	RATIO=0
fi
 
cat $input | awk -v ratio=$RATIO ' {
 totalCount++; 
 split($0,a," "); 
 dError = 0; 
 moveNumber = ($2 >= 285) ? 285 : $2;
 for(i=6; i<367; ++i){ 
	if(a[i] >=ratio || a[i] <= 0-ratio){ 
		dError += (a[i]-a[i+361])*(a[i]-a[i+361]);
		isGuessRight=0;
		if( (a[i] >= ratio && a[i+361] >=ratio) || (a[i] <= 0-ratio && a[i+361] <= 0-ratio)){ isGuessRight=1; } 
		
		guess[moveNumber]++; 
		if( isGuessRight == 1){ guessRight[moveNumber]++;} 	
	} 
 }
 allError[moveNumber]+=dError; 
 count[moveNumber]++;

} 
END { 
	print "Error\t\t\tGuessNum\t\t\tGuess%\t\t\tRight While Guess%"
	for(i=0; i<=285; ++i){ 
		print allError[i] / count[i]"\t\t\t"guess[i]"\t\t\t"guess[i]/361/count[i]"\t\t\t"guessRight[i]/guess[i];  
	}  
	print totalCount;} 
'