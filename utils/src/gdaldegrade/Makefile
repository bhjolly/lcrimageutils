CFLAGS=-I$(GDAL_INCLUDE_PATH) -I$(GDALHG_INCLUDE_PATH) $(LCRCFLAGS)
GDAL=-L$(GDAL_LIB_PATH) -lgdal -L$(GDALHG_LIB_PATH) -lgdalcmn
CC=$(LCRCC)
OUTNAME=gdaldegrade

all: gdaldegrade

.c.o:
	$(CC) $(CFLAGS) -c $<

gdaldegrade: gdaldegrade.o Makefile
	$(CC) $(GDAL_LINK_FLAGS) gdaldegrade.o $(GDAL) $(LCRLDFLAGS) -o $(OUTNAME)

install:
	cp $(OUTNAME) $(GDALHG_BIN_PATH)

clean:
	rm -f *.o $(OUTNAME)
	
	
