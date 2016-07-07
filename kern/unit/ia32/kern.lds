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
    	boot.o(.data)
    	boot.o(.bss)
	setup.o(.text)
	setup.o(.data)
	setup.o(.bss)
	real.o(.text)
	real.o(.data)
	real.o(.bss)
        mp.o(.text)
        mp.o(.data)
        mp.o(.bss)
/*
	vga.o(.text)
	vga.o(.data)
	vga.o(.bss)
*/
	vbe.o(.text)
	vbe.o(.data)
	vbe.o(.bss)
	bmfonts.o(.text)
	bmfonts.o(.data)
	bmfonts.o(.bss)
	main.o(.text)
	main.o(.data)
	main.o(.bss)
	protmode.o(.text)
	protmode.o(.data)
	protmode.o(.bss)
	trap.o(.text)
	trap.o(.data)
	trap.o(.bss)
	m_trap.o(.text)
	m_trap.o(.data)
	m_trap.o(.bss)
	tss.o(.text)
	tss.o(.data)
	tss.o(.bss)
	pic.o(.text)
	pic.o(.data)
	pic.o(.bss)

	seg.o(.text)
	seg.o(.data)
	seg.o(.bss)

	mem.o(.text)
	mem.o(.data)
	mem.o(.bss)
	pool.o(.text)
	pool.o(.data)
	pool.o(.bss)
	slab.o(.text)
	slab.o(.data)
	slab.o(.bss)

	page.o(.text)
	page.o(.data)
	page.o(.bss)

	vm.o(.text)
	vm.o(.data)
	vm.o(.bss)

	util.o(.text)
	util.o(.data)
	util.o(.bss)
	debug.o(.text)
	debug.o(.data)
	debug.o(.bss)
/*
	mpentry.o(.text)
	mpentry.o(.data)
	mpentry.o(.bss)
*/
	mp.o(.text)
	mp.o(.data)
	mp.o(.bss)

	proc.o(.text)
	proc.o(.data)
	proc.o(.bss)

/*
	task.o(.text)
	task.o(.data)
	task.o(.bss)
*/
	m_task.o(.text)
	m_task.o(.data)
	m_task.o(.bss)

/*
	sched.o(.text)
	sched.o(.data)
	sched.o(.bss)
*/

    	apic.o(.text)
    	apic.o(.data)
    	apic.o(.bss)
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
	ac97.o(.text)
	ac97.o(.data)
	ac97.o(.bss)
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
	*(EXCLUDE_FILE(*usr.o *test.o).text*)
	*(.text*)
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

