#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]){
    
    // 인자로 파일을 2개 받아야함
	if (argc < 3){
		fprintf(stderr, "usage: %s <file1> <file2>\n", argv[0]);
		exit(1);
	}

    // link함수로 argv[1]로 지정한 파일과 같은 파일을
    // argv[2]로 지정한 이름으로 새로운 파일을 생성 (동일한 디스크 공간을 가리킴)
	if (link(argv[1], argv[2]) == -1) {
		fprintf(stderr, "link error for %s\n", argv[1]);
		exit(1);
	}
	exit(0);
}
