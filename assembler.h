/* assembler.h
 *
 * This file contains the main user API for the assembler program */
#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#define ERR_RET -1
#define OK_RET 1;
#define MIN_PROG_ARGS 2
#define MAX_LINE_LEN 80 /* maximum allowed src code line length */
#define MAX_LINE_TRUE_LEN MAX_LINE_LEN + 2
#define ERR_MSG_LEN 300
#define MAX_FILENAME 1024
#define FILE_MAX_LINES 1024
#define SRC_FILE_EXT ".as"
#define ENT_FILE_EXTENS ".ent"
#define EXT_FILE_EXTENS ".ext"
#define OBJ_FILE_EXTENS ".obj"
#define MAX_CODE_LINES_N 1024 /* output codelines */
#define MAX_CODE_LINE_LEN 11
#define MAX_ENT_EXT_LINE_LEN LABEL_MAX_LEN + 6 /* .ent .ext maximum line len */
#define MAX_OUT_FILEPATH_LEN 1024

#define PREP_LINE_LENGTH_ERR -2
#define LABEL_MAX_LEN 31
#define LABEL_BUFF_LEN LABEL_MAX_LEN + 2
#define REGS_NAME_MAX_LEN 3
#define DIR_NAME_MAX_LEN 9 /* .string .external .entry .data*/
#define LABEL_ERROR -2
enum C_OUTPUT_FILE {CREATE, DESTROY}; /* output products function argument enum */
enum ADD_METHDS {IMMEDIATE = 0, DIRECT = 1, RELATIVE = 2, REG_DIRECT = 3};
enum DIRECTIVE_IDX_ENUM {DATA_DIR_IDX = 0, STR_DIR_IDX = 1, ENT_DIR_IDX = 2,  EXT_DIR_IDX = 3};
#define MAX_LEG_ADR 3 /* maximum length of legal addressing methods array */

#define RELATIVE_PREFIX '%'
#define IMMEDIATE_PREFIX '#'
#define REGS_N 8
#define REG_NAMES {"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", NULL}
#define IC_INIT (100)
#define INSTRCT_MAX_LEN 5	/* the maximum allowed length of an instruction including \0 , strlen("stop") + 1 */
#define INSTRCT_NAMES {"mov", "cmp", "add", "sub", "lea", "clr", "not", "inc", "dec", "jmp", \
						"bne", "jsr", "red", "prn", "rts", "stop"}
#define MAX_N_OPS 2
#define INSTRCT_N 16
#define INSTRCT_OPERANDS_N			{2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0}
#define INSTRCT_OPCODES    			{0, 1,  2,  2, 4,  5,  5,  5,  5,  9,  9, 9, 12, 13, 14, 15}
#define INSTRCT_FUNC_CODES 			{0, 0, 10, 11, 0, 10, 11, 12, 13, 10, 11, 12, 0,  0,  0,  0}
#define INSTRCT_SRC_LEGAL_ADRSNG 	{{0, 1, 3}, {0, 1, 3}, {0, 1, 3}, {0, 1, 3}, {1, -1, -1}, {0, -1, -1}, {0, -1, -1}, \
									{0, -1, -1}, {0, -1, -1}, {0, -1, -1}, {0, -1, -1}, {0, -1, -1}, {0, -1, -1}, \
									{0, -1, -1}, {0, -1, -1}, {0, -1, -1}}
#define INSTRCT_DST_LEGAL_ADRSNG 	{{1, 3, -1}, {0, 1, 3}, {1, 3, -1}, {1, 3, -1}, {1, 3, -1}, {1, 3, -1}, {1, 3, -1}, \
									{1, 3, -1}, {1, 3, -1}, {1, 2, -1},{1, 2, -1}, {1, 2, -1}, {1, 3, -1}, {0, 1, 3}, \
									{0, -1, -1}, {0, -1, -1}}


#endif
