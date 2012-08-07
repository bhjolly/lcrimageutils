CFLAGS=-I$(GDAL_INCLUDE_PATH) $(LCRCFLAGS)
LDFLAGS=-L$(GDAL_LIB_PATH) -lgdal -lgeos -lgeos_c
CC=$(LCRCC)
OUTNAME=ogrdissolve

all: ogrdissolve

.c.o:
	$(CC) $(CFLAGS) -c $<

ogrdissolve: ogrdissolve.o  Makefile
	$(CC) $(GDAL_LINK_FLAGS) ogrdissolve.o $(LCRLDFLAGS) $(LDFLAGS) -o ogrdissolve

install:
	cp $(OUTNAME) $(GDALHG_BIN_PATH)

clean:
	rm -f *.o $(OUTNAME)
  
  

