/**
 * @Author:	Carmine
 * @Date:	29/07/2023
 * @Name:	table.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "table.h"
#include "fsm.h"

Table *get_table(Sequence *sequence, StatesList *list) {
	Table *table = malloc(sizeof(Table) + sizeof(Row[list->nstates]));
	if (table == NULL)
		error_exit("Could not create table: %s\n", strerror(errno));

	table->nstates = list->nstates;
	Row *row;
	State *state;
	char effective_value[MAXSEQ + 1];	// effective value is value + input ('0'/'1')
	int length = 0;
	// init rows
	for (int i = 0; i < table->nstates; ++i) {
		row = &table->rows[i];
		state = list->states[i];

		strcpy(row->value, state->value);

		strncpy(effective_value, row->value, sizeof(effective_value));
		length = strlen(effective_value);
		if (*effective_value == '-')
            length = 0;

		row->next0 = list_index(list, state->left);

		effective_value[length] = '0';
		row->exit0 = valuescmp(sequence, effective_value) == 0;

		row->next1 = list_index(list, state->right);

		effective_value[length] = '1';
		row->exit1 = valuescmp(sequence, effective_value) == 0;
	}

	return table;
}

void print_table(Table *table) {
	// use graphical characters
	#define DEC_MODE "\033(0"
	#define ASCII_MODE "\033(B"
	#define draw(c) DEC_MODE c ASCII_MODE

	static const char *table_fmt =
		DEC_MODE "tqqqqqqqqqqqqnqqqqqqqqqqnqqqqqqqqqqu" ASCII_MODE "\n"
		draw("x") "Q%-4d %-4s  " draw("x") "  %4d/%d  " draw("x") "  %4d/%d  " draw("x") "\n";

	printf("\nTable:\n");
	printf(DEC_MODE "lqqqqqqqqqqqqwqqqqqqqqqqwqqqqqqqqqqk\n");
	printf(draw("x") "State       " draw("x") "     0    " draw("x") "     1    " draw("x") "\n");
	for (int i = 0; i < table->nstates; ++i)
		printf(table_fmt, i, table->rows[i].value,
			   table->rows[i].next0, table->rows[i].exit0,
			   table->rows[i].next1, table->rows[i].exit1);

	printf(DEC_MODE "mqqqqqqqqqqqqvqqqqqqqqqqvqqqqqqqqqqj\n" ASCII_MODE);
}

void optimize_table(Table *table) {
	/**
	 * 1. Delete equal rows
	 * 2. Compare each state with the followings and search for compatible states.
	 *	Two states are compatible if they have the same exits and the next states are compatible.
	 */
	bool equivalentRows = true; // NOTE: This is necessary in case sequence contains 2 or more don't cares.
	while (equivalentRows) {
		for (int i = 0; i < table->nstates - 1; ++i) {
			equivalentRows = false;
			for (int j = i + 1; j < table->nstates; ++j) {

				if (equal_rows(table, i, j)) {
					equivalentRows = true;
					printf("Q%d == Q%d\n", i, j);

					merge_rows(table, i, j);
					print_table(table);

					j--; // Check again row j
				}
			}
		}
	}
}

bool equal_rows(Table *table, int i, int j) {
	return table->rows[i].next0 == table->rows[j].next0 &&
		   table->rows[i].exit0 == table->rows[j].exit0 &&
		   table->rows[i].next1 == table->rows[j].next1 &&
		   table->rows[i].exit1 == table->rows[j].exit1;
}

void merge_rows(Table *table, int i, int j) {
	// merge sequences values: e.g "010" and "000" -> "0-0"
	char *valuei = table->rows[i].value;
	char *valuej = table->rows[j].value;
	for (int k = 0; *valuej; ++k, ++valuej)
		if (valuei[k] != *valuej)
			valuei[k] = '-';

	// delete row j
	for (int pos = j; pos < table->nstates - 1; ++pos)
		table->rows[pos] = table->rows[pos + 1];
	--table->nstates;

	// replace j with i in next's
	for (int k = 0; k < table->nstates; ++k) {
		if (table->rows[k].next0 == j)
			table->rows[k].next0 = i;
		if (table->rows[k].next1 == j)
			table->rows[k].next1 = i;
	}

	// then shift down by 1 those greater than j to update next states
	for (int k = 0; k < table->nstates; ++k) {
		if (table->rows[k].next0 > j)
			--table->rows[k].next0;
		if (table->rows[k].next1 > j)
			--table->rows[k].next1;
	}
}
