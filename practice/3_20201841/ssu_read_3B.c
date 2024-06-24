#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(void)
{
	char character;		// 문자를 한 문자씩 읽어오기 위한 변수
	int fd;				// 파일 디스크립터
	int line_count = 0; // 개행의 개수를 셀 변수

	// 파일을 읽기 전용으로 open하고 fd를 변환받음
	if ((fd = open("ssu_test.txt", O_RDONLY)) < 0)
	{															// 에러처리
		fprintf(stderr, "open error for %s\n", "ssu_test.txt"); // 에러 출력
		exit(1);												// 비정상 종료
	}

	// while 반복문을 통해 개행 문자의 개수를 세어서 몇 줄인지 출력하는 함수
	while (1)
	{
		if (read(fd, &character, 1) > 0) // read 함수를 통해 파일의 내용을 1글자씩 읽어서 character에 저장
		{
			if (character == '\n') // 만약, 읽은 문자가 개행문자인 경우 개행 개수를 세는 변수를 +1
				line_count++;
		}
		else
			break; // 문자열이 모두 끝나면 else문으로 이동하여 break가 실행되고 반복문을 빠져나감
	}

	printf("Total line : %d\n", line_count); // 최종 개행문자개수(몇 줄인지)를 출력함
	exit(0);
}
