all:
	as --32 memos-1.s -o memos-1.o
	ld -m elf_i386 -T memos-1.ld -o memos-1 memos-1.o

run:
	qemu-system-i386 -hda memos-1 -vnc :0 & \
	sleep 1 && /root/vncviewer :0

clean:
	rm -f memos-1.o memos-1
