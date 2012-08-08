CFLAGS=$(LCRCXXFLAGS) -I$(GDAL_INCLUDE_PATH) -I$(GDALHG_INCLUDE_PATH)
GDAL=-L$(GDAL_LIB_PATH) $(GDAL_LINK_FLAGS) -L$(GDALHG_LIB_PATH) -lgdalcmn
CC=$(LCRCXX)
OUTNAME=gdalclump

all: gdalclump
.SUFFIXES: .cpp

.cpp.o:
	$(CC) $(CFLAGS) -c $<

        
gdalclump: gdalclump.o Makefile
	$(CC) $(GDAL_LINK_FLAGS) gdalclump.o $(GDAL) $(LCRCXXLDFLAGS) -o $(OUTNAME)

install:
	cp $(OUTNAME) $(GDALHG_BIN_PATH)

clean:
	rm -f *.o $(OUTNAME)
	
	
