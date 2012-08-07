CFLAGS=-I$(GDAL_INCLUDE_PATH) -I$(GDALHG_INCLUDE_PATH) $(LCRCFLAGS)
GDAL=-L$(GDAL_LIB_PATH) -lgdal -L$(GDALHG_LIB_PATH) -lgdalcmn
CC=$(LCRCC)
OUTNAME=gdaldumpWKT

all: gdaldumpWKT

.c.o:
	$(CC) $(CFLAGS) -c $<

gdaldumpWKT: gdaldumpWKT.o Makefile
	$(CC) $(GDAL_LINK_FLAGS) gdaldumpWKT.o $(GDAL) $(LCRLDFLAGS) -o $(OUTNAME)

install:
	cp $(OUTNAME) $(GDALHG_BIN_PATH)

clean:
	rm -f *.o $(OUTNAME)
	
