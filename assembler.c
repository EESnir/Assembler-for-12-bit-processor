/*
 ============================================================================
 Name        : assembler.c
 Author      : snirm15@gmail.com
 Version     : 21/3/21
 Copyright   :
 Description : Assembler
 ============================================================================

 Program Objective:
 *****************
 This is an assembler program for an imaginary machine architecture.
 It processes files that their path is given as program argument one by one.
 Each file is checked for error and them translated to 1-3 output files - for file name file.as:
 	 1) file.obj - Object file that contains the machine code for instructions and initialized data segment.
 	 2) (optional) file.ent - file that holds a list of global labels and their addresses in the machine code.
 	 	 this file will not be generated if the code does not contain .entry directives.
	 3) (optional) file.ext - holds the addresses of words that should be later replaced with an external
	 	 	 	   label address

 Method of Operation:
 *******************
 The program reads each file line by line, each line is checked and normalized in terms of whitespace repetition
 and trailing/preceding whitespace truncation.
 Then, the lines are separated to instructions and directives while keeping track of all symbols witnessed in
 the source code in a symbol table, in order to detect use the of labels that haven't been assigned an address yet
 because data segment is not yet built at that point. entry definition are stored in ent_line array.
 After that, the code is built by mapping instructions ,values, opcodes, addressing methods from a structure
 called isa_struct (ISA for - Instruction Set Architecture), and address from the symbol table to a string
 containing also and the A, R, E flag, this string is saved in the global code_lines 2d array. Code lines that depend
 on addresses of labels who's addresses still unknown, are temporarily stored in code_line array as the label name only,
 to later be replaced when the data is built and given an address. if a label is declared extern a line is writen to ext_lines.
 In the last stage of processing, the directive part of line is scanned, for each directive that initializes data,
 the data is read from line and encoded to the data_lines 2d array, and the label (if exists) set to it
 is saved in the symbol table, the address of the first element of data is set to all temporary labels in code_lines.
 after the entire table is built, a scan for entry directives is performed to make sure no violations, building the .ent file.
 If all stages are done without errors, output files are generated.

 Structure:
 *********
 assembler.c - main function file, path processing functions.
 code_proc.c - contains functions to handle code processing.
 string_structs.c - contains data structures to help process text.
  	  	  	  	  	  symbol table structure, isa structure and line list structure, and their functions.
 Each has a .h file that is associated with it.

 Additional:
 **********
 The assembler will not stop processing a file (at least per stage) when an error detected, but no
 output products will be produce.
 This program was written with emphasis on simplicity, dynamic allocation was almost completely avoided.
 (used only in the symbol table in order to use with map function (which handles arrays of pointers).
 the only globals are the 2d arrays which store the strings to eventually printed to files, they were defined
 globals for the ease of sharing them between code processing functions and output writing function.

 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "assembler.h"
#include "code_proc.h"
#include "string_structs.h"

/* globals */
char code_lines[MAX_CODE_LINES_N][MAX_CODE_LINE_LEN + 1];
char data_lines[MAX_CODE_LINES_N][MAX_CODE_LINE_LEN + 1];
char ent_lines[MAX_CODE_LINES_N][MAX_ENT_EXT_LINE_LEN + 1];
char ext_lines[MAX_CODE_LINES_N][MAX_ENT_EXT_LINE_LEN + 1];

/* wrapper to fopen() that prints appropriate error message to the target string. */
FILE *myfopen(const char *filename, const char *mode) {
	FILE *return_value = NULL;
	char errormsg[ERR_MSG_LEN];

	return_value = fopen(filename, mode);
	if (!return_value) {
		sprintf(errormsg, "error opening file %s", filename);
		perror(errormsg);
	}
	return return_value;
}

/* Copies the directory path of from the input string (containing a full file path) to the target string. */
void getDirFromFilePathStr(char *path_str, char *target_str) {
	int len;
	char *ptr;

	len = strlen(path_str) - 1;
	for (ptr = path_str + len; ptr != path_str; --ptr)
		if ((*ptr == '\\') || (*ptr == '/')) {
			strncpy(target_str, path_str, ptr - path_str + 1);
			break;
		}
}



/* function to generate output files, if the content is empty it removes the file (except .obj which writes empty file) */
void createOutputFiles(int code_list_n, int data_list_n, int ent_list_n, int ext_list_n, char *filename, int create_destory) {
	int i;
	char full_file_path[MAX_OUT_FILEPATH_LEN];
	char ext_path[MAX_OUT_FILEPATH_LEN];
	char ent_path[MAX_OUT_FILEPATH_LEN];
	char obj_path[MAX_OUT_FILEPATH_LEN];
	FILE *f_ent, *f_ext, *f_obj;

	strncpy(full_file_path, filename, strlen(filename));
	sprintf(ext_path, "%s" EXT_FILE_EXTENS, full_file_path);
	sprintf(ent_path, "%s" ENT_FILE_EXTENS, full_file_path);
	sprintf(obj_path, "%s" OBJ_FILE_EXTENS, full_file_path);
	f_ext = myfopen(ext_path, "w");
	f_ent = myfopen(ent_path, "w");
	f_obj = myfopen(obj_path, "w");
	if (create_destory == DESTROY) {
		remove(ext_path);
		remove(ent_path);
		remove(obj_path);
		return;
	}
	if (create_destory == CREATE) {
		if (ext_list_n) { /* ext */
			for (i = 0; i < ext_list_n; ++i) {
				fprintf(f_ext, "%s\n", ext_lines[i]);
			}
			fclose(f_ext);
		}
		else {
			remove(ext_path);
		}

		if (ent_list_n) { /* ent */
			for (i = 0; i < ent_list_n; ++i) {
				fprintf(f_ent, "%s\n", ent_lines[i]);
			}
			fclose(f_ent);
		}
		else {
			remove(ent_path);
		}

		if (code_list_n) { /* obj */
			fprintf(f_obj, " %d %d\n", code_list_n, data_list_n);
			for (i = 0; i < code_list_n; ++i) {
				fprintf(f_obj, "%s\n", code_lines[i]);
			}
			for (i = 0; i < data_list_n; ++i) {
				fprintf(f_obj, "%s\n", data_lines[i]);
			}
			fclose(f_obj);
		}
	}
}

/* main function: for each argument in agv:
  			 -	try to open a file with the argument as path
  			  	if it fails, issue warning and continue to the next path
			 -	send file to 1st stage processing
			 	if it has errors print them and continue to next path after closing the file
			 -	send file to 2nd stage processing
				if it has errors print them and continue to next path after closing the file
			 -	create output files .obj .ent (opt) .ext (opt) */
int main(int argc, char **argv) {
	FILE *src_f;
	int file_number;
	char filename[MAX_FILENAME];
	symTbl symbol_table = {0};
	isa_struct isa = {0};
	lineList processed_line_list = {0}; /* this struct hold the src code after space trimming and normalization */
	lineList directive_line_list = {0};
	static const lineList empty_ll;
	static const symTbl empty_st;
	static char names[][INSTRCT_MAX_LEN] = INSTRCT_NAMES;
	static int n_operands[] = INSTRCT_OPERANDS_N;
	static int opcode[] = INSTRCT_OPCODES;
	static int func[] = INSTRCT_FUNC_CODES;
	static int llegal_src_add[][MAX_LEG_ADR] = INSTRCT_SRC_LEGAL_ADRSNG;
	static int llegal_dst_add[][MAX_LEG_ADR] = INSTRCT_DST_LEGAL_ADRSNG;
	int i, ic, dc, line_list_cnt, ext_list_n, ent_list_n, code_list_n, data_list_n;

	if (argc < MIN_PROG_ARGS) { /* check at least one file name provided */
		printf("no source file provided\nUsage: %s asm_file_name1 asm_file_name2\nwithout .as extension\n", *argv);
		return ERR_RET;
	}
	/* build imaginary architecture constants from .h file to isa struct */
	for (i = 0;  i < INSTRCT_N; ++i) {
		isa.name[i] = names[i];
		isa.n_operands[i] = &n_operands[i];
		isa.opcode[i] = &opcode[i];
		isa.func[i] = &func[i];
		isa.llegal_src_add[i] = &llegal_src_add[i];
		isa.llegal_dst_add[i] = &llegal_dst_add[i];
	}
	isa.name[i] = (char *) NULL;
	isa.n_operands[i] = (int *) NULL;
	isa.opcode[i] =  (int *) NULL;
	isa.func[i] = (int *) NULL;
	isa.llegal_src_add[i] =  NULL;
	isa.llegal_dst_add[i] =  NULL;

	for (file_number = 1; file_number < argc; ++file_number) { /* for each file in the argv list */
		sprintf(filename, "%s%s", argv[file_number], SRC_FILE_EXT);
		if ((src_f = myfopen(filename, "r"))) {
			memset(code_lines, 0, sizeof(code_lines[0][0]) * MAX_CODE_LINES_N * MAX_CODE_LINE_LEN + 1);
			code_list_n = 0;
			memset(data_lines, 0, sizeof(data_lines[0][0]) * MAX_CODE_LINES_N * MAX_CODE_LINE_LEN + 1);
			data_list_n = 0;
			memset(ext_lines, 0, sizeof(ext_lines[0][0]) * MAX_CODE_LINES_N * MAX_CODE_LINE_LEN + 1);
			ext_list_n = 0;
			memset(ent_lines, 0, sizeof(ent_lines[0][0]) * MAX_CODE_LINES_N * MAX_CODE_LINE_LEN + 1);
			ent_list_n = 0;
			directive_line_list = empty_ll;
			processed_line_list = empty_ll;
			symbol_table = empty_st;
			if ((buildLineList(src_f, filename, &symbol_table, &isa, &processed_line_list, &line_list_cnt, &directive_line_list, &ic)) >= 0) {
				if ((buildCodeList(filename, &symbol_table, &isa, &processed_line_list, &dc, &ext_list_n, &code_list_n)) >= 0) {
					if (buildDataList(filename, &symbol_table, &data_list_n, &directive_line_list, &dc, &ent_list_n) >= 0) {
						createOutputFiles(code_list_n, data_list_n, ent_list_n, ext_list_n, argv[file_number], CREATE);
					}
				} else { /* case proc2 failed */
					printf("failed to assemble file %s no output products will be created\n", filename);
				}
			} else { /* case proc1 failed */
				printf("failed to assemble file %s\n", filename);
			}
			fclose(src_f);
		}
	}
	mymalloc(0, FREE);
	return 0;
}

