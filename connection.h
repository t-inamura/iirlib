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
 *	オブジェクト指向のプログラム環境へ
 *	fifo や socket に関するプログラム
 * 1998 Dec 10th
 *	返り値はすべて FAIL or SUCCESS だったが，変数参照の場合と，Constructor
 *	に限り，返り値をそのまま使う事にする
 * 1998 Dec 11th
 *	connection 型を Connection 型に
 *	ローカル変数 connect を connection に．connect という命令がある!!
 * 1998 Dec 16th
 *	Send, Receive に CONNECTION_INTEGER 型をつける
 * 1999 Jan 15th
 *	CONNECTION_STREAM 型が誕生, 構造体に stream の開始ポインタを追加
 * 1999 Jan 19th
 *	STRING型で送る時の限界が 128 文字なのは，connection.c のせいであると
 *	  気が付く．magic number を使わないように変更
 * 1999 Jan 21st
 *	CONNECTION_LISP 型を新設
 * 1999 Feb 16th
 *	CONNECTION_AUTODETECT : サーバが，クライアントの type を受けとるため
 *	CONNECTION_AUTOREPORT : クライアントが,サーバに type を報告するため
 *	マルチアクセスのための関数群を登録
 * 1999 Mar 21st
 *	新関数をいろいろ増やす
 * 1999 May 12th
 *	AUTODETECT, AUTOREPORT を廃止
 *	サーバ受理確認のための文字列  BUSY, O.K. を define
 * 1999 Jun 16th
 *	Connection_Destructor を新設
 * 1999 Jul 9th
 *	MultiSocket_Connection を新設
 * 1999 Nov 9th
 *	Connection_SelectWithTime を追加
 * 2001 Aug 13th
 *	c++ から呼び出せるように
 * 2001 Sep 30th
 *	DOUBLE 型への対応
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

