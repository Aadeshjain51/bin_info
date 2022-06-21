#ifndef BIN_LOADER_H
#define BIN_LOADER_H

#include <cstdint>
#include <string>
#include <vector>

#define MAX_SYM_NAME_LEN	38		/* maximum length of a symbol name to be displayed upto */

class Binary;
class Section;
class Symbol;

/* Identifies symbols describing (currenty only functions) */
class Symbol {
	public:
		enum SymbolType {		/* Type of symbol */
			SYM_TYPE_UNK	= 0x0,	/* Unkown symbol type */
			SYM_TYPE_FUN	= 0x1,	/* Function symbol */
			SYM_TYPE_LOC	= 0x2,	/* Local symbol */
			SYM_TYPE_GLB	= 0x4,	/* Global symbol */
			SYM_TYPE_DBG	= 0x8	/* Debugging symbol */
		};

		uint8_t		type;
		std :: string	name;	/* Symbol name */
		uint64_t	addr;	/* address of symbol */

		Symbol() : type(SYM_TYPE_UNK), name(), addr(0) {}
};

/*Indentifies sections as either CODE section or DATA section */
class Section {
	public:
		enum SectionType {		/* Type of section */
			SEC_TYPE_NONE	= 0,	/* Neither code, nor data */
			SEC_TYPE_CODE	= 1,	/* Code section */
			SEC_TYPE_DATA	= 2	/* Data section */
		};

		Binary		*binary;	/* Pointer to binary object containing the section */
		std :: string	name;		/* Section name */
		SectionType	type;
		uint64_t	vma;		/* Virtual memory address to be loaded to */
		uint64_t	size;		/* Section size in bytes */
		uint8_t		*bytes;

		Section() : binary(NULL), type(SEC_TYPE_NONE), vma(0), size(0), bytes(NULL) {}

		/* Return TRUE if an address is within this section, else return FALSE */
		bool contains(uint64_t addr) { return ( addr >= vma ) && ( addr - vma < size ); }
};

/* The binary executable to inspected */
class Binary {
	public:
		enum BinaryType {		/* Type of binary file */
			BIN_TYPE_AUTO	= 0,	/* Currently unsupported file format */
			BIN_TYPE_ELF	= 1,	/* Executable and Linkable Format (ELF) file */
			BIN_TYPE_PE	= 2	/* Portable Executable file */
		};

		enum BinaryArch {		/* Target architecture of binary file */
			ARCH_NONE	= 0,	/* Currenty unsupported architecture */
			ARCH_X86	= 1, 	/* x86 OR amd64 file */
		};

		std :: string		filename;	/* Name of binary */
		BinaryType		type;
		std :: string		type_str;	/* Description of binary type */
		BinaryArch		arch;
		std :: string		arch_str;	/* Description of target architecture */
		uint32_t		bits;		/* 32 for x64, 64 for amd64 */
		uint64_t		entry;		/* Entry point of the binary*/
		std :: vector <Section> sections;	/* All sections in binary file */
		std :: vector <Symbol>	symbols;	/* All symbols in binary file */	

		Binary() : type(BIN_TYPE_AUTO), arch(ARCH_NONE), bits(0), entry(0) {}

		/* Return pointer to .text section of binary, if locatable */
		Section * get_text_section() {
			for ( auto &s : sections )
				if ( s.name == ".text" )
					return &s;
			return NULL;
		}
};

/* Load binary for inspection */
int load_binary(std :: string &fname, Binary *bin, Binary :: BinaryType type);

/* Unload binary, once inspection completes */
void unload_binary(Binary *bin);

#endif /* BIN_LOADER_H */
