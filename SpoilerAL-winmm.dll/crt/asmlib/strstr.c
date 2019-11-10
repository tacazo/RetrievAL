#include <string.h>

extern int __cdecl InstructionSet();

static char * __cdecl strstrSSE42(const char *string1, const char *string2);
static char * __cdecl strstrSSE2(const char *string1, const char *string2);
static char * __cdecl strstrGeneric(const char *string1, const char *string2);
static char * __cdecl strstrCPUDispatch(const char *string1, const char *string2);

static char *(__cdecl * strstrDispatch)(const char *string1, const char *string2) = strstrCPUDispatch;

// function dispatching
__declspec(naked) char * __cdecl strstr(const char *string1, const char *string2)
{
	__asm
	{
		jmp     dword ptr [strstrDispatch]                  // Go to appropriate version, depending on instruction set
	}
}

// SSE4.2 version
__declspec(naked) static char * __cdecl strstrSSE42(const char *string1, const char *string2)
{
	__asm
	{
		push    ebx
		push    esi
		mov     esi, dword ptr [esp + 12]                   // haystack
		mov     eax, dword ptr [esp + 16]                   // needle
		movdqu  xmm1, xmmword ptr [eax]                     // needle

		align   16
	haystacknext:
		// [esi] = haystack
		pcmpistrm xmm1, xmmword ptr [esi], 00001100b        // unsigned byte search, equal ordered, return mask in xmm0
		jc      matchbegin                                  // found beginning of a match
		jz      nomatch                                     // end of haystack found, no match
		add     esi, 16
		jmp     haystacknext

	matchbegin:
		jz      foundshort                                  // haystack ends here, a short match is found
		movd    eax, xmm0                                   // bit mask of possible matches

	nextindexbit:
		bsf     ecx, eax                                    // index of first bit in mask of possible matches

		// compare strings for full match
		lea     ebx, [esi + ecx]                            // haystack + index
		mov     edx, dword ptr [esp + 16]                   // needle

	compareloop:
		// compare loop for long match
		movdqu  xmm2, [edx]                                 // paragraph of needle
		pcmpistrm xmm2, xmmword ptr [ebx], 00001100B        // unsigned bytes, equal ordered, modifies xmm0
		// (can't use "equal each, masked" because it inverts when past end of needle, but not when past end of both)

		jno     longmatchfail                               // difference found after extending partial match
		js      longmatchsuccess                            // end of needle found, and no difference
		add     edx, 16
		add     ebx, 16
		jmp     compareloop                                 // loop to next 16 bytes

	longmatchfail:
		// remove index bit of first partial match
		btr     eax, ecx
		test    eax, eax
		jnz     nextindexbit                                // mask contains more index bits, loop to next bit in eax mask
		// mask exhausted for possible matches, continue to next haystack paragraph
		add     esi, 16
		jmp     haystacknext                                // loop to next paragraph of haystack

	longmatchsuccess:
		// match found over more than one paragraph
		lea     eax, [esi + ecx]                            // haystack + index to begin of long match
		pop     esi
		pop     ebx
		ret

	foundshort:
		// match found within single paragraph
		movd    eax, xmm0                                   // bit mask of matches
		bsf     eax, eax                                    // index of first match
		add     eax, esi                                    // pointer to first match
		pop     esi
		pop     ebx
		ret

	nomatch:
		// needle not found, return 0
		xor     eax, eax
		pop     esi
		pop     ebx
		ret
	}
}

#if 1
// SSE2 version
__declspec(naked) static char * __cdecl strstrSSE2(const char *string1, const char *string2)
{
	#define PAGE_SIZE 0x1000

	__asm
	{
		#define string1 (esp + 4)
		#define string2 (esp + 8)

		mov     ecx, dword ptr [string2]                    // str2 (the string to be searched for)
		xor     edx, edx
		mov     dl, byte ptr [ecx]                          // edx contains first char from str2
		mov     eax, dword ptr [string1]                    // str1 (the string to be searched)
		push    ebx                                         // preserve ebx
		mov     ebx, edx                                    // set 2 bytes of edx to first char
		shl     edx, 8
		push    esi                                         // preserve esi
		or      edx, ebx                                    // is str2 empty?
		jz      empty_str2                                  // if so, return str1 (ANSI mandated)
		push    edi                                         // preserve edi
		mov     esi, eax                                    // str1
		movd    xmm2, edx                                   // set all bytes of xmm2 to first char
		pshuflw xmm2, xmm2, 0
		movlhps xmm2, xmm2
		pxor    xmm3, xmm3                                  // set to zero
		lea     ebx, [ecx + 1]                              // str2 + 1

		// find the first character of str2 in the str1 by doing linear scan
		align   16
	find_first_char:
		mov     ecx, esi
		add     esi, 16
		and     ecx, PAGE_SIZE - 1
		xor     edx, edx
		cmp     ecx, PAGE_SIZE - 16
		ja      compare_first_char                          // jump if cross pages
		movdqu  xmm0, xmmword ptr [esi - 16]
		pxor    xmm1, xmm1
		pcmpeqb xmm1, xmm0
		pcmpeqb xmm0, xmm2
		por     xmm0, xmm1
		pmovmskb edx, xmm0
		test    edx, edx
		jz      find_first_char
		bsf     edx, edx
	compare_first_char:
		mov     cl, byte ptr [esi + edx - 16]               // cl is char from str1
		lea     esi, [esi + edx - 15]                       // increment pointer into str1
		test    cl, cl                                      // end of str1?
		jz      not_found                                   // yes, and no match has been found

		// check if remaining consecutive characters match continuously
		mov     eax, ebx
		mov     edi, esi
		test    eax, 15
		jnz     byte_loop
		sub     edi, 16
		sub     eax, 16

		align   16
	xmmword_loop:
		mov     ecx, PAGE_SIZE - 1
		add     edi, 16
		and     ecx, edi
		add     eax, 16
		cmp     ecx, PAGE_SIZE - 16
		ja      byte_loop                                   // jump if cross pages
		movdqu  xmm0, xmmword ptr [edi]
		movdqa  xmm1, xmmword ptr [eax]
		pcmpeqb xmm0, xmm1
		pcmpeqb xmm1, xmm3
		pcmpeqb xmm0, xmm3
		por     xmm1, xmm0
		pmovmskb ecx, xmm1
		test    ecx, ecx
		jz      xmmword_loop
		bsf     ecx, ecx
		add     eax, ecx
		add     edi, ecx

		align   16
	byte_loop:
		mov     cl, byte ptr [eax]
		mov     dl, byte ptr [edi]
		test    cl, cl
		jz      match
		cmp     cl, dl
		jne     find_first_char
		inc     eax
		inc     edi
		test    eax, 15                                     // use only eax for 'test reg, imm'
		jnz     byte_loop
		sub     edi, 16
		sub     eax, 16
		jmp     xmmword_loop

		align   16
	not_found:
		xor     eax, eax
		pop     edi
	empty_str2:
		pop     esi
		pop     ebx
		ret

		// match!  return (esi - 1)
		align   16
	match:
		lea     eax, [esi - 1]
		pop     edi
		pop     esi
		pop     ebx
		ret

		#undef string1
		#undef string2
	}

	#undef PAGE_SIZE
}
#endif

// generic version
__declspec(naked) static char * __cdecl strstrGeneric(const char *string1, const char *string2)
{
	__asm
	{
		push    esi
		push    edi
		mov     esi, dword ptr [esp + 12]                   // haystack
		mov     edi, dword ptr [esp + 16]                   // needle

		mov     ax, word ptr [edi]
		test    al, al
		jz      Found                                       // a zero-length needle is always found
		test    ah, ah
		jz      SingleCharNeedle

	SearchLoop:
		// search for first character match
		mov     cl, byte ptr [esi]
		test    cl, cl
		jz      NotFound                                    // end of haystack reached without finding
		cmp     al, cl
		je      FirstCharMatch                              // first character match

	IncompleteMatch:
		inc     esi
		jmp     SearchLoop                                  // loop through haystack

	FirstCharMatch:
		mov     ecx, esi                                    // begin of match position

	MatchLoop:
		inc     ecx
		inc     edi
		mov     al, byte ptr [edi]
		test    al, al
		jz      Found                                       // end of needle. match ok
		cmp     al, byte ptr [ecx]
		je      MatchLoop
		// match failed, recover and continue
		mov     edi, dword ptr [esp + 16]                   // needle
		mov     al, byte ptr [edi]
		jmp     IncompleteMatch

	NotFound:
		// needle not found. return 0
		xor     eax, eax
		pop     edi
		pop     esi
		ret

	Found:
		// needle found. return pointer to position in haystack
		mov     eax, esi
		pop     edi
		pop     esi
		ret

	SingleCharNeedle:
		// Needle is a single character
		movzx   ecx, byte ptr [esi]
		test    cl, cl
		jz      NotFound                                    // end of haystack reached without finding
		cmp     al, cl
		je      Found
		inc     esi
		jmp     SingleCharNeedle                            // loop through haystack
	}
}

// CPU dispatching for strstr. This is executed only once
__declspec(naked) static char * __cdecl strstrCPUDispatch(const char *string1, const char *string2)
{
	__asm
	{
		// get supported instruction set
		call    InstructionSet
		// Point to generic version of strstr
		mov     ecx, offset strstrGeneric
		cmp     eax, 4                                      // check SSE2
		jb      Q100
#if 1
		// SSE2 supported
		// Point to SSE2 version of strstr
		mov     ecx, offset strstrSSE2
		cmp     eax, 10                                     // check SSE4.2
		jb      Q100
#endif
		// SSE4.2 supported
		// Point to SSE4.2 version of strstr
		mov     ecx, offset strstrSSE42

	Q100:
		mov     dword ptr [strstrDispatch], ecx
		// Continue in appropriate version of strstr
		jmp     ecx
	}
}
