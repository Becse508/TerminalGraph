CFLAGS = 
LDLIBS = -lm

objects = buffer.o canvas.o converters.o drawing.o graph.o


cli : main.c parser.o $(objects)
	cc $(CFLAGS) -o tgcli main.c parser.o $(objects) $(LDLIBS)

test : test.c $(objects)
	cc $(CFLAGS) -o test test.c $(objects) $(LDLIBS)

parser.o : parser.h
buffer.o : defs.h buffer.h
canvas.o : defs.h canvas.h
converters.o : defs.h canvas.h converters.h
drawing.o : defs.h buffer.h drawing.h
graph.o : defs.h buffer.h drawing.h graph.h

clean:
	rm -f *.o tgcli test