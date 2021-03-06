/* Copyright (C) 1992, 1995, 1997, 2000, 2009 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Brendan Kehoe (brendan@zen.org).

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

#include <sysdep.h>

ENTRY(__longjmp)
	mov     a2, a0
	mov     a0, a1		/* get the return value in place */
	cmpnei  a0,	 0
	bt      have_return
	movi    a0, 1		/* can't let setjmp() return zero! */
have_return:
#ifdef CHECK_SP
	ldw     r10, (a2, 0)	/* jmpbuf's sp */
//	cfi_undefined (r10)
	CHECK_SP(r10)
#endif
#ifdef __CSKYABIV2__
	ldw     sp, (a2, 0)
	ldw     lr, (a2, 4)
	ldw     r4, (a2, 8)
	ldw     r5, (a2, 12)
	ldw     r6, (a2, 16)
	ldw     r7, (a2, 20)
	ldw     r8, (a2, 24)
	ldw     r9, (a2, 28)
	ldw     r10, (a2, 32)
	ldw     r11, (a2, 36)
	ldw     r16, (a2, 40)
	ldw     r17, (a2, 44)
	ldw     r26, (a2, 48)
	ldw     r27, (a2, 52)
	ldw     r28, (a2, 56)
	ldw     r29, (a2, 60)
	ldw     r30, (a2, 64)
	ldw     r31, (a2, 68)
	addi    a2, 72
#else
	ldw     sp, (a2, 0)
	ldw     lr, (a2, 4)
	ldw     r8, (a2, 8)
	ldw     r9, (a2, 12)
	ldw     r10, (a2, 16)
	ldw     r11, (a2, 20)
	ldw     r12, (a2, 24)
	ldw     r13, (a2, 28)
	ldw     r14, (a2, 32)
	addi    a2, 32
	addi	a2, 4
#endif

//	cfi_restore (sp)
//	cfi_restore (a0)
//	cfi_restore (a1)
//	cfi_restore (a2)
//	cfi_restore (a3)
//	cfi_restore (a4)
//	cfi_restore (a5)
//	cfi_restore (fp)
//	cfi_restore (lr)


	jmp lr

END(__longjmp)
