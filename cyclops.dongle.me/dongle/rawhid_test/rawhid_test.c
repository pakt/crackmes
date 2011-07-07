#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#if defined(OS_LINUX) || defined(OS_MACOSX)
#include <sys/ioctl.h>
#include <termios.h>
#elif defined(OS_WINDOWS)
#include <conio.h>
#endif

#include "hid.h"
#include "usb_rawhid.h"


int main()
{
	int i, r, num, flag;
	char buf[64];

	// solution for name: Administrator
	char *s1 = "5B2B9A62EEB59F85AC6269C1F020";
	char *s2 = "8D0C6549677CD464B8EBA5463E8D";

	r = rawhid_open(1, VENDOR_ID, PRODUCT_ID, RAWHID_USAGE_PAGE, RAWHID_USAGE);
	if (r <= 0) {
		printf("no rawhid device found\n");
		return -1;
	}
	printf("found rawhid device\n");

	flag = 0;
	while (1) {
		// check if any Raw HID packet has arrived
		num = rawhid_recv(0, buf, 64, 220);
		if (num < 0) {
			printf("\nerror reading, device went offline\n");
			rawhid_close(0);
			return 0;
		}
		if (num > 0) {
			printf("\nrecv %d bytes:\n", num);
			for (i=0; i<num; i++) {
				printf("%02X ", buf[i] & 255);
				if (i % 16 == 15 && i < num-1) printf("\n");
			}
			printf("\n");
		}

		if(!flag){
			buf[0] = strlen(s1);
			buf[1] = strlen(s2);
			memcpy(buf+2, s1, buf[0]);
			memcpy(buf+2+buf[0], s2, buf[1]);
			rawhid_send(0, buf, 64, 100);
			flag = 1;
		}
	}
}

#if defined(OS_LINUX) || defined(OS_MACOSX)
// Linux (POSIX) implementation of _kbhit().
// Morgan McGuire, morgan@cs.brown.edu
static int _kbhit() {
	static const int STDIN = 0;
	static int initialized = 0;
	int bytesWaiting;

	if (!initialized) {
		// Use termios to turn off line buffering
		struct termios term;
		tcgetattr(STDIN, &term);
		term.c_lflag &= ~ICANON;
		tcsetattr(STDIN, TCSANOW, &term);
		setbuf(stdin, NULL);
		initialized = 1;
	}
	ioctl(STDIN, FIONREAD, &bytesWaiting);
	return bytesWaiting;
}
static char _getch(void) {
	char c;
	if (fread(&c, 1, 1, stdin) < 1) return 0;
	return c;
}
#endif


