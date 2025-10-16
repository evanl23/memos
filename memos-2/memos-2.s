.text

.globl _start
_start:
    jmp real_start

    /* Multiboot header -- Safe to place this header in 1st page of memory for GRUB */
    .align 4
    .long 0x1BADB002 /* Multiboot magic number */
    .long 0x00000003 /* Align modules to 4KB, req. mem size */
                      /* See 'info multiboot' for further info */
    .long 0xE4524FFB /* Checksum */

real_start:
    /* Setup proper stack */
    movl $stack_top, %esp
    movl %esp, %ebp

    /* pass arguments to _main */
    pushl %eax
    pushl %ebx
    
    call _main
   
    add $8, %esp /* restore stack pointer */
    hlt
