#if defined(i386)
#define NEED_I386_TAS_ASM
#endif
#if defined(sparc)
#define NEED_SPARC_TAS_ASM
#endif
#define USE_POSIX_TIME
#define HAS_TEST_AND_SET
typedef unsigned char slock_t;
