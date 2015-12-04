Collection of utilities from QLD DERM and some from NZ Landcare. Probably should live somewhere more useful, but they are here for now.

# Command line utilities #

* ogrdissolve. Dissolve all input polygons into one.
* clump_lowmem.py. Clump an input file using a windowing algorithm that uses very little memory. See 'clump_lowmem.py -h'
* gdalcalcstats. Calculate statistics and create Pyramid layers for a file. Can also specify the ignore value.
* gdaldumpWKT. Prints the WKT (the coordinate string) to the terminal.
* gdalsetthematic.py. Sets a file as thematic.
* historymerge.py. Merges the history data from parent file(s) into a new output.
* historymodify.py. Updates history info.
* historyview.py. Gives a GUI that allows the history of the given file to be viewed.
* vectorstats.py. Allows stats to be gathered from a raster file for the polygons in a given vector file. See 'vectorstats.py -h'

# Python Modules #

### history ###
Utils for manipulating history data in files.
```
#!python
>>> from lcrimageutils import history
>>> help(history)
```

### mdl ###
General utilities useful from RIOS. Some mimic Imagine functions.
```
#!python
>>> from lcrimageutils import mdl
>>> help(mdl)
```


### vectorstats ###
Utilities for extracting stats from rasters for different areas.
```
#!python
>>> from lcrimageutils import vectorstats
>>> help(vectorstats)
```

### zones ###
Utilities for extracting stats clumped images and extracting stats etc.
```
#!python
>>> from lcrimageutils import zones
>>> help(zones)
```
