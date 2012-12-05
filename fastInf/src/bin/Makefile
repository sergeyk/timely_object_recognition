# Makefile for experiments
# Author: Ian McGraw

SHELL = /bin/sh

ROOTDIR = ../..

include $(ROOTDIR)/src/Makefile.config


BIN = infer makeGrid makeFullGraph mfinfer gibbsSample learning infer_timely
FULLBIN = $(addprefix $(BINBLDDIR)/,$(BIN))
BINSRC = $(addsuffix .cpp,$(addprefix $(BINSRCDIR)/,$(BIN)))

all: directory $(FULLBIN)

directory:
	@if test ! -d $(BINBLDDIR); then mkdir -p $(BINBLDDIR); fi;

clean:
	-$(RM) $(BINBLDDIR)/*.o 
	-$(RM) $(addprefix $(BINBLDDIR)/,$(BIN))

depend makedep: directory _make.dep
	mv -f $(BINBLDDIR)/_make.dep $(BINBLDDIR)/make.dep  

_make.dep: $(BINSRC)
	@echo making depend
	$(SHELL) -ec '$(GPP) -MM $(CPPFLAGS) -I. $^' | sed 's/\(.*\.o:\)/\.\.\/\.\.\/build\/bin\/\1/g' > $(BINBLDDIR)/_make.dep

$(BINBLDDIR)/%: $(BINSRCDIR)/%.cpp $(LIBBLDDIR)/$(LIBBASE) $(LEARNBLDDIR)/$(LIBLEARN)
	$(GPP) $(CPPFLAGS) -o $@ $^ $(LIB)
