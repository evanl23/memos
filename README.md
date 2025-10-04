# memos
Master boot record code that probes the upper memory of an x86 machine


Inside the directory, run:

make clean && make && make run


If address is already in use, run 

killall qemu-system-i386

And free up the address