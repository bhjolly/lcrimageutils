CFLAGS=$(LCRCXXFLAGS) -I$(GDAL_INCLUDE_PATH) -I$(GDALHG_INCLUDE_PATH)
GDAL=-L$(GDAL_LIB_PATH) -lgdal $(GDAL_LINK_FLAGS) -L$(GDALHG_LIB_PATH) -lgdalcmn
CC=$(LCRCXX)
OUTNAME=gdalcalcstats

all: gdalcalcstats

.c.o:
	$(CC) $(CFLAGS) -c $<


gdalcalcstats: gdalcalcstats.o Makefile
	$(CC) gdalcalcstats.o $(GDAL) $(LCRCXXLDFLAGS) -o $(OUTNAME)

install:
	cp $(OUTNAME) $(GDALHG_BIN_PATH)

clean:
	rm -f *.o $(OUTNAME)
	
