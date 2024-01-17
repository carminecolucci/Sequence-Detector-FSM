/**
 * @Author:	Carmine
 * @Date:	06/08/2022
 * @Name:	fsm.c
 * @Desc:	Finite state machine to recognize a given sequence
 */

#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fsm.h"
#include "table.h"

#if defined(_MSC_VER) || defined(__MINGW64__)
#include <io.h>
#define getfileno(fp) _fileno((fp))
#define isterminal(fd) _isatty((fd))
#elif defined(__linux__)
#include <unistd.h>
#define getfileno(fp) fileno((fp))
#define isterminal(fd) isatty((fd))
#endif

#define PROMPT_EXIT 0

static const char *welcome =
	"\nFinite State Machine\n"
	"Author:\tCarmine\n"
	"Date:\t06/08/2022\n"
	"Desc:\tFinite state machine to recognize a given binary sequence\n";

int main() {
	puts(welcome);

	Sequence sequence;
	get_sequence(&sequence);

	sequence.mode = get_mode();
	printf("Calculating states for %s, overlap mode: %d...\n", sequence.value, sequence.mode);

	StatesList *list = get_states_list(&sequence);

	printf("Num states: %d\n", list->nstates);

	Table *table = get_table(&sequence, list);
	print_table(table);

	if (table->nstates > 1 && (sequence.mode == NOT_OVERLAPPING || (strchr(sequence.value, '-') != NULL))) {
		printf("\nOptimizing states...\n");
		optimize_table(table);

		printf("\nOptimized table:\n");
		print_table(table);
	}

	free(table);
	delete_states_list(&list);

#if PROMPT_EXIT
	printf("Press enter to exit...");
	getchar();
#endif
	return 0;
}

void get_sequence(Sequence *sequence) {
	printf("Enter a sequence of [0, 1, -] [max %d digits]: ", MAXSEQ);
	int c;
	sequence->length = 0;
	while (sequence->length < MAXSEQ && (c = getchar()) != EOF) {
		if (c == '\n')
			break;
		if (c == '0' || c == '1' || c == '-')
			sequence->value[sequence->length++] = c;
		else
			error_exit("Invalid number: %c\n", c);
	}
}

overlapping_mode get_mode() {
	int mode, c;
	do {
		printf("Enter overlap mode [0 None, 1 Partially, 2 Totally]: ");
		scanf(" %d", &mode);
		while ((c = getchar()) != EOF && c != '\n')	 // clear input buffer
			;
		if (mode < NOT_OVERLAPPING || mode > TOTALLY_OVERLAPPING)
			printf("Invalid mode: %d\n", mode);
	} while (mode < NOT_OVERLAPPING || mode > TOTALLY_OVERLAPPING);
	return mode;
}

State *create_states(Sequence *sequence, State *reset, char *value) {
	State *state = NULL;
	if (*value == '\0')
		state = reset;
	else {
		state = malloc(sizeof(*state));
		if (state == NULL)
			error_exit("Could not create state %s\n", value);
		strncpy(state->value, value, sizeof(state->value));
		state->left = state->right = NULL;
		++sequence->nstates;

		add_state_to_graph(reset, state);
	}

	int length = strlen(value);
	char next[MAXSEQ + 1];
	strncpy(next, value, sizeof(next));
	int next_length = length + 1;

	if (sequence->mode == NOT_OVERLAPPING) {
		if (next_length == sequence->length)
			state->left = state->right = reset;
		else {
			next[length] = '0';	 // value + '0'
			state->left = create_states(sequence, reset, next);
			next[length] = '1';	 // value + '1'
			state->right = create_states(sequence, reset, next);
		}
	} else if (sequence->mode == PARTIALLY_OVERLAPPING) {
		/**
		 * reset only if the next value equals sequence.
		 * find the best match if next state is not the beginning of sequence
		 * otherwise generate the next state.
		 */
		next[length] = '0';
		if (valuescmp(sequence, next) == 0)
			state->left = reset;
		else if (valuesncmp(sequence, next, next_length) == 0)
			state->left = create_states(sequence, reset, next);
		else
			state->left = find_match(sequence, reset, next);

		next[length] = '1';
		if (valuescmp(sequence, next) == 0)
			state->right = reset;
		else if (valuesncmp(sequence, next, next_length) == 0)
			state->right = create_states(sequence, reset, next);
		else
			state->right = find_match(sequence, reset, next);
	} else {  // TOTALLY_OVERLAPPING
		/**
		 * find the best match for the next state (may be reset) if:
		 * 1.	the next value equals sequence
		 * 2.	the next value is not the beginning of sequence
		 * otherwise generate next state
		 */
		next[length] = '0';
		if (valuescmp(sequence, next) == 0 || valuesncmp(sequence, next, next_length) != 0)
			state->left = find_match(sequence, reset, next);
		else
			state->left = create_states(sequence, reset, next);

		next[length] = '1';
		if (valuescmp(sequence, next) == 0 || valuesncmp(sequence, next, next_length) != 0)
			state->right = find_match(sequence, reset, next);
		else
			state->right = create_states(sequence, reset, next);
	}

	printf("Q: value:%4s\tleft:%4s\tright:%4s\n", state->value, state->left->value, state->right->value);
	return state;
}

void add_state_to_graph(State *reset, State *state) {
	int length = strlen(state->value);

	// find state's parent
	State *parent = reset;
	for (int i = 0; i < length - 1; ++i) {
		if (state->value[i] == '0')
			parent = parent->left;
		else  // value[i] == '1'
			parent = parent->right;
	}

	// insert state as child
	if (state->value[length - 1] == '0')
		parent->left = state;
	else  // value[i] == '1'
		parent->right = state;
}

State *find_match(Sequence *sequence, State *reset, char *value) {
	++value;  // start from the 2nd character
	int length = strlen(value);
	if (length == 0)
		return reset;

	State *match = reset;
	bool exists = true;
	for (int i = 0; exists && i < length; ++i) {
		if (value[i] == '0') {
			if (match->left == NULL || match->left == reset)
				exists = false;
			match = match->left;
		} else {  // value[i] == '1'
			if (match->right == NULL || match->right == reset)
				exists = false;
			match = match->right;
		}
	}
	if (!exists)
		return find_match(sequence, reset, value);	// find match with next substring
	return match;
}

int valuescmp(Sequence *sequence, char *value) {
	char *s = sequence->value;
	if (strlen(s) != strlen(value))
		return 1;

	while (*s) {
		if (*s != '-' && *s != *value)
			return 1;
		++s;
		++value;
	}
	return 0;
}

int valuesncmp(Sequence *sequence, char *value, int n) {
	char *s = sequence->value;
	while (n-- && *s) {
		if (*s != '-' && *s != *value)
			return 1;
		++s;
		++value;
	}
	return 0;
}

StatesList *get_states_list(Sequence *sequence) {
	State *reset = malloc(sizeof(State));
	if (reset == NULL)
		error_exit("Could not create reset node: %s\n", strerror(errno));
	reset->left = reset->right = NULL;
	strcpy(reset->value, "-");
	sequence->nstates = 1;
	create_states(sequence, reset, "");

	StatesList *states_list = malloc(sizeof(StatesList) + sizeof(State * [sequence->nstates]));
	if (states_list == NULL)
		error_exit("Could not create states list: %s\n", strerror(errno));

	states_list->nstates = 0;
	list_add(states_list, reset);
	list_qsort(states_list, 0, states_list->nstates - 1);
	return states_list;
}

void list_add(StatesList *list, State *state) {
	if (list_contains(list, state))
		return;

	list->states[list->nstates++] = state;
	list_add(list, state->left);
	list_add(list, state->right);
}

bool list_contains(StatesList *list, State *state) {
	for (int i = 0; i < list->nstates; ++i)
		if (list->states[i] == state)
			return true;
	return false;
}

void list_qsort(StatesList *list, int left, int right) {
	// quick sort is O(n*log(n))
	if (left >= right)
		return;

	int i = left;
	int j = right;
	int pivot = left;

	while (i <= j) {
		while (seqcmp(list->states[i]->value, list->states[pivot]->value) <= 0 && i < right)
			i++;

		while (seqcmp(list->states[j]->value, list->states[pivot]->value) > 0)
			j--;

		if (i <= j) {
			swap(list->states, i, j);
			i++;
			j--;
		}
	}

	swap(list->states, pivot, j);

	if (left < j)
		list_qsort(list, left, j);
	if (i < right)
		list_qsort(list, i, right);
}

int list_index(StatesList *list, State *state) {
	for (int i = 0; i < list->nstates; ++i)
		if (list->states[i] == state)
			return i;
	return -1;
}

void delete_states_list(StatesList **list) {
	for (int i = 0; i < (*list)->nstates; ++i)
		free((*list)->states[i]);

	free(*list);
	*list = NULL;
}

int seqcmp(const char *seq1, const char *seq2) {
	size_t len1 = strlen(seq1);
	size_t len2 = strlen(seq2);
	if (len1 != len2)
		return len1 - len2;
	return strcmp(seq1, seq2);
}

void swap(State *states[], int i, int j) {
	State *temp = states[i];
	states[i] = states[j];
	states[j] = temp;
}

void error_exit(char *format, ...) {
	va_list args;
	va_start(args, format);
	if (isterminal(getfileno(stderr)))	// use red color if stderr is a terminal
		fprintf(stderr, "\033[91m[X] ERROR\033[0m");
	else
		fprintf(stderr, "[X] ERROR");

	fputs(": ", stderr);
	vfprintf(stderr, format, args);
	va_end(args);
	exit(1);
}
