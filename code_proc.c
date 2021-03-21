#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "assembler.h"
#include "string_structs.h"

extern char code_lines[][MAX_CODE_LINE_LEN + 1];
extern char data_lines[][MAX_CODE_LINE_LEN + 1];
extern char ent_lines[][MAX_ENT_EXT_LINE_LEN + 1];
extern char ext_lines[][MAX_ENT_EXT_LINE_LEN + 1];

/* function that searches some value search key in a general type pointer array  and returns
  the corresponding ("by index") element in the values ptr array */
void *map(char **keys, void **values, char *search_key) {
	for (; *keys && strcmp(search_key, *keys); ++keys, ++values)
		;
	return *values;
}

/* prints error to screen with collected arguments */
void printErr(char *filename, int line_bumber,const char *err_str) {
	printf("%s:%d: ERROR, %s\n", filename, line_bumber, err_str);
}

/* trims all white spaces in a given line string. return value is the new beginning of the line after trimming */
char *trimSpaces(char *line_ptr) {
	char *trim = line_ptr;
	for (; isspace(*trim); ++trim) { /* trim spaces before */
		++line_ptr;
	}
	for (trim = line_ptr + strlen(line_ptr) - 1; isspace(*trim); --trim) {/* trim spaces after */
		*trim = '\0';
	}
	return line_ptr;
}

/* function that gets a line with spaces trimmed in the edges and brings the line to a canonical form
   stores the result in *line   */
int normalizeLine(char *line) {
	char *c_src, last_c = 0;
	char temp_str[MAX_LINE_LEN + 1];
	int dest_idx = 0;
	char in_str_literal = 0, *str_lit_start = line + strlen(line) - 1, *str_lit_end = line;

	for (c_src = line; *c_src; ++c_src) {
		in_str_literal = (c_src >= str_lit_start && c_src <= str_lit_end)? 1:0;
		if (isspace(*c_src)) {
			if ((isspace(last_c) || last_c == ',') && !in_str_literal) {
				continue;
			}
			temp_str[dest_idx++] = ' ';
		}
		else {
			if (*c_src == '"' && !in_str_literal) {
				str_lit_start = c_src; /* set string literal starting pointer */
				for (str_lit_end = line + strlen(line) - 1; (*str_lit_end != '"') && (str_lit_end >= str_lit_start); --str_lit_end)
					; /* find the end of the string literal in line */
				if (str_lit_end <= str_lit_start) {
					printf("illegal string literal\n");
					return 0; /* illegal string literal */
				}
			}
			if ((*c_src == ',' && last_c == ' ') && !in_str_literal) {
				temp_str[dest_idx - 1] = ',';
				continue;
			}
			temp_str[dest_idx++] = *c_src;
		}
		last_c = temp_str[dest_idx - 1];
	}
	temp_str[dest_idx] = '\0';
	return (int) strncpy(line, temp_str, dest_idx + 1);
}

/* gets a line from file, and if it's a valid code line, prepares it. and puts it to target */
int prepLine(FILE *f, char *target) {
	char line_in[MAX_LINE_TRUE_LEN + 1] = {0}, *line_ptr = 0;
	int line_len = 0;
	int c;

	*target = '\0';

	line_ptr = fgets(line_in, MAX_LINE_TRUE_LEN + 1, f);
	line_len = strlen(line_in);
	if (line_len >= MAX_LINE_TRUE_LEN) { /* line exceeds max length */
		for (c = fgetc(f); c != EOF && c != '\n'; c = fgetc(f))
			;
		if (c == '\n')
			return PREP_LINE_LENGTH_ERR;
	}
	if (!line_ptr) { /* if EOF */
		if (!line_len)
			return EOF;
	}
	line_ptr = trimSpaces(line_ptr); 				/* trim pre and post spaces */
	if (*line_ptr == ';' || *line_ptr == '\n') { 	/* comment line or empty line */
		return 0;
	}
	if ((line_len = strlen(line_ptr))) { /* if trimmed line has length, copy it to target and return it's length */
		if(normalizeLine(line_ptr)) {
			strncpy(target, line_ptr, MAX_LINE_LEN + 1);
			return line_len;
		}
	}
	return 0; /* line is only whitespaces */
}

/* if ':' is in the line, returns the distance from ':' to start. or -1 else */
int getLabelDef(char *line) {
	char temp[LABEL_BUFF_LEN];
	int scanf_ret, chr_cnt = 0;

	sscanf(line, " %n", &chr_cnt); /* skip and count spaces */
	if (*(line + chr_cnt) == ':')
		return 0;
	if (!strstr(line, ":"))
		return ERR_RET; /* not a label */
	scanf_ret = sscanf(line + chr_cnt, "%[A-Za-z0-9]: %n", temp, &chr_cnt);
	if (scanf_ret) {
		if (!chr_cnt) {
			return LABEL_ERROR; /* LABEL_ERROR */
		}
		return strlen(temp);
	}
	else { /* not a label */
		return ERR_RET;
	}
}

/* tries to extract an instruction name from string (*line), searches it in the isa
 * 	if found it returns a pointer to the number of instruction operands */
int *getInstructionDef(char *line, isa_struct *isa, char *filename, int line_number) {
	char buffer[MAX_LINE_LEN];
	int scan_ret;

	scan_ret = sscanf(line, " %s ", buffer);
	if (scan_ret) {
		return (int *) map((char **) isa->name, (void **) isa->n_operands, buffer);
	}
	else {
		printErr(filename, line_number, (const char *) "invalid instruction name");
		return NULL;
	}
}

/* searches for '.' in a line */
int isLineDirective(char *line) {
	int scan_ret;
	char temp[MAX_LINE_LEN];

	scan_ret = sscanf(line, " %s ", temp);
	if (scan_ret)
		if (strstr(temp, "."))
			return OK_RET;
	return 0;
}

/* checks if the string dir_type is in the line */
int isLineDirectiveType(char *line, const char *dir_type) {
	int scan_ret = 0, chr_read = 0;
	char temp[DIR_NAME_MAX_LEN];

	scan_ret = sscanf(line, " .%s %n", temp, &chr_read);
	if (scan_ret)
		if (!strcmp(dir_type, temp))
			return chr_read;
	return 0;
}

/* searches label argument in both the instruction list structure (isa) and register names */
int isSavedWord(isa_struct *isa, char *label) {
	int ret = 0;
	char *reg_names[] = REG_NAMES;
	ret |= (map(isa->name, (void *) isa->name, label) == NULL)? 0 : OK_RET;
	return ret |= (map(reg_names, (void *) reg_names, label) == NULL)? 0 : OK_RET;
}

/* checks that the addressing method add_methd is found in the legal_add_methd array, return 1 if true else 0 */
int checkAddMthdLegal(int add_methd,  int legal_add_methd[MAX_N_OPS]) {
	int i;
	for (i = 0; (i < MAX_LEG_ADR) && (legal_add_methd[i] != -1); ++i) {
		if (legal_add_methd[i] == add_methd) {
			return OK_RET;
		}
	}
	return 0;
}

/*	the line by line process of preparing each line and building the symbol table
  		- get line label - check it
  			- if its illegal or already in the symbol table return error
  			- else valid_label = 1
  		- if line an instruction
 			- if valid_label insrt to sym table
  			- if valid instruction name , insert the entire line to processed code line list
  				with instruction name, original src code line, to the a
  				else - return error -1
  		- if line is a directive
  			- if extern, zero line label and valid flag, if directive label valid - insert to sym table as extern
  			- if data or string, insert line label to sym table with value 0, attribute DATA
  				and copy full line to directive_line_list
  			- if entry ignore line label and copy the line without the label to directive line list */
int processLineBuildLineList(char *line_in, lineList *processed_code_line_list, int processed_code_line_cnt, lineList *directive_line_list, char *filename, int code_line_number, int *ic, symTbl *st, isa_struct *isa) {
	int label_len, valid_label = 0, *inst_len_words = NULL, ext_label_len = 0, chrs_read = 0;
	char *line = line_in;
	char *proc_ptr = line, *lbl_ptr;
	char label[LABEL_BUFF_LEN], dummy[MAX_LINE_LEN] = {0};
	int *sym_add_val;

	if ((label_len = getLabelDef(proc_ptr)) >= 0) { /* get and check label definition */
		if (!label_len) {
			printErr(filename, code_line_number, (const char *) "empty label is not allowed");
			return ERR_RET;
		}
		if (label_len > LABEL_MAX_LEN) {
			printErr(filename, code_line_number, (const char *) "label exceeds maximum allowed length");
			return ERR_RET;
		}
		strncpy(label, proc_ptr, label_len); /* store label for processing */
		proc_ptr += label_len + 1;
		label[label_len] = '\0';

		if (isalpha((label[0]))) { /* check label start with letter */
			for (lbl_ptr = label; ((lbl_ptr - label) < label_len) && (isalnum(*lbl_ptr)) && (*lbl_ptr != ':'); ++lbl_ptr)
				;
			if (lbl_ptr - label < label_len) {
				printErr(filename, code_line_number, "illegal char in label definition");
				return ERR_RET;
			}
		}
		else {
			printErr(filename, code_line_number, "label definition should start with a letter\n");
			return ERR_RET;
		}

		if (map(st->sym_names, (void **) st->sym_names, label) != NULL) {
			printErr(filename, code_line_number,  (const char *) "label already exists in the symbol table");
			return ERR_RET;
		}
		if (isSavedWord(isa, label)) {  /* check if label already defined */
			printErr(filename, code_line_number, (const char *)  "label is a saved word");
			return ERR_RET;
		}
		valid_label = 1;
	}
	if (label_len == LABEL_ERROR) {
		printErr(filename, code_line_number, "illegal label definition");
		valid_label = 0;
		return ERR_RET;
	}

	if (!isLineDirective(proc_ptr)) { /* line is an instruction */
		inst_len_words = getInstructionDef(proc_ptr, isa, filename, code_line_number); /* check instruction and return ptr to number of machine
																		words, if invalid add error */
		if (inst_len_words && *inst_len_words >= 0) {
			if (valid_label)
				insertSymbol(st, label, *ic, CODE);
			*ic += *inst_len_words + 1; /* 1 extra word for first word of instruction */
			strncpy(processed_code_line_list->line_list[processed_code_line_list->len], line_in, strlen(line_in));
			processed_code_line_list->instruction[processed_code_line_list->len] = (char *) map(isa->name, (void **) isa->name, label);
			processed_code_line_list->src_line_n[processed_code_line_list->len] = code_line_number;
			processed_code_line_list->len++;
			return OK_RET;
		}
		else {
			printErr(filename, code_line_number, (const char *) "invalid instruction name");
			return ERR_RET;
		}
	}
	else { /* line is a directive */
		if ((isLineDirectiveType(proc_ptr, "extern"))) {
			if (valid_label) { /* if a label was defined earlier in line ignore it - clear buffer */
				valid_label = 0;
				label[0] = '\0';
			}
			if (sscanf(proc_ptr, " .extern %s %s %n", label, dummy, &chrs_read)) { /* external label extraction */
				if (strlen(dummy)) {
					printErr(filename, code_line_number, (const char *)  "extraneous text at the end of command");
					return ERR_RET;
				}
				proc_ptr += chrs_read;
				ext_label_len = strlen(label);
				if (ext_label_len && (ext_label_len <= LABEL_MAX_LEN)) { /* valid label length */
					if (proc_ptr - line <= MAX_LINE_LEN) { /* legal line length */
						if (!isSavedWord(isa, label)) { /* not a saved word */
							if ((sym_add_val = (int *) map(st->sym_names, (void **) st->sym_values, label)) == NULL) {
								insertSymbol(st, label, 0, EXTERNAL);
								return OK_RET;
							}
							else { /* label exists in symbol table - allow double external definitions */
								if (*sym_add_val & ENT_DIR_IDX) {
									printErr(filename, code_line_number, "label can't be defined both entry and external");
									return ERR_RET;
								}
							}
						}
						else { /* label is a saved word */
							printErr(filename, code_line_number, "label is a saved word");
							return ERR_RET;
						}
					}
					else { /* illegal line length */
						printErr(filename, code_line_number, "line exceeds maximum length");
						return ERR_RET;
					}
				} /*  label is empty or over-sized */
				if (!ext_label_len) {
					printErr(filename, code_line_number, "empty label is not allowed");
					return ERR_RET;
				}
				else { /* oversized label */
					printErr(filename, code_line_number, "label exceeds maximum allowed length");
				}
				return ERR_RET;
			}
		}
		if (isLineDirectiveType(proc_ptr, "data") || isLineDirectiveType(proc_ptr, "string")) {
			if (valid_label) {
				insertSymbol(st, label, 0, DATA);
			}
			/*strncpy(directive_line_list->line_list[directive_line_list->len], proc_ptr, MAX_LINE_LEN + 1);*/
			sprintf(directive_line_list->line_list[directive_line_list->len], "%s", line_in);
			directive_line_list->instruction[directive_line_list->len] = NULL;
			directive_line_list->src_line_n[directive_line_list->len] = code_line_number;
			directive_line_list->len++;
			return OK_RET;
		}
		if (isLineDirectiveType(proc_ptr, "entry")) {
			sprintf(directive_line_list->line_list[directive_line_list->len], "%s", proc_ptr);
			directive_line_list->instruction[directive_line_list->len] = NULL;
			directive_line_list->src_line_n[directive_line_list->len] = code_line_number;
			directive_line_list->len++;
		}
	}
	return OK_RET; /* line is a different directive - skip */
}

/*	this is applied to each line of processed code:
 * 		Method:
 * 			- gets and stores label
 * 			- extract instruction name
 * 			- get number of required operands for instruction type
 * 			- extract operands from line and write code
 * 			- for unknown symbols encountered, temporarily copy the line to the code segment to
 * 				be parsed after the symbol table is complete */
int processLineBuildCode(lineList *processed_line_list, int line_list_idx, char *filename, symTbl *st, isa_struct *isa, int *c_line_idx, int *ext_lines_idx) {
	int label_len, *inst_n_ops, j;
	char line_label[LABEL_MAX_LEN + 1];
	char *line_ptr = processed_line_list->line_list[line_list_idx];
	char cmd[INSTRCT_MAX_LEN + 1];
	char ops[MAX_N_OPS][LABEL_MAX_LEN + 1];
	int operand_values[MAX_N_OPS] = {-1, -1};
	int chrs_read = 0;
	int add_methd_arr[MAX_N_OPS] = {-1, -1};
	int *add_methd_arr_ptr;
	char *reg_names[] = REG_NAMES;
	int *opcode, *funct;
	unsigned int inst_add_mthd = 0;
	int *operand_attr_ptr;
	int operand_attr;
	int operand_value, *operand_value_ptr;
	char operand_are = 0;

	if ((label_len = getLabelDef(line_ptr)) != -1) { /* check and store label */
		sscanf(line_ptr, "%[^:]:%n", line_label, &chrs_read);
		line_ptr += chrs_read;
	}
	if (isLineDirective(line_ptr)) { /* should be redundant */
		return OK_RET;
	}
	sscanf(line_ptr, "%s %n", cmd, &chrs_read); /* get instruction name */
	line_ptr += chrs_read;
	inst_n_ops = (int *) map(isa->name, (void **) isa->n_operands, cmd);
	if (!inst_n_ops) {
		return ERR_RET;
	}
	if (!*inst_n_ops) { /* write code for 0 operands instructions */
		opcode = (int *) map(isa->name, (void **) isa->opcode, cmd);
		funct = (int *) map(isa->name, (void **) isa->func, cmd);
		sprintf(code_lines[*c_line_idx], "%04d %X%X%X A", (*c_line_idx) + IC_INIT, \
				(unsigned int) *opcode, (unsigned int) *funct, (unsigned int) inst_add_mthd);
		++*c_line_idx;
		return OK_RET;
	}

	for (j = 0; (j < *inst_n_ops); ++j) { /* get_operands() */
		sscanf(line_ptr, " %[^,]%n", ops[j], &chrs_read);
		line_ptr += chrs_read + 1;
		switch (*ops[j]) {  /* get addressing methods */
			case IMMEDIATE_PREFIX:
				add_methd_arr[j] = IMMEDIATE;
				if (!isspace(ops[j][1]) && ((ops[j][1] == '+') || (ops[j][1] == '-') || isdigit(ops[j][1])))  {
					if(!sscanf(ops[j], "#%d%n", &operand_values[j], &chrs_read) || !chrs_read) {
						printErr(filename, processed_line_list->src_line_n[line_list_idx], "illegal immediate");
						return ERR_RET;
					}
				}
				else {
					printErr(filename, processed_line_list->src_line_n[line_list_idx], "in immediate definition only sign and number can follow #");
					return ERR_RET;
				}
				continue;
			case RELATIVE_PREFIX:
				add_methd_arr[j] = RELATIVE;
				continue;
			default:
				if (map(reg_names, (void **) reg_names, ops[j]) != NULL) {
					add_methd_arr[j] = REG_DIRECT;
					sscanf(ops[j], "r%d", (int *) operand_values + j);
					operand_values[j] = ((0x1) << operand_values[j]); /* convert to 1-hot */
					continue;
				}
				if (!isalpha(*ops[j])) {
					printErr(filename, processed_line_list->src_line_n[line_list_idx], "undefined addressing method");
					return ERR_RET;
				}
				if (map(st->sym_names, (void **) st->sym_names, ops[j]) != NULL) {
					add_methd_arr[j] = DIRECT;
					continue;
				}
				else {
					printErr(filename, processed_line_list->src_line_n[line_list_idx], "undefined label name");
					return ERR_RET;
				}

				continue;
		}
		add_methd_arr_ptr = (int *) map(isa->name, (void **) isa->llegal_dst_add, cmd);
		if (!checkAddMthdLegal(add_methd_arr[j], add_methd_arr_ptr)) {
			printErr(filename, processed_line_list->src_line_n[line_list_idx], "invalid addressing method for instruction type");
			return ERR_RET;
		}
	}
	/* instruction 1st code word */
	opcode = (int *) map(isa->name, (void **) isa->opcode, cmd);
	funct = (int *) map(isa->name, (void **) isa->func, cmd);
	for (j = 0; j < *inst_n_ops; ++j) {
		inst_add_mthd = (inst_add_mthd << 2*j) + add_methd_arr[j];
	}
	sprintf(code_lines[*c_line_idx], "%04d %X%X%X A", (*c_line_idx) + IC_INIT, \
			(unsigned int) *opcode, (unsigned int) *funct, (unsigned int) inst_add_mthd);
	++*c_line_idx;
	/* 2nd and 3rd (opt) - get 12 bit value */
	for (j = 0; j < *inst_n_ops; ++j) {
		switch (add_methd_arr[j]) {
			case IMMEDIATE:
				sprintf(code_lines[*c_line_idx], "%04d %03X A", *c_line_idx + IC_INIT, operand_values[j] & 0xFFF);
				++*c_line_idx;
				continue;
			case REG_DIRECT:
				sprintf(code_lines[*c_line_idx], "%04d %03X A", *c_line_idx + IC_INIT, operand_values[j]);
				++*c_line_idx;
				continue;
			case RELATIVE:
				if ((operand_attr_ptr = (int *) map(st->sym_names, (void **) st->sym_attributes, ops[j] + 1)) != NULL) {
					operand_attr = (int) operand_attr_ptr;
					if (operand_attr & EXTERNAL) {
						printErr(filename, processed_line_list->src_line_n[line_list_idx], "relative call to external label is not allowed");
						return ERR_RET;
					}
					if (operand_attr & CODE) {
						operand_value_ptr = (int *) map(st->sym_names, (void **) st->sym_values, ops[j] + 1);
						operand_value = (int) operand_value_ptr;
						operand_are = 'A';
						sprintf(code_lines[*c_line_idx], "%04d %03X %c", *c_line_idx + IC_INIT, (operand_value - (*c_line_idx + IC_INIT)) & 0xFFF, operand_are);
						++*c_line_idx;
						continue;
					}
					if (operand_attr & DATA) {
						sprintf(code_lines[*c_line_idx], "%s", ops[j]);
						++*c_line_idx;
						continue;
					}
				}
				else {
					printErr(filename, processed_line_list->src_line_n[line_list_idx], "invalid label name");
					return ERR_RET;
				}
				continue;
			case DIRECT:
				if ((operand_attr_ptr = (int *) map(st->sym_names, (void **) st->sym_attributes, ops[j])) != NULL) {
					operand_attr = (int) operand_attr_ptr;
					if (operand_attr & (CODE | EXTERNAL)) {
						operand_value_ptr = (int *) map(st->sym_names, (void **) st->sym_values, ops[j]);
						operand_value = (int) operand_value_ptr;
						operand_are = (operand_attr & CODE) ? 'A' : 'E';
						if (operand_are == 'E') {
							sprintf(ext_lines[*ext_lines_idx], "%s %04d", ops[j], *c_line_idx + IC_INIT);
							++*ext_lines_idx;
						}
						sprintf(code_lines[*c_line_idx], "%04d %03X %c", *c_line_idx + IC_INIT, operand_value, operand_are);
						++*c_line_idx;
						continue;
					}
					if (operand_attr & DATA) {
						sprintf(code_lines[*c_line_idx], "%s", ops[j]);
						++*c_line_idx;
						continue;
					}
				}
				else {
					printErr(filename, processed_line_list->src_line_n[line_list_idx], "invalid operand type");
				}
				break;
			default:
				break;
		}
	}
	/*  verify end of line */
	for (;(line_ptr - processed_line_list->line_list[line_list_idx]) <= (MAX_LINE_LEN  + 1) && *line_ptr && isspace(*line_ptr); ++line_ptr)
		;
	if (*line_ptr != '\0') {
		printErr(filename, processed_line_list->src_line_n[line_list_idx], "extraneous text at the end of line");
		return ERR_RET;
	}
	return OK_RET;
}

/* for each line of directive code if its .data or .string, extract data and save it with its address in the
 * 	data_lines global array. along with updating the symbol table */
int buildDataList(char *filename, symTbl *st, int *data_list_idx, lineList *dir_line_list_st, int *dc, int *ent_list_n) {
	char entry_list[MAX_CODE_LINES_N][MAX_ENT_EXT_LINE_LEN];
	char line_label[LABEL_MAX_LEN + 1];
	char string[MAX_LINE_LEN], dummy[MAX_LINE_LEN];
	char directive[DIR_NAME_MAX_LEN];
	int i, label_len, chrs_read, label_dc, string_len, char_p, final_ic;
	char *line_ptr = dir_line_list_st->line_list[0];
	int entry_list_size = 0;
	int code_lines_idx;
	int data_num;
	int scanf_ret, comma_detected = 0, ent_idx, ent_label_add;
	char *ent_ptr, temp_ent[LABEL_MAX_LEN + 1];

	*data_list_idx = 0;
	*ent_list_n = 0;
	final_ic = *dc;
	for(i = 0; i < dir_line_list_st->len; ++i) {
		label_dc = 0;
		string_len = 0;
		line_ptr = dir_line_list_st->line_list[i];
		if ((label_len = getLabelDef(line_ptr)) != -1) { /* check and store label */
			sscanf(line_ptr, "%[^:]:%n", line_label, &chrs_read);
			line_ptr += chrs_read;
			label_dc = *dc; /* both is_valid_label flag and initial dc value */
		}
		if (isLineDirectiveType(line_ptr, "entry")) { /* entry dirs will be handled after all symT is built */
			strncpy(entry_list[entry_list_size++], line_ptr, strlen(line_ptr));
			continue;
		}
		if (isLineDirectiveType(line_ptr, "string")) { /* store string to data */
			dummy[0] = '\0';
			sscanf(line_ptr, " .%s %s %s ", directive, string, dummy);
			if (strlen(dummy)) {
				printErr(filename, dir_line_list_st->src_line_n[i], "extraneous text at the end of line");
			}
			string_len = strlen(string) - 2; /* cutting edges "" */
			if (string_len) {
				for (char_p = 0; char_p < string_len; ++char_p) { /* conversion of string to 12 bit words */
					sprintf(data_lines[*dc - final_ic], "%04d %03X A", *dc, (unsigned int) string[char_p + 1]);
					++*dc;
				}
				sprintf(data_lines[*dc - final_ic], "%04d 000 A", *dc); /* string '\0' */
				++*dc;
				if (label_dc) { /* replace temp label with code line including its address */
					for (code_lines_idx = 0; code_lines_idx < final_ic - IC_INIT; ++code_lines_idx) {
						if (!strncmp(code_lines[code_lines_idx], line_label, LABEL_MAX_LEN)) {
							sprintf(code_lines[code_lines_idx], "%04d %03X R", code_lines_idx + IC_INIT, label_dc);
							insertSymbol(st, line_label, label_dc, DATA);
						}
					}
				}
			}
		}

		if (isLineDirectiveType(line_ptr, "data")) {
			comma_detected = 1;
			sscanf(line_ptr, " .%s %n", directive, &string_len);
			line_ptr += string_len;
			if (string_len) {
				while (comma_detected) {
					/* convert line separated integers to 12 bit words */
					scanf_ret = sscanf(line_ptr, "%d%n", &data_num, &chrs_read);
					line_ptr += chrs_read;
					if (!scanf_ret) {
						printErr(filename, dir_line_list_st->src_line_n[i], "empty data definition");
						continue;
					}
					sprintf(data_lines[*dc - final_ic], "%04d %03X A", *dc, (data_num & 0xFFF));
					++*dc;
					comma_detected = (*line_ptr == ',');
					line_ptr += comma_detected;
				}
				if (*line_ptr != '\0') {
					printErr(filename, dir_line_list_st->src_line_n[i], "Extraneous text in data definition");
					continue;
				}
				if (label_dc) { /* replace temp label with code line including its address */
					for (code_lines_idx = 0; code_lines_idx < final_ic - IC_INIT; ++code_lines_idx) {
						if (!strncmp(code_lines[code_lines_idx], line_label, LABEL_MAX_LEN)) {
							sprintf(code_lines[code_lines_idx], "%04d %03X R", code_lines_idx + IC_INIT, label_dc);
							insertSymbol(st, line_label, label_dc, DATA);
						}
					}
				}
			}
		}
	*data_list_idx = *dc - final_ic;
	}
	for (ent_idx = 0; ent_idx < entry_list_size; ++ent_idx) { /* handle entry list */
		ent_ptr = entry_list[ent_idx];
		sscanf(ent_ptr, " .%s %n", directive, &chrs_read);
		ent_ptr += chrs_read;
		sscanf(ent_ptr, "%s %n", temp_ent, &chrs_read);
		if (map(st->sym_names, (void **) st->sym_names, temp_ent) != NULL) {
			ent_label_add = (int) map(st->sym_names, (void **) st->sym_values, temp_ent);
			sprintf(ent_lines[*ent_list_n], "%s %04d", temp_ent, ent_label_add);
			++*ent_list_n;
		}
		else {
			printErr(filename, dir_line_list_st->src_line_n[i], "defining entry of unknown label");
			return ERR_RET;
		}
	}
	return OK_RET;
}

/* build line list from file:
 *  - for each line in the src file:
 *  	- check line length
 *  	- normalize line and save to line_in
 *  	- run processLineBuildLineList
 *  	- if line has error set flag and continue
 *  	- if EOF return -flag */
int buildLineList(FILE *f, char *filename, symTbl *symbol_table, isa_struct *isa, lineList *processed_code_line_list_st, int *line_list_idx, lineList *directive_line_list, int *target_ic) {
	int line_list_cnt = 0, line_status = 0, src_file_line_number;
	int line_strlen = 0;
	char eof_reached = 0;
	unsigned int fatal_error_flag = 0;
	int ic = IC_INIT;
	char line_in[MAX_LINE_TRUE_LEN];

	for (src_file_line_number = 1; !eof_reached; ++src_file_line_number) {
		line_status = prepLine(f, line_in); /* if a valid line end with EOF then line_status = -1
		 	 	 	 	 	 	 	 	 	 	 	 	 and line_strlen = actual line len; */
		if (line_status == PREP_LINE_LENGTH_ERR) { /* length error */
			printErr(filename, src_file_line_number, "line exceeds maximum allowed length");
			fatal_error_flag = 1;
			continue;
		}
		line_strlen = strlen(line_in);
		if (line_status == EOF) { 	/* this separation was needed to tell the difference between the case of legal empty line,
		 	 	 	 	 	 	 	 	 a valid line ending with EOF,  or illegal line */
			if (line_strlen > 0) {
				fatal_error_flag |= processLineBuildLineList(line_in, processed_code_line_list_st, line_list_cnt, directive_line_list, filename, src_file_line_number, &ic, symbol_table, isa) == -1? 1:0;
				*target_ic = ic;
				*line_list_idx = line_list_cnt;
				return -fatal_error_flag;
			}
			if (!line_strlen) {
				*target_ic = ic;
				*line_list_idx = line_list_cnt;
				return -fatal_error_flag;
			}
		}
		if (line_status > 0) {
			fatal_error_flag |= processLineBuildLineList(line_in, processed_code_line_list_st, line_list_cnt, directive_line_list, filename, src_file_line_number, &ic, symbol_table, isa) == -1? 1:0;
			++line_list_cnt;
		}
		if (!line_status) {
			continue;
		}
	}
	*target_ic = ERR_RET;
	*line_list_idx = ERR_RET;
	return ERR_RET;
}

/* building the code list out of the processed code line list, catch errors, advance counters
 * 	- for each line in processed_line_list runs processLineBuildCode
 * 		- accumulates error flag, if a line has error the function will return negative value */
int buildCodeList(char *filename, symTbl *symbol_table, isa_struct *isa, lineList *processed_line_list, int *target_dc, int *ext_lines_idx, int *code_out_line_n) {
	int i;
	int fatal_error_flag = 0;

	*ext_lines_idx = 0;
	for (i = 0; i < processed_line_list->len; ++i) {
		fatal_error_flag |= processLineBuildCode(processed_line_list, i, filename, symbol_table, isa, code_out_line_n, ext_lines_idx) == -1? 1:0;
	}
	if (!fatal_error_flag) {
		*target_dc = *code_out_line_n + IC_INIT;
		return OK_RET;
	}
	return ERR_RET;
}
