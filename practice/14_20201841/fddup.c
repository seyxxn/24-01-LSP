#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(void)
{
	int testfd;  // 새로운 파일 디스크립터
	int fd;  // 원본 파일 디스크립터

	// "test.txt" 파일을 생성 (이미 존재하면 열기)
	fd = open("test.txt", O_CREAT);

	// 파일 디스크립터 fd를 복사하여 5 이상 중 가장 작은 번호의 파일 디스크립터 할당
	testfd = fcntl(fd, F_DUPFD, 5);
	printf("testfd : %d\n", testfd);  // 새로운 파일 디스크립터 번호 출력

	// 다시 파일 디스크립터 fd를 복사하여 5 이상 중 가장 작은 번호의 파일 디스크립터 할당
	testfd = fcntl(fd, F_DUPFD, 5);
	printf("testfd : %d\n", testfd);  // 새로운 파일 디스크립터 번호 출력

	// 사용자 입력을 기다림
	getchar();
}
