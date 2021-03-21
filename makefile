# MMN14 makeFile
# ============================================================================
# Name        : assembler
# Author      : snirm15@gmail.com
# Description : MMN14- assembler
# ============================================================================

CC = gcc
CFLAGS =  -Wall -ansi -pedantic
MAIN = assembler.c
DEPENDS = code_proc.c string_structs.c
EXE_NAME = assembler

$(EXE_NAME): $(MAIN) $(DEPENDS:.c=.o)
	$(CC) $(CFLAGS) $(MAIN) $(DEPENDS:.c=.o) -o $@ -lm

# for each .o dependency, compile it from the corresponding .c file (depends on the corresponding .h file also).
%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

# running "make clean" in local directory runs the commands specified here.
clean:
	rm *.o $(EXE_NAME) *.obj *.ext *.ent