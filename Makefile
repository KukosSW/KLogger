# Simple Makefile - this project is simple we dont need a BIG, OP Makefile

# SHELL
RM := rm -rf
AR := ar rcs

# PRETTY PRINTS
define print_cc
	$(if $(Q), @echo "[CC]        $(1)")
endef

define print_bin
	$(if $(Q), @echo "[BIN]       $(1)")
endef

define print_rm
    $(if $(Q), @echo "[RM]        $(1)")
endef

define print_ar
    $(if $(Q), @echo "[AR]        $(1)")
endef


# ARGS (Quiet OR Verbose), type make V=1 to enable verbose mode
ifeq ("$(origin V)", "command line")
	Q :=
else
	Q ?= @
endif

# DIRS
SDIR := ./src
IDIR := ./inc
ADIR := ./example

SCRIPT_DIR := ./scripts

# FILES
SRC := $(wildcard $(SDIR)/*.c)
ASRC := $(SRC) $(wildcard $(ADIR)/*.c)

LOBJ := $(SRC:%.c=%.o)
AOBJ := $(ASRC:%.c=%.o)
OBJ := $(AOBJ) $(LOBJ)

DEPS := $(OBJ:%.o=%.d)

# LIBS remember -l is added automaticly so type just m for -lm
LIB := pthread

# BINS
AEXEC := example.out
LIB_NAME := libklogger.a

# COMPI, DEFAULT GCC
CC ?= gcc

C_STD   := -std=gnu17
C_OPT   := -O3
C_FLAGS :=
C_WARNS :=

DEP_FLAGS := -MMD -MP
LINKER_FLAGS := -fPIC

H_INC := $(foreach d, $(IDIR), -I$d)
L_INC := $(foreach l, $(LIB), -l$l)

ifeq ($(CC),clang)
	C_WARNS += -Weverything -Wno-padded
else ifneq (, $(filter $(CC), cc gcc))
	C_WARNS += -Wall -Wextra -pedantic -Wcast-align \
			   -Winit-self -Wlogical-op -Wmissing-include-dirs \
			   -Wredundant-decls -Wshadow -Wstrict-overflow=5 -Wundef  \
			   -Wwrite-strings -Wpointer-arith -Wmissing-declarations \
			   -Wuninitialized -Wold-style-definition -Wstrict-prototypes \
			   -Wmissing-prototypes -Wswitch-default -Wbad-function-cast \
			   -Wnested-externs -Wconversion -Wunreachable-code
endif

ifeq ("$(origin DEBUG)", "command line")
	GGDB := -ggdb3
else
	GGDB :=
endif

INSTALL_PATH =

# Path for install KLogger
ifeq ("$(origin P)", "command line")
  INSTALL_PATH = $(P)
endif

C_FLAGS += $(C_STD) $(C_OPT) $(GGDB) $(C_WARNS) $(DEP_FLAGS) $(LINKER_FLAGS)

all: lib examples

lib: $(LIB_NAME)

install: __FORCE
	$(Q)$(SCRIPT_DIR)/install_klogger.sh $(INSTALL_PATH)


__FORCE:


$(LIB_NAME): $(LOBJ)
	$(call print_ar,$@)
	$(Q)$(AR) $@ $^

examples: $(AEXEC)

$(AEXEC): $(AOBJ)
	$(call print_bin,$@)
	$(Q)$(CC) $(C_FLAGS) $(H_INC) $(AOBJ) -o $@ $(L_INC)

%.o:%.c %.d
	$(call print_cc,$<)
	$(Q)$(CC) $(C_FLAGS) $(H_INC) -c $< -o $@

clean:
	$(call print_rm,EXEC)
	$(Q)$(RM) $(AEXEC)
	$(Q)$(RM) $(LIB_NAME)
	$(call print_rm,OBJ)
	$(Q)$(RM) $(OBJ)
	$(call print_rm,DEPS)
	$(Q)$(RM) $(DEPS)

help:
	@echo "KLogger Makefile"
	@echo -e
	@echo "Targets:"
	@echo "    all               - build klogger and examples"
	@echo "    lib               - build only klogger library"
	@echo "    examples          - examples"
	@echo "    install[P = Path] - install klogger to path P or default Path"
	@echo -e
	@echo "Makefile supports Verbose mode when V=1"
	@echo "To check default compiler (gcc) change CC variable (i.e export CC=clang)"

$(DEPS):


include $(wildcard $(DEPS))
