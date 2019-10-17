#include <memory.h>

#pragma warning(disable:4414)

extern void * __cdecl _memrchr(const void *buffer, int c, size_t count);

extern const char xmmconst_casebitA[16];
extern const char xmmconst_maskbit[32];
#define casebit xmmconst_casebitA
#define maskbit xmmconst_maskbit

#ifndef _M_IX86
void * __cdecl _memrichr(const void *buffer, int c, size_t count)
{
	char c1, c2;

	c1 = (char)c | ('a' - 'A');
	if ((unsigned char)(c1 - 'a') > 'z' - 'a')
		return _memrchr(buffer, c, count);
	buffer += count - 1;
	while (count--)
		if (((c2 = *(((char *)buffer)--)) | ('a' - 'A')) == c1)
			return (char *)buffer + 1;
		else if (!c2)
			break;
	return NULL;
}
#else
static void * __cdecl memrichrSSE2(const void *buffer, int c, size_t count);
static void * __cdecl memrichr386(const void *buffer, int c, size_t count);
static void * __cdecl memrichrCPUDispatch(const void *buffer, int c, size_t count);

static void *(__cdecl * memrichrDispatch)(const void *buffer, int c, size_t count) = memrichrCPUDispatch;

__declspec(naked) void * __cdecl _memrichr(const void *buffer, int c, size_t count)
{
	__asm
	{
		jmp     dword ptr [memrichrDispatch]
	}
}

__declspec(naked) static void * __cdecl memrichrSSE2(const void *buffer, int c, size_t count)
{
	__asm
	{
		#define buffer (esp + 4)
		#define c      (esp + 8)
		#define count  (esp + 12)

		mov     eax, dword ptr [count]                  // eax = count
		mov     ecx, dword ptr [c]
		test    eax, eax                                // check if count=0
		jz      count_equal_zero                        // if count=0, leave
		or      ecx, 'a' - 'A'
		xor     edx, edx
		mov     dl, cl
		sub     ecx, 'a'
		cmp     cl, 'z' - 'a'
		ja      _memrchr
		movd    xmm1, edx                               // xmm1 = search char
		punpcklbw xmm1, xmm1
		pshuflw xmm1, xmm1, 0
		movlhps xmm1, xmm1
		movdqa  xmm2, xmmword ptr [casebit]
		mov     ecx, dword ptr [buffer]                 // ecx = buffer
		push    ebx                                     // preserve ebx
		lea     ebx, [ecx + eax - 1]                    // ebx = last byte of buffer
		push    esi                                     // preserve esi
		and     ebx, -16                                // ebx = last xmmword of buffer
		add     ecx, eax                                // ecx = end of buffer
		sub     ebx, eax                                // ebx = last xmmword of buffer - count
		and     ecx, 15
		jz      loop_begin
		movdqa  xmm0, xmmword ptr [ebx + eax]
		por     xmm0, xmm2
		pcmpeqb xmm0, xmm1
		pmovmskb edx, xmm0
		mov     esi, 7FFFH
		xor     ecx, 15
		shr     esi, cl
		and     edx, esi
		jnz     found
		xor     ecx, 15
		sub     ebx, 16
		sub     eax, ecx
		jb      retnull
		add     ebx, ecx

		align   16
	loop_begin:
		movdqa  xmm0, xmmword ptr [ebx + eax]
		por     xmm0, xmm2
		pcmpeqb xmm0, xmm1
		pmovmskb edx, xmm0
		test    edx, edx
		jnz     has_chr
		sub     eax, 16
		jae     loop_begin
	retnull:
		xor     eax, eax
		pop     esi                                     // restore esi
		pop     ebx                                     // restore ebx
	count_equal_zero:
		ret

		align   16
	has_chr:
		cmp     eax, 16
		jae     found
		mov     ecx, ebx
		or      esi, -1
		and     ecx, 15
		shl     esi, cl
		and     edx, esi
		jz      retnull
	found:
		bsr     edx, edx
		add     eax, ebx
		pop     esi                                     // restore esi
		pop     ebx                                     // restore ebx
		add     eax, edx
		ret

		#undef buffer
		#undef c
		#undef count
	}
}

__declspec(naked) static void * __cdecl memrichr386(const void *buffer, int c, size_t count)
{
	__asm
	{
		#define buffer (esp + 4)
		#define c      (esp + 8)
		#define count  (esp + 12)

		mov     eax, dword ptr [count]                  // eax = count
		mov     edx, dword ptr [c]                      // dl = search char
		test    eax, eax                                // check if count=0
		jz      count_equal_zero                        // if count=0, leave
		or      edx, 'a' - 'A'
		xor     ecx, ecx
		mov     cl, dl
		sub     edx, 'a'
		cmp     dl, 'z' - 'a'
		ja      _memrchr
		mov     edx, eax                                // u edx = count
		mov     eax, dword ptr [buffer]                 // v eax = buffer
		                                                // set all 4 bytes of ebx to [value]
		push    ebx                                     // u preserve ebx
		push    esi                                     // v preserve esi
		mov     ebx, ecx                                // u ebx = 0/0/0/c
		add     eax, edx                                // v eax = end of buffer
		shl     ecx, 8                                  // u ecx = 0/0/c/0
		sub     eax, 4                                  // v eax = end of buffer - 4
		mov     esi, ecx                                // u esi = 0/0/c/0
		or      ecx, ebx                                // v ecx = 0/0/c/c
		shl     ecx, 16                                 // u ecx = c/c/0/0
		or      ebx, esi                                // v ebx = 0/0/c/c
		or      ebx, ecx                                // u ebx = all 4 bytes = [search char]
		mov     ecx, eax                                // v eax = end of buffer - 4
		and     ecx, 3
		jz      loop_entry
		xor     ecx, 3
		jz      modulo3
		dec     ecx
		jz      modulo2
		mov     cl, byte ptr [eax + 3]
		dec     eax
		or      cl, 'a' - 'A'
		cmp     cl, bl
		je      byte_0
		dec     edx                                     // counter--
		jz      retnull
	modulo2:
		mov     cl, byte ptr [eax + 3]
		dec     eax
		or      cl, 'a' - 'A'
		cmp     cl, bl
		je      byte_0
		dec     edx                                     // counter--
		jz      retnull
	modulo3:
		mov     cl, byte ptr [eax + 3]
		dec     eax
		or      cl, 'a' - 'A'
		cmp     cl, bl
		je      byte_0
		dec     edx                                     // counter--
		jnz     loop_entry
		jmp     retnull

		align   16
	loop_begin:
		sub     edx, 4
		jbe     retnull
	loop_entry:
		mov     ecx, dword ptr [eax]                    // read 4 bytes
		sub     eax, 4
		or      ecx, 20202020H
		xor     ecx, ebx                                // ebx is byte\byte\byte\byte
		mov     esi, 7EFEFEFFH
		add     esi, ecx
		xor     ecx, -1
		xor     ecx, esi
		and     ecx, 81010100H
		jz      loop_begin
		and     ecx, 01010100H
		jnz     byte_0_to_2
		test    esi, esi
		js      loop_begin
		cmp     edx, 3
		jbe     retnull
		add     eax, 7
		jmp     epilogue

		align   16
	byte_0_to_2:
		test    ch, ch
		jnz     byte_0
		and     ecx, 00010000H
		jnz     byte_1
		cmp     edx, 2
		jbe     retnull
		add     eax, 6
		jmp     epilogue

	byte_0:
		add     eax, 4
		jmp     epilogue

	byte_1:
		dec     edx
		jz      retnull
		add     eax, 5
		jmp     epilogue

		align   16
	retnull:
		xor     eax, eax
	epilogue:
		pop     esi                                     // restore esi
		pop     ebx                                     // restore ebx
	count_equal_zero:
		ret                                             // __cdecl return

		#undef buffer
		#undef c
		#undef count
	}
}

__declspec(naked) static void * __cdecl memrichrCPUDispatch(const void *buffer, int c, size_t count)
{
	#define __ISA_AVAILABLE_X86  0
	#define __ISA_AVAILABLE_SSE2 1

	extern unsigned int __isa_available;

	__asm
	{
		cmp     dword ptr [__isa_available], __ISA_AVAILABLE_X86
		jne     L1
		mov     dword ptr [memrichrDispatch], offset memrichr386
		jmp     memrichr386
	L1:
		mov     dword ptr [memrichrDispatch], offset memrichrSSE2
		jmp     memrichrSSE2
	}

	#undef __ISA_AVAILABLE_X86
	#undef __ISA_AVAILABLE_SSE2
}
#endif
