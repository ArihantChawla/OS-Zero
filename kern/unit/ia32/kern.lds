/*
 * load low kernel at 1M physical
 * DMA buffers (8 * 128K) at 4M physical
 * locate high kernel at 3G virtual
 */

MPENTRY		= 0x9f000;
HICORE		= 1M;
DMABUF		= 4M;
DMABUFSIZE	= 4M;
PAGETAB		= 8M;
PAGESIZE	= 4K;
PAGETABSIZE	= 4M;
VIRTBASE	= 0xc0000000;

OUTPUT_FORMAT("elf32-i386")

ENTRY(_start)

SECTIONS {
/*
    . = 0x00010000;
    .real : AT(0x00010000) {
        real.o(.*)
    }
*/

    . = HICORE;

    /* identity-mapped low kernel segment */
    .boot : AT(HICORE) {
    	boot.o(.text)
    	boot.o(.data)
    	boot.o(.bss)
	setup.o(.text)
	setup.o(.data)
	real.o(.text)
	vbe.o(.text)
	vbe.o(.data)
	vbe.o(.bss)
	init.o(.text)
	init.o(.data)
	init.o(.bss)
	main.o(.text)
	trap.o(.text)
	trap.o(.bss)
	isr.o(.text)
	isr.o(.data)
	tss.o(.text)
	tss.o(.data)
	tss.o(.bss)
	pic.o(.text)
	seg.o(.text)
	vm.o(.text)
/*	vm.o(.data)
	vm.o(.bss) */
	util.o(.text)
	mp.o(.text)
    	mp.o(.data)
    	mp.o(.bss)
    	hpet.o(.text)
    	hpet.o(.data)
    	hpet.o(.bss)
        _eboot = .;
	. = ALIGN(PAGESIZE);
    }

    /* multiprocessor bootstrap */
    .mp : AT(HICORE + SIZEOF(.boot)) {
        _mpentry = .;
	mpentry.o(.text)
	_emp = .;
    }

    . = DMABUF;

    /* DMA buffers (below 16 megabytes) */
    .dma : AT(DMABUF) {
	_dmabuf = .;
	. += DMABUFSIZE;
	_edmabuf = .;
    }

    . = PAGETAB;

    /* identity-mapped set of page tables */
    .ptab : AT(PAGETAB) {
        _pagetab = .;
  	. += PAGETABSIZE;
	_epagetab = .;
    }

    . = VIRTBASE;

    /* sections mapped by virtual addresses */

    /* read-only segment; code and some data */
    .text : AT(PAGETAB + SIZEOF(.ptab)) {
        _text = .;
	_textvirt = PAGETAB + SIZEOF(.ptab);
	*(.text*)
	*(.rodata*)
	*(.rodata.*)
	*(.eh*)
        _etext = _text + SIZEOF(.text);
	_etextvirt = _textvirt + SIZEOF(.text);
	. = ALIGN(PAGESIZE);
    }
 
    /* read-write data segment; initialised global structures */
    .data : AT(PAGETAB + SIZEOF(.ptab) + SIZEOF(.text)) {
    	_data = _etext;
	_datavirt = _etextvirt;
	*(.data)
	. = ALIGN(PAGESIZE);
    }

    /* bss segment; runtime-allocated, uninitialised data */
    .bss : AT(PAGETAB + SIZEOF(.ptab) + SIZEOF(.text) + SIZEOF(.data)) {
        _bss = _data + SIZEOF(.data);
	_bssvirt = _etextvirt + SIZEOF(.data) + SIZEOF(.text);
	*(.bss)
	*(COMMON)
        _ebss = _bss + SIZEOF(.bss);
	_ebssvirt = _bssvirt + SIZEOF(.bss);
	. = ALIGN(PAGESIZE);
    }

    _kernsize = PAGETAB + PAGETABSIZE + SIZEOF(.text) + SIZEOF(.data) + SIZEOF(.bss);
}

