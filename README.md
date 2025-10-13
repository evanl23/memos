# MemOS
Marcus Izumi, Evan Liu

Master boot record code that probes the system BIOS of a x86 machine and prints the available memory to the screen. Written for [Richard West](https://www.cs.bu.edu/fac/richwest/)'s CS552 class. 

There are two versions, ```memos-1``` probes the machine's memory in Real-Mode while ```memos-2``` probes in Protected-Mode. 


## MemOS-1

To assemble and link the program, navigate inside the directory and run:

```
make all1 
```

To boot the binary in qemu, run:
```
make run1
```

If address is already in use, run: 
```
killall qemu-system-i386
```


## MemOs-2
