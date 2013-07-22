#!/usr/bin/env python

import sys
from osgeo import gdal

if len(sys.argv) != 2:
    raise SystemExit("usage: filename")

ds = gdal.Open(sys.argv[1], gdal.GA_Update)
for band in range(ds.RasterCount):
    bh = ds.GetRasterBand(band+1)
    bh.SetMetadataItem('LAYER_TYPE', 'thematic')

del ds


