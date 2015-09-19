// --*- C++ -*--
/*
 *	C Modules  :  core library
 *	iirlib/iirlib.h
 *      
 *	Last modified on 2008-10-07 by inamura
 *
 *	Copyright (c) Tetsunari Inamura 1999--2008.
 *	All Rights Reserved.
 */

#ifndef __IIRLIB_H__
#define __IIRLIB_H__


#include <fstream>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cmath>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <dirent.h>
#include "glib.h"
#include "threshold.h"

using namespace std;

#ifndef TRUE
#define TRUE		1
#endif
#ifndef FALSE
#define FALSE		-1
#endif

#define DEG2RAD(x)		((x)*M_PI/180.0)
#define RAD2DEG(x)		((x)*180.0/M_PI)

#ifndef uchar
#define uchar	unsigned char
#endif

#define MAX_STRING		20000


/*---------------------------------------------------------------------------*/
// Added on 2002 Jan 30th
// Memo : getting a hist from glib warning
/*---------------------------------------------------------------------------*/
#define	tl_warning(format, args...)	G_STMT_START{	\
		fprintf(stderr, "\n\n[%s:line.%d] <%s> ", __FILE__, __LINE__, __PRETTY_FUNCTION__);	\
		g_log (G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, format, ##args);		\
		fprintf(stderr, "\n\n");}G_STMT_END

#define	tl_message(format, args...)	G_STMT_START{	\
   fprintf(stderr, "[%s:line.%d] <%s> : ", __FILE__, __LINE__, __PRETTY_FUNCTION__);\
   g_log (G_LOG_DOMAIN, G_LOG_LEVEL_MESSAGE, format, ##args);}G_STMT_END

	   

/*---------------------------------------------------------------------------*/
// Added on 2002 Jan 30th
// Memo : got a hint from glib's warning function
/*---------------------------------------------------------------------------*/
#define tl_return_val_if_fail(expr,mes,val)  G_STMT_START{	\
     if (!(expr))							\
       {								\
	 tl_warning(mes);						\
	 return val;							\
       };				}G_STMT_END


/*---------------------------------------------------------------------------*/
// Added : 2002 Feb 17th
// Memo  : old debugging function
/*---------------------------------------------------------------------------*/
#define tl_debug_step(mes)  G_STMT_START{	\
		fprintf(stderr, "[%s:line.%d] <%s> Step %s\n", __FILE__, __LINE__, \
	   __PRETTY_FUNCTION__, mes);    }G_STMT_END


//#ifdef __cplusplus
//extern "C" {
//#endif


typedef int (*Func_Ptr)(char *dummy);
typedef int (*Func_Ptr_with_int)(void *dummy1, int dummy2);
typedef int (*Func_Ptr_with_arg)(void *dummy);
typedef int (*Func_Ptr_with_2arg)(void *dummy1, void *dummy2);

typedef struct {
	char		*name;
	Func_Ptr	func;
}
functable_t,	*functable_ptr;


typedef struct {
	char			*name;
	Func_Ptr_with_int	func;
}
functable_with_int_t,	*functable_with_int_ptr;


typedef struct {
	char			*name;
	Func_Ptr_with_arg	func;
}
functable_with_arg_t,	*functable_with_arg_ptr;


typedef struct {
	char			*name;
	Func_Ptr_with_2arg	func;
}
functable_with_2arg_t,	*functable_with_2arg_ptr;


typedef struct
{
	double	key;	// a value to be referred
	int		info;	// ID
}
bubble_data;


typedef GArray	tlVector_t;
#define tlVector_Nth(a,i)	(((double *)((GArray *)a)->data) [(i)])
#define tlVector_Index(a,i)	(((double *)((GArray *)a)->data) [(i)])
#define tlVector_Size(a)	(int)(a->len)
#define tlVector_Length(a)	(int)(a->len)
#define tlVector_Create()	(tlVector_t *)g_array_new(FALSE, FALSE, sizeof(double))
#define tlVector_Delete(vec)	g_array_free(vec,FALSE)
#define tlVector_Destroy(vec)	g_array_free(vec,FALSE)

#define tl_sscanf_double_with_pointer_move(arg)  sscanf_double_with_pointer_move(arg)
#define tl_pickup_directory(arg1,arg2)		 pickup_directory(arg1,arg2)

int      tl_strmember			(const char *reference, const char *search);
GString	*tl_string_assoc		(char *src, char *keyword);
char	*tl_pickup_last_filename	(char *fullpath);
int		 tl_string_getkeyword		(char *src, char *keyword, char *result);
void     remove_backslash_N		(char *string );
int      tl_remove_double_q		(char *str, char *str2);
int		remove_dot			(char *string);
void		add_backslash_N			(char *sentence);
int		sscanf_int_with_pointer_move	(char **charp);
double		sscanf_double_with_pointer_move	(char **charp);
int		sscanf_word_with_pointer_move	(char **charp, char *word);
int		d_print				(const char *format, ...);
int		d_printf			(int flag, const char *format, ...);
int		remove_blacket_end		(char *string);
int		pickup_directory		(char *fullpath, char *dir );
int		max_between			(int num, int *buf);
int		max_between			(vector<double> vec);
int		min_between			(int num, int *buf);
int		min_natural_number_between	(int num, int *buf);
int		pointer_swap			(int *p1, int *p2);
void		normalize_vector		(int dim, double *src_vec, double *dst_vec);
char    	*tl_fgets_without_sharp		(char *str, int n, FILE *fp);
int	 	tl_search_1st_2nd		(int num, double *buf, int *first_no, double *first_value,
						 int *second_no, double *second_value);
int 		string_count			(char *string, char c);
int		integerp			(char *string );
int		tl_atoi				(char *string, int *value);
int		exec_functable			(functable_t *ft, char *string);
int		exec_functable_with_arg		(functable_with_arg_t *ft, char *string, void *arg);
int		exec_functable_with_name	(functable_with_arg_t *ft, char *string, char *name);
int		exec_functable_with_int		(functable_with_int_t *ft, char *string, int value);
int		pointer_swap_void		(void **p1, void **p2);
int		tl_skip_pointer			(char **charp);
int		sscanf_int_with_seeking		(char **charp, int *result);
int		sscanf_double_with_seeking	(char **charp, double *data);
int		 expand_list_to_int		(char *list, int *length, int *data);
int		expand_list_to_double		(char *list, int *length, double *data);
char		**expand_list_to_word		(char *list, int *length);
int		 bubbleSort			(bubble_data *data, int nmem, int asdes);
int		 tl_Warning			(char *process, char *function);
GList		*tl_list_string_to_glist	(char *str);
GList		*tl_csv_to_glist		(char *str);
GList		*tl_word_array_to_glist		(char *str, char eliminator);
int		 tl_glist_get_keyword		(GList *list, char *keyword, char *result);
int		 tl_glist_print			(GList *list);
double		 tl_random			(int upper);
int		 tl_random_seed			(int seed);
char		*tl_pickup_terminal_filename	(char *fullpath);
char		*tl_get_suffix			(char *filename);
int		 tl_add_suffix			(GString *filename, char *suffix);
int		 tl_remove_suffix		(GString *filename);
int		 tl_fopen_as_write		(FILE **fp, char *filename);
int		 tl_fopen_as_read 		(FILE **fp, char *filename);
int		 tl_fopen_suffix_as_read	(FILE **fp, char *filename, char *suffix);
int		 tl_fopen_suffix_as_write 	(FILE **fp, char *filename, char *suffix);
int		 tl_check_file_existence	(char *filename);
int		 tl_glist_search_string		(GList *target_list, char *target_string);
int		 tlVector_Verify		(tlVector_t *vec);
int		 tlVector_SetValue		(tlVector_t *vec, double *seq, int size);
int		 tlVector_CopyToIntPointer	(tlVector_t *vec, int *seq);
int		 tlVector_CopyToDoublePointer	(tlVector_t *vec, double *seq);
tlVector_t	*tlVector_CreateFromFile	(char *filename);
tlVector_t	*tlVector_CreateFromStream	(FILE *fp);
tlVector_t	*tlVector_CreateFromIntPointer	(int *seq, int length);
tlVector_t	*tlVector_CreateFromDoublePointer(double *seq, int length);
int	         tlVector_FileOut		(tlVector_t *vec, char *filename);
int	         tlVector_StreamOut		(tlVector_t *vec, FILE *fp);
int          tlVector_Smoothing		(tlVector_t *vector);


//#ifdef __cplusplus
//}
//#endif

#endif /* __IIRLIB_H */
