function reset() {
  nSumNodes=0;
  nBlack=0;
  nWhite=0;
  nEmpty=0;
  nNone=0;
  nDepth=0;
  maxDepth=0;
}   

BEGIN {	reset(); } 

{ 
  nSumNodes=$1;
  nBlack=$2;
  nWhite=$3;
  nEmpty=$4;
  nNone=$5;
  nDepth=$6;
  
  if (nDepth > maxDepth)
    maxDepth = nDepth;
  
  if (nSumNodes > 1) {
    nCount[nDepth]++;
    nTotalChildren=nSumNodes-1;
    nBlackRatio[nDepth]+=(nBlack/nTotalChildren);
    nWhiteRatio[nDepth]+=(nWhite/nTotalChildren);
    nEmptyRatio[nDepth]+=(nEmpty/nTotalChildren);
    nNoneRatio[nDepth]+=(nNone/nTotalChildren);
  }
} 

END {
  for (x = 0; x <= maxDepth; x++)
	print x"\t"(nBlackRatio[x]/nCount[x])"\t"(nWhiteRatio[x]/nCount[x])"\t"(nEmptyRatio[x]/nCount[x])"\t"(nNoneRatio[x]/nCount[x]);
}
