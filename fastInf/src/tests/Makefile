# Makefile for experiments
# Author: Ian McGraw

SHELL = /bin/sh

ROOTDIR = ../..
include $(ROOTDIR)/src/Makefile.config


TESTS = measureTest modelTest graphTest suffStatTest

FULLTEST = $(addprefix $(TSTBLDDIR)/,$(TESTS))

all: $(FULLTEST)

clean:
	-$(RM) $(TSTBLDDIR)/*.o 
	-$(RM) $(addprefix $(TSTBLDDIR)/,$(TESTS))

$(TSTBLDDIR)/%:	$(TSTSRCDIR)/%.cpp $(LIBBLDDIR)/$(LIBBASE) $(LEARNBLDDIR)/$(LIBLEARN)
	@echo "*** Compiling '$*' ***" 
	@if test ! -d $(TSTBLDDIR); then mkdir -p $(TSTBLDDIR); fi;
	$(GPP) $(CPPFLAGS) -o $@ $^ $(LIB)

