#!/bin/env python

"""
GUI app for displaying metadata tree

Sam Gillingham. February 2007.
"""

import sys
from qt import *
#from rsc.utils import history
from rsc.utils import history
from rsc.utils.history import ProcessingHistory # need to make pickle work TODO: sort this out

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

    # create a listview object (whcih can be a tree)
    self.listview = QListView(self.splitter)
    self.listview.setRootIsDecorated(True)
    self.listview.addColumn("Name")

    # now set up the tree
    self.rootnode = QListViewItem(self.listview,filename)
    self.rootnode.setOpen(True)
    self.rootnode.metanode = (self.histobj.thismeta,filename)
    
    for parent in sorted(self.histobj.directparents.keys()):
      node = QListViewItem(self.rootnode,parent)
      node.setOpen(True)
      node.metanode = (self.histobj.directparents[parent],parent)
      self.alreadydisplayedkeys[parent] = node
      
      self.findParents(node,parent)
    
    # create a text area that can display HTML
    self.textarea = QTextEdit(self.splitter)
    self.textarea.setReadOnly(True)
    
    # set up the events
    QObject.connect(self.listview,SIGNAL("selectionChanged()"),self.selectionChanged)
    
    # set the root element
    self.listview.setSelected(self.rootnode,True)

    # show the app
    self.splitter.setCaption("HistoryView")
    self.setMainWidget(self.splitter)
    self.splitter.show()
    self.exec_loop()

  def findParents(self,node,key):
    """
    Searches for the parents of a given key and adds them to the node
    """
    # search for any parents of 'parent'    
    for (testchild,testparent) in self.histobj.relationships:
      #print testchild,testparent, parent
      if testchild == key:
        if self.alreadydisplayedkeys.has_key(testparent):
          # already seen this one
          parentnode = QListViewItem(node,testparent)
          parentnode.setOpen(True)
          previousnode = self.alreadydisplayedkeys[testparent]
          parentnode.metanode = (previousnode,testparent)
          # add an ellipsis one underneath to show there are more
          ellipsisnode = QListViewItem(parentnode,'...')
          ellipsisnode.metanode = (previousnode,testparent)
        else:
          # ok create a node based on the meta in files
          parentnode = QListViewItem(node,testparent)
          parentnode.setOpen(True)
          parentnode.metanode = (self.histobj.files[testparent],testparent)
          self.alreadydisplayedkeys[testparent] = parentnode # so we can jump to it later
          # ok recurse and see if there are any more parents
          self.findParents(parentnode,testparent)
  
      
  def selectionChanged(self):
    """
    Called when the tree view item changed
    """
    item = self.listview.selectedItem()
    if item is not None:
      (meta,name) = item.metanode
      if isinstance(meta,QListViewItem):
        # ok this means it is a repeat of the existing one - jump to it
        self.listview.setCurrentItem(meta)
      else:
        # hopefully a dictionary - populate page
        text = "<h2>%s</h2>\n" % name
        text = text + "<h4>Metadata:</h4>\n"
        text = text + "<table border cellspacing=0 cellpadding=5><tr><th bgcolor=grey>Field</th><th bgcolor=grey>Value</th></tr>"
        keys = meta.keys()
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
