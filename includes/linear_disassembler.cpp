#include <string>
#include <capstone/capstone.h>
#include "loader.hpp"
#include "ansi_colors.hpp"

/* FUNCITON: disasm
 * INPUT ARGUMENTS:
 * 	bin : binary file loaded
 * PROCESS:
 * 	a) Retreive .text section of binary
 * 	b) Create an instance of capstone handler (dis)
 * 	c) disassemble all the instuctions in .text section and store in local variable 'insns'
 * 	d) print the disassembled instructions from insns
 * RETURN VALUE:
 *	int : statue code
 *		 0 - disassembled
 *		-1 - failure
 */
int
disasm(Binary &bin) {
    csh         dis;		/* handler to capstone api */
    cs_insn     *insns;		/* capstone internal data structure to store disassembly */
    Section     *text;		/* .text section of binary */
    size_t      n;		/* number of instructions decoded */

    text = bin.get_text_section();

    if ( !text ) {
        fprintf(stderr, "Nothing to disassemble\n");
        return 0;
    }

    if ( cs_open(CS_ARCH_X86, CS_MODE_64, &dis) != CS_ERR_OK ) {
        fprintf(stderr, "Failed to open Capstone");
        return -1;
    }

    n = cs_disasm(dis, text -> bytes, text -> size, text -> vma, 0, &insns);

    if ( n <= 0 ) {
        fprintf(stderr, "Disassembly error: %s\n", cs_strerror(cs_errno(dis)));
        return -1;
    }

    red();
    printf("[*] Disassembly of .text section:\n");
    reset_color();
    for ( size_t i = 0; i < n; ++i ) {
	green();
        printf("0x%016jx: ", insns[i].address);
	reset_color();
        
        for ( size_t j = 0; j < 16; ++j ) {
            (j < insns[i].size) ? printf("%02x ", insns[i].bytes[j]) : printf("   ");
        }

        printf("%-8s %s\n", insns[i].mnemonic, insns[i].op_str);

    }

    cs_free(insns, n);
    cs_close(&dis);

    return 0;
}
