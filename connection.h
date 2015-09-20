// --*- C++ -*--
/*
 *  IIRLIB C Connection Modules
 *      
 *  Last modified on 2008 Feb 13th by Tetsunari Inamura
 *
 *	Copyright (c) Tetsunari Inamura 1998--2008.
 *	All Rights Reserved.
 */

/* Change LOG
 * 
 * 1998 Dec 8th
 *	$B%*%V%8%'%/%H;X8~$N%W%m%0%i%`4D6-$X(B
 *	fifo $B$d(B socket $B$K4X$9$k%W%m%0%i%`(B
 * 1998 Dec 10th
 *	$BJV$jCM$O$9$Y$F(B FAIL or SUCCESS $B$@$C$?$,!$JQ?t;2>H$N>l9g$H!$(BConstructor
 *	$B$K8B$j!$JV$jCM$r$=$N$^$^;H$&;v$K$9$k(B
 * 1998 Dec 11th
 *	connection $B7?$r(B Connection $B7?$K(B
 *	$B%m!<%+%kJQ?t(B connect $B$r(B connection $B$K!%(Bconnect $B$H$$$&L?Na$,$"$k(B!!
 * 1998 Dec 16th
 *	Send, Receive $B$K(B CONNECTION_INTEGER $B7?$r$D$1$k(B
 * 1999 Jan 15th
 *	CONNECTION_STREAM $B7?$,CB@8(B, $B9=B$BN$K(B stream $B$N3+;O%]%$%s%?$rDI2C(B
 * 1999 Jan 19th
 *	STRING$B7?$GAw$k;~$N8B3&$,(B 128 $BJ8;z$J$N$O!$(Bconnection.c $B$N$;$$$G$"$k$H(B
 *	  $B5$$,IU$/!%(Bmagic number $B$r;H$o$J$$$h$&$KJQ99(B
 * 1999 Jan 21st
 *	CONNECTION_LISP $B7?$r?7@_(B
 * 1999 Feb 16th
 *	CONNECTION_AUTODETECT : $B%5!<%P$,!$%/%i%$%"%s%H$N(B type $B$r<u$1$H$k$?$a(B
 *	CONNECTION_AUTOREPORT : $B%/%i%$%"%s%H$,(B,$B%5!<%P$K(B type $B$rJs9p$9$k$?$a(B
 *	$B%^%k%A%"%/%;%9$N$?$a$N4X?t72$rEPO?(B
 * 1999 Mar 21st
 *	$B?74X?t$r$$$m$$$mA}$d$9(B
 * 1999 May 12th
 *	AUTODETECT, AUTOREPORT $B$rGQ;_(B
 *	$B%5!<%P<uM}3NG'$N$?$a$NJ8;zNs(B  BUSY, O.K. $B$r(B define
 * 1999 Jun 16th
 *	Connection_Destructor $B$r?7@_(B
 * 1999 Jul 9th
 *	MultiSocket_Connection $B$r?7@_(B
 * 1999 Nov 9th
 *	Connection_SelectWithTime $B$rDI2C(B
 * 2001 Aug 13th
 *	c++ $B$+$i8F$S=P$;$k$h$&$K(B
 * 2001 Sep 30th
 *	DOUBLE $B7?$X$NBP1~(B
 * 2008 Feb 13th
 *	Changed into c++
 */

#ifndef __IIRLIB_CONNECTION_H__
#define __IIRLIB_CONNECTION_H__

#include <cstdio>
#include <cstdlib>
#include <string>
#include <fcntl.h>
#include <cerrno>
#include <sys/stat.h>
#include <ctime>
#include <time.h>
#include <ctype.h>

#ifdef WIN32
#include "stdafx.h"
#include <winsock2.h>
#include <io.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

#include "glib.h"
#include "iirlib.h"

#ifdef WIN32
#define Win32_Winsock
#include <windows.h>
#include <io.h>
#else
#include <unistd.h>
#endif
                              

#define	CONNECTION_SUCCESS      (1)
#define	CONNECTION_FAIL         (-1)
#define	CONNECTION_BYTE         (1<<2)
#define	CONNECTION_STRING       (1<<3)
#define	CONNECTION_FIFO         (1<<4)
#define	CONNECTION_SOCKET       (1<<5)
#define	CONNECTION_FIFO_REVERSE (1<<6)
#define	CONNECTION_FIFO_NORMAL  (1<<7)
#define	CONNECTION_SELECT       (1<<8)
#define	CONNECTION_SERVER       (1<<9)
#define	CONNECTION_MSERVER      (1<<10)
#define	CONNECTION_CLIENT       (1<<11)
#define	CONNECTION_INTEGER      (1<<12)
#define	CONNECTION_WORD         (1<<13)
#define	CONNECTION_STREAM       (1<<14)
#define	CONNECTION_LISP         (1<<15)
#define	CONNECTION_C            (1<<16)
#define	CONNECTION_DOUBLE       (1<<17)
#define	CONNECTION_TLVECTOR     (1<<18)
#define	CONNECTION_MULTICLIENT  (1<<19)

#define	CONNECTION_BUSY         "BUSY"
#define	CONNECTION_OK           "O.K."

#if 0
// void *memset(void* p, int b, size_t n)   
// set n bytes to b return p
#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)

// void *memmove (void *p, const void *q, size_t n)   
// copy n bytes from q to p and return p
#define bcopy(b1,b2,len) (memmove((b2), (b1), (len)), (void) 0)

// The fsync(fildes) function moves all modified data and 
// attributes of the file descriptor (fildes) to a storage device
#define fsync(fd) 0 
#endif



class Connection
{
public:
	Connection(int type);
	virtual ~Connection();  

	int GetType          ();
	int Open             (char *readfile,char *writefile, int port);
	char *GetName        ();
	int SetName          (char *name_str);
	int FdRead           (int fd);
	int FdWrite          (int fd);
	int SetStream        (char *buf);
	int FdRead           ();
	int FdWrite          ();
	FILE *FpRead         ();
	FILE *FpWrite        ();
	FILE *FpRead         (FILE *fp);
	FILE *FpWrite        (FILE *fp);
	int Close            ();
	int MaxFD            (int max);
	int Select           ();
	int SelectWithTime   (int msec);
	int FDSet            (fd_set *readfds);  
	int FDISSET          (fd_set *readfds); 
	int DebugPrintFD     ();
	int Receive          (int typ, gpointer dat);
	int Send             (int typ, gpointer dat);
	int MultiClientClose ();
	int SocketClientOpen (char *serverhost,int port);
	int MultiClientOpen  (char *serverhost,int port);
	int PrintAll         ();
	int my_receive       (int fd, void *buf, int len);
	int my_send          (int fd, void *buf, int len);
	int read_integer     (int fd, int *data);
	int Send_Integer     (gpointer data);
	int Send_tlVector    (tlVector_t *tlvec);
	int Receive_tlVector (tlVector_t *tlvec);

private:
	char        name[64];
	char        readfile[128];
	char        writefile[128];
	int         port;        // this is not used in case of Socket
	int         type;        // fifo or socket
	int         fd_read;
	int         fd_write;
	FILE        *fp_read;
	FILE        *fp_write;
	char        *stream;     // Buffer for CONNECTION_STREAM
};

#endif /* __IIRLIB_CONNECTION_H__ */

