#include "multiboot.h"  // TODO: ensure this is the right multiboot.h
#include <stdint.h>     // for uint64_t

/* Custom strlen since no external libraries */
int my_strlen(const char* text) {
    int length = 0;
    while (text[length] != '\0') {
        length++;
    }
    return length; 
}

/* printing characters to the screen */
static unsigned short *videoram = (unsigned short *) 0xB8000;
static int attrib = 0x0F; // black background, white foreground
static int csr_x = 0, csr_y = 0;
#define COLS 80
void putc (unsigned char c) {
    if (c == 0x09) { // Tab (move to next multiple of 8) 
        csr_x = (csr_x + 8) & ~(8 - 1);
    } else if (c == '\r') { // Carriage Return 
        csr_x = 0;
    } else if (c == '\n') { // Line Feed (unix-like) 
        csr_x = 0; csr_y++;
    } else if(c >= ' ') { // Printable characters 
        /* Put the character w/attributes */
        *(videoram + (csr_y * COLS + csr_x)) = c | (attrib << 8);
        csr_x++;
    }
    if(csr_x >= COLS) { csr_x = 0; csr_y++;} // wrap around 
}

/* print string to the screen */
void puts (char *text) {
    int i;
    for (i = 0; i < my_strlen ((const char*)text); i++) {
        putc (text[i]);
    }
}

void put_uint64(uint64_t num) {
    char buf[21]; // 20 digits max for 64-bit integer + '\0'
    int k = 0;
    if (num == 0) {
        putc('0');
        return;
    }
    while (num > 0) {
        buf[k++] = '0' + (num % 10);
        num /= 10;
    }
    while (k--)
        putc(buf[k]);
}

void put_hex64(uint64_t num) {
    char hex_digits[] = "0123456789ABCDEF";
    int j;
    for (j = 60; j >= 0; j -= 4) {
        putc(hex_digits[(num >> j) & 0xF]);
    }
}

/* main program */ 
void _main(multiboot_info_t* mbd, uint32_t magic)
{
    /* Make sure the magic number matches for memory mapping*/
    if(magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        puts ("PANIC: invalid magic number!");
        return;
    }

    /* Check bit 6 to see if we have a valid memory map */
    if(!(mbd->flags >> 6 & 0x1)) {
        puts ("PANIC: invalid memory map given by GRUB bootloader");
        return;
    }

    /* Loop thorugh memory map accumulate total available mememory */
    uint64_t total;
    int i; 
    for (i = 0; i < mbd->mmap_length; i += sizeof(multiboot_memory_map_t)) {
        multiboot_memory_map_t* mmmt = (multiboot_memory_map_t *) (mbd->mmap_addr + i);

        if (mmmt->type == MULTIBOOT_MEMORY_AVAILABLE) {   // Check if memory type is available ram 
            total += ((uint64_t)mmmt->len_high << 32) | mmmt->len_low;
        }
    }

    /* Print to screen */
    puts ("MemOS: Welcome *** Total free memory: ");
    put_uint64 (total / (1024 * 1024)); /* Convert bytes to MB */
    puts ("MB\n");

    /* Loop through the memory map and display the values */
    for(i = 0; i < mbd->mmap_length; i += sizeof(multiboot_memory_map_t)) {
        multiboot_memory_map_t* mmmt = (multiboot_memory_map_t*) (mbd->mmap_addr + i);
        /* 
         * Do something with this memory block!
         * BE WARNED that some of memory shown as availiable is actually 
         * actively being used by the kernel! You'll need to take that
         * into account before writing to memory!
        */
        uint64_t addr_start = ((uint64_t)mmmt->addr_high << 32) | mmmt->addr_low;
        uint64_t length = ((uint64_t)mmmt->len_high << 32) | mmmt->len_low; 
        uint64_t addr_stop = addr_start + length - 1; 
        puts ("Address range [ 0x");
        put_hex64 (addr_start);
        puts (" : 0x");
        put_hex64 (addr_stop);
        puts ("] status: Type ");
        put_uint64 (mmmt->type);
        putc ('\n');
    }
}
