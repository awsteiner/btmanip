# -------------------------------------------------------------------
# Below are variables which may need to be modified for your system.
# After they are specified, you should be able to just type 'make' to
# compile btmanip and 'make install' it to a directory on your path.
# -------------------------------------------------------------------

FLAGS = -std=c++0x -O3

ifneq ($(USER),awsteiner)

ifeq ($(TERM_PROGRAM),Apple_Terminal)

# Settings nearly suitable for a typical OSX system

INCS = -I/usr/include -I/usr/local/include

LIBS = -L/usr/lib -L/usr/local/lib \
	-lo2scl_hdf -lo2scl_eos -lo2scl_part \
	-lo2scl -lgsl -lgslcblas -lhdf5_hl -lhdf5 -lz -lreadline \
	-lncurses -lm

BIN_DIR = /usr/local/bin

else

# Settings nearly suitable for a typical Ubuntu system

INCS = -I/usr/include -I/usr/local/include \
	-I/usr/lib/x86_64-linux-gnu/hdf5/serial/include

LIBS = -L/usr/lib -L/usr/lib/x86_64-linux-gnu \
	-L/usr/lib/x86_64-linux-gnu/hdf5/serial -L/usr/local/lib \
	-lo2scl_hdf -lo2scl_eos -lo2scl_part \
	-lo2scl -lgsl -lgslcblas -lhdf5_hl -lhdf5 -lz -lreadline \
	-lncurses -lm

BIN_DIR = /usr/local/bin

endif

else

# The values I typically use

INCS = -I$(EIGEN_INC) -I$(O2SCL_INC) -I$(HDF5_INC) -I$(GSL_INC) \
	-I$(BOOST_INC)

LIBS = -L$(GSL_LIB) -L$(O2SCL_LIB) -L$(HDF5_LIB) -L$(BOOST_LIB) \
	-lo2scl_hdf -lo2scl_eos -lo2scl_part \
	-lo2scl -lgsl -lgslcblas -lhdf5_hl -lhdf5 -lz -lreadline \
	-lncurses -lm

BIN_DIR = /usr/local/bin

endif

# You may need to define this for your compiler as well

# CXX = 

# ---------------------------------------------------------------
# Targets (hopefully you shouldn't need to change these)
# ---------------------------------------------------------------

help:
	@echo "btmanip: "
	@echo "install: "
	@echo "clean: "
	@echo "doc: "
	@echo "sync-doc: "
	@echo "test-sync: "

btmanip: btmanip.o bib_file.o hdf_bibtex.o
	$(CXX) $(FLAGS) -o btmanip btmanip.o bib_file.o hdf_bibtex.o \
		$(LIBS)
	@echo "Use 'sudo make install' to install to "
	@echo $(BIN_DIR)

install:
	cp btmanip $(BIN_DIR)

btmanip.o: btmanip.cpp bib_file.h hdf_bibtex.h
	$(CXX) $(FLAGS) $(INCS) -I. -c -o btmanip.o btmanip.cpp

hdf_bibtex.o: bib_file.h hdf_bibtex.h hdf_bibtex.cpp
	$(CXX) $(FLAGS) $(INCS) -I. -c -o hdf_bibtex.o hdf_bibtex.cpp

bib_file.o: bib_file.h hdf_bibtex.h bib_file.cpp
	$(CXX) $(FLAGS) $(INCS) -I. -c -o bib_file.o bib_file.cpp

clean:
	rm -f btmanip *.o

doc: empty
	cd doc; doxygen doxyfile
	cd sphinx; make html

sync-doc:
	rsync -Cavzu sphinx/html/* $(STATIC_DOC_DIR)/btmanip

test-sync:
	rsync -Cavzun sphinx/html/* $(STATIC_DOC_DIR)/btmanip

test:
	./btmanip -parse ~/wcs/int6/django/svn/all.bib \
		-add ~/wcs/group/mamun/paper/paper.bib

empty: 

