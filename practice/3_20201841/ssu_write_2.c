#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define S_MODE 0644
#define BUFFER_SIZE 1024

// cp 함수를 구현함
int main(int argc, char *argv[])
{
	char buf[BUFFER_SIZE]; // 문자를 읽을 버퍼를 생성
	int fd1, fd2;		   // 파일 디스크립터
	int length;			   // 읽은 문자열의 길이를 저장하는 변수 생성

	// 명령어와 2개의 파일을 입력받아야 함 (총 3개의 인자를 명령줄에서 받아와야함)
	if (argc != 3) // 그 이외의 겨우는 에러처리
	{
		fprintf(stderr, "Usage : %s filein fileout\n", argv[0]); // 에러출력
		exit(1);												 // 비정상 종료
	}

	// open함수로 복사할 파일의 파일 디스크립터를 읽기 전용으로 반환받음
	if ((fd1 = open(argv[1], O_RDONLY)) < 0) // 에러 처리
	{
		fprintf(stderr, "open error for %s\n", argv[1]); // 에러 출력
		exit(1);										 // 비정상 종료
	}

	// open함수로 복사하여 생성할 파일을 파일 디스크립터를 반환받음
	// 이때 파일에 문자열을 출력하기 위해 0644 mode로 open함
	if ((fd2 = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, S_MODE)) < 0) // 에러처리
	{
		fprintf(stderr, "open error for %s\n", argv[2]); // 에러 출력 
		exit(1); // 비정상 종료
	}

	// while 반복문을 이용하여 fd1의 내용을 fd2에 복사함
	while ((length = read(fd1, buf, BUFFER_SIZE)) > 0) // read 함수로 fd1의 내용을 읽어와서 buf에 저장하고 읽은 byte수를 length에 저장함
		write(fd2, buf, length); // fd2에 buf의 내용을 length만큼 출력함

	exit(0);
}
