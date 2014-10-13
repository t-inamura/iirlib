// --*- C++ -*--
/*
 *	IIRLIB  Multi Socket Connection Modules
 *      
 *	Last modified on 2008 Feb 13th 	by Tetsunari Inamura
 *
 *	Copyright (c) Tetsunari Inamura 1998-2008.
 *	All Rights Reserved.
 */

/*
 * 1998.Dec.8  by Inamura and Naka
 *	$B%*%V%8%'%/%H;X8~$N%W%m%0%i%`4D6-$X(B
 *	fifo $B$d(B socket $B$K4X$9$k%W%m%0%i%`(B connection.c
 * 1998 Dec 10th
 *	$BJV$jCM$O$9$Y$F(B FAIL or SUCCESS $B$@$C$?$,!$JQ?t;2>H$N>l9g$H!$(BConstructor
 *	$B$K8B$j!$JV$jCM$r$=$N$^$^;H$&;v$K$9$k(B
 * 1998 Dec 11th
 *	connection $B7?$r(B Connection $B7?$K(B
 *	$B%m!<%+%kJQ?t(B connect $B$r(B connection $B$K!%(Bconnect $B$H$$$&L?Na$,$"$k(B!!
 * 1998 Dec 16th
 *	Send, Receive $B$K(B CONNECTION_INTEGER $B7?$r$D$1$k(B
 * 1999 Jan 5th
 *	CONNECTION_WORD $B7?$r:n$k(B.STRING $B7?$O<B$O:#$^$G(B Word $B7?$@$C$?$N$rD>$9(B
 * 1999 Jan 15th
 *	CONNECTION_STREAM $B7?$r:n$k(B.
 * 1999 Jan 19th
 *	STRING$B7?$GAw$k;~$N8B3&$,(B 128 $BJ8;z$J$N$O!$(Bconnection.c $B$N$;$$$G$"$k$H(B
 *	  $B5$$,IU$/!%(Bmagic number $B$r;H$o$J$$$h$&$KJQ99(B
 * 1999 Jan 20th
 *	clisp $B$G$NJ8;zNs$O(B \0 $B$,$J$$!%J8;zNs$N=*N;%3!<%I$OB8:_$7$J$$!%(B
 *	 $B$3$3$N0c$$$r9MN8$7$F!$(BSend, Receive $B$J$I$r99?7(B
 * 1999 Jan 21st
 *	$B>e$NLdBj$KBP=h$9$k$?$a!$(BCONNECTION_LISP $B7?$r?7@_(B
 * 1999 Jan 22nd
 *	port $BHV9f$KBP$7$F(B, htons $B$r;H$&$h$&$K2~NI(B
 * 1999 Jan 23rd
 *	STRING$B7?$N(B send $B$N;~(B,$B%@%$%l%/%H$KJ8;zNs$r=q$$$F%3%^%s%IH/9T$7$FNI$$(B
 *	  $B;v$K$9$k$?$a$K!$$$$C$?$sFbMF$r%3%T!<$7$F$+$i(B send $B$9$k$h$&$K2~NI(B
 * 1999 Jan 24th
 *	STREAM $B$N;~(B, $B%A%'%C%/$N$?$a$KD9$5$r(B2$B2sAw$k!%0c$&;~$O%(%i!<(B
 * 1999 Jan 30th
 *	$B5\K\$N%=!<%9$r=,$C$F!$(BSocket $B$r(B shutdown $B$7$F$+$iJD$8$k(B
 * 1999 Feb 15th
 *	$B%^%k%A%"%/%;%W%7%g%s$N%5!<%P!<$N$?$a$N3HD%(B
 *	$B%5!<%P$N>l9g$K$O!$%[%9%HL>$r;XDj$;$:$K!$(BINADDR_ANY $B$r;H$&$h$&$K(B
 * 1999 Feb 17th
 *	$B%^%k%A%"%/%;%9$N%5!<%P$K@\B3$9$k%/%i%$%"%s%H$O=i4|@_Dj$NItJ,(B,
 *	$B@\B3$r<WCG$9$kItJ,$G!$6&DL$N<jB3$-$r$9$kI,MW$,$"$k!%(B
 *	$B$J$N$G!$(BConnection_Open $B$N:]$K(B CONNECTION_MULTICLIENT $B$G$=$l$r;XDj$9$k(B
 * 1999 Feb 23rd
 *	JIS $B$G=q$+$l$F$$$?$N$G!$(BEUC $B$K$9$k!%(BJIS $B$@$H(B GUI $B$H7Q$i$J$+$C$?(B
 *	$B$J$<$J$N$+$OJ,$+$i$J$$!)!)!)IT;W5D(B
 * 1999 Mar 9th
 *	$BDL?.$H$OD>@\4X78$N$J$$!$(Bstrmember $B$J$I$r(B mylib.c $B$K0\F0(B
 * 1999 Apr 24th
 *	$BJ8;zNs$rAw?.$9$k:]$K!$Aj<j$,(B LISP $B$@$m$&$H(B C $B$@$m$&$HF1$87A<0$GAw$k$h$&$K(B
 *	"abc" $B$J$i!$(B'a' 'b' 'c' '\n' $B$rAw$k;v!%(BC$B$N;~$O(B '\0' $B$K$J$k$h$&$K2C9)$9$k(B
 *	$B$h$C$F!$(BAutoReport AutoDetect $B$rGQ;_$9$k(B
 * 1999 Arp 27th
 *	BYTE $B7?$NDL?.$N:]$K$b(B,$BFI$_$3$\$7$r9MN8$7$FDL?.$9$k$h$&$K$9$k(B
 * 1999 Apr 28th
 *	$B%5!<%P$+$i$O@\B3$r@Z$i$J$$(B for MultiSocket_Close
 * 1999 May 11th
 *	AutoReport, AutoDetect $BL5$7$G0BDj$7$F$$$k$N$G!$(Bpurge $B$9$k(B
 * 1999 Jun 16th
 *	Connection_Destructor $B$r?7@_(B
 * 1999 Jul 9th
 *	MultiSocket_Connection $B$r?7@_(B
 * 1999 Nov 9th
 *	Connection_SelectWithTime $B$rDI2C(B
 * 1999 Nov 25th
 *	Windows $B$H$N6&B8$r$O$+$k(B
 * 2001 Sep 30th
 *	DOUBLE $B7?$X$NBP1~(B
 * 2002 Jan 30th
 *	TL_FAIL, TL_TRUE $B$X%7%U%H(B
 * 2002 Jan 31st
 *	CONNECTION_TLVECTOR $B$KBP1~(B
 * 2008 Feb 13th
 *	Changed into C++
 */

#include "iirlib.h"
#include "multisocket.h"
#include "connection.h"

#define	SOCKET_ONLY




// $B?7@_(B : 1999 Feb 15th
// $BF0:n(B : $B%^%k%A%"%/%;%9%5!<%P$N9=B$BN@8@.(B
// $BF~NO(B : *str		: $B9=B$BN$NL>A0(B
// $BF~NO(B : port		: $B;HMQ$9$k%]!<%H(B
MultiSocket::MultiSocket (char *str, int ports)
{
  
  int			i, f=1;
  struct sockaddr_in	me;
  
  strcpy (name, str);
  port = ports;
  for (i=0; i<MAX_MULTI_SOCKET; i++)
    connection[i] = NULL;

  tl_message ("%s: port=%d", name, port);

  socket_fd = socket( AF_INET, SOCK_STREAM, 0 );                             
  if (socket_fd == -1)
    {
      perror( "Multi server socket");
      return;
    }
  setsockopt (socket_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&f, sizeof f);     

  bzero((char *)&me, sizeof(me));                                         
  me.sin_family = AF_INET;
  me.sin_addr.s_addr = INADDR_ANY;		// server is itself
  me.sin_port = htons(port);

  if (bind( socket_fd, (const struct sockaddr *)&me, sizeof(me) ) == -1)
    {
      perror ("Multi server:bind :");
      return;
    }
  listen( socket_fd, MAX_MULTI_SOCKET );
}



// $B?7@_(B : 1999 Feb 15th
// $BJQ99(B : 1999 May 12th
//	  $B<u$1IU$1<uM}$+!$(BBusy $B$+$r(B 1byte $B$GAw$C$F$$$?$,!$(BEusLisp $B$,FI$a$J$$$h$&(B
//	  $B$J$N$GJ8;zNs$G$3$l$rAw?.$9$k;v$K$9$k(B
// $BJQ99(B : 1999 Jul 7th : $B@\B3$5$l$?HV9f$rJV$9$h$&$K$9$k(B
// ----
// $BF0:n(B : $BESCf$K@\B3MW5a$7$F$-$?(B client $B$KBP$7$F(B,accept $B$9$k(B
// $BJVCM(B : $B@\B3(B OK $B$J$i$P!"@\B3$5$l$?(B MultiSocket $BFbIt$G$NHV9f$rJV$9(B
//	: $B@\B3(B FAIL $B$J$i(B TL_FAIL
int MultiSocket::Accept()
{
  int			s, i, debug=0;
  struct sockaddr_in	caddr; // $B%/%i%$%"%s%HB&$N%"%I%l%9>pJs$rF~$l$k$H$3$m(B
  char			new_name[MAX_STRING], com[10];
  Connection		*tmpconnection;

  if(debug) tl_message ("%s : start", name);
  socklen_t len = (socklen_t)sizeof(caddr);
  s = accept (socket_fd, (struct sockaddr *)&caddr, &len);
  if(debug) tl_message ("Socket accepted");
  for( i=0; i<MAX_MULTI_SOCKET; i++ )
    {
      if (i==MAX_MULTI_SOCKET-1 && connection[i]!=NULL)
	{
	  // $B$b$&$$$C$Q$$$G@\B3IT2DG=$G$"$k;v$r8~$3$&$KCN$i$;$k(B
	  tl_message ("Too busy");
	  strcpy (com, CONNECTION_BUSY);
	  connection[i]->my_send (s, com, 4);
	  // Lisp, C $B6&DL;EMM$GJ8;zNs=*N;$r0UL#$9$k(B '\n' $B$rAw?.(B
	  com[0] = '\n';
	  connection[i]->my_send (s, com, 1);
	  return FALSE;
	}
      if (connection[i]==NULL)
	{
	  tmpconnection = new Connection( CONNECTION_SOCKET );
	
	  tmpconnection->FdRead(s);
	  tmpconnection->FdWrite(s);
	  tmpconnection->FpRead(fdopen( s,"r+" ));// Read&Write
	  tmpconnection->FpWrite(tmpconnection->FpRead());
	  if(debug) tl_message ("System allows this plugin : ");
	  strcpy( com, CONNECTION_OK );
	  tmpconnection->my_send (s, com, 4);	// $B<u$1IU$1(B OK $B$N?.9f$r8~$3$&$KCN$i$;$k(B
	  // Lisp, C $B6&DL;EMM$GJ8;zNs=*N;$r0UL#$9$k(B '\n' $B$rAw?.(B
	  com[0] = '\n';
	  tmpconnection->my_send (s, com, 1);
	  if(debug) tl_message ("Now reading client name...");
	  tmpconnection->Receive(CONNECTION_STRING, new_name);
	  if(debug) tl_message ("Client Name = %s", new_name);
	  tmpconnection->SetName(new_name );
	  connection[i] = tmpconnection;
	  return i;
	}
    }
  return FALSE;
}


// $BJQ99(B : 1999 Apr 28th : $B%5!<%P$+$i$O@\B3$r@Z$i$J$$(B
// $B?7@_(B : 1999 Feb 16th
// $BF0:n(B : $B%^%k%A%"%/%;%9$N%5!<%P$GF0E*$K@\B3$r@Z$k(B
int MultiSocket::Close(int no)
{
  
  if (no<0 || no>=MAX_MULTI_SOCKET)
    {
      tl_warning ("No such no.%d", no);
      return FALSE;
    }
  connection[no] = NULL;
  return TRUE;
}


// $B?7@_(B : 1999 Feb 16th
// $BF0:n(B : $B%^%k%A%"%/%;%9$N$?$a$N(B fd $B$r(B max_fd $B$HHf3S$7$F!$(Bmax_fd $B$rJV$9(B
// $BF~NO(B : *mscoket	: $B@\B3$,@.N)$7$F$$$kJ,$N%m%\%C%H$H$NDL?.O)(B
// $BF~NO(B : max		: $B8=CJ3,$G$N0lHVBg$-$$(B fd $BCM(B
int MultiSocket::MaxFD(int max)
{
  int		i, max_fd = max;
  int		debug = 0;

  if(debug) tl_message ("%s start\n", name);
  for( i=0; i<MAX_MULTI_SOCKET; i++ )
    {
      if(connection[i]!=NULL)
	{
	  if(connection[i]->FdWrite() > max_fd)
	    max_fd = connection[i]->FdWrite();
	}
    }
  if( socket_fd > max_fd )
    max_fd = socket_fd;
  if(debug) tl_message ("fd change %d -> %d", max, max_fd);
  return max_fd;
}


// $B?7@_(B : 1999 Feb 16th
// $BF0:n(B : $B@\B3:Q$_$N(B Connection $B$KBP$9$k(B FD_SET $B$*$h$S!$?7$7$$@\B3MW5a$N(B
//        $B2DG=@-$N$"$k(B fd $B$X$N(B FD_SET
int MultiSocket::FDSet(fd_set *readfds )
{
  int		i, debug=0;
  
  
  if(debug) tl_message ("%s : start", name);
  for (i=0; i<MAX_MULTI_SOCKET; i++)
    {
      if( connection[i]!=NULL )
	{
	  FD_SET( connection[i]->FdRead(), readfds );
	  if(debug) tl_message ("FD_SET(%d)", connection[i]->FdRead());
	}
    }
  FD_SET(socket_fd, readfds);
  if(debug) tl_message ("FD_SET(%d)", socket_fd);
  return TRUE;
}


// $B?7@_(B : 1999 Feb 16th
// $BJVCM(B : FAIL $B$J$i2?$b(B select $B$5$l$F$J$$;v$K$J$k(B
// $BF~NO(B : *msocket	: $B%^%k%A%=%1%C%H9=B$BN(B
// $BF~NO(B : *readfds	: fd_set $B9=B$BN$X$N%]%$%s%?(B,$B$3$3$r$$$8$k(B
// $B=PNO(B : *result	: select $B$,8!=P$7$?(B MultiSocket $BFbIt$G$NHV9f(B
//	:		: $B?7$7$$@\B3MW5a$G$"$l$P!$(BTL_FAIL
int MultiSocket::FDISSET(fd_set *readfds, int *result)
{
  int		i;
  
  for (i=0; i<MAX_MULTI_SOCKET; i++)
    {
      if (connection[i]!=NULL)
	{
	  if (FD_ISSET( connection[i]->FdRead(), readfds))
	    {
	      *result = i;
	      return TRUE;
	    }
	}
    }
  if( FD_ISSET(socket_fd, readfds ) )
    {
      *result = MULTI_SOCKET_NEW_CONNECTION;
      return TRUE;
    }
  return FALSE;
}


// $B?7@_(B : 1999 Jul 9th
// $BF0:n(B : MultiSocket $B$G4IM}$7$F$$$k(B Connection $B9=B$BN$r;2>H$9$k(B
// $BF~NO(B : *msocket	: $BBP>]$N%^%k%A%=%1%C%H9=B$BN(B
// $BF~NO(B : no		: $BBP>]$N(B Connection $B9=B$BN$NHV9f(B
Connection *MultiSocket::GetConnection (int no)
{
  if (no<0 || no>=MAX_MULTI_SOCKET)
    {
      tl_warning ("No.%d is too large. MAX is %d!", no, MAX_MULTI_SOCKET);
      return NULL;
    }
  if (!(connection[no]))
    {
      tl_warning ("Target connection Pointer is NULL!");
      return NULL;
    }
  return connection[no];
}
