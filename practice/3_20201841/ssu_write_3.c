#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "ssu_employee.h"

int main(int argc, char *argv[])
{
	struct ssu_employee record; // 구조체 변수 선언
	int fd;						// 파일 디스크립터

	// 명령줄에서 명령어와 파일명을 받아야하기 때문에 총 2개의 인자를 입력받아야 함. 파일명을 입력받지 못한 경우 예외 처리
	if (argc < 2)
	{
		fprintf(stderr, "usage : %s file\n", argv[0]); // 에러출력
		exit(1);									   // 비정상 종료
	}

	// 인자로 받은 파일을 open 함수를 이용하여 파일 디스크립터를 반환받음
	if ((fd = open(argv[1], O_WRONLY | O_CREAT | O_EXCL, 0640)) < 0)
	{													 // 에러 처리
		fprintf(stderr, "open error for %s\n", argv[1]); // 에러 출력
		exit(1);										 // 비정상 종료
	}

	// while 반목문을 통해 사용자가 '.'를 입력할 때까지 계속해서 회원의 정보를 입력받게 함
	while (1)
	{
		printf("Enter employee name <SPACE> salary : "); // 회원의 이름과 수익을 공백 기준으로 입력받음
		scanf("%s", record.name);

		if (record.name[0] == '.') // '.'를 입력하면 반복문을 탈출하고 프로그램 종료
			break;

		scanf("%d", &record.salary);
		record.pid = getpid();						// getpid() : 함수를 호출한 프로세스의 ID를 반환받음
		write(fd, (char *)&record, sizeof(record)); // 구조체의 내용을 파일에 씀
	}
	close(fd);
	exit(0);
}
