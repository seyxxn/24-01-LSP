#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

// 파일 디스크립터에 플래그 설정 함수 선언
void set_flags(int fd, int flags);
// 파일 디스크립터에서 플래그 해제 함수 선언
void clr_flags(int fd, int flags);

char buf[500000];  // 버퍼 크기 선언

int main(void){
	int ntowrite, nwrite;  // 남은 바이트 수와 쓰여진 바이트 수
	char *ptr;  // 버퍼 포인터

	// 표준 입력(STDIN_FILENO)으로부터 데이터를 읽어 버퍼에 저장
	ntowrite = read(STDIN_FILENO, buf, sizeof(buf));
	fprintf(stderr, "reading %d bytes\n", ntowrite);  // 읽은 바이트 수를 출력
	
	set_flags(STDOUT_FILENO, O_NONBLOCK);  // 표준 출력(STDOUT_FILENO)에 O_NONBLOCK 플래그 설정

	ptr = buf;  // 버퍼 포인터 초기화
	while (ntowrite > 0) {  // 남은 바이트 수가 0보다 큰 동안 반복
		errno = 0;  // 에러 번호 초기화
		nwrite = write(STDOUT_FILENO, ptr, ntowrite);  // 데이터를 표준 출력에 쓰기
		fprintf(stderr, "nwrite = %d, errno = %d\n", nwrite, errno);  // 쓰여진 바이트 수와 에러 번호 출력

		if (nwrite > 0) {  // 쓰여진 바이트 수가 0보다 크면
			ptr += nwrite;  // 포인터를 앞으로 이동
			ntowrite -= nwrite;  // 남은 바이트 수 감소
		}
	}
	clr_flags(STDOUT_FILENO, O_NONBLOCK);  // 표준 출력에서 O_NONBLOCK 플래그 해제
	exit(0);  // 프로그램 종료
}

// 파일 디스크립터에 플래그 설정 함수 정의
void set_flags(int fd, int flags)
{
	int val;

	if ((val = fcntl(fd, F_GETFL, 0)) < 0) {  // 현재 플래그 가져오기
		fprintf(stderr, "fcntl F_GETFL failed");  // 실패 시 에러 메시지 출력
		exit(1);  // 프로그램 종료
	}

	val |= flags;  // 새로운 플래그 추가

	if (fcntl(fd, F_SETFL, val) < 0) {  // 플래그 설정
		fprintf(stderr, "fcntl F_SETFL failed");  // 실패 시 에러 메시지 출력
		exit(1);  // 프로그램 종료
	}
}

// 파일 디스크립터에서 플래그 해제 함수 정의
void clr_flags(int fd, int flags)
{
	int val;

	if ((val = fcntl(fd, F_GETFL, 0)) < 0){  // 현재 플래그 가져오기
		fprintf(stderr, "fcntl F_GETFL failed");  // 실패 시 에러 메시지 출력
		exit(1);  // 프로그램 종료
	}

	val &= ~flags;  // 플래그 해제

	if (fcntl(fd, F_SETFL, val) < 0) {  // 플래그 설정
		fprintf(stderr, "fcntl F_SETFL failed");  // 실패 시 에러 메시지 출력
		exit(1);  // 프로그램 종료
	}
}
