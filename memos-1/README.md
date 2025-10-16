## MemOS 1

Evan Liu (U12686999) Marcus Izumi (U53686318)

MemOS 1 produces a 512 byte binary that is placed inside the Master Boot Record. We are in Real-Mode here, so the memory probing and printing is done using BIOS interrupts. 

### Running MemOS 1
To assemble and link the program, navigate inside the directory and run:

```
make all 
```

To boot the binary in QEMU, run:
```
make run
```

If address is already in use, run: 
```
killall qemu-system-i386
```

