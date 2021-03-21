
#ifndef CODE_PROC_H
#define CODE_PROC_H
#include "string_structs.h"


char *trimSpaces(char *line_ptr); /* trims sapces from both ends of the line */
int normalizeLine(char *line);
int prepLine(FILE *f, char *target);
int getLabelDef(char *line);
int *getInstructionDef(char *line, isa_struct *isa, char *filename, int line_number);
int isLineDirective(char *line);
int isLineDirectiveType(char *line, const char *dir_type);
int isSavedWord(isa_struct *isa, char *label);
int checkAddMthdLegal(int add_methd,  int legal_add_methd[MAX_N_OPS]);
int processLineBuildLineList(char *line_in, lineList *processed_code_line_list, int processed_code_line_cnt, \
										lineList *directive_line_list, char *filename, \
														int code_line_number, int *ic, symTbl *st, isa_struct *isa);

int processLineBuildCode(lineList *processed_line_list, int line_list_idx, char *filename, symTbl *st, \
														isa_struct *isa, int *c_line_idx, int *ext_lines_idx);

int buildDataList(char *filename, symTbl *st, int *data_list_idx, lineList *dir_line_list_st, int *dc, int *ent_list_n);
int buildLineList(FILE *f, char *filename, symTbl *symbol_table, isa_struct *isa, lineList *processed_code_line_list_st,\
														int *line_list_idx, lineList *directive_line_list, int *target_ic);

int buildCodeList(char *filename, symTbl *symbol_table, isa_struct *isa, lineList *processed_line_list, \
																	int *target_dc, int *ext_lines_idx, int *code_out_line_n);
void printErr(char *filename, int line_bumber,const char *err_str);

#endif
