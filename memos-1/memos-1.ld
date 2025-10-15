OUTPUT_FORMAT("binary")     

ENTRY(_start)           

SECTIONS {
  . = 0x7C00;                

  .text : {
    *(.text) 

  }
}


/*
This script is written to produce a raw binary boot sector rather than an ELF executable. 
The BIOS can only load and execute a flat 512-byte sector, so we set OUTPUT_FORMAT("binary") 
to strip away all headers. The code is linked at 0x7C00 because that is the fixed memory 
address where the BIOS places the boot sector before jumping into it.
*/