/* Copyright (C) 1992-2012 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _LINUX_CSKY_SYSDEP_H
#define _LINUX_CSKY_SYSDEP_H 1

/* There is some commonality.  */
#include <sysdeps/unix/sysv/linux/sysdep.h>
#include <sysdeps/unix/csky/sysdep.h>

/* Defines RTLD_PRIVATE_ERRNO and USE_DL_SYSINFO.  */
#include <dl-sysdep.h>
///#include <asm/unistd.h>

#include <tls.h>

/* In order to get __set_errno() definition in INLINE_SYSCALL.  */
#ifndef __ASSEMBLER__
#include <errno.h>
#endif

#undef SYS_ify
#define SYS_ify(syscall_name)  (__NR_##syscall_name)

#ifdef __ASSEMBLER__
/* Linux uses a negative return value to indicate syscall errors,
   unlike most Unices, which use the condition codes' carry flag.

   Since version 2.1 the return value of a system call might be
   negative even if the call succeeded.  E.g., the `lseek' system call
   might return a large offset.  Therefore we must not anymore test
   for < 0, but test for a real error by making sure the value in R0
   is a real error number.  Linus said he will make sure the no syscall
   returns a value in -1 .. -4095 as a valid result so we can safely
   test with -4095.  */

#undef	PSEUDO
#define	PSEUDO(name, syscall_name, args)				      \
  .text;								      \
  ENTRY (name);								      \
    DO_CALL (syscall_name, args);


#if defined (__PIC__)

#define __GET_GB1  \
           bsr getgb; getgb: lrw gb, getgb@GOTPC; addu gb, lr;
#else
#define __GET_GB1
#endif

#undef  PSEUDO_RET
#ifdef __PIC__
#if !IS_IN (libc)
  #define PSEUDO_RET                                                       \
    btsti  a0, 31;	\
    bf	   1f;								\
    subi sp, 8; st.w lr, (sp); st.w gb, (sp, 4);         \
    __GET_GB1    \
    bsr    SYSCALL_ERROR;   \
    ld.w lr, (sp); ld.w gb, (sp, 4); addi sp, 8;         \
1:				\
    rts
#else /* !NOT_IN_libc */
#define PSEUDO_RET							      \
    btsti  a0, 31;	\
    bf    2f;								      \
    subi sp, 8; st.w lr, (sp); st.w gb, (sp, 4);         \
    __GET_GB1;							      \
    lrw   a2, SYSCALL_ERROR@PLT;					      \
    add   a2, gb;							      \
    ld.w  a2, (a2);						              \
    jsr   a2;								      \
    ld.w lr, (sp); ld.w gb, (sp, 4); addi sp, 8;         \
2:									      \
    rts
#endif
#else/* !__PIC__ */
#if !IS_IN (libc)
#define PSEUDO_RET							      \
    btsti  a0, 31;	\
    bt     SYSCALL_ERROR;	\
    rts
#else
#define PSEUDO_RET	/* !NOT_IN_libc */					      \
    btsti  a0, 31;\
    bf     3f;								      \
    jmpi   SYSCALL_ERROR;						      \
3:									      \
    rts
#endif
#endif/* __PIC__ */


#undef ret
#define ret PSEUDO_RET

#undef	PSEUDO_END
#define	PSEUDO_END(name)						      \
  .align 4;	\
  SYSCALL_ERROR_HANDLER;						      \
  END (name)

#undef	PSEUDO_NOERRNO
#define	PSEUDO_NOERRNO(name, syscall_name, args)			      \
  .text;								      \
  ENTRY (name);								      \
    DO_CALL (syscall_name, args)

#define PSEUDO_RET_NOERRNO \
	jmp     r15;

#undef ret_NOERRNO
#define ret_NOERRNO PSEUDO_RET_NOERRNO

#undef	PSEUDO_END_NOERRNO
#define	PSEUDO_END_NOERRNO(name)					      \
  END (name)

/* The function has to return the error code.  */
#undef	PSEUDO_ERRVAL
#define	PSEUDO_ERRVAL(name, syscall_name, args) \
  .text;								      \
  ENTRY (name)								      \
    DO_CALL (syscall_name, args);					      \
    not  a0;								      \
    addi a0, 1

#undef	PSEUDO_END_ERRVAL
#define	PSEUDO_END_ERRVAL(name) \
  END (name)

#define ret_ERRVAL rts

#if !IS_IN (libc)
#define SYSCALL_ERROR __local_syscall_error
#  if RTLD_PRIVATE_ERRNO
#    ifdef __PIC__
#define SYSCALL_ERROR_HANDLER                                 \
__local_syscall_error:                                          \
        lrw     a1, rtld_errno@PLT;                             \
        addu	a1, gb;						\
        ldw     a1, (a1);                              \
        rsubi   a0, 0;                                          \
        stw     a0, (a1);                                       \
        bmaski  a0, 0;                                          \
        rts
#    else /* no pic */
#define SYSCALL_ERROR_HANDLER                                 \
__local_syscall_error:                                          \
        lrw     a1, rtld_errno;                                 \
        rsubi   a0, 0;                                          \
        stw     a0, (a1);                                       \
        bmaski  a0, 0;                                          \
        rts
#    endif /* __PIC__ */

#  else/* !RTLD_PRIVATE_ERRNO */
#    ifdef __PIC__
#define SYSCALL_ERROR_HANDLER                                   \
__local_syscall_error:                                          \
        subi    sp, 8;                                          \
        stw     a0, (sp, 0);                                    \
        stw     r15, (sp, 4);                                    \
/*      cfi_adjust_cfa_offset(8);                               \
        cfi_rel_offset(r15, 0); */                              \
        lrw     a1, __errno_location@PLT;                       \
	add	a1, gb;						\
        ldw     a1, (a1);                                       \
        jsr     a1;                                             \
        ldw     a1, (sp, 0);/* load errno*/                     \
        ldw     r15, (sp, 4);		                        \
        addi    sp, 8;                                          \
/*      cfi_adjust_cfa_offset(-8)                               \
        cfi_restore(r15)        */                              \
        movi    a2, 0;                                          \
        rsub    a1, a1, a2;                                     \
        stw     a1, (a0);                                       \
        bmaski  a0, 0;                                          \
        rts 
#    else /* no pic */
#define SYSCALL_ERROR_HANDLER                                   \
__local_syscall_error:                                          \
        subi    sp, 8;                                          \
        stw     a0, (sp, 0);                                    \
        stw     r15, (sp, 4);                                   \
/*      cfi_adjust_cfa_offset(8);                               \
        cfi_rel_offset(r15, 0); */                              \
        lrw     a1, __errno_location;                           \
        jsr     a1;                                             \
        ldw     a1, (sp, 0);  /* load errno */                  \
        ldw     r15, (sp, 4);                                   \
        addi    sp, 8;                                          \
/*      cfi_adjust_cfa_offset(-8)                               \
        cfi_restore(r15)        */                              \
        movi    a2, 0;                                          \
        rsub    a1, a1, a2;                                     \
        stw     a1, (a0);                                       \
        bmaski  a0, 0;                                          \
        rts
#    endif /* __PIC__ */
#  endif/* RTLD_PRIVATE_ERROR */
#else/* NOT_IN_libc */
#define SYSCALL_ERROR_HANDLER  /* Nothing here; code in sysdep.S is used.  */
#define SYSCALL_ERROR __syscall_error
#endif/* NOT_IN_libc */



/* define DO_CALL */
#ifdef __CSKYABIV2__
#undef	DO_CALL
#define DO_CALL(syscall_name, args)		\
    DOARGS_##args;				\
    lrw  r7, SYS_ify(syscall_name);		\
    trap 0;					\
    UNDOARGS_##args

#undef  DOARGS_0
#define DOARGS_0    \
  subi sp, 8;       \
  cfi_adjust_cfa_offset (8); \
  stw  r7, (sp, 0);  \
  cfi_rel_offset (r7, 0); 

#undef  DOARGS_1
#define DOARGS_1 DOARGS_0
#undef  DOARGS_2
#define DOARGS_2 DOARGS_0
#undef  DOARGS_3
#define DOARGS_3 DOARGS_0
#undef  DOARGS_4
#define DOARGS_4 DOARGS_0
#undef  DOARGS_5
#define DOARGS_5    \
  subi sp, 8;       \
  cfi_adjust_cfa_offset (8); \
  stw  r7, (sp, 0); \
  cfi_rel_offset (7, 0); \
  stw  r4, (sp, 4); \
  cfi_rel_offset (4, 4); \
  ldw  r4, (sp, 8)
#undef  DOARGS_6
#define DOARGS_6    \
  subi sp, 16;      \
  cfi_adjust_cfa_offset (16); \
  stw  r7, (sp, 0); \
  cfi_rel_offset (7, 0); \
  stw  r4, (sp, 4); \
  cfi_rel_offset (4, 4); \
  stw  r5, (sp, 8); \
  cfi_rel_offset (5, 8); \
  ldw  r4, (sp, 16); \
  ldw  r5, (sp, 20)

#undef  UNDOARGS_0
#define UNDOARGS_0 \
  ldw  r7, (sp, 0); \
  cfi_restore (r7); \
  addi sp, 8;   \
  cfi_adjust_cfa_offset (-8);

#undef  UNDOARGS_1
#define UNDOARGS_1 UNDOARGS_0
#undef  UNDOARGS_2
#define UNDOARGS_2 UNDOARGS_0
#undef  UNDOARGS_3
#define UNDOARGS_3 UNDOARGS_0
#undef  UNDOARGS_4
#define UNDOARGS_4 UNDOARGS_0
#undef  UNDOARGS_5
#define UNDOARGS_5  \
  ldw  r7, (sp, 0); \
  cfi_restore (r4); \
  ldw  r4, (sp, 4); \
  cfi_restore (r4); \
  addi sp, 8;  \
  cfi_adjust_cfa_offset (-8); 

#undef  UNDOARGS_6
#define UNDOARGS_6 \
  ldw  r7, (sp, 0); \
  cfi_restore (r7); \
  ldw  r4, (sp, 4); \
  cfi_restore (r4); \
  ldw  r5, (sp, 8); \
  cfi_restore (r5); \
  addi sp, 16;    \
  cfi_adjust_cfa_offset (-16); 

#else	/* __CSKYABIV1__ */

#undef  DO_CALL
#define DO_CALL(syscall_name, args)             \
    lrw  r1, SYS_ify(syscall_name);            \
    trap 0
//#endif	/* DO_CALL */
#endif	/* __CSKYABIV2__ */

/* define DO_CALL_2, only ABIV2 need DO_CALL_2 */
#ifdef __CSKYABIV2__

#undef	DO_CALL_2
#define DO_CALL_2(syscall_name, args)		\
    DOARGS2_##args;				\
    lrw  r7, SYS_ify(syscall_name);		\
    trap 0;					\
    UNDOARGS2_##args

/*
 * to be quite different with DO_CALL, DO_CALL_2 need not save r7.
 */
#undef  DOARGS2_0
#define DOARGS2_0    

#undef  DOARGS2_1
#define DOARGS2_1 DOARGS2_0
#undef  DOARGS2_2
#define DOARGS2_2 DOARGS2_0
#undef  DOARGS2_3
#define DOARGS2_3 DOARGS2_0
#undef  DOARGS2_4
#define DOARGS2_4 DOARGS2_0
#undef  DOARGS2_5
#define DOARGS2_5   \
  subi sp, 8;       \
  cfi_adjust_cfa_offset (8); \
  stw  r4, (sp, 0); \
  cfi_rel_offset (4, 0); \
  ldw  r4, (sp, 20)
#undef  DOARGS2_6
#define DOARGS2_6    \
  subi sp, 8;       \
  cfi_adjust_cfa_offset (8); \
  stw  r4, (sp, 0); \
  cfi_rel_offset (4, 0); \
  stw  r5, (sp, 4); \
  cfi_rel_offset (5, 0); \
  ldw  r4, (sp, 20); \
  ldw  r5, (sp, 24)

#undef  UNDOARGS2_0
#define UNDOARGS2_0 

#undef  UNDOARGS2_1
#define UNDOARGS2_1 UNDOARGS2_0
#undef  UNDOARGS2_2
#define UNDOARGS2_2 UNDOARGS2_0
#undef  UNDOARGS2_3
#define UNDOARGS2_3 UNDOARGS2_0
#undef  UNDOARGS2_4
#define UNDOARGS2_4 UNDOARGS2_0
#undef  UNDOARGS2_5
#define UNDOARGS2_5  \
  ldw  r4, (sp, 0); \
  addi sp, 8

#undef  UNDOARGS2_6
#define UNDOARGS2_6 \
  ldw  r4, (sp, 0); \
  ldw  r5, (sp, 4); \
  addi sp, 8

#endif  /* DO_CALL_2 */

#else /* not __ASSEMBLER__ */


/* Define a macro which expands into the inline wrapper code for a system
   call.  */
#undef INLINE_SYSCALL
#define INLINE_SYSCALL(name, nr, args...)                               \
  ({ unsigned int _inline_sys_result = INTERNAL_SYSCALL (name, , nr, args);     \
     if (__builtin_expect (INTERNAL_SYSCALL_ERROR_P (_inline_sys_result, ), 0)) \
       {                                                                \
         __set_errno (INTERNAL_SYSCALL_ERRNO (_inline_sys_result, ));           \
         _inline_sys_result = (unsigned int) -1;                                \
       }                                                                \
     (int) _inline_sys_result; })

#undef INTERNAL_SYSCALL_DECL
#define INTERNAL_SYSCALL_DECL(err) do { } while (0)

#undef INTERNAL_SYSCALL_ERROR_P
#define INTERNAL_SYSCALL_ERROR_P(val, err) \
  ((unsigned int) (val) >= 0xffffff01u)

#undef INTERNAL_SYSCALL_ERRNO
#define INTERNAL_SYSCALL_ERRNO(val, err)        (-(val))


#undef INTERNAL_SYSCALL_RAW
#ifndef __CSKYABIV2__
#define INTERNAL_SYSCALL_RAW0(name, err, dummy...)                      \
  ({unsigned int __sys_result;                                          \
     {                                                                  \
       register int _a1 __asm__ ("a0"), _nr __asm__ ("r1");             \
       _nr = name;                                                      \
       __asm__ __volatile__ ("trap  0 \n\t"                             \
                             : "=r" (_a1)                               \
                             : "r" (_nr)                                \
                             : "memory");                               \
               __sys_result = _a1;                                      \
     }                                                                  \
     (int) __sys_result; })

#define INTERNAL_SYSCALL_RAW1(name, err, arg1)                          \
  ({unsigned int __sys_result;                                          \
    register int _tmp_arg1 = (int)(arg1);                               \
     {                                                                  \
       register int _a1 __asm__ ("a0"), _nr __asm__ ("r1");             \
       _a1 = _tmp_arg1;                                                 \
       _nr = name;                                                      \
       __asm__ __volatile__ ("trap  0 \n\t"                             \
                             : "=r" (_a1)                               \
                             : "r" (_nr), "r" (_a1)                     \
                             : "memory");                               \
               __sys_result = _a1;                                      \
     }                                                                  \
     (int) __sys_result; })

#define INTERNAL_SYSCALL_RAW2(name, err, arg1, arg2)                    \
  ({unsigned int __sys_result;                                          \
    register int _tmp_arg1 = (int)(arg1), _tmp_arg2 = (int)(arg2);      \
     {                                                                  \
       register int _nr __asm__ ("r1");                                 \
       register int _a1 __asm__ ("a0"), _a2 __asm__ ("a1");             \
       _a1 = _tmp_arg1, _a2 = _tmp_arg2;                                \
       _nr = name;                                                      \
       __asm__ __volatile__ ("trap  0 \n\t"                             \
                             : "=r" (_a1)                               \
                             : "r" (_nr), "r" (_a1), "r" (_a2)          \
                             : "memory");                               \
               __sys_result = _a1;                                      \
     }                                                                  \
     (int) __sys_result; })

#define INTERNAL_SYSCALL_RAW3(name, err, arg1, arg2, arg3)              \
  ({unsigned int __sys_result;                                          \
    register int _tmp_arg1 = (int)(arg1), _tmp_arg2 = (int)(arg2);      \
    register int _tmp_arg3 = (int)(arg3);                               \
     {                                                                  \
       register int _nr __asm__ ("r1");                                 \
       register int _a1 __asm__ ("a0"), _a2 __asm__ ("a1");             \
       register int _a3 __asm__ ("a2");                                 \
       _a1 = _tmp_arg1;                                                 \
       _a2 = _tmp_arg2;                                                 \
       _a3 = _tmp_arg3;                                                 \
       _nr = name;                                                      \
       __asm__ __volatile__ ("trap  0 \n\t"                             \
                             : "=r" (_a1)                               \
                             : "r" (_nr), "r" (_a1), "r" (_a2),         \
                               "r" (_a3)                                \
                             : "memory");                               \
               __sys_result = _a1;                                      \
     }                                                                  \
     (int) __sys_result; })

#define INTERNAL_SYSCALL_RAW4(name, err, arg1, arg2, arg3, arg4)        \
  ({unsigned int __sys_result;                                          \
    register int _tmp_arg1 = (int)(arg1), _tmp_arg2 = (int)(arg2);      \
    register int _tmp_arg3 = (int)(arg3), _tmp_arg4 = (int)(arg4);      \
     {                                                                  \
       register int _nr __asm__ ("r1");                                 \
       register int _a1 __asm__ ("a0"), _a2 __asm__ ("a1");             \
       register int _a3 __asm__ ("a2"), _a4 __asm__ ("a3");             \
       _a1 = _tmp_arg1, _a2 = _tmp_arg2, _a3 = _tmp_arg3;               \
       _a4 = _tmp_arg4;                                                 \
       _nr = name;                                                      \
       __asm__ __volatile__ ("trap  0 \n\t"                             \
                             : "=r" (_a1)                               \
                             : "r" (_nr), "r" (_a1), "r" (_a2),         \
                               "r" (_a3), "r" (_a4)                     \
                             : "memory");                               \
               __sys_result = _a1;                                      \
     }                                                                  \
     (int) __sys_result; })

#define INTERNAL_SYSCALL_RAW5(name, err, arg1, arg2, arg3, arg4,        \
                              arg5)                                     \
  ({unsigned int __sys_result;                                          \
    register int _tmp_arg1 = (int)(arg1), _tmp_arg2 = (int)(arg2);      \
    register int _tmp_arg3 = (int)(arg3), _tmp_arg4 = (int)(arg4);      \
    register int _tmp_arg5 = (int)(arg5);                               \
     {                                                                  \
       register int _nr __asm__ ("r1");                                 \
       register int _a1 __asm__ ("a0"), _a2 __asm__ ("a1");             \
       register int _a3 __asm__ ("a2"), _a4 __asm__ ("a3");             \
       register int _a5 __asm__ ("a4");                                 \
       _a1 = _tmp_arg1, _a2 = _tmp_arg2, _a3 = _tmp_arg3;               \
       _a4 = _tmp_arg4, _a5 = _tmp_arg5;                                \
       _nr = name;                                                      \
       __asm__ __volatile__ ("trap  0 \n\t"                             \
                             : "=r" (_a1)                               \
                             : "r" (_nr), "r" (_a1), "r" (_a2),         \
                               "r" (_a3), "r" (_a4), "r" (_a5)          \
                             : "memory");                               \
               __sys_result = _a1;                                      \
     }                                                                  \
     (int) __sys_result; })

#define INTERNAL_SYSCALL_RAW6(name, err, arg1, arg2, arg3, arg4,        \
                              arg5, arg6)                               \
  ({unsigned int __sys_result;                                          \
    register int _tmp_arg1 = (int)(arg1), _tmp_arg2 = (int)(arg2);      \
    register int _tmp_arg3 = (int)(arg3), _tmp_arg4 = (int)(arg4);      \
    register int _tmp_arg5 = (int)(arg5), _tmp_arg6 = (int)(arg6);      \
     {                                                                  \
       register int _nr __asm__ ("r1");                                 \
       register int _a1 __asm__ ("a0"), _a2 __asm__ ("a1");             \
       register int _a3 __asm__ ("a2"), _a4 __asm__ ("a3");             \
       register int _a5 __asm__ ("a4"), _a6 __asm__ ("a5");             \
       _a1 = _tmp_arg1, _a2 = _tmp_arg2, _a3 = _tmp_arg3;               \
       _a4 = _tmp_arg4, _a5 = _tmp_arg5, _a6 = _tmp_arg6;               \
       _nr = name;                                                      \
       __asm__ __volatile__ ("trap  0 \n\t"                             \
                             : "=r" (_a1)                               \
                             : "r" (_nr), "r" (_a1), "r" (_a2),         \
                               "r" (_a3), "r" (_a4), "r" (_a5),         \
                               "r" (_a6)                                \
                             : "memory");                               \
               __sys_result = _a1;                                      \
     }                                                                  \
     (int) __sys_result; })

#define INTERNAL_SYSCALL_RAW7(name, err, arg1, arg2, arg3, arg4,        \
                              arg5, arg6, arg7)                         \
  ({unsigned int __sys_result;                                          \
    register int _tmp_arg1 = (int)(arg1), _tmp_arg2 = (int)(arg2);      \
    register int _tmp_arg3 = (int)(arg3), _tmp_arg4 = (int)(arg4);      \
    register int _tmp_arg5 = (int)(arg5), _tmp_arg6 = (int)(arg6);      \
    register int _tmp_arg7 = (int)(arg7);                               \
     {                                                                  \
       register int _nr __asm__ ("r1");                                 \
       register int _a1 __asm__ ("a0"), _a2 __asm__ ("a1");             \
       register int _a3 __asm__ ("a2"), _a4 __asm__ ("a3");             \
       register int _a5 __asm__ ("a4"), _a6 __asm__ ("a5");             \
       register int _a7 __asm__ ("r8");                                 \
       _a1 = _tmp_arg1, _a2 = _tmp_arg2, _a3 = _tmp_arg3;               \
       _a4 = _tmp_arg4, _a5 = _tmp_arg5, _a6 = _tmp_arg6;               \
       _a7 = _tmp_arg7;                                                 \
       _nr = name;                                                      \
       __asm__ __volatile__ ("trap  0 \n\t"                             \
                             : "=r" (_a1)                               \
                             : "r" (_nr), "r" (_a1), "r" (_a2),         \
                               "r" (_a3), "r" (_a4), "r" (_a5),         \
                               "r" (_a6), "r" (_a7)                     \
                             : "memory");                               \
               __sys_result = _a1;                                      \
     }                                                                  \
     (int) __sys_result; })

#else
#define INTERNAL_SYSCALL_RAW0(name, err, dummy...)                      \
  ({unsigned int __sys_result;                                          \
     {                                                                  \
       register int _a1 __asm__ ("a0"), _nr __asm__ ("r7");             \
       _nr = name;                                                      \
       __asm__ __volatile__ ("trap  0 \n\t"                             \
                             : "=r" (_a1)                               \
                             : "r" (_nr)                                \
                             : "memory");                               \
               __sys_result = _a1;                                      \
     }                                                                  \
     (int) __sys_result; })

#define INTERNAL_SYSCALL_RAW1(name, err, arg1)                          \
  ({unsigned int __sys_result;                                          \
    register int _tmp_arg1 = (int)(arg1);                               \
     {                                                                  \
       register int _a1 __asm__ ("a0"), _nr __asm__ ("r7");             \
       _a1 = _tmp_arg1;                                                 \
       _nr = name;                                                      \
       __asm__ __volatile__ ("trap  0 \n\t"                             \
                             : "=r" (_a1)                               \
                             : "r" (_nr), "r" (_a1)                     \
                             : "memory");                               \
               __sys_result = _a1;                                      \
     }                                                                  \
     (int) __sys_result; })

#define INTERNAL_SYSCALL_RAW2(name, err, arg1, arg2)                    \
  ({unsigned int __sys_result;                                          \
    register int _tmp_arg1 = (int)(arg1), _tmp_arg2 = (int)(arg2);      \
     {                                                                  \
       register int _nr __asm__ ("r7");                                 \
       register int _a1 __asm__ ("a0"), _a2 __asm__ ("a1");             \
       _a1 = _tmp_arg1, _a2 = _tmp_arg2;                                \
       _nr = name;                                                      \
       __asm__ __volatile__ ("trap  0 \n\t"                             \
                             : "=r" (_a1)                               \
                             : "r" (_nr), "r" (_a1), "r" (_a2)          \
                             : "memory");                               \
               __sys_result = _a1;                                      \
     }                                                                  \
     (int) __sys_result; })

#define INTERNAL_SYSCALL_RAW3(name, err, arg1, arg2, arg3)              \
  ({unsigned int __sys_result;                                          \
    register int _tmp_arg1 = (int)(arg1), _tmp_arg2 = (int)(arg2);      \
    register int _tmp_arg3 = (int)(arg3);                               \
     {                                                                  \
       register int _nr __asm__ ("r7");                                 \
       register int _a1 __asm__ ("a0"), _a2 __asm__ ("a1");             \
       register int _a3 __asm__ ("a2");                                 \
       _a1 = _tmp_arg1;                                                 \
       _a2 = _tmp_arg2;                                                 \
       _a3 = _tmp_arg3;                                                 \
       _nr = name;                                                      \
       __asm__ __volatile__ ("trap  0 \n\t"                             \
                             : "=r" (_a1)                               \
                             : "r" (_nr), "r" (_a1), "r" (_a2),         \
                               "r" (_a3)                                \
                             : "memory");                               \
               __sys_result = _a1;                                      \
     }                                                                  \
     (int) __sys_result; })

#define INTERNAL_SYSCALL_RAW4(name, err, arg1, arg2, arg3, arg4)        \
  ({unsigned int __sys_result;                                          \
    register int _tmp_arg1 = (int)(arg1), _tmp_arg2 = (int)(arg2);      \
    register int _tmp_arg3 = (int)(arg3), _tmp_arg4 = (int)(arg4);      \
     {                                                                  \
       register int _nr __asm__ ("r7");                                 \
       register int _a1 __asm__ ("a0"), _a2 __asm__ ("a1");             \
       register int _a3 __asm__ ("a2"), _a4 __asm__ ("a3");             \
       _a1 = _tmp_arg1, _a2 = _tmp_arg2, _a3 = _tmp_arg3;               \
       _a4 = _tmp_arg4;                                                 \
       _nr = name;                                                      \
       __asm__ __volatile__ ("trap  0 \n\t"                             \
                             : "=r" (_a1)                               \
                             : "r" (_nr), "r" (_a1), "r" (_a2),         \
                               "r" (_a3), "r" (_a4)                     \
                             : "memory");                               \
               __sys_result = _a1;                                      \
     }                                                                  \
     (int) __sys_result; })

#define INTERNAL_SYSCALL_RAW5(name, err, arg1, arg2, arg3, arg4,        \
                              arg5)                                     \
  ({unsigned int __sys_result;                                          \
    register int _tmp_arg1 = (int)(arg1), _tmp_arg2 = (int)(arg2);      \
    register int _tmp_arg3 = (int)(arg3), _tmp_arg4 = (int)(arg4);      \
    register int _tmp_arg5 = (int)(arg5);                               \
     {                                                                  \
       register int _nr __asm__ ("r7");                                 \
       register int _a1 __asm__ ("a0"), _a2 __asm__ ("a1");             \
       register int _a3 __asm__ ("a2"), _a4 __asm__ ("a3");             \
       register int _a5 __asm__ ("r4");                                 \
       _a1 = _tmp_arg1, _a2 = _tmp_arg2, _a3 = _tmp_arg3;               \
       _a4 = _tmp_arg4, _a5 = _tmp_arg5;                                \
       _nr = name;                                                      \
       __asm__ __volatile__ ("trap  0 \n\t"                             \
                             : "=r" (_a1)                               \
                             : "r" (_nr), "r" (_a1), "r" (_a2),         \
                               "r" (_a3), "r" (_a4), "r" (_a5)          \
                             : "memory");                               \
               __sys_result = _a1;                                      \
     }                                                                  \
     (int) __sys_result; })

#define INTERNAL_SYSCALL_RAW6(name, err, arg1, arg2, arg3, arg4,        \
                              arg5, arg6)                               \
  ({unsigned int __sys_result;                                          \
    register int _tmp_arg1 = (int)(arg1), _tmp_arg2 = (int)(arg2);      \
    register int _tmp_arg3 = (int)(arg3), _tmp_arg4 = (int)(arg4);      \
    register int _tmp_arg5 = (int)(arg5), _tmp_arg6 = (int)(arg6);      \
     {                                                                  \
       register int _nr __asm__ ("r7");                                 \
       register int _a1 __asm__ ("a0"), _a2 __asm__ ("a1");             \
       register int _a3 __asm__ ("a2"), _a4 __asm__ ("a3");             \
       register int _a5 __asm__ ("r4"), _a6 __asm__ ("r5");             \
       _a1 = _tmp_arg1, _a2 = _tmp_arg2, _a3 = _tmp_arg3;               \
       _a4 = _tmp_arg4, _a5 = _tmp_arg5, _a6 = _tmp_arg6;               \
       _nr = name;                                                      \
       __asm__ __volatile__ ("trap  0 \n\t"                             \
                             : "=r" (_a1)                               \
                             : "r" (_nr), "r" (_a1), "r" (_a2),         \
                               "r" (_a3), "r" (_a4), "r" (_a5),         \
                               "r" (_a6)                                \
                             : "memory");                               \
               __sys_result = _a1;                                      \
     }                                                                  \
     (int) __sys_result; })

#define INTERNAL_SYSCALL_RAW7(name, err, arg1, arg2, arg3, arg4,        \
                              arg5, arg6, arg7)                         \
  ({unsigned int __sys_result;                                          \
    register int _tmp_arg1 = (int)(arg1), _tmp_arg2 = (int)(arg2);      \
    register int _tmp_arg3 = (int)(arg3), _tmp_arg4 = (int)(arg4);      \
    register int _tmp_arg5 = (int)(arg5), _tmp_arg6 = (int)(arg6);      \
    register int _tmp_arg7 = (int)(arg7);                               \
     {                                                                  \
       register int _nr __asm__ ("r7");                                 \
       register int _a1 __asm__ ("a0"), _a2 __asm__ ("a1");             \
       register int _a3 __asm__ ("a2"), _a4 __asm__ ("a3");             \
       register int _a5 __asm__ ("r4"), _a6 __asm__ ("r5");             \
       register int _a7 __asm__ ("r6");                                 \
       _a1 = _tmp_arg1, _a2 = _tmp_arg2, _a3 = _tmp_arg3;               \
       _a4 = _tmp_arg4, _a5 = _tmp_arg5, _a6 = _tmp_arg6;               \
       _a7 = _tmp_arg7;                                                 \
       _nr = name;                                                      \
       __asm__ __volatile__ ("trap  0 \n\t"                             \
                             : "=r" (_a1)                               \
                             : "r" (_nr), "r" (_a1), "r" (_a2),         \
                               "r" (_a3), "r" (_a4), "r" (_a5),         \
                               "r" (_a6), "r" (_a7)                     \
                             : "memory");                               \
               __sys_result = _a1;                                      \
     }                                                                  \
     (int) __sys_result; })

#endif /* __ABI_CSKY_V2__ */

#undef INTERNAL_SYSCALL
#define INTERNAL_SYSCALL(name, err, nr, args...)                \
        INTERNAL_SYSCALL_RAW##nr(SYS_ify(name), err, args)

#undef INTERNAL_SYSCALL_NCS
#define INTERNAL_SYSCALL_NCS(number, err, nr, args...)          \
  INTERNAL_SYSCALL_RAW##nr (number, err, args)

#endif	/* __ASSEMBLER__ */

/* Pointer mangling is not yet supported for CSKY.  */
#define PTR_MANGLE(var) (void) (var)
#define PTR_DEMANGLE(var) (void) (var)

#endif /* linux/csky/sysdep.h */
