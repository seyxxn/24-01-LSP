#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define NAMESIZE 50  // 이름의 최대 크기

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
	int fd, recnum, pid;  // 파일 디스크립터, 레코드 번호, 프로세스 ID
	long position;  // 파일에서의 위치

	// 읽기/쓰기 모드로 파일 열기
	if ((fd = open(argv[1], O_RDWR)) == -1) {
		perror(argv[1]);  // 파일 열기에 실패하면 에러 메시지 출력 후 종료
		exit(1);
	}

	pid = getpid();  // 현재 프로세스 ID를 가져옴
	for(;;) {  // 무한 루프 시작
		printf("\nEnter record number: ");
		scanf("%d", &recnum);  // 레코드 번호 입력
		if (recnum < 0)  // 음수 입력 시 루프 종료
			break;
		position = recnum * sizeof(record);  // 레코드 번호에 따른 파일 위치 계산

		// 파일 잠금 설정
		lock.l_type = F_WRLCK;  // 쓰기 잠금
		lock.l_whence = 0;  // 파일 시작 위치 기준
		lock.l_start = position;  // 시작 오프셋
		lock.l_len = sizeof(record);  // 잠글 데이터의 길이

		// 파일 잠금을 설정하고, 실패 시 에러 처리
		if (fcntl(fd, F_SETLKW, &lock) == -1) {
			perror(argv[1]);
			exit(2);
		}

		// 파일 위치를 이동하고 레코드 읽기
		lseek(fd, position, 0);
		if (read(fd, (char *)&record, sizeof(record)) == 0) {
			// 레코드를 찾지 못하면 메시지 출력 후 잠금 해제
			printf("record %d not found\n", recnum);
			lock.l_type = F_UNLCK;
			fcntl(fd, F_SETLK, &lock);
			continue;
		}

		// 직원 정보 출력
		printf("Employee: %s, salary: %d\n", record.name, record.salary);
		record.pid = pid;  // 현재 프로세스 ID를 기록

		// 새로운 급여 입력
		printf("Enter new salary: ");
		scanf("%d", &record.salary);

		// 파일 위치를 이동하고 레코드 쓰기
		lseek(fd, position, 0);
		write(fd, (char*)&record, sizeof(record));

		// 파일 잠금 해제
		lock.l_type = F_UNLCK;
		fcntl(fd, F_SETLK, &lock);
	}
	close(fd);  // 파일 디스크립터 닫기
}
