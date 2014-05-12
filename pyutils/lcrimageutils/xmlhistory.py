#!/bin/env python

"""
Module for reading and manipulating SLATS metadata 'trees'.
Example XML format in meta.xml

Sam Gillingham. February 2007.

Now called history.py, and modified to work on XML files as well as GDAL files. 
Neil Flood. August 2008.
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

import os
import xml.sax.handler
import xml.sax
import xml.dom.minidom
from osgeo import gdal
from osgeo.gdalconst import *

metadataName = 'SLATS_Metadata'
metadataTag = 'SLATS_Metadata_Document'

class Node:
  """
  One 'node' of a tree. Has a list of parent's names and a dictionary of metadata
  """
  def __init__(self,name):
    self.parents = []
    self.meta = {}
    self.name = name
    
  def addParent(self,parent):
    """
    Adds a parent to the list of parents
    """
    if parent.name == self.name:
      # node's parent cannot be itself
      raise ValueError("Invalid Node")
    self.parents.append(parent)
    
  def addMeta(self,name,data):
    """
    Adds an item to the metadata dictionary
    """
    self.meta[name] = data
    
  def equal(self,node):
    """
    Tests for equality between this node and the parameter node
    """
    return self.name == node.name and self.parents == node.parents and self.meta == node.meta
    
class Tree:
  """
  Class that handles a tree of Nodes. 
  """
  def __init__(self,head):
    self.head = head
    
  def traverseTree(self,callback,node = None):
    """
    Traverses tree, calling callback.processNode() with each node in the tree.
    Set node to None to start from the head
    """
    if node is None:
      node = self.head
    callback.processNode(node)
    for parent in node.parents:
      self.traverseTree(callback,parent)
      
  @staticmethod
  def mergeTrees(head,treelist):
    """
    Takes a new head and creates a new tree with the parents of the head
    being the trees in treelist
    """
    for tree in treelist:
      head.addParent(tree.head)
    return Tree(head)
    
  def toXML(self,doc,parent=None,node = None):
    """
    Returns the tree converted back into XML.
    """
    if node is None:
      # start of the tree
      node = self.head
    if parent is None:
      # haven't created the head node yet
      parent = doc.createElement(metadataTag)
      if doc.firstChild != None:
        # In this case, we are probably in an XML document, and just adding to its root tag
        
        # First we need to remove existing instances of this tag name
        existingElements = doc.getElementsByTagName(metadataTag)
        for el in existingElements:
            doc.childNodes[0].removeChild(el)
        
        # Now add the new one.
        doc.firstChild.appendChild(parent)
      else:
        # Here we are probably adding to a GDAL file, in which case there is not already a root XML tag
        doc.appendChild(parent)
      
    # open the node
    el = doc.createElement('node')
    el.setAttribute('name',node.name)
    parent.appendChild(el)
    # process the parents
    for parent in node.parents:
      self.toXML(doc,el,parent)
    # write this nodes metadata
    for key in node.meta.keys():
      subel = doc.createElement('info')
      subel.setAttribute('name',key)
      data = doc.createTextNode(str(node.meta[key]))
      subel.appendChild(data)
      el.appendChild(subel)
      
  def toXMLString(self):
    doc = xml.dom.minidom.Document()
    self.toXML(doc)
    return doc.toprettyxml(indent='')
    
  @staticmethod
  def fromXMLString(string):
    """
    Creates an instance of Tree (the metadata tree) from a string. 
    """
    handler = TreeBuilder()
    xml.sax.parseString(string,handler)
    return handler.tree
    
      
class TreeBuilder(xml.sax.handler.ContentHandler):
  """
  Class for parsing the SLATS metadata XML. 
  Ignores anything outside the <SLATS_Metadata_Document> tags.
  """
  def __init__(self):
    self.currnodes = []
    self.indoc = False
     
  def startElement(self, name, attributes):
    if name == metadataTag:
      # we are ready to start reading in the tree
      self.indoc = True
    elif self.indoc and name == "node":
      # have the start of a node. Add it to the list
      name = attributes.getValue("name")
      newnode = Node(name)
      if len(self.currnodes) > 0:
        # the new node will be a parent of the current one
        self.currnodes[-1].addParent(newnode)
      self.currnodes.append(newnode)
    elif self.indoc and name == "info":
      self.infoname = attributes.getValue("name")
    self.data = ''
 
  def characters(self, data):
    self.data = self.data + data
    
  def endElement(self, name):
    if name == metadataTag:
      self.indoc = False
    elif self.indoc and name == "node":
      if len(self.currnodes) == 1:
        # last one - create the tree class
        self.tree = Tree(self.currnodes[0])
      self.currnodes.pop()
    elif self.indoc and name == "info":
      self.currnodes[-1].addMeta(self.infoname,self.data)

#        
# Module functions
#

def readTreeFromDataset(dataset):
  """
  Reads the metadata XML out of an GDAL dataset and returns a Tree instance
  """
  band = dataset.GetRasterBand(1)
  meta = band.GetMetadata()
  if meta.has_key(metadataName):
    tree = Tree.fromXMLString(meta[metadataName])
  else:
    # no metadata in this file - manufacture a single node tree
    #print "warning: %s has no metadata" % dataset.GetDescription()
    name = os.path.basename(dataset.GetDescription())
    node = Node(name)
    tree = Tree(node)
  return tree
  
def readTreeFromFilename(imgfile):
  """
  Same as readTreeFromDataset() but takes a filename
  """
  
  if len(imgfile) > 8 and imgfile[-8:] == ".tseries":
    # a .tseries file
    # should handle this properly and pull all the metadata out of that
    # just create a simple node for now
    name = os.path.basename(imgfile)
    node = Node(name)
    tree = Tree(node)
  elif fileIsXML(imgfile):
    dom = xml.dom.minidom.parse(imgfile)
    metadataElementList = dom.getElementsByTagName(metadataTag)
    if len(metadataElementList) > 0:
        element = metadataElementList[0]
        tree = Tree.fromXMLString(element.toxml())
    else:
        # No metadata tag present, make an empty tree
        name = os.path.basename(imgfile)
        node = Node(name)
        tree = Tree(node)
  else:
    # must be a .img file - get GDAL to do the work
    ds = gdal.Open(imgfile,GA_ReadOnly)
    tree = readTreeFromDataset(ds)
    del ds
  return tree


def addMandatoryFields(node, script, argv):
  """
  Add the mandatory fields to the given metadata node
  If script is None, works it out from sys.argv[0]. 
  Similarly for argv, to get the commandline arguments. 
  """
  import sys
  import time
  import lcl
  
  node.addMeta('timestamp', time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()))
  login = os.getenv('LOGNAME','unknown') # see http://docs.python.org/lib/os-procinfo.html#l2h-2578
  if login == 'unknown':
    login = os.getenv('USER','unknown')
  node.addMeta('login', login )
  
  uname = os.uname()
  node.addMeta('uname_os', uname[0] )
  node.addMeta('uname_host', uname[1] )
  node.addMeta('uname_release', uname[2] )
  node.addMeta('uname_version', uname[3] )
  node.addMeta('uname_machine', uname[4] )
  node.addMeta('cwd', os.getcwd())
  
  # get the SVN revision number
  info = lcl.bt('svn info $SVN_CINRS_REPO | grep Revision')
  svn_rev = int(info[1])
  node.addMeta('subversion_revision', svn_rev )
  
  if argv is None:
    argv = ' '.join(sys.argv[1:])
  node.addMeta('commandline', argv)
  
  if script is None:
    script = sys.argv[0]
  node.addMeta('script', os.path.basename(script))
  node.addMeta('script_dir', os.path.dirname(script))


def createListOfParentTrees(parent_list):
  """
  Creates a list of trees of all the metadata nodes from all the parents.
  """
  treelist = []
  for parent in parent_list:
    treelist.append(readTreeFromFilename(parent))
  return treelist
  
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
  
  treelist = createListOfParentTrees(parent_list)
    
  if len(parent_list) == 0:
    print("Warning: file has no parents - is this correct?")

  name = os.path.basename(dataset.GetDescription())
  node = Node(name)
  
  # mandatory fields
  addMandatoryFields(node, script, argv)
    
  # optional fields
  for key in optional_dict.keys():
    node.addMeta(key,optional_dict[key])
    
  newtree = Tree.mergeTrees(node,treelist)
  
  band = dataset.GetRasterBand(1)
  meta = band.GetMetadata()
  if meta.has_key(metadataName):
    print("Warning: overwriting existing metadata")
  
  doc = xml.dom.minidom.Document()
  newtree.toXML(doc)
  xmlStr = doc.toprettyxml(indent='')
  meta[metadataName] = xmlStr
  band.SetMetadata(meta)
  
  
def insertMetadataFilename(imgfile,parent_list,optional_dict,script=None,argv=None):
  """
  Same as insertMetadataDataset but takes a filename rather than a dataset
  Also adapted to work if the file is an XML file, by detecting this and calling the right routine.
  """
  if fileIsXML(imgfile):
    insertMetadataXMLfile(imgfile,parent_list,optional_dict,script,argv)
  else:
    ds = gdal.Open(imgfile,GA_Update)
    insertMetadataDataset(ds,parent_list,optional_dict,script,argv)
    del ds


def fileIsXML(filename):
  """
  Check the beginning of a file and determine whether it appears to be an XML file or not. 
  """
  magicNumberString = open(filename, 'r').read(5)
  return (magicNumberString == "<?xml")


def insertMetadataXMLdoc(doc, filename, parent_list, optional_dict, script=None, argv=None):
  """
  Insert the file history metadata into an XML document, already composed in memory. 
  If script is None, works it out from sys.argv[0]. 
  Similarly for argv, to get the commandline arguments. 
  """
  treelist = createListOfParentTrees(parent_list)
  
  node = Node(os.path.basename(filename))
  
  addMandatoryFields(node, script, argv)
  
  for key in optional_dict.keys():
    node.addMeta(key, optional_dict[key])
  
  newtree = Tree.mergeTrees(node, treelist)
  newtree.toXML(doc)


def insertMetadataXMLfile(filename, parent_list, optional_dict, script=None, argv=None):
  """
  Same as insertMetadataXMLdoc, but working on an existing file. Opens the file, reads it, adds the
  metadata, and re-writes the file. 
  """
  import xml.dom.minidom
  doc = xml.dom.minidom.parse(filename)
  insertMetadataXMLdoc(doc, filename, parent_list, optional_dict, script, argv)
  xml = doc.toprettyxml(indent=' ')
  f = open(filename, 'w')
  f.write(xml)
  f.close()




#    
# Test code
#

if __name__ =='__main__':
  import sys

  string = open(sys.argv[1]).read()
  mytree = Tree.fromXMLString(string)
  
  class DumpTree:
    def processNode(self,node):
      print(node.name)

  t = DumpTree()

  mytree.traverseTree(t)
  
  print(mytree.toXMLString())
