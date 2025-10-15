all:
	# assemble memos-2.s
	as --32 memos-2.s -o memos-2.o
	# compile mmap.c to produce a mmap.o object file
	gcc -m32 -ffreestanding -c mmap.c mmap.o 	
	# link both scipts
	ld -m elf_i386 -T memos-2.ld -o memos-2 memos-2.o mmap.o 
