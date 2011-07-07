#include <iostream>
#include <windows.h>
#include <tchar.h>
#include "ecnr.h"
#include "big.h"
#include "crc32.h"
#include "usb_rawhid.h"	//constants

extern "C" {
	#include "hid.h"
}


using namespace std;

#ifndef MR_NOFULLWIDTH
Miracl precision(50,0);
#else
Miracl precision(50,MAXBASE);
#endif

#define USER_NAME_MAX_SIZE 256

void printError( TCHAR* msg )
{
  DWORD eNum;
  TCHAR sysMsg[256];
  TCHAR* p;

  eNum = GetLastError( );
  FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM |
         FORMAT_MESSAGE_IGNORE_INSERTS,
         NULL, eNum,
         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
         sysMsg, 256, NULL );

  // Trim the end of the line and terminate it with a null
  p = sysMsg;
  while( ( *p > 31 ) || ( *p == 9 ) )
    ++p;
  do { *p-- = 0; } while( ( p >= sysMsg ) &&
                          ( ( *p == '.' ) || ( *p < 33 ) ) );

  // Display the message
  _tprintf( TEXT("\n\t%s failed with error %d (%s)"), msg, (int)eNum, sysMsg );
}

int main(int argc, char *argv[]){
	Big n;
	ECNR signer;
	char *a = (char*)"DB7C2ABF62E35E668076BEAD2088";
	char *b = (char*)"659EF8BA043916EEDE8911702B22";
	char *p = (char*)"DB7C2ABF62E35E668076BEAD208B";

	char *Px = (char*)"9487239995A5EE76B55F9C2F098";
	char *Py = (char*)"A89CE5AF8724C0A23E0E0FF77500";
	char *ord = (char*)"DB7C2ABF62E35E7628DFAC6561C5";	//point order

	//char *Qx = (char*)"45CF81634B4CA4C6AAC505843B94";
	//char *Qy = (char*)"BDA8EEA7A5004255FA03C48D4AE8";
	char *k = (char*)"f6893de509504e9be7e85b7ae3b";	//ECDLP solution

	char sig1[64], sig2[64], buf[RAWHID_TX_SIZE];

	int r, l1, l2;

	TCHAR  userName[USER_NAME_MAX_SIZE];
	DWORD  bufCharCount = USER_NAME_MAX_SIZE;

    miracl *mip=&precision;
    mip->IOBASE=16;

	signer = ECNR();
	signer.set_curve(a, b, p);
	signer.set_point(k, ord, Px, Py);

	if(!GetUserName(userName, &bufCharCount))
		printError( TEXT((TCHAR*)"GetUserName") );
	_tprintf( TEXT("\nuser name: %s\n"), userName);

	unsigned int crc;
	crc = crc32((unsigned char*)userName, strlen(userName));
	printf("user name crc: %08x\n", crc);

	Big msg;
	Big n1 = Big(0), n2 = Big(0);
	Big &s1 = n1, &s2 = n2;
	//bool ok;

	msg = crc;
	signer.sign(msg, s1, s2);

	cout << "sig1=" << s1 << endl;
	cout << "sig2=" << s2 << endl;

	//ok = signer.verify(msg, s1, s2);
	//cout << "ok=" << ok << endl;

	r = rawhid_open(1, VENDOR_ID, PRODUCT_ID, RAWHID_USAGE_PAGE, RAWHID_USAGE);
	if (r <= 0) {
		printf("no rawhid device found, please plug in the dongle\n");
		return -1;
	}
	printf("found rawhid device\n");

	sig1 << s1;
	sig2 << s2;

	l1 = strlen(sig1);
	l2 = strlen(sig2);

	if(l1+l2+2+1 > RAWHID_TX_SIZE){
		printf("signature won't fit in xfer packet :p\n");
		rawhid_close(0);
		return 1;
	}

	buf[0] = strlen(sig1);
	buf[1] = strlen(sig2);
	memcpy(buf+2, sig1, l1);
	memcpy(buf+2+l1, sig2, l2);

	printf("sending the packet...\n");

	r = rawhid_send(0, buf, 64, 100);

	if(r != RAWHID_TX_SIZE){
		printf("sending packet failed :(\n");
		rawhid_close(0);
		return 1;
	}

	printf("success!\n");
	rawhid_close(0);

	return 0;
}
