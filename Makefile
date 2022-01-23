# CC - compiler
# OBJ - compiled source files that should be linked
# COPT - compiler flags
# BIN - binary
CC=clang
OBJ=helper1.o
P2OBJ=p2helper.o
COPT=-Wall -Wpedantic -g
BIN_PHASE1=phase1
BIN_PHASE2=dns_svr

# Running "make" with no argument will make the first target in the file
all: $(BIN_PHASE1) $(BIN_PHASE2)


$(BIN_PHASE2): main.c $(P2OBJ)
	$(CC) -o $(BIN_PHASE2) main.c $(P2OBJ) $(COPT)

$(BIN_PHASE1): phase1.c $(OBJ)
	$(CC) -o $(BIN_PHASE1) phase1.c $(OBJ) $(COPT)

# Wildcard rule to make any  .o  file,
# given a .c and .h file with the same leading filename component
%.o: %.c %.h
	$(CC) -c $< $(COPT) -g

format:
	clang-format -i *.c *.h

clean:
	rm -f *.o phase1 dns_svr
