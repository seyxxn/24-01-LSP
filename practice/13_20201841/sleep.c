#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

// 현재 시간을 출력하는 함수
void ssu_timestamp(char *str);

int main(void)
{
    unsigned int ret; // sleep 함수의 반환값을 저장할 변수

    // sleep 호출 전에 현재 시간 출력
    ssu_timestamp("before sleep()");

    // sleep 함수를 호출하여 10초 동안 프로세스를 대기
    ret = sleep(10);

    // sleep 호출 후에 현재 시간 출력
    ssu_timestamp("after sleep()");

    // sleep 함수의 반환값 출력
    printf("sleep() returned %d\n", ret);

    exit(0);
}

// 현재 시간을 출력하는 함수
void ssu_timestamp(char *str) {
    time_t time_val;

    // 현재 시간을 구하고 출력
    time(&time_val);
    printf("%s the time is %s\n", str, ctime(&time_val));
}
