#include <cstdio>
#include <cstdint>
#include <cstring>
#include "includes/loader.hpp"
#include "includes/ansi_colors.hpp"

/* FUNCTION: main
 * INPUT ARGUMENTS:
 * 	argc	: count of arguments passed to program
 * 	argv	: array of arguments passed to program
 * PROCESS:
 * 	a) verify if proper number of arguments have been passed
 * 	b) load the binary executable
 * 	c) print name, type, target architecture, size and entry point of binary
 * 	d) dump section information of binary
 * 	e) dump symbol information of binary
 * 	f) cleanup
 * RETURN VALUE:
 * 	int : status code
 * 		0 - success
 * 		1 - failure
 */
int
main(int argc, char **argv) {
	size_t		i;		/* loop iterator */
	Binary		bin;		/* program internal representation of binary as an object */
	Section		*sec;		/* program internal representation of sections of a binary */
	Symbol		*sym;		/* program internal representation of symbols in a binary */
	std :: string	fname;		/* filename of binary executable to be loaded for inspection */

	if ( argc != 2 ) {
		printf("Usage: %s <binary>\n", argv[0]);
		return 1;
	}

	fname.assign(argv[1]);

	if ( load_binary(fname, &bin, Binary :: BIN_TYPE_AUTO) < 0 ) {
		return 1;
	}

	/* print information concering entire binary executable */
	underlined_red();
	printf("[*] Loaded binary '%s'\n", bin.filename.c_str());
	bold_blue();
	printf("[*] Architecture: %s/%s (%u bits)\n", bin.type_str.c_str(), bin.arch_str.c_str(), bin.bits);
	bold_yellow();
	printf("[*] Entry point: 0x%016jx\n\n", bin.entry);

	/* print information regarding section headers */
	red();
	printf("[*] Scanned section headers:\n");
	yellow();
	printf(" %s %13s %8s %20s\n", &"VIRT ADDR", &"SIZE", &"NAME", &"TYPE");
	reset_color();
	printf(" %s %44s\n", &"RAW BYTES", &"ASCII");

	for ( i = 0; i < bin.sections.size(); ++i ) {
		sec = &bin.sections[i];
		yellow();
		printf("\n 0x%016jx %-8ju %-20s %s\n",
			sec -> vma, sec -> size, sec -> name.c_str(),
			sec -> type == Section :: SEC_TYPE_CODE ? "CODE" : "DATA");
		reset_color();
		raw_dump(sec);
	}

	/* print information regrading symbols (if present) */
	if ( bin.symbols.size() > 0 ) {
		printf("\n");
		red();
		printf("[*] Scanned symbol tables:\n");
		blue();
		printf(" %-40s %18s %20s\n", &"NAME", &"ADDRESS", &"SYMBOL TYPE");
		reset_color();
		
		for ( i = 0; i < bin.symbols.size(); ++i ) {
			sym = &bin.symbols[i];

			printf(" %-40s 0x%016jx ", sym -> name.c_str(), sym -> addr);
			if ( sym -> type & Symbol :: SYM_TYPE_FUN )
				printf("%20s", &"FUNCTION");
			if ( sym -> type & Symbol :: SYM_TYPE_LOC )
				printf("%20s", &"LOCAL-SYMBOL");
			if ( sym -> type & Symbol :: SYM_TYPE_GLB )
				printf("%20s", &"GLOBAL-SYMBOL");
			if ( sym -> type & Symbol :: SYM_TYPE_DBG )
				printf("%20s", &"DEBUGGING-SYMBOL");
			printf("\n");
		}
	}

	unload_binary(&bin);

	return 0;
}
