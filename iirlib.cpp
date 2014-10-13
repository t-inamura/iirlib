// --*- C++ -*--
/*
 *	IIR (Intelligent Interactive Robotics Lab.) Library: C++ Core Module
 *      
 *	Last modified on 2008-10-05 by Tetsunari Inamura
 *
 *	Copyright (c) Tetsunari Inamura 1999--2008.
 *	All Rights Reserved.
 */

/*
 * 1999 Mar 9th project created
 *	starting from remove_backslash_N
 * 1999 Mar 13th
 *	imported to CVS
 * 1999 Apr 14th
 *	fgets_without_sharp added
 *	sscanf_double_with_pointer_move added
 * 1999 May 11th
 *	string_getkeyword added
 *	sscanf_word_with_pointer_move added
 * 1999 May 22nd
 *	bubble sort, search_1st_2nd are added
 * 1999 Jun 16th
 *	string_count, integerp added
 * 1999 Jul 7th
 *	several functions were destroyed by HDD crash
 *	normalize_vector adopted to zero vector
 *	exec_functable added
 * 1999 Jul 12th
 *      pointer_swap_void added (tentative)
 * 1999 Jul 19th
 *	getkeyword reads S-format in Lisp (ex. (a (b c)) is read as 1 word)
 * 1999 Jul 26th
 *	int sscanf_int_with_seeking, expand_list_to_int added
 * 1999 Aug 16th
 *	using glib
 * 2000 Mar 30th
 *	exec_functable_with_arg added
 * 2001 Nov 22nd
 *	with_pointer_move can read csv files
 *	Modified to be called from C++
 * 2002 Jan 23rd
 *	tl_warning added
 * 2002 Apr 19th
 *	using tlVector
 * 2003 May 28th
 *	destroyed TETSULIB_*** 
 * 2008 Jan 21st
 *	applying for C++
 * 2008 Feb 9th
 *	rename TL_TRUE to TRUE, TL_FALSE to FALSE
 * 2008 Feb 17th
 *	Reading CRLF as end of line in Windows/Cygwin environment
 * 2008-10-05
 *	debug on tl_csv_to_glist
 * 2008-10-07
 *	add tl_word_array_to_glist to use any eliminators
 */


#include "iirlib.h"



/*-----------------------------------------------------------------------------------*/
// Added  1999 Jan 21st
// Modify 1999 Aug 2nd : Added exception procedure
// Input : reference		: string to be referred
// Input : search		: target string to the search
// Memo : searching the [search] string from [reference] string
// Return : location of the search string in the reference (starting from 1)
//        : if it is not found, return 0
/*-----------------------------------------------------------------------------------*/
int tl_strmember (const char *reference, const char *search)
{
  int		i;

  if (reference==NULL || search==NULL)
    return 0;
  // if length of referred string is shorter than target, return immediately
  if (strlen(reference) < strlen(search) )
    return 0;

  for (i=0; i<(int)(strlen(reference))-(int)(strlen(search))+1; i++)
    {
      if (!strncmp( reference+i, search, strlen(search) ) )
	return i+1;
    }
  return 0;
}



/*-----------------------------------------------------------------------------------*/
// Added  : 2002 Aug 12th : string_getkeyword is modified
// Memo   : if there is *keyword like :hoge in *src string, return the next word as GString type
//	  : return NULL if the target keyword is not exist
// Input  : *src		: String to be referred
// Input  : *keyword	: Key string to the searching
// Return : (GString *)	: result
/*-----------------------------------------------------------------------------------*/
GString *tl_string_assoc (char *src, char *keyword)
{
  char		*charp = NULL, tmp[MAX_STRING];
  int		i, j, debug=0;
  GString	*result = NULL;
  
  charp = src;
  result = g_string_new (NULL);
  
  if (!tl_strmember(src, keyword))
    {
      tl_warning ("No such keyword {%s} in {%s}", keyword, src );
      return NULL;
    }
  for(;;) {
    sscanf_word_with_pointer_move (&charp, tmp);
    if (!strcmp( tmp, keyword )) {
      // In case of matching
      tl_skip_pointer( &charp );
      if (*charp == '(' ) {
	// If the head char of the result is '(', chunking till ')' comes
	for( i=0; ;i++ ) {
	  if (*(charp+i)==')'  ) break;
	  if (*(charp+i)=='\0' ) { i--; break; }
	}
	for( j=0; j<=i; j++ ) g_string_append_c (result, *(charp+j));
	g_string_append_c (result, '\0');
	if(debug) tl_message ("get with '()' : i=%d : result=%s", i, result->str);
	return result;
      }
      else {
	// In case of normal word without '(' and ')'
	if (sscanf_word_with_pointer_move( &charp, tmp )==TRUE ) {
	  // If case of result, the last ')' is deleted
	  g_string_append (result, tmp);
	  remove_blacket_end (result->str);
	  return result;
	}
	else {
	  // In case of matching, but result is not exist
	  return result;
	}
      }
    }
    else {
      // Keyword not found
      if (*charp=='\0' ) return result;
    }
  }
}




/*-----------------------------------------------------------------------------------*/
// Added : 1999 May 11th
// Modified : 1999 Jul 27th : reading the S-format in lisp as 1 chunk
// Memo : If *keyword in :hoge style is found in *src, copy of the next word is input in *result 
//	: if *keyword not found, *result is set to NULL
// ----------------------------------------------------------------------------------
// Input : *src		: string to be referred
// Input : *keyword	: target keyword
// Output : *result	: result data
// Return : keyword found TRUE, keyword not found FALSE
/*-----------------------------------------------------------------------------------*/
int tl_string_getkeyword (char *src, char *keyword, char *result)
{
  char		*charp=NULL, tmp[MAX_STRING];
  int		i, j, debug=0;

  charp = src;

  if (!tl_strmember(src, keyword))
    {
      tl_warning ("No such keyword {%s} in {%s}", keyword, src );
      result = NULL;
      return FALSE;
    }
  for(;;)
    {
      sscanf_word_with_pointer_move( &charp, tmp );
      if (!strcmp( tmp, keyword ) )
	{
	  // In case of matching
	  tl_skip_pointer (&charp);
	  if (*charp == '(' )
	    {
	      // If the head char of the result is '(', chunking till ')' comes
	      for( i=0; ;i++ )
		{
		  if (*(charp+i)==')'  ) break;
		  if (*(charp+i)=='\0' ) { i--; break;
		  }
		}
	      for( j=0; j<=i; j++ ) result[j] = *(charp+j);
	      result[j] = '\0';
	      //strncpy( result, charp, i+1 );// This will be error when a string without '\0' comes
	      if(debug) tl_message (" getkeyword with '()' : i=%d : result=%s", i, result );
	      return TRUE;
	    }
	  else {
	    // In case of normal word without '(' and ')'
	    if (sscanf_word_with_pointer_move( &charp, tmp )==TRUE ) {
	      // If case of result, the last ')' is deleted
	      strcpy( result, tmp );
	      remove_blacket_end( result );
	      return TRUE;
	    }
	    else {
	      strcpy(result, "\0");
	      return FALSE;
	    }
	  }
	}
      else {
	// Keyword not found , search more
	if (*charp=='\0' ) {
	  strcpy(result, "\0");
	  return FALSE;
	}
      }
    }
}


/*-----------------------------------------------------------------------------------*/
// Memo     : removing the last '\n' from string to be sent to Lisp process
// Modified on 2008-02-17 : If CRLF is found, remove them (for Windows) 
// Input    : *string	: target string
/*-----------------------------------------------------------------------------------*/
void remove_backslash_N (char *str)
{
  int		debug=0;
  int		i;

  for( i=0; i<=(int)(strlen(str)); i++ )
    {
      if(debug) tl_message ("str[%d]='%c' <%d>", i, str[i], str[i]);
      if (str[i] == '\r' && str[i+1]=='\n')
	{
	  if(debug) tl_message ("Found CRLF!");
	  str[i]   = '\0';
	  str[i+1] = '\0';
	  return;
	}
      if (str[i] == '\n')
	{
	  tl_message ("Found LF (\n)!");
	  str[i] = '\0';
	  return;
	}
    }
}


int remove_dot( char *string )
{
  int		i;

  for( i=0; i<(int)(strlen(string)); i++ )
    {
      if (string[i] == '.' )
	{
	  string[i] = '\0';
	  return 1;
	}
    }
  return 0;
}


/*-----------------------------------------------------------------------------------*/
// Input  : *str	: target string with double quotation
// Output : *str2	: result
/*-----------------------------------------------------------------------------------*/
int tl_remove_double_q (char *str, char *str2)
{
  int		i, j;
  int		start=0, end=0, count=0;

  for( i=0; ;i++ )
    {
      if (str[i] == '"' && start!=0 )
	{
	  end = i-1;
	  count++;
	}
      if (str[i] == '"' && start==0 )
	{
	  start = i+1;
	  count=1;
	}
      if (count==2) break;
    }
  for ( j=0,i=start; i<=end; i++,j++ )
    {
      str2[j] = str[i];
    }
  str2[j] = '\0';
  return TRUE;
}




/*-----------------------------------------------------------------------------------*/
// Input : *sentence	: target string to be added the '\n'
/*-----------------------------------------------------------------------------------*/
void add_backslash_N (char *sentence)
{
  int		i;

  for( i=0; ;i++ ) {
    if (sentence[i]=='\0' ) {
      if (sentence[i-1]=='\n' ) return;	// normal case
      else {
	sentence[i] = '\n';
	sentence[i+1] = '\0';
	return;
      }
    }
  }
}



/*-----------------------------------------------------------------------------------*/
// Modified : 1999 Jul 26th : printing warning if digit is included
// Modified : 2001 Nov 22nd : comma ',' is acceptable
// Modified : 2008 Feb 17th : CRLF is acceptable for Windows
// Memo : modify the sscanf, to be used as fscanf with pointer move
// TODO : This function can be replaced by sstream, to be discarded
// Input : **charp	: pointer for the string address (pointer) of the target string
/*-----------------------------------------------------------------------------------*/
int sscanf_int_with_pointer_move (char **charp)
{
  int		value;
  int		digit;
  char		tmp[32];

  if (**charp=='\n' || **charp=='\r' || **charp=='\0')
    {
      tl_warning ("End of line. You had better using 'with_seeking'" );
      return FALSE;
    }
  // Move the pointer of string in order to skip the space between digits/figures
  for(;;)
    {
      if ((**charp==' ') || (**charp=='\t') || (**charp==','))
	(*charp)++;
      else if (((**charp < '0') || (**charp > '9')) && (**charp!='-') )
	{
	  // skipping irregular chars with warning
	  tl_message ("!CAUTION! Not space, Not digit <%c> is mingled! in str {%s}", **charp, *charp );
	  (*charp)++;
	}
      else break;
    }

  sscanf( *charp, "%d", &value );
  sprintf( tmp, "%d", value );
  digit = strlen( tmp );
  *charp += digit;
  // skipping the pointer to the next digit
  for(;;)
    {
      if (**charp==' ' || **charp=='\t' || **charp==',')
	(*charp)++;
      else break;
    }
  return value;
}




/*-----------------------------------------------------------------------------------*/
// Added    : 1999 Jul 26th
// Modified : 2008 Feb 17th : CRLF is acceptable for Windows
// Memo     : basically, it is same as sscanf_int_with_pointer_move, but error exception available
// Input    : **charp	: pointer for the string address (pointer) of the target string
// Output   : *result	: result
// Return   : Success / Fail
/*-----------------------------------------------------------------------------------*/
int sscanf_int_with_seeking( char **charp, int *result )
{
  int		value, digit;
  int		debug=0;
  char		tmp[32];

  if(debug) tl_message ("target string is <%s>", *charp);
  for(;;)
    {
      if ((**charp==' ') || (**charp=='\t') || (**charp==','))
	(*charp)++;
      
      else if ((**charp=='\0') || (**charp=='\n') || (**charp=='\r') )
	return FALSE;
      
      else if (((**charp < '0') || (**charp > '9')) && (**charp!='-') )
	{
	  tl_warning ("Not space, Not digit <%c> is mingled!", **charp);
	  (*charp)++;
	}
      else break;
    }

  sscanf( *charp, "%d", &value );
  sprintf( tmp, "%d", value );
  digit = strlen( tmp );
  *charp += digit;

  for(;;)
    {
      if (**charp==' ' || **charp=='\t' || **charp==',') (*charp)++;
      else break;
    }
  *result = value;
  if(debug) tl_message ("result = %d", value);
  return TRUE;
}



/*-----------------------------------------------------------------------------------*/
// Added    : 1999 Apr 14th
// Modified : 2001 Nov 22nd : ',' segmentation acceptable
// Memo     : double version of sscanf_int_with_seeking
//            error detection and exception can be performed
// Input    : **charp	: pointer for the address of target string
// Output   : *data	: read double value
// Return   : (int)	: TRUE / FALSE
/*-----------------------------------------------------------------------------------*/
int sscanf_double_with_seeking (char **charp, double *data)
{
  float		value;
  int		debug=0;

  if(debug) tl_message ("trying ... %s", *charp);
  if( **charp=='\n' || **charp=='\r' || **charp=='\0' )
    {
      if(debug) tl_warning ("End of line. You had better using 'with_seeking'");
      return FALSE;
    }
  // Moving the pointer of string in order to skip the digits
  for(;;) {
    if (**charp==' ' || **charp=='\t' || **charp==',') (*charp)++;
    else break;
  }
  if( **charp=='\n' || **charp=='\r' || **charp=='\0' )
    {
      if(debug) tl_warning ("End of line. You had better using 'with_seeking'");
      return FALSE;
    }
  // reading floating point value from a string got by fgets
  sscanf (*charp, "%f", &value);
  // Moving the pointer of string till it hit to digit, sign char or '.'
  for(;;)
    {
      if( (**charp>='0' && **charp<='9') || **charp=='.' || **charp=='-')
	(*charp)++;
      else break;
    }
  *data = (double)value; 
  return TRUE;
}




/*-----------------------------------------------------------------------------------*/
// Added    : 1999 Apr 14th
// Modified : 2001 Nov 22nd : ',' acceptable
// Modified : 2008 Feb 17th : CRLF is acceptable for Windows
// Input : **charp	: same as int version
// Return : (double)	: same as int version
/*-----------------------------------------------------------------------------------*/
double sscanf_double_with_pointer_move (char **charp)
{
  float		value;

  if (**charp=='\n' || **charp=='\r' || **charp=='\0' )
    {
      tl_warning ("End of line. You had better using 'with_seeking'");
      return 0.0;
    }
  for(;;)
    {
      if (**charp==' ' || **charp=='\t' || **charp==',') (*charp)++;
      else break;
    }
  sscanf( *charp, "%f", &value );

  for(;;)
    {
      if (**charp!=' ' && **charp!='\t' && **charp!=',' && **charp!='\n' && **charp!='\0')
	(*charp)++;
      else break;
    }
  return (double)value;
}



/*-----------------------------------------------------------------------------------*/
// Added    : 1999 May 11th 
// Modified : 2008 Feb 17th : CRLF is acceptable for Windows
// Memo   : same as int and double
// Input  : **charp	: same as int and double
// Output : *word	: result word
// Return : (int)	: Success or Fail
/*-----------------------------------------------------------------------------------*/
int sscanf_word_with_pointer_move (char **charp, char *word)
{
  if (**charp=='\0' || **charp=='\n' || **charp=='\r')
    return FALSE;

  for(;;)
    {
      if (**charp==' ' || **charp=='\t' || **charp==',')
	{
	  (*charp)++;
	  if (**charp=='\0' || **charp=='\n' || **charp=='\r')
	    return FALSE;
	}
      else break;
    }

  sscanf( *charp, "%s", word );

  (*charp) += strlen( word );
  return TRUE;
}



/*-----------------------------------------------------------------------------------*/
// Added : 1999 Jul 19th
// Modified : 2001 Nov 22nd : ',' acceptable
// Memo     : skipping space, tab and so on
// Input    : pointer for the target string
/*-----------------------------------------------------------------------------------*/
int tl_skip_pointer (char **charp)
{
  if (**charp=='\0' ) return FALSE;

  for(;;)
    {
      if (**charp==' ' || **charp=='\t' || **charp==',')
	{
	  (*charp)++;
	  if (**charp=='\0' ) return FALSE;
	}
      else return TRUE;
    }
}




/*-----------------------------------------------------------------------------------*/
// Added  : 1999 Jan 21st
// Memo   : delete ')' at the end of string
// Return : TRUE or FALSE
/*-----------------------------------------------------------------------------------*/
int remove_blacket_end( char *string )
{
  if (string[ strlen(string)-1 ] == ')')
    {
      string[ strlen(string)-1 ] = '\0';
      return TRUE;
    }
  return FALSE;
}



/*-----------------------------------------------------------------------------------*/
// Added : 1999 Feb 25th
// Memo  : Extract only filename from fullpath expression
// Input : *fullpath		: fullpath filename
// Return: (char *)		: extracted filename / NULL if failed
/*-----------------------------------------------------------------------------------*/
char *tl_pickup_last_filename (char *fullpath)
{
  int		i, debug=0;
  char		*filename = NULL;

  if (fullpath[strlen(fullpath)-1] == '/' )
    {
      tl_warning ("this is directory name {%s}", fullpath);
      return NULL;
    }
  // if '/' is not found, return without any modification
  if (!tl_strmember (fullpath, "/"))
    {
      return fullpath;
    }
  for( i=strlen(fullpath); i>0; i-- )
    {
      if (fullpath[i] == '/') break;
    }
  filename = fullpath+i+1;
  if(debug)
    {
      tl_message ("fullpath %s", fullpath);
      tl_message ("filename %s", filename);
    }
  return filename;
}


/*-----------------------------------------------------------------------------------*/
// Added : 1999 Feb 25th
// Memo  : Extract only directory name from fullpath expression
// Input : *fullpath
// Output : *dir
// Return : (int)		: TRUE / FALSE
/*-----------------------------------------------------------------------------------*/
int pickup_directory (char *fullpath, char *dir)
{
  int		i, flag=0, debug=0;

  tl_return_val_if_fail (fullpath, "string is NULL", FALSE);
  
  // If '/' not found, current directory is output
  for (i=strlen(fullpath); i>=0; i--)
    if (fullpath[i]=='/') flag=1;
  if (flag==0)
    {
      strcpy (dir, ".");
      return TRUE;
    }

  // If the last char is '/', output without any modification
  if (fullpath[strlen(fullpath)] == '/')
    {
      if(debug) tl_message ("%s", fullpath );
      strcpy( dir, fullpath );
      return TRUE;
    }

  for( i=strlen(fullpath); i>=3; i-- )
    {
      if (fullpath[i] == '/') break;
      if (i==0) {
	tl_warning ("Path name <%s> is too short", fullpath);
	dir[0] = '\0';
	return FALSE;
      }
    }
  strncpy (dir, fullpath, i);
  dir[i] = '\0';
  if(debug) tl_message ( " <pickup_last_file> : fullpath  %s", fullpath);
  if(debug) tl_message ( " <pickup_last_file> : directory %s", dir);
  return TRUE;
}



/*---------------------------------------------------------------------------*/
// Added : 2002 Mar 1st
// Memo : delete suffix from filename
// Output : *filename		: filename in GString type
// Return : (int)		: Success or Failed
/*---------------------------------------------------------------------------*/
int tl_remove_suffix (GString *filename)
{
  int		i, debug=0;

  tl_return_val_if_fail (filename, "filename is NULL", FALSE);

  if(debug) tl_message ("input  : %s", filename->str);

  for (i=filename->len; i>=1; i--)
    {
      if (filename->str[i] == '.' ) break;
      if (i==1)
	{
	  tl_warning ("Too short !  Input string is {%s}", filename->str);
	  return FALSE;
	}
    }
  //filename[i] = '\0';
  g_string_erase (filename, i, filename->len-i);
  if(debug) tl_message ("output : %s", filename->str);
  return TRUE;
}



/*---------------------------------------------------------------------------*/
// Added : 2002 Feb 17th
// Memo : Extrace suffix from fullpath expression
// Input : *fullpath
// Return : (char *)		: pointer to the suffix, or NULL in case of fail
// Memo   : renamed from tl_pickup_suffix to tl_pickup_terminal_filename
/*---------------------------------------------------------------------------*/
char *tl_pickup_terminal_filename (char *fullpath)
{
  int		i, debug=0;

  if (fullpath[strlen(fullpath)-1] == '/')
    {
      tl_warning ("This is directory name. Input string is {%s}", fullpath);
      return NULL;
    }
  for (i=strlen(fullpath)-1; i>0; i--)
    {
      if (fullpath[i] == '/' ) break;
    }
  if(debug)
    {
      tl_message ("fullpath %s", fullpath);
      tl_message ("suffix   %s", fullpath+i+1);
    }
  return fullpath+i+1;
}



/*-------------------------------------------------------------------------------------*/
// Added : 2002 Aug 12th
// Memo  : Extract filename from fullpath expression
// Input : *filename	: filename in fullpath expression
// Return: (char *)	: pointer to the string of suffix, or NULL if FAIL
/*-------------------------------------------------------------------------------------*/
char *tl_get_suffix (char *filename)
{
  int		i;
  int		debug=0;
  
  tl_return_val_if_fail (filename, "filename is NULL", NULL);

  if (tl_strmember (filename, ".")==0)
    return NULL;

  for (i=strlen(filename)-1; i>0; i--)
    {
      if (filename[i] == '/')
	return NULL;			// If '/' is found without '.', report it as error
      if (filename[i] == '.') break;
    }
  if(debug)
    {
      tl_message ("filename %s", filename);
      tl_message ("suffix   %s", filename + i + 1);
    }
  return filename+i+1;
}





/*---------------------------------------------------------------------------*/
// Added : 2002 Aug 12th
// Memo  : If target suffix is not found, it will be added
// Input : *GString	: filename
// Input : *suffix	: target suffix
/*---------------------------------------------------------------------------*/
int tl_add_suffix (GString *filename, char *suffix)
{
  int		debug=0;
  char		*suffix_p = NULL;

  tl_return_val_if_fail (filename, "filename is NULL", FALSE);
  tl_return_val_if_fail (suffix,   "suffix   is NULL", FALSE);

  suffix_p = tl_get_suffix (filename->str);
  if(debug) tl_message ("input filename  : %s", filename->str);
  if(debug) tl_message ("suffix of input : %s", suffix_p);

  if (suffix_p==NULL)
    {
      // In case that suffix is not found in input file
      if(debug) tl_message ("suffix of desired : %s", suffix);
      g_string_append (filename, ".");
      g_string_append (filename, suffix);
    }
  else if (strcmp (suffix, suffix_p))
    {
      // In case that suffix is not the same
      if(debug) tl_message ("suffix of desired : %s", suffix);
      g_string_append (filename, ".");
      g_string_append (filename, suffix);
    }

  if(debug) tl_message ("file result is : %s", filename->str);
  return TRUE;
}





/*-----------------------------------------------------------------------------------*/
// Added : 1999 Feb 19th 
// Modified : 1999 Apr 2nd 
// Input : num		: size of data buffer
// Input : *buf		: pointer of data buffer
// Return: (int)	: max value in the data. If the all data is equal to each other, FALSE is returned
/*-----------------------------------------------------------------------------------*/
int max_between (int num, int *buf)
{
  int		min_value, max_value, max_no;
  int		i;

  max_value = buf[0];
  min_value = buf[0];
  max_no    = 0;
  
  for( i=0; i<num; i++ ) {
    if (max_value < buf[i] ) {
      max_value = buf[i];
      max_no    = i;
    }
    if (min_value > buf[i] )
      min_value = buf[i];
  }
  if (min_value==max_value ) return -1;
  return max_no;
}


/*-----------------------------------------------------------------------------------*/
// Added : 2008 Feb 15th
/*-----------------------------------------------------------------------------------*/
int max_between (vector<double> vec)
{
  double	max_value;
  int		max_no = 0;

  max_value = vec[0];
  for (int i=0; i<(int)vec.size(); i++)
    {
      if (max_value < vec[i])
	{
	  max_value = vec[i];
	  max_no    = i;
	}
    }
  return max_no;
}



/*-----------------------------------------------------------------------------------*/
// Added : 1999 Mar 15th
// Input : num		: size of data buffer
// Input : *buf		: pointer of data buffer
// Return: (int)	: min value in the data. If the all data is equal to each other, FALSE is returned
/*-----------------------------------------------------------------------------------*/
int min_between( int num, int *buf )
{
  int		min_value, min_no;
  int		i;

  min_value = buf[0];
  min_no    = 0;
  
  for( i=0; i<num; i++ ) {
    if (min_value > buf[i] ) {
      min_value = buf[i];
      min_no    = i;
    }
  }
  return min_no;
}


/*-----------------------------------------------------------------------------------*/
// Added : 1999 Mar 18th
// Memo  : Finding the minimum value in sequence
// Input : num		: size of data buffer
// Input : *buf		: pointer of data buffer
// Return: (int)	: min value in the data. If the all data is equal to each other, FALSE is returned
/*-----------------------------------------------------------------------------------*/
int min_natural_number_between (int num, int *buf)
{
  int		min_value, min_no;
  int		i, flag=0;

  min_value = 99999;
  min_no    = 0;
  
  for( i=0; i<num; i++ ) {
    if (min_value > buf[i] && buf[i]>0 ) {
      min_value = buf[i];
      min_no    = i;
      flag = 1;
    }
  }
  if (flag==1 ) return min_no;
  else return -1;
}


/*-----------------------------------------------------------------------------------*/
// Added : 1999 May 22nd
// Memo : subfunction of bubble sort
/*-----------------------------------------------------------------------------------*/
static void bubble_swap (bubble_data *a, bubble_data *b)
{
  bubble_data t;

  t.key   = a->key;
  t.info  = a->info;
  a->key  = b->key;
  a->info = b->info;
  b->key  = t.key;
  b->info = t.info;
}


/*-----------------------------------------------------------------------------------*/
// Added : 1999 May 22nd
// Reference : http://alfin.mine.utsunomiya-u.ac.jp/~niy/algo/index.html
// Memo : Bubble sort
// Input : *data	: data sequence in bubble_data type
// Input : nmem		: size of buffer
// Input : asdes	: 0: ascending sequence,  1: descending order
/*-----------------------------------------------------------------------------------*/
int bubbleSort(bubble_data *data, int nmem, int asdes)
{
  int i, j;

  for (i = 0; i < nmem - 1; i++) {
    for (j = nmem - 1; j >= i + 1; j--)
      if (asdes == 0) {
	if (data[j].key < data[j-1].key)
	  bubble_swap(&data[j], &data[j-1]);
      } else {
	if (data[j].key > data[j-1].key)
	  bubble_swap(&data[j], &data[j-1]);
      }
  }
  return TRUE;
}
      
/*-----------------------------------------------------------------------------------*/
// Added : 1999 May 22nd
// Input : num		: size of data buffer
// Input : *buf		: data buffer
// Output : *first_no	: index no. of the max data
// Output : *first_val  : value of the max data
// Output : *second_no	: index no. of the second max data
// Output : *second_val	: value of the 2nd max data
/*-----------------------------------------------------------------------------------*/
int tl_search_1st_2nd (int num, double *buf, int *first_no, double *first_value, int *second_no, double *second_value)
{
  int		i;
  bubble_data	*sequence;

  tl_return_val_if_fail (buf, "buffer is NULL", FALSE);

  sequence = (bubble_data *)malloc (sizeof(bubble_data) * num);

  for( i=0; i<num; i++ ) {
    sequence[i].key  = buf[i];
    sequence[i].info = i;
  }
  bubbleSort (sequence, num, 1);
  *first_value  = sequence[0].key;
  *first_no     = sequence[0].info;
  *second_value = sequence[1].key;
  *second_no    = sequence[1].info;

  free (sequence);
  return TRUE;
}


/*-----------------------------------------------------------------------------------*/
// Added : 1999 Jul 12th
/*-----------------------------------------------------------------------------------*/
int pointer_swap_void (void **p1, void **p2)
{
  int		debug=0;
  void		*tmp=NULL;

  if(debug) tl_message ("p1 : %p\t\tp2 : %p", *p1, *p2);
  tmp  = *p2;
  *p2  = *p1;
  *p1  = tmp;
  if(debug) tl_message ("p1 : %p\t\tp2 : %p", *p1, *p2);
  return TRUE;
}


int pointer_swap( int *p1, int *p2 )
{
  int		tmp;

  tmp = *p2;
  *p2 = *p1;
  *p1 = tmp;
  return TRUE;
}


/*-----------------------------------------------------------------------------------*/
// Added    : 1999 Mar 25th
// Modified : 1999 Jul 7th  : Added exception for zero vector

// Memo   : vector normalization (too old)
// Input  : dim			: dimension of the vector
// Input  : *src_vec		: vector by double sequence
// Output : *dst_vec		: normalized vector
/*-----------------------------------------------------------------------------------*/
void normalize_vector (int dim, double *src_vec, double *dst_vec)
{
  double		sum=0.0;
  int			i;

  for( i=0; i<dim; i++ )
    sum += src_vec[i];
  if (sum < 0.00001 )
    for( i=0; i<dim; i++ )  dst_vec[i] = 0.0;
  else
    for( i=0; i<dim; i++ )  dst_vec[i] = src_vec[i] / sum;
}



/*---------------------------------------------------------------------------*/
// Added    : 1999 Mar 14th
// Modified : 2008 Feb 17th : CRLF is acceptable for Windows
// Memo   : reading with fgets with skipping the line starting from '#' (ex comment)
//          empty line will be also skipped
// Input  : *str, n, *fp		: same as fgets
// Return : NULL in case of failure. Return NULL when the result of fgets is NULL
/*---------------------------------------------------------------------------*/
char *tl_fgets_without_sharp (char *str, int n, FILE *fp)
{
  char	tmp[MAX_STRING];
  int	end_flag=0;

  if (n>MAX_STRING)
    {
      tl_warning ("size %d is larger than %d", n, MAX_STRING);
      return NULL;
    }
  do {
    if (fgets( tmp, n, fp )==NULL ) return NULL;
    if (tmp[0]=='#' || tmp[0]=='\n' || tmp[0]=='\r')
      end_flag = 0;
    else
      end_flag = 1;
  }
  while (end_flag != 1);
  strcpy( str, tmp );
  return str;
}



/*-----------------------------------------------------------------------------------*/
// Added : 1999 Jun 16th
// Memo  : Counting target char [c] in a string [*string]
// Return: the number of chars
/*-----------------------------------------------------------------------------------*/
int string_count( char *string, char c )
{
  int	i, n=0;

  for (i=0; i<(int)(strlen(string)); i++)
    {
      if (string[i]==c) n++;
    }
  return n;
}


/*-----------------------------------------------------------------------------------*/
// Added : 1999 Jun 16th
// Input : *string
// Return: TRUE: all the char is digit, FALSE: or not
/*-----------------------------------------------------------------------------------*/
int integerp (char *string)
{
  int		i;

  for (i=0; i<(int)(strlen(string)); i++)
    {
      if (string[i]<'0' || string[i]>'9' )
	if (string[i]!='-' ) return FALSE;
    }
  return TRUE;
}


/*-----------------------------------------------------------------------------------*/
// Added : 1999 Sep 15th
// Memo   : atoi with error exception
// Input  : string	: target string
// Output : value	: read value
// Return : Success or Fail
/*-----------------------------------------------------------------------------------*/
int tl_atoi (char *string, int *value)
{
  if (integerp(string)==FALSE)
    {
      tl_message ("input string is not integer! : %s", string );
      return FALSE;
    }
  *value = atoi(string);
  return TRUE;
}


/*-----------------------------------------------------------------------------------*/
// Added : 1999 May 14th
// Memo  : execute function with string of the name of function
// Input : *ft		: function table in functable_t style
// Input : *string	: name of the target function
/*-----------------------------------------------------------------------------------*/
int exec_functable (functable_t *ft, char *string)
{
  int		i, debug=0;

  if(debug) tl_message( "Start : str={%s}", string );
  for(i=0;;i++)
    {
      if(debug) tl_message( "Check func name {%s}", ft[i].name );
      if (tl_strmember( string, ft[i].name ))
	{
	  if(debug) tl_message( "Hit function name %s", ft[i].name );
	  (ft[i].func)(string);
	  return TRUE;
	}
      else if (!strcmp( "FUNCEND", ft[i].name ) )
	{
	  tl_warning ("No such command {%s}", string);
	  return FALSE;
	}
    }
}


/*-----------------------------------------------------------------------------------*/
// Added : 1999 Sep 1st
// Memo  : exec_function with 2 arguments
/*-----------------------------------------------------------------------------------*/
int exec_functable_with_int (functable_with_int_t *ft, char *string, int value)
{
  int		i, debug=0;

  if(debug) tl_message("Start : str={%s}", string);
  for(i=0;;i++)
    {
      if(debug) tl_message ("Check func name {%s}", ft[i].name );
      if (tl_strmember( string, ft[i].name ))
	{
	  if(debug) tl_message ("Hit function name <%s>", ft[i].name );
	  (ft[i].func)(string, value);
	  return TRUE;
	}
      else if (!strcmp( "FUNCEND", ft[i].name ) )
	{
	  tl_warning ("No such command {%s}", string );
	  return FALSE;
	}
    }
}


/*-----------------------------------------------------------------------------------*/
// Added : 1999 Aug 24th
// Memo  : exec_function with debugging message
// Input : *string	: the name of target function
// Input : *name	: debugging message
/*-----------------------------------------------------------------------------------*/
int exec_functable_with_name( functable_with_arg_t *ft, char *string, char *name )
{
  int		i, debug=0;

  if(debug) tl_message ("Start : str={%s}", string );
  for(i=0;;i++) {
    if(debug) tl_message ("Check func name {%s}", ft[i].name );
    if (tl_strmember( string, ft[i].name ))
      {
	if(debug) tl_message ("Hit function name %s", ft[i].name);
	(ft[i].func)(string);
	return TRUE;
      }
    else if (!strcmp( "FUNCEND", ft[i].name ) )
      {
	tl_warning ("exec_functable");
	tl_message ("<%s> No such command {%s}", name, string );
	return FALSE;
      }
  }
}



/*-----------------------------------------------------------------------------------*/
// Added : 2000 Mar 30th
// Memo  : execute function with string of the name of function and general argument
// Input : *ft		: same as exec_functable
// Input : *string	: same as exec_functable
// Input : *arg		: argument for the function
/*-----------------------------------------------------------------------------------*/
int exec_functable_with_2arg( functable_with_2arg_t *ft, char *string, void *arg )
{
  int		i, debug=0;

  if(debug) tl_message ("Start : str={%s}", string );
  for(i=0;;i++) {
    if(debug) tl_message ("Check func name {%s}", ft[i].name);
    if (tl_strmember( string, ft[i].name ))
      {
	if(debug) tl_message ("Hit function name %s", ft[i].name );
	(ft[i].func)(string, arg);
	return TRUE;
      }
    else if (!strcmp( "FUNCEND", ft[i].name ) )
      {
	tl_warning ("exec_functable");
	tl_message ("No such command {%s}", string);
	return FALSE;
      }
  }
}



/*-----------------------------------------------------------------------------------*/
// Added  : 1999 Jul 26th
// Memo   : Expand list of int in Lisp style into int sequence
// Input  : *list	: S-format in Lisp
// Output : *length	: number of element (malloc is required before calling this function)
// Output : *data	: sequence of data  (malloc is required before calling this function)
// Return : (int)	: FAIL in case of error
/*-----------------------------------------------------------------------------------*/
int expand_list_to_int (char *list, int *length, int *data)
{
  int		i, ret, value;
  char		*charp;
  
  charp = list;
  if (list[0]=='(' )
    charp += 1;

  for( i=0; ;i++ )
    {
      if ( (*charp==')') || (*charp=='\0') || (*charp=='\n') )
	{
	  *length = i;
	  return TRUE;
	}
      ret = sscanf_int_with_seeking (&charp, &value);
      if (ret==FALSE )
	{
	  *length = i;
	  return FALSE;
	}
      data[i] = value;
    }
  return FALSE;
}


/*-----------------------------------------------------------------------------------*/
// Added  : 2008 Aug 6th
// Memo   : Expand list of double in string into double sequence
// Input  : *list	: S-format in Lisp or string
// Output : *length	: number of element (malloc is required before calling this function)
// Output : *data	: sequence of data  (malloc is required before calling this function)
// Return : (int)	: FAIL in case of error
/*-----------------------------------------------------------------------------------*/
int expand_list_to_double (char *list, int *length, double *data)
{
  int		i, ret;
  char		*charp;
  double	value;
  
  charp = list;
  if (list[0]=='(' )
    charp += 1;

  for( i=0; ;i++ )
    {
      if ( (*charp==')') || (*charp=='\0') || (*charp=='\n') )
	{
	  *length = i;
	  return TRUE;
	}
      ret = sscanf_double_with_seeking (&charp, &value);
      if (ret==FALSE )
	{
	  *length = i;
	  return FALSE;
	}
      data[i] = value;
    }
  return FALSE;
}



/*---------------------------------------------------------------------------*/
// Added  : 1999 Jul 27th
// Memo   : Expand S-format string in Lisp into sequence of word
// Input  : *list	: S-format string in Lisp
// Output : *length	: number of element word (this memory should be created by user)
// Output : **data	: sequence of word (this library execute malloc)
// return : (int)	: FAIL in case of error
/*---------------------------------------------------------------------------*/
char **expand_list_to_word (char *list, int *length)
{
  int		i, size, ret, debug=0;
  char		*charp, word[MAX_STRING], **result;
  
  if (list[0]!='(') {
    tl_warning ("expand_list_to_word");
    tl_message ("This is not list");
    return NULL;
  }
  if(debug) tl_message ("Start!");

  charp = list+1;

  for( size=0; ; )
    {
      if (*charp==')' ) break;
      ret = sscanf_word_with_pointer_move( &charp, word );
      if (ret==FALSE )      break;
      size++;
      if (tl_strmember(word, ")") ) break;
    }
  // create word element x the number of element
  if(debug) tl_message ("There is %d words", size);
  *length = size;
  result = (char **)malloc( sizeof(char *) * size);
  for( i=0; i<size; i++ )
    result[i] = (char *)malloc(MAX_STRING);
  
  charp = list+1;
  if(debug) tl_message ("Going to Step 2");
  for (i=0; i<size; i++)
    {
      sscanf_word_with_pointer_move (&charp, word);
      remove_blacket_end( word );
      if(debug) tl_message ("Now strcpy {%s}!", word);
      strcpy (result[i], word);
      if(debug) tl_message ("End of strcpy {%s}!", result[i]);
    }
  return result;
}



/*-----------------------------------------------------------------------------------*/
// Added : 1999 Aug 16th
// Memo  : transform S-format string in Lisp into Glist of glib
// Input : *str		: S-format string
// Output: (GList *)	: NULL if failed
/*-----------------------------------------------------------------------------------*/
GList *tl_list_string_to_glist (char *str)
{
  int		size, ret, debug=0;
  char		*charp, *word, buf[MAX_STRING];
  GList		*glist=NULL;

  if(debug) tl_message ("Src = {%s}", str);
  g_return_val_if_fail (str != NULL, NULL);
  g_return_val_if_fail (str[0] == '(', NULL);

  charp = str+1;
  
  for (size=0; ;)
    {
      if (*charp==')' ) break;
      ret = sscanf_word_with_pointer_move( &charp, buf );
      if (ret==FALSE )      break;
      if (buf[strlen(buf)-1]==')' ) buf[strlen(buf)-1] = '\0';
      size++;
      if(debug) tl_message("Expansion : %s", buf);
      word = g_strdup (buf);
      glist = g_list_append( glist, word );
      if (tl_strmember(word, ")") ) break;
    }
  return glist;
}


/*-----------------------------------------------------------------------------------*/
// Created on 2008-01-22 by inamura
// Modified on 2008-10-05 by inamura : change 3rd parm. of g_strsplit to 0
// Function : Convert line string of csv format into Glist by glib
// Input	: *str string written by csv
// Output	: (GList *) 
/*-----------------------------------------------------------------------------------*/
GList *tl_csv_to_glist (char *str)
{
  int		num, debug=0;
  char		*charp, *word;
  gchar		**result;
  GList		*glist=NULL;

  if(debug) tl_message ("Src = {%s}", str);
  g_return_val_if_fail (str != NULL, NULL);

  charp = str;
  num = string_count (str, ',') + 1;
  result = g_strsplit (str, ",", 0);
  if(debug) cerr << "number of factor = " << num << endl;
  
  for (int i=0; i<num; i++)
    {
      if(debug) tl_message ("step %d-1: result[%d] = %s", i, i, result[i]);
      word = g_strdup (result[i]);
      //strcpy (word, result[i]);
      if(debug) tl_message ("step %d-2: add %s into glist", i, (char *)word);
      glist = g_list_append (glist, word);
      //g_list_append (glist, word);
      if(debug) tl_message ("step %d-3: length = %d: contents %s", i, (int)g_list_length(glist), (char *)g_list_nth_data(glist, i));
    }
  return glist;
}


/*-----------------------------------------------------------------------------------*/
// Created      : 2008-10-07 by inamura
// Function     : Another version of tl_csv_to_glist, only the input arguument differs
// Input	: *str string written by csv
// Output	: (GList *) 
/*-----------------------------------------------------------------------------------*/
GList *tl_word_array_to_glist (char *str, char elim)
{
  int		num, debug=0;
  char		*charp, *word, eliminator[2];
  gchar		**result;
  GList		*glist=NULL;

  if(debug) tl_message ("Src = {%s}", str);
  g_return_val_if_fail (str != NULL, NULL);

  eliminator[0] = elim;	eliminator[1] = '\0';
  
  charp = str;
  num = string_count (str, eliminator[0]) + 1;
  result = g_strsplit (str, eliminator, 0);
  if(debug) cerr << "number of factor = " << num << endl;
  
  for (int i=0; i<num; i++)
    {
      if(debug) tl_message ("step %d-1: result[%d] = %s", i, i, result[i]);
      word = g_strdup (result[i]);
      //strcpy (word, result[i]);
      if(debug) tl_message ("step %d-2: add %s into glist", i, (char *)word);
      glist = g_list_append (glist, word);
      //g_list_append (glist, word);
      if(debug) tl_message ("step %d-3: length = %d: contents %s", i, (int)g_list_length(glist), (char *)g_list_nth_data(glist, i));
    }
  return glist;
}


/*-----------------------------------------------------------------------------------*/
// Added : 1999 Aug 20th
// Memo  : Execute get keyword using GList
// Input : *list	: target List
// Input : *keyword	: searching keyword
// Output: *result	: result string (this memory should be created by user)
/*-----------------------------------------------------------------------------------*/
int tl_glist_get_keyword (GList *list, char *keyword, char *result)
{
  int		i;
  GList		*key_list=NULL, *target_list=NULL;

  for (i=0; i<(int)(g_list_length(list)); i++ )
    {
      if (tl_strmember( (char *)(g_list_nth( list, i )->data), keyword ))
	key_list = g_list_nth( list, i );
    }
  target_list = g_list_next (key_list);

  if (target_list==NULL) {
    result = NULL;
    return FALSE;
  }
  strcpy (result, (char *)(target_list->data));
  return TRUE;
}



// Added : 1999 Aug 20th
int tl_glist_print (GList *list)
{
  int		i;
  int		debug=0;

  tl_return_val_if_fail (list, "list pointer is NULL", FALSE);

  if(debug) tl_message ("start");
  cerr << "List = (";

  for (i=0; i<(int)(g_list_length(list)); i++)
    cerr << g_list_nth( list, i )->data;
  cerr << endl;
  return TRUE;
}



/*-----------------------------------------------------------------------------------*/
// Added : 1999 Nov 10th
// Memo  : Set random seed
// Input : seed		: if the seed equals to 0, the process ID is used as the seed
/*-----------------------------------------------------------------------------------*/
int tl_random_seed (int seed)
{
  if (seed)
    srand (seed);
  else
    srand (getpid());
  return TRUE;
}

		    

/*-----------------------------------------------------------------------------------*/
// Added  : 1999 Oct 17th
// Memo   : output random value. range will be from 0 to [upper]
// Input  : upper
// Return : (double)	: ramdom value
/*-----------------------------------------------------------------------------------*/
double tl_random (int upper)
{
  double	result;
  
  result = ((double)upper*rand()/(RAND_MAX+1.0));
  return result;
}


/*----------------------------------------------------------------------------*/
// Added : 2002 Jan 30th
/*----------------------------------------------------------------------------*/
int tlVector_SetValue (tlVector_t *vec, double *seq, int size)
{
  int		i;
  
  for (i=0; i<size; i++)
    g_array_append_val ((GArray *)vec, seq[i]);
  
  return TRUE;
}


/*----------------------------------------------------------------------------*/
// Created : 2002 Apr 17th
// Memo    : This function is too old. This should be discarded
// Output  : *seq
/*----------------------------------------------------------------------------*/
int tlVector_CopyToIntPointer (tlVector_t *vec, int *seq)
{
  int		i;
  
  tl_return_val_if_fail (vec, "vec is NULL", FALSE);

  for (i=0; i<tlVector_Size (vec); i++)
    seq[i] = (int)(tlVector_Index (vec, i));
  
  return TRUE;
}



/*----------------------------------------------------------------------------*/
// Added   : 2002 Aug 19th
// Memo    : This function is too old. This should be discarded
/*----------------------------------------------------------------------------*/
int tlVector_CopyToDoublePointer (tlVector_t *vec, double *seq)
{
  int		i;
  
  tl_return_val_if_fail (vec, "vec is NULL", FALSE);

  for (i=0; i<tlVector_Size (vec); i++)
    seq[i] = tlVector_Index (vec, i);
  
  return TRUE;
}


/*----------------------------------------------------------------------------*/
// Added : 2002 Jul 17th
// Memo    : This function is too old. This should be discarded
/*----------------------------------------------------------------------------*/
int tlVector_FileOut (tlVector_t *vec, char *filename)
{
  int		i;
  FILE		*fp=NULL;
  
  tl_return_val_if_fail (vec, "vec is NULL", FALSE);
  tl_return_val_if_fail (filename, "filename is NULL", FALSE);

  if (tl_fopen_as_write (&fp, filename)==FALSE) {
    tl_warning ("Ouch!");
    return FALSE;
  }

  fprintf (fp, "%d\t", tlVector_Length (vec));
  for (i=0; i<tlVector_Length (vec); i++)
    {
      fprintf (fp, "%g ", tlVector_Nth (vec, i));
    }
  fclose (fp);
  return TRUE;
}



/*----------------------------------------------------------------------------*/
// Added : 2002 Jul 17th
// Memo    : This function is too old. This should be discarded
/*----------------------------------------------------------------------------*/
int tlVector_StreamOut (tlVector_t *vec, FILE *fp)
{
  int		i;
  
  tl_return_val_if_fail (vec, "vec is NULL", FALSE);
  tl_return_val_if_fail (fp,  "file stream is NULL", FALSE);

  fprintf (fp, "%d\t", tlVector_Length (vec));
  for (i=0; i<tlVector_Length (vec); i++)
    {
      fprintf (fp, "%g ", tlVector_Nth (vec, i));
    }

  return TRUE;
}




/*----------------------------------------------------------------------------*/
// Created : 2002 Apr 19th
// Example of the contents of file
// 5   0.1 0.2 0.3 0.1 0.1
// [length]  [data] [data] [data] [data].....
/*----------------------------------------------------------------------------*/
tlVector_t *tlVector_CreateFromFile (char *filename)
{
  int		i, length;
  double	*result;
  char		ctmp[MAX_STRING], *charp;
  FILE		*fp;
  tlVector_t	*vec = NULL;

  if ((fp = fopen (filename, "r"))==NULL)
    {
      tl_warning ("No such file <%s>", filename);
      return NULL;
    }
  
  vec = tlVector_Create ();

  fgets (ctmp, MAX_STRING, fp);
  charp = ctmp;

  length = (int) sscanf_double_with_pointer_move (&charp);
  result = (double *)malloc (sizeof(double) * length);

  for (i=0; i<length; i++)
    {
      result[i] = sscanf_double_with_pointer_move (&charp);
    }
  tlVector_SetValue (vec, result, length);

  free (result);
  fclose (fp);
  
  return vec;
}



/*----------------------------------------------------------------------------*/
// Added : 2002 Aug 12th
// Memo  : creating vector from file stream
/*----------------------------------------------------------------------------*/
tlVector_t *tlVector_CreateFromStream (FILE *fp)
{
  int		i, length;
  double	*result = NULL;
  char		ctmp[MAX_STRING], *charp = NULL;
  tlVector_t	*vec = NULL;

  vec = tlVector_Create ();

  if (fgets (ctmp, MAX_STRING, fp)==NULL) return NULL;
  charp = ctmp;
  length = sscanf_int_with_pointer_move (&charp);
  result = (double *)malloc (sizeof(double) * length);

  for (i=0; i<length; i++)
    {
      result[i] = sscanf_double_with_pointer_move (&charp);
    }
  tlVector_SetValue (vec, result, length);

  free (result);
  return vec;
}




/*----------------------------------------------------------------------------*/
// Created : 2002 Apr 19th
/*----------------------------------------------------------------------------*/
tlVector_t *tlVector_CreateFromIntPointer (int *seq, int length)
{
  int		i;
  double	*dseq;
  tlVector_t	*vec;

  dseq = (double *)malloc (sizeof(double) * length);

  vec = tlVector_Create ();
  for (i=0; i<length; i++)
    dseq[i] = (double)seq[i];
  tlVector_SetValue (vec, dseq, length);

  free (dseq);
  return vec;
}



/*----------------------------------------------------------------------------*/
// Added : 2002 May 9th
// Memo  : creating tlVector from double sequence
// TODO  : not yet
/*----------------------------------------------------------------------------*/
tlVector_t *tlVector_CreateFromDoublePointer(double *seq, int length)
{
  tlVector_t *vector;
  
  vector = tlVector_Create();
  return vector;
}



/*----------------------------------------------------------------------------*/
// Added : 2002 May 9th
// TODO  : not yet
/*----------------------------------------------------------------------------*/
int tlVector_Smoothing (tlVector_t *vector)
{
  tl_return_val_if_fail (vector, "vector is NULL", FALSE);

  return TRUE;
}


/*----------------------------------------------------------------------------*/
// Added : 2002 Feb 1st
/*----------------------------------------------------------------------------*/
int tlVector_Verify (tlVector_t *vec)
{
  int		i;

  tl_return_val_if_fail (vec, "vec is NULL", FALSE);
  
  cerr << "-----------------   tlVector_Verify  (start)  -----------------" << endl;
  cerr << "      length   = " << (int)(tlVector_Size(vec)) << endl;
  cerr << "      contents = ";
  for (i=0; i<(int)(tlVector_Size(vec)); i++)
    cerr << "         %g" << tlVector_Index (vec, i);
  cerr << endl;
  cerr << "-----------------   tlVector_Verify   (end)   -----------------" << endl;

  return TRUE;
}


/*---------------------------------------------------------------------------*/
// Added : 2002 Apr 15th
// Memo  : If the target directory is not exist, it will be created automatically
/*---------------------------------------------------------------------------*/
int tl_fopen_as_write (FILE **fp, char *filename)
{
  DIR		*dp;
  char		dirname[MAX_STRING], com[MAX_STRING];
  int		debug=0;

  if(debug) tl_debug_step ("start");
  tl_pickup_directory (filename, dirname);
  
  if ((dp = opendir(dirname))==NULL) 
    {
      if(debug) tl_message ("directory <%s> is not found", dirname);
      if(debug) tl_message ("new directory is created");
      sprintf (com, "mkdir -p %s", dirname);
      //mkdir (dirname, 0755);
      system (com);
    }
  else
    {
      if(debug) tl_message ("directory <%s> is suitable", dirname);
      closedir (dp);
    }
  

  if ((*fp = fopen (filename, "w"))==NULL)
    {
      tl_warning ("File <%s> cannot created", filename);
      return FALSE;
    }
  
  if(debug) tl_debug_step ("end");
  return TRUE;
}



/*---------------------------------------------------------------------------*/
// Added : 2002 Jun 29th
// Memo  : fopen with error exception
/*---------------------------------------------------------------------------*/
int tl_fopen_as_read (FILE **fp, char *filename)
{
  if ((*fp = fopen (filename, "r"))==NULL)
    {
      tl_warning ("File <%s> cannot opened", filename);
      return FALSE;
    }
  
  return TRUE;
}



/*---------------------------------------------------------------------------*/
// Added : 2002 Aug 3rd
// Memo  : fopen with error exception and automatic complement of suffix string
/*---------------------------------------------------------------------------*/
int tl_fopen_suffix_as_read (FILE **fp, char *filename, char *suffix)
{
  int		debug=0;
  char		*real_suffix=NULL, real_filename[MAX_STRING];
  
  tl_return_val_if_fail (filename, "filename is NULL", FALSE);
  tl_return_val_if_fail (suffix,   "suffix is NULL",   FALSE);

  real_suffix = tl_get_suffix (filename);
  if(debug) tl_message ("get_suffix : %s", real_suffix);
  if(debug) tl_message ("filename   : %s", filename);
  if(debug) tl_message ("suffix     : %s", suffix);
  
  // Checking suffix: If there is no target suffix, automatically complemented
  if (real_suffix==NULL)
    {
      if(debug) tl_message ("suffix is not exist");
      sprintf (real_filename, "%s.%s", filename, suffix);
    }
  else if (strcmp (real_suffix, suffix))
    {
      // Checking suffix: If input suffix is not the same as the target, return FALSE
      tl_warning ("suffix didn't match; <%s> is needed but <%s> is the real suffix", suffix, real_suffix);
      return FALSE;
    }
  else
    {
      if(debug) tl_message ("through : filename = %s", filename);
      strcpy (real_filename, filename);
    }
  if(debug) tl_message ("final filename   : %s", real_filename);
  if ((*fp = fopen (real_filename, "r"))==NULL)
    {
      tl_warning ("File <%s> cannot opened", real_filename);
      return FALSE;
    }
  if(debug) tl_debug_step ("end");
  return TRUE;
}



/*---------------------------------------------------------------------------*/
// Added : 2002 Aug 16th
// Memo  : suffix will be automatically complemented as same as read
/*---------------------------------------------------------------------------*/
int tl_fopen_suffix_as_write (FILE **fp, char *filename, char *suffix)
{
  int		debug=0;
  char		*real_suffix=NULL, real_filename[MAX_STRING];
  
  tl_return_val_if_fail (filename, "filename is NULL", FALSE);
  tl_return_val_if_fail (suffix,   "suffix is NULL",   FALSE);

  // checking suffix. If there is no suffix, automatically complemented
  if (tl_get_suffix (filename)==NULL) {
    sprintf (real_filename, "%s.%s", filename, suffix);
  }
  else if (strcmp (tl_get_suffix (filename), suffix))
    {
      // Checking suffix: If input suffix is not the same as the target, return FALSE
      tl_warning ("suffix didn't match; <%s> is needed but <%s> is the real suffix", suffix, real_suffix);
      return FALSE;
    }
  else
    strcpy (real_filename, filename);

  if(debug) tl_message ("filename   : %s", filename);
  if(debug) tl_message ("suffix     : %s", suffix);
  if(debug) tl_message ("get_suffix : %s", tl_get_suffix(filename));
  if(debug) tl_message ("final filename : %s", real_filename);
  if (tl_fopen_as_write (fp, real_filename)==FALSE)
    {
      tl_warning ("File <%s> cannot opened", real_filename);
      return FALSE;
    }
  
  return TRUE;
}



int tl_check_file_existence (char *filename)
{
  FILE		*fp;
  
  if ((fp = fopen (filename, "r"))==NULL)
    return FALSE;

  fclose (fp);
  return TRUE;
}


// Created on 2008-09-27 by inamura
int tl_glist_search_string (GList *target_list, char *target_string)
{
  int	debug=0;
  
  for (int i=0; i<(int)g_list_length (target_list); i++)
    {
      if(debug) tl_message ("check %s with %s", (char *)g_list_nth_data (target_list, i), target_string);
      if (!strcmp ((char *)g_list_nth_data (target_list, i), target_string))
	return i;
    }
  return -1;
}


/*-----------------------------------------------------------------------------------*/
// Memo : Debugging message
/*-----------------------------------------------------------------------------------*/
int d_print (char const *format, ...)
{
  char		s[MAX_STRING];
  va_list	list;

  va_start(list, format);
  vsprintf(s, format, list);
  va_end(list);
  fprintf (stderr, "%s", s);
  fflush (stderr);
  return TRUE;
}



/*-----------------------------------------------------------------------------------*/
// Memo  : Debugging message
//         if the first argument flag is TRUE, then message will be appeared
/*-----------------------------------------------------------------------------------*/
int d_printf (int flag, const char *format, ...)
{
  char		s[MAX_STRING];
  va_list		list;

  if (flag)
    {
      va_start(list, format);
      vsprintf(s, format, list);
      va_end(list);
      fprintf( stderr, "%s", s );
    }
  return TRUE;
}
