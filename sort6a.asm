;*******************************************************************
;   sorts.cpp
;   Sorting Networks
;
;	Author: Kareem Omar
;	kareem.omar@uah.edu
;	https://github.com/komrad36
;
;	Last updated Dec 10, 2016
;******************************************************************/
;
; SUMMARY: I present novel and state-of-the-art sorting of 4 int32_t
; and sorting of 6 int8_t
; using SSE4. These are the fastest known approaches on modern CPUs.
;
; 4 int32_t can be sorted in 1.4 nanoseconds.
; 6 int8_t can be sorted in 2.4 nanoseconds.
;
; I also provide optimal assembly snippets for sorting arrays of int32_t with
; 2-6 elements and POSSIBLE C++ equivalents that could compile to optimal assembly
; under SOME compilers and conditions but OFTEN DO NOT (test your environment!).
;
; Conventional sorting algorithms like quicksort and mergesort
; are not great at small array sizes. Simpler sorts like insertion
; and selection tend to win. HOWEVER, even these are not NEARLY
; as fast as they could be for fixed-size, small arrays.
;
; In this case a sorting network is required. This application is
; indended to show the desired ideal amd64 assembly of such a sorting
; network for array sizes between 2 and 6, inclusive, and so I provide
; assembly snippets in MASM for each of these cases.
;
; Note that these use the Windows x64 calling convention, and then
; proceed to clobber a bunch of registers they shouldn't. Normally
; they'd be inlined. Feel free to callee-save registers that your
; application cannot safely clobber.
;
; Ideally, of course, your compiler is smart enough to generate assembly
; this good for you from a C++ sorting network. But it's not immediately
; obvious what sort of C++ to write to convince the compiler to do so.
;
; Suggestions for C++ implementations of sorting networks for sizes
; of 2 and 6 are thus provided.
;
; NOTE THAT MOST COMPILERS STILL FAIL TO PRODUCE OPTIMAL ASSEMBLY!!
; Therefore the point of these snippets is to be TESTED ON YOUR
; ENVIRONMENT, and the generated assembly to be COMPARED TO THE
; ASSEMBLY SNIPPETS to see whether the compiler succeeded.
;
; Clang seems to do a good job, although it's too afraid of using
; amd64 GPRs.
;
; MSVC fails miserably for sizes above about 3.
;
; g++ is very picky and sometimes does, sometimes doesn't, depending
; on compiler options and version. g++ doesn't like using GPRs 
; much, nor does it like conditional moves - which ARE FASTER 
; for this application on modern CPUs.
;
; However, the best results for int32_t sorting, that NO compiler will
; produce from anything other than carefully crafted SIMD intrinsics,
; are obtained using SSE... as long as your array size
; is 4 or less. This is by FAR the fastest option.
;
; As an example, my simdsort4() function uses only SSE4, no AVX,
; to accomplish a 4-sort in just a few cycles
; (1.40 nanoseconds on my machine (!)).
;
; NOTE that I provide two different implementations of simdsort4(),
; one using all integer instructions but two extra logical operations,
; and one using fewer logical operations but with a float reinterpretation.
; Which is faster will depend on your architecture; comment out the current
; one and uncomment the other to try it. It's important to only leave one
; uncommented at a time, as the extra data can actually affect timings
; significantly.
;
; Similarly, the best results for int8_t sorting, as long as your array
; size is 16 or less, by FAR, are obtained using SSE.
;
; As an example, my simdsort6() function uses only SSE4, no AVX,
; to accomplish a 6-sort of 8-bit signed integers in just a few cycles
; (2.40 nanoseconds on my machine (!)).
;
; If you want to experiment with timings,
; be sure to disable link-time optimization,
; a.k.a whole-program optimization (MSVC),
; a.k.a -flto (g++) or otherwise ensure that
; the sort functions don't get optimized away
; if you want accurate timings for testing.
;
; If, however, you just want to steal the sort functions,
; obviously you can and should use LTO.
;
; Available sorts and their function names:
;
; >>> SSE (fast as hell. FASTEST option on modern CPUs):
; Sorting 4 int32_t  |  simdsort4()
; Sorting 6 int8_t   |  simdsort6()
;
; >>> Assembly (these are written in MASM for Win64;
; but it's Intel syntax and you can make the small
; modifications required for other assemblers. These
; are mainly intended as targets to compare compiler output
; to, in order to ensure that your compiler is generating
; the right thing from a C++ sort function like that of sort2()
; or sort6(). Note that MOST COMPILERS DO THIS VERY POORLY.
; Sorting 2 int32_t  |  sort2a()
; Sorting 3 int32_t  |  sort3a()
; Sorting 4 int32_t  |  sort4a()
; Sorting 5 int32_t  |  sort5a()
; Sorting 6 int32_t  |  sort6a()
;
; >>> C++ (MAKE SURE YOUR COMPILER SUCCESSFULLY GENERATES
; THE DESIRED ASSEMBLY by comparing it to the assembly snippets!)
; Sorting 2 int32_t  |  sort2()
; Sorting 6 int32_t  |  sort6()
;

.CODE

sort6a PROC
	mov		edx, [rcx]
	mov		r11d, [rcx+4]
	mov		r15d, [rcx+8]
	cmp		r11d, r15d
	mov		r12d, r15d
	cmovl	r12d, r11d
	cmovl	r11d, r15d
	cmp		edx, r11d
	mov		r13d, r11d
	cmovl	r13d, edx
	cmovg	r11d, edx
	cmp		r13d, r12d
	mov		r8d, r12d
	cmovl	r8d, r13d
	cmovl	r13d, r12d
	mov		r12d, [rcx+16]
	mov		r9d, [rcx+20]
	cmp		r12d, r9d
	mov		r10d, r9d
	cmovl	r10d, r12d
	cmovl	r12d, r9d
	mov		r9d, [rcx+12]
	cmp		r9d, r12d
	mov		edx, r12d
	cmovl	edx, r9d
	cmovg	r12d, r9d
	cmp		edx, r10d
	mov		eax, r10d
	cmovl	eax, edx
	cmovl	edx, r10d
	cmp		r8d, eax
	mov		r9d, eax
	cmovl	r9d, r8d
	cmovg	eax, r8d
	mov		[rcx], r9d
	cmp		r13d, edx
	mov		r15d, edx
	cmovl	r15d, r13d
	cmovg	edx, r13d
	cmp		r11d, r12d
	mov		r13d, r12d
	cmovl	r13d, r11d
	cmovg	r12d, r11d
	mov		[rcx+20], r12d
	cmp		r13d, edx
	mov		r12d, edx
	cmovl	r12d, r13d
	cmovl	r13d, edx
	mov		[rcx+16], r13d
	cmp		r15d, eax
	mov		edx, eax
	cmovl	edx, r15d
	cmovl	r15d, eax
	mov		[rcx+4], edx
	cmp		r12d, r15d
	mov		eax, r15d
	cmovl	eax, r12d
	cmovg	r15d, r12d
	mov		[rcx+8], eax
	mov		[rcx+12], r15d
	ret
sort6a ENDP
END