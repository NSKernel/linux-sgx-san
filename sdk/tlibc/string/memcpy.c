/*	$OpenBSD: memcpy.c,v 1.2 2015/08/31 02:53:57 guenther Exp $ */
/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

/*
 * sizeof(word) MUST BE A POWER OF TWO
 * SO THAT wmask BELOW IS ALL ONES
 */
typedef	long word;		/* "word" used for optimal copy speed */

#define	wsize	sizeof(word)
#define	wmask	(wsize - 1)

#ifdef _TLIBC_USE_INTEL_FAST_STRING_
extern void *_intel_fast_memcpy(void *, void *, size_t);
#endif

#define __memcpy __unsan_memcpy

/*
 * Copy a block of memory, not handling overlap.
 */
void *
__unsan_memcpy(void *dst0, const void *src0, size_t length)
{
	char *dst = (char *)dst0;
	const char *src = (const char *)src0;
	size_t t;

	if (length == 0 || dst == src)		/* nothing to do */
		goto done;

	if ((dst < src && dst + length > src) ||
	    (src < dst && src + length > dst)) {
        /* backwards memcpy */
		abort();
	}

	/*
	 * Macros: loop-t-times; and loop-t-times, t>0
	 */
#define	TLOOP(s) if (t) TLOOP1(s)
#define	TLOOP1(s) do { s; } while (--t)

	/*
	 * Copy forward.
	 */
	t = (long)src;	/* only need low bits */
	if ((t | (long)dst) & wmask) {
		/*
		 * Try to align operands.  This cannot be done
		 * unless the low bits match.
		 */
		if ((t ^ (long)dst) & wmask || length < wsize)
			t = length;
		else
			t = wsize - (t & wmask);
		length -= t;
		TLOOP1(*dst++ = *src++);
	}
	/*
	 * Copy whole words, then mop up any trailing bytes.
	 */
	t = length / wsize;
	TLOOP(*(word *)dst = *(word *)src; src += wsize; dst += wsize);
	t = length & wmask;
	TLOOP(*dst++ = *src++);
done:
	return (dst0);
}


void *
memcpy(void *dst0, const void *src0, size_t length)
{
#ifdef _TLIBC_USE_INTEL_FAST_STRING_
 	return _intel_fast_memcpy(dst0, (void*)src0, length);
#else
	return __memcpy(dst0, src0, length);
#endif
}
