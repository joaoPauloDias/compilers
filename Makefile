TYPE ?= debug

ifeq ($(TYPE),debug)
	CFLAGS := -fsanitize=address -Wall
else ifeq ($(TYPE), release)
	CFLAGS := -O3
else ifeq ($(TYPE), coverage)
	CC := gcc
	CFLAGS := -fsanitize=address -Wall -fprofile-arcs -ftest-coverage
endif

TARGET = etapa2
LEX_OUTPUT = lex.yy.c lex.yy.h
PARSER_OUTPUT = parser.tab.c parser.tab.h
OBJS = parser.tab.o lex.yy.o main.o

TEST_FILES := $(wildcard tests/*.txt)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

parser.tab.c: parser.y
	bison -d -g $<

lex.yy.c: scanner.l parser.tab.c
	flex $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: test

test: all $(TEST_FILES)
	@for file in $(TEST_FILES); do \
		echo "Testing $$file"; \
		./$(TARGET) < $$file || exit 1; \
	done

.PHONY: coverage

coverage: all $(TEST_FILES)
	lcov --directory . --zerocounters
	@for file in $(TEST_FILES); do \
		echo "Testing $$file"; \
		./$(TARGET) < $$file; \
	done
	lcov --capture --directory . --output-file coverage.info
	genhtml coverage.info --output-directory coverage_report

.PHONY: clean

clean:
	rm -f $(TARGET) $(OBJS) $(LEX_OUTPUT) $(PARSER_OUTPUT) *.gv *.info
	lcov --directory . --zerocounters
	rm -rf coverage_report
