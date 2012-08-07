CFLAGS=-I$(GDAL_INCLUDE_PATH) -I$(GDALHG_INCLUDE_PATH) $(LCRCFLAGS) -fPIC
FFLAGS=$(LCRFFLAGS)
GDAL=-L$(GDAL_LIB_PATH) -lgdal -L$(GDALHG_LIB_PATH) -lgdalcmn
CC=$(LCRCC)
FC=$(LCRFC)
OUTNAME=libimgf90.so
TESTNAME=test

.SUFFIXES: .c .f90 .o

all: libimgf90 test

.c.o:
	$(CC) $(CFLAGS) -c $<
.f90.o:
	$(FC) $(FFLAGS) -c $<

libimgf90: libimgf90.o libimgf90mod.o Makefile
	$(CC) $(GDAL_LINK_FLAGS) libimgf90.o $(GDAL) $(LCRFLDFLAGS) -o $(OUTNAME) $(LCRSHARED)

test: test.o Makefile 
	$(FC) -o $(TESTNAME) $(LCRLDFLAGS) test.o $(OUTNAME) $(LCRFLDFLAGS)
        
test.o: test.f90 libimgf90mod.o

install:
	cp *.mod $(GDALHG_MOD_PATH)
	cp $(OUTNAME) $(GDALHG_LIB_PATH)
        
clean:
	rm -f *.o $(OUTNAME) *.mod $(TESTNAME)
	
