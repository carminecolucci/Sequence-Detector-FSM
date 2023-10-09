# Sequence Detector with a Finite State Machine
Generates a Finite State Machine to recognize a binary sequence

`sequence` can contain only `0`, `1` and don't cares (`-`).

A don't care means either a `0` or a `1`.

Let `n` be the length of the sequence to be detected.

The output of the fsm is either a `0` (do not match) or a `1` (match).

The machine can recognize sequences in 3 modes:
- `NOT_OVERLAPPING`: Sequences cannot have overlapping bits (like a password). The fsm reads `n` bits of the input sequence before trying to match.
  After an output is given, the machine state becomes `reset` and a new sequence can start.
- `PARTIALLY_OVERLAPPING`: Sequences can have overlapping bits until a match is found. When a match is found, the machine state becomes `reset`.
- `TOTALLY_OVERLAPPING`: Sequences can have overlapping bits. The machine is always trying to find a match. The state becomes `reset` only if there is no possible match.

## Example
```bash
$ make
$ ./fsm

Finite State Machine
Author: Carmine
Date:   06/08/2022
Desc:   Finite state machine to recognize a given binary sequence

Enter a sequence of [0, 1, -] [max 4 digits]: 0-1-
Enter overlap mode [0 None, 1 Partially, 2 Totally]: 1
Calculating states for 0-1-, overlap mode: 1...

```
The output table for this sequence is:
``` bash
Optimized table:

Table:
┌────────────┬──────────┬──────────┐
│State       │     0    │     1    │
├────────────┼──────────┼──────────┤
│Q0    -     │     1/0  │     0/0  │
├────────────┼──────────┼──────────┤
│Q1    0     │     2/0  │     3/0  │
├────────────┼──────────┼──────────┤
│Q2    00    │     2/0  │     4/0  │
├────────────┼──────────┼──────────┤
│Q3    01    │     1/0  │     4/0  │
├────────────┼──────────┼──────────┤
│Q4    0-1   │     0/1  │     0/1  │
└────────────┴──────────┴──────────┘
```

Each cell of the table consists of two parts: `next state` / `exit value`, where

- `next state` is the state in which the fsm goes after the input bit
- `exit value` is the output of the fsm 
