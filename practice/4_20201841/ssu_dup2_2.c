#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024

int main(void){
	char buf[BUFFER_SIZE];
	char *fname = "ssu_test.txt";
	int fd;
	int length;
	
	if ((fd = open(fname, O_RDONLY, 0644)) < 0){
		fprintf(stderr, "open error for %s\n", fname);
		exit(1);
	}

    // 1(표준 출력)을 4로 복사하여 4도 표준 출력을 함
	if (dup2(1,4) != 4) {
		fprintf(stderr, "dup2 call failed\n");
		exit(1);
	}

	while(1){
		length = read(fd, buf, BUFFER_SIZE); 
		// read 함수로 파일의 내용을 읽고 읽은 바이트 수 만큼 length에 저장
		
		if (length <= 0) // 읽지 못한 경우 에러 처리 
			break;

		write(4, buf, length);
		// 표준 출력 1번을 4번으로 복사했기 때문에
		// 4번 파일 디스크립터를 인자로하여 write()하면 표준 출력과 같기 때문에 파일의 내용이 출력됨
	}
	exit(0);
}
