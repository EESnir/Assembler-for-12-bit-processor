#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H
#include "assembler.h"

#define SYMTBL_MAX_LEN 1000
enum ATTRIBUTES_ENUM {CODE = 1, DATA = 2, EXTERNAL = 4};
enum MYMALLOC {ALOCATE, FREE};

typedef struct SymbolTable {
	char *sym_names[SYMTBL_MAX_LEN];
	int sym_values[SYMTBL_MAX_LEN];
	int sym_attributes[SYMTBL_MAX_LEN];
	int len;
} symTbl;

typedef struct isaStructure {
	char *name[INSTRCT_N + 1];
	int *n_operands[INSTRCT_N + 1];
	int *opcode[INSTRCT_N + 1];
	int *func[INSTRCT_N + 1];
	int (*llegal_src_add[INSTRCT_N + 1])[MAX_LEG_ADR];
	int (*llegal_dst_add[INSTRCT_N + 1])[MAX_LEG_ADR];
} isa_struct;

/* structure used to hold lines of text so that each line is associated the source line number */
typedef struct LineListStruct {
	char line_list[FILE_MAX_LINES][MAX_LINE_TRUE_LEN];
	int src_line_n[FILE_MAX_LINES];
	char *instruction[FILE_MAX_LINES];
	int len;
} lineList;

void *mymalloc(size_t n_bytes, int aloc__free); /* used only in symbol table */
void insertSymbol(symTbl *st, char *symbol_name, int value, int attribute_enum);

#endif
