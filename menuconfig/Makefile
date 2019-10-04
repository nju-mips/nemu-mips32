.DEFAULT_GOAL := mconf

OBJ_DIR ?= build
CFLAGS := -Wall -O2 -fomit-frame-pointer -std=gnu89 -DYYDEBUG

OBJS := \
  confdata.o \
  symbol.o \
  lxdialog/yesno.o \
  lxdialog/util.o \
  lxdialog/checklist.o \
  lxdialog/inputbox.o \
  lxdialog/textbox.o \
  lxdialog/menubox.o \
  preprocess.o \
  expr.o \
  lexer.lex.o \
  mconf.o \
  parser.tab.o

OBJS := $(OBJS:%=$(OBJ_DIR)/%)

./parser.tab.c ./parser.tab.h: parser.y
	@echo + YACC $@
	@bison -v $< --defines=./parser.tab.h -o ./parser.tab.c

./lexer.lex.c: lexer.l ./parser.tab.h
	@echo + LEX $@
	@flex -o $@ $<

$(OBJ_DIR)/%.o: ./%.c
	@echo + CC $<
	@mkdir -p $(@D)
	@gcc -c $(CFLAGS) $< -o $@

mconf: $(OBJS)
	@echo + GEN $@
	@gcc $^ -o $@ -lncurses -ltinfo

conf: $(patsubst %/mconf.o,%/conf.o,$(OBJS))
	@echo + GEN $@
	@gcc $^ -o $@ -lncurses -ltinfo

clean:
	rm -rf conf mconf
	rm -rf parser.tab.* lexer.lex.*
	rm -rf $(OBJS)
