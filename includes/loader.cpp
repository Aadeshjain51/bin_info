#include <bfd.h>
#include "loader.hpp"

/* FUNCTION: open_bfd
 * INPUT ARGUMENTS:
 * 	fname : name of the binary file to open
 * PROCESS:
 * 	a) open file
 * 	b) check if the file is indeed a binary executable file
 * 	c) check the file format (ELF, PE, UNKNOWN)
 * RETURN VALUE:
 * 	static bfd * : pointer to bfd structure defining the binary
 */
static bfd *
open_bfd(std :: string &fname) {
	bfd		*bfd_h;			/* pointer to binary executable file */
	static int	bfd_init_flag = 0;

	if ( !bfd_init_flag ) {
		bfd_init();		/* initialize internal data structures of libbfd */
		bfd_init_flag = 1;	/* set flag appropriatly */
	}

	if ( !( bfd_h = bfd_openr(fname.c_str(), NULL) ) ) {
		fprintf(stderr, "[!!] Failed to open binary '%s' (%s)\n",
			 fname.c_str(), bfd_errmsg(bfd_get_error()));
		return NULL;
	}

	/* bfd_object: describes executable, relocatable object or shared library */
	if ( !bfd_check_format(bfd_h, bfd_object) ) {
		fprintf(stderr, "[!!] File '%s' does not appear to be an executable (%s)\n",
			fname.c_str(), bfd_errmsg(bfd_get_error()));
		return NULL;
	}

	/* some versions of bfd_check_format set a wrong format error before detecting 
	 * format and then neglect to unset it once format has been detected, we unset
	 * it manually to prevent issues.
	 */
	bfd_set_error(bfd_error_no_error);

	/* bfd flavours: binary format (elf, coff, msdos, mach_o, etc. ) */
	if ( bfd_get_flavour(bfd_h) == bfd_target_unknown_flavour ) {
		fprintf(stderr, "[!!] Unrecognized format for binary '%s' (%s)\n",
			fname.c_str(), bfd_errmsg(bfd_get_error()));
		return NULL;
	}

	return bfd_h;
}

/* FUNCTION: load_symbols_bfd
 * INPUT ARGUMENTS:
 * 	bfd_h	: binary's bfd headers (bfd internal representation)
 * 	bin	: binary's object (program internal representation)
 * PROCESS:
 * 	a) read size of symbol table in binary file
 * 	b) allocate heap space to store symbol table entries
 * 	c) read symbol table
 * 	d) tag function symbols, local & global symbols and debugging symbols
 * 	e) cleanup and return
 * RETURN VALUE:
 * 	static int : status code
 * 		 0 - success
 * 		-1 - failure 
 */
static int
load_symbols_bfd(bfd *bfd_h, Binary *bin) {
	int	ret;
	long	n, nsyms, i;		/* n: total size of symbol table (in bytes)
					 * nsysm: number of symbols in binary
					 * i: loop iterator
					 */
	asymbol		**bfd_symtab;	/* symbol table */
	Symbol		*sym;		/* single symbol instance */

	bfd_symtab = NULL;

	/* get size of symbol table */
	if ( ( n = bfd_get_symtab_upper_bound(bfd_h) ) < 0 ) {
		fprintf(stderr, "[!!] Failed to read symtab (%s)\n",
			bfd_errmsg(bfd_get_error()));
		goto fail;
	} else if ( n ) {
		/* allocate memory for symbol table */
		if ( !( bfd_symtab = (asymbol **) malloc(n) ) ) {
			fprintf(stderr, "[!!] Out of memory\n");
			goto fail;
		}

		/* read symbols from binary */
		if ( ( nsyms = bfd_canonicalize_symtab(bfd_h, bfd_symtab) ) < 0 ) {
			fprintf(stderr, "[!!] Failed to read symbols table (%s)\n",
				bfd_errmsg(bfd_get_error()));
			goto fail;
		}

		for ( i = 0; i < nsyms; ++i ) {
			/* create program internal instance of symbol */
			bin -> symbols.push_back(Symbol());
			sym = &bin -> symbols.back();
			
			/* populate information in instance of symbol */
			sym -> name = std :: string(bfd_symtab[i] -> name);

			/* format symbol name if too large */
			if ( sym -> name.size() > MAX_SYM_NAME_LEN ) {
				sym -> name[36] = '.';
				sym -> name[37] = '.';
				sym -> name[38] = '.';
				sym -> name[39] = '\0';
			}


			sym -> addr = bfd_asymbol_value(bfd_symtab[i]);

			/* dynamic symbols associated with functions */
			if ( bfd_symtab[i] -> flags & BSF_FUNCTION )
				sym -> type = sym -> type | Symbol :: SYM_TYPE_FUN;
			/* dynamic symbols associated with local symbols */
			if ( bfd_symtab[i] -> flags & BSF_LOCAL)
				sym -> type = sym -> type | Symbol :: SYM_TYPE_LOC;
			/* dynamic symbols associated with global symbols */
			if ( bfd_symtab[i] -> flags & BSF_GLOBAL)
				sym -> type = sym -> type | Symbol :: SYM_TYPE_GLB;
			/* dynamic symbols associated with debugging symbols */
			if ( bfd_symtab[i] -> flags & BSF_DEBUGGING )
				sym -> type = sym -> type | Symbol :: SYM_TYPE_DBG;
		}
	}

	ret = 0;
	goto cleanup;

	fail:
		ret = -1;
	cleanup:
		if ( bfd_symtab ) free(bfd_symtab);

	return ret;
}

/* FUNCTION: load_dynsym_bfd
 * INPUT ARGUMENTS:
 * 	bfd_h	: binary's bfd headers (bfd internal representation)
 * 	bin	: binary's object (program internal representation)
 * PROCESS:
 * 	a) read size of dynamic symbol table in binary file
 * 	b) allocate heap space to store dynamic symbol table entries
 * 	c) read dynamic symbol table
 * 	d) tag function symbols, local & global symbols and debugging symbols
 * 	e) cleanup and return
 * RETURN VALUE:
 * 	static int : status code
 * 		 0 - success
 * 		-1 - failure 
 */
static int
load_dynsym_bfd(bfd *bfd_h, Binary *bin) {
	int	ret;
	long	n, nsyms, i;		/* n: total size of dynamic symbol table (in bytes)
					 * nsysm: number of dynamic symbols in binary
					 * i: loop iterator
					 */
	asymbol	**bfd_dynsym;		/* dynamic symbol table */
	Symbol	*sym;			/* single symbol instance */

	bfd_dynsym = NULL;

	/* get size of dynamic symbol table */
	if ( ( n = bfd_get_dynamic_symtab_upper_bound(bfd_h) ) < 0 ) {
		fprintf(stderr, "[!!] Failed to read dynamic symtab (%s)\n",
			bfd_errmsg(bfd_get_error()));
		goto fail;
	} else if ( n ) {
		/* allocate memory for dynamic symbol table */
		if ( !( bfd_dynsym = (asymbol **)malloc(n) ) ) {
			fprintf(stderr, "[!!] Out of memory\n");
			goto fail;
		}

		/* read dynamic symbols from binary */
		if ( ( nsyms = bfd_canonicalize_dynamic_symtab(bfd_h, bfd_dynsym) ) < 0 ) {
			fprintf(stderr, "[!!] Failed to read dynamic symbol table (%s)\n",
				bfd_errmsg(bfd_get_error()));
			goto fail;
		}
		
		for ( i = 0; i < nsyms; ++i ) {
			/* create program internal instance of symbol */
			bin -> symbols.push_back(Symbol());
			sym = &bin -> symbols.back();
			
			/* populate information in instance of symbol */
			sym -> name = std :: string(bfd_dynsym[i] -> name);

			/* format symbol name if too large */
			if ( sym -> name.size() > MAX_SYM_NAME_LEN ) {
				sym -> name[36] = '.';
				sym -> name[37] = '.';
				sym -> name[38] = '.';
				sym -> name[39] = '\0';
			}

			sym -> addr = bfd_asymbol_value(bfd_dynsym[i]);

			/* dynamic symbols associated with functions */
			if ( bfd_dynsym[i] -> flags & BSF_FUNCTION )
				sym -> type = Symbol :: SYM_TYPE_FUN;
			/* dynamic symbols associated with local symbols */
			if ( bfd_dynsym[i] -> flags & BSF_LOCAL)
				sym -> type = Symbol :: SYM_TYPE_LOC;
			/* dynamic symbols associated with global symbols */
			if ( bfd_dynsym[i] -> flags & BSF_GLOBAL)
				sym -> type = Symbol :: SYM_TYPE_GLB;
			/* dynamic symbols associated with debugging symbols */
			if ( bfd_dynsym[i] -> flags & BSF_DEBUGGING )
				sym -> type = Symbol :: SYM_TYPE_DBG;
		}
	}

	ret = 0;
	goto cleanup;

	fail:
		ret = -1;
	
	cleanup:
		if ( bfd_dynsym ) free(bfd_dynsym);
	
	return ret;
}

/* FUNCTION: load_sections_bfd
 * INPUT ARGUMENTS:
 * 	bfd_h	: binary's bfd headers (bfd internal representation)
 * 	bin	: binary's object (program internal representation)
 * PROCESS:
 * 	a) for each node in linked list representation of sections of binary
 * 		a1) retrieve section flags and set appropriate section type
 * 		a2) retrieve section virtual memory address, size, and name
 * 		a3) populate information in program internal representation of binary section
 * 		a4) allocate size to store section contents
 * 		a5) retrieve section contents and store them
 * RETURN VALUE:
 * 	static int : status code
 * 		 0 - success
 * 		-1 - failure 
 */
static int
load_sections_bfd(bfd *bfd_h, Binary *bin) {
	int			bfd_flags;	/* flags of given section. eg: code, data */
	uint64_t		vma, size;	/* vma: virtual memory address to load the seciton at
						 * size: size of section to load
						 */
	const char		*secname;	/* section name */
	asection		*bfd_sec;	/* bfd internal representation of section */
	Section			*sec;		/* program internal representation of section */
	Section	:: SectionType	sectype;	/* program internal representation of section type */


	for ( bfd_sec = bfd_h -> sections; bfd_sec; bfd_sec = bfd_sec -> next ) {
		bfd_flags	= bfd_section_flags(bfd_sec);


		/* set appropriate section type */
		sectype		= Section :: SEC_TYPE_NONE;
		/* if the current section belongs to one of the code sections */
		if ( bfd_flags & SEC_CODE ) {
			sectype = Section :: SEC_TYPE_CODE;
		/* if the current section belongs to one of the data sections */
		} else if ( bfd_flags & SEC_DATA ) {
			sectype = Section :: SEC_TYPE_DATA;
		/* if the current section belongs to none of the code or data sections */
		} else {
			/* skip further processing and go for next section */
			continue;
		}

		vma	= bfd_section_vma(bfd_sec);	/* get virtual memory address */
		size	= bfd_section_size(bfd_sec);	/* get size of section */
		secname = bfd_section_name(bfd_sec);	/* get section name */

		if ( !secname ) secname = "<unnamed>";	/* if failed to retrieve section name */

		/* create program internal instance of section for binary */
		bin -> sections.push_back(Section());
		sec = &bin -> sections.back();

		/* populate information in instance of section */
		sec -> binary	= bin;
		sec -> name	= std :: string(secname);
		sec -> type	= sectype;
		sec -> vma	= vma;
		sec -> size	= size;
	
		/* allocate memory to store section contents */
		if ( !( sec -> bytes = (uint8_t *)malloc(size) ) ) {
			fprintf(stderr, "[!!] Out of memory\n");
			return -1;
		}

		/* retrieve section contents */
		if ( !bfd_get_section_contents(bfd_h, bfd_sec, sec -> bytes, 0, size) ) {
			fprintf(stderr, "[!!] Failed to read section '%s' (%s)\n",
				secname, bfd_errmsg(bfd_get_error()));
			return -1;
		}
	}

	return 0;
}

/* FUNCTION: load_binary_bfd
 * INPUT ARGUMENTS:
 * 	fname	: name of binary file to load
 * 	bin	: binary's object (program internal representation)
 * 	type	: supported types of binary file
 * PROCESS:
 * 	a) set filename and entry point in 'bin'
 * 	b) set executalbe type in 'bin'
 * 	c) set target architecture type in 'bin'
 * 	d) load static symbols (if present)
 * 	e) load dynamic symbols
 * 	f) load sections
 * 	g) cleanup
 * RETURN VALUE:
 * 	static int : status code
 * 		 0 - success
 * 		-1 - failure 
 */
static int
load_binary_bfd(std :: string fname, Binary *bin, Binary :: BinaryType type) {
	int				ret;
	bfd				*bfd_h;
	const bfd_arch_info_type	*bfd_info;

	bfd_h	= NULL;
	bfd_h	= open_bfd(fname);

	if ( !bfd_h ) {
		goto fail;
	}

	/* setting general information */
	bin -> filename	= std :: string(fname);			/* executable name */
	bin -> entry	= bfd_get_start_address(bfd_h);		/* executable entry point */

	/* setting appropriate executable type */
	bin -> type_str = std :: string(bfd_h -> xvec -> name);

	switch ( bfd_h -> xvec -> flavour ) {
		case bfd_target_elf_flavour:
			bin -> type = Binary :: BIN_TYPE_ELF;
			break;
		case bfd_target_coff_flavour:
			bin -> type = Binary :: BIN_TYPE_PE;
			break;
		case bfd_target_unknown_flavour:
		default:
			fprintf(stderr, "unsupported binary type (%s)\n", bfd_h -> xvec -> name);
			goto fail;
	}

	/* setting appropriate executable architecture */
	bfd_info	= bfd_get_arch_info(bfd_h);
	bin -> arch_str = std :: string(bfd_info -> printable_name);

	switch ( bfd_info -> mach ) {
		case bfd_mach_i386_i386:
			bin -> arch = Binary :: ARCH_X86;
			bin -> bits = 32;
			break;
		case bfd_mach_x86_64:
			bin -> arch = Binary :: ARCH_X86;
			bin -> bits = 64;
			break;
		default:
			fprintf(stderr, "unsupported architecture (%s)\n", bfd_info -> printable_name);
		       goto fail;
	}

	/* symbols may not be present if the binary is stripped */
	load_symbols_bfd(bfd_h, bin);	/* attempt to load static symbols */
	load_dynsym_bfd(bfd_h, bin);	/* attempt to load dynamic symbols */

	/* attempt to load sections */
	if ( load_sections_bfd(bfd_h, bin) < 0 ) goto fail;

	ret = 0;
	goto cleanup;

	fail:
		ret = -1;
	
	cleanup:
		if ( bfd_h ) bfd_close(bfd_h);
	
	return  ret;
}

/* FUNCTION: load_binary
 * INPUT ARGUMENTS:
 * 	fname	: name of binary file to examine
 * 	bin	: binary's object (program internal representation)
 * 	type	: supported types of binary file
 * PROCESS:
 * 	a) load the binary and examine
 * RETURN VALUE:
 * 	int : status code
 * 		 0 - success
 * 		-1 - failure 
 */
int
load_binary(std :: string &fname, Binary *bin, Binary :: BinaryType type) {
	return load_binary_bfd(fname, bin, type);
}

/* FUNCTION: unload_binary
 * INPUT ARGUMENTS:
 * 	bin : binary's object (program internal representation)
 * PROCESS:
 * 	a) for each section in binary's object
 * 		a1) free space allocated to store its contents
 * RETURN VALUE: NONE
 */
void
unload_binary(Binary *bin) {
	size_t	i;	/* loop iterator */
	Section *sec;	/* program internal representation of sections */

	for ( i = 0; i < bin -> sections.size(); ++i ) {
		sec = &bin -> sections[i];	/* get the section in binary */
		if ( sec -> bytes )
			free(sec -> bytes);	/* de-allocate memory space used to store its contents */
	}
}

/* FUNCTION: raw_dump
 * INPUT ARGUMENTS:
 * 	sec : section who's content are to be printed as raw bytes
 * PROCESS:
 * 	a) for each byte in section
 * 		a1) print hexadecimal value of each byte
 * 		a2) print corresponding ascii character if it exists
 * RETURN VALUE: NONE
 */
void
raw_dump(Section *sec) {
	size_t		i, j, size;			/* i, j: loop iterators
					 		* size: size of section
					 		*/
	
	char		line[MAX_LINE_LEN + 1];		/* string to store a line to print */
	char		ascii_code;			/* single character/byte in section content */

	size = sec -> size;

	/* loop over each byte in section */
	for ( i = 0; i < size; ++i ) {
		ascii_code = sec -> bytes[i];

		/* print hex value for each byte */
		printf(" %02x", (uint8_t) ascii_code);
		
		/* add character to print to line and print the line */
		line[i % MAX_LINE_LEN] = (ascii_code >= 32 && ascii_code <=128) ? ascii_code : '.';

		if ( ( i != 0 && ( i + 1 ) % MAX_LINE_LEN == 0 ) || i == size - 1 ) {
			line[i % MAX_LINE_LEN + 1] = '\0';

			/* additional space for last line less than maximum length */
			for ( j = strlen(line); j < MAX_LINE_LEN; ++j )
				printf("   ");

			/* column to separate hex and character columns */
			printf("  ");

			printf("%s\n", line);
		}
	}
}
