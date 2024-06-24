#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

void ssu_setbuf(FILE *fp, char *buf);

int main(void){
	char buf[BUFFER_SIZE];
	char *fname = "/dev/pts/2"; 
	// tty 명령어로부터 현재 표준 입력에 접속된 터미널 장치 파일 이름을 알아냄
	FILE *fp;
    
    // 현재 버퍼 오픈
	if ((fp = fopen(fname, "w")) == NULL) {
		fprintf(stderr, "fopen error for %s", fname);
		exit(1);
	}

	ssu_setbuf(fp, buf);
	// ssu_setbuf 함수를 호출하여 파일 스트림의 버퍼링을 설정
	// 여기서는 사용자 정의 버퍼인 buf를 사용
	fprintf(fp, "Hello, ");
	sleep(1);
	fprintf(fp, "UNIX!!");
	sleep(1);
	fprintf(fp, "\n");
	sleep(1);
	// 버퍼가 설정된 후 실행되기 때문에 버퍼에 넣은 후 한 번에 출력됨
	
	ssu_setbuf(fp, NULL);
	// 여기서는 NULL을 전달하여 비버퍼링 모드로 전환
	fprintf(fp, "HOW");
	sleep(1);
	fprintf(fp, " ARE");
	sleep(1);
	fprintf(fp, " YOU?");
	sleep(1);
	fprintf(fp, "\n");
	sleep(1);
	// fprintf()를 호출할때마다 버퍼에 넣지 않고 바로 출력
	
	exit(0);
}

void ssu_setbuf(FILE *fp, char *buf) {
	size_t size;
	int fd;
	int mode;

	fd = fileno(fp); // 파일 스트림에서 사용 중인 파일 디스크립터 얻음

	if (isatty(fd)) // 해당 파일 디스크립터가 터미널에 연결되어있는지 확인
		mode = _IOLBF; // 연결되어있다면 라인버퍼링모드를 사용
	else
		mode = _IONBF; // 그렇지 않다면 비버퍼링모드를 사용
    
    // 제공된 버퍼인 buf가 NULL이면 비버퍼링 모드 선택
	if (buf == NULL) {
		mode = _IONBF;
		size = 0;
	}else 
		size = BUFFER_SIZE; // 그렇지 않으면 BUFFER_SIZE 만큼의 버퍼 크기를 설정

	setvbuf(fp, buf, mode, size);
	// setvbuf로 파일 스트림의 버퍼링 설정을 적용
}
