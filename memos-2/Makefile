all:
	# assemble memos-2.s
	as --32 memos-2.s -o memos-2.o
	# compile mmap.c to produce a mmap.o object file and include multiboot.h
	gcc -m32 -fno-stack-protector -fno-builtin -nostdinc -c mmap.c -o mmap.o 	
	# link both scipts
	ld -m elf_i386 -T memos-2.ld memos-2.o mmap.o -o memos-2.elf 
