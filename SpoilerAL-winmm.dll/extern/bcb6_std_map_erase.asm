.486
.model flat, c
assume fs:nothing

extrn bcb6_std_node_alloc_deallocate:dword
extrn F005D54CC:dword
extrn F005F5078:dword
extrn F004D3BAC:dword

public bcb6_std_map_erase@8

.const

data1 label byte
	dword   004D3B20H
	dword   0000400FH, -4
	dword   00000000H

data2 label byte
	dword   00000000H, -40
	dword   00050000H, 00000000H
	dword   00000000H, 00050008H
	dword   00000000H
	dword   data1
	dword   00050014H, 00000000H
	dword   00000000H, 00000000H

.code

align 16

bcb6_std_map_erase@8 proc near

	push    ebp
	mov     eax, offset data2
	mov     ebp, esp
	sub     esp, 152
	push    ebx
	mov     ebx, dword ptr [ebp + 8H]
	call    dword ptr [F005D54CC]
	mov     eax, dword ptr [ebx + 8H]
	mov     edx, dword ptr [ebp + 0CH]
	add     eax, 12
	mov     dword ptr [ebp - 30H], edx
	push    eax
	sub     eax, 4
	push    eax
	sub     eax, 4
	push    eax
	mov     dword ptr [ebp - 2CH], edx
	push    edx
	call    dword ptr [F005F5078]
	add     esp, 16
	lea     edx, [eax + 10H]
	test    edx, edx
	mov     dword ptr [ebp - 4H], edx
	jz      L1
	push    eax
	mov     eax, dword ptr [ebp - 0CH]
	push    0
	sub     eax, 8
	add     edx, 8
	mov     dword ptr [ebp - 0CH], eax
	mov     eax, dword ptr [edx]
	mov     ecx, dword ptr [edx + 4H]
	mov     dword ptr [ebp - 98H], eax
	mov     dword ptr [ebp - 94H], ecx
	mov     dword ptr [ebp - 78H], eax
	mov     dword ptr [ebp - 74H], ecx
	mov     dword ptr [ebp - 58H], eax
	mov     dword ptr [ebp - 54H], ecx
	mov     eax, dword ptr [edx + 8H]
	mov     ecx, dword ptr [edx + 0CH]
	mov     dword ptr [ebp - 90H], eax
	mov     dword ptr [ebp - 8CH], ecx
	mov     dword ptr [ebp - 70H], eax
	mov     dword ptr [ebp - 6CH], ecx
	mov     dword ptr [ebp - 50H], eax
	mov     dword ptr [ebp - 4CH], ecx
	mov     eax, dword ptr [edx + 10H]
	mov     ecx, dword ptr [edx + 14H]
	mov     dword ptr [ebp - 88H], eax
	mov     dword ptr [ebp - 84H], ecx
	mov     dword ptr [ebp - 68H], eax
	mov     dword ptr [ebp - 64H], ecx
	mov     dword ptr [ebp - 48H], eax
	mov     dword ptr [ebp - 44H], ecx
	mov     eax, dword ptr [edx + 18H]
	mov     ecx, dword ptr [edx + 1CH]
	mov     dword ptr [ebp - 80H], eax
	mov     dword ptr [ebp - 7CH], ecx
	mov     dword ptr [ebp - 60H], eax
	mov     dword ptr [ebp - 5CH], ecx
	mov     dword ptr [ebp - 40H], eax
	mov     dword ptr [ebp - 3CH], ecx
	mov     dword ptr [ebp - 38H], edx
	mov     dword ptr [ebp - 34H], edx
	push    edx
	call    dword ptr [F004D3BAC]
	mov     eax, dword ptr [esp + 8]
	add     esp, 8 + 4
L1:
	test    eax, eax
	jz      L2
	push    88
	push    eax
	call    dword ptr [bcb6_std_node_alloc_deallocate]
	add     esp, 8
L2:
	mov     eax, dword ptr [ebx + 10H]
	mov     ecx, dword ptr [ebp - 40]
	dec     eax
	mov     dword ptr fs:[0], ecx
	mov     dword ptr [ebx + 10H], eax
	pop     ebx
	mov     esp, ebp
	pop     ebp
	ret     8

bcb6_std_map_erase@8 endp

end
