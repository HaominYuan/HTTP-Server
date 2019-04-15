INCS = ./include

run : main.c ./include/*.c
	gcc -o $@ $^ -I $(INCS) -Wall 
