/* src/tclXconfig.h.  Generated from tclXconfig.h.in by configure.  */
/* src/tclXconfig.h.in.  Generated automatically from configure.in by autoheader.  */

/* Define to the type of elements in the array set by `getgroups'.
   Usually this is either `int' or `gid_t'.  */
/* #undef GETGROUPS_T */

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef gid_t */

/* Define if your struct tm has tm_zone.  */
#define HAVE_TM_ZONE 1

/* Define if you don't have tm_zone but do have the external array
   tzname.  */
/* #undef HAVE_TZNAME */

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef mode_t */

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef pid_t */

/* Define as the return type of signal handlers (int or void).  */
#define RETSIGTYPE void

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
/* #undef size_t */

/* Define if you can safely include both <sys/time.h> and <time.h>.  */
#define TIME_WITH_SYS_TIME 1

/* Define if your <sys/time.h> declares struct tm.  */
/* #undef TM_IN_SYS_TIME */

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef uid_t */

/* Define if you don't have sys/socket.h */
/* #undef NO_SYS_SOCKET_H */

/* Define if you have and need sys/select.h */
/* #undef HAVE_SYS_SELECT_H */

/* Define if don't you have values.h */
/* #undef NO_VALUES_H */

/* Define if your struct tm has tm_zone.  */
#define HAVE_TM_ZONE 1

/* Define if you have the timezone variable.  */
#define HAVE_TIMEZONE_VAR 1

/* Define if your struct tm has tm_tzadj.  */
/* #undef HAVE_TM_TZADJ */

/* Define if your struct tm has tm_gmtoff.  */
#define HAVE_TM_GMTOFF 1

/* Define if you don't have tm_zone but do have the external array
   tzname.  */
/* #undef HAVE_TZNAME */

/* Define as the return type of signal handlers (int or void).  */
#define RETSIGTYPE void

/* Define if your sys/time.h declares struct tm.  */
/* #undef TM_IN_SYS_TIME */

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef uid_t */
/* #undef gid_t */
/* #undef pid_t */
/* #undef mode_t */
/* #undef time_t */

/* Define to the type of elements in the array set by `getgroups'.
   Usually this is either `int' or `gid_t'.  */
/* #undef GETGROUPS_T */

/* Define if you do not have dirent.h */
/* #undef NO_DIRENT_H */

/* Define if you do not have readdir but you do have V7-style directories */
/* #undef USE_DIRENT2_H */

/* Define if you do not have float.h */
/* #undef NO_FLOAT_H */

/* Define if you do not have limits.h */
/* #undef NO_LIMITS_H */

/* Define if you do not have stdlib.h */
/* #undef NO_STDLIB_H */

/* Define if you do not have string.h */
/* #undef NO_STRING_H */

/* Define if you do not have sys/time.h */
/* #undef NO_SYS_TIME_H */

/* Define if you do not have sys/wait.h */
/* #undef NO_SYS_WAIT_H */

/* Define if time.h can be included with sys/time.h */
#define TIME_WITH_SYS_TIME 1

/* Define if wait does not return a union */
/* #undef NO_UNION_WAIT */

/* Define if "times" returns the elasped real time */
#define TIMES_RETS_REAL_TIME 1

/* Define if gettimeofday is not available */
/* #undef NO_GETTOD */

/* Define if errno.h is not available */
/* #undef NO_ERRNO_H */

/* Define if fd_set is not defined. */
/* #undef NO_FD_SET */

/* Define if stdlib.h defines random */
#define STDLIB_DEFS_RANDOM 1

/* Define if we must use our own version of random */
/* #undef NO_RANDOM */

/* Define if you have bcopy.  */
#define HAVE_BCOPY 1

/* Define if you have bzero.  */
#define HAVE_BZERO 1

/* Define if you have catgets.  */
#define HAVE_CATGETS 1

/* Define if you have fsync.  */
#define HAVE_FSYNC 1

/* Define if you have getcwd.  */
#define HAVE_GETCWD 1

/* Define if you have gethostbyname.  */
#define HAVE_GETHOSTBYNAME 1

/* Define if you have getpriority.  */
#define HAVE_GETPRIORITY 1

/* Define if you have gettimeofday.  */
#define HAVE_GETTIMEOFDAY 1

/* Define if you have select.  */
#define HAVE_SELECT 1

/* Define if you have setitimer.  */
#define HAVE_SETITIMER 1

/* Define if you have setpgid.  */
#define HAVE_SETPGID 1

/* Define if you have setvbuf.  */
#define HAVE_SETVBUF 1

/* Define if you have sigaction.  */
#define HAVE_SIGACTION 1

/* Define if you have the <unistd.h> header file.  */
#define HAVE_UNISTD_H 1

/* Define if you have the X11 library (-lX11).  */
/* #undef HAVE_LIBX11 */

/* Define if you have the X11_s library (-lX11_s).  */
/* #undef HAVE_LIBX11_S */

/* Define if you have the Xbsd library (-lXbsd).  */
/* #undef HAVE_LIBXBSD */

/* Define if you have the Xwindow library (-lXwindow).  */
/* #undef HAVE_LIBXWINDOW */

/* Define if you have the bsd library (-lbsd).  */
/* #undef HAVE_LIBBSD */

/* Define if you have the c_s library (-lc_s).  */
/* #undef HAVE_LIBC_S */

/* Define if you have the dl library (-ldl).  */
/* #undef HAVE_LIBDL */

/* Define if you have the dnet_stub library (-ldnet_stub).  */
/* #undef HAVE_LIBDNET_STUB */

/* Define if you have the ieee library (-lieee).  */
/* #undef HAVE_LIBIEEE */

/* Define if you have the intl library (-lintl).  */
/* #undef HAVE_LIBINTL */

/* Define if you have the nsl library (-lnsl).  */
/* #undef HAVE_LIBNSL */

/* Define if you have the socket library (-lsocket).  */
/* #undef HAVE_LIBSOCKET */

/* Define if you have the sun library (-lsun).  */
/* #undef HAVE_LIBSUN */
