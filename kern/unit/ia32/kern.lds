/*
 * load low kernel at 1M physical
 * DMA buffers (8 * 128K) at 4M physical
 * locate high kernel at 3G virtual
 */

MPENTRY		= 0x9f000;
HICORE		= 1M;
DMABUF		= 8M;
DMABUFSIZE	= 8M;
PAGETAB		= 16M;
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

    . = MPENTRY;

    .mp : AT(MPENTRY) {
        mpentry.o(.*)
    }

    . = HICORE;

    /* identity-mapped low kernel segment */
    .boot : AT(HICORE) {
    	boot.o(.*)
	setup.o(.*)
	real.o(.*)
	vga.o(.*)
	vbe.o(.*)
	bmfonts.o(.*)
	init.o(.*)
	main.o(.*)
	trap.o(.*)
	isr.o(.*)
	tss.o(.*)
	pic.o(.*)
	seg.o(.*)
	vm.o(.*)
	util.o(.*)
/*
	mpentry.o(.text)
	mpentry.o(.data)
	mpentry.o(.bss)
*/
	mp.o(.*)
    	hpet.o(.*)
	sig.o(.*)
	cons.o(.*)
	ata.o(.*)
        _eboot = .;
	. = ALIGN(PAGESIZE);
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

