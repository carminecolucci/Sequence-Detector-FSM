GCC=gcc
FLAGS=-Wall -Wextra -Wpedantic -Werror
EXE=fsm
OBJ=fsm.o table.o

$(EXE): $(OBJ)
	$(GCC) $(FLAGS) -o $(EXE) $(OBJ)

fsm.o: fsm.c fsm.h
	$(GCC) $(FLAGS) -c fsm.c

table.o: table.c table.h
	$(GCC) $(FLAGS) -c table.c

clean:
	${RM} $(EXE) $(OBJ)
