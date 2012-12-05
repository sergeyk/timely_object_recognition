# makefile for lb lib
# Ariel Jaimovich 
## Modified by Ian McGraw

SHELL = /bin/sh

ROOTDIR = ../..
include $(ROOTDIR)/src/Makefile.config

LEARNSRC =  lbSuffStat.cpp GSLOptimizer.cpp lbGSLLearningObject.cpp \
ObjectiveFunction.cpp \
lbNewtonOptimizer.cpp lbGradientAscent.cpp

all: directory $(LEARNBLDDIR)/$(LIBLEARN)

directory:
	@if test ! -d $(LEARNBLDDIR); then mkdir -p $(LEARNBLDDIR); fi;

$(LEARNBLDDIR)/$(LIBLEARN): $(addprefix $(LEARNBLDDIR)/,$(LEARNSRC:.cpp=.o))
	ar rv $@ $?
	ranlib $@


$(LEARNBLDDIR)/%.o: $(LEARNSRCDIR)/%.cpp
	$(GPP) $(CPPFLAGS) -l $(LIBBLDDIR)/$(LIBBASE) -c -o $@ $<

clean:
	-$(RM) $(LEARNBLDDIR)/*.o $(LEARNBLDDIR)/$(LIBLEARN)

depend makedep: directory _make.dep
	mv -f $(LEARNBLDDIR)/_make.dep $(LEARNBLDDIR)/make.dep  

_make.dep: $(addprefix $(LEARNSRCDIR)/,$(LEARNSRC))
	@echo making depend
	$(SHELL) -ec '$(GPP) -MM $(CPPFLAGS) -I. $^' | sed 's/\(.*\.o:\)/\.\.\/\.\.\/build\/learn\/\1/g' > $(LEARNBLDDIR)/_make.dep

sinclude $(LEARNBLDDIR)/make.dep
