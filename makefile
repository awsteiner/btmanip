FLAGS = -std=c++0x -ggdb -O3 -Wno-deprecated-declarations \
	-Wno-ignored-attributes

INCS = -I$(EIGEN_INC) -I$(O2SCL_INC) -I$(HDF5_INC) -I$(GSL_INC)

LIBS = -L$(GSL_LIB) -L$(O2SCL_LIB) -L$(HDF5_LIB) \
	-lo2scl_hdf -lo2scl_eos -lo2scl_part \
	-lo2scl -lgsl -lgslcblas -lhdf5_hl -lhdf5 -lz -lreadline \
	-lncurses -lm 

btmanip.o: btmanip.cpp bib_file.h hdf_bibtex.h
	$(CXX) $(FLAGS) $(INCS) -I. -c -o btmanip.o btmanip.cpp

btmanip: btmanip.o
	$(CXX) $(FLAGS) -o btmanip btmanip.o $(LIBS)
	cp btmanip ~/bin

clean:
	rm -f btmanip *.o

# -------------------------------------------------------

doc: empty
	cd doc; doxygen Doxyfile

docp: empty
	cd doc/latex; $(MAKE)

empty: 

