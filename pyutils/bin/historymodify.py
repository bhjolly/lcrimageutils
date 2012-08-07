#!/usr/bin/env python3

"""
Command line program to call change metdatada in a file

Sam Gillingham. February 2007.
"""

import sys
import optparse
from lcrimageutils import history
from osgeo import gdal
from osgeo.gdalconst import *

class CmdArgs(object):
  def __init__(self):
    parser = optparse.OptionParser(usage="%prog [options]")
    parser.add_option("-d","--dest", dest="dest", help="Name of file metadata to be written to")
    parser.add_option("-o","--optional", dest="optionals", help="Any optional metadata for this file in the form: -o tag=value",action="append")
    self.parser = parser
    (options, args) = parser.parse_args()
    # Some magic to copy the particular fields into our final object
    self.__dict__.update(options.__dict__)
 
# Use the class above to create the command args object
cmdargs = CmdArgs()

if not cmdargs.dest or not cmdargs.optionals:
  cmdargs.parser.print_help()
  sys.exit()

ds = gdal.Open(cmdargs.dest,GA_Update)
obj = history.readTreeFromDataset(ds)

for opt in cmdargs.optionals:
  (key,value) = opt.split('=')
  obj.thismeta[key] = value

  
band = ds.GetRasterBand(1)
metadata = band.GetMetadata()
  
metadata[history.metadataName] = obj.toString()
band.SetMetadata(metadata)

del ds
