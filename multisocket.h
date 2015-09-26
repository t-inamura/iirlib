// --*- C++ -*--
/*
 *	IIRLIB  Multi Socket Connection Modules
 *      
 *	Last modified on 2008 Feb 13th 	by Tetsunari Inamura
 *
 *	Copyright (c) Tetsunari Inamura 1998-2015.
 *	All Rights Reserved.
 */

#ifndef __MULTISOCKET_H__
#define __MULTISOCKET_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <time.h>

#ifdef WIN32
#include <io.h>
#include <winsock2.h>
#else

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

#include "glib.h"
#include "connection.h"

#define MAX_MULTI_SOCKET    3
#define NAMESIZE            64
#define MULTI_SOCKET_NEW_CONNECTION	(-1)


class  MultiSocket
{
public:
	MultiSocket                  (char *str, int port);
	int        Accept            ();
	int        Close             (int no);
	int        MaxFD             (int max );
	int        FDSet             (fd_set *readfds);
	int        FDISSET           (fd_set *readfds, int *result);
	int        DebugPrintFD      ();
	Connection *    GetConnection (int n);

private:
	char       name[NAMESIZE];
	int        socket_fd;
	int        port;
    Connection *connection[MAX_MULTI_SOCKET];        // sequence of Connection instances};

};

#endif
