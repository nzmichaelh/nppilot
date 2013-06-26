/* Very simple memory layout */
ENTRY(_start)

SECTIONS
{
	. = 0x08000000;
	.vectors : { KEEP(*(.vectors)) }
	. = 0x08000200;
	.text : ALIGN(4) { *(.text) *(.text.*) *(.ARM.extab* .gnu.linkonce.armextab.*) _etext = .; }
        .init_array : { __init_array_start = .;
                    KEEP (*(SORT(.init_array.*)))
                    KEEP (*(.init_array))
                    __init_array_end = .;
        }

	.timers.fixed : { *(.timers.fixed) }
        .rodata : ALIGN(4) { *(.rodata) *(.rodata.*) . = ALIGN(4); _erodata = .; }
	.timers.ro : ALIGN(4) { __timers_ro_start = .; KEEP(*(.timers.ro)); __timers_ro_end = .; }
        .ARM.exidx : {
            __exidx_start = .;
            *(.ARM.exidx* .gnu.linkonce.armexidx.*)
            __exidx_end = .;
        }
         __data_load = .;
	. = 0x20000000;
	.data : AT (__data_load) { __data_start = .; *(.data) *(.data.*); . = ALIGN(4); __data_end = .; }
	.bss : { __bss_start = .; *(.bss) *(.bss.*); . = ALIGN(4); __bss_end = .; }
	.timers : { __timers_start = .; KEEP(*(.timers)); __timers_end = .; }
	. = 0x20005000;
        __stack_top = .;

        /DISCARD/ : { *(.note*) }
}
