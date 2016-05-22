
OBJ = main.o AaMem.o

test: $(OBJ)
	gcc -pthread -o test $(OBJ)
    
main.o: AaMem.h
	gcc -c main.c

AaMem.o: AaMem.h
	gcc -c AaMem.c

.PHONY : clean
clean:
	-rm -rf test
	-rm -rf $(OBJ)
	
