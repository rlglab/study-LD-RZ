#!/bin/awk -f

function reset() {
  pIndex=0;
  nCount=0;
  pDiff=0;
  nDiff=0;
  nTTsize=0;
  nEntry=0;
  nMaxEntry=0;
  nCompare=0; # How many comparision in this lookup
  nMaxCompare=0; # maximum comparision among the lookups
  nLeftNullLeaf=0;
  nRightNullLeaf=0;
  nLeftIDSkipEdge=0;
  nRightIDSkipEdge=0;
  nLeftFoundStopEdge=0;
  nRightFoundStopEdge=0;
  nLeftCompareEdge=0; # normal visit edge
  nRightCompareEdge=0;
  nNullTotalLegnth=0;
  nIdSkipTotalLength=0;
  nFoundStopTotalLength=0;
  nCompareTotalLength=0;
  nNullPath=0;
  nIdSkipPath=0;
  nFoundStopPath=0;
  nComparePath=0;
  nMaxNullPathLength=0;
  nMaxIdSkipPathLength=0;
  nMaxFoundStopPathLength=0;
  nMaxComparePathLength=0;
  nFound=0;
  nBlack=0;
  nWhite=0;
  nEmpty=0;
  nNone=0;
}   

BEGIN {	reset(); } 
{ 
	pIndex=$1;
	nCount++;
	if($2>0) { pDiff++; }
	else { nDiff++; }
	nTTsize+=$3;
	nEntry+=$5;
	if ($6>nMaxEntry) { nMaxEntry=$6; }
	nCompare+=$7;
	if ($8>nMaxCompare) { nMaxCompare=$8; }
	nLeftNullLeaf+=$9;
	nRightNullLeaf+=$10;
	nLeftIDSkipEdge+=$11;
	nRightIDSkipEdge+=$12;
	nLeftFoundStopEdge+=$13;
	nRightFoundStopEdge+=$14;
	nLeftCompareEdge+=$15; 
	nRightCompareEdge+=$16;
	nNullTotalLegnth+=$17;
	nIdSkipTotalLength+=$18;
	nFoundStopTotalLength+=$19;
	nCompareTotalLength+=$20;
	nNullPath+=$21;
	nIdSkipPath+=$22;
	nFoundStopPath+=$23;
	nComparePath+=$24;
	if ($25>nMaxNullPathLength) { nMaxNullPathLength=$25; }
	if ($26>nMaxIdSkipPathLength) { nMaxIdSkipPathLength=$26; }
	if ($27>nMaxFoundStopPathLength) { nMaxFoundStopPathLength=$27; }
	if ($28>nMaxComparePathLength) { nMaxComparePathLength=$28; }
	nFound+=$29;
	nBlack+=$30;
	nWhite+=$31;
	nEmpty+=$32;
	nNone+=$33;
} 

END { 
	printAll();
}

function printAll() {
  print pIndex"\t"nCount"\t"pDiff"\t"nDiff"\t"nTTsize"\t"nEntry"\t"nMaxEntry"\t"nCompare"\t"nMaxCompare"\t"\
    nLeftNullLeaf"\t"nRightNullLeaf"\t"nLeftIDSkipEdge"\t"nRightIDSkipEdge"\t"nLeftFoundStopEdge"\t"nRightFoundStopEdge"\t"\
    nLeftCompareEdge"\t"nRightCompareEdge"\t"nNullTotalLegnth"\t"nIdSkipTotalLength"\t"nFoundStopTotalLength"\t"nCompareTotalLength"\t"\
		nNullPath"\t"nIdSkipPath"\t"nFoundStopPath"\t"nComparePath"\t"\
		nMaxNullPathLength"\t"nMaxIdSkipPathLength"\t"nMaxFoundStopPathLength"\t"nMaxComparePathLength"\t"\
		nFound"\t"nBlack"\t"nWhite"\t"nEmpty"\t"nNone
    ;
}
