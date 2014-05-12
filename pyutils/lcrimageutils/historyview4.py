#!/usr/bin/env python3

"""
GUI app for displaying metadata tree

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

from PyQt4.QtGui import *
from PyQt4.QtCore import *

from lcrimageutils import history
from lcrimageutils.history import ProcessingHistory # need to make pickle work TODO: sort this out

class HistoryViewApp(QApplication):

  def __init__(self):
    QApplication.__init__(self,sys.argv)
    self.alreadydisplayedkeys = {}
    
  def run(self,filename):
    """
    Reads the xml in and constructs the GUI
    """
    
    # read the History
    self.histobj = history.readTreeFromFilename(filename)

    # create the splitter widget
    self.splitter = QSplitter()
    self.splitter.resize(800,600)

    self.treeview = QTreeWidget(self.splitter)
    self.treeview.setRootIsDecorated(True)
    self.treeview.headerItem().setText(0,"Name")

    # now set up the tree
    self.rootnode = QTreeWidgetItem(self.treeview,[filename])
    self.rootnode.setExpanded(True)
    self.rootnode.metanode = (self.histobj.thismeta,filename)
    
    for parent in sorted(self.histobj.directparents.keys()):
      node = QTreeWidgetItem(self.rootnode,[parent])
      node.setExpanded(True)
      node.metanode = (self.histobj.directparents[parent],parent)
      self.alreadydisplayedkeys[parent] = node
      
      self.findParents(node,parent)
    
    # create a text area that can display HTML
    self.textarea = QTextEdit(self.splitter)
    self.textarea.setReadOnly(True)
    
    # set up the events
    QObject.connect(self.treeview,SIGNAL("currentItemChanged(QTreeWidgetItem *,QTreeWidgetItem *)"),self.currentItemChanged)
    
    # set the root element
    self.treeview.setCurrentItem(self.rootnode)

    # show the app
    self.splitter.setWindowTitle("HistoryView")
    self.splitter.show()
    self.exec_()

  def findParents(self,node,key):
    """
    Searches for the parents of a given key and adds them to the node
    """
    # search for any parents of 'parent'    
    for (testchild,testparent) in self.histobj.relationships:
      #print testchild,testparent, parent
      if testchild == key:
        if testparent in self.alreadydisplayedkeys:
          # already seen this one
          parentnode = QTreeWidgetItem(node,[testparent])
          parentnode.setExpanded(True)
          previousnode = self.alreadydisplayedkeys[testparent]
          parentnode.metanode = (previousnode,testparent)
          # add an ellipsis one underneath to show there are more
          ellipsisnode = QTreeWidgetItem(parentnode,['...'])
          ellipsisnode.metanode = (previousnode,testparent)
        else:
          # ok create a node based on the meta in files
          parentnode = QTreeWidgetItem(node,[testparent])
          parentnode.setExpanded(True)
          parentnode.metanode = (self.histobj.files[testparent],testparent)
          self.alreadydisplayedkeys[testparent] = parentnode # so we can jump to it later
          # ok recurse and see if there are any more parents
          self.findParents(parentnode,testparent)
  
      
  def currentItemChanged(self,item,previous):
    """
    Called when the tree view item changed
    """
    if item is not None:
      (meta,name) = item.metanode
      if isinstance(meta,QTreeWidgetItem):
        # ok this means it is a repeat of the existing one - jump to it
        self.treeview.setCurrentItem(meta,0,QItemSelectionModel.ClearAndSelect)
      else:
        # hopefully a dictionary - populate page
        text = "<h2>%s</h2>\n" % name
        text = text + "<h4>Metadata:</h4>\n"
        text = text + "<table border cellspacing=0 cellpadding=5><tr><th bgcolor=grey>Field</th><th bgcolor=grey>Value</th></tr>"
        keys = list(meta.keys())
        keys.sort()
        for key in keys:
          text = text + "<tr><td>%s</td><td>%s</td></tr>" % (key,meta[key])
        text = text + "</table>"
        self.textarea.setText(text)

def run():
  """
  Main part of program - displays window.
  """
  if len(sys.argv) != 2:
    raise SystemExit("usage: historyview.py filename")
  
  filename = sys.argv[1]

  app = HistoryViewApp()
  app.run(filename)

if __name__ == '__main__':
  run()
