#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(void)
{
	char c; // 문자를 한 문자씩 읽어오기 위한 변수
	int fd; // 파일 디스크립터

	// 파일을 읽기 전용으로 open하고 fd를 변환받음
	if ((fd = open("ssu_test.txt", O_RDONLY)) < 0)
	{															// 에러처리
		fprintf(stderr, "open error for %s\n", "ssu_test.txt"); // 에러 출력
		exit(1);												// 비정상 종료
	}

	// while 반복문을 통해 내용을 한 글자씩 읽어서 표준 출력으로 출력함
	while (1)
	{
		if (read(fd, &c, 1) > 0) // read 함수를 통해 파일의 내용을 1글자씩 읽어서 c에 저장함
			putchar(c);			 // putchar 함수는 매개변수로 들어온 문자를 표준출력하여 문자로 출력함
		else
			break; // 문자열이 모두 끝나면 else문으로 이동하여 break가 실행되고 반복문을 빠져나감
	}

	exit(0);
}
