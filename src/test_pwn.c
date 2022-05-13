// Author: i0gan
// Github: https://github.com/i0gan/pwn_waf
// Pwn Waf for AWD CTF

#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>

int main(void) {
	setbuf(stdin, NULL);
	setbuf(stdout, NULL);
	char buf[128];
	puts("Test puts:");

#define show_str "Test write\x00\x01\x02\x03\xff\n"
	write(1, show_str, sizeof(show_str) - 1);

	puts("Test read:");
	read(0, buf, 128);

	puts("Test gets:\n");
	gets(buf);

	puts("Test get shell:\n");
	//system("/bin/sh");

    execve("/bin/sh", NULL, NULL);

	return 0;
}
