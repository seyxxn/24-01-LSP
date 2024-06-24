#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>

#define MAX_CALL 100  // 호출 기록의 최대 개수
#define MAX_BUF 20000  // 버퍼의 최대 크기

void serror(char *str)
{
	fprintf(stderr, "%s\n", str);  // 에러 메시지 출력 함수
	exit(1);  // 프로그램 종료
}

int main(void){
	int nread, nwrite, val, i = 0;  // 읽은 바이트 수, 쓰여진 바이트 수, fcntl 리턴 값, 반복 횟수
	char *ptr;  // 버퍼 포인터
	char buf[MAX_BUF];  // 입력 버퍼

	int call[MAX_CALL];  // 호출 기록을 저장할 배열

	// 표준 입력(STDIN_FILENO)에서 데이터를 읽어 버퍼에 저장
	nread = read(STDIN_FILENO, buf, sizeof(buf));
	fprintf(stderr, "read %d bytes\n", nread);  // 읽은 바이트 수 출력

	// 표준 출력(STDOUT_FILENO)의 플래그를 가져와서 O_NONBLOCK 플래그 추가
	if ((val = fcntl(STDOUT_FILENO, F_GETFL, 0)) < 0)
		serror("fcntl F_GETFL error");
	val |= O_NONBLOCK;
	if (fcntl(STDOUT_FILENO, F_SETFL, val) < 0)
		serror("fcntl F_SETFL error");

	// 버퍼에 있는 데이터를 표준 출력에 쓰기
	for (ptr = buf; nread > 0; i++){
		errno = 0;
		nwrite = write(STDOUT_FILENO, ptr, nread);

		// 호출 기록에 쓰여진 바이트 수 저장
		if (i < MAX_CALL)
			call[i] = nwrite;

		fprintf(stderr, "nwrite = %d, errno = %d\n", nwrite, errno);  // 쓰여진 바이트 수와 에러 번호 출력
		if (nwrite > 0){
			ptr += nwrite;
			nread -= nwrite;
		}
	}

	// 표준 출력(STDOUT_FILENO)의 플래그를 가져와서 O_NONBLOCK 플래그 해제
	if ((val = fcntl(STDOUT_FILENO, F_GETFL, 0)) < 0)
		serror("fcntl F_GETFL error");
	val &= ~O_NONBLOCK;
	if (fcntl(STDOUT_FILENO, F_SETFL, val) < 0)
		serror("fcntl F_SETFL error");

	// 호출 기록 출력
	for(i = 0; i < MAX_CALL; i++)
		fprintf(stdout, "call[%d] = %d\n", i, call[i]);

	exit(0);  // 프로그램 종료
}
