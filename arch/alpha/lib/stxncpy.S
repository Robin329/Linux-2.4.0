/*
 * arch/alpha/lib/stxncpy.S
 * Contributed by Richard Henderson (rth@tamu.edu)
 *
 * Copy no more than COUNT bytes of the null-terminated string from
 * SRC to DST.
 *
 * This is an internal routine used by strncpy, stpncpy, and strncat.
 * As such, it uses special linkage conventions to make implementation
 * of these public functions more efficient.
 *
 * On input:
 *	t9 = return address
 *	a0 = DST
 *	a1 = SRC
 *	a2 = COUNT
 *
 * Furthermore, COUNT may not be zero.
 *
 * On output:
 *	t0  = last word written
 *	t10 = bitmask (with one bit set) indicating the byte position of
 *	      the end of the range specified by COUNT
 *	t12 = bitmask (with one bit set) indicating the last byte written
 *	a0  = unaligned address of the last *word* written
 *	a2  = the number of full words left in COUNT
 *
 * Furthermore, v0, a3-a5, t11, t12, and $at are untouched.
 */

#include <alpha/regdef.h>

	.set noat
	.set noreorder

	.text

/* There is a problem with either gdb (as of 4.16) or gas (as of 2.7) that
   doesn't like putting the entry point for a procedure somewhere in the
   middle of the procedure descriptor.  Work around this by putting the
   aligned copy in its own procedure descriptor */

	.ent stxncpy_aligned
	.align 3
stxncpy_aligned:
	.frame sp, 0, t9, 0
	.prologue 0

	/* On entry to this basic block:
	   t0 == the first destination word for masking back in
	   t1 == the first source word.  */

	/* Create the 1st output word and detect 0's in the 1st input word.  */
	lda	t2, -1		# e1    : build a mask against false zero
	mskqh	t2, a1, t2	# e0    :   detection in the src word
	mskqh	t1, a1, t3	# e0    :
	ornot	t1, t2, t2	# .. e1 :
	mskql	t0, a1, t0	# e0    : assemble the first output word
	cmpbge	zero, t2, t8	# .. e1 : bits set iff null found
	or	t0, t3, t0	# e0    :
	beq	a2, $a_eoc	# .. e1 :
	bne	t8, $a_eos	# .. e1 :

	/* On entry to this basic block:
	   t0 == a source word not containing a null.  */

$a_loop:
	stq_u	t0, 0(a0)	# e0    :
	addq	a0, 8, a0	# .. e1 :
	ldq_u	t0, 0(a1)	# e0    :
	addq	a1, 8, a1	# .. e1 :
	subq	a2, 1, a2	# e0    :
	cmpbge	zero, t0, t8	# .. e1 (stall)
	beq	a2, $a_eoc      # e1    :
	beq	t8, $a_loop	# e1    :

	/* Take care of the final (partial) word store.  At this point
	   the end-of-count bit is set in t8 iff it applies.

	   On entry to this basic block we have:
	   t0 == the source word containing the null
	   t8 == the cmpbge mask that found it.  */

$a_eos:
	negq	t8, t12		# e0    : find low bit set
	and	t8, t12, t12	# e1 (stall)

	/* For the sake of the cache, don't read a destination word
	   if we're not going to need it.  */
	and	t12, 0x80, t6	# e0    :
	bne	t6, 1f		# .. e1 (zdb)

	/* We're doing a partial word store and so need to combine
	   our source and original destination words.  */
	ldq_u	t1, 0(a0)	# e0    :
	subq	t12, 1, t6	# .. e1 :
	or	t12, t6, t8	# e0    :
	unop			#
	zapnot	t0, t8, t0	# e0    : clear src bytes > null
	zap	t1, t8, t1	# .. e1 : clear dst bytes <= null
	or	t0, t1, t0	# e1    :

1:	stq_u	t0, 0(a0)	# e0    :
	ret	(t9)		# e1    :

	/* Add the end-of-count bit to the eos detection bitmask.  */
$a_eoc:
	or	t10, t8, t8
	br	$a_eos

	.end stxncpy_aligned

	.align 3
	.ent __stxncpy
	.globl __stxncpy
__stxncpy:
	.frame sp, 0, t9, 0
	.prologue 0

	/* Are source and destination co-aligned?  */
	xor	a0, a1, t1	# e0    :
	and	a0, 7, t0	# .. e1 : find dest misalignment
	and	t1, 7, t1	# e0    :
	addq	a2, t0, a2	# .. e1 : bias count by dest misalignment
	subq	a2, 1, a2	# e0    :
	and	a2, 7, t2	# e1    :
	srl	a2, 3, a2	# e0    : a2 = loop counter = (count - 1)/8
	addq	zero, 1, t10	# .. e1 :
	sll	t10, t2, t10	# e0    : t10 = bitmask of last count byte
	bne	t1, $unaligned	# .. e1 :

	/* We are co-aligned; take care of a partial first word.  */

	ldq_u	t1, 0(a1)	# e0    : load first src word
	addq	a1, 8, a1	# .. e1 :

	beq	t0, stxncpy_aligned     # avoid loading dest word if not needed
	ldq_u	t0, 0(a0)	# e0    :
	br	stxncpy_aligned	# .. e1 :


/* The source and destination are not co-aligned.  Align the destination
   and cope.  We have to be very careful about not reading too much and
   causing a SEGV.  */

	.align 3
$u_head:
	/* We know just enough now to be able to assemble the first
	   full source word.  We can still find a zero at the end of it
	   that prevents us from outputting the whole thing.

	   On entry to this basic block:
	   t0 == the first dest word, unmasked
	   t1 == the shifted low bits of the first source word
	   t6 == bytemask that is -1 in dest word bytes */

	ldq_u	t2, 8(a1)	# e0    : load second src word
	addq	a1, 8, a1	# .. e1 :
	mskql	t0, a0, t0	# e0    : mask trailing garbage in dst
	extqh	t2, a1, t4	# e0    :
	or	t1, t4, t1	# e1    : first aligned src word complete
	mskqh	t1, a0, t1	# e0    : mask leading garbage in src
	or	t0, t1, t0	# e0    : first output word complete
	or	t0, t6, t6	# e1    : mask original data for zero test
	cmpbge	zero, t6, t8	# e0    :
	beq	a2, $u_eocfin	# .. e1 :
	bne	t8, $u_final	# e1    :

	lda	t6, -1			# e1    : mask out the bits we have
	mskql	t6, a1, t6		# e0    :   already seen
	stq_u	t0, 0(a0)		# e0    : store first output word
	or      t6, t2, t2		# .. e1 :
	cmpbge	zero, t2, t8		# e0    : find nulls in second partial
	addq	a0, 8, a0		# .. e1 :
	subq	a2, 1, a2		# e0    :
	bne	t8, $u_late_head_exit	# .. e1 :

	/* Finally, we've got all the stupid leading edge cases taken care
	   of and we can set up to enter the main loop.  */

	extql	t2, a1, t1	# e0    : position hi-bits of lo word
	ldq_u	t2, 8(a1)	# .. e1 : read next high-order source word
	addq	a1, 8, a1	# e0    :
	cmpbge	zero, t2, t8	# e1 (stall)
	beq	a2, $u_eoc	# e1    :
	bne	t8, $u_eos	# e1    :

	/* Unaligned copy main loop.  In order to avoid reading too much,
	   the loop is structured to detect zeros in aligned source words.
	   This has, unfortunately, effectively pulled half of a loop
	   iteration out into the head and half into the tail, but it does
	   prevent nastiness from accumulating in the very thing we want
	   to run as fast as possible.

	   On entry to this basic block:
	   t1 == the shifted high-order bits from the previous source word
	   t2 == the unshifted current source word

	   We further know that t2 does not contain a null terminator.  */

	.align 3
$u_loop:
	extqh	t2, a1, t0	# e0    : extract high bits for current word
	addq	a1, 8, a1	# .. e1 :
	extql	t2, a1, t3	# e0    : extract low bits for next time
	addq	a0, 8, a0	# .. e1 :
	or	t0, t1, t0	# e0    : current dst word now complete
	ldq_u	t2, 0(a1)	# .. e1 : load high word for next time
	stq_u	t0, -8(a0)	# e0    : save the current word
	mov	t3, t1		# .. e1 :
	subq	a2, 1, a2	# e0    :
	cmpbge	zero, t2, t8	# .. e1 : test new word for eos
	beq	a2, $u_eoc	# e1    :
	beq	t8, $u_loop	# e1    :

	/* We've found a zero somewhere in the source word we just read.
	   If it resides in the lower half, we have one (probably partial)
	   word to write out, and if it resides in the upper half, we
	   have one full and one partial word left to write out.

	   On entry to this basic block:
	   t1 == the shifted high-order bits from the previous source word
	   t2 == the unshifted current source word.  */
$u_eos:
	extqh	t2, a1, t0	# e0    :
	or	t0, t1, t0	# e1    : first (partial) source word complete

	cmpbge	zero, t0, t8	# e0    : is the null in this first bit?
	bne	t8, $u_final	# .. e1 (zdb)

	stq_u	t0, 0(a0)	# e0    : the null was in the high-order bits
	addq	a0, 8, a0	# .. e1 :
	subq	a2, 1, a2	# e1    :

$u_late_head_exit:
	extql	t2, a1, t0	# .. e0 :
	cmpbge	zero, t0, t8	# e0    :
	or	t8, t10, t6	# e1    :
	cmoveq	a2, t6, t8	# e0    :
	nop			# .. e1 :

	/* Take care of a final (probably partial) result word.
	   On entry to this basic block:
	   t0 == assembled source word
	   t8 == cmpbge mask that found the null.  */
$u_final:
	negq	t8, t6		# e0    : isolate low bit set
	and	t6, t8, t12	# e1    :

	and	t12, 0x80, t6	# e0    : avoid dest word load if we can
	bne	t6, 1f		# .. e1 (zdb)

	ldq_u	t1, 0(a0)	# e0    :
	subq	t12, 1, t6	# .. e1 :
	or	t6, t12, t8	# e0    :
	zapnot	t0, t8, t0	# .. e1 : kill source bytes > null
	zap	t1, t8, t1	# e0    : kill dest bytes <= null
	or	t0, t1, t0	# e1    :

1:	stq_u	t0, 0(a0)	# e0    :
	ret	(t9)		# .. e1 :

$u_eoc:				# end-of-count
	extqh	t2, a1, t0
	or	t0, t1, t0
	cmpbge	zero, t0, t8

$u_eocfin:			# end-of-count, final word
	or	t10, t8, t8
	br	$u_final

	/* Unaligned copy entry point.  */
	.align 3
$unaligned:

	ldq_u	t1, 0(a1)	# e0    : load first source word

	and	a0, 7, t4	# .. e1 : find dest misalignment
	and	a1, 7, t5	# e0    : find src misalignment

	/* Conditionally load the first destination word and a bytemask
	   with 0xff indicating that the destination byte is sacrosanct.  */

	mov	zero, t0	# .. e1 :
	mov	zero, t6	# e0    :
	beq	t4, 1f		# .. e1 :
	ldq_u	t0, 0(a0)	# e0    :
	lda	t6, -1		# .. e1 :
	mskql	t6, a0, t6	# e0    :
1:
	subq	a1, t4, a1	# .. e1 : sub dest misalignment from src addr

	/* If source misalignment is larger than dest misalignment, we need
	   extra startup checks to avoid SEGV.  */

	cmplt	t4, t5, t12	# e1    :
	extql	t1, a1, t1	# .. e0 : shift src into place
	lda	t2, -1		# e0    : for creating masks later
	beq	t12, $u_head	# e1    :

	mskqh	t2, t5, t2	# e0    : begin src byte validity mask
	cmpbge	zero, t1, t8	# .. e1 : is there a zero?
	extql	t2, a1, t2	# e0    :
	or	t8, t10, t5	# .. e1 : test for end-of-count too
	cmpbge	zero, t2, t3	# e0    :
	cmoveq	a2, t5, t8	# .. e1 :
	andnot	t8, t3, t8	# e0    :
	beq	t8, $u_head	# .. e1 (zdb)

	/* At this point we've found a zero in the first partial word of
	   the source.  We need to isolate the valid source data and mask
	   it into the original destination data.  (Incidentally, we know
	   that we'll need at least one byte of that original dest word.) */

	ldq_u	t0, 0(a0)	# e0    :
	negq	t8, t6		# .. e1 : build bitmask of bytes <= zero
	mskqh	t1, t4, t1	# e0    :
	and	t6, t8, t12	# .. e1 :
	subq	t12, 1, t6	# e0    :
	or	t6, t12, t8	# e1    :

	zapnot	t2, t8, t2	# e0    : prepare source word; mirror changes
	zapnot	t1, t8, t1	# .. e1 : to source validity mask

	andnot	t0, t2, t0	# e0    : zero place for source to reside
	or	t0, t1, t0	# e1    : and put it there
	stq_u	t0, 0(a0)	# e0    :
	ret	(t9)		# .. e1 :

	.end __stxncpy
