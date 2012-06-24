/* Very simple memory layout */
SECTIONS
{
	. = 0x08003000;
	.vectors : { *(.vectors) }
	. = 0x08003100;
	.text : { *(.text) *(.note*) }
	. = 0x20000000;
	.data : { *(.data) }
	.bss : { *(.bss) }
	. = 0x20005000;
        _stack_top = .;
}
