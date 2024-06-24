#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#define LINE_MAX 2048

static void ssu_alarm(int signo); // SIGALRM 시그널을 처리할 함수 선언

int main(void)
{
	char buf[LINE_MAX]; // 입력 데이터를 저장할 버퍼
	int n; // read() 함수의 반환값을 저장할 변수

    // SIGALRM 시그널에 대한 시그널 핸들러를 등록하고, 등록 실패 시 오류 메시지 출력 후 프로그램 종료
	if (signal(SIGALRM, ssu_alarm) == SIG_ERR) {
		fprintf(stderr, "SIGALRM error\n");
		exit(1);
	}

	alarm(10); // 10초 후에 SIGALRM 시그널 발생

    // 표준 입력으로부터 데이터를 읽어들임, 실패 시 오류 메시지 출력 후 프로그램 종료
	if ((n = read(STDIN_FILENO, buf, LINE_MAX)) < 0) {
		fprintf(stderr, "read() error\n");
		exit(1);
	}

	alarm(0); // SIGALRM 알람 해제
	write(STDOUT_FILENO, buf, n); // 읽어들인 데이터를 표준 출력에 씀
	exit(0); // 프로그램 종료
}

// SIGALRM 시그널을 처리하는 함수
static void ssu_alarm(int signo) {
	printf("ssu_alarm() called!\n");
}
