#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int main(void){
	char buf[BUFFER_SIZE];

	setbuf(stdout, buf);
	// setbuf 함수를 이용하여 stdout의 버퍼를 위에서 정의한 buf로 설정
	// stdout의 출력이 즉시 화면에 나타나는 것이 아니라 먼저 buf 배열에 저장되게 됨
	// 이 배열이 꽉 차거나 버퍼가 플러시 될 때까지 출력 내용이 화면에 나타나지 않음
	printf("Hello, ");
	sleep(1);
	printf("OSLAB!!");
	sleep(1);
	printf("\n");
	sleep(1);
	// 위의 세 출력은 사용자 정의 버퍼를 사용하여 출력됨

	setbuf(stdout, NULL);
	// setbuf 함수를 이용하여 버퍼를 NULL로 설정하면 버퍼링 없이 직접 출력됨
	printf("How");
	sleep(1);
	printf(" are");
	sleep(1);
	printf(" you?");
	sleep(1);
	printf("\n");
	exit(0);
}
