all:
	as --32 memos-2.s -o memos-2.o
	gcc -m32 -fno-stack-protector -fno-builtin -nostdinc -c mmap.c -Iinclude -o mmap.o 	
	ld -m elf_i386 -T memos-2.ld memos-2.o mmap.o -o memos-2.elf

clean: 
	rm memos-2.o mmap.o memos-2.elf
