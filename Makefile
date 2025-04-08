TYPE ?= debug

ifeq ($(TYPE),debug)
	CFLAGS=-fsanitize=address -Wall
else ifeq ($(TYPE), coverage)
	CC = gcc
	CFLAGS=-fsanitize=address -Wall -fprofile-arcs -ftest-coverage
endif

TARGET = etapa2
LEX_OUTPUT = lex.yy.c lex.yy.h
PARSER_OUTPUT = parser.tab.c parser.tab.h
OBJS = parser.tab.o lex.yy.o main.o

all: $(TARGET)

TEST_FILES := $(wildcard tests/*.txt)

test: all $(TEST_FILES)
	lcov --directory . --zerocounters
	@for file in $(TEST_FILES); do \
		echo "Testing $$file"; \
		./etapa2 < $$file; \
	done
	lcov --capture --directory . --output-file coverage.info
	genhtml coverage.info --output-directory coverage_report

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

parser.tab.c: parser.y
	bison -d -g $<

lex.yy.c: scanner.l parser.tab.c
	flex $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJS) $(LEX_OUTPUT) $(PARSER_OUTPUT)

.PHONY: all clean
