#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024

int main(void)
{
	char buf[BUFFER_SIZE];		  // 문자를 읽어들이기 위한 buf 배열 선언
	char *fname = "ssu_test.txt"; // 파일 이름
	int count;					  // 읽은 바이트 수를 저장할 변수 선언
	int fd1, fd2;				  // 파일 디스크립터

	// open 함수를 이용하여 파일 디스크립터 값 받음
	fd1 = open(fname, O_RDONLY, 0644);
	fd2 = open(fname, O_RDONLY, 0644);

	if (fd1 < 0 || fd2 < 0) // 파일 디스크립터 반환 값이 음수라면 에러 처리
	{
		fprintf(stderr, "open error for %s\n", fname); // 에러 출력
		exit(1);									   // 비정상 종료
	}

	count = read(fd1, buf, 25);				   // buf를 이용하여 25byte를 읽어옴
											   // 읽어온 문자는 buf에 저장되고, read는 읽어온 byte수를 반환하기 때문에 count는 읽은 byte수를 가짐
	buf[count] = 0;							   // 가장 끝의 문자열에 0을 넣어 문자열의 끝을 나타냄
	printf("fd1's first printf : %s\n", buf);  // buf를 출력하여 저장한 문자열을 확인함
	lseek(fd1, 1, SEEK_CUR);				   // lseek 함수를 이용하여 커서를 이동, 1칸 이동
											   // 현재 read로 읽은 곳을 커서가 가리키고 있고 1칸을 이동시켜 개행을 건너 띔
	count = read(fd1, buf, 24);				   // buf를 이용하여 24byte를 읽어옴
	buf[count] = 0;							   // 가장 끝의 문자열에 0을 넣어 문자열의 끝을 나타냄
	printf("fd1's second printf : %s\n", buf); // buf를 출력하어 저장한 문자열을 확인함

	count = read(fd2, buf, 25);				   // buf를 이용하여 25byte를 읽어옴
	buf[count] = 0;							   // 가장 끝의 문자열에 0을 넣어 문자열의 끝을 나타냄
	printf("fd2's first printf : %s\n", buf);  // buf를 출력하여 저장한 문자열을 확인함
	lseek(fd2, 1, SEEK_CUR);				   // lseek 함수를 이용하여 커서를 이동, 1칸 이동
	count = read(fd2, buf, 24);				   // 현재 read로 읽은 곳을 커서가 가리키고 있고 1칸을 이동시켜 개행을 건너 뜀
	buf[count] = 0;							   // 가장 끝의 문자열에 0을 넣어 문자열의 끝을 나타냄
	printf("fd2's second printf : %s\n", buf); // buf를 출력하여 저장한 문자열을 확인함

	exit(0); // 정상종료
}
