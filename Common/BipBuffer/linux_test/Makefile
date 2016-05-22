
OBJ = main_print.o BipBuffer.o

hello: $(OBJ)
	gcc -o hello $(OBJ)
    
main_print.o: BipBuffer.h
	gcc -c main_print.c

BipBuffer.o: BipBuffer.h
	gcc -c BipBuffer.c

.PHONY : clean
clean:
	-rm -rf hello
	-rm -rf $(OBJ)
	
