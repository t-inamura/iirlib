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
 *	オブジェクト指向のプログラム環境へ
 *	fifo や socket に関するプログラム connection.c
 * 1998 Dec 10th
 *	返り値はすべて FAIL or SUCCESS だったが，変数参照の場合と，Constructor
 *	に限り，返り値をそのまま使う事にする
 * 1998 Dec 11th
 *	connection 型を Connection 型に
 *	ローカル変数 connect を connection に．connect という命令がある!!
 * 1998 Dec 16th
 *	Send, Receive に CONNECTION_INTEGER 型をつける
 * 1999 Jan 5th
 *	CONNECTION_WORD 型を作る.STRING 型は実は今まで Word 型だったのを直す
 * 1999 Jan 15th
 *	CONNECTION_STREAM 型を作る.
 * 1999 Jan 19th
 *	STRING型で送る時の限界が 128 文字なのは，connection.c のせいであると
 *	  気が付く．magic number を使わないように変更
 * 1999 Jan 20th
 *	clisp での文字列は \0 がない．文字列の終了コードは存在しない．
 *	 ここの違いを考慮して，Send, Receive などを更新
 * 1999 Jan 21st
 *	上の問題に対処するため，CONNECTION_LISP 型を新設
 * 1999 Jan 22nd
 *	port 番号に対して, htons を使うように改良
 * 1999 Jan 23rd
 *	STRING型の send の時,ダイレクトに文字列を書いてコマンド発行して良い
 *	  事にするために，いったん内容をコピーしてから send するように改良
 * 1999 Jan 24th
 *	STREAM の時, チェックのために長さを2回送る．違う時はエラー
 * 1999 Jan 30th
 *	宮本のソースを習って，Socket を shutdown してから閉じる
 * 1999 Feb 15th
 *	マルチアクセプションのサーバーのための拡張
 *	サーバの場合には，ホスト名を指定せずに，INADDR_ANY を使うように
 * 1999 Feb 17th
 *	マルチアクセスのサーバに接続するクライアントは初期設定の部分,
 *	接続を遮断する部分で，共通の手続きをする必要がある．
 *	なので，Connection_Open の際に CONNECTION_MULTICLIENT でそれを指定する
 * 1999 Feb 23rd
 *	JIS で書かれていたので，EUC にする．JIS だと GUI と継らなかった
 *	なぜなのかは分からない？？？不思議
 * 1999 Mar 9th
 *	通信とは直接関係のない，strmember などを mylib.c に移動
 * 1999 Apr 24th
 *	文字列を送信する際に，相手が LISP だろうと C だろうと同じ形式で送るように
 *	"abc" なら，'a' 'b' 'c' '\n' を送る事．Cの時は '\0' になるように加工する
 *	よって，AutoReport AutoDetect を廃止する
 * 1999 Arp 27th
 *	BYTE 型の通信の際にも,読みこぼしを考慮して通信するようにする
 * 1999 Apr 28th
 *	サーバからは接続を切らない for MultiSocket_Close
 * 1999 May 11th
 *	AutoReport, AutoDetect 無しで安定しているので，purge する
 * 1999 Jun 16th
 *	Connection_Destructor を新設
 * 1999 Jul 9th
 *	MultiSocket_Connection を新設
 * 1999 Nov 9th
 *	Connection_SelectWithTime を追加
 * 1999 Nov 25th
 *	Windows との共存をはかる
 * 2001 Sep 30th
 *	DOUBLE 型への対応
 * 2002 Jan 30th
 *	TL_FAIL, TL_TRUE へシフト
 * 2002 Jan 31st
 *	CONNECTION_TLVECTOR に対応
 * 2008 Feb 13th
 *	Changed into C++
 */

#include "iirlib.h"
#include "multisocket.h"
#include "connection.h"

#define	SOCKET_ONLY




// 新設 : 1999 Feb 15th
// 動作 : マルチアクセスサーバの構造体生成
// 入力 : *str		: 構造体の名前
// 入力 : port		: 使用するポート
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



// 新設 : 1999 Feb 15th
// 変更 : 1999 May 12th
//	  受け付け受理か，Busy かを 1byte で送っていたが，EusLisp が読めないよう
//	  なので文字列でこれを送信する事にする
// 変更 : 1999 Jul 7th : 接続された番号を返すようにする
// ----
// 動作 : 途中に接続要求してきた client に対して,accept する
// 返値 : 接続 OK ならば、接続された MultiSocket 内部での番号を返す
//	: 接続 FAIL なら TL_FAIL
int MultiSocket::Accept()
{
  int			s, i, debug=0;
  struct sockaddr_in	caddr; // クライアント側のアドレス情報を入れるところ
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
	  // もういっぱいで接続不可能である事を向こうに知らせる
	  tl_message ("Too busy");
	  strcpy (com, CONNECTION_BUSY);
	  connection[i]->my_send (s, com, 4);
	  // Lisp, C 共通仕様で文字列終了を意味する '\n' を送信
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
	  tmpconnection->my_send (s, com, 4);	// 受け付け OK の信号を向こうに知らせる
	  // Lisp, C 共通仕様で文字列終了を意味する '\n' を送信
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


// 変更 : 1999 Apr 28th : サーバからは接続を切らない
// 新設 : 1999 Feb 16th
// 動作 : マルチアクセスのサーバで動的に接続を切る
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


// 新設 : 1999 Feb 16th
// 動作 : マルチアクセスのための fd を max_fd と比較して，max_fd を返す
// 入力 : *mscoket	: 接続が成立している分のロボットとの通信路
// 入力 : max		: 現段階での一番大きい fd 値
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


// 新設 : 1999 Feb 16th
// 動作 : 接続済みの Connection に対する FD_SET および，新しい接続要求の
//        可能性のある fd への FD_SET
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


// 新設 : 1999 Feb 16th
// 返値 : FAIL なら何も select されてない事になる
// 入力 : *msocket	: マルチソケット構造体
// 入力 : *readfds	: fd_set 構造体へのポインタ,ここをいじる
// 出力 : *result	: select が検出した MultiSocket 内部での番号
//	:		: 新しい接続要求であれば，TL_FAIL
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


// 新設 : 1999 Jul 9th
// 動作 : MultiSocket で管理している Connection 構造体を参照する
// 入力 : *msocket	: 対象のマルチソケット構造体
// 入力 : no		: 対象の Connection 構造体の番号
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
