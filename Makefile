CC=gcc
CFLAGS=-I.

TARGET = etapa1
LEX_OUTPUT = lex.yy.c 
OBJS = lex.yy.o main.o 

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

lex.yy.o: lex.yy.c
	$(CC) $(CFLAGS) -c $<

lex.yy.c: scanner.l
	flex $<

main.o: main.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(TARGET) $(OBJS) $(LEX_OUTPUT)

.PHONY: all clean