#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
	struct flock lock;  // 파일 잠금을 위한 구조체
	int fd;  // 파일 디스크립터
	char command[100];  // 시스템 명령어 문자열

	// 읽기/쓰기 모드로 파일 열기
	if ((fd = open(argv[1], O_RDWR)) == -1) {
		perror(argv[1]);  // 파일 열기에 실패하면 에러 메시지 출력 후 종료
		exit(1);
	}

	// 파일 잠금 설정
	lock.l_type = F_WRLCK;  // 쓰기 잠금
	lock.l_whence = 0;  // 파일 시작 위치 기준
	lock.l_start = 0l;  // 시작 오프셋
	lock.l_len = 0l;  // 전체 파일 잠금

	// 파일 잠금을 설정하고, 실패 시 에러 처리
	if (fcntl(fd, F_SETLK, &lock) == -1) {
		if (errno == EACCES) {  // 접근 권한 문제일 경우
			printf("%s busy -- try later\n", argv[1]);  // 파일이 바쁨을 알림
			exit(2);
		}
		perror(argv[1]);  // 다른 에러 발생 시 에러 메시지 출력 후 종료
		exit(3);
	}

	// 파일을 vim으로 열기 위한 명령어 문자열 생성
	sprintf(command, "vim %s\n", argv[1]);
	system(command);  // 시스템 명령어 실행

	// 파일 잠금 해제
	lock.l_type = F_UNLCK;
	fcntl(fd, F_SETLK, &lock);
	close(fd);  // 파일 디스크립터 닫기

	return 0;
}
