#!/usr/bin/env python

"""
Command line program to call change metdatada in a file

"""
# This file is part of 'gdalutils'
# Copyright (C) 2014 Sam Gillingham
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

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
