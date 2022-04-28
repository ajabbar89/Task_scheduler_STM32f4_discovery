ENTRY(Reset_handler)
MEMORY
{
	FLASH(rx)  : ORIGIN = 0x08000000,LENGTH = 1024K
	SRAM(rwx)  : ORIGIN = 0x20000000,LENGTH = 128K
}
SECTIONS
{
	.isr_vectors :
	{
		*(.isr_vectors)
	}> FLASH

	.text :
	{
		*(.text)	
		*(.rodata)
		. = ALIGN(4);
		_end_text = .;
	}> FLASH 

	.data :
	{
		_start_data = .;
		*(.data)
		. = ALIGN(4);
		_end_data = .;
	}> SRAM AT> FLASH
	
	.bss :
	{
		_start_bss = .;
		*(.bss)
		*(COMMON)
		. = ALIGN(4);
		_end_bss = .;
	}> SRAM
}

