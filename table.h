/**
 * @Author:	Carmine
 * @Date:	29/07/2023
 * @Name:	table.h
 */

#if !defined(TABLE_H)
#define TABLE_H

#include "fsm.h"

typedef struct row {
	char value[MAXSEQ + 1];
	int next0, exit0;	// next state and exit on '0'
	int next1, exit1;	// next state and exit on '1'
} Row;

typedef struct table {
	int nstates;
	Row rows[];
} Table;


Table *get_table(Sequence *sequence, StatesList *list);
void print_table(Table *table);
void optimize_table(Table *table);
bool equal_rows(Table *table, int i, int j);

/**
 * Merges the row values into row i and deletes row j.
 */
void merge_rows(Table *table, int i, int j);

#endif /* TABLE_H */
