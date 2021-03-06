/* include/config.h.  Generated automatically by configure.  */


/* the purpose of this file is to reduce the use of #ifdef's through
 * the code base by those porting the software, and to facilitate the
 * eventual use of autoconf to build the server 
 */

#ifndef CONFIG_H
#define CONFIG_H

#define BLCKSZ	8192

/* 
 * The following is set using configure.  
 */

/* Set to 1 if you have <string.h> */
#define HAVE_STRING_H 1

/* Set to 1 if you have <strings.h> */
#define HAVE_STRINGS_H 1

/* Set to 1 if you have <getopt.h> */
#define HAVE_GETOPT_H 1

/* Set to 1 if you have <fp_class.h> */
/* #undef HAVE_FP_CLASS_H */

/* Set to 1 if you have <netinet/in.h> */
#define HAVE_NETINET_IN_H 1

/* Set to 1 if you have <ieeefp.h> */
/* #undef HAVE_IEEEFP_H */

/* Set to 1 if you have <arpa/inet.h> */
#define HAVE_ARPA_INET_H 1

/* Set to 1 if you have <netdb.h> */
#define HAVE_NETDB_H 1

/* Set to 1 if you have <endian.h> */
#define HAVE_ENDIAN_H 1

/* Set to 1 if you have <crypt.h> */
/* #undef HAVE_CRYPT_H */

/* Set to 1 if you have <termios.h> */
#define HAVE_TERMIOS_H 1

/* Set to 1 if you have <float.h> */
#define HAVE_FLOAT_H 1

/* Set to 1 if you have <limits.h> */
#define HAVE_LIMITS_H 1

/* Set to 1 if  you have <values.h> */
#define HAVE_VALUES_H 1

/* Set to 1 if  you have <sys/select.h> */
/* #undef HAVE_SYS_SELECT_H */

/* Set to 1 if you have <readline.h> */
#define HAVE_READLINE_H 1

/* Set to 1 if you have <history.h> */
#define HAVE_HISTORY 1

/* Set to 1 if you have <readline/history.h> */
#define HAVE_READLINE_HISTORY_H 1

/* Set to 1 if you have <dld.h> */
/* #undef HAVE_DLD_H */

/* Set to 1 if you have fp_class() */
/* #undef HAVE_FP_CLASS */

/* Set to 1 if you have class() */
/* #undef HAVE_CLASS */

/* Set to 1 if you have fp_class_d() */
/* #undef HAVE_FP_CLASS_D */

/* Set to 1 if you have fpclass() */
/* #undef HAVE_FPCLASS */

/* Set to 1 if you have isinf() */
#define HAVE_ISINF 1
#ifndef HAVE_ISINF
int isinf(double x);
#endif

/* Set to 1 if you have tzset() */
#define HAVE_TZSET 1

/* Set to 1 if you have gethostname() */
#define HAVE_GETHOSTNAME 1
#ifndef HAVE_GETHOSTNAME
int  gethostname(char *name, int namelen);
#endif

/* Set to 1 if you have int timezone */
#define HAVE_INT_TIMEZONE 1

/* Set to 1 if you have cbrt() */
#define HAVE_CBRT 1

/* Set to 1 if you have inet_aton() */
#define HAVE_INET_ATON 1
#ifndef HAVE_INET_ATON
# ifdef HAVE_ARPA_INET_H
#  ifdef HAVE_NETINET_IN_H
#   include <sys/types.h>
#   include <netinet/in.h>
#  endif
#  include <arpa/inet.h>
# endif
extern int  inet_aton(const char *cp, struct in_addr * addr);
#endif

/* Set to 1 if you have fcvt() */
#define HAVE_FCVT 1

/* Set to 1 if you have strerror() */
#define HAVE_STRERROR 1

/* Set to 1 if you have rint() */
#define HAVE_RINT 1 

/* Set to 1 if you have memmove() */
#define HAVE_MEMMOVE 1

/* Set to 1 if you have sigsetjmp() */
/* #undef HAVE_SIGSETJMP */

/* Set to 1 if you have kill() */
#define HAVE_KILL 1

/* Set to 1 if you have vfork() */
#define HAVE_VFORK 1

/* Set to 1 if you have sysconf() */
#define HAVE_SYSCONF 1

/* Set to 1 if you have getrusage() */
#define HAVE_GETRUSAGE 1

/* Set to 1 if you have waitpid() */
#define HAVE_WAITPID 1

/* Set to 1 if you have setsid() */
#define HAVE_SETSID 1

/* Set to 1 if you have sigprocmask() */
#define HAVE_SIGPROCMASK 1

/* Set to 1 if you have sigprocmask() */
#define HAVE_STRCASECMP 1
#ifndef HAVE_STRCASECMP
extern int  strcasecmp(char *s1, char *s2);
#endif

/* Set to 1 if you have strtol() */
#define HAVE_STRTOL 1

/* Set to 1 if you have strtoul() */
#define HAVE_STRTOUL 1

/* Set to 1 if you have strdup() */
#define HAVE_STRDUP 1
#ifndef HAVE_STRDUP
extern char *strdup(char const *);
#endif

/* Set to 1 if you have random() */
#define HAVE_RANDOM 1
#ifndef HAVE_RANDOM
extern long random(void);
#endif

/* Set to 1 if you have srandom() */
#define HAVE_SRANDOM 1
#ifndef HAVE_SRANDOM
extern void srandom(int seed);
#endif

/* Set to 1 if you have libreadline.a */
#define HAVE_LIBREADLINE 1

/* Set to 1 if you have libhistory.a */
/* #undef HAVE_LIBHISTORY */

/* Set to 1 if you have union semun */
#define HAVE_UNION_SEMUN 1

/* Set to 1 if you want to USE_LOCALE */
/* #undef USE_LOCALE */

/* Set to 1 if you want CYR_RECODE (cyrillic recode) */
/* #undef CYR_RECODE */

/* Set to 1 if you want to Disable ASSERT CHECKING */
/* #undef NO_ASSERT_CHECKING */

/*
 * Code below this point should not require changes
 */

#include "os.h"

/*
 * The following is used as the arg list for signal handlers.  Any ports
 * that take something other than an int argument should change this in
 * the port specific makefile.  Note that variable names are required
 * because it is used in both the prototypes as well as the definitions.
 * Note also the long name.  We expect that this won't collide with
 * other names causing compiler warnings.
 */ 

#ifndef       SIGNAL_ARGS
#  define SIGNAL_ARGS int postgres_signal_arg
#endif

/* 
 * DEF_PGPORT is the TCP port number on which the Postmaster listens by
 * default.  This can be overriden by command options, environment variables,
 * and the postconfig hook. (set by build script)
 */ 

#define DEF_PGPORT "5432" 

/*
 * If you do not plan to use Host based authentication,
 * comment out the following line (set by build script)
 */
/* #undef HBA */

/*
 * On architectures for which we have not implemented spinlocks (or
 * cannot do so), we use System V semaphores.  We also use them for
 * long locks.  For some reason union semun is never defined in the
 * System V header files so we must do it ourselves.
 */

/*  Debug and various "defines" that should be documented */

/* found in function aclparse() in src/backend/utils/adt/acl.c */
/* #define ACLDEBUG */

/* found in src/backend/utils/adt/arrayfuncs.c
   code seems broken without it, Bruce Momjian */
/* #define LOARRAY */

/*
 * As soon as the backend blocks on a lock, it waits this number of seconds
 * before checking for a deadlock.  If not, it keeps checking every this
 * number of seconds.
 * We don't check for deadlocks just before sleeping because a deadlock is
 * a rare event, and checking is an expensive operation.
 */
#define DEADLOCK_CHECK_TIMER 60

/*
 * This flag enables the use of idexes in plans generated for function
 * executions which normally are always executed with sequential scans.
 */
#define INDEXSCAN_PATCH 

/* #define DATEDEBUG */

/*
 * Define this if you want to use date constants with a short year
 * like '01/05/96'.
 */
/* #define USE_SHORT_YEAR */

/*
 * defining unsafe floats's will make float4 and float8
 * ops faster at the cost of safety, of course!        
 */
/* #define UNSAFE_FLOATS */

/*
 * There is a bug in the function executor. The backend crashes while trying to
 * execute an sql function containing an utility command (create, notify, ...).
 * The bug is part in the planner, which returns a number of plans different
 * than the number of commands if there are utility commands in the query, and
 * in part in the function executor which assumes that all commands are normal
 * query commands and causes a SIGSEGV trying to execute commands without plan.
 */
#define FUNC_UTIL_PATCH

/*
 * Define this if you want to retrieve arrays attributes as Tcl lists instead
 * of postgres C-like arrays, for example {{"a1" "a2"} {"b1" "b2"}} instead 
 * of {{"a1","a2"},{"b1","b2"}}.
 */
#define TCL_ARRAYS

/*
 * The comparison routines for text and char data type give incorrect results
 * if the input data contains characters greater than 127.  As these routines
 * perform the comparison using signed char variables all character codes
 * greater than 127 are interpreted as less than 0.  These codes are used to
 * encode the iso8859 char sets.  Define this flag to correct the problem.
 */
#define UNSIGNED_CHAR_TEXT

/*
 * The following flag allows limiting the number of rows returned by a query.
 * You will need the loadable module utils.c to use this feature.
 */
#define QUERY_LIMIT

/*
 * The following flag allows copying tables from files with number of columns
 * different than the number of attributes setting missing attributes to NULL
 * and ignoring extra columns.  This also avoids the shift of the attributes
 * of the rest of the file if one line has a wrong column count.
 */
#define COPY_PATCH

/*
 * User locks are handled totally on the application side as long term
 * cooperative locks which extend beyond the normal transaction boundaries.
 * Their purpose is to indicate to an application that someone is `working'
 * on an item.  Define this flag to enable user locks.  You will need the
 * loadable module user-locks.c to use this feature.
 */
#define USER_LOCKS

/* Debug #defines */
/* #define IPORTAL_DEBUG  */
/* #define HEAPDEBUGALL  */
/* #define ISTRATDEBUG  */
/* #define FASTBUILD_DEBUG */
#define RTDEBUG 
#define GISTDEBUG 
/* #define DEBUG_RECIPE */
/* #define ASYNC_DEBUG */
/* #define COPY_DEBUG */
/* #define VACUUM_DEBUG */
/* #define NBTINSERT_PATCH_DEBUG */


/* The following don't have any apparent purpose, but are in the
 * code.  someday, will take them out altogether, but for now, 
 * document them here
 */
/* #define OMIT_PARTIAL_INDEX */
/* #define NO_BUFFERISVALID   */
/* #define NO_SECURITY        */
/* #define TIOGA              */
/* #define OLD_REWRITE        */
/* #define NOTYET             */

/* Genetic Query Optimization (GEQO):
 * 
 * The GEQO module in PostgreSQL is intended for the solution of the
 * query optimization problem by means of a Genetic Algorithm (GA).
 * It allows the handling of large JOIN queries through non-exhaustive
 * search.
 * For further information see README.GEQO <utesch@aut.tu-freiberg.de>.
 */
#define GEQO /* backend/optimizer/path/allpaths.c */

/*
 * Define this if you want psql to _always_ ask for a username and a password
 * for password authentication.
 */
/* #define PSQL_ALWAYS_GET_PASSWORDS */

/*
 * Use btree bulkload code: 
 * this code is moderately slow (~10% slower) compared to the regular
 * btree (insertion) build code on sorted or well-clustered data.  on
 * random data, however, the insertion build code is unusable -- the
 * difference on a 60MB heap is a factor of 15 because the random
 * probes into the btree thrash the buffer pool.
 *
 * Great thanks to Paul M. Aoki (aoki@CS.Berkeley.EDU)
 */
#define FASTBUILD /* access/nbtree/nbtsort.c */

/* 
 * BTREE_VERSION_1: we must guarantee that all tuples in A LEVEL
 * are unique, not in ALL INDEX. So, we can use bti_itup->t_tid                
 * as unique identifier for a given index tuple (logical position
 * within a level) and take off bti_oid & bti_dummy (8 bytes total)
 * from btree items.
 */
#define BTREE_VERSION_1

/*
 * TBL_FREE_CMD_MEMORY: free memory allocated for an user query inside
 * transaction block after this query is done. 
 */
#define TBL_FREE_CMD_MEMORY

#endif /* CONFIG_H */
