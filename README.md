# Finite-State-Machine
Generates a Finite State Machine to recognize a binary sequence

`sequence` can contain only `0`, `1` and don't cares (`-`).

A don't care means either a `0` or a `1`.

Let `n` be the length of the sequence to be recognized.

The output of the fsm is either a `0` (do not match) or a `1` (match).

The machine can recognize sequences in 3 modes:
- `NOT_OVERLAPPING`: Sequences cannot have overlapping bits (like a password). The fsm reads `n` bits of the input sequence before trying to match.
  After an output is given, the machine state becomes `reset` and a new sequence can start.
- `PARTIALLY_OVERLAPPING`: Sequences can have overlapping bits until a match is found. When a match is found, the machine state becomes `reset`.
- `TOTALLY_OVERLAPPING`: Sequences can have overlapping bits. The machine is always trying to find a match. The state becomes `reset` only if there is no possible match.
