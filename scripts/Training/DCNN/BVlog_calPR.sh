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
 for(i=6; i<367; ++i){ 
	if(a[i] >=ratio || a[i] <= 0-ratio){ 
		dError += (a[i]-a[i+361])*(a[i]-a[i+361]);
		if($2 > 285){ guess[285]++; }
		else{ guess[$2]++; } 
	} 
 }
 if($2 > 285){ 
	allError[285]+=dError; 
	count[285]++;
 } 
 else{ 
	allError[$2]+=dError; 
	count[$2]++;  
 } 
} 
END { 
	print "Error\t\t\tguess"
	for(i=0; i<=285; ++i){ 
		print allError[i] / count[i]"\t\t\t"guess[i];  
	}  
	print totalCount;} 
'