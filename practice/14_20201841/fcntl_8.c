#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "ssu_employee.h"  // 구조체 ssu_employee의 정의를 포함

#define DUMMY 0  // fcntl 함수 호출 시 사용되는 더미 값

int main(int argc, char *argv[])
{
	struct ssu_employee record;  // 직원 정보를 저장할 구조체 변수
	int fd;  // 파일 디스크립터
	int flags;  // 파일 상태 플래그
	int length;  // 기록할 데이터의 길이
	int pid;  // 현재 프로세스 ID

	// 프로그램 인자가 부족할 경우 사용법을 출력하고 종료
	if (argc < 2) {
		fprintf(stderr, "Usage : %s file\n", argv[0]);
		exit(1);
	}

	// argv[1] 파일을 읽기/쓰기 모드로 엶
	if ((fd = open(argv[1], O_RDWR)) < 0) {
		fprintf(stderr, "open error for %s\n", argv[1]);
		exit(1);
	}

	// 파일 상태 플래그를 가져옴
	if ((flags = fcntl(fd, F_GETFL, DUMMY)) == -1) {
		fprintf(stderr, "fcntl F_GETFL error\n");
		exit(1);
	}

	// O_APPEND 플래그를 설정
	flags |= O_APPEND;

	// 설정된 플래그를 파일 디스크립터에 적용
	if (fcntl(fd, F_SETFL, flags) == -1) {
		fprintf(stderr, "fcntl F_GETFL error\n");
		exit(1);
	}

	pid = getpid();  // 현재 프로세스 ID를 가져옴

	while(1) {
		printf("Enter employee name : ");
		scanf("%s", record.name);  // 직원 이름 입력

		// 이름이 '.'이면 루프 종료
		if (record.name[0] == '.')
			break;

		printf("Enter employee salary : ");
		scanf("%d", &record.salary);  // 직원 급여 입력
		record.pid = pid;  // 현재 프로세스 ID를 저장
		length = sizeof(record);  // 기록할 데이터의 길이 설정

		// 파일에 직원 정보를 기록, 기록 길이가 맞지 않으면 오류 출력 후 종료
		if (write(fd, (char *)&record, length) != length) {
			fprintf(stderr, "record write error\n");
			exit(1);
		}
	}

	close(fd);  // 파일 디스크립터 닫기
	exit(0);
}
