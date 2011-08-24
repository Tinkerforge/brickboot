OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
/*ENTRY(entry)*/

/* Memory Spaces Definitions */
MEMORY
{
    romcodesram (W!RX) : ORIGIN = 0x20000000, LENGTH = 0x0800
    sram (W!RX) : ORIGIN = 0x20000800, LENGTH = 0x00007800 /* sram, 32K - sizeof(romcodesram) */
}

/* Entry point */
ENTRY (ResetException)

SECTIONS
{
    /* startup code in the .isr_vector */
    .text :
    {
        . = ALIGN(4);
        _stext = .;
        KEEP(*(.isr_vector .isr_vector.*))
        *(.mailbox)
        *(.text .text.*)
        *(.rodata .rodata.*)
        *(.glue_7)
        *(.glue_7t)
        *(.gcc_except_table)
        *(.rodata .rodata*)
        *(.gnu.linkonce.r.*)
        . = ALIGN(4);
        _etext = .;
    } > sram

    /* data */
    .data :
    {
        . = ALIGN(4);
        _sidata = .;
        _sdata = .;

        *(.data)
        *(.data.*)
        . = ALIGN(4);
        _edata = .;
    } > sram

    .bss (NOLOAD) : {
        _szero = .;
        *(.bss)
        . = ALIGN(4);
        _ezero = .;
    } >sram

    /* Stack in SRAM */
    _sstack = 0x20007FFC;
}
end = .;

/* end of allocated ram _end */
PROVIDE(_start_heap = end);

/* end of heap, aligned by 4 byte */ 
PROVIDE (_end_heap = ALIGN(ORIGIN(sram) + LENGTH(sram), 4));