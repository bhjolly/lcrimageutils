#!/usr/bin/env python3

try:
  from lcrimageutils import historyview4 as historyview
except ImportError:
  from lcrimageutils import historyview3 as historyview

historyview.run()
