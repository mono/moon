# 1 "check-has-hidden-symbols.c"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "check-has-hidden-symbols.c"
# 1 "cairoint.h" 1
# 50 "cairoint.h"
# 1 "../config.h" 1
# 51 "cairoint.h" 2


# 1 "/usr/include/assert.h" 1 3 4
# 37 "/usr/include/assert.h" 3 4
# 1 "/usr/include/features.h" 1 3 4
# 322 "/usr/include/features.h" 3 4
# 1 "/usr/include/sys/cdefs.h" 1 3 4
# 324 "/usr/include/sys/cdefs.h" 3 4
# 1 "/usr/include/bits/wordsize.h" 1 3 4
# 325 "/usr/include/sys/cdefs.h" 2 3 4
# 323 "/usr/include/features.h" 2 3 4
# 345 "/usr/include/features.h" 3 4
# 1 "/usr/include/gnu/stubs.h" 1 3 4



# 1 "/usr/include/bits/wordsize.h" 1 3 4
# 5 "/usr/include/gnu/stubs.h" 2 3 4


# 1 "/usr/include/gnu/stubs-32.h" 1 3 4
# 8 "/usr/include/gnu/stubs.h" 2 3 4
# 346 "/usr/include/features.h" 2 3 4
# 38 "/usr/include/assert.h" 2 3 4
# 66 "/usr/include/assert.h" 3 4



extern void __assert_fail (__const char *__assertion, __const char *__file,
      unsigned int __line, __const char *__function)
     __attribute__ ((__nothrow__)) __attribute__ ((__noreturn__));


extern void __assert_perror_fail (int __errnum, __const char *__file,
      unsigned int __line,
      __const char *__function)
     __attribute__ ((__nothrow__)) __attribute__ ((__noreturn__));




extern void __assert (const char *__assertion, const char *__file, int __line)
     __attribute__ ((__nothrow__)) __attribute__ ((__noreturn__));



# 54 "cairoint.h" 2
# 1 "/usr/include/stdlib.h" 1 3 4
# 33 "/usr/include/stdlib.h" 3 4
# 1 "/usr/lib/gcc/i586-suse-linux/4.2.1/include/stddef.h" 1 3 4
# 214 "/usr/lib/gcc/i586-suse-linux/4.2.1/include/stddef.h" 3 4
typedef unsigned int size_t;
# 326 "/usr/lib/gcc/i586-suse-linux/4.2.1/include/stddef.h" 3 4
typedef long int wchar_t;
# 34 "/usr/include/stdlib.h" 2 3 4


# 96 "/usr/include/stdlib.h" 3 4


typedef struct
  {
    int quot;
    int rem;
  } div_t;



typedef struct
  {
    long int quot;
    long int rem;
  } ldiv_t;



# 140 "/usr/include/stdlib.h" 3 4
extern size_t __ctype_get_mb_cur_max (void) __attribute__ ((__nothrow__)) ;




extern double atof (__const char *__nptr)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) ;

extern int atoi (__const char *__nptr)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) ;

extern long int atol (__const char *__nptr)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) ;





__extension__ extern long long int atoll (__const char *__nptr)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) ;





extern double strtod (__const char *__restrict __nptr,
        char **__restrict __endptr)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;

# 182 "/usr/include/stdlib.h" 3 4


extern long int strtol (__const char *__restrict __nptr,
   char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;

extern unsigned long int strtoul (__const char *__restrict __nptr,
      char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;




__extension__
extern long long int strtoq (__const char *__restrict __nptr,
        char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;

__extension__
extern unsigned long long int strtouq (__const char *__restrict __nptr,
           char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;





__extension__
extern long long int strtoll (__const char *__restrict __nptr,
         char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;

__extension__
extern unsigned long long int strtoull (__const char *__restrict __nptr,
     char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;

# 279 "/usr/include/stdlib.h" 3 4
extern double __strtod_internal (__const char *__restrict __nptr,
     char **__restrict __endptr, int __group)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;
extern float __strtof_internal (__const char *__restrict __nptr,
    char **__restrict __endptr, int __group)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;
extern long double __strtold_internal (__const char *__restrict __nptr,
           char **__restrict __endptr,
           int __group)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;

extern long int __strtol_internal (__const char *__restrict __nptr,
       char **__restrict __endptr,
       int __base, int __group)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;



extern unsigned long int __strtoul_internal (__const char *__restrict __nptr,
          char **__restrict __endptr,
          int __base, int __group)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;




__extension__
extern long long int __strtoll_internal (__const char *__restrict __nptr,
      char **__restrict __endptr,
      int __base, int __group)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;



__extension__
extern unsigned long long int __strtoull_internal (__const char *
         __restrict __nptr,
         char **__restrict __endptr,
         int __base, int __group)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;
# 429 "/usr/include/stdlib.h" 3 4
extern char *l64a (long int __n) __attribute__ ((__nothrow__)) ;


extern long int a64l (__const char *__s)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) ;




# 1 "/usr/include/sys/types.h" 1 3 4
# 29 "/usr/include/sys/types.h" 3 4


# 1 "/usr/include/bits/types.h" 1 3 4
# 28 "/usr/include/bits/types.h" 3 4
# 1 "/usr/include/bits/wordsize.h" 1 3 4
# 29 "/usr/include/bits/types.h" 2 3 4


# 1 "/usr/lib/gcc/i586-suse-linux/4.2.1/include/stddef.h" 1 3 4
# 32 "/usr/include/bits/types.h" 2 3 4


typedef unsigned char __u_char;
typedef unsigned short int __u_short;
typedef unsigned int __u_int;
typedef unsigned long int __u_long;


typedef signed char __int8_t;
typedef unsigned char __uint8_t;
typedef signed short int __int16_t;
typedef unsigned short int __uint16_t;
typedef signed int __int32_t;
typedef unsigned int __uint32_t;




__extension__ typedef signed long long int __int64_t;
__extension__ typedef unsigned long long int __uint64_t;







__extension__ typedef long long int __quad_t;
__extension__ typedef unsigned long long int __u_quad_t;
# 134 "/usr/include/bits/types.h" 3 4
# 1 "/usr/include/bits/typesizes.h" 1 3 4
# 135 "/usr/include/bits/types.h" 2 3 4


__extension__ typedef __u_quad_t __dev_t;
__extension__ typedef unsigned int __uid_t;
__extension__ typedef unsigned int __gid_t;
__extension__ typedef unsigned long int __ino_t;
__extension__ typedef __u_quad_t __ino64_t;
__extension__ typedef unsigned int __mode_t;
__extension__ typedef unsigned int __nlink_t;
__extension__ typedef long int __off_t;
__extension__ typedef __quad_t __off64_t;
__extension__ typedef int __pid_t;
__extension__ typedef struct { int __val[2]; } __fsid_t;
__extension__ typedef long int __clock_t;
__extension__ typedef unsigned long int __rlim_t;
__extension__ typedef __u_quad_t __rlim64_t;
__extension__ typedef unsigned int __id_t;
__extension__ typedef long int __time_t;
__extension__ typedef unsigned int __useconds_t;
__extension__ typedef long int __suseconds_t;

__extension__ typedef int __daddr_t;
__extension__ typedef long int __swblk_t;
__extension__ typedef int __key_t;


__extension__ typedef int __clockid_t;


__extension__ typedef void * __timer_t;


__extension__ typedef long int __blksize_t;




__extension__ typedef long int __blkcnt_t;
__extension__ typedef __quad_t __blkcnt64_t;


__extension__ typedef unsigned long int __fsblkcnt_t;
__extension__ typedef __u_quad_t __fsblkcnt64_t;


__extension__ typedef unsigned long int __fsfilcnt_t;
__extension__ typedef __u_quad_t __fsfilcnt64_t;

__extension__ typedef int __ssize_t;



typedef __off64_t __loff_t;
typedef __quad_t *__qaddr_t;
typedef char *__caddr_t;


__extension__ typedef int __intptr_t;


__extension__ typedef unsigned int __socklen_t;
# 32 "/usr/include/sys/types.h" 2 3 4



typedef __u_char u_char;
typedef __u_short u_short;
typedef __u_int u_int;
typedef __u_long u_long;
typedef __quad_t quad_t;
typedef __u_quad_t u_quad_t;
typedef __fsid_t fsid_t;




typedef __loff_t loff_t;



typedef __ino_t ino_t;
# 62 "/usr/include/sys/types.h" 3 4
typedef __dev_t dev_t;




typedef __gid_t gid_t;




typedef __mode_t mode_t;




typedef __nlink_t nlink_t;




typedef __uid_t uid_t;





typedef __off_t off_t;
# 100 "/usr/include/sys/types.h" 3 4
typedef __pid_t pid_t;




typedef __id_t id_t;




typedef __ssize_t ssize_t;





typedef __daddr_t daddr_t;
typedef __caddr_t caddr_t;





typedef __key_t key_t;
# 133 "/usr/include/sys/types.h" 3 4
# 1 "/usr/include/time.h" 1 3 4
# 75 "/usr/include/time.h" 3 4


typedef __time_t time_t;



# 93 "/usr/include/time.h" 3 4
typedef __clockid_t clockid_t;
# 105 "/usr/include/time.h" 3 4
typedef __timer_t timer_t;
# 134 "/usr/include/sys/types.h" 2 3 4
# 147 "/usr/include/sys/types.h" 3 4
# 1 "/usr/lib/gcc/i586-suse-linux/4.2.1/include/stddef.h" 1 3 4
# 148 "/usr/include/sys/types.h" 2 3 4



typedef unsigned long int ulong;
typedef unsigned short int ushort;
typedef unsigned int uint;
# 195 "/usr/include/sys/types.h" 3 4
typedef int int8_t __attribute__ ((__mode__ (__QI__)));
typedef int int16_t __attribute__ ((__mode__ (__HI__)));
typedef int int32_t __attribute__ ((__mode__ (__SI__)));
typedef int int64_t __attribute__ ((__mode__ (__DI__)));


typedef unsigned int u_int8_t __attribute__ ((__mode__ (__QI__)));
typedef unsigned int u_int16_t __attribute__ ((__mode__ (__HI__)));
typedef unsigned int u_int32_t __attribute__ ((__mode__ (__SI__)));
typedef unsigned int u_int64_t __attribute__ ((__mode__ (__DI__)));

typedef int register_t __attribute__ ((__mode__ (__word__)));
# 217 "/usr/include/sys/types.h" 3 4
# 1 "/usr/include/endian.h" 1 3 4
# 37 "/usr/include/endian.h" 3 4
# 1 "/usr/include/bits/endian.h" 1 3 4
# 38 "/usr/include/endian.h" 2 3 4
# 218 "/usr/include/sys/types.h" 2 3 4


# 1 "/usr/include/sys/select.h" 1 3 4
# 31 "/usr/include/sys/select.h" 3 4
# 1 "/usr/include/bits/select.h" 1 3 4
# 32 "/usr/include/sys/select.h" 2 3 4


# 1 "/usr/include/bits/sigset.h" 1 3 4
# 24 "/usr/include/bits/sigset.h" 3 4
typedef int __sig_atomic_t;




typedef struct
  {
    unsigned long int __val[(1024 / (8 * sizeof (unsigned long int)))];
  } __sigset_t;
# 35 "/usr/include/sys/select.h" 2 3 4



typedef __sigset_t sigset_t;





# 1 "/usr/include/time.h" 1 3 4
# 121 "/usr/include/time.h" 3 4
struct timespec
  {
    __time_t tv_sec;
    long int tv_nsec;
  };
# 45 "/usr/include/sys/select.h" 2 3 4

# 1 "/usr/include/bits/time.h" 1 3 4
# 69 "/usr/include/bits/time.h" 3 4
struct timeval
  {
    __time_t tv_sec;
    __suseconds_t tv_usec;
  };
# 47 "/usr/include/sys/select.h" 2 3 4


typedef __suseconds_t suseconds_t;





typedef long int __fd_mask;
# 67 "/usr/include/sys/select.h" 3 4
typedef struct
  {






    __fd_mask __fds_bits[1024 / (8 * sizeof (__fd_mask))];


  } fd_set;






typedef __fd_mask fd_mask;
# 99 "/usr/include/sys/select.h" 3 4

# 109 "/usr/include/sys/select.h" 3 4
extern int select (int __nfds, fd_set *__restrict __readfds,
     fd_set *__restrict __writefds,
     fd_set *__restrict __exceptfds,
     struct timeval *__restrict __timeout);
# 121 "/usr/include/sys/select.h" 3 4
extern int pselect (int __nfds, fd_set *__restrict __readfds,
      fd_set *__restrict __writefds,
      fd_set *__restrict __exceptfds,
      const struct timespec *__restrict __timeout,
      const __sigset_t *__restrict __sigmask);



# 221 "/usr/include/sys/types.h" 2 3 4


# 1 "/usr/include/sys/sysmacros.h" 1 3 4
# 30 "/usr/include/sys/sysmacros.h" 3 4
__extension__
extern __inline unsigned int gnu_dev_major (unsigned long long int __dev)
     __attribute__ ((__nothrow__));
__extension__
extern __inline unsigned int gnu_dev_minor (unsigned long long int __dev)
     __attribute__ ((__nothrow__));
__extension__
extern __inline unsigned long long int gnu_dev_makedev (unsigned int __major,
       unsigned int __minor)
     __attribute__ ((__nothrow__));


__extension__ extern __inline unsigned int
__attribute__ ((__nothrow__)) gnu_dev_major (unsigned long long int __dev)
{
  return ((__dev >> 8) & 0xfff) | ((unsigned int) (__dev >> 32) & ~0xfff);
}

__extension__ extern __inline unsigned int
__attribute__ ((__nothrow__)) gnu_dev_minor (unsigned long long int __dev)
{
  return (__dev & 0xff) | ((unsigned int) (__dev >> 12) & ~0xff);
}

__extension__ extern __inline unsigned long long int
__attribute__ ((__nothrow__)) gnu_dev_makedev (unsigned int __major, unsigned int __minor)
{
  return ((__minor & 0xff) | ((__major & 0xfff) << 8)
   | (((unsigned long long int) (__minor & ~0xff)) << 12)
   | (((unsigned long long int) (__major & ~0xfff)) << 32));
}
# 224 "/usr/include/sys/types.h" 2 3 4
# 235 "/usr/include/sys/types.h" 3 4
typedef __blkcnt_t blkcnt_t;



typedef __fsblkcnt_t fsblkcnt_t;



typedef __fsfilcnt_t fsfilcnt_t;
# 270 "/usr/include/sys/types.h" 3 4
# 1 "/usr/include/bits/pthreadtypes.h" 1 3 4
# 36 "/usr/include/bits/pthreadtypes.h" 3 4
typedef unsigned long int pthread_t;


typedef union
{
  char __size[36];
  long int __align;
} pthread_attr_t;


typedef struct __pthread_internal_slist
{
  struct __pthread_internal_slist *__next;
} __pthread_slist_t;




typedef union
{
  struct __pthread_mutex_s
  {
    int __lock;
    unsigned int __count;
    int __owner;


    int __kind;
    unsigned int __nusers;
    __extension__ union
    {
      int __spins;
      __pthread_slist_t __list;
    };
  } __data;
  char __size[24];
  long int __align;
} pthread_mutex_t;

typedef union
{
  char __size[4];
  long int __align;
} pthread_mutexattr_t;




typedef union
{
  struct
  {
    int __lock;
    unsigned int __futex;
    __extension__ unsigned long long int __total_seq;
    __extension__ unsigned long long int __wakeup_seq;
    __extension__ unsigned long long int __woken_seq;
    void *__mutex;
    unsigned int __nwaiters;
    unsigned int __broadcast_seq;
  } __data;
  char __size[48];
  __extension__ long long int __align;
} pthread_cond_t;

typedef union
{
  char __size[4];
  long int __align;
} pthread_condattr_t;



typedef unsigned int pthread_key_t;



typedef int pthread_once_t;





typedef union
{
  struct
  {
    int __lock;
    unsigned int __nr_readers;
    unsigned int __readers_wakeup;
    unsigned int __writer_wakeup;
    unsigned int __nr_readers_queued;
    unsigned int __nr_writers_queued;


    unsigned int __flags;
    int __writer;
  } __data;
  char __size[32];
  long int __align;
} pthread_rwlock_t;

typedef union
{
  char __size[8];
  long int __align;
} pthread_rwlockattr_t;





typedef volatile int pthread_spinlock_t;




typedef union
{
  char __size[20];
  long int __align;
} pthread_barrier_t;

typedef union
{
  char __size[4];
  int __align;
} pthread_barrierattr_t;
# 271 "/usr/include/sys/types.h" 2 3 4



# 439 "/usr/include/stdlib.h" 2 3 4






extern long int random (void) __attribute__ ((__nothrow__));


extern void srandom (unsigned int __seed) __attribute__ ((__nothrow__));





extern char *initstate (unsigned int __seed, char *__statebuf,
   size_t __statelen) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2)));



extern char *setstate (char *__statebuf) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));







struct random_data
  {
    int32_t *fptr;
    int32_t *rptr;
    int32_t *state;
    int rand_type;
    int rand_deg;
    int rand_sep;
    int32_t *end_ptr;
  };

extern int random_r (struct random_data *__restrict __buf,
       int32_t *__restrict __result) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));

extern int srandom_r (unsigned int __seed, struct random_data *__buf)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2)));

extern int initstate_r (unsigned int __seed, char *__restrict __statebuf,
   size_t __statelen,
   struct random_data *__restrict __buf)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2, 4)));

extern int setstate_r (char *__restrict __statebuf,
         struct random_data *__restrict __buf)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));






extern int rand (void) __attribute__ ((__nothrow__));

extern void srand (unsigned int __seed) __attribute__ ((__nothrow__));




extern int rand_r (unsigned int *__seed) __attribute__ ((__nothrow__));







extern double drand48 (void) __attribute__ ((__nothrow__));
extern double erand48 (unsigned short int __xsubi[3]) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern long int lrand48 (void) __attribute__ ((__nothrow__));
extern long int nrand48 (unsigned short int __xsubi[3])
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern long int mrand48 (void) __attribute__ ((__nothrow__));
extern long int jrand48 (unsigned short int __xsubi[3])
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern void srand48 (long int __seedval) __attribute__ ((__nothrow__));
extern unsigned short int *seed48 (unsigned short int __seed16v[3])
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));
extern void lcong48 (unsigned short int __param[7]) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));





struct drand48_data
  {
    unsigned short int __x[3];
    unsigned short int __old_x[3];
    unsigned short int __c;
    unsigned short int __init;
    unsigned long long int __a;
  };


extern int drand48_r (struct drand48_data *__restrict __buffer,
        double *__restrict __result) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));
extern int erand48_r (unsigned short int __xsubi[3],
        struct drand48_data *__restrict __buffer,
        double *__restrict __result) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));


extern int lrand48_r (struct drand48_data *__restrict __buffer,
        long int *__restrict __result)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));
extern int nrand48_r (unsigned short int __xsubi[3],
        struct drand48_data *__restrict __buffer,
        long int *__restrict __result)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));


extern int mrand48_r (struct drand48_data *__restrict __buffer,
        long int *__restrict __result)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));
extern int jrand48_r (unsigned short int __xsubi[3],
        struct drand48_data *__restrict __buffer,
        long int *__restrict __result)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));


extern int srand48_r (long int __seedval, struct drand48_data *__buffer)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2)));

extern int seed48_r (unsigned short int __seed16v[3],
       struct drand48_data *__buffer) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));

extern int lcong48_r (unsigned short int __param[7],
        struct drand48_data *__buffer)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));









extern void *malloc (size_t __size) __attribute__ ((__nothrow__)) __attribute__ ((__malloc__)) ;

extern void *calloc (size_t __nmemb, size_t __size)
     __attribute__ ((__nothrow__)) __attribute__ ((__malloc__)) ;







extern void *realloc (void *__ptr, size_t __size)
     __attribute__ ((__nothrow__)) __attribute__ ((__malloc__)) __attribute__ ((__warn_unused_result__));

extern void free (void *__ptr) __attribute__ ((__nothrow__));




extern void cfree (void *__ptr) __attribute__ ((__nothrow__));



# 1 "/usr/include/alloca.h" 1 3 4
# 25 "/usr/include/alloca.h" 3 4
# 1 "/usr/lib/gcc/i586-suse-linux/4.2.1/include/stddef.h" 1 3 4
# 26 "/usr/include/alloca.h" 2 3 4







extern void *alloca (size_t __size) __attribute__ ((__nothrow__));






# 613 "/usr/include/stdlib.h" 2 3 4




extern void *valloc (size_t __size) __attribute__ ((__nothrow__)) __attribute__ ((__malloc__)) ;




extern int posix_memalign (void **__memptr, size_t __alignment, size_t __size)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;




extern void abort (void) __attribute__ ((__nothrow__)) __attribute__ ((__noreturn__));



extern int atexit (void (*__func) (void)) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));





extern int on_exit (void (*__func) (int __status, void *__arg), void *__arg)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));






extern void exit (int __status) __attribute__ ((__nothrow__)) __attribute__ ((__noreturn__));

# 658 "/usr/include/stdlib.h" 3 4


extern char *getenv (__const char *__name) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;




extern char *__secure_getenv (__const char *__name)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;





extern int putenv (char *__string) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));





extern int setenv (__const char *__name, __const char *__value, int __replace)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2)));


extern int unsetenv (__const char *__name) __attribute__ ((__nothrow__));






extern int clearenv (void) __attribute__ ((__nothrow__));
# 698 "/usr/include/stdlib.h" 3 4
extern char *mktemp (char *__template) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;
# 709 "/usr/include/stdlib.h" 3 4
extern int mkstemp (char *__template) __attribute__ ((__nonnull__ (1))) ;
# 729 "/usr/include/stdlib.h" 3 4
extern char *mkdtemp (char *__template) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;








extern int system (__const char *__command) ;

# 755 "/usr/include/stdlib.h" 3 4
extern char *realpath (__const char *__restrict __name,
         char *__restrict __resolved) __attribute__ ((__nothrow__)) ;






typedef int (*__compar_fn_t) (__const void *, __const void *);









extern void *bsearch (__const void *__key, __const void *__base,
        size_t __nmemb, size_t __size, __compar_fn_t __compar)
     __attribute__ ((__nonnull__ (1, 2, 5))) ;



extern void qsort (void *__base, size_t __nmemb, size_t __size,
     __compar_fn_t __compar) __attribute__ ((__nonnull__ (1, 4)));



extern int abs (int __x) __attribute__ ((__nothrow__)) __attribute__ ((__const__)) ;
extern long int labs (long int __x) __attribute__ ((__nothrow__)) __attribute__ ((__const__)) ;












extern div_t div (int __numer, int __denom)
     __attribute__ ((__nothrow__)) __attribute__ ((__const__)) ;
extern ldiv_t ldiv (long int __numer, long int __denom)
     __attribute__ ((__nothrow__)) __attribute__ ((__const__)) ;

# 820 "/usr/include/stdlib.h" 3 4
extern char *ecvt (double __value, int __ndigit, int *__restrict __decpt,
     int *__restrict __sign) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3, 4))) ;




extern char *fcvt (double __value, int __ndigit, int *__restrict __decpt,
     int *__restrict __sign) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3, 4))) ;




extern char *gcvt (double __value, int __ndigit, char *__buf)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3))) ;




extern char *qecvt (long double __value, int __ndigit,
      int *__restrict __decpt, int *__restrict __sign)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3, 4))) ;
extern char *qfcvt (long double __value, int __ndigit,
      int *__restrict __decpt, int *__restrict __sign)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3, 4))) ;
extern char *qgcvt (long double __value, int __ndigit, char *__buf)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3))) ;




extern int ecvt_r (double __value, int __ndigit, int *__restrict __decpt,
     int *__restrict __sign, char *__restrict __buf,
     size_t __len) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3, 4, 5)));
extern int fcvt_r (double __value, int __ndigit, int *__restrict __decpt,
     int *__restrict __sign, char *__restrict __buf,
     size_t __len) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3, 4, 5)));

extern int qecvt_r (long double __value, int __ndigit,
      int *__restrict __decpt, int *__restrict __sign,
      char *__restrict __buf, size_t __len)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3, 4, 5)));
extern int qfcvt_r (long double __value, int __ndigit,
      int *__restrict __decpt, int *__restrict __sign,
      char *__restrict __buf, size_t __len)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3, 4, 5)));







extern int mblen (__const char *__s, size_t __n) __attribute__ ((__nothrow__)) ;


extern int mbtowc (wchar_t *__restrict __pwc,
     __const char *__restrict __s, size_t __n) __attribute__ ((__nothrow__)) ;


extern int wctomb (char *__s, wchar_t __wchar) __attribute__ ((__nothrow__)) ;



extern size_t mbstowcs (wchar_t *__restrict __pwcs,
   __const char *__restrict __s, size_t __n) __attribute__ ((__nothrow__));

extern size_t wcstombs (char *__restrict __s,
   __const wchar_t *__restrict __pwcs, size_t __n)
     __attribute__ ((__nothrow__));








extern int rpmatch (__const char *__response) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) ;
# 925 "/usr/include/stdlib.h" 3 4
extern int posix_openpt (int __oflag) ;
# 960 "/usr/include/stdlib.h" 3 4
extern int getloadavg (double __loadavg[], int __nelem)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));
# 976 "/usr/include/stdlib.h" 3 4

# 55 "cairoint.h" 2
# 1 "/usr/include/string.h" 1 3 4
# 28 "/usr/include/string.h" 3 4





# 1 "/usr/lib/gcc/i586-suse-linux/4.2.1/include/stddef.h" 1 3 4
# 34 "/usr/include/string.h" 2 3 4




extern void *memcpy (void *__restrict __dest,
       __const void *__restrict __src, size_t __n)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));


extern void *memmove (void *__dest, __const void *__src, size_t __n)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));






extern void *memccpy (void *__restrict __dest, __const void *__restrict __src,
        int __c, size_t __n)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));





extern void *memset (void *__s, int __c, size_t __n) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern int memcmp (__const void *__s1, __const void *__s2, size_t __n)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));


extern void *memchr (__const void *__s, int __c, size_t __n)
      __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));

# 82 "/usr/include/string.h" 3 4


extern char *strcpy (char *__restrict __dest, __const char *__restrict __src)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));

extern char *strncpy (char *__restrict __dest,
        __const char *__restrict __src, size_t __n)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));


extern char *strcat (char *__restrict __dest, __const char *__restrict __src)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));

extern char *strncat (char *__restrict __dest, __const char *__restrict __src,
        size_t __n) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));


extern int strcmp (__const char *__s1, __const char *__s2)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));

extern int strncmp (__const char *__s1, __const char *__s2, size_t __n)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));


extern int strcoll (__const char *__s1, __const char *__s2)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));

extern size_t strxfrm (char *__restrict __dest,
         __const char *__restrict __src, size_t __n)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2)));

# 130 "/usr/include/string.h" 3 4
extern char *strdup (__const char *__s)
     __attribute__ ((__nothrow__)) __attribute__ ((__malloc__)) __attribute__ ((__nonnull__ (1)));
# 165 "/usr/include/string.h" 3 4


extern char *strchr (__const char *__s, int __c)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));

extern char *strrchr (__const char *__s, int __c)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));

# 181 "/usr/include/string.h" 3 4



extern size_t strcspn (__const char *__s, __const char *__reject)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));


extern size_t strspn (__const char *__s, __const char *__accept)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));

extern char *strpbrk (__const char *__s, __const char *__accept)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));

extern char *strstr (__const char *__haystack, __const char *__needle)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));



extern char *strtok (char *__restrict __s, __const char *__restrict __delim)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2)));




extern char *__strtok_r (char *__restrict __s,
    __const char *__restrict __delim,
    char **__restrict __save_ptr)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2, 3)));

extern char *strtok_r (char *__restrict __s, __const char *__restrict __delim,
         char **__restrict __save_ptr)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2, 3)));
# 240 "/usr/include/string.h" 3 4


extern size_t strlen (__const char *__s)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));

# 254 "/usr/include/string.h" 3 4


extern char *strerror (int __errnum) __attribute__ ((__nothrow__));

# 270 "/usr/include/string.h" 3 4
extern int strerror_r (int __errnum, char *__buf, size_t __buflen) __asm__ ("" "__xpg_strerror_r") __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2)));
# 294 "/usr/include/string.h" 3 4
extern void __bzero (void *__s, size_t __n) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));



extern void bcopy (__const void *__src, void *__dest, size_t __n)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));


extern void bzero (void *__s, size_t __n) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern int bcmp (__const void *__s1, __const void *__s2, size_t __n)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));


extern char *index (__const char *__s, int __c)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));


extern char *rindex (__const char *__s, int __c)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));



extern int ffs (int __i) __attribute__ ((__nothrow__)) __attribute__ ((__const__));
# 331 "/usr/include/string.h" 3 4
extern int strcasecmp (__const char *__s1, __const char *__s2)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));


extern int strncasecmp (__const char *__s1, __const char *__s2, size_t __n)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
# 354 "/usr/include/string.h" 3 4
extern char *strsep (char **__restrict __stringp,
       __const char *__restrict __delim)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));
# 432 "/usr/include/string.h" 3 4

# 56 "cairoint.h" 2
# 1 "/usr/lib/gcc/i586-suse-linux/4.2.1/include/stdarg.h" 1 3 4
# 43 "/usr/lib/gcc/i586-suse-linux/4.2.1/include/stdarg.h" 3 4
typedef __builtin_va_list __gnuc_va_list;
# 105 "/usr/lib/gcc/i586-suse-linux/4.2.1/include/stdarg.h" 3 4
typedef __gnuc_va_list va_list;
# 57 "cairoint.h" 2
# 1 "/usr/lib/gcc/i586-suse-linux/4.2.1/include/stddef.h" 1 3 4
# 152 "/usr/lib/gcc/i586-suse-linux/4.2.1/include/stddef.h" 3 4
typedef int ptrdiff_t;
# 58 "cairoint.h" 2




# 1 "/usr/include/math.h" 1 3 4
# 30 "/usr/include/math.h" 3 4




# 1 "/usr/include/bits/huge_val.h" 1 3 4
# 35 "/usr/include/math.h" 2 3 4
# 47 "/usr/include/math.h" 3 4
# 1 "/usr/include/bits/mathdef.h" 1 3 4
# 48 "/usr/include/math.h" 2 3 4
# 71 "/usr/include/math.h" 3 4
# 1 "/usr/include/bits/mathcalls.h" 1 3 4
# 53 "/usr/include/bits/mathcalls.h" 3 4


extern double acos (double __x) __attribute__ ((__nothrow__)); extern double __acos (double __x) __attribute__ ((__nothrow__));

extern double asin (double __x) __attribute__ ((__nothrow__)); extern double __asin (double __x) __attribute__ ((__nothrow__));

extern double atan (double __x) __attribute__ ((__nothrow__)); extern double __atan (double __x) __attribute__ ((__nothrow__));

extern double atan2 (double __y, double __x) __attribute__ ((__nothrow__)); extern double __atan2 (double __y, double __x) __attribute__ ((__nothrow__));


extern double cos (double __x) __attribute__ ((__nothrow__)); extern double __cos (double __x) __attribute__ ((__nothrow__));

extern double sin (double __x) __attribute__ ((__nothrow__)); extern double __sin (double __x) __attribute__ ((__nothrow__));

extern double tan (double __x) __attribute__ ((__nothrow__)); extern double __tan (double __x) __attribute__ ((__nothrow__));




extern double cosh (double __x) __attribute__ ((__nothrow__)); extern double __cosh (double __x) __attribute__ ((__nothrow__));

extern double sinh (double __x) __attribute__ ((__nothrow__)); extern double __sinh (double __x) __attribute__ ((__nothrow__));

extern double tanh (double __x) __attribute__ ((__nothrow__)); extern double __tanh (double __x) __attribute__ ((__nothrow__));

# 87 "/usr/include/bits/mathcalls.h" 3 4


extern double acosh (double __x) __attribute__ ((__nothrow__)); extern double __acosh (double __x) __attribute__ ((__nothrow__));

extern double asinh (double __x) __attribute__ ((__nothrow__)); extern double __asinh (double __x) __attribute__ ((__nothrow__));

extern double atanh (double __x) __attribute__ ((__nothrow__)); extern double __atanh (double __x) __attribute__ ((__nothrow__));







extern double exp (double __x) __attribute__ ((__nothrow__)); extern double __exp (double __x) __attribute__ ((__nothrow__));


extern double frexp (double __x, int *__exponent) __attribute__ ((__nothrow__)); extern double __frexp (double __x, int *__exponent) __attribute__ ((__nothrow__));


extern double ldexp (double __x, int __exponent) __attribute__ ((__nothrow__)); extern double __ldexp (double __x, int __exponent) __attribute__ ((__nothrow__));


extern double log (double __x) __attribute__ ((__nothrow__)); extern double __log (double __x) __attribute__ ((__nothrow__));


extern double log10 (double __x) __attribute__ ((__nothrow__)); extern double __log10 (double __x) __attribute__ ((__nothrow__));


extern double modf (double __x, double *__iptr) __attribute__ ((__nothrow__)); extern double __modf (double __x, double *__iptr) __attribute__ ((__nothrow__));

# 127 "/usr/include/bits/mathcalls.h" 3 4


extern double expm1 (double __x) __attribute__ ((__nothrow__)); extern double __expm1 (double __x) __attribute__ ((__nothrow__));


extern double log1p (double __x) __attribute__ ((__nothrow__)); extern double __log1p (double __x) __attribute__ ((__nothrow__));


extern double logb (double __x) __attribute__ ((__nothrow__)); extern double __logb (double __x) __attribute__ ((__nothrow__));

# 152 "/usr/include/bits/mathcalls.h" 3 4


extern double pow (double __x, double __y) __attribute__ ((__nothrow__)); extern double __pow (double __x, double __y) __attribute__ ((__nothrow__));


extern double sqrt (double __x) __attribute__ ((__nothrow__)); extern double __sqrt (double __x) __attribute__ ((__nothrow__));





extern double hypot (double __x, double __y) __attribute__ ((__nothrow__)); extern double __hypot (double __x, double __y) __attribute__ ((__nothrow__));






extern double cbrt (double __x) __attribute__ ((__nothrow__)); extern double __cbrt (double __x) __attribute__ ((__nothrow__));








extern double ceil (double __x) __attribute__ ((__nothrow__)) __attribute__ ((__const__)); extern double __ceil (double __x) __attribute__ ((__nothrow__)) __attribute__ ((__const__));


extern double fabs (double __x) __attribute__ ((__nothrow__)) __attribute__ ((__const__)); extern double __fabs (double __x) __attribute__ ((__nothrow__)) __attribute__ ((__const__));


extern double floor (double __x) __attribute__ ((__nothrow__)) __attribute__ ((__const__)); extern double __floor (double __x) __attribute__ ((__nothrow__)) __attribute__ ((__const__));


extern double fmod (double __x, double __y) __attribute__ ((__nothrow__)); extern double __fmod (double __x, double __y) __attribute__ ((__nothrow__));




extern int __isinf (double __value) __attribute__ ((__nothrow__)) __attribute__ ((__const__));


extern int __finite (double __value) __attribute__ ((__nothrow__)) __attribute__ ((__const__));





extern int isinf (double __value) __attribute__ ((__nothrow__)) __attribute__ ((__const__));


extern int finite (double __value) __attribute__ ((__nothrow__)) __attribute__ ((__const__));


extern double drem (double __x, double __y) __attribute__ ((__nothrow__)); extern double __drem (double __x, double __y) __attribute__ ((__nothrow__));



extern double significand (double __x) __attribute__ ((__nothrow__)); extern double __significand (double __x) __attribute__ ((__nothrow__));





extern double copysign (double __x, double __y) __attribute__ ((__nothrow__)) __attribute__ ((__const__)); extern double __copysign (double __x, double __y) __attribute__ ((__nothrow__)) __attribute__ ((__const__));

# 231 "/usr/include/bits/mathcalls.h" 3 4
extern int __isnan (double __value) __attribute__ ((__nothrow__)) __attribute__ ((__const__));



extern int isnan (double __value) __attribute__ ((__nothrow__)) __attribute__ ((__const__));


extern double j0 (double) __attribute__ ((__nothrow__)); extern double __j0 (double) __attribute__ ((__nothrow__));
extern double j1 (double) __attribute__ ((__nothrow__)); extern double __j1 (double) __attribute__ ((__nothrow__));
extern double jn (int, double) __attribute__ ((__nothrow__)); extern double __jn (int, double) __attribute__ ((__nothrow__));
extern double y0 (double) __attribute__ ((__nothrow__)); extern double __y0 (double) __attribute__ ((__nothrow__));
extern double y1 (double) __attribute__ ((__nothrow__)); extern double __y1 (double) __attribute__ ((__nothrow__));
extern double yn (int, double) __attribute__ ((__nothrow__)); extern double __yn (int, double) __attribute__ ((__nothrow__));






extern double erf (double) __attribute__ ((__nothrow__)); extern double __erf (double) __attribute__ ((__nothrow__));
extern double erfc (double) __attribute__ ((__nothrow__)); extern double __erfc (double) __attribute__ ((__nothrow__));
extern double lgamma (double) __attribute__ ((__nothrow__)); extern double __lgamma (double) __attribute__ ((__nothrow__));

# 265 "/usr/include/bits/mathcalls.h" 3 4
extern double gamma (double) __attribute__ ((__nothrow__)); extern double __gamma (double) __attribute__ ((__nothrow__));






extern double lgamma_r (double, int *__signgamp) __attribute__ ((__nothrow__)); extern double __lgamma_r (double, int *__signgamp) __attribute__ ((__nothrow__));







extern double rint (double __x) __attribute__ ((__nothrow__)); extern double __rint (double __x) __attribute__ ((__nothrow__));


extern double nextafter (double __x, double __y) __attribute__ ((__nothrow__)) __attribute__ ((__const__)); extern double __nextafter (double __x, double __y) __attribute__ ((__nothrow__)) __attribute__ ((__const__));





extern double remainder (double __x, double __y) __attribute__ ((__nothrow__)); extern double __remainder (double __x, double __y) __attribute__ ((__nothrow__));



extern double scalbn (double __x, int __n) __attribute__ ((__nothrow__)); extern double __scalbn (double __x, int __n) __attribute__ ((__nothrow__));



extern int ilogb (double __x) __attribute__ ((__nothrow__)); extern int __ilogb (double __x) __attribute__ ((__nothrow__));
# 359 "/usr/include/bits/mathcalls.h" 3 4





extern double scalb (double __x, double __n) __attribute__ ((__nothrow__)); extern double __scalb (double __x, double __n) __attribute__ ((__nothrow__));
# 72 "/usr/include/math.h" 2 3 4
# 94 "/usr/include/math.h" 3 4
# 1 "/usr/include/bits/mathcalls.h" 1 3 4
# 53 "/usr/include/bits/mathcalls.h" 3 4


extern float acosf (float __x) __attribute__ ((__nothrow__)); extern float __acosf (float __x) __attribute__ ((__nothrow__));

extern float asinf (float __x) __attribute__ ((__nothrow__)); extern float __asinf (float __x) __attribute__ ((__nothrow__));

extern float atanf (float __x) __attribute__ ((__nothrow__)); extern float __atanf (float __x) __attribute__ ((__nothrow__));

extern float atan2f (float __y, float __x) __attribute__ ((__nothrow__)); extern float __atan2f (float __y, float __x) __attribute__ ((__nothrow__));


extern float cosf (float __x) __attribute__ ((__nothrow__)); extern float __cosf (float __x) __attribute__ ((__nothrow__));

extern float sinf (float __x) __attribute__ ((__nothrow__)); extern float __sinf (float __x) __attribute__ ((__nothrow__));

extern float tanf (float __x) __attribute__ ((__nothrow__)); extern float __tanf (float __x) __attribute__ ((__nothrow__));




extern float coshf (float __x) __attribute__ ((__nothrow__)); extern float __coshf (float __x) __attribute__ ((__nothrow__));

extern float sinhf (float __x) __attribute__ ((__nothrow__)); extern float __sinhf (float __x) __attribute__ ((__nothrow__));

extern float tanhf (float __x) __attribute__ ((__nothrow__)); extern float __tanhf (float __x) __attribute__ ((__nothrow__));

# 87 "/usr/include/bits/mathcalls.h" 3 4


extern float acoshf (float __x) __attribute__ ((__nothrow__)); extern float __acoshf (float __x) __attribute__ ((__nothrow__));

extern float asinhf (float __x) __attribute__ ((__nothrow__)); extern float __asinhf (float __x) __attribute__ ((__nothrow__));

extern float atanhf (float __x) __attribute__ ((__nothrow__)); extern float __atanhf (float __x) __attribute__ ((__nothrow__));







extern float expf (float __x) __attribute__ ((__nothrow__)); extern float __expf (float __x) __attribute__ ((__nothrow__));


extern float frexpf (float __x, int *__exponent) __attribute__ ((__nothrow__)); extern float __frexpf (float __x, int *__exponent) __attribute__ ((__nothrow__));


extern float ldexpf (float __x, int __exponent) __attribute__ ((__nothrow__)); extern float __ldexpf (float __x, int __exponent) __attribute__ ((__nothrow__));


extern float logf (float __x) __attribute__ ((__nothrow__)); extern float __logf (float __x) __attribute__ ((__nothrow__));


extern float log10f (float __x) __attribute__ ((__nothrow__)); extern float __log10f (float __x) __attribute__ ((__nothrow__));


extern float modff (float __x, float *__iptr) __attribute__ ((__nothrow__)); extern float __modff (float __x, float *__iptr) __attribute__ ((__nothrow__));

# 127 "/usr/include/bits/mathcalls.h" 3 4


extern float expm1f (float __x) __attribute__ ((__nothrow__)); extern float __expm1f (float __x) __attribute__ ((__nothrow__));


extern float log1pf (float __x) __attribute__ ((__nothrow__)); extern float __log1pf (float __x) __attribute__ ((__nothrow__));


extern float logbf (float __x) __attribute__ ((__nothrow__)); extern float __logbf (float __x) __attribute__ ((__nothrow__));

# 152 "/usr/include/bits/mathcalls.h" 3 4


extern float powf (float __x, float __y) __attribute__ ((__nothrow__)); extern float __powf (float __x, float __y) __attribute__ ((__nothrow__));


extern float sqrtf (float __x) __attribute__ ((__nothrow__)); extern float __sqrtf (float __x) __attribute__ ((__nothrow__));





extern float hypotf (float __x, float __y) __attribute__ ((__nothrow__)); extern float __hypotf (float __x, float __y) __attribute__ ((__nothrow__));






extern float cbrtf (float __x) __attribute__ ((__nothrow__)); extern float __cbrtf (float __x) __attribute__ ((__nothrow__));








extern float ceilf (float __x) __attribute__ ((__nothrow__)) __attribute__ ((__const__)); extern float __ceilf (float __x) __attribute__ ((__nothrow__)) __attribute__ ((__const__));


extern float fabsf (float __x) __attribute__ ((__nothrow__)) __attribute__ ((__const__)); extern float __fabsf (float __x) __attribute__ ((__nothrow__)) __attribute__ ((__const__));


extern float floorf (float __x) __attribute__ ((__nothrow__)) __attribute__ ((__const__)); extern float __floorf (float __x) __attribute__ ((__nothrow__)) __attribute__ ((__const__));


extern float fmodf (float __x, float __y) __attribute__ ((__nothrow__)); extern float __fmodf (float __x, float __y) __attribute__ ((__nothrow__));




extern int __isinff (float __value) __attribute__ ((__nothrow__)) __attribute__ ((__const__));


extern int __finitef (float __value) __attribute__ ((__nothrow__)) __attribute__ ((__const__));





extern int isinff (float __value) __attribute__ ((__nothrow__)) __attribute__ ((__const__));


extern int finitef (float __value) __attribute__ ((__nothrow__)) __attribute__ ((__const__));


extern float dremf (float __x, float __y) __attribute__ ((__nothrow__)); extern float __dremf (float __x, float __y) __attribute__ ((__nothrow__));



extern float significandf (float __x) __attribute__ ((__nothrow__)); extern float __significandf (float __x) __attribute__ ((__nothrow__));





extern float copysignf (float __x, float __y) __attribute__ ((__nothrow__)) __attribute__ ((__const__)); extern float __copysignf (float __x, float __y) __attribute__ ((__nothrow__)) __attribute__ ((__const__));

# 231 "/usr/include/bits/mathcalls.h" 3 4
extern int __isnanf (float __value) __attribute__ ((__nothrow__)) __attribute__ ((__const__));



extern int isnanf (float __value) __attribute__ ((__nothrow__)) __attribute__ ((__const__));


extern float j0f (float) __attribute__ ((__nothrow__)); extern float __j0f (float) __attribute__ ((__nothrow__));
extern float j1f (float) __attribute__ ((__nothrow__)); extern float __j1f (float) __attribute__ ((__nothrow__));
extern float jnf (int, float) __attribute__ ((__nothrow__)); extern float __jnf (int, float) __attribute__ ((__nothrow__));
extern float y0f (float) __attribute__ ((__nothrow__)); extern float __y0f (float) __attribute__ ((__nothrow__));
extern float y1f (float) __attribute__ ((__nothrow__)); extern float __y1f (float) __attribute__ ((__nothrow__));
extern float ynf (int, float) __attribute__ ((__nothrow__)); extern float __ynf (int, float) __attribute__ ((__nothrow__));






extern float erff (float) __attribute__ ((__nothrow__)); extern float __erff (float) __attribute__ ((__nothrow__));
extern float erfcf (float) __attribute__ ((__nothrow__)); extern float __erfcf (float) __attribute__ ((__nothrow__));
extern float lgammaf (float) __attribute__ ((__nothrow__)); extern float __lgammaf (float) __attribute__ ((__nothrow__));

# 265 "/usr/include/bits/mathcalls.h" 3 4
extern float gammaf (float) __attribute__ ((__nothrow__)); extern float __gammaf (float) __attribute__ ((__nothrow__));






extern float lgammaf_r (float, int *__signgamp) __attribute__ ((__nothrow__)); extern float __lgammaf_r (float, int *__signgamp) __attribute__ ((__nothrow__));







extern float rintf (float __x) __attribute__ ((__nothrow__)); extern float __rintf (float __x) __attribute__ ((__nothrow__));


extern float nextafterf (float __x, float __y) __attribute__ ((__nothrow__)) __attribute__ ((__const__)); extern float __nextafterf (float __x, float __y) __attribute__ ((__nothrow__)) __attribute__ ((__const__));





extern float remainderf (float __x, float __y) __attribute__ ((__nothrow__)); extern float __remainderf (float __x, float __y) __attribute__ ((__nothrow__));



extern float scalbnf (float __x, int __n) __attribute__ ((__nothrow__)); extern float __scalbnf (float __x, int __n) __attribute__ ((__nothrow__));



extern int ilogbf (float __x) __attribute__ ((__nothrow__)); extern int __ilogbf (float __x) __attribute__ ((__nothrow__));
# 359 "/usr/include/bits/mathcalls.h" 3 4





extern float scalbf (float __x, float __n) __attribute__ ((__nothrow__)); extern float __scalbf (float __x, float __n) __attribute__ ((__nothrow__));
# 95 "/usr/include/math.h" 2 3 4
# 141 "/usr/include/math.h" 3 4
# 1 "/usr/include/bits/mathcalls.h" 1 3 4
# 53 "/usr/include/bits/mathcalls.h" 3 4


extern long double acosl (long double __x) __attribute__ ((__nothrow__)); extern long double __acosl (long double __x) __attribute__ ((__nothrow__));

extern long double asinl (long double __x) __attribute__ ((__nothrow__)); extern long double __asinl (long double __x) __attribute__ ((__nothrow__));

extern long double atanl (long double __x) __attribute__ ((__nothrow__)); extern long double __atanl (long double __x) __attribute__ ((__nothrow__));

extern long double atan2l (long double __y, long double __x) __attribute__ ((__nothrow__)); extern long double __atan2l (long double __y, long double __x) __attribute__ ((__nothrow__));


extern long double cosl (long double __x) __attribute__ ((__nothrow__)); extern long double __cosl (long double __x) __attribute__ ((__nothrow__));

extern long double sinl (long double __x) __attribute__ ((__nothrow__)); extern long double __sinl (long double __x) __attribute__ ((__nothrow__));

extern long double tanl (long double __x) __attribute__ ((__nothrow__)); extern long double __tanl (long double __x) __attribute__ ((__nothrow__));




extern long double coshl (long double __x) __attribute__ ((__nothrow__)); extern long double __coshl (long double __x) __attribute__ ((__nothrow__));

extern long double sinhl (long double __x) __attribute__ ((__nothrow__)); extern long double __sinhl (long double __x) __attribute__ ((__nothrow__));

extern long double tanhl (long double __x) __attribute__ ((__nothrow__)); extern long double __tanhl (long double __x) __attribute__ ((__nothrow__));

# 87 "/usr/include/bits/mathcalls.h" 3 4


extern long double acoshl (long double __x) __attribute__ ((__nothrow__)); extern long double __acoshl (long double __x) __attribute__ ((__nothrow__));

extern long double asinhl (long double __x) __attribute__ ((__nothrow__)); extern long double __asinhl (long double __x) __attribute__ ((__nothrow__));

extern long double atanhl (long double __x) __attribute__ ((__nothrow__)); extern long double __atanhl (long double __x) __attribute__ ((__nothrow__));







extern long double expl (long double __x) __attribute__ ((__nothrow__)); extern long double __expl (long double __x) __attribute__ ((__nothrow__));


extern long double frexpl (long double __x, int *__exponent) __attribute__ ((__nothrow__)); extern long double __frexpl (long double __x, int *__exponent) __attribute__ ((__nothrow__));


extern long double ldexpl (long double __x, int __exponent) __attribute__ ((__nothrow__)); extern long double __ldexpl (long double __x, int __exponent) __attribute__ ((__nothrow__));


extern long double logl (long double __x) __attribute__ ((__nothrow__)); extern long double __logl (long double __x) __attribute__ ((__nothrow__));


extern long double log10l (long double __x) __attribute__ ((__nothrow__)); extern long double __log10l (long double __x) __attribute__ ((__nothrow__));


extern long double modfl (long double __x, long double *__iptr) __attribute__ ((__nothrow__)); extern long double __modfl (long double __x, long double *__iptr) __attribute__ ((__nothrow__));

# 127 "/usr/include/bits/mathcalls.h" 3 4


extern long double expm1l (long double __x) __attribute__ ((__nothrow__)); extern long double __expm1l (long double __x) __attribute__ ((__nothrow__));


extern long double log1pl (long double __x) __attribute__ ((__nothrow__)); extern long double __log1pl (long double __x) __attribute__ ((__nothrow__));


extern long double logbl (long double __x) __attribute__ ((__nothrow__)); extern long double __logbl (long double __x) __attribute__ ((__nothrow__));

# 152 "/usr/include/bits/mathcalls.h" 3 4


extern long double powl (long double __x, long double __y) __attribute__ ((__nothrow__)); extern long double __powl (long double __x, long double __y) __attribute__ ((__nothrow__));


extern long double sqrtl (long double __x) __attribute__ ((__nothrow__)); extern long double __sqrtl (long double __x) __attribute__ ((__nothrow__));





extern long double hypotl (long double __x, long double __y) __attribute__ ((__nothrow__)); extern long double __hypotl (long double __x, long double __y) __attribute__ ((__nothrow__));






extern long double cbrtl (long double __x) __attribute__ ((__nothrow__)); extern long double __cbrtl (long double __x) __attribute__ ((__nothrow__));








extern long double ceill (long double __x) __attribute__ ((__nothrow__)) __attribute__ ((__const__)); extern long double __ceill (long double __x) __attribute__ ((__nothrow__)) __attribute__ ((__const__));


extern long double fabsl (long double __x) __attribute__ ((__nothrow__)) __attribute__ ((__const__)); extern long double __fabsl (long double __x) __attribute__ ((__nothrow__)) __attribute__ ((__const__));


extern long double floorl (long double __x) __attribute__ ((__nothrow__)) __attribute__ ((__const__)); extern long double __floorl (long double __x) __attribute__ ((__nothrow__)) __attribute__ ((__const__));


extern long double fmodl (long double __x, long double __y) __attribute__ ((__nothrow__)); extern long double __fmodl (long double __x, long double __y) __attribute__ ((__nothrow__));




extern int __isinfl (long double __value) __attribute__ ((__nothrow__)) __attribute__ ((__const__));


extern int __finitel (long double __value) __attribute__ ((__nothrow__)) __attribute__ ((__const__));





extern int isinfl (long double __value) __attribute__ ((__nothrow__)) __attribute__ ((__const__));


extern int finitel (long double __value) __attribute__ ((__nothrow__)) __attribute__ ((__const__));


extern long double dreml (long double __x, long double __y) __attribute__ ((__nothrow__)); extern long double __dreml (long double __x, long double __y) __attribute__ ((__nothrow__));



extern long double significandl (long double __x) __attribute__ ((__nothrow__)); extern long double __significandl (long double __x) __attribute__ ((__nothrow__));





extern long double copysignl (long double __x, long double __y) __attribute__ ((__nothrow__)) __attribute__ ((__const__)); extern long double __copysignl (long double __x, long double __y) __attribute__ ((__nothrow__)) __attribute__ ((__const__));

# 231 "/usr/include/bits/mathcalls.h" 3 4
extern int __isnanl (long double __value) __attribute__ ((__nothrow__)) __attribute__ ((__const__));



extern int isnanl (long double __value) __attribute__ ((__nothrow__)) __attribute__ ((__const__));


extern long double j0l (long double) __attribute__ ((__nothrow__)); extern long double __j0l (long double) __attribute__ ((__nothrow__));
extern long double j1l (long double) __attribute__ ((__nothrow__)); extern long double __j1l (long double) __attribute__ ((__nothrow__));
extern long double jnl (int, long double) __attribute__ ((__nothrow__)); extern long double __jnl (int, long double) __attribute__ ((__nothrow__));
extern long double y0l (long double) __attribute__ ((__nothrow__)); extern long double __y0l (long double) __attribute__ ((__nothrow__));
extern long double y1l (long double) __attribute__ ((__nothrow__)); extern long double __y1l (long double) __attribute__ ((__nothrow__));
extern long double ynl (int, long double) __attribute__ ((__nothrow__)); extern long double __ynl (int, long double) __attribute__ ((__nothrow__));






extern long double erfl (long double) __attribute__ ((__nothrow__)); extern long double __erfl (long double) __attribute__ ((__nothrow__));
extern long double erfcl (long double) __attribute__ ((__nothrow__)); extern long double __erfcl (long double) __attribute__ ((__nothrow__));
extern long double lgammal (long double) __attribute__ ((__nothrow__)); extern long double __lgammal (long double) __attribute__ ((__nothrow__));

# 265 "/usr/include/bits/mathcalls.h" 3 4
extern long double gammal (long double) __attribute__ ((__nothrow__)); extern long double __gammal (long double) __attribute__ ((__nothrow__));






extern long double lgammal_r (long double, int *__signgamp) __attribute__ ((__nothrow__)); extern long double __lgammal_r (long double, int *__signgamp) __attribute__ ((__nothrow__));







extern long double rintl (long double __x) __attribute__ ((__nothrow__)); extern long double __rintl (long double __x) __attribute__ ((__nothrow__));


extern long double nextafterl (long double __x, long double __y) __attribute__ ((__nothrow__)) __attribute__ ((__const__)); extern long double __nextafterl (long double __x, long double __y) __attribute__ ((__nothrow__)) __attribute__ ((__const__));





extern long double remainderl (long double __x, long double __y) __attribute__ ((__nothrow__)); extern long double __remainderl (long double __x, long double __y) __attribute__ ((__nothrow__));



extern long double scalbnl (long double __x, int __n) __attribute__ ((__nothrow__)); extern long double __scalbnl (long double __x, int __n) __attribute__ ((__nothrow__));



extern int ilogbl (long double __x) __attribute__ ((__nothrow__)); extern int __ilogbl (long double __x) __attribute__ ((__nothrow__));
# 359 "/usr/include/bits/mathcalls.h" 3 4





extern long double scalbl (long double __x, long double __n) __attribute__ ((__nothrow__)); extern long double __scalbl (long double __x, long double __n) __attribute__ ((__nothrow__));
# 142 "/usr/include/math.h" 2 3 4
# 157 "/usr/include/math.h" 3 4
extern int signgam;
# 284 "/usr/include/math.h" 3 4
typedef enum
{
  _IEEE_ = -1,
  _SVID_,
  _XOPEN_,
  _POSIX_,
  _ISOC_
} _LIB_VERSION_TYPE;




extern _LIB_VERSION_TYPE _LIB_VERSION;
# 309 "/usr/include/math.h" 3 4
struct exception

  {
    int type;
    char *name;
    double arg1;
    double arg2;
    double retval;
  };




extern int matherr (struct exception *__exc);
# 465 "/usr/include/math.h" 3 4

# 63 "cairoint.h" 2
# 1 "/usr/lib/gcc/i586-suse-linux/4.2.1/include/limits.h" 1 3 4
# 11 "/usr/lib/gcc/i586-suse-linux/4.2.1/include/limits.h" 3 4
# 1 "/usr/lib/gcc/i586-suse-linux/4.2.1/include/syslimits.h" 1 3 4






# 1 "/usr/lib/gcc/i586-suse-linux/4.2.1/include/limits.h" 1 3 4
# 122 "/usr/lib/gcc/i586-suse-linux/4.2.1/include/limits.h" 3 4
# 1 "/usr/include/limits.h" 1 3 4
# 145 "/usr/include/limits.h" 3 4
# 1 "/usr/include/bits/posix1_lim.h" 1 3 4
# 153 "/usr/include/bits/posix1_lim.h" 3 4
# 1 "/usr/include/bits/local_lim.h" 1 3 4
# 36 "/usr/include/bits/local_lim.h" 3 4
# 1 "/usr/include/linux/limits.h" 1 3 4
# 37 "/usr/include/bits/local_lim.h" 2 3 4
# 154 "/usr/include/bits/posix1_lim.h" 2 3 4
# 146 "/usr/include/limits.h" 2 3 4



# 1 "/usr/include/bits/posix2_lim.h" 1 3 4
# 150 "/usr/include/limits.h" 2 3 4
# 123 "/usr/lib/gcc/i586-suse-linux/4.2.1/include/limits.h" 2 3 4
# 8 "/usr/lib/gcc/i586-suse-linux/4.2.1/include/syslimits.h" 2 3 4
# 12 "/usr/lib/gcc/i586-suse-linux/4.2.1/include/limits.h" 2 3 4
# 64 "cairoint.h" 2
# 1 "/usr/include/stdio.h" 1 3 4
# 30 "/usr/include/stdio.h" 3 4




# 1 "/usr/lib/gcc/i586-suse-linux/4.2.1/include/stddef.h" 1 3 4
# 35 "/usr/include/stdio.h" 2 3 4
# 45 "/usr/include/stdio.h" 3 4
struct _IO_FILE;



typedef struct _IO_FILE FILE;





# 65 "/usr/include/stdio.h" 3 4
typedef struct _IO_FILE __FILE;
# 75 "/usr/include/stdio.h" 3 4
# 1 "/usr/include/libio.h" 1 3 4
# 32 "/usr/include/libio.h" 3 4
# 1 "/usr/include/_G_config.h" 1 3 4
# 14 "/usr/include/_G_config.h" 3 4
# 1 "/usr/lib/gcc/i586-suse-linux/4.2.1/include/stddef.h" 1 3 4
# 355 "/usr/lib/gcc/i586-suse-linux/4.2.1/include/stddef.h" 3 4
typedef unsigned int wint_t;
# 15 "/usr/include/_G_config.h" 2 3 4
# 24 "/usr/include/_G_config.h" 3 4
# 1 "/usr/include/wchar.h" 1 3 4
# 48 "/usr/include/wchar.h" 3 4
# 1 "/usr/lib/gcc/i586-suse-linux/4.2.1/include/stddef.h" 1 3 4
# 49 "/usr/include/wchar.h" 2 3 4

# 1 "/usr/include/bits/wchar.h" 1 3 4
# 51 "/usr/include/wchar.h" 2 3 4
# 76 "/usr/include/wchar.h" 3 4
typedef struct
{
  int __count;
  union
  {
    wint_t __wch;
    char __wchb[4];
  } __value;
} __mbstate_t;
# 25 "/usr/include/_G_config.h" 2 3 4

typedef struct
{
  __off_t __pos;
  __mbstate_t __state;
} _G_fpos_t;
typedef struct
{
  __off64_t __pos;
  __mbstate_t __state;
} _G_fpos64_t;
# 44 "/usr/include/_G_config.h" 3 4
# 1 "/usr/include/gconv.h" 1 3 4
# 28 "/usr/include/gconv.h" 3 4
# 1 "/usr/include/wchar.h" 1 3 4
# 48 "/usr/include/wchar.h" 3 4
# 1 "/usr/lib/gcc/i586-suse-linux/4.2.1/include/stddef.h" 1 3 4
# 49 "/usr/include/wchar.h" 2 3 4
# 29 "/usr/include/gconv.h" 2 3 4


# 1 "/usr/lib/gcc/i586-suse-linux/4.2.1/include/stddef.h" 1 3 4
# 32 "/usr/include/gconv.h" 2 3 4





enum
{
  __GCONV_OK = 0,
  __GCONV_NOCONV,
  __GCONV_NODB,
  __GCONV_NOMEM,

  __GCONV_EMPTY_INPUT,
  __GCONV_FULL_OUTPUT,
  __GCONV_ILLEGAL_INPUT,
  __GCONV_INCOMPLETE_INPUT,

  __GCONV_ILLEGAL_DESCRIPTOR,
  __GCONV_INTERNAL_ERROR
};



enum
{
  __GCONV_IS_LAST = 0x0001,
  __GCONV_IGNORE_ERRORS = 0x0002
};



struct __gconv_step;
struct __gconv_step_data;
struct __gconv_loaded_object;
struct __gconv_trans_data;



typedef int (*__gconv_fct) (struct __gconv_step *, struct __gconv_step_data *,
       __const unsigned char **, __const unsigned char *,
       unsigned char **, size_t *, int, int);


typedef wint_t (*__gconv_btowc_fct) (struct __gconv_step *, unsigned char);


typedef int (*__gconv_init_fct) (struct __gconv_step *);
typedef void (*__gconv_end_fct) (struct __gconv_step *);



typedef int (*__gconv_trans_fct) (struct __gconv_step *,
      struct __gconv_step_data *, void *,
      __const unsigned char *,
      __const unsigned char **,
      __const unsigned char *, unsigned char **,
      size_t *);


typedef int (*__gconv_trans_context_fct) (void *, __const unsigned char *,
       __const unsigned char *,
       unsigned char *, unsigned char *);


typedef int (*__gconv_trans_query_fct) (__const char *, __const char ***,
     size_t *);


typedef int (*__gconv_trans_init_fct) (void **, const char *);
typedef void (*__gconv_trans_end_fct) (void *);

struct __gconv_trans_data
{

  __gconv_trans_fct __trans_fct;
  __gconv_trans_context_fct __trans_context_fct;
  __gconv_trans_end_fct __trans_end_fct;
  void *__data;
  struct __gconv_trans_data *__next;
};



struct __gconv_step
{
  struct __gconv_loaded_object *__shlib_handle;
  __const char *__modname;

  int __counter;

  char *__from_name;
  char *__to_name;

  __gconv_fct __fct;
  __gconv_btowc_fct __btowc_fct;
  __gconv_init_fct __init_fct;
  __gconv_end_fct __end_fct;



  int __min_needed_from;
  int __max_needed_from;
  int __min_needed_to;
  int __max_needed_to;


  int __stateful;

  void *__data;
};



struct __gconv_step_data
{
  unsigned char *__outbuf;
  unsigned char *__outbufend;



  int __flags;



  int __invocation_counter;



  int __internal_use;

  __mbstate_t *__statep;
  __mbstate_t __state;



  struct __gconv_trans_data *__trans;
};



typedef struct __gconv_info
{
  size_t __nsteps;
  struct __gconv_step *__steps;
  __extension__ struct __gconv_step_data __data [];
} *__gconv_t;
# 45 "/usr/include/_G_config.h" 2 3 4
typedef union
{
  struct __gconv_info __cd;
  struct
  {
    struct __gconv_info __cd;
    struct __gconv_step_data __data;
  } __combined;
} _G_iconv_t;

typedef int _G_int16_t __attribute__ ((__mode__ (__HI__)));
typedef int _G_int32_t __attribute__ ((__mode__ (__SI__)));
typedef unsigned int _G_uint16_t __attribute__ ((__mode__ (__HI__)));
typedef unsigned int _G_uint32_t __attribute__ ((__mode__ (__SI__)));
# 33 "/usr/include/libio.h" 2 3 4
# 167 "/usr/include/libio.h" 3 4
struct _IO_jump_t; struct _IO_FILE;
# 177 "/usr/include/libio.h" 3 4
typedef void _IO_lock_t;





struct _IO_marker {
  struct _IO_marker *_next;
  struct _IO_FILE *_sbuf;



  int _pos;
# 200 "/usr/include/libio.h" 3 4
};


enum __codecvt_result
{
  __codecvt_ok,
  __codecvt_partial,
  __codecvt_error,
  __codecvt_noconv
};
# 268 "/usr/include/libio.h" 3 4
struct _IO_FILE {
  int _flags;




  char* _IO_read_ptr;
  char* _IO_read_end;
  char* _IO_read_base;
  char* _IO_write_base;
  char* _IO_write_ptr;
  char* _IO_write_end;
  char* _IO_buf_base;
  char* _IO_buf_end;

  char *_IO_save_base;
  char *_IO_backup_base;
  char *_IO_save_end;

  struct _IO_marker *_markers;

  struct _IO_FILE *_chain;

  int _fileno;



  int _flags2;

  __off_t _old_offset;



  unsigned short _cur_column;
  signed char _vtable_offset;
  char _shortbuf[1];



  _IO_lock_t *_lock;
# 316 "/usr/include/libio.h" 3 4
  __off64_t _offset;
# 325 "/usr/include/libio.h" 3 4
  void *__pad1;
  void *__pad2;
  void *__pad3;
  void *__pad4;
  size_t __pad5;

  int _mode;

  char _unused2[15 * sizeof (int) - 4 * sizeof (void *) - sizeof (size_t)];

};


typedef struct _IO_FILE _IO_FILE;


struct _IO_FILE_plus;

extern struct _IO_FILE_plus _IO_2_1_stdin_;
extern struct _IO_FILE_plus _IO_2_1_stdout_;
extern struct _IO_FILE_plus _IO_2_1_stderr_;
# 361 "/usr/include/libio.h" 3 4
typedef __ssize_t __io_read_fn (void *__cookie, char *__buf, size_t __nbytes);







typedef __ssize_t __io_write_fn (void *__cookie, __const char *__buf,
     size_t __n);







typedef int __io_seek_fn (void *__cookie, __off64_t *__pos, int __w);


typedef int __io_close_fn (void *__cookie);
# 413 "/usr/include/libio.h" 3 4
extern int __underflow (_IO_FILE *);
extern int __uflow (_IO_FILE *);
extern int __overflow (_IO_FILE *, int);
extern wint_t __wunderflow (_IO_FILE *);
extern wint_t __wuflow (_IO_FILE *);
extern wint_t __woverflow (_IO_FILE *, wint_t);
# 451 "/usr/include/libio.h" 3 4
extern int _IO_getc (_IO_FILE *__fp);
extern int _IO_putc (int __c, _IO_FILE *__fp);
extern int _IO_feof (_IO_FILE *__fp) __attribute__ ((__nothrow__));
extern int _IO_ferror (_IO_FILE *__fp) __attribute__ ((__nothrow__));

extern int _IO_peekc_locked (_IO_FILE *__fp);





extern void _IO_flockfile (_IO_FILE *) __attribute__ ((__nothrow__));
extern void _IO_funlockfile (_IO_FILE *) __attribute__ ((__nothrow__));
extern int _IO_ftrylockfile (_IO_FILE *) __attribute__ ((__nothrow__));
# 481 "/usr/include/libio.h" 3 4
extern int _IO_vfscanf (_IO_FILE * __restrict, const char * __restrict,
   __gnuc_va_list, int *__restrict);
extern int _IO_vfprintf (_IO_FILE *__restrict, const char *__restrict,
    __gnuc_va_list);
extern __ssize_t _IO_padn (_IO_FILE *, int, __ssize_t);
extern size_t _IO_sgetn (_IO_FILE *, void *, size_t);

extern __off64_t _IO_seekoff (_IO_FILE *, __off64_t, int, int);
extern __off64_t _IO_seekpos (_IO_FILE *, __off64_t, int);

extern void _IO_free_backup_area (_IO_FILE *) __attribute__ ((__nothrow__));
# 76 "/usr/include/stdio.h" 2 3 4
# 89 "/usr/include/stdio.h" 3 4


typedef _G_fpos_t fpos_t;




# 141 "/usr/include/stdio.h" 3 4
# 1 "/usr/include/bits/stdio_lim.h" 1 3 4
# 142 "/usr/include/stdio.h" 2 3 4



extern struct _IO_FILE *stdin;
extern struct _IO_FILE *stdout;
extern struct _IO_FILE *stderr;









extern int remove (__const char *__filename) __attribute__ ((__nothrow__));

extern int rename (__const char *__old, __const char *__new) __attribute__ ((__nothrow__));














extern FILE *tmpfile (void) ;
# 188 "/usr/include/stdio.h" 3 4
extern char *tmpnam (char *__s) __attribute__ ((__nothrow__)) ;





extern char *tmpnam_r (char *__s) __attribute__ ((__nothrow__)) ;
# 206 "/usr/include/stdio.h" 3 4
extern char *tempnam (__const char *__dir, __const char *__pfx)
     __attribute__ ((__nothrow__)) __attribute__ ((__malloc__)) ;








extern int fclose (FILE *__stream);




extern int fflush (FILE *__stream);

# 231 "/usr/include/stdio.h" 3 4
extern int fflush_unlocked (FILE *__stream);
# 245 "/usr/include/stdio.h" 3 4






extern FILE *fopen (__const char *__restrict __filename,
      __const char *__restrict __modes) ;




extern FILE *freopen (__const char *__restrict __filename,
        __const char *__restrict __modes,
        FILE *__restrict __stream) ;
# 274 "/usr/include/stdio.h" 3 4

# 285 "/usr/include/stdio.h" 3 4
extern FILE *fdopen (int __fd, __const char *__modes) __attribute__ ((__nothrow__)) ;
# 306 "/usr/include/stdio.h" 3 4



extern void setbuf (FILE *__restrict __stream, char *__restrict __buf) __attribute__ ((__nothrow__));



extern int setvbuf (FILE *__restrict __stream, char *__restrict __buf,
      int __modes, size_t __n) __attribute__ ((__nothrow__));





extern void setbuffer (FILE *__restrict __stream, char *__restrict __buf,
         size_t __size) __attribute__ ((__nothrow__));


extern void setlinebuf (FILE *__stream) __attribute__ ((__nothrow__));








extern int fprintf (FILE *__restrict __stream,
      __const char *__restrict __format, ...);




extern int printf (__const char *__restrict __format, ...);

extern int sprintf (char *__restrict __s,
      __const char *__restrict __format, ...) __attribute__ ((__nothrow__));





extern int vfprintf (FILE *__restrict __s, __const char *__restrict __format,
       __gnuc_va_list __arg);




extern int vprintf (__const char *__restrict __format, __gnuc_va_list __arg);

extern int vsprintf (char *__restrict __s, __const char *__restrict __format,
       __gnuc_va_list __arg) __attribute__ ((__nothrow__));





extern int snprintf (char *__restrict __s, size_t __maxlen,
       __const char *__restrict __format, ...)
     __attribute__ ((__nothrow__)) __attribute__ ((__format__ (__printf__, 3, 4)));

extern int vsnprintf (char *__restrict __s, size_t __maxlen,
        __const char *__restrict __format, __gnuc_va_list __arg)
     __attribute__ ((__nothrow__)) __attribute__ ((__format__ (__printf__, 3, 0)));

# 400 "/usr/include/stdio.h" 3 4





extern int fscanf (FILE *__restrict __stream,
     __const char *__restrict __format, ...) ;




extern int scanf (__const char *__restrict __format, ...) ;

extern int sscanf (__const char *__restrict __s,
     __const char *__restrict __format, ...) __attribute__ ((__nothrow__));

# 442 "/usr/include/stdio.h" 3 4





extern int fgetc (FILE *__stream);
extern int getc (FILE *__stream);





extern int getchar (void);

# 466 "/usr/include/stdio.h" 3 4
extern int getc_unlocked (FILE *__stream);
extern int getchar_unlocked (void);
# 477 "/usr/include/stdio.h" 3 4
extern int fgetc_unlocked (FILE *__stream);











extern int fputc (int __c, FILE *__stream);
extern int putc (int __c, FILE *__stream);





extern int putchar (int __c);

# 510 "/usr/include/stdio.h" 3 4
extern int fputc_unlocked (int __c, FILE *__stream);







extern int putc_unlocked (int __c, FILE *__stream);
extern int putchar_unlocked (int __c);






extern int getw (FILE *__stream);


extern int putw (int __w, FILE *__stream);








extern char *fgets (char *__restrict __s, int __n, FILE *__restrict __stream)
     ;






extern char *gets (char *__s) ;

# 591 "/usr/include/stdio.h" 3 4





extern int fputs (__const char *__restrict __s, FILE *__restrict __stream);





extern int puts (__const char *__s);






extern int ungetc (int __c, FILE *__stream);






extern size_t fread (void *__restrict __ptr, size_t __size,
       size_t __n, FILE *__restrict __stream) ;




extern size_t fwrite (__const void *__restrict __ptr, size_t __size,
        size_t __n, FILE *__restrict __s) ;

# 644 "/usr/include/stdio.h" 3 4
extern size_t fread_unlocked (void *__restrict __ptr, size_t __size,
         size_t __n, FILE *__restrict __stream) ;
extern size_t fwrite_unlocked (__const void *__restrict __ptr, size_t __size,
          size_t __n, FILE *__restrict __stream) ;








extern int fseek (FILE *__stream, long int __off, int __whence);




extern long int ftell (FILE *__stream) ;




extern void rewind (FILE *__stream);

# 680 "/usr/include/stdio.h" 3 4
extern int fseeko (FILE *__stream, __off_t __off, int __whence);




extern __off_t ftello (FILE *__stream) ;
# 699 "/usr/include/stdio.h" 3 4






extern int fgetpos (FILE *__restrict __stream, fpos_t *__restrict __pos);




extern int fsetpos (FILE *__stream, __const fpos_t *__pos);
# 722 "/usr/include/stdio.h" 3 4

# 731 "/usr/include/stdio.h" 3 4


extern void clearerr (FILE *__stream) __attribute__ ((__nothrow__));

extern int feof (FILE *__stream) __attribute__ ((__nothrow__)) ;

extern int ferror (FILE *__stream) __attribute__ ((__nothrow__)) ;




extern void clearerr_unlocked (FILE *__stream) __attribute__ ((__nothrow__));
extern int feof_unlocked (FILE *__stream) __attribute__ ((__nothrow__)) ;
extern int ferror_unlocked (FILE *__stream) __attribute__ ((__nothrow__)) ;








extern void perror (__const char *__s);






# 1 "/usr/include/bits/sys_errlist.h" 1 3 4
# 27 "/usr/include/bits/sys_errlist.h" 3 4
extern int sys_nerr;
extern __const char *__const sys_errlist[];
# 761 "/usr/include/stdio.h" 2 3 4




extern int fileno (FILE *__stream) __attribute__ ((__nothrow__)) ;




extern int fileno_unlocked (FILE *__stream) __attribute__ ((__nothrow__)) ;
# 780 "/usr/include/stdio.h" 3 4
extern FILE *popen (__const char *__command, __const char *__modes) ;





extern int pclose (FILE *__stream);





extern char *ctermid (char *__s) __attribute__ ((__nothrow__));
# 820 "/usr/include/stdio.h" 3 4
extern void flockfile (FILE *__stream) __attribute__ ((__nothrow__));



extern int ftrylockfile (FILE *__stream) __attribute__ ((__nothrow__)) ;


extern void funlockfile (FILE *__stream) __attribute__ ((__nothrow__));
# 850 "/usr/include/stdio.h" 3 4

# 65 "cairoint.h" 2

# 1 "cairo.h" 1
# 41 "cairo.h"
# 1 "cairo-embed.h" 1
# 42 "cairo.h" 2

# 1 "./cairo-features.h" 1
# 44 "cairo.h" 2
# 1 "./cairo-deprecated.h" 1
# 45 "cairo.h" 2


# 58 "cairo.h"
 int
moonlight_cairo_version (void);

 const char*
moonlight_cairo_version_string (void);
# 78 "cairo.h"
typedef int cairo_bool_t;
# 93 "cairo.h"
typedef struct _cairo cairo_t;
# 111 "cairo.h"
typedef struct _cairo_surface cairo_surface_t;
# 130 "cairo.h"
typedef struct _cairo_matrix {
    double xx; double yx;
    double xy; double yy;
    double x0; double y0;
} cairo_matrix_t;
# 155 "cairo.h"
typedef struct _cairo_pattern cairo_pattern_t;
# 165 "cairo.h"
typedef void (*cairo_destroy_func_t) (void *data);
# 177 "cairo.h"
typedef struct _cairo_user_data_key {
    int unused;
} cairo_user_data_key_t;
# 216 "cairo.h"
typedef enum _cairo_status {
    CAIRO_STATUS_SUCCESS = 0,
    CAIRO_STATUS_NO_MEMORY,
    CAIRO_STATUS_INVALID_RESTORE,
    CAIRO_STATUS_INVALID_POP_GROUP,
    CAIRO_STATUS_NO_CURRENT_POINT,
    CAIRO_STATUS_INVALID_MATRIX,
    CAIRO_STATUS_INVALID_STATUS,
    CAIRO_STATUS_NULL_POINTER,
    CAIRO_STATUS_INVALID_STRING,
    CAIRO_STATUS_INVALID_PATH_DATA,
    CAIRO_STATUS_READ_ERROR,
    CAIRO_STATUS_WRITE_ERROR,
    CAIRO_STATUS_SURFACE_FINISHED,
    CAIRO_STATUS_SURFACE_TYPE_MISMATCH,
    CAIRO_STATUS_PATTERN_TYPE_MISMATCH,
    CAIRO_STATUS_INVALID_CONTENT,
    CAIRO_STATUS_INVALID_FORMAT,
    CAIRO_STATUS_INVALID_VISUAL,
    CAIRO_STATUS_FILE_NOT_FOUND,
    CAIRO_STATUS_INVALID_DASH,
    CAIRO_STATUS_INVALID_DSC_COMMENT,
    CAIRO_STATUS_INVALID_INDEX,
    CAIRO_STATUS_CLIP_NOT_REPRESENTABLE,
    CAIRO_STATUS_TEMP_FILE_ERROR

} cairo_status_t;
# 258 "cairo.h"
typedef enum _cairo_content {
    CAIRO_CONTENT_COLOR = 0x1000,
    CAIRO_CONTENT_ALPHA = 0x2000,
    CAIRO_CONTENT_COLOR_ALPHA = 0x3000
} cairo_content_t;
# 280 "cairo.h"
typedef cairo_status_t (*cairo_write_func_t) (void *closure,
           const unsigned char *data,
           unsigned int length);
# 300 "cairo.h"
typedef cairo_status_t (*cairo_read_func_t) (void *closure,
          unsigned char *data,
          unsigned int length);


 cairo_t *
moonlight_cairo_create (cairo_surface_t *target);

 cairo_t *
moonlight_cairo_reference (cairo_t *cr);

 void
moonlight_cairo_destroy (cairo_t *cr);

 unsigned int
moonlight_cairo_get_reference_count (cairo_t *cr);

 void *
moonlight_cairo_get_user_data (cairo_t *cr,
       const cairo_user_data_key_t *key);

 cairo_status_t
moonlight_cairo_set_user_data (cairo_t *cr,
       const cairo_user_data_key_t *key,
       void *user_data,
       cairo_destroy_func_t destroy);

 void
moonlight_cairo_save (cairo_t *cr);

 void
moonlight_cairo_restore (cairo_t *cr);

 void
moonlight_cairo_push_group (cairo_t *cr);

 void
moonlight_cairo_push_group_with_content (cairo_t *cr, cairo_content_t content);

 cairo_pattern_t *
moonlight_cairo_pop_group (cairo_t *cr);

 void
moonlight_cairo_pop_group_to_source (cairo_t *cr);



typedef enum _cairo_operator {
    CAIRO_OPERATOR_CLEAR,

    CAIRO_OPERATOR_SOURCE,
    CAIRO_OPERATOR_OVER,
    CAIRO_OPERATOR_IN,
    CAIRO_OPERATOR_OUT,
    CAIRO_OPERATOR_ATOP,

    CAIRO_OPERATOR_DEST,
    CAIRO_OPERATOR_DEST_OVER,
    CAIRO_OPERATOR_DEST_IN,
    CAIRO_OPERATOR_DEST_OUT,
    CAIRO_OPERATOR_DEST_ATOP,

    CAIRO_OPERATOR_XOR,
    CAIRO_OPERATOR_ADD,
    CAIRO_OPERATOR_SATURATE
} cairo_operator_t;

 void
moonlight_cairo_set_operator (cairo_t *cr, cairo_operator_t op);

 void
moonlight_cairo_set_source (cairo_t *cr, cairo_pattern_t *source);

 void
moonlight_cairo_set_source_rgb (cairo_t *cr, double red, double green, double blue);

 void
moonlight_cairo_set_source_rgba (cairo_t *cr,
         double red, double green, double blue,
         double alpha);

 void
moonlight_cairo_set_source_surface (cairo_t *cr,
     cairo_surface_t *surface,
     double x,
     double y);

 void
moonlight_cairo_set_tolerance (cairo_t *cr, double tolerance);
# 403 "cairo.h"
typedef enum _cairo_antialias {
    CAIRO_ANTIALIAS_DEFAULT,
    CAIRO_ANTIALIAS_NONE,
    CAIRO_ANTIALIAS_GRAY,
    CAIRO_ANTIALIAS_SUBPIXEL
} cairo_antialias_t;

 void
moonlight_cairo_set_antialias (cairo_t *cr, cairo_antialias_t antialias);
# 436 "cairo.h"
typedef enum _cairo_fill_rule {
    CAIRO_FILL_RULE_WINDING,
    CAIRO_FILL_RULE_EVEN_ODD
} cairo_fill_rule_t;

 void
moonlight_cairo_set_fill_rule (cairo_t *cr, cairo_fill_rule_t fill_rule);

 void
moonlight_cairo_set_line_width (cairo_t *cr, double width);
# 455 "cairo.h"
typedef enum _cairo_line_cap {
    CAIRO_LINE_CAP_BUTT,
    CAIRO_LINE_CAP_ROUND,
    CAIRO_LINE_CAP_SQUARE
} cairo_line_cap_t;

 void
moonlight_cairo_set_line_cap (cairo_t *cr, cairo_line_cap_t line_cap);
# 475 "cairo.h"
typedef enum _cairo_line_join {
    CAIRO_LINE_JOIN_MITER,
    CAIRO_LINE_JOIN_ROUND,
    CAIRO_LINE_JOIN_BEVEL
} cairo_line_join_t;

 void
moonlight_cairo_set_line_join (cairo_t *cr, cairo_line_join_t line_join);

 void
moonlight_cairo_set_dash (cairo_t *cr,
  const double *dashes,
  int num_dashes,
  double offset);

 void
moonlight_cairo_set_miter_limit (cairo_t *cr, double limit);

 void
moonlight_cairo_translate (cairo_t *cr, double tx, double ty);

 void
moonlight_cairo_scale (cairo_t *cr, double sx, double sy);

 void
moonlight_cairo_rotate (cairo_t *cr, double angle);

 void
moonlight_cairo_transform (cairo_t *cr,
   const cairo_matrix_t *matrix);

 void
moonlight_cairo_set_matrix (cairo_t *cr,
    const cairo_matrix_t *matrix);

 void
moonlight_cairo_identity_matrix (cairo_t *cr);

 void
moonlight_cairo_user_to_device (cairo_t *cr, double *x, double *y);

 void
moonlight_cairo_user_to_device_distance (cairo_t *cr, double *dx, double *dy);

 void
moonlight_cairo_device_to_user (cairo_t *cr, double *x, double *y);

 void
moonlight_cairo_device_to_user_distance (cairo_t *cr, double *dx, double *dy);


 void
moonlight_cairo_new_path (cairo_t *cr);

 void
moonlight_cairo_move_to (cairo_t *cr, double x, double y);

 void
moonlight_cairo_new_sub_path (cairo_t *cr);

 void
moonlight_cairo_line_to (cairo_t *cr, double x, double y);

 void
moonlight_cairo_curve_to (cairo_t *cr,
  double x1, double y1,
  double x2, double y2,
  double x3, double y3);

 void
moonlight_cairo_arc (cairo_t *cr,
    double xc, double yc,
    double radius,
    double angle1, double angle2);

 void
moonlight_cairo_arc_negative (cairo_t *cr,
      double xc, double yc,
      double radius,
      double angle1, double angle2);
# 564 "cairo.h"
 void
moonlight_cairo_rel_move_to (cairo_t *cr, double dx, double dy);

 void
moonlight_cairo_rel_line_to (cairo_t *cr, double dx, double dy);

 void
moonlight_cairo_rel_curve_to (cairo_t *cr,
      double dx1, double dy1,
      double dx2, double dy2,
      double dx3, double dy3);

 void
moonlight_cairo_rectangle (cairo_t *cr,
   double x, double y,
   double width, double height);






 void
moonlight_cairo_close_path (cairo_t *cr);

 void
cairo_path_extents (cairo_t *cr,
      double *x1, double *y1,
      double *x2, double *y2);


 void
moonlight_cairo_paint (cairo_t *cr);

 void
moonlight_cairo_paint_with_alpha (cairo_t *cr,
   double alpha);

 void
moonlight_cairo_mask (cairo_t *cr,
     cairo_pattern_t *pattern);

 void
moonlight_cairo_mask_surface (cairo_t *cr,
      cairo_surface_t *surface,
      double surface_x,
      double surface_y);

 void
moonlight_cairo_stroke (cairo_t *cr);

 void
moonlight_cairo_stroke_preserve (cairo_t *cr);

 void
moonlight_cairo_fill (cairo_t *cr);

 void
moonlight_cairo_fill_preserve (cairo_t *cr);

 void
moonlight_cairo_copy_page (cairo_t *cr);

 void
moonlight_cairo_show_page (cairo_t *cr);


 cairo_bool_t
moonlight_cairo_in_stroke (cairo_t *cr, double x, double y);

 cairo_bool_t
moonlight_cairo_in_fill (cairo_t *cr, double x, double y);


 void
moonlight_cairo_stroke_extents (cairo_t *cr,
        double *x1, double *y1,
        double *x2, double *y2);

 void
moonlight_cairo_fill_extents (cairo_t *cr,
      double *x1, double *y1,
      double *x2, double *y2);


 void
moonlight_cairo_reset_clip (cairo_t *cr);

 void
moonlight_cairo_clip (cairo_t *cr);

 void
moonlight_cairo_clip_preserve (cairo_t *cr);

 void
moonlight_cairo_clip_extents (cairo_t *cr,
      double *x1, double *y1,
      double *x2, double *y2);
# 674 "cairo.h"
typedef struct _cairo_rectangle {
    double x, y, width, height;
} cairo_rectangle_t;
# 689 "cairo.h"
typedef struct _cairo_rectangle_list {
    cairo_status_t status;
    cairo_rectangle_t *rectangles;
    int num_rectangles;
} cairo_rectangle_list_t;

 cairo_rectangle_list_t *
moonlight_cairo_copy_clip_rectangle_list (cairo_t *cr);

 void
moonlight_cairo_rectangle_list_destroy (cairo_rectangle_list_t *rectangle_list);
# 718 "cairo.h"
typedef struct _cairo_scaled_font cairo_scaled_font_t;
# 737 "cairo.h"
typedef struct _cairo_font_face cairo_font_face_t;
# 763 "cairo.h"
typedef struct {
  unsigned long index;
  double x;
  double y;
} cairo_glyph_t;
# 796 "cairo.h"
typedef struct {
    double x_bearing;
    double y_bearing;
    double width;
    double height;
    double x_advance;
    double y_advance;
} cairo_text_extents_t;
# 846 "cairo.h"
typedef struct {
    double ascent;
    double descent;
    double height;
    double max_x_advance;
    double max_y_advance;
} cairo_font_extents_t;
# 862 "cairo.h"
typedef enum _cairo_font_slant {
  CAIRO_FONT_SLANT_NORMAL,
  CAIRO_FONT_SLANT_ITALIC,
  CAIRO_FONT_SLANT_OBLIQUE
} cairo_font_slant_t;
# 875 "cairo.h"
typedef enum _cairo_font_weight {
  CAIRO_FONT_WEIGHT_NORMAL,
  CAIRO_FONT_WEIGHT_BOLD
} cairo_font_weight_t;
# 897 "cairo.h"
typedef enum _cairo_subpixel_order {
    CAIRO_SUBPIXEL_ORDER_DEFAULT,
    CAIRO_SUBPIXEL_ORDER_RGB,
    CAIRO_SUBPIXEL_ORDER_BGR,
    CAIRO_SUBPIXEL_ORDER_VRGB,
    CAIRO_SUBPIXEL_ORDER_VBGR
} cairo_subpixel_order_t;
# 927 "cairo.h"
typedef enum _cairo_hint_style {
    CAIRO_HINT_STYLE_DEFAULT,
    CAIRO_HINT_STYLE_NONE,
    CAIRO_HINT_STYLE_SLIGHT,
    CAIRO_HINT_STYLE_MEDIUM,
    CAIRO_HINT_STYLE_FULL
} cairo_hint_style_t;
# 948 "cairo.h"
typedef enum _cairo_hint_metrics {
    CAIRO_HINT_METRICS_DEFAULT,
    CAIRO_HINT_METRICS_OFF,
    CAIRO_HINT_METRICS_ON
} cairo_hint_metrics_t;
# 974 "cairo.h"
typedef struct _cairo_font_options cairo_font_options_t;

 cairo_font_options_t *
moonlight_cairo_font_options_create (void);

 cairo_font_options_t *
moonlight_cairo_font_options_copy (const cairo_font_options_t *original);

 void
moonlight_cairo_font_options_destroy (cairo_font_options_t *options);

 cairo_status_t
moonlight_cairo_font_options_status (cairo_font_options_t *options);

 void
moonlight_cairo_font_options_merge (cairo_font_options_t *options,
     const cairo_font_options_t *other);
 cairo_bool_t
moonlight_cairo_font_options_equal (const cairo_font_options_t *options,
     const cairo_font_options_t *other);

 unsigned long
moonlight_cairo_font_options_hash (const cairo_font_options_t *options);

 void
moonlight_cairo_font_options_set_antialias (cairo_font_options_t *options,
      cairo_antialias_t antialias);
 cairo_antialias_t
moonlight_cairo_font_options_get_antialias (const cairo_font_options_t *options);

 void
moonlight_cairo_font_options_set_subpixel_order (cairo_font_options_t *options,
           cairo_subpixel_order_t subpixel_order);
 cairo_subpixel_order_t
moonlight_cairo_font_options_get_subpixel_order (const cairo_font_options_t *options);

 void
moonlight_cairo_font_options_set_hint_style (cairo_font_options_t *options,
       cairo_hint_style_t hint_style);
 cairo_hint_style_t
moonlight_cairo_font_options_get_hint_style (const cairo_font_options_t *options);

 void
moonlight_cairo_font_options_set_hint_metrics (cairo_font_options_t *options,
         cairo_hint_metrics_t hint_metrics);
 cairo_hint_metrics_t
moonlight_cairo_font_options_get_hint_metrics (const cairo_font_options_t *options);




 void
moonlight_cairo_select_font_face (cairo_t *cr,
   const char *family,
   cairo_font_slant_t slant,
   cairo_font_weight_t weight);

 void
moonlight_cairo_set_font_size (cairo_t *cr, double size);

 void
moonlight_cairo_set_font_matrix (cairo_t *cr,
         const cairo_matrix_t *matrix);

 void
moonlight_cairo_get_font_matrix (cairo_t *cr,
         cairo_matrix_t *matrix);

 void
moonlight_cairo_set_font_options (cairo_t *cr,
   const cairo_font_options_t *options);

 void
moonlight_cairo_get_font_options (cairo_t *cr,
   cairo_font_options_t *options);

 void
moonlight_cairo_set_font_face (cairo_t *cr, cairo_font_face_t *font_face);

 cairo_font_face_t *
moonlight_cairo_get_font_face (cairo_t *cr);

 void
moonlight_cairo_set_scaled_font (cairo_t *cr,
         const cairo_scaled_font_t *scaled_font);

 cairo_scaled_font_t *
moonlight_cairo_get_scaled_font (cairo_t *cr);

 void
moonlight_cairo_show_text (cairo_t *cr, const char *utf8);

 void
moonlight_cairo_show_glyphs (cairo_t *cr, const cairo_glyph_t *glyphs, int num_glyphs);

 void
moonlight_cairo_text_path (cairo_t *cr, const char *utf8);

 void
moonlight_cairo_glyph_path (cairo_t *cr, const cairo_glyph_t *glyphs, int num_glyphs);

 void
moonlight_cairo_text_extents (cairo_t *cr,
      const char *utf8,
      cairo_text_extents_t *extents);

 void
moonlight_cairo_glyph_extents (cairo_t *cr,
       const cairo_glyph_t *glyphs,
       int num_glyphs,
       cairo_text_extents_t *extents);

 void
moonlight_cairo_font_extents (cairo_t *cr,
      cairo_font_extents_t *extents);



 cairo_font_face_t *
moonlight_cairo_font_face_reference (cairo_font_face_t *font_face);

 void
moonlight_cairo_font_face_destroy (cairo_font_face_t *font_face);

 unsigned int
moonlight_cairo_font_face_get_reference_count (cairo_font_face_t *font_face);

 cairo_status_t
moonlight_cairo_font_face_status (cairo_font_face_t *font_face);
# 1140 "cairo.h"
typedef enum _cairo_font_type {
    CAIRO_FONT_TYPE_TOY,
    CAIRO_FONT_TYPE_FT,
    CAIRO_FONT_TYPE_WIN32,
    CAIRO_FONT_TYPE_ATSUI
} cairo_font_type_t;

 cairo_font_type_t
moonlight_cairo_font_face_get_type (cairo_font_face_t *font_face);

 void *
moonlight_cairo_font_face_get_user_data (cairo_font_face_t *font_face,
          const cairo_user_data_key_t *key);

 cairo_status_t
moonlight_cairo_font_face_set_user_data (cairo_font_face_t *font_face,
          const cairo_user_data_key_t *key,
          void *user_data,
          cairo_destroy_func_t destroy);



 cairo_scaled_font_t *
moonlight_cairo_scaled_font_create (cairo_font_face_t *font_face,
     const cairo_matrix_t *font_matrix,
     const cairo_matrix_t *ctm,
     const cairo_font_options_t *options);

 cairo_scaled_font_t *
moonlight_cairo_scaled_font_reference (cairo_scaled_font_t *scaled_font);

 void
moonlight_cairo_scaled_font_destroy (cairo_scaled_font_t *scaled_font);

 unsigned int
moonlight_cairo_scaled_font_get_reference_count (cairo_scaled_font_t *scaled_font);

 cairo_status_t
moonlight_cairo_scaled_font_status (cairo_scaled_font_t *scaled_font);

 cairo_font_type_t
moonlight_cairo_scaled_font_get_type (cairo_scaled_font_t *scaled_font);

 void *
moonlight_cairo_scaled_font_get_user_data (cairo_scaled_font_t *scaled_font,
     const cairo_user_data_key_t *key);

 cairo_status_t
moonlight_cairo_scaled_font_set_user_data (cairo_scaled_font_t *scaled_font,
     const cairo_user_data_key_t *key,
     void *user_data,
     cairo_destroy_func_t destroy);

 void
moonlight_cairo_scaled_font_extents (cairo_scaled_font_t *scaled_font,
      cairo_font_extents_t *extents);

 void
moonlight_cairo_scaled_font_text_extents (cairo_scaled_font_t *scaled_font,
    const char *utf8,
    cairo_text_extents_t *extents);

 void
moonlight_cairo_scaled_font_glyph_extents (cairo_scaled_font_t *scaled_font,
     const cairo_glyph_t *glyphs,
     int num_glyphs,
     cairo_text_extents_t *extents);

 cairo_font_face_t *
moonlight_cairo_scaled_font_get_font_face (cairo_scaled_font_t *scaled_font);

 void
moonlight_cairo_scaled_font_get_font_matrix (cairo_scaled_font_t *scaled_font,
       cairo_matrix_t *font_matrix);

 void
moonlight_cairo_scaled_font_get_ctm (cairo_scaled_font_t *scaled_font,
      cairo_matrix_t *ctm);

 void
moonlight_cairo_scaled_font_get_font_options (cairo_scaled_font_t *scaled_font,
        cairo_font_options_t *options);



 cairo_operator_t
moonlight_cairo_get_operator (cairo_t *cr);

 cairo_pattern_t *
moonlight_cairo_get_source (cairo_t *cr);

 double
moonlight_cairo_get_tolerance (cairo_t *cr);

 cairo_antialias_t
moonlight_cairo_get_antialias (cairo_t *cr);

 void
moonlight_cairo_get_current_point (cairo_t *cr, double *x, double *y);

 cairo_fill_rule_t
moonlight_cairo_get_fill_rule (cairo_t *cr);

 double
moonlight_cairo_get_line_width (cairo_t *cr);

 cairo_line_cap_t
moonlight_cairo_get_line_cap (cairo_t *cr);

 cairo_line_join_t
moonlight_cairo_get_line_join (cairo_t *cr);

 double
moonlight_cairo_get_miter_limit (cairo_t *cr);

 int
moonlight_cairo_get_dash_count (cairo_t *cr);

 void
moonlight_cairo_get_dash (cairo_t *cr, double *dashes, double *offset);

 void
moonlight_cairo_get_matrix (cairo_t *cr, cairo_matrix_t *matrix);

 cairo_surface_t *
moonlight_cairo_get_target (cairo_t *cr);

 cairo_surface_t *
moonlight_cairo_get_group_target (cairo_t *cr);
# 1281 "cairo.h"
typedef enum _cairo_path_data_type {
    CAIRO_PATH_MOVE_TO,
    CAIRO_PATH_LINE_TO,
    CAIRO_PATH_CURVE_TO,
    CAIRO_PATH_CLOSE_PATH
} cairo_path_data_type_t;
# 1354 "cairo.h"
typedef union _cairo_path_data_t cairo_path_data_t;
union _cairo_path_data_t {
    struct {
 cairo_path_data_type_t type;
 int length;
    } header;
    struct {
 double x, y;
    } point;
};
# 1384 "cairo.h"
typedef struct cairo_path {
    cairo_status_t status;
    cairo_path_data_t *data;
    int num_data;
} cairo_path_t;

 cairo_path_t *
moonlight_cairo_copy_path (cairo_t *cr);

 cairo_path_t *
moonlight_cairo_copy_path_flat (cairo_t *cr);

 void
moonlight_cairo_append_path (cairo_t *cr,
     const cairo_path_t *path);

 void
moonlight_cairo_path_destroy (cairo_path_t *path);



 cairo_status_t
moonlight_cairo_status (cairo_t *cr);

 const char *
moonlight_cairo_status_to_string (cairo_status_t status);



 cairo_surface_t *
moonlight_cairo_surface_create_similar (cairo_surface_t *other,
         cairo_content_t content,
         int width,
         int height);

 cairo_surface_t *
moonlight_cairo_surface_reference (cairo_surface_t *surface);

 void
moonlight_cairo_surface_finish (cairo_surface_t *surface);

 void
moonlight_cairo_surface_destroy (cairo_surface_t *surface);

 unsigned int
moonlight_cairo_surface_get_reference_count (cairo_surface_t *surface);

 cairo_status_t
moonlight_cairo_surface_status (cairo_surface_t *surface);
# 1473 "cairo.h"
typedef enum _cairo_surface_type {
    CAIRO_SURFACE_TYPE_IMAGE,
    CAIRO_SURFACE_TYPE_PDF,
    CAIRO_SURFACE_TYPE_PS,
    CAIRO_SURFACE_TYPE_XLIB,
    CAIRO_SURFACE_TYPE_XCB,
    CAIRO_SURFACE_TYPE_GLITZ,
    CAIRO_SURFACE_TYPE_QUARTZ,
    CAIRO_SURFACE_TYPE_WIN32,
    CAIRO_SURFACE_TYPE_BEOS,
    CAIRO_SURFACE_TYPE_DIRECTFB,
    CAIRO_SURFACE_TYPE_SVG,
    CAIRO_SURFACE_TYPE_OS2,
    CAIRO_SURFACE_TYPE_WIN32_PRINTING
} cairo_surface_type_t;

 cairo_surface_type_t
moonlight_cairo_surface_get_type (cairo_surface_t *surface);

 cairo_content_t
moonlight_cairo_surface_get_content (cairo_surface_t *surface);



 cairo_status_t
moonlight_cairo_surface_write_to_png (cairo_surface_t *surface,
       const char *filename);

 cairo_status_t
moonlight_cairo_surface_write_to_png_stream (cairo_surface_t *surface,
       cairo_write_func_t write_func,
       void *closure);



 void *
moonlight_cairo_surface_get_user_data (cairo_surface_t *surface,
        const cairo_user_data_key_t *key);

 cairo_status_t
moonlight_cairo_surface_set_user_data (cairo_surface_t *surface,
        const cairo_user_data_key_t *key,
        void *user_data,
        cairo_destroy_func_t destroy);

 void
moonlight_cairo_surface_get_font_options (cairo_surface_t *surface,
    cairo_font_options_t *options);

 void
moonlight_cairo_surface_flush (cairo_surface_t *surface);

 void
moonlight_cairo_surface_mark_dirty (cairo_surface_t *surface);

 void
moonlight_cairo_surface_mark_dirty_rectangle (cairo_surface_t *surface,
        int x,
        int y,
        int width,
        int height);

 void
moonlight_cairo_surface_set_device_offset (cairo_surface_t *surface,
     double x_offset,
     double y_offset);

 void
moonlight_cairo_surface_get_device_offset (cairo_surface_t *surface,
     double *x_offset,
     double *y_offset);

 void
moonlight_cairo_surface_set_fallback_resolution (cairo_surface_t *surface,
           double x_pixels_per_inch,
           double y_pixels_per_inch);

 cairo_status_t
cairo_surface_copy_page (cairo_surface_t *surface);

 cairo_status_t
cairo_surface_show_page (cairo_surface_t *surface);
# 1585 "cairo.h"
typedef enum _cairo_format {
    CAIRO_FORMAT_ARGB32,
    CAIRO_FORMAT_RGB24,
    CAIRO_FORMAT_A8,
    CAIRO_FORMAT_A1




} cairo_format_t;

 cairo_surface_t *
moonlight_cairo_image_surface_create (cairo_format_t format,
       int width,
       int height);

 cairo_surface_t *
moonlight_cairo_image_surface_create_for_data (unsigned char *data,
         cairo_format_t format,
         int width,
         int height,
         int stride);

 unsigned char *
moonlight_cairo_image_surface_get_data (cairo_surface_t *surface);

 cairo_format_t
moonlight_cairo_image_surface_get_format (cairo_surface_t *surface);

 int
moonlight_cairo_image_surface_get_width (cairo_surface_t *surface);

 int
moonlight_cairo_image_surface_get_height (cairo_surface_t *surface);

 int
moonlight_cairo_image_surface_get_stride (cairo_surface_t *surface);



 cairo_surface_t *
moonlight_cairo_image_surface_create_from_png (const char *filename);

 cairo_surface_t *
moonlight_cairo_image_surface_create_from_png_stream (cairo_read_func_t read_func,
         void *closure);





 cairo_pattern_t *
moonlight_cairo_pattern_create_rgb (double red, double green, double blue);

 cairo_pattern_t *
moonlight_cairo_pattern_create_rgba (double red, double green, double blue,
      double alpha);

 cairo_pattern_t *
moonlight_cairo_pattern_create_for_surface (cairo_surface_t *surface);

 cairo_pattern_t *
moonlight_cairo_pattern_create_linear (double x0, double y0,
        double x1, double y1);

 cairo_pattern_t *
moonlight_cairo_pattern_create_radial (double cx0, double cy0, double radius0,
        double cx1, double cy1, double radius1);

 cairo_pattern_t *
moonlight_cairo_pattern_reference (cairo_pattern_t *pattern);

 void
moonlight_cairo_pattern_destroy (cairo_pattern_t *pattern);

 unsigned int
moonlight_cairo_pattern_get_reference_count (cairo_pattern_t *pattern);

 cairo_status_t
moonlight_cairo_pattern_status (cairo_pattern_t *pattern);

 void *
moonlight_cairo_pattern_get_user_data (cairo_pattern_t *pattern,
        const cairo_user_data_key_t *key);

 cairo_status_t
moonlight_cairo_pattern_set_user_data (cairo_pattern_t *pattern,
        const cairo_user_data_key_t *key,
        void *user_data,
        cairo_destroy_func_t destroy);
# 1706 "cairo.h"
typedef enum _cairo_pattern_type {
    CAIRO_PATTERN_TYPE_SOLID,
    CAIRO_PATTERN_TYPE_SURFACE,
    CAIRO_PATTERN_TYPE_LINEAR,
    CAIRO_PATTERN_TYPE_RADIAL
} cairo_pattern_type_t;

 cairo_pattern_type_t
moonlight_cairo_pattern_get_type (cairo_pattern_t *pattern);

 void
moonlight_cairo_pattern_add_color_stop_rgb (cairo_pattern_t *pattern,
      double offset,
      double red, double green, double blue);

 void
moonlight_cairo_pattern_add_color_stop_rgba (cairo_pattern_t *pattern,
       double offset,
       double red, double green, double blue,
       double alpha);

 void
moonlight_cairo_pattern_set_matrix (cairo_pattern_t *pattern,
     const cairo_matrix_t *matrix);

 void
moonlight_cairo_pattern_get_matrix (cairo_pattern_t *pattern,
     cairo_matrix_t *matrix);
# 1751 "cairo.h"
typedef enum _cairo_extend {
    CAIRO_EXTEND_NONE,
    CAIRO_EXTEND_REPEAT,
    CAIRO_EXTEND_REFLECT,
    CAIRO_EXTEND_PAD
} cairo_extend_t;

 void
moonlight_cairo_pattern_set_extend (cairo_pattern_t *pattern, cairo_extend_t extend);

 cairo_extend_t
moonlight_cairo_pattern_get_extend (cairo_pattern_t *pattern);

typedef enum _cairo_filter {
    CAIRO_FILTER_FAST,
    CAIRO_FILTER_GOOD,
    CAIRO_FILTER_BEST,
    CAIRO_FILTER_NEAREST,
    CAIRO_FILTER_BILINEAR,
    CAIRO_FILTER_GAUSSIAN
} cairo_filter_t;

 void
moonlight_cairo_pattern_set_filter (cairo_pattern_t *pattern, cairo_filter_t filter);

 cairo_filter_t
moonlight_cairo_pattern_get_filter (cairo_pattern_t *pattern);

 cairo_status_t
moonlight_cairo_pattern_get_rgba (cairo_pattern_t *pattern,
   double *red, double *green,
   double *blue, double *alpha);

 cairo_status_t
moonlight_cairo_pattern_get_surface (cairo_pattern_t *pattern,
      cairo_surface_t **surface);


 cairo_status_t
moonlight_cairo_pattern_get_color_stop_rgba (cairo_pattern_t *pattern,
       int index, double *offset,
       double *red, double *green,
       double *blue, double *alpha);

 cairo_status_t
moonlight_cairo_pattern_get_color_stop_count (cairo_pattern_t *pattern,
        int *count);

 cairo_status_t
moonlight_cairo_pattern_get_linear_points (cairo_pattern_t *pattern,
     double *x0, double *y0,
     double *x1, double *y1);

 cairo_status_t
moonlight_cairo_pattern_get_radial_circles (cairo_pattern_t *pattern,
      double *x0, double *y0, double *r0,
      double *x1, double *y1, double *r1);



 void
moonlight_cairo_matrix_init (cairo_matrix_t *matrix,
     double xx, double yx,
     double xy, double yy,
     double x0, double y0);

 void
moonlight_cairo_matrix_init_identity (cairo_matrix_t *matrix);

 void
moonlight_cairo_matrix_init_translate (cairo_matrix_t *matrix,
        double tx, double ty);

 void
moonlight_cairo_matrix_init_scale (cairo_matrix_t *matrix,
    double sx, double sy);

 void
moonlight_cairo_matrix_init_rotate (cairo_matrix_t *matrix,
     double radians);

 void
moonlight_cairo_matrix_translate (cairo_matrix_t *matrix, double tx, double ty);

 void
moonlight_cairo_matrix_scale (cairo_matrix_t *matrix, double sx, double sy);

 void
moonlight_cairo_matrix_rotate (cairo_matrix_t *matrix, double radians);

 cairo_status_t
moonlight_cairo_matrix_invert (cairo_matrix_t *matrix);

 void
moonlight_cairo_matrix_multiply (cairo_matrix_t *result,
         const cairo_matrix_t *a,
         const cairo_matrix_t *b);

 void
moonlight_cairo_matrix_transform_distance (const cairo_matrix_t *matrix,
     double *dx, double *dy);

 void
moonlight_cairo_matrix_transform_point (const cairo_matrix_t *matrix,
         double *x, double *y);


 void
moonlight_cairo_debug_reset_static_data (void);


# 67 "cairoint.h" 2
# 1 "/usr/include/pixman-1/pixman.h" 1
# 90 "/usr/include/pixman-1/pixman.h"
# 1 "/usr/include/stdint.h" 1 3 4
# 28 "/usr/include/stdint.h" 3 4
# 1 "/usr/include/bits/wordsize.h" 1 3 4
# 29 "/usr/include/stdint.h" 2 3 4
# 49 "/usr/include/stdint.h" 3 4
typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;

typedef unsigned int uint32_t;





__extension__
typedef unsigned long long int uint64_t;






typedef signed char int_least8_t;
typedef short int int_least16_t;
typedef int int_least32_t;



__extension__
typedef long long int int_least64_t;



typedef unsigned char uint_least8_t;
typedef unsigned short int uint_least16_t;
typedef unsigned int uint_least32_t;



__extension__
typedef unsigned long long int uint_least64_t;






typedef signed char int_fast8_t;





typedef int int_fast16_t;
typedef int int_fast32_t;
__extension__
typedef long long int int_fast64_t;



typedef unsigned char uint_fast8_t;





typedef unsigned int uint_fast16_t;
typedef unsigned int uint_fast32_t;
__extension__
typedef unsigned long long int uint_fast64_t;
# 126 "/usr/include/stdint.h" 3 4
typedef int intptr_t;


typedef unsigned int uintptr_t;
# 138 "/usr/include/stdint.h" 3 4
__extension__
typedef long long int intmax_t;
__extension__
typedef unsigned long long int uintmax_t;
# 91 "/usr/include/pixman-1/pixman.h" 2





typedef int pixman_bool_t;




typedef int64_t pixman_fixed_32_32_t;
typedef pixman_fixed_32_32_t pixman_fixed_48_16_t;
typedef uint32_t pixman_fixed_1_31_t;
typedef uint32_t pixman_fixed_1_16_t;
typedef int32_t pixman_fixed_16_16_t;
typedef pixman_fixed_16_16_t pixman_fixed_t;
# 126 "/usr/include/pixman-1/pixman.h"
typedef struct pixman_color pixman_color_t;
typedef struct pixman_point_fixed pixman_point_fixed_t;
typedef struct pixman_line_fixed pixman_line_fixed_t;
typedef struct pixman_vector pixman_vector_t;
typedef struct pixman_transform pixman_transform_t;

struct pixman_color
{
    uint16_t red;
    uint16_t green;
    uint16_t blue;
    uint16_t alpha;
};

struct pixman_point_fixed
{
    pixman_fixed_t x;
    pixman_fixed_t y;
};

struct pixman_line_fixed
{
    pixman_point_fixed_t p1, p2;
};

struct pixman_vector
{
    pixman_fixed_t vector[3];
};

struct pixman_transform
{
    pixman_fixed_t matrix[3][3];
};

pixman_bool_t pixman_transform_point_3d (pixman_transform_t *transform,
      pixman_vector_t *vector);


typedef enum
{
    PIXMAN_REPEAT_NONE,
    PIXMAN_REPEAT_NORMAL,
    PIXMAN_REPEAT_PAD,
    PIXMAN_REPEAT_REFLECT
} pixman_repeat_t;

typedef enum
{
    PIXMAN_FILTER_FAST,
    PIXMAN_FILTER_GOOD,
    PIXMAN_FILTER_BEST,
    PIXMAN_FILTER_NEAREST,
    PIXMAN_FILTER_BILINEAR,
    PIXMAN_FILTER_CONVOLUTION
} pixman_filter_t;

typedef enum
{
    PIXMAN_OP_CLEAR = 0x00,
    PIXMAN_OP_SRC = 0x01,
    PIXMAN_OP_DST = 0x02,
    PIXMAN_OP_OVER = 0x03,
    PIXMAN_OP_OVER_REVERSE = 0x04,
    PIXMAN_OP_IN = 0x05,
    PIXMAN_OP_IN_REVERSE = 0x06,
    PIXMAN_OP_OUT = 0x07,
    PIXMAN_OP_OUT_REVERSE = 0x08,
    PIXMAN_OP_ATOP = 0x09,
    PIXMAN_OP_ATOP_REVERSE = 0x0a,
    PIXMAN_OP_XOR = 0x0b,
    PIXMAN_OP_ADD = 0x0c,
    PIXMAN_OP_SATURATE = 0x0d,

    PIXMAN_OP_DISJOINT_CLEAR = 0x10,
    PIXMAN_OP_DISJOINT_SRC = 0x11,
    PIXMAN_OP_DISJOINT_DST = 0x12,
    PIXMAN_OP_DISJOINT_OVER = 0x13,
    PIXMAN_OP_DISJOINT_OVER_REVERSE = 0x14,
    PIXMAN_OP_DISJOINT_IN = 0x15,
    PIXMAN_OP_DISJOINT_IN_REVERSE = 0x16,
    PIXMAN_OP_DISJOINT_OUT = 0x17,
    PIXMAN_OP_DISJOINT_OUT_REVERSE = 0x18,
    PIXMAN_OP_DISJOINT_ATOP = 0x19,
    PIXMAN_OP_DISJOINT_ATOP_REVERSE = 0x1a,
    PIXMAN_OP_DISJOINT_XOR = 0x1b,

    PIXMAN_OP_CONJOINT_CLEAR = 0x20,
    PIXMAN_OP_CONJOINT_SRC = 0x21,
    PIXMAN_OP_CONJOINT_DST = 0x22,
    PIXMAN_OP_CONJOINT_OVER = 0x23,
    PIXMAN_OP_CONJOINT_OVER_REVERSE = 0x24,
    PIXMAN_OP_CONJOINT_IN = 0x25,
    PIXMAN_OP_CONJOINT_IN_REVERSE = 0x26,
    PIXMAN_OP_CONJOINT_OUT = 0x27,
    PIXMAN_OP_CONJOINT_OUT_REVERSE = 0x28,
    PIXMAN_OP_CONJOINT_ATOP = 0x29,
    PIXMAN_OP_CONJOINT_ATOP_REVERSE = 0x2a,
    PIXMAN_OP_CONJOINT_XOR = 0x2b
} pixman_op_t;




typedef struct pixman_region16_data pixman_region16_data_t;
typedef struct pixman_box16 pixman_box16_t;
typedef struct pixman_rectangle16 pixman_rectangle16_t;
typedef struct pixman_region16 pixman_region16_t;

struct pixman_region16_data {
    long size;
    long numRects;

};

struct pixman_rectangle16
{
    int16_t x, y;
    uint16_t width, height;
};

struct pixman_box16
{
    int16_t x1, y1, x2, y2;
};

struct pixman_region16
{
    pixman_box16_t extents;
    pixman_region16_data_t *data;
};

typedef enum
{
    PIXMAN_REGION_OUT,
    PIXMAN_REGION_IN,
    PIXMAN_REGION_PART
} pixman_region_overlap_t;




void pixman_region_set_static_pointers (pixman_box16_t *empty_box,
          pixman_region16_data_t *empty_data,
          pixman_region16_data_t *broken_data);


void pixman_region_init (pixman_region16_t *region);
void pixman_region_init_rect (pixman_region16_t *region,
        int x,
        int y,
        unsigned int width,
        unsigned int height);
void pixman_region_init_with_extents (pixman_region16_t *region,
        pixman_box16_t *extents);
void pixman_region_fini (pixman_region16_t *region);


void pixman_region_translate (pixman_region16_t *region,
        int x,
        int y);
pixman_bool_t pixman_region_copy (pixman_region16_t *dest,
        pixman_region16_t *source);
pixman_bool_t pixman_region_intersect (pixman_region16_t *newReg,
        pixman_region16_t *reg1,
        pixman_region16_t *reg2);
pixman_bool_t pixman_region_union (pixman_region16_t *newReg,
        pixman_region16_t *reg1,
        pixman_region16_t *reg2);
pixman_bool_t pixman_region_union_rect (pixman_region16_t *dest,
        pixman_region16_t *source,
        int x,
        int y,
        unsigned int width,
        unsigned int height);
pixman_bool_t pixman_region_subtract (pixman_region16_t *regD,
        pixman_region16_t *regM,
        pixman_region16_t *regS);
pixman_bool_t pixman_region_inverse (pixman_region16_t *newReg,
        pixman_region16_t *reg1,
        pixman_box16_t *invRect);
pixman_bool_t pixman_region_contains_point (pixman_region16_t *region,
            int x, int y, pixman_box16_t *box);
pixman_region_overlap_t pixman_region_contains_rectangle (pixman_region16_t *pixman_region16_t,
         pixman_box16_t *prect);
pixman_bool_t pixman_region_not_empty (pixman_region16_t *region);
pixman_box16_t * pixman_region_extents (pixman_region16_t *region);
int pixman_region_n_rects (pixman_region16_t *region);
pixman_box16_t * pixman_region_rectangles (pixman_region16_t *region,
        int *n_rects);
pixman_bool_t pixman_region_equal (pixman_region16_t *region1,
          pixman_region16_t *region2);
pixman_bool_t pixman_region_selfcheck (pixman_region16_t *region);
void pixman_region_reset (pixman_region16_t *region, pixman_box16_t *box);
pixman_bool_t pixman_region_init_rects (pixman_region16_t *region,
        pixman_box16_t *boxes, int count);


pixman_bool_t pixman_blt (uint32_t *src_bits,
     uint32_t *dst_bits,
     int src_stride,
     int dst_stride,
     int src_bpp,
     int dst_bpp,
     int src_x, int src_y,
     int dst_x, int dst_y,
     int width, int height);
pixman_bool_t pixman_fill (uint32_t *bits,
      int stride,
      int bpp,
      int x,
      int y,
      int width,
      int height,
      uint32_t xor);



typedef union pixman_image pixman_image_t;
typedef struct pixman_indexed pixman_indexed_t;
typedef struct pixman_gradient_stop pixman_gradient_stop_t;

typedef uint32_t (* pixman_read_memory_func_t) (const void *src, int size);
typedef void (* pixman_write_memory_func_t) (void *dst, uint32_t value, int size);

struct pixman_gradient_stop {
    pixman_fixed_t x;
    pixman_color_t color;
};




typedef uint8_t pixman_index_type;


struct pixman_indexed
{
    pixman_bool_t color;
    uint32_t rgba[256];
    pixman_index_type ent[32768];
};
# 406 "/usr/include/pixman-1/pixman.h"
typedef enum {
    PIXMAN_a8r8g8b8 = (((32) << 24) | ((2) << 16) | ((8) << 12) | ((8) << 8) | ((8) << 4) | ((8))),
    PIXMAN_x8r8g8b8 = (((32) << 24) | ((2) << 16) | ((0) << 12) | ((8) << 8) | ((8) << 4) | ((8))),
    PIXMAN_a8b8g8r8 = (((32) << 24) | ((3) << 16) | ((8) << 12) | ((8) << 8) | ((8) << 4) | ((8))),
    PIXMAN_x8b8g8r8 = (((32) << 24) | ((3) << 16) | ((0) << 12) | ((8) << 8) | ((8) << 4) | ((8))),


    PIXMAN_r8g8b8 = (((24) << 24) | ((2) << 16) | ((0) << 12) | ((8) << 8) | ((8) << 4) | ((8))),
    PIXMAN_b8g8r8 = (((24) << 24) | ((3) << 16) | ((0) << 12) | ((8) << 8) | ((8) << 4) | ((8))),


    PIXMAN_r5g6b5 = (((16) << 24) | ((2) << 16) | ((0) << 12) | ((5) << 8) | ((6) << 4) | ((5))),
    PIXMAN_b5g6r5 = (((16) << 24) | ((3) << 16) | ((0) << 12) | ((5) << 8) | ((6) << 4) | ((5))),

    PIXMAN_a1r5g5b5 = (((16) << 24) | ((2) << 16) | ((1) << 12) | ((5) << 8) | ((5) << 4) | ((5))),
    PIXMAN_x1r5g5b5 = (((16) << 24) | ((2) << 16) | ((0) << 12) | ((5) << 8) | ((5) << 4) | ((5))),
    PIXMAN_a1b5g5r5 = (((16) << 24) | ((3) << 16) | ((1) << 12) | ((5) << 8) | ((5) << 4) | ((5))),
    PIXMAN_x1b5g5r5 = (((16) << 24) | ((3) << 16) | ((0) << 12) | ((5) << 8) | ((5) << 4) | ((5))),
    PIXMAN_a4r4g4b4 = (((16) << 24) | ((2) << 16) | ((4) << 12) | ((4) << 8) | ((4) << 4) | ((4))),
    PIXMAN_x4r4g4b4 = (((16) << 24) | ((2) << 16) | ((0) << 12) | ((4) << 8) | ((4) << 4) | ((4))),
    PIXMAN_a4b4g4r4 = (((16) << 24) | ((3) << 16) | ((4) << 12) | ((4) << 8) | ((4) << 4) | ((4))),
    PIXMAN_x4b4g4r4 = (((16) << 24) | ((3) << 16) | ((0) << 12) | ((4) << 8) | ((4) << 4) | ((4))),


    PIXMAN_a8 = (((8) << 24) | ((1) << 16) | ((8) << 12) | ((0) << 8) | ((0) << 4) | ((0))),
    PIXMAN_r3g3b2 = (((8) << 24) | ((2) << 16) | ((0) << 12) | ((3) << 8) | ((3) << 4) | ((2))),
    PIXMAN_b2g3r3 = (((8) << 24) | ((3) << 16) | ((0) << 12) | ((3) << 8) | ((3) << 4) | ((2))),
    PIXMAN_a2r2g2b2 = (((8) << 24) | ((2) << 16) | ((2) << 12) | ((2) << 8) | ((2) << 4) | ((2))),
    PIXMAN_a2b2g2r2 = (((8) << 24) | ((3) << 16) | ((2) << 12) | ((2) << 8) | ((2) << 4) | ((2))),

    PIXMAN_c8 = (((8) << 24) | ((4) << 16) | ((0) << 12) | ((0) << 8) | ((0) << 4) | ((0))),
    PIXMAN_g8 = (((8) << 24) | ((5) << 16) | ((0) << 12) | ((0) << 8) | ((0) << 4) | ((0))),

    PIXMAN_x4a4 = (((8) << 24) | ((1) << 16) | ((4) << 12) | ((0) << 8) | ((0) << 4) | ((0))),

    PIXMAN_x4c4 = (((8) << 24) | ((4) << 16) | ((0) << 12) | ((0) << 8) | ((0) << 4) | ((0))),
    PIXMAN_x4g4 = (((8) << 24) | ((5) << 16) | ((0) << 12) | ((0) << 8) | ((0) << 4) | ((0))),


    PIXMAN_a4 = (((4) << 24) | ((1) << 16) | ((4) << 12) | ((0) << 8) | ((0) << 4) | ((0))),
    PIXMAN_r1g2b1 = (((4) << 24) | ((2) << 16) | ((0) << 12) | ((1) << 8) | ((2) << 4) | ((1))),
    PIXMAN_b1g2r1 = (((4) << 24) | ((3) << 16) | ((0) << 12) | ((1) << 8) | ((2) << 4) | ((1))),
    PIXMAN_a1r1g1b1 = (((4) << 24) | ((2) << 16) | ((1) << 12) | ((1) << 8) | ((1) << 4) | ((1))),
    PIXMAN_a1b1g1r1 = (((4) << 24) | ((3) << 16) | ((1) << 12) | ((1) << 8) | ((1) << 4) | ((1))),

    PIXMAN_c4 = (((4) << 24) | ((4) << 16) | ((0) << 12) | ((0) << 8) | ((0) << 4) | ((0))),
    PIXMAN_g4 = (((4) << 24) | ((5) << 16) | ((0) << 12) | ((0) << 8) | ((0) << 4) | ((0))),


    PIXMAN_a1 = (((1) << 24) | ((1) << 16) | ((1) << 12) | ((0) << 8) | ((0) << 4) | ((0))),

    PIXMAN_g1 = (((1) << 24) | ((5) << 16) | ((0) << 12) | ((0) << 8) | ((0) << 4) | ((0))),


    PIXMAN_yuy2 = (((16) << 24) | ((6) << 16) | ((0) << 12) | ((0) << 8) | ((0) << 4) | ((0))),
    PIXMAN_yv12 = (((12) << 24) | ((7) << 16) | ((0) << 12) | ((0) << 8) | ((0) << 4) | ((0)))
} pixman_format_code_t;


pixman_image_t *pixman_image_create_solid_fill (pixman_color_t *color);
pixman_image_t *pixman_image_create_linear_gradient (pixman_point_fixed_t *p1,
            pixman_point_fixed_t *p2,
            const pixman_gradient_stop_t *stops,
            int n_stops);
pixman_image_t *pixman_image_create_radial_gradient (pixman_point_fixed_t *inner,
            pixman_point_fixed_t *outer,
            pixman_fixed_t inner_radius,
            pixman_fixed_t outer_radius,
            const pixman_gradient_stop_t *stops,
            int n_stops);
pixman_image_t *pixman_image_create_conical_gradient (pixman_point_fixed_t *center,
            pixman_fixed_t angle,
            const pixman_gradient_stop_t *stops,
            int n_stops);
pixman_image_t *pixman_image_create_bits (pixman_format_code_t format,
            int width,
            int height,
            uint32_t *bits,
            int rowstride_bytes);


pixman_image_t *pixman_image_ref (pixman_image_t *image);
pixman_bool_t pixman_image_unref (pixman_image_t *image);



pixman_bool_t pixman_image_set_clip_region (pixman_image_t *image,
            pixman_region16_t *region);
void pixman_image_set_has_client_clip (pixman_image_t *image,
            pixman_bool_t clien_clip);
pixman_bool_t pixman_image_set_transform (pixman_image_t *image,
            const pixman_transform_t *transform);
void pixman_image_set_repeat (pixman_image_t *image,
            pixman_repeat_t repeat);
pixman_bool_t pixman_image_set_filter (pixman_image_t *image,
            pixman_filter_t filter,
            const pixman_fixed_t *filter_params,
            int n_filter_params);
void pixman_image_set_filter_params (pixman_image_t *image,
            pixman_fixed_t *params,
            int n_params);
void pixman_image_set_source_clipping (pixman_image_t *image,
            pixman_bool_t source_clipping);
void pixman_image_set_alpha_map (pixman_image_t *image,
            pixman_image_t *alpha_map,
            int16_t x,
            int16_t y);
void pixman_image_set_component_alpha (pixman_image_t *image,
            pixman_bool_t component_alpha);
void pixman_image_set_accessors (pixman_image_t *image,
            pixman_read_memory_func_t read_func,
            pixman_write_memory_func_t write_func);
void pixman_image_set_indexed (pixman_image_t *image,
            const pixman_indexed_t *indexed);
uint32_t *pixman_image_get_data (pixman_image_t *image);
int pixman_image_get_width (pixman_image_t *image);
int pixman_image_get_height (pixman_image_t *image);
int pixman_image_get_stride (pixman_image_t *image);
int pixman_image_get_depth (pixman_image_t *image);
pixman_bool_t pixman_image_fill_rectangles (pixman_op_t op,
            pixman_image_t *image,
            pixman_color_t *color,
            int n_rects,
            const pixman_rectangle16_t *rects);


pixman_bool_t pixman_compute_composite_region (pixman_region16_t * pRegion,
       pixman_image_t * pSrc,
       pixman_image_t * pMask,
       pixman_image_t * pDst,
       int16_t xSrc,
       int16_t ySrc,
       int16_t xMask,
       int16_t yMask,
       int16_t xDst,
       int16_t yDst,
       uint16_t width,
       uint16_t height);
void pixman_image_composite (pixman_op_t op,
       pixman_image_t *src,
       pixman_image_t *mask,
       pixman_image_t *dest,
       int16_t src_x,
       int16_t src_y,
       int16_t mask_x,
       int16_t mask_y,
       int16_t dest_x,
       int16_t dest_y,
       uint16_t width,
       uint16_t height);




typedef struct pixman_edge pixman_edge_t;
typedef struct pixman_trapezoid pixman_trapezoid_t;
typedef struct pixman_trap pixman_trap_t;
typedef struct pixman_span_fix pixman_span_fix_t;






struct pixman_edge
{
    pixman_fixed_t x;
    pixman_fixed_t e;
    pixman_fixed_t stepx;
    pixman_fixed_t signdx;
    pixman_fixed_t dy;
    pixman_fixed_t dx;

    pixman_fixed_t stepx_small;
    pixman_fixed_t stepx_big;
    pixman_fixed_t dx_small;
    pixman_fixed_t dx_big;
};

struct pixman_trapezoid
{
    pixman_fixed_t top, bottom;
    pixman_line_fixed_t left, right;
};
# 598 "/usr/include/pixman-1/pixman.h"
struct pixman_span_fix
{
    pixman_fixed_t l, r, y;
};

struct pixman_trap
{
    pixman_span_fix_t top, bot;
};

pixman_fixed_t pixman_sample_ceil_y (pixman_fixed_t y,
         int bpp);
pixman_fixed_t pixman_sample_floor_y (pixman_fixed_t y,
         int bpp);
void pixman_edge_step (pixman_edge_t *e,
         int n);
void pixman_edge_init (pixman_edge_t *e,
         int bpp,
         pixman_fixed_t y_start,
         pixman_fixed_t x_top,
         pixman_fixed_t y_top,
         pixman_fixed_t x_bot,
         pixman_fixed_t y_bot);
void pixman_line_fixed_edge_init (pixman_edge_t *e,
         int bpp,
         pixman_fixed_t y,
         const pixman_line_fixed_t *line,
         int x_off,
         int y_off);
void pixman_rasterize_edges (pixman_image_t *image,
         pixman_edge_t *l,
         pixman_edge_t *r,
         pixman_fixed_t t,
         pixman_fixed_t b);
void pixman_add_traps (pixman_image_t *image,
         int16_t x_off,
         int16_t y_off,
         int ntrap,
         pixman_trap_t *traps);
void pixman_add_trapezoids (pixman_image_t *image,
         int16_t x_off,
         int y_off,
         int ntraps,
         const pixman_trapezoid_t *traps);
void pixman_rasterize_trapezoid (pixman_image_t *image,
         const pixman_trapezoid_t *trap,
         int x_off,
         int y_off);
# 68 "cairoint.h" 2

# 1 "cairo-compiler-private.h" 1
# 41 "cairo-compiler-private.h"

# 125 "cairo-compiler-private.h"

# 70 "cairoint.h" 2


# 146 "cairoint.h"
static inline uint16_t
cpu_to_be16(uint16_t v)
{
    return (v << 8) | (v >> 8);
}

static inline uint16_t
be16_to_cpu(uint16_t v)
{
    return cpu_to_be16 (v);
}

static inline uint32_t
cpu_to_be32(uint32_t v)
{
    return (cpu_to_be16 (v) << 16) | cpu_to_be16 (v >> 16);
}

static inline uint32_t
be32_to_cpu(uint32_t v)
{
    return cpu_to_be32 (v);
}



# 1 "cairo-types-private.h" 1
# 44 "cairo-types-private.h"
# 1 "cairo-fixed-type-private.h" 1
# 40 "cairo-fixed-type-private.h"
# 1 "cairo-wideint-type-private.h" 1
# 41 "cairo-wideint-type-private.h"
# 1 "../config.h" 1
# 42 "cairo-wideint-type-private.h" 2
# 90 "cairo-wideint-type-private.h"
typedef uint64_t cairo_uint64_t;
typedef int64_t cairo_int64_t;



typedef struct _cairo_uquorem64 {
    cairo_uint64_t quo;
    cairo_uint64_t rem;
} cairo_uquorem64_t;

typedef struct _cairo_quorem64 {
    cairo_int64_t quo;
    cairo_int64_t rem;
} cairo_quorem64_t;




typedef struct cairo_uint128 {
    cairo_uint64_t lo, hi;
} cairo_uint128_t, cairo_int128_t;
# 119 "cairo-wideint-type-private.h"
typedef struct _cairo_uquorem128 {
    cairo_uint128_t quo;
    cairo_uint128_t rem;
} cairo_uquorem128_t;

typedef struct _cairo_quorem128 {
    cairo_int128_t quo;
    cairo_int128_t rem;
} cairo_quorem128_t;
# 41 "cairo-fixed-type-private.h" 2





typedef int32_t cairo_fixed_16_16_t;
typedef cairo_int64_t cairo_fixed_32_32_t;
typedef cairo_int64_t cairo_fixed_48_16_t;
typedef cairo_int128_t cairo_fixed_64_64_t;
typedef cairo_int128_t cairo_fixed_96_32_t;
# 65 "cairo-fixed-type-private.h"
typedef int32_t cairo_fixed_t;


typedef uint32_t cairo_fixed_unsigned_t;
# 45 "cairo-types-private.h" 2

typedef struct _cairo_array cairo_array_t;
typedef struct _cairo_hash_table cairo_hash_table_t;
typedef struct _cairo_cache cairo_cache_t;
typedef struct _cairo_hash_entry cairo_hash_entry_t;
typedef struct _cairo_surface_backend cairo_surface_backend_t;
typedef struct _cairo_clip cairo_clip_t;
typedef struct _cairo_output_stream cairo_output_stream_t;
typedef struct _cairo_scaled_font_subsets cairo_scaled_font_subsets_t;
typedef struct _cairo_paginated_surface_backend cairo_paginated_surface_backend_t;
typedef struct _cairo_scaled_font_backend cairo_scaled_font_backend_t;
typedef struct _cairo_font_face_backend cairo_font_face_backend_t;
typedef struct _cairo_xlib_screen_info cairo_xlib_screen_info_t;
typedef cairo_array_t cairo_user_data_array_t;
# 94 "cairo-types-private.h"
struct _cairo_hash_entry {
    unsigned long hash;
};

struct _cairo_array {
    unsigned int size;
    unsigned int num_elements;
    unsigned int element_size;
    char **elements;

    cairo_bool_t is_snapshot;
};

struct _cairo_font_options {
    cairo_antialias_t antialias;
    cairo_subpixel_order_t subpixel_order;
    cairo_hint_style_t hint_style;
    cairo_hint_metrics_t hint_metrics;
};

struct _cairo_cache {
    cairo_hash_table_t *hash_table;

    cairo_destroy_func_t entry_destroy;

    unsigned long max_size;
    unsigned long size;

    int freeze_count;
};

typedef enum _cairo_paginated_mode {
    CAIRO_PAGINATED_MODE_ANALYZE,
    CAIRO_PAGINATED_MODE_RENDER,
    CAIRO_PAGINATED_MODE_FALLBACK
} cairo_paginated_mode_t;




typedef enum _cairo_int_status {
    CAIRO_INT_STATUS_DEGENERATE = 1000,
    CAIRO_INT_STATUS_UNSUPPORTED,
    CAIRO_INT_STATUS_NOTHING_TO_DO,
    CAIRO_INT_STATUS_CACHE_EMPTY,
    CAIRO_INT_STATUS_FLATTEN_TRANSPARENCY,
    CAIRO_INT_STATUS_IMAGE_FALLBACK,
    CAIRO_INT_STATUS_ANALYZE_META_SURFACE_PATTERN
} cairo_int_status_t;

typedef enum _cairo_internal_surface_type {
    CAIRO_INTERNAL_SURFACE_TYPE_META = 0x1000,
    CAIRO_INTERNAL_SURFACE_TYPE_PAGINATED,
    CAIRO_INTERNAL_SURFACE_TYPE_ANALYSIS,
    CAIRO_INTERNAL_SURFACE_TYPE_TEST_META,
    CAIRO_INTERNAL_SURFACE_TYPE_TEST_FALLBACK,
    CAIRO_INTERNAL_SURFACE_TYPE_TEST_PAGINATED
} cairo_internal_surface_type_t;

typedef struct _cairo_region cairo_region_t;

typedef struct _cairo_point {
    cairo_fixed_t x;
    cairo_fixed_t y;
} cairo_point_t;

typedef struct _cairo_slope
{
    cairo_fixed_t dx;
    cairo_fixed_t dy;
} cairo_slope_t, cairo_distance_t;

typedef struct _cairo_point_double {
    double x;
    double y;
} cairo_point_double_t;

typedef struct _cairo_distance_double {
    double dx;
    double dy;
} cairo_distance_double_t;

typedef struct _cairo_line {
    cairo_point_t p1;
    cairo_point_t p2;
} cairo_line_t, cairo_box_t;

typedef struct _cairo_trapezoid {
    cairo_fixed_t top, bottom;
    cairo_line_t left, right;
} cairo_trapezoid_t;

typedef struct _cairo_rectangle_int16 {
    int16_t x, y;
    uint16_t width, height;
} cairo_rectangle_int16_t, cairo_glyph_size_t;

typedef struct _cairo_rectangle_int32 {
    int32_t x, y;
    uint32_t width, height;
} cairo_rectangle_int32_t;

typedef struct _cairo_point_int16 {
    int16_t x, y;
} cairo_point_int16_t;

typedef struct _cairo_point_int32 {
    int32_t x, y;
} cairo_point_int32_t;

typedef struct _cairo_box_int16 {
    cairo_point_int16_t p1;
    cairo_point_int16_t p2;
} cairo_box_int16_t;

typedef struct _cairo_box_int32 {
    cairo_point_int32_t p1;
    cairo_point_int32_t p2;
} cairo_box_int32_t;
# 222 "cairo-types-private.h"
typedef cairo_rectangle_int32_t cairo_rectangle_int_t;
typedef cairo_point_int32_t cairo_point_int_t;
typedef cairo_box_int32_t cairo_box_int_t;






typedef enum _cairo_direction {
    CAIRO_DIRECTION_FORWARD,
    CAIRO_DIRECTION_REVERSE
} cairo_direction_t;

typedef struct _cairo_path_fixed cairo_path_fixed_t;
typedef enum _cairo_clip_mode {
    CAIRO_CLIP_MODE_PATH,
    CAIRO_CLIP_MODE_REGION,
    CAIRO_CLIP_MODE_MASK
} cairo_clip_mode_t;
typedef struct _cairo_clip_path cairo_clip_path_t;

typedef struct _cairo_edge {
    cairo_line_t edge;
    int clockWise;

    cairo_fixed_t current_x;
} cairo_edge_t;

typedef struct _cairo_polygon {
    cairo_status_t status;

    cairo_point_t first_point;
    cairo_point_t current_point;
    cairo_bool_t has_current_point;

    int num_edges;
    int edges_size;
    cairo_edge_t *edges;
    cairo_edge_t edges_embedded[8];
} cairo_polygon_t;

typedef struct _cairo_spline_knots {
    cairo_point_t a, b, c, d;
} cairo_spline_knots_t;
typedef struct _cairo_spline {
    cairo_spline_knots_t knots;

    cairo_slope_t initial_slope;
    cairo_slope_t final_slope;

    int num_points;
    int points_size;
    cairo_point_t *points;
    cairo_point_t points_embedded[8];
} cairo_spline_t;

typedef struct _cairo_pen_vertex {
    cairo_point_t point;

    cairo_slope_t slope_ccw;
    cairo_slope_t slope_cw;
} cairo_pen_vertex_t;

typedef struct _cairo_pen {
    double radius;
    double tolerance;

    cairo_pen_vertex_t *vertices;
    int num_vertices;
} cairo_pen_t;

typedef struct _cairo_color cairo_color_t;
typedef struct _cairo_image_surface cairo_image_surface_t;

typedef struct _cairo_stroke_style {
    double line_width;
    cairo_line_cap_t line_cap;
    cairo_line_join_t line_join;
    double miter_limit;
    double *dash;
    unsigned int num_dashes;
    double dash_offset;
} cairo_stroke_style_t;

typedef struct _cairo_format_masks {
    int bpp;
    unsigned long alpha_mask;
    unsigned long red_mask;
    unsigned long green_mask;
    unsigned long blue_mask;
} cairo_format_masks_t;

typedef enum {
    CAIRO_STOCK_WHITE,
    CAIRO_STOCK_BLACK,
    CAIRO_STOCK_TRANSPARENT
} cairo_stock_t;
# 173 "cairoint.h" 2
# 1 "cairo-cache-private.h" 1
# 86 "cairo-cache-private.h"
typedef struct _cairo_cache_entry {
    unsigned long hash;
    unsigned long size;
} cairo_cache_entry_t;

typedef cairo_bool_t
(*cairo_cache_keys_equal_func_t) (const void *key_a, const void *key_b);

typedef void
(*cairo_cache_callback_func_t) (void *entry,
    void *closure);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_cache_t *
_moonlight_cairo_cache_create (cairo_cache_keys_equal_func_t keys_equal,
       cairo_destroy_func_t entry_destroy,
       unsigned long max_size);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_cache_destroy (cairo_cache_t *cache);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_cache_freeze (cairo_cache_t *cache);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_cache_thaw (cairo_cache_t *cache);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_bool_t
_moonlight_cairo_cache_lookup (cairo_cache_t *cache,
       cairo_cache_entry_t *key,
       cairo_cache_entry_t **entry_return);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_cache_insert (cairo_cache_t *cache,
       cairo_cache_entry_t *entry);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_cache_foreach (cairo_cache_t *cache,
        cairo_cache_callback_func_t cache_callback,
        void *closure);
# 174 "cairoint.h" 2
# 1 "cairo-reference-count-private.h" 1
# 40 "cairo-reference-count-private.h"
# 1 "cairo-atomic-private.h" 1
# 41 "cairo-atomic-private.h"
# 1 "../config.h" 1
# 42 "cairo-atomic-private.h" 2



# 62 "cairo-atomic-private.h"
typedef int cairo_atomic_int_t;

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_cairo_atomic_int_inc (int *x);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_bool_t
_cairo_atomic_int_dec_and_test (int *x);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) int
_cairo_atomic_int_cmpxchg (int *x, int oldv, int newv);
# 101 "cairo-atomic-private.h"

# 41 "cairo-reference-count-private.h" 2




typedef struct {
    cairo_atomic_int_t ref_count;
} cairo_reference_count_t;
# 64 "cairo-reference-count-private.h"

# 175 "cairoint.h" 2

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_box_round_to_rectangle (cairo_box_t *box, cairo_rectangle_int_t *rectangle);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_rectangle_intersect (cairo_rectangle_int_t *dest, cairo_rectangle_int_t *src);



__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_array_init (cairo_array_t *array, int element_size);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_array_init_snapshot (cairo_array_t *array,
       const cairo_array_t *other);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_array_fini (cairo_array_t *array);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_array_grow_by (cairo_array_t *array, int additional);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_array_truncate (cairo_array_t *array, unsigned int num_elements);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_array_append (cairo_array_t *array, const void *element);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_array_append_multiple (cairo_array_t *array,
         const void *elements,
         int num_elements);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_array_allocate (cairo_array_t *array,
         unsigned int num_elements,
         void **elements);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void *
_moonlight_cairo_array_index (cairo_array_t *array, unsigned int index);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_array_copy_element (cairo_array_t *array, int index, void *dst);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) int
_moonlight_cairo_array_num_elements (cairo_array_t *array);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) int
_moonlight_cairo_array_size (cairo_array_t *array);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_user_data_array_init (cairo_user_data_array_t *array);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_user_data_array_fini (cairo_user_data_array_t *array);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void *
_moonlight_cairo_user_data_array_get_data (cairo_user_data_array_t *array,
     const cairo_user_data_key_t *key);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_user_data_array_set_data (cairo_user_data_array_t *array,
     const cairo_user_data_key_t *key,
     void *user_data,
     cairo_destroy_func_t destroy);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) unsigned long
_moonlight_cairo_hash_string (const char *c);

typedef struct _cairo_unscaled_font_backend cairo_unscaled_font_backend_t;





typedef struct _cairo_unscaled_font {
    cairo_hash_entry_t hash_entry;
    cairo_reference_count_t ref_count;
    const cairo_unscaled_font_backend_t *backend;
} cairo_unscaled_font_t;

typedef struct _cairo_scaled_glyph {
    cairo_cache_entry_t cache_entry;
    cairo_scaled_font_t *scaled_font;
    cairo_text_extents_t metrics;
    cairo_box_t bbox;
    int16_t x_advance;
    int16_t y_advance;
    cairo_image_surface_t *surface;
    cairo_path_fixed_t *path;
    void *surface_private;
} cairo_scaled_glyph_t;




# 1 "cairo-scaled-font-private.h" 1
# 44 "cairo-scaled-font-private.h"
# 1 "cairo-mutex-type-private.h" 1
# 45 "cairo-mutex-type-private.h"
# 1 "../config.h" 1
# 46 "cairo-mutex-type-private.h" 2





# 155 "cairo-mutex-type-private.h"
# 1 "/usr/include/pthread.h" 1 3 4
# 24 "/usr/include/pthread.h" 3 4
# 1 "/usr/include/sched.h" 1 3 4
# 29 "/usr/include/sched.h" 3 4
# 1 "/usr/include/time.h" 1 3 4
# 30 "/usr/include/sched.h" 2 3 4


# 1 "/usr/include/bits/sched.h" 1 3 4
# 66 "/usr/include/bits/sched.h" 3 4
struct sched_param
  {
    int __sched_priority;
  };





extern int clone (int (*__fn) (void *__arg), void *__child_stack,
    int __flags, void *__arg, ...) __attribute__ ((__nothrow__));


extern int unshare (int __flags) __attribute__ ((__nothrow__));


extern int sched_getcpu (void) __attribute__ ((__nothrow__));










struct __sched_param
  {
    int __sched_priority;
  };
# 108 "/usr/include/bits/sched.h" 3 4
typedef unsigned long int __cpu_mask;






typedef struct
{
  __cpu_mask __bits[1024 / (8 * sizeof (__cpu_mask))];
} cpu_set_t;
# 134 "/usr/include/bits/sched.h" 3 4
extern int __sched_cpucount (size_t __setsize, cpu_set_t *__setp) __attribute__ ((__nothrow__));
# 33 "/usr/include/sched.h" 2 3 4







extern int sched_setparam (__pid_t __pid, __const struct sched_param *__param)
     __attribute__ ((__nothrow__));


extern int sched_getparam (__pid_t __pid, struct sched_param *__param) __attribute__ ((__nothrow__));


extern int sched_setscheduler (__pid_t __pid, int __policy,
          __const struct sched_param *__param) __attribute__ ((__nothrow__));


extern int sched_getscheduler (__pid_t __pid) __attribute__ ((__nothrow__));


extern int sched_yield (void) __attribute__ ((__nothrow__));


extern int sched_get_priority_max (int __algorithm) __attribute__ ((__nothrow__));


extern int sched_get_priority_min (int __algorithm) __attribute__ ((__nothrow__));


extern int sched_rr_get_interval (__pid_t __pid, struct timespec *__t) __attribute__ ((__nothrow__));
# 85 "/usr/include/sched.h" 3 4

# 25 "/usr/include/pthread.h" 2 3 4
# 1 "/usr/include/time.h" 1 3 4
# 31 "/usr/include/time.h" 3 4








# 1 "/usr/lib/gcc/i586-suse-linux/4.2.1/include/stddef.h" 1 3 4
# 40 "/usr/include/time.h" 2 3 4



# 1 "/usr/include/bits/time.h" 1 3 4
# 44 "/usr/include/time.h" 2 3 4
# 59 "/usr/include/time.h" 3 4


typedef __clock_t clock_t;



# 132 "/usr/include/time.h" 3 4


struct tm
{
  int tm_sec;
  int tm_min;
  int tm_hour;
  int tm_mday;
  int tm_mon;
  int tm_year;
  int tm_wday;
  int tm_yday;
  int tm_isdst;


  long int tm_gmtoff;
  __const char *tm_zone;




};








struct itimerspec
  {
    struct timespec it_interval;
    struct timespec it_value;
  };


struct sigevent;
# 181 "/usr/include/time.h" 3 4



extern clock_t clock (void) __attribute__ ((__nothrow__));


extern time_t time (time_t *__timer) __attribute__ ((__nothrow__));


extern double difftime (time_t __time1, time_t __time0)
     __attribute__ ((__nothrow__)) __attribute__ ((__const__));


extern time_t mktime (struct tm *__tp) __attribute__ ((__nothrow__));





extern size_t strftime (char *__restrict __s, size_t __maxsize,
   __const char *__restrict __format,
   __const struct tm *__restrict __tp) __attribute__ ((__nothrow__));

# 229 "/usr/include/time.h" 3 4



extern struct tm *gmtime (__const time_t *__timer) __attribute__ ((__nothrow__));



extern struct tm *localtime (__const time_t *__timer) __attribute__ ((__nothrow__));





extern struct tm *gmtime_r (__const time_t *__restrict __timer,
       struct tm *__restrict __tp) __attribute__ ((__nothrow__));



extern struct tm *localtime_r (__const time_t *__restrict __timer,
          struct tm *__restrict __tp) __attribute__ ((__nothrow__));





extern char *asctime (__const struct tm *__tp) __attribute__ ((__nothrow__));


extern char *ctime (__const time_t *__timer) __attribute__ ((__nothrow__));







extern char *asctime_r (__const struct tm *__restrict __tp,
   char *__restrict __buf) __attribute__ ((__nothrow__));


extern char *ctime_r (__const time_t *__restrict __timer,
        char *__restrict __buf) __attribute__ ((__nothrow__));




extern char *__tzname[2];
extern int __daylight;
extern long int __timezone;




extern char *tzname[2];



extern void tzset (void) __attribute__ ((__nothrow__));



extern int daylight;
extern long int timezone;





extern int stime (__const time_t *__when) __attribute__ ((__nothrow__));
# 312 "/usr/include/time.h" 3 4
extern time_t timegm (struct tm *__tp) __attribute__ ((__nothrow__));


extern time_t timelocal (struct tm *__tp) __attribute__ ((__nothrow__));


extern int dysize (int __year) __attribute__ ((__nothrow__)) __attribute__ ((__const__));
# 327 "/usr/include/time.h" 3 4
extern int nanosleep (__const struct timespec *__requested_time,
        struct timespec *__remaining);



extern int clock_getres (clockid_t __clock_id, struct timespec *__res) __attribute__ ((__nothrow__));


extern int clock_gettime (clockid_t __clock_id, struct timespec *__tp) __attribute__ ((__nothrow__));


extern int clock_settime (clockid_t __clock_id, __const struct timespec *__tp)
     __attribute__ ((__nothrow__));






extern int clock_nanosleep (clockid_t __clock_id, int __flags,
       __const struct timespec *__req,
       struct timespec *__rem);


extern int clock_getcpuclockid (pid_t __pid, clockid_t *__clock_id) __attribute__ ((__nothrow__));




extern int timer_create (clockid_t __clock_id,
    struct sigevent *__restrict __evp,
    timer_t *__restrict __timerid) __attribute__ ((__nothrow__));


extern int timer_delete (timer_t __timerid) __attribute__ ((__nothrow__));


extern int timer_settime (timer_t __timerid, int __flags,
     __const struct itimerspec *__restrict __value,
     struct itimerspec *__restrict __ovalue) __attribute__ ((__nothrow__));


extern int timer_gettime (timer_t __timerid, struct itimerspec *__value)
     __attribute__ ((__nothrow__));


extern int timer_getoverrun (timer_t __timerid) __attribute__ ((__nothrow__));
# 416 "/usr/include/time.h" 3 4

# 26 "/usr/include/pthread.h" 2 3 4


# 1 "/usr/include/signal.h" 1 3 4
# 31 "/usr/include/signal.h" 3 4


# 1 "/usr/include/bits/sigset.h" 1 3 4
# 34 "/usr/include/signal.h" 2 3 4
# 400 "/usr/include/signal.h" 3 4

# 29 "/usr/include/pthread.h" 2 3 4

# 1 "/usr/include/bits/setjmp.h" 1 3 4
# 29 "/usr/include/bits/setjmp.h" 3 4
typedef int __jmp_buf[6];
# 31 "/usr/include/pthread.h" 2 3 4
# 1 "/usr/include/bits/wordsize.h" 1 3 4
# 32 "/usr/include/pthread.h" 2 3 4



enum
{
  PTHREAD_CREATE_JOINABLE,

  PTHREAD_CREATE_DETACHED

};



enum
{
  PTHREAD_MUTEX_TIMED_NP,
  PTHREAD_MUTEX_RECURSIVE_NP,
  PTHREAD_MUTEX_ERRORCHECK_NP,
  PTHREAD_MUTEX_ADAPTIVE_NP
# 62 "/usr/include/pthread.h" 3 4
};
# 114 "/usr/include/pthread.h" 3 4
enum
{
  PTHREAD_RWLOCK_PREFER_READER_NP,
  PTHREAD_RWLOCK_PREFER_WRITER_NP,
  PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP,
  PTHREAD_RWLOCK_DEFAULT_NP = PTHREAD_RWLOCK_PREFER_READER_NP
};
# 144 "/usr/include/pthread.h" 3 4
enum
{
  PTHREAD_INHERIT_SCHED,

  PTHREAD_EXPLICIT_SCHED

};



enum
{
  PTHREAD_SCOPE_SYSTEM,

  PTHREAD_SCOPE_PROCESS

};



enum
{
  PTHREAD_PROCESS_PRIVATE,

  PTHREAD_PROCESS_SHARED

};
# 179 "/usr/include/pthread.h" 3 4
struct _pthread_cleanup_buffer
{
  void (*__routine) (void *);
  void *__arg;
  int __canceltype;
  struct _pthread_cleanup_buffer *__prev;
};


enum
{
  PTHREAD_CANCEL_ENABLE,

  PTHREAD_CANCEL_DISABLE

};
enum
{
  PTHREAD_CANCEL_DEFERRED,

  PTHREAD_CANCEL_ASYNCHRONOUS

};
# 217 "/usr/include/pthread.h" 3 4





extern int pthread_create (pthread_t *__restrict __newthread,
      __const pthread_attr_t *__restrict __attr,
      void *(*__start_routine) (void *),
      void *__restrict __arg) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 3)));





extern void pthread_exit (void *__retval) __attribute__ ((__noreturn__));







extern int pthread_join (pthread_t __th, void **__thread_return);
# 260 "/usr/include/pthread.h" 3 4
extern int pthread_detach (pthread_t __th) __attribute__ ((__nothrow__));



extern pthread_t pthread_self (void) __attribute__ ((__nothrow__)) __attribute__ ((__const__));


extern int pthread_equal (pthread_t __thread1, pthread_t __thread2) __attribute__ ((__nothrow__));







extern int pthread_attr_init (pthread_attr_t *__attr) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern int pthread_attr_destroy (pthread_attr_t *__attr)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern int pthread_attr_getdetachstate (__const pthread_attr_t *__attr,
     int *__detachstate)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_attr_setdetachstate (pthread_attr_t *__attr,
     int __detachstate)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));



extern int pthread_attr_getguardsize (__const pthread_attr_t *__attr,
          size_t *__guardsize)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_attr_setguardsize (pthread_attr_t *__attr,
          size_t __guardsize)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));



extern int pthread_attr_getschedparam (__const pthread_attr_t *__restrict
           __attr,
           struct sched_param *__restrict __param)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_attr_setschedparam (pthread_attr_t *__restrict __attr,
           __const struct sched_param *__restrict
           __param) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_attr_getschedpolicy (__const pthread_attr_t *__restrict
     __attr, int *__restrict __policy)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_attr_setschedpolicy (pthread_attr_t *__attr, int __policy)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern int pthread_attr_getinheritsched (__const pthread_attr_t *__restrict
      __attr, int *__restrict __inherit)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_attr_setinheritsched (pthread_attr_t *__attr,
      int __inherit)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));



extern int pthread_attr_getscope (__const pthread_attr_t *__restrict __attr,
      int *__restrict __scope)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_attr_setscope (pthread_attr_t *__attr, int __scope)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern int pthread_attr_getstackaddr (__const pthread_attr_t *__restrict
          __attr, void **__restrict __stackaddr)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2))) __attribute__ ((__deprecated__));





extern int pthread_attr_setstackaddr (pthread_attr_t *__attr,
          void *__stackaddr)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1))) __attribute__ ((__deprecated__));


extern int pthread_attr_getstacksize (__const pthread_attr_t *__restrict
          __attr, size_t *__restrict __stacksize)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));




extern int pthread_attr_setstacksize (pthread_attr_t *__attr,
          size_t __stacksize)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));



extern int pthread_attr_getstack (__const pthread_attr_t *__restrict __attr,
      void **__restrict __stackaddr,
      size_t *__restrict __stacksize)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2, 3)));




extern int pthread_attr_setstack (pthread_attr_t *__attr, void *__stackaddr,
      size_t __stacksize) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));
# 410 "/usr/include/pthread.h" 3 4
extern int pthread_setschedparam (pthread_t __target_thread, int __policy,
      __const struct sched_param *__param)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3)));


extern int pthread_getschedparam (pthread_t __target_thread,
      int *__restrict __policy,
      struct sched_param *__restrict __param)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2, 3)));


extern int pthread_setschedprio (pthread_t __target_thread, int __prio)
     __attribute__ ((__nothrow__));
# 463 "/usr/include/pthread.h" 3 4
extern int pthread_once (pthread_once_t *__once_control,
    void (*__init_routine) (void)) __attribute__ ((__nonnull__ (1, 2)));
# 475 "/usr/include/pthread.h" 3 4
extern int pthread_setcancelstate (int __state, int *__oldstate);



extern int pthread_setcanceltype (int __type, int *__oldtype);


extern int pthread_cancel (pthread_t __th);




extern void pthread_testcancel (void);




typedef struct
{
  struct
  {
    __jmp_buf __cancel_jmp_buf;
    int __mask_was_saved;
  } __cancel_jmp_buf[1];
  void *__pad[4];
} __pthread_unwind_buf_t __attribute__ ((__aligned__));
# 509 "/usr/include/pthread.h" 3 4
struct __pthread_cleanup_frame
{
  void (*__cancel_routine) (void *);
  void *__cancel_arg;
  int __do_it;
  int __cancel_type;
};
# 649 "/usr/include/pthread.h" 3 4
extern void __pthread_register_cancel (__pthread_unwind_buf_t *__buf)
     __attribute__ ((__regparm__ (1)));
# 660 "/usr/include/pthread.h" 3 4
extern void __pthread_unregister_cancel (__pthread_unwind_buf_t *__buf)
  __attribute__ ((__regparm__ (1)));
# 700 "/usr/include/pthread.h" 3 4
extern void __pthread_unwind_next (__pthread_unwind_buf_t *__buf)
     __attribute__ ((__regparm__ (1))) __attribute__ ((__noreturn__))

     __attribute__ ((__weak__))

     ;



struct __jmp_buf_tag;
extern int __sigsetjmp (struct __jmp_buf_tag *__env, int __savemask) __attribute__ ((__nothrow__));





extern int pthread_mutex_init (pthread_mutex_t *__mutex,
          __const pthread_mutexattr_t *__mutexattr)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern int pthread_mutex_destroy (pthread_mutex_t *__mutex)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern int pthread_mutex_trylock (pthread_mutex_t *__mutex)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern int pthread_mutex_lock (pthread_mutex_t *__mutex)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));



extern int pthread_mutex_timedlock (pthread_mutex_t *__restrict __mutex,
                                    __const struct timespec *__restrict
                                    __abstime) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));



extern int pthread_mutex_unlock (pthread_mutex_t *__mutex)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));
# 771 "/usr/include/pthread.h" 3 4
extern int pthread_mutexattr_init (pthread_mutexattr_t *__attr)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern int pthread_mutexattr_destroy (pthread_mutexattr_t *__attr)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern int pthread_mutexattr_getpshared (__const pthread_mutexattr_t *
      __restrict __attr,
      int *__restrict __pshared)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_mutexattr_setpshared (pthread_mutexattr_t *__attr,
      int __pshared)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));
# 843 "/usr/include/pthread.h" 3 4
extern int pthread_rwlock_init (pthread_rwlock_t *__restrict __rwlock,
    __const pthread_rwlockattr_t *__restrict
    __attr) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern int pthread_rwlock_destroy (pthread_rwlock_t *__rwlock)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern int pthread_rwlock_rdlock (pthread_rwlock_t *__rwlock)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern int pthread_rwlock_tryrdlock (pthread_rwlock_t *__rwlock)
  __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));



extern int pthread_rwlock_timedrdlock (pthread_rwlock_t *__restrict __rwlock,
           __const struct timespec *__restrict
           __abstime) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));



extern int pthread_rwlock_wrlock (pthread_rwlock_t *__rwlock)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern int pthread_rwlock_trywrlock (pthread_rwlock_t *__rwlock)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));



extern int pthread_rwlock_timedwrlock (pthread_rwlock_t *__restrict __rwlock,
           __const struct timespec *__restrict
           __abstime) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));



extern int pthread_rwlock_unlock (pthread_rwlock_t *__rwlock)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));





extern int pthread_rwlockattr_init (pthread_rwlockattr_t *__attr)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern int pthread_rwlockattr_destroy (pthread_rwlockattr_t *__attr)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern int pthread_rwlockattr_getpshared (__const pthread_rwlockattr_t *
       __restrict __attr,
       int *__restrict __pshared)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_rwlockattr_setpshared (pthread_rwlockattr_t *__attr,
       int __pshared)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern int pthread_rwlockattr_getkind_np (__const pthread_rwlockattr_t *
       __restrict __attr,
       int *__restrict __pref)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_rwlockattr_setkind_np (pthread_rwlockattr_t *__attr,
       int __pref) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));







extern int pthread_cond_init (pthread_cond_t *__restrict __cond,
         __const pthread_condattr_t *__restrict
         __cond_attr) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern int pthread_cond_destroy (pthread_cond_t *__cond)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern int pthread_cond_signal (pthread_cond_t *__cond)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern int pthread_cond_broadcast (pthread_cond_t *__cond)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));






extern int pthread_cond_wait (pthread_cond_t *__restrict __cond,
         pthread_mutex_t *__restrict __mutex)
     __attribute__ ((__nonnull__ (1, 2)));
# 955 "/usr/include/pthread.h" 3 4
extern int pthread_cond_timedwait (pthread_cond_t *__restrict __cond,
       pthread_mutex_t *__restrict __mutex,
       __const struct timespec *__restrict
       __abstime) __attribute__ ((__nonnull__ (1, 2, 3)));




extern int pthread_condattr_init (pthread_condattr_t *__attr)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern int pthread_condattr_destroy (pthread_condattr_t *__attr)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern int pthread_condattr_getpshared (__const pthread_condattr_t *
                                        __restrict __attr,
                                        int *__restrict __pshared)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_condattr_setpshared (pthread_condattr_t *__attr,
                                        int __pshared) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));



extern int pthread_condattr_getclock (__const pthread_condattr_t *
          __restrict __attr,
          __clockid_t *__restrict __clock_id)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_condattr_setclock (pthread_condattr_t *__attr,
          __clockid_t __clock_id)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));
# 999 "/usr/include/pthread.h" 3 4
extern int pthread_spin_init (pthread_spinlock_t *__lock, int __pshared)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern int pthread_spin_destroy (pthread_spinlock_t *__lock)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern int pthread_spin_lock (pthread_spinlock_t *__lock)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern int pthread_spin_trylock (pthread_spinlock_t *__lock)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern int pthread_spin_unlock (pthread_spinlock_t *__lock)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));






extern int pthread_barrier_init (pthread_barrier_t *__restrict __barrier,
     __const pthread_barrierattr_t *__restrict
     __attr, unsigned int __count)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern int pthread_barrier_destroy (pthread_barrier_t *__barrier)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern int pthread_barrier_wait (pthread_barrier_t *__barrier)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));



extern int pthread_barrierattr_init (pthread_barrierattr_t *__attr)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern int pthread_barrierattr_destroy (pthread_barrierattr_t *__attr)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern int pthread_barrierattr_getpshared (__const pthread_barrierattr_t *
        __restrict __attr,
        int *__restrict __pshared)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_barrierattr_setpshared (pthread_barrierattr_t *__attr,
                                           int __pshared)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));
# 1066 "/usr/include/pthread.h" 3 4
extern int pthread_key_create (pthread_key_t *__key,
          void (*__destr_function) (void *))
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern int pthread_key_delete (pthread_key_t __key) __attribute__ ((__nothrow__));


extern void *pthread_getspecific (pthread_key_t __key) __attribute__ ((__nothrow__));


extern int pthread_setspecific (pthread_key_t __key,
    __const void *__pointer) __attribute__ ((__nothrow__)) ;




extern int pthread_getcpuclockid (pthread_t __thread_id,
      __clockid_t *__clock_id)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2)));
# 1100 "/usr/include/pthread.h" 3 4
extern int pthread_atfork (void (*__prepare) (void),
      void (*__parent) (void),
      void (*__child) (void)) __attribute__ ((__nothrow__));
# 1114 "/usr/include/pthread.h" 3 4

# 156 "cairo-mutex-type-private.h" 2

  typedef pthread_mutex_t cairo_mutex_t;
# 208 "cairo-mutex-type-private.h"

# 45 "cairo-scaled-font-private.h" 2


struct _cairo_scaled_font {
# 79 "cairo-scaled-font-private.h"
    cairo_hash_entry_t hash_entry;


    cairo_status_t status;
    cairo_reference_count_t ref_count;
    cairo_user_data_array_t user_data;


    cairo_font_face_t *font_face;
    cairo_matrix_t font_matrix;
    cairo_matrix_t ctm;
    cairo_font_options_t options;


    cairo_matrix_t scale;
    cairo_font_extents_t extents;


    cairo_mutex_t mutex;

    cairo_cache_t *glyphs;






    const cairo_surface_backend_t *surface_backend;
    void *surface_private;


    const cairo_scaled_font_backend_t *backend;
};
# 272 "cairoint.h" 2

struct _cairo_font_face {

    cairo_hash_entry_t hash_entry;
    cairo_status_t status;
    cairo_reference_count_t ref_count;
    cairo_user_data_array_t user_data;
    const cairo_font_face_backend_t *backend;
};

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_font_reset_static_data (void);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_ft_font_reset_static_data (void);



struct _cairo_unscaled_font_backend {
    void (*destroy) (void *unscaled_font);
};





typedef struct _cairo_toy_font_face {
    cairo_font_face_t base;
    const char *family;
    cairo_bool_t owns_family;
    cairo_font_slant_t slant;
    cairo_font_weight_t weight;
} cairo_toy_font_face_t;

typedef enum _cairo_scaled_glyph_info {
    CAIRO_SCALED_GLYPH_INFO_METRICS = (1 << 0),
    CAIRO_SCALED_GLYPH_INFO_SURFACE = (1 << 1),
    CAIRO_SCALED_GLYPH_INFO_PATH = (1 << 2)
} cairo_scaled_glyph_info_t;

typedef struct _cairo_scaled_font_subset {
    cairo_scaled_font_t *scaled_font;
    unsigned int font_id;
    unsigned int subset_id;




    unsigned long *glyphs;
    unsigned long *to_unicode;
    char **glyph_names;
    unsigned int num_glyphs;
    cairo_bool_t is_composite;
} cairo_scaled_font_subset_t;

struct _cairo_scaled_font_backend {
    cairo_font_type_t type;

    __attribute__((__warn_unused_result__)) cairo_status_t
    (*create_toy) (cairo_toy_font_face_t *toy_face,
      const cairo_matrix_t *font_matrix,
      const cairo_matrix_t *ctm,
      const cairo_font_options_t *options,
      cairo_scaled_font_t **scaled_font);

    void
    (*fini) (void *scaled_font);

    __attribute__((__warn_unused_result__)) cairo_int_status_t
    (*scaled_glyph_init) (void *scaled_font,
     cairo_scaled_glyph_t *scaled_glyph,
     cairo_scaled_glyph_info_t info);





    __attribute__((__warn_unused_result__)) cairo_int_status_t
    (*text_to_glyphs) (void *scaled_font,
         double x,
         double y,
         const char *utf8,
         cairo_glyph_t **glyphs,
         int *num_glyphs);

    unsigned long
    (*ucs4_to_index) (void *scaled_font,
     uint32_t ucs4);
    __attribute__((__warn_unused_result__)) cairo_int_status_t
    (*show_glyphs) (void *scaled_font,
    cairo_operator_t op,
    cairo_pattern_t *pattern,
    cairo_surface_t *surface,
    int source_x,
    int source_y,
    int dest_x,
    int dest_y,
    unsigned int width,
    unsigned int height,
    cairo_glyph_t *glyphs,
    int num_glyphs);

    __attribute__((__warn_unused_result__)) cairo_int_status_t
    (*load_truetype_table)(void *scaled_font,
                           unsigned long tag,
                           long offset,
                           unsigned char *buffer,
                           unsigned long *length);

    __attribute__((__warn_unused_result__)) cairo_int_status_t
    (*map_glyphs_to_unicode)(void *scaled_font,
                                   cairo_scaled_font_subset_t *font_subset);

};

struct _cairo_font_face_backend {
    cairo_font_type_t type;




    void
    (*destroy) (void *font_face);

    __attribute__((__warn_unused_result__)) cairo_status_t
    (*scaled_font_create) (void *font_face,
      const cairo_matrix_t *font_matrix,
      const cairo_matrix_t *ctm,
      const cairo_font_options_t *options,
      cairo_scaled_font_t **scaled_font);
};




extern const __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) struct _cairo_scaled_font_backend moonlight_cairo_ft_scaled_font_backend;
# 423 "cairoint.h"
struct _cairo_surface_backend {
    cairo_surface_type_t type;

    cairo_surface_t *
    (*create_similar) (void *surface,
     cairo_content_t content,
     int width,
     int height);

    __attribute__((__warn_unused_result__)) cairo_status_t
    (*finish) (void *surface);

    __attribute__((__warn_unused_result__)) cairo_status_t
    (*acquire_source_image) (void *abstract_surface,
     cairo_image_surface_t **image_out,
     void **image_extra);

    void
    (*release_source_image) (void *abstract_surface,
     cairo_image_surface_t *image,
     void *image_extra);

    __attribute__((__warn_unused_result__)) cairo_status_t
    (*acquire_dest_image) (void *abstract_surface,
     cairo_rectangle_int_t *interest_rect,
     cairo_image_surface_t **image_out,
     cairo_rectangle_int_t *image_rect,
     void **image_extra);

    void
    (*release_dest_image) (void *abstract_surface,
     cairo_rectangle_int_t *interest_rect,
     cairo_image_surface_t *image,
     cairo_rectangle_int_t *image_rect,
     void *image_extra);
# 469 "cairoint.h"
    __attribute__((__warn_unused_result__)) cairo_status_t
    (*clone_similar) (void *surface,
     cairo_surface_t *src,
     int src_x,
     int src_y,
     int width,
     int height,
     cairo_surface_t **clone_out);


    __attribute__((__warn_unused_result__)) cairo_int_status_t
    (*composite) (cairo_operator_t op,
     cairo_pattern_t *src,
     cairo_pattern_t *mask,
     void *dst,
     int src_x,
     int src_y,
     int mask_x,
     int mask_y,
     int dst_x,
     int dst_y,
     unsigned int width,
     unsigned int height);

    __attribute__((__warn_unused_result__)) cairo_int_status_t
    (*fill_rectangles) (void *surface,
     cairo_operator_t op,
     const cairo_color_t *color,
     cairo_rectangle_int_t *rects,
     int num_rects);


    __attribute__((__warn_unused_result__)) cairo_int_status_t
    (*composite_trapezoids) (cairo_operator_t op,
     cairo_pattern_t *pattern,
     void *dst,
     cairo_antialias_t antialias,
     int src_x,
     int src_y,
     int dst_x,
     int dst_y,
     unsigned int width,
     unsigned int height,
     cairo_trapezoid_t *traps,
     int num_traps);

    __attribute__((__warn_unused_result__)) cairo_int_status_t
    (*copy_page) (void *surface);

    __attribute__((__warn_unused_result__)) cairo_int_status_t
    (*show_page) (void *surface);
# 534 "cairoint.h"
    __attribute__((__warn_unused_result__)) cairo_int_status_t
    (*set_clip_region) (void *surface,
     cairo_region_t *region);
# 552 "cairoint.h"
    __attribute__((__warn_unused_result__)) cairo_int_status_t
    (*intersect_clip_path) (void *dst,
     cairo_path_fixed_t *path,
     cairo_fill_rule_t fill_rule,
     double tolerance,
     cairo_antialias_t antialias);
# 569 "cairoint.h"
    __attribute__((__warn_unused_result__)) cairo_int_status_t
    (*get_extents) (void *surface,
     cairo_rectangle_int_t *rectangle);






    __attribute__((__warn_unused_result__)) cairo_int_status_t
    (*old_show_glyphs) (cairo_scaled_font_t *font,
     cairo_operator_t op,
     cairo_pattern_t *pattern,
     void *surface,
     int source_x,
     int source_y,
     int dest_x,
     int dest_y,
     unsigned int width,
     unsigned int height,
     cairo_glyph_t *glyphs,
     int num_glyphs);

    void
    (*get_font_options) (void *surface,
     cairo_font_options_t *options);

    __attribute__((__warn_unused_result__)) cairo_status_t
    (*flush) (void *surface);

    __attribute__((__warn_unused_result__)) cairo_status_t
    (*mark_dirty_rectangle) (void *surface,
     int x,
     int y,
     int width,
     int height);

    void
    (*scaled_font_fini) (cairo_scaled_font_t *scaled_font);

    void
    (*scaled_glyph_fini) (cairo_scaled_glyph_t *scaled_glyph,
     cairo_scaled_font_t *scaled_font);




    __attribute__((__warn_unused_result__)) cairo_int_status_t
    (*paint) (void *surface,
     cairo_operator_t op,
     cairo_pattern_t *source);

    __attribute__((__warn_unused_result__)) cairo_int_status_t
    (*mask) (void *surface,
     cairo_operator_t op,
     cairo_pattern_t *source,
     cairo_pattern_t *mask);

    __attribute__((__warn_unused_result__)) cairo_int_status_t
    (*stroke) (void *surface,
     cairo_operator_t op,
     cairo_pattern_t *source,
     cairo_path_fixed_t *path,
     cairo_stroke_style_t *style,
     cairo_matrix_t *ctm,
     cairo_matrix_t *ctm_inverse,
     double tolerance,
     cairo_antialias_t antialias);

    __attribute__((__warn_unused_result__)) cairo_int_status_t
    (*fill) (void *surface,
     cairo_operator_t op,
     cairo_pattern_t *source,
     cairo_path_fixed_t *path,
     cairo_fill_rule_t fill_rule,
     double tolerance,
     cairo_antialias_t antialias);

    __attribute__((__warn_unused_result__)) cairo_int_status_t
    (*show_glyphs) (void *surface,
     cairo_operator_t op,
     cairo_pattern_t *source,
     cairo_glyph_t *glyphs,
     int num_glyphs,
     cairo_scaled_font_t *scaled_font);

    cairo_surface_t *
    (*snapshot) (void *surface);

    cairo_bool_t
    (*is_similar) (void *surface_a,
                          void *surface_b,
     cairo_content_t content);

    __attribute__((__warn_unused_result__)) cairo_status_t
    (*reset) (void *surface);

    __attribute__((__warn_unused_result__)) cairo_int_status_t
    (*fill_stroke) (void *surface,
     cairo_operator_t fill_op,
     cairo_pattern_t *fill_source,
     cairo_fill_rule_t fill_rule,
     double fill_tolerance,
     cairo_antialias_t fill_antialias,
     cairo_path_fixed_t *path,
     cairo_operator_t stroke_op,
     cairo_pattern_t *stroke_source,
     cairo_stroke_style_t *stroke_style,
     cairo_matrix_t *stroke_ctm,
     cairo_matrix_t *stroke_ctm_inverse,
     double stroke_tolerance,
     cairo_antialias_t stroke_antialias);
};

# 1 "cairo-surface-private.h" 1
# 46 "cairo-surface-private.h"
struct _cairo_surface {
    const cairo_surface_backend_t *backend;




    cairo_surface_type_t type;

    cairo_content_t content;

    cairo_reference_count_t ref_count;
    cairo_status_t status;
    cairo_bool_t finished;
    cairo_user_data_array_t user_data;

    cairo_matrix_t device_transform;
    cairo_matrix_t device_transform_inverse;


    double x_resolution;
    double y_resolution;





    double x_fallback_resolution;
    double y_fallback_resolution;

    cairo_clip_t *clip;






    unsigned int next_clip_serial;
# 91 "cairo-surface-private.h"
    unsigned int current_clip_serial;


    cairo_bool_t is_snapshot;






    cairo_bool_t has_font_options;
    cairo_font_options_t font_options;
};
# 684 "cairoint.h" 2

struct _cairo_image_surface {
    cairo_surface_t base;

    pixman_format_code_t pixman_format;
    cairo_format_t format;
    unsigned char *data;
    cairo_bool_t owns_data;
    cairo_bool_t has_clip;

    int width;
    int height;
    int stride;
    int depth;

    pixman_image_t *pixman_image;
};

extern const __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_surface_backend_t moonlight_cairo_image_surface_backend;







struct _cairo_color {
    double red;
    double green;
    double blue;
    double alpha;

    unsigned short red_short;
    unsigned short green_short;
    unsigned short blue_short;
    unsigned short alpha_short;
};





struct _cairo_pattern {
    cairo_pattern_type_t type;
    cairo_reference_count_t ref_count;
    cairo_status_t status;
    cairo_user_data_array_t user_data;

    cairo_matrix_t matrix;
    cairo_filter_t filter;
    cairo_extend_t extend;
};

typedef struct _cairo_solid_pattern {
    cairo_pattern_t base;
    cairo_color_t color;
    cairo_content_t content;
} cairo_solid_pattern_t;

extern const __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_solid_pattern_t _moonlight_cairo_pattern_nil;
extern const __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_solid_pattern_t moonlight_cairo_pattern_none;

typedef struct _cairo_surface_pattern {
    cairo_pattern_t base;

    cairo_surface_t *surface;
} cairo_surface_pattern_t;

typedef struct _cairo_gradient_stop {
    cairo_fixed_t x;
    cairo_color_t color;
} cairo_gradient_stop_t;

typedef struct _cairo_gradient_pattern {
    cairo_pattern_t base;

    unsigned int n_stops;
    unsigned int stops_size;
    cairo_gradient_stop_t *stops;
    cairo_gradient_stop_t stops_embedded[2];
} cairo_gradient_pattern_t;

typedef struct _cairo_linear_pattern {
    cairo_gradient_pattern_t base;

    cairo_point_t p1;
    cairo_point_t p2;
} cairo_linear_pattern_t;

typedef struct _cairo_radial_pattern {
    cairo_gradient_pattern_t base;

    cairo_point_t c1;
    cairo_fixed_t r1;
    cairo_point_t c2;
    cairo_fixed_t r2;
} cairo_radial_pattern_t;

typedef union {
    cairo_gradient_pattern_t base;

    cairo_linear_pattern_t linear;
    cairo_radial_pattern_t radial;
} cairo_gradient_pattern_union_t;

typedef union {
    cairo_pattern_t base;

    cairo_solid_pattern_t solid;
    cairo_surface_pattern_t surface;
    cairo_gradient_pattern_union_t gradient;
} cairo_pattern_union_t;

typedef struct _cairo_surface_attributes {
    cairo_matrix_t matrix;
    cairo_extend_t extend;
    cairo_filter_t filter;
    int x_offset;
    int y_offset;
    cairo_bool_t acquired;
    void *extra;
} cairo_surface_attributes_t;

typedef struct _cairo_traps {
    cairo_status_t status;

    cairo_box_t extents;

    int num_traps;
    int traps_size;
    cairo_trapezoid_t *traps;
    cairo_trapezoid_t traps_embedded[1];

    cairo_bool_t has_limits;
    cairo_box_t limits;
} cairo_traps_t;
# 862 "cairoint.h"
typedef struct _cairo_gstate cairo_gstate_t;

typedef struct _cairo_stroke_face {
    cairo_point_t ccw;
    cairo_point_t point;
    cairo_point_t cw;
    cairo_slope_t dev_vector;
    cairo_point_double_t usr_vector;
} cairo_stroke_face_t;


__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_restrict_value (double *value, double min, double max);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) int
_moonlight_cairo_lround (double d);


__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_gstate_init (cairo_gstate_t *gstate,
      cairo_surface_t *target);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_gstate_fini (cairo_gstate_t *gstate);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_gstate_save (cairo_gstate_t **gstate);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_gstate_restore (cairo_gstate_t **gstate);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_bool_t
_moonlight_cairo_gstate_is_redirected (cairo_gstate_t *gstate);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_gstate_redirect_target (cairo_gstate_t *gstate, cairo_surface_t *child);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_surface_t *
_moonlight_cairo_gstate_get_target (cairo_gstate_t *gstate);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_surface_t *
_moonlight_cairo_gstate_get_parent_target (cairo_gstate_t *gstate);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_surface_t *
_moonlight_cairo_gstate_get_original_target (cairo_gstate_t *gstate);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_clip_t *
_moonlight_cairo_gstate_get_clip (cairo_gstate_t *gstate);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_gstate_set_source (cairo_gstate_t *gstate, cairo_pattern_t *source);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_pattern_t *
_moonlight_cairo_gstate_get_source (cairo_gstate_t *gstate);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_gstate_set_operator (cairo_gstate_t *gstate, cairo_operator_t op);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_operator_t
_moonlight_cairo_gstate_get_operator (cairo_gstate_t *gstate);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_gstate_set_tolerance (cairo_gstate_t *gstate, double tolerance);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) double
_moonlight_cairo_gstate_get_tolerance (cairo_gstate_t *gstate);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_gstate_set_fill_rule (cairo_gstate_t *gstate, cairo_fill_rule_t fill_rule);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_fill_rule_t
_moonlight_cairo_gstate_get_fill_rule (cairo_gstate_t *gstate);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_gstate_set_line_width (cairo_gstate_t *gstate, double width);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) double
_moonlight_cairo_gstate_get_line_width (cairo_gstate_t *gstate);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_gstate_set_line_cap (cairo_gstate_t *gstate, cairo_line_cap_t line_cap);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_line_cap_t
_moonlight_cairo_gstate_get_line_cap (cairo_gstate_t *gstate);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_gstate_set_line_join (cairo_gstate_t *gstate, cairo_line_join_t line_join);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_line_join_t
_moonlight_cairo_gstate_get_line_join (cairo_gstate_t *gstate);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_gstate_set_dash (cairo_gstate_t *gstate, const double *dash, int num_dashes, double offset);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_gstate_get_dash (cairo_gstate_t *gstate, double *dash, int *num_dashes, double *offset);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_gstate_set_miter_limit (cairo_gstate_t *gstate, double limit);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) double
_moonlight_cairo_gstate_get_miter_limit (cairo_gstate_t *gstate);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_gstate_get_matrix (cairo_gstate_t *gstate, cairo_matrix_t *matrix);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_gstate_translate (cairo_gstate_t *gstate, double tx, double ty);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_gstate_scale (cairo_gstate_t *gstate, double sx, double sy);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_gstate_rotate (cairo_gstate_t *gstate, double angle);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_gstate_transform (cairo_gstate_t *gstate,
    const cairo_matrix_t *matrix);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_gstate_set_matrix (cairo_gstate_t *gstate,
     const cairo_matrix_t *matrix);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_gstate_identity_matrix (cairo_gstate_t *gstate);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_gstate_user_to_device (cairo_gstate_t *gstate, double *x, double *y);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_gstate_user_to_device_distance (cairo_gstate_t *gstate, double *dx, double *dy);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_gstate_device_to_user (cairo_gstate_t *gstate, double *x, double *y);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_gstate_device_to_user_distance (cairo_gstate_t *gstate, double *dx, double *dy);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_gstate_user_to_backend (cairo_gstate_t *gstate, double *x, double *y);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_gstate_backend_to_user (cairo_gstate_t *gstate, double *x, double *y);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_gstate_backend_to_user_rectangle (cairo_gstate_t *gstate,
                                         double *x1, double *y1,
                                         double *x2, double *y2,
                                         cairo_bool_t *is_tight);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_cairo_gstate_path_extents (cairo_gstate_t *gstate,
       cairo_path_fixed_t *path,
       double *x1, double *y1,
       double *x2, double *y2);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_gstate_paint (cairo_gstate_t *gstate);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_gstate_mask (cairo_gstate_t *gstate,
      cairo_pattern_t *mask);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_gstate_stroke (cairo_gstate_t *gstate, cairo_path_fixed_t *path);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_gstate_fill (cairo_gstate_t *gstate, cairo_path_fixed_t *path);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_gstate_copy_page (cairo_gstate_t *gstate);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_gstate_show_page (cairo_gstate_t *gstate);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_gstate_stroke_extents (cairo_gstate_t *gstate,
         cairo_path_fixed_t *path,
                              double *x1, double *y1,
         double *x2, double *y2);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_gstate_fill_extents (cairo_gstate_t *gstate,
       cairo_path_fixed_t *path,
                            double *x1, double *y1,
       double *x2, double *y2);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_gstate_in_stroke (cairo_gstate_t *gstate,
    cairo_path_fixed_t *path,
    double x,
    double y,
    cairo_bool_t *inside_ret);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_gstate_in_fill (cairo_gstate_t *gstate,
         cairo_path_fixed_t *path,
         double x,
         double y,
         cairo_bool_t *inside_ret);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_gstate_clip (cairo_gstate_t *gstate, cairo_path_fixed_t *path);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_gstate_reset_clip (cairo_gstate_t *gstate);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_gstate_clip_extents (cairo_gstate_t *gstate,
              double *x1,
              double *y1,
              double *x2,
              double *y2);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_rectangle_list_t*
_moonlight_cairo_gstate_copy_clip_rectangle_list (cairo_gstate_t *gstate);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_cairo_gstate_show_surface (cairo_gstate_t *gstate,
       cairo_surface_t *surface,
       double x,
       double y,
       double width,
       double height);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_gstate_select_font_face (cairo_gstate_t *gstate,
    const char *family,
    cairo_font_slant_t slant,
    cairo_font_weight_t weight);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_gstate_set_font_size (cairo_gstate_t *gstate,
        double size);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_gstate_get_font_matrix (cairo_gstate_t *gstate,
          cairo_matrix_t *matrix);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_gstate_set_font_matrix (cairo_gstate_t *gstate,
          const cairo_matrix_t *matrix);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_gstate_get_font_options (cairo_gstate_t *gstate,
    cairo_font_options_t *options);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_gstate_set_font_options (cairo_gstate_t *gstate,
    const cairo_font_options_t *options);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_gstate_get_font_face (cairo_gstate_t *gstate,
        cairo_font_face_t **font_face);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_gstate_get_scaled_font (cairo_gstate_t *gstate,
          cairo_scaled_font_t **scaled_font);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_gstate_get_font_extents (cairo_gstate_t *gstate,
    cairo_font_extents_t *extents);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_gstate_set_font_face (cairo_gstate_t *gstate,
        cairo_font_face_t *font_face);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_gstate_text_to_glyphs (cairo_gstate_t *font,
         const char *utf8,
         double x,
         double y,
         cairo_glyph_t **glyphs,
         int *num_glyphs);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_gstate_glyph_extents (cairo_gstate_t *gstate,
        const cairo_glyph_t *glyphs,
        int num_glyphs,
        cairo_text_extents_t *extents);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_gstate_show_glyphs (cairo_gstate_t *gstate,
      const cairo_glyph_t *glyphs,
      int num_glyphs);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_gstate_glyph_path (cairo_gstate_t *gstate,
     const cairo_glyph_t *glyphs,
     int num_glyphs,
     cairo_path_fixed_t *path);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_bool_t
_moonlight_cairo_operator_bounded_by_mask (cairo_operator_t op);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_bool_t
_moonlight_cairo_operator_bounded_by_source (cairo_operator_t op);


__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) const cairo_color_t *
_moonlight_cairo_stock_color (cairo_stock_t stock);





__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) uint16_t
_moonlight_cairo_color_double_to_short (double d);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_color_init (cairo_color_t *color);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_color_init_rgb (cairo_color_t *color,
         double red, double green, double blue);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_color_init_rgba (cairo_color_t *color,
   double red, double green, double blue,
   double alpha);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_color_multiply_alpha (cairo_color_t *color,
        double alpha);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_color_get_rgba (cairo_color_t *color,
         double *red,
         double *green,
         double *blue,
         double *alpha);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_color_get_rgba_premultiplied (cairo_color_t *color,
         double *red,
         double *green,
         double *blue,
         double *alpha);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_bool_t
_moonlight_cairo_color_equal (const cairo_color_t *color_a,
                    const cairo_color_t *color_b);



__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_scaled_font_freeze_cache (cairo_scaled_font_t *scaled_font);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_scaled_font_thaw_cache (cairo_scaled_font_t *scaled_font);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_scaled_font_reset_cache (cairo_scaled_font_t *scaled_font);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_scaled_font_set_error (cairo_scaled_font_t *scaled_font,
         cairo_status_t status);

extern const __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_font_face_t _moonlight_cairo_font_face_nil;
extern const __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_scaled_font_t _moonlight_cairo_scaled_font_nil;

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_font_face_init (cairo_font_face_t *font_face,
         const cairo_font_face_backend_t *backend);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_cairo_font_face_set_error (cairo_font_face_t *font_face,
                     cairo_status_t status);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_font_face_t *
_moonlight_cairo_toy_font_face_create (const char *family,
        cairo_font_slant_t slant,
        cairo_font_weight_t weight);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_unscaled_font_init (cairo_unscaled_font_t *font,
      const cairo_unscaled_font_backend_t *backend);

__attribute__((__visibility__("hidden"))) cairo_unscaled_font_t *
_moonlight_cairo_unscaled_font_reference (cairo_unscaled_font_t *font);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_unscaled_font_destroy (cairo_unscaled_font_t *font);



__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_font_options_init_default (cairo_font_options_t *options);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_font_options_init_copy (cairo_font_options_t *options,
          const cairo_font_options_t *other);


__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_hull_compute (cairo_pen_vertex_t *vertices, int *num_vertices);


__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) unsigned char *
_moonlight_cairo_lzw_compress (unsigned char *data, unsigned long *size_in_out);


__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_path_fixed_init (cairo_path_fixed_t *path);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_path_fixed_init_copy (cairo_path_fixed_t *path,
        cairo_path_fixed_t *other);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_bool_t
_cairo_path_fixed_is_equal (cairo_path_fixed_t *path,
       cairo_path_fixed_t *other);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_path_fixed_t *
_moonlight_cairo_path_fixed_create (void);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_path_fixed_fini (cairo_path_fixed_t *path);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_path_fixed_destroy (cairo_path_fixed_t *path);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_path_fixed_move_to (cairo_path_fixed_t *path,
      cairo_fixed_t x,
      cairo_fixed_t y);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_path_fixed_new_sub_path (cairo_path_fixed_t *path);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_path_fixed_rel_move_to (cairo_path_fixed_t *path,
          cairo_fixed_t dx,
          cairo_fixed_t dy);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_path_fixed_line_to (cairo_path_fixed_t *path,
      cairo_fixed_t x,
      cairo_fixed_t y);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_path_fixed_rel_line_to (cairo_path_fixed_t *path,
          cairo_fixed_t dx,
          cairo_fixed_t dy);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_path_fixed_curve_to (cairo_path_fixed_t *path,
       cairo_fixed_t x0, cairo_fixed_t y0,
       cairo_fixed_t x1, cairo_fixed_t y1,
       cairo_fixed_t x2, cairo_fixed_t y2);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_path_fixed_rel_curve_to (cairo_path_fixed_t *path,
    cairo_fixed_t dx0, cairo_fixed_t dy0,
    cairo_fixed_t dx1, cairo_fixed_t dy1,
    cairo_fixed_t dx2, cairo_fixed_t dy2);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_path_fixed_close_path (cairo_path_fixed_t *path);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_bool_t
_moonlight_cairo_path_fixed_get_current_point (cairo_path_fixed_t *path,
         cairo_fixed_t *x,
         cairo_fixed_t *y);

typedef cairo_status_t
(cairo_path_fixed_move_to_func_t) (void *closure,
       cairo_point_t *point);

typedef cairo_status_t
(cairo_path_fixed_line_to_func_t) (void *closure,
       cairo_point_t *point);

typedef cairo_status_t
(cairo_path_fixed_curve_to_func_t) (void *closure,
        cairo_point_t *p0,
        cairo_point_t *p1,
        cairo_point_t *p2);

typedef cairo_status_t
(cairo_path_fixed_close_path_func_t) (void *closure);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_path_fixed_interpret (cairo_path_fixed_t *path,
         cairo_direction_t dir,
         cairo_path_fixed_move_to_func_t *move_to,
         cairo_path_fixed_line_to_func_t *line_to,
         cairo_path_fixed_curve_to_func_t *curve_to,
         cairo_path_fixed_close_path_func_t *close_path,
         void *closure);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_cairo_path_fixed_interpret_flat (cairo_path_fixed_t *path,
         cairo_direction_t dir,
         cairo_path_fixed_move_to_func_t *move_to,
         cairo_path_fixed_line_to_func_t *line_to,
         cairo_path_fixed_close_path_func_t *close_path,
         void *closure,
         double tolerance);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_path_fixed_bounds (cairo_path_fixed_t *path,
     double *x1, double *y1,
     double *x2, double *y2,
     double tolerance);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_path_fixed_device_transform (cairo_path_fixed_t *path,
        cairo_matrix_t *device_transform);


__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_path_fixed_fill_to_traps (cairo_path_fixed_t *path,
     cairo_fill_rule_t fill_rule,
     double tolerance,
     cairo_traps_t *traps);


__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_path_fixed_stroke_to_traps (cairo_path_fixed_t *path,
       cairo_stroke_style_t *stroke_style,
       cairo_matrix_t *ctm,
       cairo_matrix_t *ctm_inverse,
       double tolerance,
       cairo_traps_t *traps);



__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_scaled_font_init (cairo_scaled_font_t *scaled_font,
    cairo_font_face_t *font_face,
    const cairo_matrix_t *font_matrix,
    const cairo_matrix_t *ctm,
    const cairo_font_options_t *options,
    const cairo_scaled_font_backend_t *backend);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_scaled_font_set_metrics (cairo_scaled_font_t *scaled_font,
    cairo_font_extents_t *fs_metrics);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_scaled_font_fini (cairo_scaled_font_t *scaled_font);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_cairo_scaled_font_font_extents (cairo_scaled_font_t *scaled_font,
     cairo_font_extents_t *extents);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_scaled_font_text_to_glyphs (cairo_scaled_font_t *scaled_font,
       double x,
       double y,
       const char *utf8,
       cairo_glyph_t **glyphs,
       int *num_glyphs);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_scaled_font_glyph_device_extents (cairo_scaled_font_t *scaled_font,
      const cairo_glyph_t *glyphs,
      int num_glyphs,
      cairo_rectangle_int_t *extents);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_scaled_font_show_glyphs (cairo_scaled_font_t *scaled_font,
    cairo_operator_t op,
    cairo_pattern_t *source,
    cairo_surface_t *surface,
    int source_x,
    int source_y,
    int dest_x,
    int dest_y,
    unsigned int width,
    unsigned int height,
    cairo_glyph_t *glyphs,
    int num_glyphs);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_scaled_font_glyph_path (cairo_scaled_font_t *scaled_font,
          const cairo_glyph_t *glyphs,
          int num_glyphs,
          cairo_path_fixed_t *path);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_scaled_glyph_set_metrics (cairo_scaled_glyph_t *scaled_glyph,
     cairo_scaled_font_t *scaled_font,
     cairo_text_extents_t *fs_metrics);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_scaled_glyph_set_surface (cairo_scaled_glyph_t *scaled_glyph,
     cairo_scaled_font_t *scaled_font,
     cairo_image_surface_t *surface);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_scaled_glyph_set_path (cairo_scaled_glyph_t *scaled_glyph,
         cairo_scaled_font_t *scaled_font,
         cairo_path_fixed_t *path);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_int_status_t
_moonlight_cairo_scaled_glyph_lookup (cairo_scaled_font_t *scaled_font,
       unsigned long index,
       cairo_scaled_glyph_info_t info,
       cairo_scaled_glyph_t **scaled_glyph_ret);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_scaled_font_map_destroy (void);



__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_stroke_style_init (cairo_stroke_style_t *style);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_stroke_style_init_copy (cairo_stroke_style_t *style,
          cairo_stroke_style_t *other);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_stroke_style_fini (cairo_stroke_style_t *style);



__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_surface_t *
_cairo_surface_create_in_error (cairo_status_t status);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_surface_set_error (cairo_surface_t *surface,
     cairo_status_t status);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_cairo_surface_set_resolution (cairo_surface_t *surface,
                               double x_res,
                               double y_res);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_surface_t *
_moonlight_cairo_surface_create_similar_scratch (cairo_surface_t *other,
           cairo_content_t content,
           int width,
           int height);




__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_surface_t *
_moonlight_cairo_surface_create_similar_solid (cairo_surface_t *other,
         cairo_content_t content,
         int width,
         int height,
         const cairo_color_t *color,
         cairo_pattern_t *color_pattern);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_surface_init (cairo_surface_t *surface,
       const cairo_surface_backend_t *backend,
       cairo_content_t content);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_surface_set_font_options (cairo_surface_t *surface,
     cairo_font_options_t *options);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_clip_mode_t
_moonlight_cairo_surface_get_clip_mode (cairo_surface_t *surface);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_surface_composite (cairo_operator_t op,
     cairo_pattern_t *src,
     cairo_pattern_t *mask,
     cairo_surface_t *dst,
     int src_x,
     int src_y,
     int mask_x,
     int mask_y,
     int dst_x,
     int dst_y,
     unsigned int width,
     unsigned int height);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_surface_fill_rectangle (cairo_surface_t *surface,
          cairo_operator_t op,
          const cairo_color_t *color,
          int x,
          int y,
          int width,
          int height);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_surface_fill_region (cairo_surface_t *surface,
       cairo_operator_t op,
       const cairo_color_t *color,
       cairo_region_t *region);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_surface_fill_rectangles (cairo_surface_t *surface,
    cairo_operator_t op,
    const cairo_color_t *color,
    cairo_rectangle_int_t *rects,
    int num_rects);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_surface_paint (cairo_surface_t *surface,
        cairo_operator_t op,
        cairo_pattern_t *source);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_surface_mask (cairo_surface_t *surface,
       cairo_operator_t op,
       cairo_pattern_t *source,
       cairo_pattern_t *mask);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_cairo_surface_fill_stroke (cairo_surface_t *surface,
       cairo_operator_t fill_op,
       cairo_pattern_t *fill_source,
       cairo_fill_rule_t fill_rule,
       double fill_tolerance,
       cairo_antialias_t fill_antialias,
       cairo_path_fixed_t *path,
       cairo_operator_t stroke_op,
       cairo_pattern_t *stroke_source,
       cairo_stroke_style_t *stroke_style,
       cairo_matrix_t *stroke_ctm,
       cairo_matrix_t *stroke_ctm_inverse,
       double stroke_tolerance,
       cairo_antialias_t stroke_antialias);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_surface_stroke (cairo_surface_t *surface,
         cairo_operator_t op,
         cairo_pattern_t *source,
         cairo_path_fixed_t *path,
         cairo_stroke_style_t *style,
         cairo_matrix_t *ctm,
         cairo_matrix_t *ctm_inverse,
         double tolerance,
         cairo_antialias_t antialias);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_surface_fill (cairo_surface_t *surface,
       cairo_operator_t op,
       cairo_pattern_t *source,
       cairo_path_fixed_t *path,
       cairo_fill_rule_t fill_rule,
       double tolerance,
       cairo_antialias_t antialias);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_surface_show_glyphs (cairo_surface_t *surface,
       cairo_operator_t op,
       cairo_pattern_t *source,
       cairo_glyph_t *glyphs,
       int num_glyphs,
       cairo_scaled_font_t *scaled_font);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_surface_composite_trapezoids (cairo_operator_t op,
         cairo_pattern_t *pattern,
         cairo_surface_t *dst,
         cairo_antialias_t antialias,
         int src_x,
         int src_y,
         int dst_x,
         int dst_y,
         unsigned int width,
         unsigned int height,
         cairo_trapezoid_t *traps,
         int ntraps);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_surface_acquire_source_image (cairo_surface_t *surface,
         cairo_image_surface_t **image_out,
         void **image_extra);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_surface_release_source_image (cairo_surface_t *surface,
         cairo_image_surface_t *image,
         void *image_extra);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_surface_acquire_dest_image (cairo_surface_t *surface,
       cairo_rectangle_int_t *interest_rect,
       cairo_image_surface_t **image_out,
       cairo_rectangle_int_t *image_rect,
       void **image_extra);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_surface_release_dest_image (cairo_surface_t *surface,
       cairo_rectangle_int_t *interest_rect,
       cairo_image_surface_t *image,
       cairo_rectangle_int_t *image_rect,
       void *image_extra);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_surface_clone_similar (cairo_surface_t *surface,
         cairo_surface_t *src,
         int src_x,
         int src_y,
         int width,
         int height,
         cairo_surface_t **clone_out);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_surface_t *
_moonlight_cairo_surface_snapshot (cairo_surface_t *surface);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_bool_t
_moonlight_cairo_surface_is_similar (cairo_surface_t *surface_a,
                    cairo_surface_t *surface_b,
      cairo_content_t content);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_surface_reset (cairo_surface_t *surface);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) unsigned int
_moonlight_cairo_surface_get_current_clip_serial (cairo_surface_t *surface);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) unsigned int
_moonlight_cairo_surface_allocate_clip_serial (cairo_surface_t *surface);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_surface_reset_clip (cairo_surface_t *surface);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_surface_set_clip_region (cairo_surface_t *surface,
    cairo_region_t *region,
    unsigned int serial);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_int_status_t
_moonlight_cairo_surface_intersect_clip_path (cairo_surface_t *surface,
        cairo_path_fixed_t *path,
        cairo_fill_rule_t fill_rule,
        double tolerance,
        cairo_antialias_t antialias);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_surface_set_clip (cairo_surface_t *surface, cairo_clip_t *clip);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_surface_get_extents (cairo_surface_t *surface,
       cairo_rectangle_int_t *rectangle);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_surface_old_show_glyphs (cairo_scaled_font_t *scaled_font,
    cairo_operator_t op,
    cairo_pattern_t *pattern,
    cairo_surface_t *surface,
    int source_x,
    int source_y,
    int dest_x,
    int dest_y,
    unsigned int width,
    unsigned int height,
    cairo_glyph_t *glyphs,
    int num_glyphs);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_surface_composite_fixup_unbounded (cairo_surface_t *dst,
       cairo_surface_attributes_t *src_attr,
       int src_width,
       int src_height,
       cairo_surface_attributes_t *mask_attr,
       int mask_width,
       int mask_height,
       int src_x,
       int src_y,
       int mask_x,
       int mask_y,
       int dst_x,
       int dst_y,
       unsigned int width,
       unsigned int height);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_surface_composite_shape_fixup_unbounded (cairo_surface_t *dst,
      cairo_surface_attributes_t *src_attr,
      int src_width,
      int src_height,
      int mask_width,
      int mask_height,
      int src_x,
      int src_y,
      int mask_x,
      int mask_y,
      int dst_x,
      int dst_y,
      unsigned int width,
      unsigned int height);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_bool_t
_cairo_surface_is_opaque (const cairo_surface_t *surface);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_surface_set_device_scale (cairo_surface_t *surface,
     double sx,
     double sy);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_bool_t
_moonlight_cairo_surface_has_device_transform (cairo_surface_t *surface);
# 1798 "cairoint.h"
__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_format_t
_cairo_format_width (cairo_format_t format);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_format_t
_moonlight_cairo_format_from_content (cairo_content_t content);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_content_t
_moonlight_cairo_content_from_format (cairo_format_t format);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_surface_t *
_moonlight_cairo_image_surface_create_for_pixman_image (pixman_image_t *pixman_image,
           pixman_format_code_t pixman_format);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) pixman_format_code_t
_pixman_format_from_masks (cairo_format_masks_t *masks);

void
_pixman_format_to_masks (pixman_format_code_t pixman_format,
    uint32_t *bpp,
    uint32_t *red,
    uint32_t *green,
    uint32_t *blue);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_surface_t *
_cairo_image_surface_create_with_pixman_format (unsigned char *data,
      pixman_format_code_t pixman_format,
      int width,
      int height,
      int stride);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_surface_t *
_moonlight_cairo_image_surface_create_with_masks (unsigned char *data,
     cairo_format_masks_t *format,
     int width,
     int height,
     int stride);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_surface_t *
_moonlight_cairo_image_surface_create_with_content (cairo_content_t content,
       int width,
       int height);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_surface_t *
_moonlight_cairo_image_surface_create_for_data_with_content (unsigned char *data,
         cairo_content_t content,
         int width,
         int height,
         int stride);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_image_surface_assume_ownership_of_data (cairo_image_surface_t *surface);
# 1859 "cairoint.h"
__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_int_status_t
_moonlight_cairo_image_surface_set_clip_region (void *abstract_surface,
          cairo_region_t *region);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_image_surface_t *
_moonlight_cairo_image_surface_clone (cairo_image_surface_t *surface,
       cairo_format_t format);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_bool_t
_moonlight_cairo_surface_is_image (const cairo_surface_t *surface);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_bool_t
_moonlight_cairo_surface_is_meta (const cairo_surface_t *surface);


__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_pen_init (cairo_pen_t *pen,
   double radius,
   double tolerance,
   cairo_matrix_t *ctm);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_pen_init_empty (cairo_pen_t *pen);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_pen_init_copy (cairo_pen_t *pen, cairo_pen_t *other);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_pen_fini (cairo_pen_t *pen);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_pen_add_points (cairo_pen_t *pen, cairo_point_t *point, int num_points);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_cairo_pen_add_points_for_slopes (cairo_pen_t *pen,
      cairo_point_t *a,
      cairo_point_t *b,
      cairo_point_t *c,
      cairo_point_t *d);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_pen_find_active_cw_vertex_index (cairo_pen_t *pen,
     cairo_slope_t *slope,
     int *active);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_pen_find_active_ccw_vertex_index (cairo_pen_t *pen,
      cairo_slope_t *slope,
      int *active);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_pen_stroke_spline (cairo_pen_t *pen,
     cairo_spline_t *spline,
     double tolerance,
     cairo_traps_t *traps);


__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_polygon_init (cairo_polygon_t *polygon);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_polygon_fini (cairo_polygon_t *polygon);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_polygon_status (cairo_polygon_t *polygon);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_polygon_add_edge (cairo_polygon_t *polygon, cairo_point_t *p1, cairo_point_t *p2);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_polygon_move_to (cairo_polygon_t *polygon, cairo_point_t *point);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_polygon_line_to (cairo_polygon_t *polygon, cairo_point_t *point);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_polygon_close (cairo_polygon_t *polygon);


__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_int_status_t
_moonlight_cairo_spline_init (cairo_spline_t *spline,
      const cairo_point_t *a,
      const cairo_point_t *b,
      const cairo_point_t *c,
      const cairo_point_t *d);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_spline_decompose (cairo_spline_t *spline, double tolerance);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_spline_fini (cairo_spline_t *spline);


__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_matrix_get_affine (const cairo_matrix_t *matrix,
     double *xx, double *yx,
     double *xy, double *yy,
     double *x0, double *y0);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_matrix_transform_bounding_box (const cairo_matrix_t *matrix,
          double *x1, double *y1,
          double *x2, double *y2,
          cairo_bool_t *is_tight);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_bool_t
_cairo_matrix_is_invertible (const cairo_matrix_t *matrix);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_matrix_compute_determinant (const cairo_matrix_t *matrix, double *det);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_matrix_compute_scale_factors (const cairo_matrix_t *matrix,
         double *sx, double *sy, int x_major);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_bool_t
_moonlight_cairo_matrix_is_identity (const cairo_matrix_t *matrix);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_bool_t
_moonlight_cairo_matrix_is_translation (const cairo_matrix_t *matrix);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_bool_t
_moonlight_cairo_matrix_is_integer_translation(const cairo_matrix_t *matrix,
         int *itx, int *ity);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) double
_moonlight_cairo_matrix_transformed_circle_major_axis(cairo_matrix_t *matrix, double radius);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_matrix_to_pixman_matrix (const cairo_matrix_t *matrix,
    pixman_transform_t *pixman_transform);


__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_traps_init (cairo_traps_t *traps);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_traps_limit (cairo_traps_t *traps,
      cairo_box_t *limits);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_traps_init_box (cairo_traps_t *traps,
         cairo_box_t *box);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_traps_fini (cairo_traps_t *traps);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_traps_status (cairo_traps_t *traps);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_traps_translate (cairo_traps_t *traps, int x, int y);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_traps_tessellate_triangle (cairo_traps_t *traps, cairo_point_t t[3]);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_traps_tessellate_convex_quad (cairo_traps_t *traps, cairo_point_t q[4]);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_cairo_traps_tessellate_polygon (cairo_traps_t *traps,
     cairo_polygon_t *poly,
     cairo_fill_rule_t fill_rule);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_traps_add_trap_from_points (cairo_traps_t *traps, cairo_fixed_t top, cairo_fixed_t bottom,
       cairo_point_t left_p1, cairo_point_t left_p2,
       cairo_point_t right_p1, cairo_point_t right_p2);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_bentley_ottmann_tessellate_polygon (cairo_traps_t *traps,
        cairo_polygon_t *polygon,
        cairo_fill_rule_t fill_rule);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) int
_moonlight_cairo_traps_contain (cairo_traps_t *traps, double x, double y);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_traps_extents (cairo_traps_t *traps, cairo_box_t *extents);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_int_status_t
_moonlight_cairo_traps_extract_region (cairo_traps_t *tr,
        cairo_region_t *region);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_trapezoid_array_translate_and_scale (cairo_trapezoid_t *offset_traps,
         cairo_trapezoid_t *src_traps,
         int num_traps,
         double tx, double ty,
         double sx, double sy);


__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_slope_init (cairo_slope_t *slope, cairo_point_t *a, cairo_point_t *b);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) int
_moonlight_cairo_slope_compare (cairo_slope_t *a, cairo_slope_t *b);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) int
_moonlight_cairo_slope_clockwise (cairo_slope_t *a, cairo_slope_t *b);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) int
_moonlight_cairo_slope_counter_clockwise (cairo_slope_t *a, cairo_slope_t *b);



__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_cairo_pattern_create_copy (cairo_pattern_t **pattern,
       const cairo_pattern_t *other);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_pattern_init_copy (cairo_pattern_t *pattern,
     const cairo_pattern_t *other);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_pattern_init_solid (cairo_solid_pattern_t *pattern,
      const cairo_color_t *color,
      cairo_content_t content);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_pattern_init_for_surface (cairo_surface_pattern_t *pattern,
     cairo_surface_t *surface);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_pattern_init_linear (cairo_linear_pattern_t *pattern,
       double x0, double y0, double x1, double y1);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_pattern_init_radial (cairo_radial_pattern_t *pattern,
       double cx0, double cy0, double radius0,
       double cx1, double cy1, double radius1);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_pattern_fini (cairo_pattern_t *pattern);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_pattern_t *
_moonlight_cairo_pattern_create_solid (const cairo_color_t *color,
        cairo_content_t content);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_pattern_transform (cairo_pattern_t *pattern,
     const cairo_matrix_t *ctm_inverse);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_bool_t
_moonlight_cairo_pattern_is_opaque_solid (const cairo_pattern_t *pattern);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_bool_t
_moonlight_cairo_pattern_is_opaque (const cairo_pattern_t *abstract_pattern);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_int_status_t
_moonlight_cairo_pattern_acquire_surface (cairo_pattern_t *pattern,
    cairo_surface_t *dst,
    int x,
    int y,
    unsigned int width,
    unsigned int height,
    cairo_surface_t **surface_out,
    cairo_surface_attributes_t *attributes);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_pattern_release_surface (cairo_pattern_t *pattern,
    cairo_surface_t *surface,
    cairo_surface_attributes_t *attributes);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_int_status_t
_moonlight_cairo_pattern_acquire_surfaces (cairo_pattern_t *src,
     cairo_pattern_t *mask,
     cairo_surface_t *dst,
     int src_x,
     int src_y,
     int mask_x,
     int mask_y,
     unsigned int width,
     unsigned int height,
     cairo_surface_t **src_out,
     cairo_surface_t **mask_out,
     cairo_surface_attributes_t *src_attributes,
     cairo_surface_attributes_t *mask_attributes);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_pattern_get_extents (cairo_pattern_t *pattern,
       cairo_rectangle_int_t *extents);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_pattern_reset_static_data (void);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_gstate_set_antialias (cairo_gstate_t *gstate,
        cairo_antialias_t antialias);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_antialias_t
_moonlight_cairo_gstate_get_antialias (cairo_gstate_t *gstate);



# 1 "cairo-region-private.h" 1
# 46 "cairo-region-private.h"
struct _cairo_region {
    pixman_region16_t rgn;
};

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_cairo_region_init (cairo_region_t *region);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_cairo_region_init_rect (cairo_region_t *region,
    cairo_rectangle_int_t *rect);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_int_status_t
_cairo_region_init_boxes (cairo_region_t *region,
     cairo_box_int_t *boxes,
     int count);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_cairo_region_fini (cairo_region_t *region);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_int_status_t
_cairo_region_copy (cairo_region_t *dst,
      cairo_region_t *src);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) int
_cairo_region_num_boxes (cairo_region_t *region);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_int_status_t
_cairo_region_get_boxes (cairo_region_t *region,
    int *num_boxes,
    cairo_box_int_t **boxes);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_cairo_region_boxes_fini (cairo_region_t *region,
     cairo_box_int_t *boxes);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_cairo_region_get_extents (cairo_region_t *region,
      cairo_rectangle_int_t *extents);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_int_status_t
_cairo_region_subtract (cairo_region_t *dst,
   cairo_region_t *a,
   cairo_region_t *b);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_int_status_t
_cairo_region_intersect (cairo_region_t *dst,
    cairo_region_t *a,
    cairo_region_t *b);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_int_status_t
_cairo_region_union_rect (cairo_region_t *dst,
     cairo_region_t *src,
     cairo_rectangle_int_t *rect);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_bool_t
_cairo_region_not_empty (cairo_region_t *region);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_cairo_region_translate (cairo_region_t *region,
    int x, int y);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) pixman_region_overlap_t
_cairo_region_contains_rectangle (cairo_region_t *region, cairo_rectangle_int_t *box);
# 2155 "cairoint.h" 2



__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_utf8_to_ucs4 (const unsigned char *str,
       int len,
       uint32_t **result,
       int *items_written);
# 2175 "cairoint.h"
__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_error (cairo_status_t status);
# 2185 "cairoint.h"
extern __typeof (moonlight_cairo_clip_preserve) moonlight_cairo_clip_preserve __asm__ ("" "INT_moonlight_cairo_clip_preserve") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_close_path) moonlight_cairo_close_path __asm__ ("" "INT_moonlight_cairo_close_path") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_create) moonlight_cairo_create __asm__ ("" "INT_moonlight_cairo_create") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_curve_to) moonlight_cairo_curve_to __asm__ ("" "INT_moonlight_cairo_curve_to") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_destroy) moonlight_cairo_destroy __asm__ ("" "INT_moonlight_cairo_destroy") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_fill_preserve) moonlight_cairo_fill_preserve __asm__ ("" "INT_moonlight_cairo_fill_preserve") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_font_face_destroy) moonlight_cairo_font_face_destroy __asm__ ("" "INT_moonlight_cairo_font_face_destroy") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_font_face_reference) moonlight_cairo_font_face_reference __asm__ ("" "INT_moonlight_cairo_font_face_reference") __attribute__((__visibility__("hidden")));
extern __typeof (moonlight_cairo_font_options_create) moonlight_cairo_font_options_create __asm__ ("" "INT_moonlight_cairo_font_options_create") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_font_options_destroy) moonlight_cairo_font_options_destroy __asm__ ("" "INT_moonlight_cairo_font_options_destroy") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_font_options_equal) moonlight_cairo_font_options_equal __asm__ ("" "INT_moonlight_cairo_font_options_equal") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_font_options_hash) moonlight_cairo_font_options_hash __asm__ ("" "INT_moonlight_cairo_font_options_hash") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_font_options_merge) moonlight_cairo_font_options_merge __asm__ ("" "INT_moonlight_cairo_font_options_merge") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_font_options_set_antialias) moonlight_cairo_font_options_set_antialias __asm__ ("" "INT_moonlight_cairo_font_options_set_antialias") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_font_options_set_hint_metrics) moonlight_cairo_font_options_set_hint_metrics __asm__ ("" "INT_moonlight_cairo_font_options_set_hint_metrics") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_font_options_set_hint_style) moonlight_cairo_font_options_set_hint_style __asm__ ("" "INT_moonlight_cairo_font_options_set_hint_style") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_font_options_set_subpixel_order) moonlight_cairo_font_options_set_subpixel_order __asm__ ("" "INT_moonlight_cairo_font_options_set_subpixel_order") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_font_options_status) moonlight_cairo_font_options_status __asm__ ("" "INT_moonlight_cairo_font_options_status") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_get_current_point) moonlight_cairo_get_current_point __asm__ ("" "INT_moonlight_cairo_get_current_point") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_get_matrix) moonlight_cairo_get_matrix __asm__ ("" "INT_moonlight_cairo_get_matrix") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_get_tolerance) moonlight_cairo_get_tolerance __asm__ ("" "INT_moonlight_cairo_get_tolerance") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_image_surface_create) moonlight_cairo_image_surface_create __asm__ ("" "INT_moonlight_cairo_image_surface_create") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_image_surface_create_for_data) moonlight_cairo_image_surface_create_for_data __asm__ ("" "INT_moonlight_cairo_image_surface_create_for_data") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_image_surface_get_height) moonlight_cairo_image_surface_get_height __asm__ ("" "INT_moonlight_cairo_image_surface_get_height") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_image_surface_get_width) moonlight_cairo_image_surface_get_width __asm__ ("" "INT_moonlight_cairo_image_surface_get_width") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_line_to) moonlight_cairo_line_to __asm__ ("" "INT_moonlight_cairo_line_to") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_mask) moonlight_cairo_mask __asm__ ("" "INT_moonlight_cairo_mask") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_matrix_init) moonlight_cairo_matrix_init __asm__ ("" "INT_moonlight_cairo_matrix_init") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_matrix_init_identity) moonlight_cairo_matrix_init_identity __asm__ ("" "INT_moonlight_cairo_matrix_init_identity") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_matrix_init_rotate) moonlight_cairo_matrix_init_rotate __asm__ ("" "INT_moonlight_cairo_matrix_init_rotate") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_matrix_init_scale) moonlight_cairo_matrix_init_scale __asm__ ("" "INT_moonlight_cairo_matrix_init_scale") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_matrix_init_translate) moonlight_cairo_matrix_init_translate __asm__ ("" "INT_moonlight_cairo_matrix_init_translate") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_matrix_invert) moonlight_cairo_matrix_invert __asm__ ("" "INT_moonlight_cairo_matrix_invert") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_matrix_multiply) moonlight_cairo_matrix_multiply __asm__ ("" "INT_moonlight_cairo_matrix_multiply") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_matrix_scale) moonlight_cairo_matrix_scale __asm__ ("" "INT_moonlight_cairo_matrix_scale") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_matrix_transform_distance) moonlight_cairo_matrix_transform_distance __asm__ ("" "INT_moonlight_cairo_matrix_transform_distance") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_matrix_transform_point) moonlight_cairo_matrix_transform_point __asm__ ("" "INT_moonlight_cairo_matrix_transform_point") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_matrix_translate) moonlight_cairo_matrix_translate __asm__ ("" "INT_moonlight_cairo_matrix_translate") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_move_to) moonlight_cairo_move_to __asm__ ("" "INT_moonlight_cairo_move_to") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_new_path) moonlight_cairo_new_path __asm__ ("" "INT_moonlight_cairo_new_path") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_paint) moonlight_cairo_paint __asm__ ("" "INT_moonlight_cairo_paint") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (cairo_path_extents) cairo_path_extents __asm__ ("" "INT_cairo_path_extents") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_pattern_create_for_surface) moonlight_cairo_pattern_create_for_surface __asm__ ("" "INT_moonlight_cairo_pattern_create_for_surface") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_pattern_create_rgb) moonlight_cairo_pattern_create_rgb __asm__ ("" "INT_moonlight_cairo_pattern_create_rgb") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_pattern_create_rgba) moonlight_cairo_pattern_create_rgba __asm__ ("" "INT_moonlight_cairo_pattern_create_rgba") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_pattern_destroy) moonlight_cairo_pattern_destroy __asm__ ("" "INT_moonlight_cairo_pattern_destroy") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_pattern_get_extend) moonlight_cairo_pattern_get_extend __asm__ ("" "INT_moonlight_cairo_pattern_get_extend") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_pattern_get_type) moonlight_cairo_pattern_get_type __asm__ ("" "INT_moonlight_cairo_pattern_get_type") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_pattern_reference) moonlight_cairo_pattern_reference __asm__ ("" "INT_moonlight_cairo_pattern_reference") __attribute__((__visibility__("hidden")));
extern __typeof (moonlight_cairo_pattern_set_matrix) moonlight_cairo_pattern_set_matrix __asm__ ("" "INT_moonlight_cairo_pattern_set_matrix") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_pattern_status) moonlight_cairo_pattern_status __asm__ ("" "INT_moonlight_cairo_pattern_status") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_pop_group) moonlight_cairo_pop_group __asm__ ("" "INT_moonlight_cairo_pop_group") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_pop_group_to_source) moonlight_cairo_pop_group_to_source __asm__ ("" "INT_moonlight_cairo_pop_group_to_source") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_push_group) moonlight_cairo_push_group __asm__ ("" "INT_moonlight_cairo_push_group") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_push_group_with_content) moonlight_cairo_push_group_with_content __asm__ ("" "INT_moonlight_cairo_push_group_with_content") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_rel_line_to) moonlight_cairo_rel_line_to __asm__ ("" "INT_moonlight_cairo_rel_line_to") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_restore) moonlight_cairo_restore __asm__ ("" "INT_moonlight_cairo_restore") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_save) moonlight_cairo_save __asm__ ("" "INT_moonlight_cairo_save") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_scale) moonlight_cairo_scale __asm__ ("" "INT_moonlight_cairo_scale") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_scaled_font_create) moonlight_cairo_scaled_font_create __asm__ ("" "INT_moonlight_cairo_scaled_font_create") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_scaled_font_destroy) moonlight_cairo_scaled_font_destroy __asm__ ("" "INT_moonlight_cairo_scaled_font_destroy") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_scaled_font_extents) moonlight_cairo_scaled_font_extents __asm__ ("" "INT_moonlight_cairo_scaled_font_extents") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_scaled_font_get_ctm) moonlight_cairo_scaled_font_get_ctm __asm__ ("" "INT_moonlight_cairo_scaled_font_get_ctm") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_scaled_font_get_font_face) moonlight_cairo_scaled_font_get_font_face __asm__ ("" "INT_moonlight_cairo_scaled_font_get_font_face") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_scaled_font_get_font_matrix) moonlight_cairo_scaled_font_get_font_matrix __asm__ ("" "INT_moonlight_cairo_scaled_font_get_font_matrix") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_scaled_font_get_font_options) moonlight_cairo_scaled_font_get_font_options __asm__ ("" "INT_moonlight_cairo_scaled_font_get_font_options") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_scaled_font_glyph_extents) moonlight_cairo_scaled_font_glyph_extents __asm__ ("" "INT_moonlight_cairo_scaled_font_glyph_extents") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_scaled_font_reference) moonlight_cairo_scaled_font_reference __asm__ ("" "INT_moonlight_cairo_scaled_font_reference") __attribute__((__visibility__("hidden")));
extern __typeof (moonlight_cairo_scaled_font_status) moonlight_cairo_scaled_font_status __asm__ ("" "INT_moonlight_cairo_scaled_font_status") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_set_operator) moonlight_cairo_set_operator __asm__ ("" "INT_moonlight_cairo_set_operator") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_set_source) moonlight_cairo_set_source __asm__ ("" "INT_moonlight_cairo_set_source") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_set_source_surface) moonlight_cairo_set_source_surface __asm__ ("" "INT_moonlight_cairo_set_source_surface") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_status) moonlight_cairo_status __asm__ ("" "INT_moonlight_cairo_status") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_stroke_preserve) moonlight_cairo_stroke_preserve __asm__ ("" "INT_moonlight_cairo_stroke_preserve") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_surface_create_similar) moonlight_cairo_surface_create_similar __asm__ ("" "INT_moonlight_cairo_surface_create_similar") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_surface_destroy) moonlight_cairo_surface_destroy __asm__ ("" "INT_moonlight_cairo_surface_destroy") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_surface_finish) moonlight_cairo_surface_finish __asm__ ("" "INT_moonlight_cairo_surface_finish") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_surface_get_content) moonlight_cairo_surface_get_content __asm__ ("" "INT_moonlight_cairo_surface_get_content") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_surface_get_device_offset) moonlight_cairo_surface_get_device_offset __asm__ ("" "INT_moonlight_cairo_surface_get_device_offset") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_surface_get_font_options) moonlight_cairo_surface_get_font_options __asm__ ("" "INT_moonlight_cairo_surface_get_font_options") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_surface_get_type) moonlight_cairo_surface_get_type __asm__ ("" "INT_moonlight_cairo_surface_get_type") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_surface_mark_dirty_rectangle) moonlight_cairo_surface_mark_dirty_rectangle __asm__ ("" "INT_moonlight_cairo_surface_mark_dirty_rectangle") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_surface_reference) moonlight_cairo_surface_reference __asm__ ("" "INT_moonlight_cairo_surface_reference") __attribute__((__visibility__("hidden")));
extern __typeof (moonlight_cairo_surface_set_device_offset) moonlight_cairo_surface_set_device_offset __asm__ ("" "INT_moonlight_cairo_surface_set_device_offset") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_surface_set_fallback_resolution) moonlight_cairo_surface_set_fallback_resolution __asm__ ("" "INT_moonlight_cairo_surface_set_fallback_resolution") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (cairo_surface_copy_page) cairo_surface_copy_page __asm__ ("" "INT_cairo_surface_copy_page") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (cairo_surface_show_page) cairo_surface_show_page __asm__ ("" "INT_cairo_surface_show_page") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_surface_status) moonlight_cairo_surface_status __asm__ ("" "INT_moonlight_cairo_surface_status") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));
extern __typeof (moonlight_cairo_version_string) moonlight_cairo_version_string __asm__ ("" "INT_moonlight_cairo_version_string") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));



extern __typeof (moonlight_cairo_surface_write_to_png_stream) moonlight_cairo_surface_write_to_png_stream __asm__ ("" "INT_moonlight_cairo_surface_write_to_png_stream") __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__));





# 1 "cairo-mutex-private.h" 1
# 45 "cairo-mutex-private.h"
# 1 "../config.h" 1
# 46 "cairo-mutex-private.h" 2
# 64 "cairo-mutex-private.h"




# 1 "cairo-mutex-list-private.h" 1
# 35 "cairo-mutex-list-private.h"
extern cairo_mutex_t _moonlight_cairo_pattern_solid_pattern_cache_lock;
extern cairo_mutex_t _moonlight_cairo_pattern_solid_surface_cache_lock;

extern cairo_mutex_t _moonlight_cairo_font_face_mutex;
extern cairo_mutex_t _moonlight_cairo_scaled_font_map_mutex;


extern cairo_mutex_t _moonlight_cairo_ft_unscaled_font_map_mutex;



extern cairo_mutex_t _moonlight_cairo_xlib_display_mutex;



extern cairo_mutex_t _cairo_atomic_mutex;
# 69 "cairo-mutex-private.h" 2
# 151 "cairo-mutex-private.h"

# 2284 "cairoint.h" 2
# 1 "cairo-fixed-private.h" 1
# 42 "cairo-fixed-private.h"
# 1 "cairo-wideint-private.h" 1
# 142 "cairo-wideint-private.h"
cairo_uquorem64_t __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__))
_moonlight_cairo_uint64_divrem (cairo_uint64_t num, cairo_uint64_t den);

cairo_quorem64_t __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__))
_moonlight_cairo_int64_divrem (cairo_int64_t num, cairo_int64_t den);
# 156 "cairo-wideint-private.h"
cairo_uint128_t __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) _moonlight_cairo_uint32_to_uint128 (uint32_t i);
cairo_uint128_t __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) _moonlight_cairo_uint64_to_uint128 (cairo_uint64_t i);


cairo_uint128_t __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) _moonlight_cairo_uint128_add (cairo_uint128_t a, cairo_uint128_t b);
cairo_uint128_t __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) _moonlight_cairo_uint128_sub (cairo_uint128_t a, cairo_uint128_t b);
cairo_uint128_t __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) _moonlight_cairo_uint128_mul (cairo_uint128_t a, cairo_uint128_t b);
cairo_uint128_t __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) _moonlight_cairo_uint64x64_128_mul (cairo_uint64_t a, cairo_uint64_t b);
cairo_uint128_t __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) _moonlight_cairo_uint128_lsl (cairo_uint128_t a, int shift);
cairo_uint128_t __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) _moonlight_cairo_uint128_rsl (cairo_uint128_t a, int shift);
cairo_uint128_t __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) _moonlight_cairo_uint128_rsa (cairo_uint128_t a, int shift);
int __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) _moonlight_cairo_uint128_lt (cairo_uint128_t a, cairo_uint128_t b);
int __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) _moonlight_cairo_uint128_eq (cairo_uint128_t a, cairo_uint128_t b);
cairo_uint128_t __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) _moonlight_cairo_uint128_negate (cairo_uint128_t a);

cairo_uint128_t __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) _moonlight_cairo_uint128_not (cairo_uint128_t a);




cairo_int128_t __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) _moonlight_cairo_int32_to_int128 (int32_t i);
cairo_int128_t __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) _moonlight_cairo_int64_to_int128 (cairo_int64_t i);





cairo_int128_t __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) _moonlight_cairo_int64x64_128_mul (cairo_int64_t a, cairo_int64_t b);



int __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) _moonlight_cairo_int128_lt (cairo_int128_t a, cairo_int128_t b);
# 234 "cairo-wideint-private.h"
cairo_uquorem128_t __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__))
_moonlight_cairo_uint128_divrem (cairo_uint128_t num, cairo_uint128_t den);

cairo_quorem128_t __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__))
_moonlight_cairo_int128_divrem (cairo_int128_t num, cairo_int128_t den);

cairo_uquorem64_t __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__))
_moonlight_cairo_uint_96by64_32x64_divrem (cairo_uint128_t num,
     cairo_uint64_t den);

cairo_quorem64_t __attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__))
_moonlight_cairo_int_96by64_32x64_divrem (cairo_int128_t num,
    cairo_int64_t den);
# 43 "cairo-fixed-private.h" 2
# 58 "cairo-fixed-private.h"
static inline cairo_fixed_t
_moonlight_cairo_fixed_from_int (int i)
{
    return i << 8;
}
# 107 "cairo-fixed-private.h"
static inline cairo_fixed_t
_moonlight_cairo_fixed_from_double (double d)
{
    union {
        double d;
        int32_t i[2];
    } u;

    u.d = d + ((1LL << (52 - 8)) * 1.5);



    return u.i[0];

}






static inline cairo_fixed_t
_moonlight_cairo_fixed_from_26_6 (uint32_t i)
{

    return i << (8 - 6);



}

static inline double
_moonlight_cairo_fixed_to_double (cairo_fixed_t f)
{
    return ((double) f) / ((double)(1 << 8));
}

static inline int
_moonlight_cairo_fixed_is_integer (cairo_fixed_t f)
{
    return (f & (((cairo_fixed_unsigned_t)(-1)) >> (32 - 8))) == 0;
}

static inline int
_moonlight_cairo_fixed_integer_part (cairo_fixed_t f)
{
    return f >> 8;
}

static inline int
_moonlight_cairo_fixed_integer_floor (cairo_fixed_t f)
{
    if (f >= 0)
        return f >> 8;
    else
        return -((-f - 1) >> 8) - 1;
}

static inline int
_moonlight_cairo_fixed_integer_ceil (cairo_fixed_t f)
{
    if (f > 0)
 return ((f - 1)>>8) + 1;
    else
 return - (-f >> 8);
}





static inline cairo_fixed_16_16_t
_cairo_fixed_to_16_16 (cairo_fixed_t f)
{






    cairo_fixed_16_16_t x;




    if ((f >> 8) < (-32767-1)) {
 x = (-2147483647-1);
    } else if ((f >> 8) > (32767)) {
 x = (2147483647);
    } else {
 x = f << (16 - 8);
    }

    return x;

}

static inline cairo_fixed_16_16_t
_cairo_fixed_16_16_from_double (double d)
{
    union {
        double d;
        int32_t i[2];
    } u;

    u.d = d + (103079215104.0);



    return u.i[0];

}



static inline cairo_fixed_t
_cairo_fixed_mul (cairo_fixed_t a, cairo_fixed_t b)
{
    cairo_int64_t temp = ((int64_t) (a) * (b));
    return ((int32_t) (((int64_t) ((uint64_t) (temp) >> (8)))));
}
# 2285 "cairoint.h" 2

# 1 "cairo-malloc-private.h" 1
# 2287 "cairoint.h" 2
# 1 "cairo-hash-private.h" 1
# 50 "cairo-hash-private.h"
typedef cairo_bool_t
(*cairo_hash_keys_equal_func_t) (const void *key_a, const void *key_b);

typedef cairo_bool_t
(*cairo_hash_predicate_func_t) (void *entry);

typedef void
(*cairo_hash_callback_func_t) (void *entry,
          void *closure);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_hash_table_t *
_moonlight_cairo_hash_table_create (cairo_hash_keys_equal_func_t keys_equal);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_hash_table_destroy (cairo_hash_table_t *hash_table);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_bool_t
_moonlight_cairo_hash_table_lookup (cairo_hash_table_t *hash_table,
     cairo_hash_entry_t *key,
     cairo_hash_entry_t **entry_return);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void *
_moonlight_cairo_hash_table_random_entry (cairo_hash_table_t *hash_table,
    cairo_hash_predicate_func_t predicate);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) cairo_status_t
_moonlight_cairo_hash_table_insert (cairo_hash_table_t *hash_table,
     cairo_hash_entry_t *entry);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_hash_table_remove (cairo_hash_table_t *hash_table,
     cairo_hash_entry_t *key);

__attribute__((__visibility__("hidden"))) __attribute__((__warn_unused_result__)) void
_moonlight_cairo_hash_table_foreach (cairo_hash_table_t *hash_table,
      cairo_hash_callback_func_t hash_callback,
      void *closure);
# 2288 "cairoint.h" 2
# 2 "check-has-hidden-symbols.c" 2

1
