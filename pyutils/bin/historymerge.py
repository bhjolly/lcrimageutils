#!/usr/bin/env python3

"""
Command line program to call meta.insertMetadataFilename.

Sam Gillingham. February 2007.
"""

import sys
import optparse
from lcrimageutils import history

class CmdArgs(object):
  def __init__(self):
    parser = optparse.OptionParser(usage="%prog [options]")
    parser.add_option("-d","--dest", dest="dest", help="Name of file metadata to be written to")
    parser.add_option("-p","--parent", dest="parents", help="Name of parent file - can be specified multiple times for multiple parents",action="append")
    parser.add_option("-a","--argv", dest="argv", help="Command line of script calling this. Should be quoted - ie '$argv'")
    parser.add_option("-s","--script", dest="script", help="Name of script calling this. Should be quoted - ie '$0'")
    parser.add_option("-o","--optional", dest="optionals", help="Any optional metadata for this file in the form: -o tag=value",action="append")
    self.parser = parser
    (options, args) = parser.parse_args()
    # Some magic to copy the particular fields into our final object
    self.__dict__.update(options.__dict__)
 
# Use the class above to create the command args object
cmdargs = CmdArgs()

if not cmdargs.dest or not cmdargs.argv or not cmdargs.script:
  cmdargs.parser.print_help()
  sys.exit()

# populate the optional dictionary
optional_dict = {}
if cmdargs.optionals:
  for opt in cmdargs.optionals:
    (key,value) = opt.split('=')
    optional_dict[key] = value
    
# it's possible there are no parent - import etc.
# insertMetadataFilename does display a warning
if not cmdargs.parents:
  cmdargs.parents = []
  
# call the method in the meta module that does the insert and merge
history.insertMetadataFilename(cmdargs.dest,cmdargs.parents,optional_dict,cmdargs.script,cmdargs.argv)
