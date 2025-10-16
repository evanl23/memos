## MemOS 2

Evan Liu (U12686999) Marcus Izumi (U53686318)

MemOS 2 produces actual kernel code that probes and prints memory mappings. The probing is done by utilizing a struct that GRUB loads when booting. Thus, we create a disk image and boot our OS using GRUB. 

### Disk Configuration
* Byte size: 512
* Cylinders: 10 + 99 (last two digits of Liu's BUID) = 109 
* Heads: 16
* Sectors: 63

### Compiling
To compile this program, simply run 
```
make all
```
and you should get an .elf file. 

### Running MemOS 2 
To start the OS, first create a virtual disk, mount its filesystems, mount the disk image partition under the host OS, and install GRUB. For a complete guide, look [here](https://www.cs.bu.edu/fac/richwest/cs552_fall_2025/assignments/memos/disk-image-HOWTO). 

Once the disk image is ready, place `menu.lst` in /boot/grub of your virtual disk. Next place the compiled .elf file in /boot so that grub knows there to find it. 

Finally, use a PC emulator to boot your disk image and display text on screen. If you have QEMU and vncviewer, type something like 
```
qemu-system-i386 -hda memos-2.img -vnc :0 &
/root/vncviewer :0
```

If address is already in use, run: 
```
killall qemu-system-i386
```

