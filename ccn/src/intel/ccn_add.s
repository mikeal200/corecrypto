# Copyright (c) 2011,2015,2016,2017,2018 Apple Inc. All rights reserved.
#
# corecrypto Internal Use License Agreement
#
# IMPORTANT:  This Apple corecrypto software is supplied to you by Apple Inc. ("Apple")
# in consideration of your agreement to the following terms, and your download or use
# of this Apple software constitutes acceptance of these terms.  If you do not agree
# with these terms, please do not download or use this Apple software.
#
# 1.    As used in this Agreement, the term "Apple Software" collectively means and
# includes all of the Apple corecrypto materials provided by Apple here, including
# but not limited to the Apple corecrypto software, frameworks, libraries, documentation
# and other Apple-created materials. In consideration of your agreement to abide by the
# following terms, conditioned upon your compliance with these terms and subject to
# these terms, Apple grants you, for a period of ninety (90) days from the date you
# download the Apple Software, a limited, non-exclusive, non-sublicensable license
# under Apple’s copyrights in the Apple Software to make a reasonable number of copies
# of, compile, and run the Apple Software internally within your organization only on
# devices and computers you own or control, for the sole purpose of verifying the
# security characteristics and correct functioning of the Apple Software; provided
# that you must retain this notice and the following text and disclaimers in all
# copies of the Apple Software that you make. You may not, directly or indirectly,
# redistribute the Apple Software or any portions thereof. The Apple Software is only
# licensed and intended for use as expressly stated above and may not be used for other
# purposes or in other contexts without Apple's prior written permission.  Except as
# expressly stated in this notice, no other rights or licenses, express or implied, are
# granted by Apple herein.
#
# 2.    The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
# WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES
# OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING
# THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS,
# SYSTEMS, OR SERVICES. APPLE DOES NOT WARRANT THAT THE APPLE SOFTWARE WILL MEET YOUR
# REQUIREMENTS, THAT THE OPERATION OF THE APPLE SOFTWARE WILL BE UNINTERRUPTED OR
# ERROR-FREE, THAT DEFECTS IN THE APPLE SOFTWARE WILL BE CORRECTED, OR THAT THE APPLE
# SOFTWARE WILL BE COMPATIBLE WITH FUTURE APPLE PRODUCTS, SOFTWARE OR SERVICES. NO ORAL
# OR WRITTEN INFORMATION OR ADVICE GIVEN BY APPLE OR AN APPLE AUTHORIZED REPRESENTATIVE
# WILL CREATE A WARRANTY.
#
# 3.    IN NO EVENT SHALL APPLE BE LIABLE FOR ANY DIRECT, SPECIAL, INDIRECT, INCIDENTAL
# OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING
# IN ANY WAY OUT OF THE USE, REPRODUCTION, COMPILATION OR OPERATION OF THE APPLE
# SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING
# NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
# 4.    This Agreement is effective until terminated. Your rights under this Agreement will
# terminate automatically without notice from Apple if you fail to comply with any term(s)
# of this Agreement.  Upon termination, you agree to cease all use of the Apple Software
# and destroy all copies, full or partial, of the Apple Software. This Agreement will be
# governed and construed in accordance with the laws of the State of California, without
# regard to its choice of law rules.
#
# You may report security issues about Apple products to product-security@apple.com,
# as described here:  https://www.apple.com/support/security/.  Non-security bugs and
# enhancement requests can be made via https://bugreport.apple.com as described
# here: https://developer.apple.com/bug-reporting/
#
# EA1350
# 10/5/15

#include <corecrypto/cc_config.h>


#if (defined(__x86_64__) || defined(__i386__)) && CCN_ADD_ASM

        .globl  _ccn_add
        .align  4, 0x90
_ccn_add:

#ifdef	__x86_64__

		// push rbp and set up frame base
        pushq   %rbp
        movq    %rsp, %rbp

		// symbolicate used registers

		#define	size	%rdi			// size in cc_unit (8-bytes)
		#define	r		%rsi			// destination
		#define	s		%rdx			// source 1
		#define	t		%rcx			// source 2

		#define	o		%rax
		#define	i		%r8

		xor		o, o					// used as a potential output (carry = 0 for size=0), and also sahf for an initial carry = 0

		// macro for adding 2 quad-words
		.macro	myadc
        movq    $0(t, i, 8), o			// 2nd source
        adc		$0(s, i, 8), o			// add with carry 1st source
		movq	o, $0(r, i, 8)			// save to destination
		.endm

		xor		i, i					// address offset and also loop counter

		subq	$4, size
		jl		2f						// if size < 4, skip the following code that processes 4 blocks/iteration

0:

		sahf							// load Carry from ah
		myadc	0
		myadc	8
		myadc	16
		myadc	24
		lahf							// save Carry in ah
		add		$4, i					// i+=4;
		subq	$4, size					// size vs i
		jge		0b						// repeat if size > i

2:		testq	$2, size
		je		1f						// if size < 2, skip the following code that process 2 blocks
		sahf							// load Carry from ah
		myadc	0
		myadc	8
		lahf							// save Carry in ah
		add		$2, i					// i+=4;

1:		testq	$1, size
		je		3f
		sahf							// load Carry from ah
		myadc	0
		lahf							// save Carry in ah
3:

		xor     i, i
		sahf
		adc		$0, i // to return the final carry signal
		mov     i, o

9:
        popq    %rbp
        ret

#else		// i386

		// set up frame and push save/restore registers
		push	%ebp
		mov		%esp, %ebp
		push    %ebx
	    push    %esi
		push    %edi

		// symbolicate registers
		#define	size	%edi
		#define	r		%esi
		#define	s		%edx
		#define	t		%ecx
		#define	o		%eax
		#define i		%ebx

		movl	8(%ebp), size
		movl	12(%ebp), r
		movl	16(%ebp), s
		movl	20(%ebp), t

		xor		o, o				// used as a potential output (carry = 0 for size=0), and also sahf for an initial carry = 0

		cmp		$0, size
		jle		9f					// early exit should size <= 0

		// macro for add with carry for 2 4-byte words
		.macro myadc
		movl	$0(s, i, 4), o
		adc		$0(t, i, 4), o
		movl	o, $0(r, i, 4)
		.endm

		xor		i, i				// 4-byte index

		subl	$8, size
		jl		4f					// if size < 8, skip the code that processes 8 blocks/iteration
0:
		sahf
		myadc	0
		myadc	4
		myadc	8
		myadc	12
		myadc	16
		myadc	20
		myadc	24
		myadc	28
		lahf
		addl	$8, i
		subl	$8, size
		jge		0b

4:
		testl	$4, size
		je		2f					// if size < 4, skip the code that processes the remaining 4 blocks
		sahf
		myadc	0
		myadc	4
		myadc	8
		myadc	12
		lahf
		addl	$4, i

2:
		testl	$2, size
		je		1f					// if size < 2, skip the code that processes the remaining 2 blocks
		sahf
		myadc	0
		myadc	4
		lahf
		addl	$2, i

1:
		testl	$1, size
		je		3f					// if size < 1, skip the code that processes the remaining 1 block
		sahf
		myadc	0
		lahf
3:

		xor     i, i
		sahf
		adc		$0, i // to return the final carry signal
		mov     i, o

9:
		pop		%edi
		pop		%esi
		pop		%ebx
		pop		%ebp
		ret

#endif
#endif

