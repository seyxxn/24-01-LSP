#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "ssu_employee.h"

int main(int argc, char *argv[])
{
	struct ssu_employee record; // 구조체 변수를 선언
	int fd;						// 파일 디스크립터
	int record_num;				// 출력할 회원의 번호를 입력받을 변수

	if (argc < 2) // 명령어에 들어온 인자의 개수가 2개 이상이면 에러 처리
	{
		fprintf(stderr, "Usage : %s file\n", argv[0]); // 에러 출력
		exit(1);									   // 비정상 종료
	}

	if ((fd = open(argv[1], O_RDONLY)) < 0) // 명령줄에서 받은 파일을 open하고 에러 처리
	{
		fprintf(stderr, "open error for %s\n", argv[1]); // 에러 출력
		exit(1);										 // 비정상 종료
	}

	// 사용자가 음수를 입력하지 않는 이상 계속해서 회원 정보를 출력
	while (1)
	{
		printf("Enter record number : ");
		scanf("%d", &record_num); // 사용자에게 출력할 회원의 번호를 입력받음

		if (record_num < 0) // 음수를 입력받으면 break문을 통해 반복문을 탈출하고 프로그램 종료
			break;

		// lseek 함수로 출력하고자 하는 회원정보를 담고있는 부분으로 커서를 이동시킴
		// 이때, 가장 처음부터 ssu_employee 구조체의 크기 * 번호 만큼 커서를 이동시켜야 하므로,
		// whence인자로는 0, offset인자로는 회원 번호 * 구조체 크기 를 가짐
		if (lseek(fd, (long)record_num * sizeof(record), 0) < 0) // 에러처리
		{
			fprintf(stderr, "lseek error\n");
			exit(1); // 비정상 종료
		}

		if (read(fd, (char *)&record, sizeof(record)) > 0)
			printf("Employee : %s Salary : %d\n", record.name, record.salary);
		else
			printf("Record %d not found\n", record_num);
	}

	close(fd);
	exit(0);
}
