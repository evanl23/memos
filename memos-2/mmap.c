#include "multiboot.h"   
#include "stdint.h"     // for uint64_t

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
#define GRAPH_COLS 70
#define GRAPH_ROW_BASE 12
#define GRAPH_START_COL 5
#define TYPE_COLOR_COUNT 3

static const unsigned char FULL_BLOCK = 0xDB;
static const uint8_t type_color[TYPE_COLOR_COUNT] = {
    0x07, 
    0x0A, // available 
    0x0C  // reserved
};
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

static void set_cell(int row, int col, uint8_t color, unsigned char ch) {
    *(videoram + (row * COLS + col)) = ch | (color << 8);
}

static void draw(int row, int start_col, int width, uint8_t color) {
    int end = start_col + width;
    if (end > COLS) {
        end = COLS;
    }
    int col;
    for (col = start_col; col < end; col++) {
        set_cell(row, col, color, FULL_BLOCK);
    }
}

/* print string to the screen */
void puts (char *text) {
    int i;
    for (i = 0; i < my_strlen ((const char*)text); i++) {
        putc (text[i]);
    }
}

void put_uint32(uint32_t num) {
    char buf[11]; // 10 digits max for 32-bit integer + '\0'
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

void put_uint64(uint64_t num) {
    uint32_t high = (uint32_t)(num >> 32);
    uint32_t low = (uint32_t)num;
    
    if (high != 0) {
        put_uint32(high);
        put_uint32(low);
    } else {
        put_uint32(low);
    }
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
    uint64_t total_free = 0;
    uint64_t total_phys = 0;
    int i; 
    for (i = 0; i < mbd->mmap_length; i += sizeof(multiboot_memory_map_t)) {
        multiboot_memory_map_t* mmmt = (multiboot_memory_map_t *) (mbd->mmap_addr + i);
        uint64_t length = ((uint64_t)mmmt->len_high << 32) | mmmt->len_low;

        if (mmmt->type == MULTIBOOT_MEMORY_AVAILABLE) {   // Check if memory type is available ram 
            total_free += length;
        }
        total_phys += length;
    }

    /* Print to screen */
    puts ("MemOS: Welcome *** Total free memory: ");
    put_uint64 (total_free / (1024 * 1024)); /* Convert bytes to MB */
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
        puts ("Address range [0x");
        put_hex64 (addr_start);
        puts (" : 0x");
        put_hex64 (addr_stop);
        puts ("] status: Type ");
        put_uint32 (mmmt->type);
        putc ('\n');
    }

    if (total_phys == 0) {
        return;
    }

    puts ("\nAvailable = Green Reserved =R ed\n");

    int graph_row = GRAPH_ROW_BASE;
    int graph_col = GRAPH_START_COL;

    for(i = 0; i < mbd->mmap_length; i += sizeof(multiboot_memory_map_t)) {
        multiboot_memory_map_t* mmmt = (multiboot_memory_map_t*) (mbd->mmap_addr + i);
        uint64_t length = ((uint64_t)mmmt->len_high << 32) | mmmt->len_low; 
        if (length == 0) {
            continue;
        }
        int width = 0;
        uint64_t remainder = 0;
        int step;
        for (step = 0; step < GRAPH_COLS; step++) {
            remainder += length;
            if (remainder >= total_phys) {
                width++;
                remainder -= total_phys;
            }
        }
        if (width < 1 && length > 0) {
            width = 1;
        }
        if (graph_col + width >= COLS) {
            graph_row++;
            graph_col = GRAPH_START_COL;
        }
        uint8_t color = 0x07;
        if (mmmt->type < TYPE_COLOR_COUNT) {
            color = type_color[mmmt->type];
        }
        draw(graph_row, graph_col, width, color);
        graph_col += width;
        if (graph_col >= COLS) {
            graph_row++;
            graph_col = GRAPH_START_COL;
        }
    }
}
