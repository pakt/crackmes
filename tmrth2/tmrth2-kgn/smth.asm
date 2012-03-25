section .data

tab1    dd 0
tab1p   TIMES 1000h dd 0

section .text

global init_smth
global do_smth64

init_smth:
                push    esi
                push    edi
                xor     edi, edi

loc_401004:                             ; CODE XREF: init_smth+100j
                mov     eax, edi
                cdq
                mov     esi, eax
                shrd    eax, edx, 1
                xor     ecx, ecx
                and     esi, 1
                shr     edx, 1
                or      esi, ecx
                mov     ecx, eax
                mov     eax, edx
                jz      short loc_401027
                xor     ecx, 0CAFEF00Dh
                xor     eax, 0DEADBABEh

loc_401027:                             ; CODE XREF: init_smth+1Aj
                mov     edx, ecx
                shrd    ecx, eax, 1
                and     edx, 1
                xor     esi, esi
                shr     eax, 1
                or      edx, esi
                jz      short loc_401043
                xor     ecx, 0CAFEF00Dh
                xor     eax, 0DEADBABEh

loc_401043:                             ; CODE XREF: init_smth+36j
                mov     edx, ecx
                shrd    ecx, eax, 1
                and     edx, 1
                xor     esi, esi
                shr     eax, 1
                or      edx, esi
                jz      short loc_40105F
                xor     ecx, 0CAFEF00Dh
                xor     eax, 0DEADBABEh

loc_40105F:                             ; CODE XREF: init_smth+52j
                mov     edx, ecx
                shrd    ecx, eax, 1
                and     edx, 1
                xor     esi, esi
                shr     eax, 1
                or      edx, esi
                jz      short loc_40107B
                xor     ecx, 0CAFEF00Dh
                xor     eax, 0DEADBABEh

loc_40107B:                             ; CODE XREF: init_smth+6Ej
                mov     edx, ecx
                shrd    ecx, eax, 1
                and     edx, 1
                xor     esi, esi
                shr     eax, 1
                or      edx, esi
                jz      short loc_401097
                xor     ecx, 0CAFEF00Dh
                xor     eax, 0DEADBABEh

loc_401097:                             ; CODE XREF: init_smth+8Aj
                mov     edx, ecx
                shrd    ecx, eax, 1
                and     edx, 1
                xor     esi, esi
                shr     eax, 1
                or      edx, esi
                jz      short loc_4010B3
                xor     ecx, 0CAFEF00Dh
                xor     eax, 0DEADBABEh

loc_4010B3:                             ; CODE XREF: init_smth+A6j
                mov     edx, ecx
                shrd    ecx, eax, 1
                and     edx, 1
                xor     esi, esi
                shr     eax, 1
                or      edx, esi
                jz      short loc_4010CF
                xor     ecx, 0CAFEF00Dh
                xor     eax, 0DEADBABEh

loc_4010CF:                             ; CODE XREF: init_smth+C2j
                mov     edx, ecx
                shrd    ecx, eax, 1
                and     edx, 1
                xor     esi, esi
                shr     eax, 1
                or      edx, esi
                jz      short loc_4010EB
                xor     ecx, 0CAFEF00Dh
                xor     eax, 0DEADBABEh

loc_4010EB:                             ; CODE XREF: init_smth+DEj
                mov     [tab1+edi*8], ecx
                mov     [tab1p+edi*8], eax
                inc     edi
                cmp     edi, 100h
                jl      loc_401004
                pop     edi
                pop     esi
                retn

; Attributes: bp-based frame

do_smth64:

                mov ecx, [esp+4]    ;msg
                mov edi, [esp+8]    ;msg_len
                xor     eax, eax
                xor     edx, edx
                xor     esi, esi
                test    edi, edi
                jz      short loc_401145

loc_40111F:                             ; CODE XREF: smth64+33j
                mov ecx, [esp+4]    ;msg
                movzx   ecx, byte [esi+ecx]
                movzx   ebx, al
                shrd    eax, edx, 8
                xor     ecx, ebx
                xor     eax, [tab1+ecx*8]
                shr     edx, 8
                xor     edx, [tab1p+ecx*8]
                inc     esi
                cmp     esi, edi
                jb      short loc_40111F

loc_401145:                             ; CODE XREF: smth64+Dj
                mov ebx, [esp+12]
                mov [ebx], eax      ;1st dword
                mov [ebx+4], edx      ;2nd one
                
                retn
