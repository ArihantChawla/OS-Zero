HICORE		= 1M;
DMABUF		= 4M;
DMABUFSIZE	= 4M;
PAGESIZE	= 4K;
VIRTBASE	= -2G;

OUTPUT_FORMAT(elf64-x86-64)

ENTRY(_start)

SECTIONS {
    . = HICORE;

    .boot : AT(HICORE) {
        boot.o(.text)
	boot.o(.data)
	boot.o(.bss)
        setup.o(.text)
	setup.o(.data)
	setup.o(.bss)
    }

    . = DMABUF;

    .dma : AT(DMABUF) {
        _dmabuf = .;
        . += DMABUFSIZE;
        _edmabuf = .;
    }

    . = VIRTBASE;

    .text : AT(DMABUF + DMABUFSIZE) {
        _text = .;
	_textvirt = DMABUF + DMABUFSIZE;
	*(.text*)
	*(.rodata*)
	*(.rodata.*)
	*(.eh*)
	_etext = _textvirt + SIZEOF(.text);
	_etextvirt = _textvirt + SIZEOF(.text);
	. = ALIGN(PAGESIZE);
    }

    .data : AT(DMABUF + DMABUFSIZE + SIZEOF(.text)) {
        _date = _etext;
	_datavirt = _etextvirt;
	*(.data)
	. = ALIGN(PAGESIZE);
    }

    .bss : AT(DMABUF + DMABUFSIZE + SIZEOF(.text) + SIZEOF(.data)) {
        _bss = _data + SIZEOF(.data);
	_bssvirt = _etextvirt + SIZEOF(.data);
	*(.bss)
	*(COMMON)
	_ebss = _bss + SIZEOF(.bss);
	_ebssvirt = _bssvirt + SIZEOF(.bss);
	. = ALIGN(PAGESIZE);
    }

    _kernsize = DMABUF + DMABUFSIZE + SIZEOF(.text) + SIZEOF(.data) + SIZEOF(.bss);
}

