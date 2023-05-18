# -------------------------------------------------------------------
# Below are variables which may need to be modified for your system.
# After they are specified, you should be able to just type 'make' to
# compile btmanip and 'make install' it to a directory on your path.
# -------------------------------------------------------------------

# ----------------------------------------------------------------------
# This section has variables which may need to be modified. The rest
# of the makefile should not need too much editing.
# ----------------------------------------------------------------------

# This variable should include the directories for the O2scl, GSL, and
# HDF5 libraries. By default, this is taken from the enviroment
# variable LDFLAGS.

LIB_DIRS = -L/usr/lib -L/usr/lib/x86_64-linux-gnu \
	-L/usr/lib/x86_64-linux-gnu/hdf5/serial -L/usr/local/lib \
	-lo2scl_hdf -lo2scl_eos -lo2scl_part \
	-lo2scl -lgsl -lgslcblas -lhdf5_hl -lhdf5 -lz -lreadline \
	-lm

# This variable may need to be modified to specify the include
# directories for the GSL, Boost, HDF5, and O2scl header files. By
# default this is taken from the environment variable CXXFLAGS.

INC_DIRS = -I/usr/include -I/usr/local/include \
	-I/usr/lib/x86_64-linux-gnu/hdf5/serial/include

# Generic (no MPI necessary) C++ compiler (e.g. g++)

# CXX = 

COMPILER_FLAGS = -O3 -Wno-unused -DBOOST_PHOENIX_STL_TUPLE_H_

# Location of final executable

BIN_DIR = /usr/local/bin

# ----------------------------------------------------------------------
# UTK makefile
# ----------------------------------------------------------------------

ifdef UTKNA_MAKEFILE

include $(UTKNA_MAKEFILE)

# UTK configuration
LIB_DIRS = $(UTKNA_O2SCL_LIBS) $(UTKNA_PYTHON_LDFLAGS)
INC_DIRS = $(UTKNA_O2SCL_INCS)
CXX = $(UTKNA_CXX) 
COMPILER_FLAGS = $(UTKNA_CFLAGS) -DBOOST_PHOENIX_STL_TUPLE_H_

endif


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
	$(CXX) $(COMPILER_FLAGS) -o btmanip btmanip.o bib_file.o hdf_bibtex.o \
		$(LIB_DIRS)
	@echo "Use 'sudo make install' to install to "
	@echo $(BIN_DIR)

install:
	cp btmanip $(BIN_DIR)

btmanip.o: btmanip.cpp bib_file.h hdf_bibtex.h
	$(CXX) $(COMPILER_FLAGS) $(INC_DIRS) -I. -c -o btmanip.o btmanip.cpp

hdf_bibtex.o: bib_file.h hdf_bibtex.h hdf_bibtex.cpp
	$(CXX) $(COMPILER_FLAGS) $(INC_DIRS) -I. -c -o hdf_bibtex.o hdf_bibtex.cpp

bib_file.o: bib_file.h hdf_bibtex.h bib_file.cpp
	$(CXX) $(COMPILER_FLAGS) $(INC_DIRS) -I. -c -o bib_file.o bib_file.cpp

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

test2:
	./btmanip -bbl-dups ~/wcs/frh/proposal/*.bbl \
		~/wcs/frh/proposal/*.bib

test3:
	./btmanip -bbl-dups ~/wcs/comb_ns/*.bbl \
		~/wcs/comb_ns/*.bib

empty: 

