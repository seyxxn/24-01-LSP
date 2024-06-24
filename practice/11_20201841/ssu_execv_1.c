#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/resource.h>
#include <sys/wait.h>

// 함수 원형 선언
double ssu_maketime(struct timeval *time);
void term_stat(int stat);
void ssu_print_child_info(int stat, struct rusage *rusage);

int main(void) {
    struct rusage rusage; // 자원 사용량 정보를 저장할 구조체
    pid_t pid; // 자식 프로세스 ID 저장 변수
    int status; // 자식 프로세스의 종료 상태를 저장할 변수

    // 자식 프로세스 생성
    if ((pid = fork()) == 0) { // 자식 프로세스인 경우
        char *args[] = {"find", "/", "-maxdepth", "4", "-name", "stdio.h", NULL}; // execv로 실행할 명령어 및 옵션
        if (execv("/usr/bin/find", args) < 0) { // execv로 명령어 실행
            fprintf(stderr, "execv error\n"); // 오류 발생 시 에러 메시지 출력
            exit(1); // 프로세스 종료
        }
    }

    // 부모 프로세스가 자식 프로세스의 종료를 대기
    if (wait3(&status, 0, &rusage) == pid) // 자식 프로세스의 종료를 기다림
        ssu_print_child_info(status, &rusage); // 종료 정보 출력
    else {
        fprintf(stderr, "wait3 error\n"); // wait3 함수 실패 시 에러 메시지 출력
        exit(1); // 프로세스 종료
    }

    exit(0); // 프로그램 종료
}

// timeval 구조체를 이용하여 시간을 계산하는 함수
double ssu_maketime(struct timeval *time) {
    return ((double) time->tv_sec + (double) time->tv_usec / 1000000.0); // 초 단위 시간 반환
}

// 자식 프로세스의 종료 상태에 대한 정보를 출력하는 함수
void term_stat(int stat) {
    if (WIFEXITED(stat)) // 자식 프로세스가 정상적으로 종료된 경우
        printf("normally terminated. exit status = %d\n", WEXITSTATUS(stat)); // 종료 상태 출력
    else if (WIFSIGNALED(stat)) // 자식 프로세스가 시그널에 의해 종료된 경우
        printf("abnormal termination by signal %d. %s\n", WTERMSIG(stat), 
#ifdef WCOREDUMP
                WCOREDUMP(stat) ? "core dumped" : "no core" // 코어 덤프 여부 출력
#else
                NULL
#endif
                );
    else if (WIFSTOPPED(stat)) // 자식 프로세스가 중단된 경우
        printf("stopped by signal %d\n", WSTOPSIG(stat)); // 중단된 시그널 출력
}

// 자식 프로세스의 종료 정보와 자원 사용량 정보를 출력하는 함수
void ssu_print_child_info(int stat, struct rusage *rusage) {
    printf("Termination info follows\n"); // 종료 정보 출력 시작
    term_stat(stat); // 종료 상태 정보 출력
    printf("user CPU time : %.2f(sec)\n", ssu_maketime(&rusage->ru_utime)); // 사용자 CPU 시간 출력
    printf("system CPU time : %.2f(sec)\n", ssu_maketime(&rusage->ru_stime)); // 시스템 CPU 시간 출력
}
