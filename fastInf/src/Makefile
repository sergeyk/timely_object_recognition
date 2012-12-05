
ROOTDIR = ..
include $(ROOTDIR)/src/Makefile.config

## To compile a tag table understandable by emacs (this is a tag
## table for both the library and the test files)
TAGS_SOURCES = `find $(SRCDIR)/ -path $(BOOSTDIR) -prune -o -regex ".*[ch]\(pp\)?" -print`
TAGSFILE = TAGS

all:
	@echo ""
	@echo "******************************************************" 
	@echo "************** MAKING INFERENCE LIBRARY **************" 
	@echo "******************************************************" 
	$(MAKE) -C $(LIBSRCDIR) all
# When adding a new file run: $(MAKE) -C $(LIBSRCDIR) depend

	@echo ""
	@echo "*****************************************************" 
	@echo "************** MAKING LEARNING LIBRARY **************"
	@echo "*****************************************************"  
	$(MAKE) -C $(LEARNSRCDIR) all
# When adding a new file run: $(MAKE) -C $(LEARNSRCDIR) depend

	@echo ""
	@echo "******************************************" 
	@echo "************** MAKING TESTS **************" 
	@echo "******************************************" 
	$(MAKE) -C $(TSTSRCDIR)

	@echo ""
	@echo "****************************************" 
	@echo "************** MAKING BIN **************"
	@echo "****************************************"  
	$(MAKE) -C $(BINSRCDIR)

lib:
	$(MAKE) -C $(LIBSRCDIR) all

learn:
	$(MAKE) -C $(LEARNSRCDIR) all

examples:
	$(MAKE) -C $(EXASRCDIR) 

tests:
	$(MAKE) -C $(TSTSRCDIR) 

bin:
	$(MAKE) -C $(BINSRCDIR) 

depend:
	$(MAKE) -C $(LIBSRCDIR) depend
	$(MAKE) -C $(LEARNSRCDIR) depend
	$(MAKE) -C $(BINSRCDIR) depend

clean:
	cd $(LIBSRCDIR) && $(MAKE) clean
	cd $(EXASRCDIR) && $(MAKE) clean
	cd $(TSTSRCDIR) && $(MAKE) clean
	cd $(BINSRCDIR) && $(MAKE) clean
	cd $(LEARNSRCDIR) && $(MAKE) clean

clnsrc:
	find ./ -name "*\~" -exec rm -v {} \;
	find ./ -name "*\\#*\#" -exec rm -v {} \;

clobber:
	-$(RM) -rf $(BLDDIR)

TAGS:
	etags --members $(TAGS_SOURCES) -o $(BLDDIR)/$(TAGSFILE)

EXTLIBDIR = $(ROOTDIR)/extLibs
ROOTPATH = $(realpath $(ROOTDIR))
externalLibs:
	mkdir -p $(EXTLIBDIR)
	@echo "Downloading gsl and glpk..."
	cd $(EXTLIBDIR) && \
wget http://ftp.gnu.org/gnu/gsl/gsl-1.14.tar.gz && \
tar xzvf gsl-1.14.tar.gz && \
wget http://ftp.gnu.org/gnu/glpk/glpk-4.43.tar.gz && \
tar xzvf glpk-4.43.tar.gz
	@echo "Building gsl..."
	cd $(EXTLIBDIR)/gsl-1.14 && \
./configure --prefix=$(ROOTPATH)/extLibs/gsl-1.14 && \
make && \
make install
	cd $(ROOTDIR) && \
ln -s extLibs/gsl-1.14 gsl
	@echo "Building glpk..."
	cd $(EXTLIBDIR)/glpk-4.43 && \
mkdir build && \
./configure --prefix=$(ROOTPATH)/extLibs/glpk-4.43/build && \
make && \
make install
	cd $(ROOTDIR) && \
ln -s extLibs/glpk-4.43 glpk
	@echo "Done building external libraries"
# DO NOT DELETE
