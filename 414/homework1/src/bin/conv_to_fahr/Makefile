#########################################################################
# Student: Jason Rice
#
# Course: 605.414 Systems Development in a Unix Environment
#
# File: Makefile
#
# Description:
#   This is a node (leaf) level Makefile and acts upon the contents
# encapsulated within the current working directory. This Makefile is
# responsible for compiling and linking in necessary libraries in order
# to generate a conv_to_fahr executable that is then place within the
# 414/homework1/bin directory as 'conv_to_fahr' This makefile contains
# no dependencies and is used during the clean target.
#########################################################################
ROOT = ../../..
LIBS = $(ROOT)/lib
LIB_NAME = temp_conv
BIN = $(ROOT)/bin
INCLUDE = $(ROOT)/include

PROGRAM = conv_to_fahr
# The program source will have main in it.
PROGRAM_SRC = $(PROGRAM).c
OBJS = $(PROGRAM_SRC:.c=.o)
# CFLAGS combines all the libraries. These should have been previously built.
CFLAGS = -I$(INCLUDE) -L$(LIBS) -l$(LIB_NAME)
CC   = gcc

# Suffix rule for autocompiling sources with local include.
.c.o:
	$(CC) $(CFLAGS) -c $<

$(PROGRAM): $(OBJS)
	$(CC) $(OBJS) $(CFLAGS) -o $(PROGRAM)

###
# Note that a 'make install' against the libraries within
# ./414/homework1/src/lib/ for 'make it' to work here.
###
it: $(PROGRAM)
	@echo "Compiling sources with libraries."

install: $(PROGRAM)
	cp $(PROGRAM) $(BIN)
	@echo "Executable named '$(PROGRAM)' was placed in '$(BIN)' and is ready to run."

clean: 
	@echo "Cleaning up object files and executables. Clearing dependencies."
	rm -f $(OBJS) $(BIN)/$(PROGRAM) $(PROGRAM)
	cp Makefile.nodependencies Makefile

depend: $(PROGRAM_SRC)
	@echo "Generating the dependencies."
	$(CC) -M $(CFLAGS) $(PROGRAM_SRC) > dependencies
	sed -e '1,/^# DO NOT DELETE/!d' Makefile > make.tmp
	cat dependencies >> make.tmp
	mv make.tmp Makefile
	rm dependencies

# DO NOT DELETE THIS LINE - make depend uses this
conv_to_fahr.o: conv_to_fahr.c /usr/include/stdio.h \
 /usr/include/sys/feature_tests.h /usr/include/sys/ccompile.h \
 /usr/include/sys/isa_defs.h /usr/include/iso/stdio_iso.h \
 /usr/include/sys/va_list.h /usr/include/stdio_tag.h \
 /usr/include/stdio_impl.h /usr/include/iso/stdio_c99.h \
 /usr/include/stdlib.h /usr/include/iso/stdlib_iso.h \
 /usr/include/iso/stdlib_c99.h /usr/include/string.h \
 /usr/include/iso/string_iso.h ../../../include/temp_conv.h
