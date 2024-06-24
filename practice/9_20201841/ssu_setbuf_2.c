#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024

int main(void){
	char buf[BUFFER_SIZE];
	int a,b;
	int i;

    // setbuf 함수로 stdin의 버퍼를 buf 배열로 설정함
    // 표준 입력으로부터 읽어들인 데이터가 우선적으로 buf 배열에 저장됨
	setbuf(stdin, buf);
	scanf("%d %d", &a, &b);
    
    // 입력된 버퍼의 내용을 순회하며 출력
	for (i = 0; buf[i] != '\n'; i++)
		putchar(buf[i]);

	putchar('\n');
	exit(0);
}
