# ---------------------------------------------------------------
# Variables which may need to be modified for your system
# ---------------------------------------------------------------

FLAGS = -std=c++0x -O3

ifeq ($(USER),awsteiner)

# The values I typically use

INCS = -I$(EIGEN_INC) -I$(O2SCL_INC) -I$(HDF5_INC) -I$(GSL_INC)

LIBS = -L$(GSL_LIB) -L$(O2SCL_LIB) -L$(HDF5_LIB) \
	-lo2scl_hdf -lo2scl_eos -lo2scl_part \
	-lo2scl -lgsl -lgslcblas -lhdf5_hl -lhdf5 -lz -lreadline \
	-lncurses -lm

else

ifeq ($(TERM_PROGRAM),Apple_Terminal)

# Settings nearly suitable for a typical OSX system

INCS = -I/usr/include -I/usr/local/include

LIBS = -L/usr/lib -L/usr/local/lib \
	-lo2scl_hdf -lo2scl_eos -lo2scl_part \
	-lo2scl -lgsl -lgslcblas -lhdf5_hl -lhdf5 -lz -lreadline \
	-lncurses -lm

else

# Settings nearly suitable for a typical Ubuntu system

INCS = -I/usr/include -I/usr/local/include \
	-I/usr/lib/x86_64-linux-gnu/hdf5/serial/include

LIBS = -L/usr/lib -L/usr/lib/x86_64-linux-gnu \
	-L/usr/lib/x86_64-linux-gnu/hdf5/serial -L/usr/local/lib \
	-lo2scl_hdf -lo2scl_eos -lo2scl_part \
	-lo2scl -lgsl -lgslcblas -lhdf5_hl -lhdf5 -lz -lreadline \
	-lncurses -lm

endif

endif

# You may need to define this for your compiler as well

# CXX = 

# ---------------------------------------------------------------
# Targets (hopefully you shouldn't need to change these)
# ---------------------------------------------------------------

btmanip.o: btmanip.cpp bib_file.h hdf_bibtex.h
	$(CXX) $(FLAGS) $(INCS) -I. -c -o btmanip.o btmanip.cpp

btmanip: btmanip.o
	$(CXX) $(FLAGS) -o btmanip btmanip.o $(LIBS)
	cp btmanip ~/bin

clean:
	rm -f btmanip *.o

doc: empty
	cd doc; doxygen doxyfile
	cd sphinx; make html
	cd ~/wcs/int4/web/utk/btmanip; cp -r html/* .; \
		rm -rf html

empty: 

