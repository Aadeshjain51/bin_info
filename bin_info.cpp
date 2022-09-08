#include <cstdio>
#include <cstdint>
#include <cstring>
#include <unistd.h>
#include "includes/loader.hpp"

extern void disasm(Binary &bin);
void usage(char *);

/* FUNCTION: main
 * INPUT ARGUMENTS:
 * 	argc	: count of arguments passed to program
 * 	argv	: array of arguments passed to program
 * PROCESS:
 * 	a) verify if proper number of arguments have been passed
 * 	b) load the binary executable
 * 	f) cleanup
 * RETURN VALUE:
 * 	int : status code
 * 		0 - success
 * 		1 - failure
 */
int
main(int argc, char **argv) {
	int		opt;		/* command line option */
	uint8_t		examine_header;	/* flag to explore binary header structure*/
	uint8_t		linear_disasm;	/* flag to perform linear disassembly of binary */
	Binary		bin;		/* program internal representation of binary as an object */
	std :: string	fname;		/* filename of binary executable to be loaded for inspection */

	examine_header	= 0;
	linear_disasm	= 0;
	
	while( (opt = getopt(argc, argv, "f:xlh")) != EOF) {
		switch(opt) {
			case 'f':
				fname.assign(optarg);	break;
			case 'x':
				examine_header = 1;	break;
			case 'l':
				linear_disasm = 1;	break;
			case 'h':
			case '?':
			default:
				usage(argv[0]);
				return -1;
		}
	}


	if ( load_binary(fname, &bin, Binary :: BIN_TYPE_AUTO) < 0 ) {
		return 1;
	}

	if ( examine_header )
		print_binary_header(bin);
	if ( linear_disasm )
		disasm(bin);

	unload_binary(&bin);

	return 0;
}

/* FUNCTION: usage
 * INPUT ARGUMENTS:
 * 	program: path of this program
 * PROCESS:
 * 	a) print usage and available options
 * RETURN VALUE: NONE
 */
void usage(char *program) {
	printf("Usage: %s [options] <binary>\n", program);
	printf("Options:\n");
	printf("\t-f FILENAME\t\tpass file name\n");
	printf("\t-x         \t\textract binary header information\n");
	printf("\t-l         \t\tperform linear disassembly\n");
}

