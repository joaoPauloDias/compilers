CFLAGS=-fsanitize=address -Wall

TARGET = etapa2
LEX_OUTPUT = lex.yy.c lex.yy.h
PARSER_OUTPUT = parser.tab.c parser.tab.h
OBJS = parser.tab.o lex.yy.o main.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

parser.tab.c: parser.y
	bison -d $<

lex.yy.c: scanner.l
	flex $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJS) $(LEX_OUTPUT) $(PARSER_OUTPUT)

.PHONY: all clean
