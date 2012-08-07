#!/usr/bin/env python3

"""
Module for reading and writing ProcessingHistory objects from images
"""

import os
import sys
import json
import time
from osgeo import gdal
from osgeo.gdalconst import *
from . import xmlhistory

metadataName = 'LCR_ProcessingHistory'

class ProcessingHistory:
  """
  Class that stores the 'processing history' information for a file.
  This is the 'new' implementation that stores the information for the
  files and the file relationships seperately.
  """
  def __init__(self):
    self.files = {}
    # a dictionary. Key is created by createKey()
    # Stored against each key is the 'meta' dictionary
    # with all the info for the node.
    
    self.thismeta = {}
    # The 'meta' dictionary for 'this' file
    
    self.relationships = []
    # A list of tuples describing the relationship.
    # First element in the tuple is key of child,
    # second element is key of parent.
    # NOTE: this excludes the direct parents which 
    # are stored seperately below.
    
    self.directparents = {}
    # a dictionary. Key is created by createKey()
    # Stored against each key is the 'meta' dictionary
    # with all the info for the node OF THE DIRECT PARENTS
    
  def dump(self):
    """
    Prints the contents of this object out.
    """
    print('thismeta:', self.thismeta)
    print('-------------------------------------')
    print('direct parents:')
    for key in self.directparents.keys():
      print(key, self.directparents[key])
    print('-------------------------------------')
    print('Files:')
    for key in self.files.keys():
      print(key, self.files[key])
    print('-------------------------------------')
    print('Relationships:')
    for (child,parent) in self.relationships:
      print(child, parent)
    print('-------------------------------------')
    #print 'toString:'
    #print self.toString()
        
  @staticmethod
  def fromXMLTree(tree):
    """
    Turns an old style XML tree into a ProcessingHistory 
    object and returns it
    """
    obj = ProcessingHistory()
    
    # meta for the head
    obj.thismeta = tree.head.meta
    
    # first add the direct parents
    for parent in tree.head.parents:
      timestamp = None
      if 'timestamp' in parent.meta:
        timestamp = parent.meta['timestamp']
      key = obj.createKey(parent.name,timestamp)
      obj.addDirectParent(key,parent.meta)
      # and the metadata for all their parents to the other lists
      tree.traverseTree(obj,parent)
      
    return obj
    
  def addFile(self,key,meta):
    """
    Add a file and its metadata if not already recorded.
    Returns True if new file
    NOTE: should not be a direct parent - use addDirectParent instead
    """
    added = False
    if key not in self.files.keys():
      self.files[key] = meta
      added = True
    return added
    
  def addRelationship(self,newchildkey,newparentkey):
    """
    Add a relationship between a child file and parent file if not already recorded.
    Returns True if a new relationship.
    NOTE: should not be a direct parent - use addDirectParent instead
    """
    found = (newchildkey, newparentkey) in self.relationships
    if not found:
      relationship = (newchildkey,newparentkey)
      self.relationships.append(relationship)
    return not found
    
  def addDirectParent(self,key,meta):
    """
    Adds metadata to the list of direct parents
    """
    self.directparents[key] = meta
    
  @staticmethod
  def createKey(name,timestamp=None):
    """
    Creates a key given a filename and a timestamp.
    If timestamp not specified the current time is used.
    """
    if timestamp is None:
      timestamp = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime())
    return '%s %s' % (name.strip('\r\n'),timestamp.strip('\r\n'))
    
  def mergeHistory(self,key,history):
    """
    Takes another processing history obj and merges it in.
    It is assumed that the current object is the current file
    the has self.thismeta set and the object to be merged in 
    is one of the direct parents.
    """
    # add all the non-direct files
    for keyh in history.files.keys():
      self.addFile(keyh,history.files[keyh])
    for keyh in history.directparents.keys():
      self.addFile(keyh,history.directparents[keyh])
    for (child,parent) in history.relationships:
      self.addRelationship(child,parent)
     
    self.addDirectParent(key,history.thismeta)
    # add it's direct relationships as parents
    for parent in history.directparents:
      self.addRelationship(key,parent)

  def processNode(self,node):
    """
    Process a node from an old xml style history file
    (using xmlhistory.Tree.traverseTree)
    """
    # do we have this node?
    timestamp = None
    if 'timestamp' in node.meta:
      timestamp = node.meta['timestamp']
    key = self.createKey(node.name,timestamp)
    self.addFile(key,node.meta)
    for parent in node.parents:
      timestamp = None
      if 'timestamp' in parent.meta:
        timestamp = parent.meta['timestamp']
      parentkey = self.createKey(parent.name,timestamp)
      self.addRelationship(key,parentkey)
      
  def toString(self):
    """
    Returns this instance as a ASCII string that can 
    be stored and recreated as an object using fromString()
    """
    rep = {'files':self.files,'thismeta':self.thismeta,'relationships':self.relationships,'directparents':self.directparents}
    return json.dumps(rep)
    
  @staticmethod
  def fromString(s):
    """
    Creates and returns a new ProcessingHistory object from
    a string previously returned by toString()
    """
    rep = json.loads(s)
    obj = ProcessingHistory()
    obj.files = rep['files']
    obj.thismeta = rep['thismeta']
    obj.relationships = rep['relationships']
    obj.directparents = rep['directparents']
    return obj

#        
# Module functions
#

def readTreeFromDataset(dataset):
  """
  Reads the processing history out of an GDAL dataset and returns it
  """
  band = dataset.GetRasterBand(1)
  meta = band.GetMetadata()
  if metadataName in meta:
    # file has they processinghisory stored as pickled object
    s = meta[metadataName]
    obj = ProcessingHistory.fromString(s)
  elif xmlhistory.metadataName in meta:
    # this should convert from an xml tree to a ProcessingHistory
    tree = xmlhistory.readTreeFromDataset(dataset)
    obj = ProcessingHistory.fromXMLTree(tree)
  else:
    # no metadata in this file - manufacture an empty object
    #print "warning: %s has no metadata" % dataset.GetDescription()
    obj = ProcessingHistory()
    
  return obj
  
def readTreeFromFilename(imgfile):
  """
  Same as readTreeFromDataset() but takes a filename
  """
  
  if len(imgfile) > 8 and imgfile[-8:] == ".tseries":
    # a .tseries file
    # should handle this properly and pull all the metadata out of that
    # just create a simple node for now
    obj = ProcessingHistory() 
  elif fileIsXML(imgfile):
    import xml.dom.minidom
    dom = xml.dom.minidom.parse(imgfile)
    metadataElementList = dom.getElementsByTagName(xmlhistory.metadataTag)
    if len(metadataElementList) > 0:
      element = metadataElementList[0]
      tree = xmlhistory.Tree.fromXMLString(element.toxml())
      obj = ProcessingHistory.fromXMLTree(tree)
    else:
      # Look under the new name
      metadataElementList = dom.getElementsByTagName(metadataName)
      if len(metadataElementList) > 0:
        element = metadataElementList[0]
        pickled = str(element.firstChild.data).strip()
        obj = ProcessingHistory.fromString(pickled)
      else:
        # No metadata tag present, make an empty tree
        obj = ProcessingHistory()
  else:
    # must be a .img file - get GDAL to do the work
    ds = gdal.Open(str(imgfile), GA_ReadOnly)
    obj = readTreeFromDataset(ds)
    del ds

  return obj


def getMandatoryFields(script, argv):
  """
  Get the mandatory fields and return as a dictionary
  If script is None, works it out from sys.argv[0]. 
  Similarly for argv, to get the commandline arguments. 
  """
  import sys
  import subprocess
  
  dictn = {}
  
  dictn['timestamp'] = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime())
  login = os.getenv('LOGNAME','unknown') # see http://docs.python.org/lib/os-procinfo.html#l2h-2578
  if login == 'unknown':
    login = os.getenv('USER','unknown')
  dictn['login'] = login
  
  uname = os.uname()
  dictn['uname_os'] = uname[0]
  dictn['uname_host'] = uname[1]
  dictn['uname_release'] = uname[2]
  dictn['uname_version'] = uname[3]
  dictn['uname_machine'] = uname[4]
  dictn['cwd'] = os.getcwd()
  
  if argv is None:
    argv = ' '.join(sys.argv[1:])
  dictn['commandline'] = argv
  
  if script is None:
    script = sys.argv[0]
  dictn['script'] = os.path.basename(script)
  dictn['script_dir'] = os.path.dirname(script)

  return dictn


def insertHistory(name,parent_list,optional_dict,script=None,argv=None):
  """
  Creates a new node with mandatory metadata
  and any optional metadata passed in optional_dict. It merges all of the metadata from the parent filenames
  passed in parent_list.
  If this is being called from a Python script leave script None it will read it from the 
  current environment. If calling from a C-Shell script pass in $0
  If this is being called from a Python script leave argv None it will read it from the 
  current environment. If calling from a C-Shell script pass in $argv
  """
  
  if len(parent_list) == 0:
    print("Warning: file has no parents - is this correct?")

  obj = ProcessingHistory()
  obj.thismeta = getMandatoryFields(script, argv)
  # optional fields
  for key in optional_dict.keys():
    obj.thismeta[key] = optional_dict[key]
  
  for parent in parent_list:
    parentobj = readTreeFromFilename(parent)
    parentTimestamp = None
    if 'timestamp' in parentobj.thismeta:
        parentTimestamp = parentobj.thismeta['timestamp']
    key = obj.createKey(parent, parentTimestamp)
    obj.mergeHistory(key,parentobj)
    
  return obj

def insertMetadataDataset(dataset,parent_list,optional_dict,script=None,argv=None):
  """
  Takes a dataset (opened with GA_Update, or Create()) and creates a new node with mandatory metadata
  and any optional metadata passed in optional_dict. It merges all of the metadata from the parent filenames
  passed in parent_list.
  If this is being called from a Python script leave script None it will read it from the 
  current environment. If calling from a C-Shell script pass in $0
  If this is being called from a Python script leave argv None it will read it from the 
  current environment. If calling from a C-Shell script pass in $argv
  """
  
  name = os.path.basename(dataset.GetDescription())
    
  obj = insertHistory(name,parent_list,optional_dict,script,argv)
  setProcessingHistoryDataset(dataset,obj)

  
def setProcessingHistoryDataset(dataset,obj):
  band = dataset.GetRasterBand(1)
  meta = band.GetMetadata()
  if metadataName in meta:
    print("Warning: overwriting existing metadata")
  
  meta[metadataName] = obj.toString()
  band.SetMetadata(meta)
  
  
  
def insertMetadataFilename(imgfile,parent_list,optional_dict,script=None,argv=None):
  """
  Same as insertMetadataDataset but takes a filename rather than a dataset
  Also adapted to work if the file is an XML file, by detecting this and calling the right routine.
  """
  if fileIsXML(imgfile):
    insertMetadataXMLfile(imgfile,parent_list,optional_dict,script,argv)
  else:
    ds = gdal.Open(str(imgfile), GA_Update)
    insertMetadataDataset(ds,parent_list,optional_dict,script,argv)
    del ds


def fileIsXML(filename):
  """
  Check the beginning of a file and determine whether it appears to be an XML file or not. 
  """
  magicNumberString = open(filename, 'rb').read(5)
  return (magicNumberString == "<?xml")

def insertMetadataXMLfile(filename, parent_list, optional_dict, script=None, argv=None):
  """
  Opens the file, reads it, adds the metadata, and re-writes the file. 
  """

  obj = insertHistory(filename,parent_list,optional_dict,script=None,argv=None)

  import xml.dom.minidom
  doc = xml.dom.minidom.parse(filename) # read existing xml (not history)
  
  parent = doc.createElement(metadataName)
  doc.firstChild.appendChild(parent)
  data = doc.createTextNode(obj.toString())
  parent.appendChild(data)
   
  xml = doc.toprettyxml(indent=' ')
  f = open(filename, 'w')
  f.write(xml)
  f.close()




#    
# Test code
#

if __name__ =='__main__':
  import sys
  
  infile1 = sys.argv[1]
  infile2 = sys.argv[2]
  outfile = sys.argv[3]
  
  insertMetadataFilename(outfile,[infile1,infile2],{})  
  
