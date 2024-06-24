#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
    // 실행할 프로그램의 이름과 전달할 인자들을 지정한 배열
    char *argv[] = {
        "ssu_execl_test_1", "param1", "param2", (char *)0
    };

    // 실행할 프로그램에 전달할 환경 변수들을 지정한 배열
    char *env[] = {
        "NAME=value",       // NAME 환경 변수 설정
        "nextname=nextvalue", // nextname 환경 변수 설정
        "HOME=/home/seyeon", // HOME 환경 변수 설정
        (char *)0           // 배열의 끝을 나타내는 NULL 포인터
    };

    printf("this is the original program\n"); // 프로그램 시작을 알리는 메시지 출력
    execve("./ssu_execl_test_1", argv, env); // ssu_execl_test_1 프로그램을 실행하고 인자와 환경 변수를 전달
    printf("%s\n", "This line should never get printed\n"); // execve가 성공적으로 실행되면 이 코드는 실행되지 않음
    exit(0); // 프로그램 종료
}
