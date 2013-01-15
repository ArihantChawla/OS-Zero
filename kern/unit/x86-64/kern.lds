HICORE		= 1M;
DMABUF		= 4M;
DMABUFSIZE	= 4M;
PAGESIZE	= 4K;

OUTPUT_FORMAT(elf64-x86-64)

ENTRY(_start)

SECTIONS {
    . = HICORE;

    .boot : AT(HICORE) {
        boot.o(.text)
	boot.o(.data)
	boot.o(.bss)
    }
