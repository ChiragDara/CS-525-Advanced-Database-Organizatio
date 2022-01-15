# -*- MakeFile -*-
CC := gcc
SRC := dberror.c storage_mgr.c test_assign1_1.c
OBJ := dberror.o storage_mgr.o test_assign1_1.o
assignment1: $(OBJ)
	$(CC) -o test_assign1 $(SRC)
$(OBJ): $(SRC)
	$(CC) -g -c $(SRC)
run: assignment1
	./test_assign1
clean:
	rm -rf test_assign1 *.o