TYPE ?= debug

ifeq ($(TYPE),debug)
	CFLAGS=-fsanitize=address -Wall -fprofile-arcs -ftest-coverage -O0
else ifeq ($(TYPE), coverage)
	CC = gcc
	CFLAGS=-fsanitize=address -Wall -fprofile-arcs -ftest-coverage -O0
endif

TARGET = etapa2
LEX_OUTPUT = lex.yy.c lex.yy.h
PARSER_OUTPUT = parser.tab.c parser.tab.h
OBJS = parser.tab.o lex.yy.o main.o

all: $(TARGET)

test: all
	lcov --directory . --zerocounters
	./etapa2 < tests/all.txt
	lcov --capture --directory . --output-file coverage.info
	genhtml coverage.info --output-directory coverage_report

$(TARGET): $(OBJS)
	gcc $(CFLAGS) -o $@ $^

parser.tab.c: parser.y
	bison -d $<

lex.yy.c: scanner.l parser.tab.c
	flex $<

%.o: %.c
	gcc $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJS) $(LEX_OUTPUT) $(PARSER_OUTPUT)

.PHONY: all clean
