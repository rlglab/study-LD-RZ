#! /bin/bash

awk -v board_size=$1 'BENGIN {
} {
	n=split($0,chars,"");
	for(i=1; i<=n; i++) {
		if(chars[i]==";" && (chars[i+1]=="B" || chars[i+1]=="W")) {
			move_color = chars[i+1];
			for(i+=2; chars[i]!="]"; i++) { move = move""chars[i]; }
			move = move"]";
			if(chars[i+1]!="C") { printf ";"move_color move; move = ""; }
		} else if(chars[i]=="C" && chars[i+1]=="[") {
			comment = "";
			comment_to_label = "";
			i += (chars[i+2]=="*")? 3: 2;
			for(; chars[i]!="*"; i++) { comment_to_label = comment_to_label""chars[i]; }
			for(i++; chars[i]!="]"; i++) { comment = comment""chars[i]; }
			
			nLabel=split(comment_to_label,label,"[,:]");
			printf "LB"
			for(j=1; j<nLabel; j+=2) {
				if(label[j]==board_size*board_size) { continue; }
				printf("[%c%c:%d]",label[j]%board_size+97,board_size-1-int(label[j]/board_size)+97,label[j+1]);
			}
			printf ";"move_color move;
			printf "C["comment"]";
			move = "";
		} else {
			printf chars[i];
		}
	}
	print "";
}'
