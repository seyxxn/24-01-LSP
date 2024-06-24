#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(int argc, char *argv[]){
	struct stat statbuf; // 파일 정보를 받아 저장할 stat 구조체 변수 선언
	
    // 파일명까지 입력받아야하기 때문에 argc가 2가 아니면 에러처리
	if (argc != 2) {
		fprintf(stderr, "usage: %s <file>\n", argv[0]);
		exit(1);
	}

    // stat함수를 이용해서 인자로 받은 파일의 정보를 statbuf에 저장
	if ((stat(argv[1], &statbuf)) < 0){
		fprintf(stderr, "stat error\n");
		exit(1);
	}

	printf("%s is %ld bytes\n", argv[1], statbuf.st_size);
	// 저장된 파일의 정보 중 파일의 크기(st_size)를 출력
	exit(0);
}
