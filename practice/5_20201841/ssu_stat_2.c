#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

struct stat statbuf; // 파일의 정보를 받아 저장할 stat 구조체 변수 선언

void ssu_checkfile(char *fname, time_t *time);

int main(int argc, char *argv[]){
	time_t intertime; // 시간 값을 가질 변수 선언
    
    // 파일명까지 입력받아야하기 때문에 argc가 2가 아니면 에러처리
	if (argc != 2) {
		fprintf(stderr, "usage: %s <file>\n", argv[0]);
		exit(1);
	}

    // stat 함수를 이용해서 인자로 받은 파일의 정보를 statbuf에 저장
	if (stat(argv[1], &statbuf) < 0) {
		fprintf(stderr, "stat error for %s\n", argv[1]);
		exit(1);
	}

	intertime = statbuf.st_mtime;
	// st_mtime 멤버 변수로 파일의 최종 수정시간을 받음
	
	while (1) {
		ssu_checkfile(argv[1], &intertime);
		// 파일 명과 시간값을 가진 intertime 변수를 넘겨줌
		sleep(10); // 10초 동안 대기
	}
}

// 파일의 변경 여부를 확인하는 함수
void ssu_checkfile (char *fname, time_t *time) {
    // stat 함수를 이용하여 fname으로 받은 파일의 정보를 statbuf에 저장
	if (stat(fname, &statbuf) < 0) {
		fprintf(stderr, "Warning : ssu_checkfile() error!\n");
		// 파일이 삭제되면 stat 함수가 제대로 반환되지 않기 때문에 에러 발생
		exit(1);
	}
	else
		if (statbuf.st_mtime != *time) { 
		    // 이 전에 가지고 있던 시간 값과 현재 stat 함수로 받은 정보에서 가져온 st_mtime이 다르면
		    // 파일이 수정되었음을 의미함
			printf("Warning : %s was modified!.\n", fname);
			*time = statbuf.st_mtime; 
			// 현재 stat 함수로 받은 정보를 time에 갱신하여 저장
		}
}
