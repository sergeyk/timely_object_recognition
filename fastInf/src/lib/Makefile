# makefile for lb lib
# Ariel Jaimovich 
## Modified by Ian McGraw

SHELL = /bin/sh

ROOTDIR = ../..
include $(ROOTDIR)/src/Makefile.config


LIBSRC = lbRandomProb.cpp lbOptions.cpp lbMessageBank.cpp		\
lbMeasureDispatcher.cpp lbUtils.cpp lbValue.cpp lbMessageQueue.cpp	\
lbRegionGraph.cpp lbDirectedGraph.cpp lbGraphBase.cpp			\
lbGraphStruct.cpp lbSubgraph.cpp lbSupport.cpp lbDefinitions.cpp	\
lbRegionModel.cpp lbDAG.cpp lbVariableElimination.cpp lbMeasure.cpp	\
lbAssignedMeasure.cpp lbModel.cpp lbMessage.cpp lbSpanningTree.cpp	\
lbMultinomialMeasure.cpp lbDriver.cpp					\
Matrix.cpp							\
lbFeatureTableMeasure.cpp lbWeightedTableMeasure.cpp			\
lbInferenceMonitor.cpp lbInferenceObject.cpp lbBeliefPropagation.cpp	\
lbPropagationInference.cpp lbRegionBP.cpp \
lbMeanField.cpp \
lbBasicGraph.cpp lbJunctionTree.cpp \
inferUtils.cpp

all: directory $(LIBBLDDIR)/$(LIBBASE)

directory:
	@if test ! -d $(LIBBLDDIR); then mkdir -p $(LIBBLDDIR); fi;

$(LIBBLDDIR)/$(LIBBASE): $(addprefix $(LIBBLDDIR)/,$(LIBSRC:.cpp=.o))
	ar rv $@ $?
	ranlib $@

$(LIBBLDDIR)/%.o: $(LIBSRCDIR)/%.cpp
	$(GPP) $(CPPFLAGS) -c -o $@ $<

clean:
	-$(RM) $(LIBBLDDIR)/*.o $(LIBBLDDIR)/$(LIBBASE)

depend makedep: directory _make.dep
	mv -f $(LIBBLDDIR)/_make.dep $(LIBBLDDIR)/make.dep  

_make.dep: $(addprefix $(LIBSRCDIR)/,$(LIBSRC))
	@echo making depend
	$(SHELL) -ec '$(GPP) -MM $(CPPFLAGS) -I. $^' | sed 's/\(.*\.o:\)/\.\.\/\.\.\/build\/lib\/\1/g' > $(LIBBLDDIR)/_make.dep

sinclude $(LIBBLDDIR)/make.dep
