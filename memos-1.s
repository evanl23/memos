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

# print welcome message 
    movw $welcome_string, %di     # point DI to string (not SI)
    call print_string

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

    movw 0x8000, %cx      # CX = entry count
    movw $0x8004, %si     # SI = start of entries

print_loop:
    # print address range
    movw $address_string, %di
    call print_string

    # Print base address (32-bit low part only)
    movl 0(%si), %eax     # load 32 bits of base address
    call print_dword      # print base address
   
    movb $0x0E, %ah
    movb $0x3A, %al       # print colon ":" 
    int  $0x10
    
    # Calculate end address: base + length - 1
    movl 0(%si), %eax     # reload base address
    movl 8(%si), %edx     # load length into EDX
    addl %edx, %eax       # add length to base
    subl $1, %eax         # subtract 1 to get last valid address
    call print_dword      # print end address

    # print status
    movw $status_string, %di 
    call print_string
    
    # Print type
    movl 16(%si), %eax    # load type 
    call print            # print type (just 1 byte)
    
    # newline after each entry
    movb $0x0E, %ah
    movb $0x0D, %al
    int  $0x10
    movb $0x0A, %al
    int  $0x10

    addw $24, %si         # next entry 
    loop print_loop

hang:
    jmp hang

    mmap_ent = 0x8000             # the number of entries will be stored at x8000

do_e820:
    movw $0x8004, %di # Set di to 0x8004. Otherwise this code will get stuck in `int 0x15` after some entries are fetched 
    xor %ebx, %ebx		# ebx must be 0 to start
    xor %bp, %bp		# keep an entry count in bp
    movl $0x534D4150, %edx # place "smap" into edx
    movl $0xE820, %eax
    movl $1, %es:20(%di) # force a valid ACPI 3.x entry
    movl $24, %ecx # ask for 24 bytes 
    int $0x15

    jc .failed	# carry set on first call means "unsupported function"
    movl $0x0534D4150, %edx	# Some BIOSes apparently trash this register?
    cmp %edx, %eax # on success, eax must have been reset to "SMAP"
    jne .failed
    test %ebx, %ebx		# ebx = 0 implies list is only 1 entry long (worthless)
    je .failed
    jmp .jmpin

.e820lp:
    mov $0xe820, %eax		# eax, ecx get trashed on every int 0x15 call
    movl $1, %es:20(%di)	# force a valid ACPI 3.X entry
    movl $24, %ecx  		# ask for 24 bytes again
    int $0x15
    jc .e820f		# carry set means "end of list already reached"
    movl $0x0534D4150, %edx	# repair potentially trashed register

.jmpin:
    jcxz .skipent		# skip any 0 length entries
    cmp $20, %cl	# got a 24 byte ACPI 3.X response?
    jbe .notext
    testb $1, %es:20(%di) # if so: is the "ignore this data" bit clear?
    je .skipent

.notext:
    movl %es:8(%di), %ecx # get lower uint32_t of memory region length
    orl %es:12(%di), %ecx	# "or" it with upper uint32_t to test for zero
    jz .skipent		# if length uint64_t is 0, skip entry
    incw %bp		# got a good entry: ++count, move to next storage spot
    addw $24, %di

.skipent:
    test %ebx, %ebx		# if ebx resets to 0, list is complete
    jne .e820lp

.e820f:
    movw %bp, %es:mmap_ent # store the entry count
    clc			# there is "jc" on end of list to this point, so the carry must be cleared
	  ret

.failed:
    stc			# "function unsupported" error exit
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

# Print 32-bit value in EAX as 8 hex digits
print_dword:
    pushl %eax            # save original value
    
    # Print byte 3 (most significant - bits 31-24)
    shrl $24, %eax        # shift right 24 bits to get top byte
    call print            # print top byte
    
    popl %eax             # restore original value
    pushl %eax            # save it again
    
    # Print byte 2 (bits 23-16)
    shrl $16, %eax        # shift right 16 bits
    call print            # print byte 2
    
    popl %eax             # restore original value  
    pushl %eax            # save it again
    
    # Print byte 1 (bits 15-8)
    shrl $8, %eax         # shift right 8 bits
    call print            # print byte 1
    
    popl %eax             # restore original value
    
    # Print byte 0 (bits 7-0) 
    call print            # print lowest byte
    
    ret

print_string:
    pusha                # save all registers
    movb $0x0E, %ah      # BIOS teletype function
    movw $0x07, %bx      # page/color attributes
1:
    movb (%di), %al      # load byte from [DI] into AL
    test %al, %al        # check if null terminator
    jz 2f                # jump if zero (end of string)
    int $0x10            # print character
    incw %di             # increment DI manually
    jmp 1b               # loop back
2:
    popa                 # restore all registers
    ret

welcome_string: .asciz "MemOS: Welcome *** System Memory is: " 
address_string: .asciz "Address range ["
status_string: .asciz "] status: "

    .org 0x1FE
    .byte 0x55
    .byte 0xAA
