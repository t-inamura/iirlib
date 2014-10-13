// --*- C++ -*--
/*
 *  IIRLIB C Connection Modules
 *      
 *  Last modified on 2008 Feb 13th by Tetsunari Inamura
 *
 *	Copyright (c) Tetsunari Inamura 1998--2008.
 *	All Rights Reserved.
 */

#include "connection.h"
#include "stdio.h"                         

// 変更 : 1999 Jun 16th : C or Lisp を強制的に選択させる部分をなくした
// Input : type		: Socket なのか，FIFO なのか？
// 返値 : *connection	: 生成した connection 構造体へのポインタ
//			: 失敗なら NULL

// Constructor: Constructor for Connection class
// Only defines the initial values for the object 
// parameter: type, defines the type of the socket
Connection::Connection (int target_type)
{
#if 0
  d_print( "[Constructor] type : %d (", type );
  print_bit( type );
  d_print( ")\n" );
#endif

  if( !((target_type & CONNECTION_FIFO) || (target_type & CONNECTION_SOCKET )) )
    {
      tl_message ("You must decide Socket or FIFO");
      return;
    }
  
  type     = target_type;
  fd_read  = FALSE;
  fd_write = FALSE;
  fp_read  = NULL;
  fp_write = NULL;
  stream   = NULL;
  sprintf(name, "NoName" );
}

//This function returns the type of the connection
// there is no parameter passed for this
int Connection::GetType()
{
  //chekcing for good object goes where it is created
  return type;
}


//This function returns the name of the connection
// there is no parameter passed for this
char * Connection::GetName()
{
  //chekcing for good object goes where it is created
  return name;
}


// Input : *connection	: 使用するコネクション構造体へのポインタ
// Input : *name_str	: 命名する名前の文字列
int Connection::SetName(char *name_str)
{
  //chekcing for good object goes where it is created
  int		debug=0;
  strcpy( name, name_str );
  if(debug) tl_message ("%s",GetName());
  return CONNECTION_SUCCESS;
}



// Input : *connection	: 使用するコネクション構造体へのポインタ
// Input : *buf		: 使用するバッファのポインタ
int Connection::SetStream (char *buf)
{
  //chekcing for good object goes where it is created
  stream = buf;
  return CONNECTION_SUCCESS;
}




// Added  : : 1999 Feb 16th
// 動作 : MultiSocket の奴をまねして作った
int Connection::MaxFD( int max )
{
  //chekcing for good object goes where it is created
  int		max_fd = max;
  
  if(fd_read > max_fd )
    max_fd = fd_read;
  
  return max_fd;
}

// 変更 : 1999 Apr 24th	: AutoReport の廃止
// Added  : : 1999 Feb 17th
// 動作 : クライアントとして Socket を普通に作る
// 注意 : Open & MultiClientOpen から同時使用されている
int Connection::SocketClientOpen(char *serverhost, int port_01)
{
  struct hostent	*servhost;      
  struct sockaddr_in	server;
  int			s, debug=1;
    
  if(debug) tl_message ("%s: SocketClient %s:%d", GetName(), serverhost, port_01);
  port = port_01;
  servhost = gethostbyname(serverhost);
  bzero( (char *)&server, sizeof(server) );

  server.sin_family = AF_INET;
  server.sin_port = htons(port);
  bcopy( servhost ->h_addr, (char *)&server.sin_addr,servhost->h_length);
  s = socket( AF_INET, SOCK_STREAM, 0 );
  connect( s, (const struct sockaddr *)&server, sizeof(server) );
  fd_read = s;
  fd_write = s;
  fp_read = fdopen( fd_read, "r+" );	// Read&Write
  fp_write = fp_read;
  if( fd_read ==FALSE || fp_read ==NULL || fd_write==FALSE || fp_write==NULL )
    {
      tl_warning ("reverse  Error !!");
      return CONNECTION_FAIL;
    }
  return CONNECTION_SUCCESS;
}

/*---------------------------------------------------------------------------*/
// 変更 : 1999 Apr 24th : 文字列を受けとる際に Lisp/C の区別はしないようにする
//      : 1999 Apr 26th : BYTE 型の時にも読みこぼしをちゃんと考慮するようにする
//      : 2001 Sep 30th : DOUBLE 型への対応
// Input : *connection	: 使用するコネクション構造体へのポインタ
// Input : type		: 1byte or string? , Select をするかしないか
// 出力 : *data		: 読み込み結果
// 返値 :		: SUCCESS or FAIL
// 注意 : STRING 型で読み込む時は，'\n' がない状態の文字列を返す
// 注意 : STREAM 型の時は，*data には長さが入る．受けとるバッファは
//        Connectoin_SetStream で設定しておく
/*---------------------------------------------------------------------------*/
int Connection::Receive (int type, gpointer data)
{
  int		ret, debug=0;
  char		word[MAX_STRING];

  // if (!connection) return CONNECTION_FAIL;
  if (type & CONNECTION_SELECT)
    {
      if (Select( )==CONNECTION_FAIL)
	return CONNECTION_FAIL;
    }
      
  if( type & CONNECTION_BYTE )
    {
      // 通信できていない場合は Retry するようにする : 1999 Apr 26th
      int		fd;
      fd = FdRead( );
      do {
	ret = my_receive (fd, data, sizeof(char));
	if (ret==-1)
	  {
	    tl_warning ("%s : STRING : Error(%d)!", GetName(), errno);
	    return CONNECTION_FAIL;
	  }
      }
      while( ret!=1 );
    }
  else if( (type & CONNECTION_STRING) )
    {
      // C で文字列を受けとる時は最後の '\n' を '\0' に変換する
      int		fd, ret, p, debug=0;
      fd = FdRead( );
      for (p=0;;p++)
	{
	  do
	    {
	      ret = my_receive( fd, (gpointer)((char *)data+p), 1 );
	      if (ret==FALSE)
		tl_message ("%s : STRING : Error!", GetName());
	      if(debug) tl_message ("%c(%d)::", ((char *)data)[p], ((char *)data)[p]);
	    }
	  while( ret!=sizeof(char) );
	  if ( ((char *)data)[p]=='\n' ) break;
	  if (ret!=sizeof(char) || p>=MAX_STRING)
	    {
	      tl_message ("%s : STRING : OverFlow Error!", name );
	    }
	}
      ((char *)data)[p] = '\0';
    }
  else if( type & CONNECTION_WORD )
    {
      FILE	*fp;
      fp = FpRead();
      if( fgets( word, MAX_STRING, fp )==NULL )
	{
	  tl_message ("WORD : Error!");
	  return CONNECTION_FAIL;
	}
      sscanf (word, "%s", (char *)data);
    }
  else if (type & CONNECTION_INTEGER)
    {
      int		fd, value=0;
      fd = FdRead();
      ret = my_receive( fd, &value, sizeof(int) );
      if( ret!=sizeof(int) )
	{
	  tl_message ("INTEGER : Error!");
	  return CONNECTION_FAIL;
	}
      *( (int *)data ) = value;
    }
  else if (type & CONNECTION_DOUBLE)
    {
      int		fd;
      double		value=0.0;
      fd = FdRead ();
      ret = my_receive (fd, &value, sizeof(double));
      if (ret!=sizeof(double))
	{
	  tl_warning ("<DOUBLE> : ret = %d ", ret);
	  return FALSE;
	}
      *( (double *)data ) = value;
    }
  else if (type & CONNECTION_TLVECTOR)
    Receive_tlVector ((tlVector_t *)data);   //KEEP AN EYE OVER HERE !!!!

  else if (type & CONNECTION_STREAM )
    {
      // STREAM 型の場合，最初に int 型で長さを受けとってから，データをもらう
      int		fd, length=0, p, tmp=5;
      fd = FdRead();
      if(stream == NULL )
	{
	  tl_message ("STREAM :%s : Point is NULL!", GetName());
	  return CONNECTION_FAIL;
	}
      read_integer (fd, &tmp);
      tl_message ("Read Check 1 : length = %d", tmp);
      read_integer (fd, &length);
      tl_message ("Read Check 2 : length = %d", length);
      fsync(fd);
      if( tmp!=length )
	{
	  tl_message ("STREAM :%s : length mismatch %d!=%d", GetName(), tmp, length);
	  ret = my_receive( fd, &tmp, sizeof(int) );
	  return CONNECTION_FAIL;
	}
      tl_message ("STREAM :%s : length = %d", GetName(), length );
      *((int *)data) = length;
      if( length<=0 )
	{
	  tl_message ("STREAM :%s : length error!", GetName());
	  return CONNECTION_FAIL;
	}
      for (p=0; p<length; p++)
	{
	  ret = my_receive( fd, (stream)+p, 1 );
	  if (ret!= 1)
	    {
	      tl_warning ("STREAM :%s : read error!", GetName());
	      return CONNECTION_FAIL;
	    }
	}
      if(debug) tl_message ("STREAM :%s (2): length = %d", GetName(), length);
    }
  else
    {
      tl_message ("%s: Error! Set Send Type.(%d)", GetName(), type);
      return CONNECTION_FAIL;
    }
  return CONNECTION_SUCCESS;
}


// Added  : : 1999 Feb 17th
// Memo  : To establish a connection to Multi-Server
// Input : *serverhost		: hostname of server
// Input : port			: port no.
int Connection::MultiClientOpen( char *serverhost,int port )
{

  char		com[10];

  if (SocketClientOpen( serverhost, port )==CONNECTION_FAIL)
    {
      perror( "<MultiClinetOpen> socket client open");
      return CONNECTION_FAIL;
    }
  // Receiving BUSY / O.K.
  Receive( CONNECTION_STRING, com );
  if (!strcmp(com, CONNECTION_BUSY))
    {
      Close();
      tl_message ("Server is too busy, connection refused");
      return CONNECTION_FAIL;
    }
  else if( strcmp(com, CONNECTION_OK) )
    {
      tl_warning ("!SERIOUS WARNING! Server ought to send BUSY or O.K.  But {%s} came.", com);
      MultiClientClose();
      return CONNECTION_FAIL;
    }
  // Send a name to server after connection
  Send (CONNECTION_STRING, name);
  return CONNECTION_SUCCESS;
}



int Connection::Open (char *readfile, char *writefile, int target_port)
{
  int		type, debug=1;
  char		filename[MAX_STRING];

  if(debug) tl_message ("Start!");
  type = GetType ();
  if(debug) tl_message ("Start<2>!");
  
  // FIFO で，NORMAL の順番で Open
  if( (type & CONNECTION_FIFO) &&  (type & CONNECTION_FIFO_NORMAL) )
    {
      if(debug) tl_message ("%s: FIFO Normal",GetName() );
      strcpy( filename, readfile );
      fd_read = open( filename, O_RDONLY );
      fp_read = fdopen( fd_read, "r" );

      strcpy( filename, writefile );
      fd_write = open( filename, O_WRONLY );
      fp_write = fdopen( fd_write, "w" );

      if(fd_read ==FALSE || fp_read ==NULL || fd_write==FALSE ||fp_write==NULL )
	{
	  if(debug) tl_message ("non_reverse  Error !!");
	  return CONNECTION_FAIL;
	}
    }
  

  // FIFO で，Reverse の順番で Open
  else if( (type & CONNECTION_FIFO) && (type & CONNECTION_FIFO_REVERSE) )
    {
      if(debug) tl_message ("%s:FIFO Reverse", GetName());
      strcpy( filename, writefile );
      fd_write = open( filename, O_WRONLY );
      fp_write = fdopen( fd_write, "w" );

      strcpy( filename, readfile );
      fd_read = open( filename, O_RDONLY );
      fp_read = fdopen( fd_read, "r" );

      if (fd_read ==FALSE || fp_read ==NULL || fd_write==FALSE || fp_write==NULL )
	{
	  if(debug) tl_message ("reverse  Error !!");
	  return CONNECTION_FAIL;
	}
    }

  // Socket で，サーバー側の Open
  // Multi Access ではなく，1vs1の普通のサーバ
  else if ( (type & CONNECTION_SOCKET) && (type & CONNECTION_SERVER) )
    {
      struct sockaddr_in	me;
      int			s_waiting, s;

      if(debug) tl_message ("%s:Socket Server %s:%d", GetName(), readfile, port );
      port = target_port;
      bzero((char *)&me, sizeof(me));
      me.sin_family = AF_INET;
      me.sin_port = htons(port);
      me.sin_addr.s_addr = INADDR_ANY;
      s_waiting = socket(AF_INET, SOCK_STREAM,0); 
      bind (s_waiting, (const struct sockaddr *)&me,sizeof(me)); 
      listen (s_waiting, 1);  
      s = accept (s_waiting, NULL, NULL);
      close (s_waiting);
      fd_read  = s;
      fd_write = s;
      fp_read  = fdopen( fd_read, "r+" );// Read&Write
      fp_write = fp_read;
      if( fd_read==-1 || fp_read==NULL || fd_write==-1 || fp_write==NULL )
	{
	  if(debug) tl_message ("reverse  Error !!");
	  return CONNECTION_FAIL;
	}
    }

  // マルチサーバーへのクライアント接続
  else if( type & CONNECTION_MULTICLIENT )
    // readfile にサーバホスト名が入っている
    MultiClientOpen (readfile, target_port);
  
  // Socket で，クライアント側の Open
  else if ( (type & CONNECTION_SOCKET) && (type & CONNECTION_CLIENT) )
    {
      if(debug) tl_message ("%s:Socket Server %s:%d", GetName(), readfile, target_port);
      SocketClientOpen (readfile, target_port);
    }
  
  // Open の時のオプション，Type の指定エラー
  else {
    if(debug) tl_warning ("%s:Error !!", GetName() );
    return CONNECTION_FAIL;
  }
  return CONNECTION_SUCCESS;
}


int Connection::FdRead()
{
  // if( connection==NULL ) return FALSE;
  return fd_read;
}

int Connection::FdRead(int value)
{
  // if( connection==NULL ) return FALSE;
  fd_read = value;
  return CONNECTION_SUCCESS;
}

int Connection::FdWrite()
{
  //if( connection==NULL ) return FALSE;
  return fd_write;
}
int Connection::FdWrite(int value)
{
  //if( connection==NULL ) return FALSE;
  fd_write = value;
  return CONNECTION_SUCCESS;
}

FILE * Connection::FpRead()
{
  // if( connection==NULL ) return NULL;
  return fp_read;
}

FILE * Connection::FpRead(FILE *fp)
{
  // if( connection==NULL ) return NULL;
  fp_read=fp;
  return fp;
}



FILE * Connection::FpWrite(  )
{
  // if( connection==NULL ) return NULL;
  return fp_write;
}
FILE * Connection::FpWrite(FILE *fp  )
{
  // if( connection==NULL ) return NULL;
  fp_write=fp;
  return fp;
}



int Connection::FDISSET( fd_set *readfds )
{
	
  //  if( connection==NULL ) return 0;
  if( FD_ISSET( FdRead(), readfds ) )
    return 1;
  else return 0;
}




// Input : *connection	: 使用するコネクション構造体へのポインタ
// 返値			: SUCCESS or FAIL
int Connection::Select ()
{
	

  fd_set	readfds;   
  struct	timeval tv;          //WHERE IT IS
  int 	        fd, nfds=0;

  // if( connection==NULL ) return CONNECTION_FAIL;
  fd = FdRead();
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  FD_ZERO(&readfds);
  FD_SET(fd, &readfds);
  nfds = select( fd+1, &readfds, NULL, NULL, &tv);
  if( nfds!=0 ) return CONNECTION_SUCCESS;
  else return CONNECTION_FAIL;
}


// Added  : : 1999 Nov 9th
// 動作 : 制限時間付きの select
int Connection::SelectWithTime( int msec )
{
	
  fd_set	readfds;
  struct	timeval tv;             // timeval 
  int 	        fd, nfds=0;

  // if (!connection) return CONNECTION_FAIL;
  fd = FdRead();
  tv.tv_sec = msec / 1000;
  tv.tv_usec = (msec & 1000) * 1000;
  FD_ZERO(&readfds);
  FD_SET(fd, &readfds);
  nfds = select( fd+1, &readfds, NULL, NULL, &tv);
  if( nfds!=0 ) return CONNECTION_SUCCESS;
  else return CONNECTION_FAIL;
}



//---------------------------------------------------------------------------
// 機能 : tlVector をネットワークで受信する
// Input : data		: tlVector へのポインタ (すでにユーザ側で作成のこと)
//---------------------------------------------------------------------------
int Connection::Receive_tlVector (tlVector_t *vec)
{
  int		i, size;
  double	data;
  GArray	*array;

  array = (GArray *)vec;
  // まずはベクトルの長さを受信する
  Receive (CONNECTION_INTEGER, (gpointer)&size);

  for (i=0; i<size; i++)
    {
      Receive (CONNECTION_DOUBLE, (gpointer)&data);
      g_array_append_val (array, data);
    }
  return TRUE;
}




// 変更 : 1999 Apr 24th
//	・この関数を使うのは C プロセスなのでいつでも abc\n という列を出力する
//	  下手に相手が Lisp だから C だから，という小細工はしない方針
// 変更 : 2001 Sep 30th : DOUBLE 型への対応
// 変更 : 2002 Jan 30th : データを gpointer 型にする
//-----------------------------
// Input : *connection	: 使用するコネクション構造体へのポインタ
// Input : type		: 文字列 or 1byte
// Input : *data		: 送信するデータ
// 返値			: SUCCESS or FAIL
// 注意	送信するデータが STRING 型の時は，'\n' を付けてはダメ!!
// 注意	STREAM 型の時は，*data はデータの長さを意味する  (Jan 15th)
//	実際のデータバッファは SetStream で設定
int Connection::Send(int type, gpointer data)
{
  int		ret, debug=0;
  char		string[MAX_STRING];
  
  // tl_return_val_if_fail (connection, "Pointer connection is NULL", FALSE);
  //  tl_return_val_if_fail (data, "Pointer of data is NULL", FALSE);

  if (type & CONNECTION_BYTE)
    {
      int		fd;
      fd = FdWrite();
      ret = my_send (fd, data, sizeof(char));
#if 0
      if( ret!=sizeof(char) )
	{
	  tl_warning ("BYTE : Cannot write");
	  return CONNECTION_FAIL;
	}
      //#else
      //    tl_return_val_if_fail ((ret==sizeof(char)), "BYTE : Connot write", FALSE);
#endif
    }
  else if( (type & CONNECTION_STRING) )
    {
      // C から文字列を送信する場合は最後の '\0' を '\n' に変換して送信する
      char	tmp='\n';
      int		ret, fd, i;
      fd = FdWrite();
      strcpy (string, (char *)data );
      for (i=0; i<MAX_STRING; i++)
	{
	  if( string[i]=='\0' )
	    {
	      do {
		ret = my_send( fd, &tmp, 1 );
		if( ret==-1 ) {
		  tl_warning ("STRING : %s : %s", GetName(), string);
		}
	      }
	      while( ret!=1 );
	      break;
	    }
	  else
	    {
	      do {
		ret = my_send (fd, string+i, 1);
		if( ret==-1 )
		  tl_message ("STRING :%s : Error(%d)!", GetName (), errno );
	      }
	      while (ret!=1);
	    }
	}
    }
  else if (type & CONNECTION_WORD)
    {
      FILE	*fp;
      fp = FpWrite();
      fprintf (fp, "%s", (char *)data);
      fflush( fp );
    }
  else if( type & CONNECTION_INTEGER )
    Send_Integer (data);
  
  else if (type & CONNECTION_DOUBLE)
    {
      int	fd;
      double	value;
      fd = FdWrite ();
      value = *((double *)data);
      ret = my_send (fd, &value, sizeof(double));
      fsync(fd);
      // tl_return_val_if_fail ((ret==sizeof(double)), "DOUBLE : Cannot write", FALSE);
      if (ret!=sizeof(double))
	{
	  tl_warning ("DOUBLE : Fd<%d> : Cannot write (%d)", fd, ret);
	  return CONNECTION_FAIL;
	}
    }
  else if (type & CONNECTION_TLVECTOR)
    Send_tlVector ((tlVector_t *)data);
  else if( type & CONNECTION_STREAM )
    {
      int		fd, length, p;
      fd = FdWrite();
      if( stream==NULL )
	{
	  tl_warning ("STREAM :%s :Point is NULL!", GetName() );
	  return CONNECTION_FAIL;
	}
      length = *(int *)data;
      if(debug) tl_message ("STREAM :%s : length = %d", GetName(), length);
      if( length <=0 )
	{
	  tl_warning ("STREAM :%s :length error !", GetName() );
	  return CONNECTION_FAIL;
	}
      // STREAM の時には，まずデータの長さを送信してから，実際のデータを送る
      if(debug) tl_message ("Check 1: length = %d", length );
      ret = my_send( fd, &length, sizeof(int) );
      fsync(fd);
      if( ret!=sizeof(int) ) tl_message ("write miss..." );
      tl_message ("Check 2: length = %d", length );
      ret = my_send( fd, &length, sizeof(int) );
      fsync(fd);
      if( ret!=sizeof(int) ) tl_message ("write miss..." );
      for( p=0; p<length; p++ )
	{
	  ret = my_send( fd, (stream)+p, 1 );
	  if( ret!=1 )
	    {
	      tl_warning ("STREAM :%s : write error!", GetName());
	      return CONNECTION_FAIL;
	    }
	}
      stream = NULL;
    }
  else
    {
      tl_message ("%s: Error! Set Send Type.(%d)", GetName(), type);
      return CONNECTION_FAIL;
    }
  return CONNECTION_SUCCESS;
}



// Added  : : 1999 Feb 17th
// 動作 : マルチアクセスのサーバへ接続しているクライアントを遮断する場合の関数
// Input : *connection	: 使用するコネクション構造体へのポインタ
int Connection::MultiClientClose()
{
  Send (CONNECTION_STRING, (gpointer)"disconnect");
  Close();
  return CONNECTION_SUCCESS;
}

// 動作 : socket を shutdown してから，close する
// Input : *connection	: 使用するコネクション構造体へのポインタ
int Connection::Close()
{
  int		fd;

  
  fd = FdRead();
  // Socket の場合は shutdown して閉じる
  // 片一方閉じれば，両方閉じた事になる
  if( GetType() & CONNECTION_SOCKET )
    {
      shutdown( fd, 2 );	// 2 : disable for both read and write
      close( fd );
      return CONNECTION_SUCCESS;
    }
  if( close(fd)==-1 )
    {
      tl_message ("Error at fd");
      return CONNECTION_FAIL;
    }
  
  fd = FdWrite();
  if( close(fd)==-1 )
    {
      tl_message ("Error at fd");
      return CONNECTION_FAIL;
    }
  return CONNECTION_SUCCESS;
}




// 動作 : デバック関数，構造体の表示
// Input : *connection		: 構造体へのポインタ
int Connection::PrintAll()
{
  cerr << "[PrintAll] fd_read  = " << FdRead ()  << endl;
  cerr << "           fd_write = " << FdWrite()  << endl;
  cerr << "           fp_read  = " << FpRead ()  << endl;
  cerr << "           fp_write = " << FpWrite()  << endl;
  cerr << "           type     = " << GetType()  << endl;
  return CONNECTION_SUCCESS;
}


// Added  : : 1999 Feb 17th
// 動作 : デバッグ用関数
int Connection::DebugPrintFD( )
{
  int		debug=0;
  
  if(debug) tl_message ("%s -> %d", name, fd_read);
  return CONNECTION_SUCCESS;
}

//////////////////////////////////////////////////////////////
// Added  : : 1999 Nov 24th
// 動作 : Win32 との共存のために Socket send をするサブ関数
// 返値 : (int)		: -1 が Fail
int Connection::my_send (int fd, void *buf, int len)
{
  int	ret;
#ifdef SOCKET_ONLY
  ret = send (fd, buf, len, 0);
#else
  ret = write (fd, buf, len);
#endif
  return ret;
}


// Added  : : 1999 Nov 24th
// 動作 : Win32 との共存のために Socket send をするサブ関数
int Connection::my_receive (int fd, void *buf, int len)
{
  int	ret;
#ifdef SOCKET_ONLY
  ret = recv( fd, buf, len, 0 );
#else
  ret = read( fd, buf, len );
#endif
  return ret;
}

int Connection::read_integer( int fd, int *data )
{
  int		ret, debug=0;

  do
    {
      ret = my_receive( fd, data, sizeof(int) );
      if( ret!=sizeof(int) )
	{
	  if(debug) tl_message ("read miss : ret = %d : data = %d [%d][%d][%d][%d]", ret, *data, data[0], data[1], data[2], data[3] );
	}
    }
  while (ret!=sizeof(int) );

  return 0;
}

/*-------------------------------------------------------------------------*/
// Added  : : 2002 Jan 31st
// 機能 : tlVector (GArray) によって実装されているベクトルを送信する
/*-------------------------------------------------------------------------*/
int Connection::Send_tlVector (tlVector_t *tlvec)
{
  int		length, debug=0;
  GArray	*array;

  array = (GArray *)tlvec;
  if(debug) tlVector_Verify (tlvec);

  // まずはベクトルの長さを送信する
  length = array->len;
  Send(CONNECTION_INTEGER, (gpointer)&length);

  for (unsigned int i=0; i<array->len; i++)
    {
      Send(CONNECTION_DOUBLE, (gpointer)&(g_array_index (array, double, i)));
    }

  return TRUE;
}



/*-------------------------------------------------------------------------*/
// Added  : : 2002 Feb 1st
// 機能 : integer の送信
// Input : data		: 対象となる値へのポインタ (gpointer)
/*-------------------------------------------------------------------------*/
int Connection::Send_Integer (gpointer data)
{
  int		ret, fd, value;
  
  fd = FdWrite();
  value = *((int *)data);
  ret = my_send(fd, &value, sizeof(int));
  fsync(fd);

  if (ret!=sizeof(int))
    {
      tl_warning ("Fd<%d>: Cannot write (%d)", fd, ret);
      return FALSE;
    }
  return TRUE;
}




/////////////////////////////////////////////////////////////////////////
// Added  : 1999 Jun 16th : 今までなかったのか !?
// 動作 : Connection 型の構造体を Destructor する
Connection::~Connection()        //NOT A DESTRUCTOR
{
  // 通信路が開いているかどうか,チェック．開いていたら閉じる
  if( fsync( FdWrite() )==0 )
    {
      tl_warning ("FD haven't closed yet!!");
      Close();
    }
  //   delete;
  //  return TL_SUCCESS;
}
