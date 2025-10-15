#include "multiboot.h" // TODO: ensure this is the right multiboot.h

/* Custom strlen since no external libraries */
int my_strlen(char* text) {
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
    for (int i = 0; i < my_strlen ((const char*)text); i++) {
        putc (text[i]);
    }
}

void _main(multiboot_info_t* mbd, uint32_t magic)
{
  /* Make sure the magic number matches for memory mapping*/
    if(magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        panic("invalid magic number!");
    }

    /* Check bit 6 to see if we have a valid memory map */
    if(!(mbd->flags >> 6 & 0x1)) {
        panic("invalid memory map given by GRUB bootloader");
    }

    /* Loop thorugh memory map accumulate total available mememory */
    long total;
    int i; 
    for (i = 0; i < mbd->mmap_length; i += sizeof(multiboot_memory_map_t)) {
        multiboot_memory_map_t* mmmt = (multiboot_memory_map_t *) (mbd->mmap_addr + i);

        if (mmmt->type == MULTIBOOT_MEMORY_AVAILABLE) {   // Check if memory type is available ram 
            total += mmmt->len;
        }
    }

    /* Print to screen */

    /* Loop through the memory map and display the values */
    int i;
    for(i = 0; i < mbd->mmap_length; 
        i += sizeof(multiboot_memory_map_t)) 
    {
        multiboot_memory_map_t* mmmt = 
            (multiboot_memory_map_t*) (mbd->mmap_addr + i);

        printf("Start Addr: %x | Length: %x | Size: %x | Type: %d\n",
            mmmt->addr, mmmt->len, mmmt->size, mmmt->type);

        if(mmmt->type == MULTIBOOT_MEMORY_AVAILABLE) {
            /* 
             * Do something with this memory block!
             * BE WARNED that some of memory shown as availiable is actually 
             * actively being used by the kernel! You'll need to take that
             * into account before writing to memory!
             */
        }
    }
}
