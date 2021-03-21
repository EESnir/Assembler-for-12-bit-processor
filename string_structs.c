#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "assembler.h"
#include "string_structs.h"

void *map(char **keys, void **values, char *search_key);

/*  malloc wrapper that stores that allocated pointers in a static pointer array.
  	the aloc_free flag tells the function if to allocate new space or free all allocated pointers */
void *mymalloc(size_t n_bytes, int aloc__free) {
	static char *dyn_arr[SYMTBL_MAX_LEN];
	static int dyn_arr_size;
	int i;

	for (i = 0; (i < dyn_arr_size) && aloc__free == FREE; ++i) {
		free(dyn_arr[i]);
	}

	if ((dyn_arr[dyn_arr_size] = malloc(n_bytes)))
		return dyn_arr[dyn_arr_size++];
	return NULL;
}

/* insert a symbol to a table given a pointer to it and initial values */
void insertSymbol(symTbl *st, char *symbol_name, int value, int attribute_enum) {
	char *find;
	int wr_idx;
	int read_att;

	find = (char *) map(st->sym_names, (void **) st->sym_names, symbol_name);
	if (find) {
		wr_idx = ((find - st->sym_names[0]) / (st->sym_names[1] - st->sym_names[0]));
		read_att = (int) map(st->sym_names, (void **) st->sym_attributes, symbol_name);
		st->sym_attributes[wr_idx] = attribute_enum | read_att;
		st->sym_values[wr_idx] = value;
		return;
	}

	if (st) {
		wr_idx = st->len;
		if (wr_idx < SYMTBL_MAX_LEN) {
			st->sym_names[wr_idx] = strcpy(mymalloc(strlen(symbol_name) + 1, ALOCATE), symbol_name);
			st->sym_values[wr_idx] = value;
			st->sym_attributes[wr_idx] = attribute_enum;
			(st->len)++;
			return;
		}
	}
}
