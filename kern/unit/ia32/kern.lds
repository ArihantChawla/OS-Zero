/*
 * load low kernel at 1M physical
 * DMA buffers (8 * 128K) at 4M physical
 * locate high kernel at 3G virtual
 */

MPENTRY		= 0x9f000;
HICORE		= 1M;
USYSINFOSIZE    = PAGESIZE;
DMABUF		= 8M;
DMABUFSIZE	= 8M;
PAGETAB		= 16M;
PAGESIZE	= 4K;
PAGETABSIZE	= 4M;
UPAGETAB	= 20M;
VIRTBASE	= 0xc0000000;

OUTPUT_FORMAT("elf32-i386")

ENTRY(_start)

SECTIONS {

/*
    . = MPENTRY;

    .mp : AT(MPENTRY) {
        mpentry.o(.text)
        mpentry.o(.data)
        mpentry.o(.bss)
        mp.o(.text)
        mp.o(.data)
        mp.o(.bss)
    }
*/

    . = HICORE;

    /* identity-mapped low kernel segment */
    .boot : AT(HICORE) {
    	boot.o(.text)
    	boot.o(.text.*)
    	boot.o(.data)
	setup.o(.text)
	setup.o(.text.*)
	setup.o(.data)
	real.o(.text)
	real.o(.text.*)
	real.o(.data)
	main.o(.text)
	main.o(.text.*)
	main.o(.data)
/*
        mp.o(.text)
        mp.o(.data)
        mp.o(.bss)
*/
	seg.o(.text)
	seg.o(.text.*)
	seg.o(.data)
	trap.o(.text)
	trap.o(.text.*)
	trap.o(.data)
	m_trap.o(.text)
	m_trap.o(.text.*)
	m_trap.o(.data)
	pic.o(.text)
	pic.o(.text.*)
	pic.o(.data)
	protmode.o(.text)
	protmode.o(.text.*)
	protmode.o(.data)
	vm.o(.text)
	vm.o(.text.*)
	vm.o(.data)
	sig.o(.text)
	sig.o(.text.*)
	sig.o(.data)
	sig.o(.bss)
/* 	vm.o(.bss) */
	util.o(.text)
	util.o(.text.*)
	util.o(.data)
/*	util.o(.bss) */
	ps2.o(.text)
	ps2.o(.text.*)
	ps2.o(.data)
	vbe.o(.text)
	vbe.o(.text.*)
	vbe.o(.data)
	vbe.o(.bss)
        test.o(.text)
        test.o(.text.*)
/*
	bmfonts.o(.text)
	bmfonts.o(.data)
	bmfonts.o(.bss)
	tss.o(.text)
	tss.o(.data)
	tss.o(.bss)
	debug.o(.text)
	debug.o(.data)
	debug.o(.bss)
*/
	. = ALIGN(PAGESIZE);
        _eboot = .;
    }

    .usysinfo : {
        _usysinfo = .;
	. += USYSINFOSIZE;
	_eusysinfo = .;
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
    .pgtab : AT(PAGETAB) {
        _pagetab = .;
  	. += PAGETABSIZE;
	_epagetab = .;
    }

/*    . = UPAGETAB; */

    . = VIRTBASE;

    /* sections mapped by virtual addresses */

    /* read-only segment; code and some data */
    .text : AT(PAGETAB + SIZEOF(.pgtab)) {
        _text = .;
	_textvirt = PAGETAB + SIZEOF(.pgtab);
	*(EXCLUDE_FILE(*usr.o) .text*)
	*(.text)
	*(.text.*)
	*(.rodata*)
	*(.rodata.*)
	*(.eh*)
        _etext = _text + SIZEOF(.text);
	_etextvirt = _textvirt + SIZEOF(.text);
	. = ALIGN(PAGESIZE);
    }

    /* read-write data segment; initialised global structures */
    .data : AT(PAGETAB + SIZEOF(.pgtab) + SIZEOF(.text)) {
    	_data = _etext;
	_datavirt = _etextvirt;
	*(.data)
	. = ALIGN(PAGESIZE);
    }

    /* bss segment; runtime-allocated, uninitialised data */
    .bss : AT(PAGETAB + SIZEOF(.pgtab) + SIZEOF(.text) + SIZEOF(.data)) {
        _bss = _data + SIZEOF(.data);
	_bssvirt = _etextvirt + SIZEOF(.data) + SIZEOF(.text);
	*(.bss)
	*(COMMON)
        _ebss = _bss + SIZEOF(.bss);
	_ebssvirt = _bssvirt + SIZEOF(.bss);
/*	_ebssphys = LOADADDR(.bss) + SIZEOF(.bss); */
	. = ALIGN(PAGESIZE);
    }

    .usr : AT(PAGETAB + SIZEOF(.pgtab) + SIZEOF(.text) + SIZEOF(.data)
    	    + SIZEOF(.bss)) {
        _usr = _bss + SIZEOF(.bss);
	_usrvirt = _ebssvirt + SIZEOF(.bss);
	usr.o(.text)
	_eusr = _usr + SIZEOF(.usr);
	_eusrvirt = _usrvirt + SIZEOF(.usr);
	. = ALIGN(PAGESIZE);
    }

    _kernsize = PAGETAB + PAGETABSIZE + SIZEOF(.text) + SIZEOF(.data)
    	      + SIZEOF(.bss) + SIZEOF(.usr);
/*
    _kernsize = PAGETAB + PAGETABSIZE + SIZEOF(.text) + SIZEOF(.data)
    	      + SIZEOF(.bss);
*/
}

