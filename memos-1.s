      .global _start
      .code16

_start:

# set video mode	
	    movw $0x0003, %ax
	    int $0x10

# detecting memory
# result is stored in memory at ES:DI


# Call E820 memory detection
    call do_e820

	movw 0x8000, %ax   # load entry count
	movb %ah, %al      # move high byte into AL
	call print         # prints high byte

	movw 0x8000, %ax   # reload AX from memory
	call print         # prints low byte (AL)


# Print newline (CR + LF)
    movb $0x0E, %ah
    movb $0x0D, %al         # carriage return
    int  $0x10
    movb $0x0A, %al         # line feed
    int  $0x10

hang:
    jmp hang

mmap_ent equ 0x8000             ; the number of entries will be stored at 0x8000
do_e820:
      movw $0x8004, %di # Set di to 0x8004. Otherwise this code will get stuck in `int 0x15` after some entries are fetched 
      xor %ebx, %ebx		; ebx must be 0 to start
      xor %bp, %bp		; keep an entry count in bp
      movl $0x534D4150, %edx # place "smap" into edx
      movw, $0xE820, %eax
      mov dword 1, [%es:%di + 20] # force a valid ACPI 3.x entry
      movb $24, %ecx # ask for 24 bytes 
      INT 0x15

      jc .failed	; carry set on first call means "unsupported function"
	    mov $0x0534D4150, %edx	; Some BIOSes apparently trash this register?
	    cmp %edx, %eax ; on success, eax must have been reset to "SMAP"
	    jne .failed
	    test %ebx, %ebx		; ebx = 0 implies list is only 1 entry long (worthless)
	    je .failed
	    jmp .jmpin

.e820lp:
	    movw $0xe820, %eax		; eax, ecx get trashed on every int 0x15 call
	    mov [es:di + 20], dword 1	; force a valid ACPI 3.X entry
	    mov ecx, 24		; ask for 24 bytes again
	    int 0x15
	    jc short .e820f		; carry set means "end of list already reached"
	    mov edx, 0x0534D4150	; repair potentially trashed register

.jmpin:
	    jcxz .skipent		; skip any 0 length entries
	    cmp $20, %cl	; got a 24 byte ACPI 3.X response?
	    jbe .notext
	    test $1, byte [%es:%di + 20] ; if so: is the "ignore this data" bit clear?
	    je .skipent

.notext:
      mov [%es:%di + 8], %ecx	; get lower uint32_t of memory region length
      or [%es:%di + 12], %ecx	; "or" it with upper uint32_t to test for zero
      jz .skipent		; if length uint64_t is 0, skip entry
      inc bp			; got a good entry: ++count, move to next storage spot
      add $24, %di

.skipent:
	    test %ebx, %ebx		; if ebx resets to 0, list is complete
	    jne .e820lp

.e820f:
	    mov %bp, [%es:$mmap_ent] ; store the entry count
	    clc			; there is "jc" on end of list to this point, so the carry must be cleared
	    ret
.failed:
	    stc			; "function unsupported" error exit
	    ret


print:
    pushw %dx
    movb %al, %dl
    shrb $4, %al
    cmpb $10, %al
    jge 1f
    addb $0x30, %al
    jmp 2f
1:  addb $55, %al
2:  movb $0x0E, %ah
    movw $0x07, %bx
    int $0x10

    movb %dl, %al
    andb $0x0f, %al
    cmpb $10, %al
    jge 1f
    addb $0x30, %al
    jmp 2f
1:  addb $55, %al
2:  movb $0x0E, %ah
    movw $0x07, %bx
    int $0x10
    popw %dx
    ret



      .org 0x1FE
      .byte 0x55
      .byte 0xAA