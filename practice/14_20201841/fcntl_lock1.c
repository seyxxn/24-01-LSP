#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <error.h>
#include <errno.h>

#define NAMESIZE 50  // 이름의 최대 크기
#define MAXTRIES 5  // 최대 재시도 횟수

// 직원 정보를 저장할 구조체 정의
struct employee {
	char name[NAMESIZE];
	int salary;
	int pid;
};

int main(int argc, char *argv[])
{
	struct flock lock;  // 파일 잠금을 위한 구조체
	struct employee record;  // 직원 정보를 저장할 구조체 변수
	int fd, sum = 0, try = 0;  // 파일 디스크립터, 급여 합계, 시도 횟수

	// 프로그램 시작 시 10초 대기
	sleep(10);

	// 읽기 전용으로 파일 열기
	if ((fd = open(argv[1], O_RDONLY)) == -1) {
		perror(argv[1]);  // 파일 열기에 실패하면 에러 메시지 출력 후 종료
		exit(1);
	}

	// 파일 잠금 설정
	lock.l_type = F_RDLCK;  // 읽기 잠금
	lock.l_whence = 0;  // 파일 시작 위치 기준
	lock.l_start = 0L;  // 시작 오프셋
	lock.l_len = 0L;  // 전체 파일 잠금

	// 파일 잠금을 설정하고, 실패 시 최대 MAXTRIES만큼 재시도
	while (fcntl(fd, F_SETLK, &lock) == -1) {
		if (errno == EACCES) {  // 접근 권한 문제일 경우
			if (try++ < MAXTRIES) {  // 최대 재시도 횟수 미만일 경우
				sleep(1);  // 1초 대기 후 재시도
				continue;
			}
			// 최대 재시도 횟수를 초과하면 에러 메시지 출력 후 종료
			printf("%s busy -- try later\n", argv[1]);
			exit(2);
		}
		perror(argv[1]);  // 다른 에러 발생 시 에러 메시지 출력 후 종료
		exit(3);
	}

	// 파일에서 직원 정보를 읽어와 출력하고 급여 합계를 계산
	sum = 0;
	while (read(fd, (char *)&record, sizeof(record)) > 0) {
		printf("Employee: %s, Salary: %d\n", record.name, record.salary);
		sum += record.salary;
	}
	printf("\nTotal salary: %d\n", sum);  // 총 급여 합계 출력

	// 파일 잠금 해제
	lock.l_type = F_UNLCK;
	fcntl(fd, F_SETLK, &lock);
	close(fd);  // 파일 디스크립터 닫기
}
