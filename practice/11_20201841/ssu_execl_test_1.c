#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    extern char **environ; // 전역 변수로 선언된 환경 변수 배열
    char **str; // 환경 변수 배열을 탐색하기 위한 포인터
    int i;

    // 전달된 명령행 인자들을 출력하는 반복문
    for (i = 0; i < argc; i++)
        printf("argv[%d]: %s\n", i, argv[i]); // argv 배열의 각 요소를 출력

    // 환경 변수 배열을 탐색하면서 각 환경 변수의 값을 출력하는 반복문
    for (str = environ; *str != 0; str++)
        printf("%s\n", *str); // 환경 변수의 값을 출력

    exit(0); // 프로그램 종료
}
