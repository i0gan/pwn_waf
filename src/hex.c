/*
	print binary data as hex string
	author: i0gan
*/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

int show(const char *binary_path) {
	int fd = open(binary_path, O_RDONLY);
	unsigned char ch;
	char buf[16];
	if(-1 == fd) {
		perror("open:");
		return -1;
	}
	write(1, "\"", 1);
	while(read(fd, &ch, 1)) {
		sprintf(buf, "\\x%02x", ch);
		write(1, buf, strlen(buf));
	}
	write(1, "\"\n", 2);
	close(fd);
}

void help() {
	printf("Usage: ./dump -f [your_binary_waf]\n");
}

int main(int argc, char *argv[]) {
	int c;
	if(argc < 2) {
		help();
		return -1;
	}
	while((c = getopt(argc, argv, "f:")) != -1) {
		switch(c) {
			case 'f':
				show(optarg);
			break;
			default:
			break;
		}
	}
	return 0;	
}
