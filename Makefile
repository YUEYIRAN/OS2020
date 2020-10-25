# To run, enter
# make all

all: prod cons dph 

prod: prod.c
	gcc prod.c -lm -lrt -lpthread -o prod

cons: cons.c
	gcc cons.c -lm -lrt -lpthread -o cons

dph: dph.c
	gcc dph.c -lrt -lpthread -o dph
