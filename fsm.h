/**
 * @Author:	Carmine
 * @Date:	06/08/2022
 * @Name:	fsm.h
 * @Desc:	Finite state machine to recognize a given sequence
 */

#if !defined(FSM_H)
#define FSM_H

typedef enum overlapping_mode { NOT_OVERLAPPING, PARTIALLY_OVERLAPPING, TOTALLY_OVERLAPPING } overlapping_mode;

#define MAXSEQ 4
typedef struct sequence {
	char value[MAXSEQ + 1];
	int length;
	overlapping_mode mode;
	int nstates;
} Sequence;

typedef struct state {
	char value[MAXSEQ + 1];
	struct state *left;
	struct state *right;
} State;

void get_sequence(Sequence *sequence);
overlapping_mode get_mode();

/**
 *  Creates all the states for the given sequence
 */
State *create_states(Sequence *sequence, State *reset, char *value);

/**
 * Adds new states to the graph. The first node is reset.
 * Needed to find the best match.
 */
void add_state_to_graph(State *reset, State *state);

/**
 * Returns the best match for a given sequence, or reset if it does not exist.
 * only for partial and full overlapping.
 * Finds the longest rightmost substring of value that equals the beginning of sequence.
 */
State *find_match(Sequence *sequence, State *reset, char *value);

/**
 * Returns 0 if sequence equals value.
 * Don't care ('-') matches both "0" and "1" ('-' cannot be present in value by construction).
 */
int valuescmp(Sequence *sequence, char *value);

/**
 * Returns 0 if the first n characters of sequence and value are equal.
 * Don't care ('-') matches both "0" and "1".
 */
int valuesncmp(Sequence *sequence, char *value, int n);

typedef struct stateslist {
	int nstates;
	State *states[];
} StatesList;

/**
 * Generates all the states for sequence, and returns them in a list.
 */
StatesList *get_states_list(Sequence *sequence);
void list_add(StatesList *list, State *state);
bool list_contains(StatesList *list, State *state);
void list_qsort(StatesList *list, int left, int right);
int list_index(StatesList *list, State *state);

/**
 * Deletes all the states and the list.
 */
void delete_states_list(StatesList **list);

/**
 * Returns 0 if seq1 and seq2 are _exactly_ equal.
 */
int seqcmp(const char *seq1, const char *seq2);
void swap(State *states[], int i, int j);

void error_exit(char *format, ...);

#endif /* FSM_H */
