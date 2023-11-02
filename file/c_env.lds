OUTPUT_ARCH(riscv)
ENTRY(_start)
MEMORY
{
    ram (wxa!ri) :   ORIGIN = 0x80000000, LENGTH = 100K
}

SECTIONS
{
   __stack_size = DEFINED(__stack_size) ? __stack_size : 8K;
	.text :
	{
		*(.init)
		*(.text*)
	} >ram

    . = ALIGN(4);
	.data ALIGN(4) :
	{
		*(.data*)
    	*(.rodata*)
		. = ALIGN(4);
	} >ram
	. = ALIGN(4);
	PROVIDE(__data_end = .);

    . = ALIGN(4);
  	PROVIDE( __bss_start = . );
	.bss ALIGN(4) :
	{
		*(.bss*)
		. = ALIGN(4);
	} >ram
	. = ALIGN(4);
	PROVIDE(__bss_end = .);

    .stack ORIGIN(ram) + LENGTH(ram) - __stack_size :
    {
        PROVIDE( _heap_end = . );
        . = __stack_size;
        PROVIDE( _heap_start = . );
    } >ram
}