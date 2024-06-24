#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define NAMESIZE 50

// 구조체 정의: 직원 정보를 저장하는 구조체
struct employee {
    char name[NAMESIZE]; // 직원 이름
    int salary;          // 직원 급여
    int pid;             // 프로세스 ID
};

int main(int argc, char *argv[])
{
    // 파일 잠금 구조체 및 기타 변수 선언
    struct flock lock;
    struct employee record;
    int fd, recnum, pid;
    long position;
    char ans[5];

    // 파일 열기
    if ((fd = open(argv[1], O_RDWR)) == -1) {
        perror(argv[1]); // 에러 출력
        exit(1);         // 프로그램 종료
    }

    // 현재 프로세스 ID 가져오기
    pid = getpid();

    // 무한 루프: 사용자가 음수 값을 입력할 때까지 반복
    for (;;) {
        printf("\nEnter record number : ");
        scanf("%d", &recnum); // 사용자로부터 레코드 번호 입력 받음

        // 음수가 입력되면 루프를 종료
        if (recnum < 0)
            break;

        // 레코드의 위치 계산
        position = recnum * sizeof(record);

        // 파일 잠금 설정
        lock.l_type = F_RDLCK;   // 읽기 잠금 설정
        lock.l_whence = 0;        // 파일의 시작부터 오프셋 계산
        lock.l_start = position;  // 잠금 시작 위치
        lock.l_len = sizeof(record); // 잠금 길이

        // 파일 잠금 설정 요청
        if (fcntl(fd, F_SETLKW, &lock) == -1) {
            perror(argv[1]); // 에러 출력
            exit(2);         // 프로그램 종료
        }

        // 파일 오프셋 이동 및 레코드 읽기
        lseek(fd, position, 0);
        if (read(fd, (char *)&record, sizeof(record)) == 0) {
            printf("record %d not found\n", recnum); // 레코드를 찾지 못한 경우 메시지 출력
            lock.l_type = F_UNLCK; // 잠금 해제
            fcntl(fd, F_SETLK, &lock); // 파일 잠금 설정 해제 요청
            continue; // 반복문의 다음 반복으로 이동
        }

        // 직원 정보 출력
        printf("Employee : %s, salary : %d\n", record.name, record.salary);

        // 급여를 업데이트할 지 여부 확인
        printf("Do you want to update salary (y or n)? ");
        scanf("%s", ans);

        // 사용자 입력에 따라 처리
        if (ans[0] != 'y') {
            lock.l_type = F_UNLCK; // 잠금 해제
            fcntl(fd, F_SETLK, &lock); // 파일 잠금 설정 해제 요청
            continue; // 반복문의 다음 반복으로 이동
        }

        // 쓰기 잠금 설정
        lock.l_type = F_WRLCK; // 쓰기 잠금 설정
        if (fcntl(fd, F_SETLKW, &lock) == -1) {
            perror(argv[1]); // 에러 출력
            exit(3);         // 프로그램 종료
        }

        // 프로세스 ID와 새로운 급여 입력 받음
        record.pid = pid;
        printf("Enter new salary: ");
        scanf("%d", &record.salary);

        // 파일 오프셋 이동 및 레코드 쓰기
        lseek(fd, position, 0);
        write(fd, (char *)&record, sizeof(record));

        // 파일 잠금 설정 해제
        lock.l_type = F_UNLCK; // 잠금 해제
        fcntl(fd, F_SETLK, &lock); // 파일 잠금 설정 해제 요청
    }

    // 파일 닫기
    close(fd);

    // 프로그램 종료
    return 0;
}
