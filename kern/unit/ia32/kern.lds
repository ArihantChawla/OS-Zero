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

    . = MPENTRY;

    .mp : AT(MPENTRY) {
        mpentry.o(.text)
        mpentry.o(.data)
        mpentry.o(.bss)
        mp.o(.text)
        mp.o(.data)
        mp.o(.bss)
    }

    . = HICORE;

    /* identity-mapped low kernel segment */
    .boot : AT(HICORE + SIZEOF(.hicore)) {
    	boot.o(.text)
    	boot.o(.data)
    	boot.o(.bss)
	setup.o(.text)
	setup.o(.data)
	setup.o(.bss)
	real.o(.text)
	real.o(.data)
	real.o(.bss)
	vga.o(.text)
	vga.o(.data)
	vga.o(.bss)
	vbe.o(.text)
	vbe.o(.data)
	vbe.o(.bss)
	bmfonts.o(.text)
	bmfonts.o(.data)
	bmfonts.o(.bss)
	init.o(.text)
	init.o(.data)
	init.o(.bss)
	main.o(.text)
	main.o(.data)
	main.o(.bss)
	trap.o(.text)
	trap.o(.data)
	trap.o(.bss)
	isr.o(.text)
	isr.o(.data)
	isr.o(.bss)
	tss.o(.text)
	tss.o(.data)
	tss.o(.bss)
	pic.o(.text)
	pic.o(.data)
	pic.o(.bss)
	seg.o(.text)
	seg.o(.data)
	seg.o(.bss)
	vm.o(.text)
	vm.o(.data)
	vm.o(.bss)
	util.o(.text)
	util.o(.data)
	util.o(.bss)
/*
	mpentry.o(.text)
	mpentry.o(.data)
	mpentry.o(.bss)
*/
	mp.o(.text)
	mp.o(.data)
	mp.o(.bss)

	thr.o(.text)
	thr.o(.data)
	thr.o(.bss)
	tcb.o(.text)
	tcb.o(.data)
	tcb.o(.bss)



    	hpet.o(.text)
    	hpet.o(.data)
    	hpet.o(.bss)
	sig.o(.text)
	sig.o(.data)
	sig.o(.bss)
	cons.o(.text)
	cons.o(.data)
	cons.o(.bss)
	ata.o(.text)
	ata.o(.data)
	ata.o(.bss)
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

