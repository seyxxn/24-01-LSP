#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

// printf 함수를 구현함
int main(void)
{
	char buf[BUFFER_SIZE]; // 문자를 읽을 버퍼를 생성
	int length;			   // 읽은 문자열의 길이를 저장하는 변수 생성

	length = read(0, buf, BUFFER_SIZE); // read 함수에서는 buf 변수로 BUFFER_SIZE 만큼의 문자열을 읽음
										// 이떄 fd를 0으로 지정하면, 이는 표준 입력이기 때문에 사용자가 입력한 문자열을 읽는다는 의미
										// 읽은 문자열의 byte수를 length에 저장
	write(1, buf, length);				// fd가 1이므로 표준 출력을 통해 length길이 만큼 buf에 저장되어있는 문자열을 출력
	exit(0);
}
