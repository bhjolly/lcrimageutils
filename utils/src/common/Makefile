CFLAGS=-I$(GDAL_INCLUDE_PATH) -fPIC
GDAL=-L$(GDAL_LIB_PATH) $(GDAL_LINK_FLAGS)
OUTNAME=libgdalcmn.so

all: gdalcommon

.c.o:
	$(LCRCC) $(LCRCFLAGS) $(CFLAGS) -c $<
.cpp.o:
	$(LCRCXX) $(LCRCXXFLAGS) $(CFLAGS) -c $<

gdalcommon: gdalcommon.o gdalcommon_cpp.o histogram.o inv_dist_weighting.o glayer.o gutils.o ginfo.h fparse.o
	$(LCRCXX) gdalcommon.o gdalcommon_cpp.o histogram.o inv_dist_weighting.o glayer.o gutils.o fparse.o $(GDAL) $(LCRCXXLDFLAGS) $(GDAL_LINK_FLAGS) -o $(OUTNAME) $(LCRSHARED)

install:
	cp $(OUTNAME) $(GDALHG_LIB_PATH)
	cp gdalcommon.h $(GDALHG_INCLUDE_PATH)
	cp gdalcommon_cpp.h $(GDALHG_INCLUDE_PATH)
	cp histogram.h $(GDALHG_INCLUDE_PATH)
	cp glayer.h $(GDALHG_INCLUDE_PATH)
	cp fparse.h $(GDALHG_INCLUDE_PATH)
	cp ginfo.h $(GDALHG_INCLUDE_PATH)
	cp gutils.h $(GDALHG_INCLUDE_PATH)

clean:
	rm -f *.o  $(OUTNAME)
	
