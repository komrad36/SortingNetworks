/*******************************************************************
*   sorts.cpp
*   Sorting Networks
*
*	Author: Kareem Omar
*	kareem.omar@uah.edu
*	https://github.com/komrad36
*
*	Last updated Dec 10, 2016
*******************************************************************/
//
// SUMMARY: I present novel and state-of-the-art sorting of 4 int32_t
// and sorting of 6 int8_t
// using SSE4. These are the fastest known approaches on modern CPUs.
//
// 4 int32_t can be sorted in 1.4 nanoseconds.
// 6 int8_t can be sorted in 2.4 nanoseconds.
//
// I also provide optimal assembly snippets for sorting arrays of int32_t with
// 2-6 elements and POSSIBLE C++ equivalents that could compile to optimal assembly
// under SOME compilers and conditions but OFTEN DO NOT (test your environment!).
//
// Conventional sorting algorithms like quicksort and mergesort
// are not great at small array sizes. Simpler sorts like insertion
// and selection tend to win. HOWEVER, even these are not NEARLY
// as fast as they could be for fixed-size, small arrays.
//
// In this case a sorting network is required. This application is
// indended to show the desired ideal amd64 assembly of such a sorting
// network for array sizes between 2 and 6, inclusive, and so I provide
// assembly snippets in MASM for each of these cases.
//
// Note that these use the Windows x64 calling convention, and then
// proceed to clobber a bunch of registers they shouldn't. Normally
// they'd be inlined. Feel free to callee-save registers that your
// application cannot safely clobber.
//
// Ideally, of course, your compiler is smart enough to generate assembly
// this good for you from a C++ sorting network. But it's not immediately
// obvious what sort of C++ to write to convince the compiler to do so.
//
// Suggestions for C++ implementations of sorting networks for sizes
// of 2 and 6 are thus provided.
//
// NOTE THAT MOST COMPILERS STILL FAIL TO PRODUCE OPTIMAL ASSEMBLY!!
// Therefore the point of these snippets is to be TESTED ON YOUR
// ENVIRONMENT, and the generated assembly to be COMPARED TO THE
// ASSEMBLY SNIPPETS to see whether the compiler succeeded.
//
// Clang seems to do a good job, although it's too afraid of using
// amd64 GPRs.
//
// MSVC fails miserably for sizes above about 3.
//
// g++ is very picky and sometimes does, sometimes doesn't, depending
// on compiler options and version. g++ doesn't like using GPRs 
// much, nor does it like conditional moves - which ARE FASTER 
// for this application on modern CPUs.
//
// However, the best results for int32_t sorting, that NO compiler will
// produce from anything other than carefully crafted SIMD intrinsics,
// are obtained using SSE... as long as your array size
// is 4 or less. This is by FAR the fastest option.
//
// As an example, my simdsort4() function uses only SSE4, no AVX,
// to accomplish a 4-sort in just a few cycles
// (1.40 nanoseconds on my machine (!)).
//
// NOTE that I provide two different implementations of simdsort4(),
// one using all integer instructions but two extra logical operations,
// and one using fewer logical operations but with a float reinterpretation.
// Which is faster will depend on your architecture; comment out the current
// one and uncomment the other to try it. It's important to only leave one
// uncommented at a time, as the extra data can actually affect timings
// significantly.
//
// Similarly, the best results for int8_t sorting, as long as your array
// size is 16 or less, by FAR, are obtained using SSE.
//
// As an example, my simdsort6() function uses only SSE4, no AVX,
// to accomplish a 6-sort of 8-bit signed integers in just a few cycles
// (2.40 nanoseconds on my machine (!)).
//
// If you want to experiment with timings,
// be sure to disable link-time optimization,
// a.k.a whole-program optimization (MSVC),
// a.k.a -flto (g++) or otherwise ensure that
// the sort functions don't get optimized away
// if you want accurate timings for testing.
//
// If, however, you just want to steal the sort functions,
// obviously you can and should use LTO.
//
// Available sorts and their function names:
//
// >>> SSE (fast as hell. FASTEST option on modern CPUs):
// Sorting 4 int32_t  |  simdsort4()
// Sorting 6 int8_t   |  simdsort6()
//
// >>> Assembly (these are written in MASM for Win64;
// but it's Intel syntax and you can make the small
// modifications required for other assemblers. These
// are mainly intended as targets to compare compiler output
// to, in order to ensure that your compiler is generating
// the right thing from a C++ sort function like that of sort2()
// or sort6(). Note that MOST COMPILERS DO THIS VERY POORLY.
// Sorting 2 int32_t  |  sort2a()
// Sorting 3 int32_t  |  sort3a()
// Sorting 4 int32_t  |  sort4a()
// Sorting 5 int32_t  |  sort5a()
// Sorting 6 int32_t  |  sort6a()
//
// >>> C++ (MAKE SURE YOUR COMPILER SUCCESSFULLY GENERATES
// THE DESIRED ASSEMBLY by comparing it to the assembly snippets!)
// Sorting 2 int32_t  |  sort2()
// Sorting 6 int32_t  |  sort6()
//

#include "sorts.h"

void sort2(int* __restrict v) {
	const int a = v[0];
	const int b = v[1];
	v[0] = b > a ? a : b;
	v[1] = b > a ? b : a;
}

void sort6(int* __restrict d) {
#define min(a, b) ((a < b) ? a : b)
#define max(a, b) ((a < b) ? b : a)
#define SWAP(x,y) { int a = min(d[x], d[y]); int b = max(d[x], d[y]); d[x] = a; d[y] = b; }
	SWAP(1, 2);
	SWAP(0, 2);
	SWAP(0, 1);
	SWAP(4, 5);
	SWAP(3, 5);
	SWAP(3, 4);
	SWAP(0, 3);
	SWAP(1, 4);
	SWAP(2, 5);
	SWAP(2, 4);
	SWAP(1, 3);
	SWAP(2, 3);
#undef SWAP
#undef max
#undef min
}

//const __m128i pass1_add4s = _mm_setr_epi8(4, 5, 6, 7, 4, 5, 6, 7, 12, 13, 14, 15, 12, 13, 14, 15);
//const __m128i pass2_add4s = _mm_setr_epi8(8, 9, 10, 11, 12, 13, 14, 15, 8, 9, 10, 11, 12, 13, 14, 15);
//const __m128i pass3_add4s = _mm_setr_epi8(0, 1, 2, 3, 8, 9, 10, 11, 8, 9, 10, 11, 12, 13, 14, 15);
//void simdsort4(int* __restrict v) {
//	__m128i b, a = _mm_loadu_si128(reinterpret_cast<const __m128i*>(v));
//
//	b = _mm_shuffle_epi32(a, 177);
//	b = _mm_cmpgt_epi32(b, a);
//	b = _mm_and_si128(b, _mm_set1_epi8(-4));
//	b = _mm_add_epi8(b, pass1_add4s);
//	a = _mm_shuffle_epi8(a, b);
//
//	b = _mm_shuffle_epi32(a, 78);
//	b = _mm_cmpgt_epi32(b, a);
//	b = _mm_and_si128(b, _mm_set1_epi8(-8));
//	b = _mm_add_epi8(b, pass2_add4s);
//	a = _mm_shuffle_epi8(a, b);
//
//	b = _mm_shuffle_epi32(a, 216);
//	b = _mm_cmpgt_epi32(b, a);
//	b = _mm_and_si128(b, _mm_set1_epi8(-4));
//	b = _mm_add_epi8(b, pass3_add4s);
//	a = _mm_shuffle_epi8(a, b);
//
//	_mm_storeu_si128(reinterpret_cast<__m128i*>(v), a);
//}

const __m128i pass1_add4 = _mm_setr_epi32(1, 1, 3, 3);
const __m128i pass2_add4 = _mm_setr_epi32(2, 3, 2, 3);
const __m128i pass3_add4 = _mm_setr_epi32(0, 2, 2, 3);
void simdsort4(int* __restrict v) {
	__m128i b, a = _mm_loadu_si128(reinterpret_cast<const __m128i*>(v));

	b = _mm_shuffle_epi32(a, 177);
	b = _mm_cmpgt_epi32(b, a);
	b = _mm_add_epi32(b, pass1_add4);
	a = _mm_castps_si128(_mm_permutevar_ps(_mm_castsi128_ps(a), b));

	b = _mm_shuffle_epi32(a, 78);
	b = _mm_cmpgt_epi32(b, a);
	b = _mm_and_si128(b, _mm_set1_epi8(-2));
	b = _mm_add_epi32(b, pass2_add4);
	a = _mm_castps_si128(_mm_permutevar_ps(_mm_castsi128_ps(a), b));

	b = _mm_shuffle_epi32(a, 216);
	b = _mm_cmpgt_epi32(b, a);
	b = _mm_add_epi32(b, pass3_add4);
	a = _mm_castps_si128(_mm_permutevar_ps(_mm_castsi128_ps(a), b));

	_mm_storeu_si128(reinterpret_cast<__m128i*>(v), a);
}

const __m128i pass1_shf = _mm_setr_epi8(1, 0, 3, 2, 5, 4, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
const __m128i pass1_add = _mm_setr_epi8(1, 1, 3, 3, 5, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
const __m128i pass2_shf = _mm_setr_epi8(2, 4, 0, 5, 1, 3, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
const __m128i pass2_and = _mm_setr_epi8(-2, -3, -2, -2, -3, -2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
const __m128i pass2_add = _mm_setr_epi8(2, 4, 2, 5, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
const __m128i pass4_shf = _mm_setr_epi8(0, 2, 1, 4, 3, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
const __m128i pass4_add = _mm_setr_epi8(0, 2, 2, 4, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
const __m128i pass5_shf = _mm_setr_epi8(0, 1, 3, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
const __m128i pass5_add = _mm_setr_epi8(0, 1, 3, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
void simdsort6(char* __restrict v) {
	__m128i b, a = _mm_cvtsi32_si128(*reinterpret_cast<const int*>(v));
	a = _mm_insert_epi16(a, *reinterpret_cast<const int*>(v + 4), 2);

	b = _mm_shuffle_epi8(a, pass1_shf);
	b = _mm_cmpgt_epi8(b, a);
	b = _mm_add_epi8(b, pass1_add);
	a = _mm_shuffle_epi8(a, b);

	b = _mm_shuffle_epi8(a, pass2_shf);
	b = _mm_cmpgt_epi8(b, a);
	b = _mm_and_si128(b, pass2_and);
	b = _mm_add_epi8(b, pass2_add);
	a = _mm_shuffle_epi8(a, b);

	b = _mm_shuffle_epi8(a, pass1_shf);
	b = _mm_cmpgt_epi8(b, a);
	b = _mm_add_epi8(b, pass1_add);
	a = _mm_shuffle_epi8(a, b);

	b = _mm_shuffle_epi8(a, pass4_shf);
	b = _mm_cmpgt_epi8(b, a);
	b = _mm_add_epi8(b, pass4_add);
	a = _mm_shuffle_epi8(a, b);

	b = _mm_shuffle_epi8(a, pass5_shf);
	b = _mm_cmpgt_epi8(b, a);
	b = _mm_add_epi8(b, pass5_add);
	a = _mm_shuffle_epi8(a, b);

	*reinterpret_cast<int*>(v) = _mm_cvtsi128_si32(a);
	*reinterpret_cast<int16_t*>(v + 4) = _mm_extract_epi16(a, 2);
}
