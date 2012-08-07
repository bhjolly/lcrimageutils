CFLAGS=-I$(GDAL_INCLUDE_PATH) -I$(GDALHG_INCLUDE_PATH) $(LCRCXXFLAGS)
GDAL=-L$(GDAL_LIB_PATH) -lgdal -L$(GDALHG_LIB_PATH) -lgdalcmn
CC=$(LCRCXX)
OUTNAME=gdalregiongrow
OUTLABELLING=testlabelling

all: gdalregiongrow testlabelling

.cpp.o:
	$(CC) $(CFLAGS) -c $<

gdalregiongrow: gdalregiongrow.o Makefile
	$(CC) $(GDAL_LINK_FLAGS) gdalregiongrow.o $(GDAL) $(LCRCXXLDFLAGS) -o $(OUTNAME)

testlabelling: testlabelling.o regionlabelling.o Makefile
	$(CC) $(GDAL_LINK_FLAGS) testlabelling.o regionlabelling.o $(GDAL) $(LCRCXXLDFLAGS) -o $(OUTLABELLING)
	
install:
	cp $(OUTNAME) $(GDALHG_BIN_PATH)

clean:
	rm -f *.o $(OUTNAME) $(OUTLABELLING)
	
