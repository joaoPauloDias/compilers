TYPE ?= debug

ifeq ($(TYPE),debug)
    CFLAGS := -g -Wall -std=gnu17 -fsanitize=address
else ifeq ($(TYPE), release)
    CFLAGS := -O3 -std=gnu17
else ifeq ($(TYPE), coverage)
    CC := gcc
    CFLAGS := -g -Wall -std=gnu17 -fprofile-arcs -ftest-coverage
endif

TARGET = etapa6
LEX_OUTPUT = lex.yy.c lex.yy.h
PARSER_OUTPUT = parser.tab.c parser.tab.h
OBJS = parser.tab.o lex.yy.o main.o asd.o arena.o symbol_table.o errors.o code_utils.o asm.o

TEST_FILES := $(wildcard tests/*.txt) 
SIMULATOR = ./ilocsim.py

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
		echo -n "$$file"; \
		./$(TARGET) < $$file > tests/$$(basename $$file .txt).s; \
		gcc tests/$$(basename $$file .txt).s -o  tests/program_$$(basename $$file .txt); \
		./tests/program_$$(basename $$file .txt); \
		ret_val=$$?; \
		echo "\nReturn value: $$ret_val"; \
	done

.PHONY: coverage

coverage: all $(TEST_FILES)
	lcov --directory . --zerocounters
	@for file in $(TEST_FILES); do \
		echo "Testing $$file"; \
		./$(TARGET) < $$file > /dev/null; \
	done
	lcov --capture --directory . --output-file coverage.info
	genhtml coverage.info --output-directory coverage_report

.PHONY: clean

clean:
	rm -f $(TARGET) $(OBJS) $(LEX_OUTPUT) $(PARSER_OUTPUT); \
	rm -f *.gv *.info *.gcno *.dot
	rm -f tests/*.iloc tests/*.stats tests/*.s tests/program_*
	lcov --directory . --zerocounters
	rm -rf coverage_report
