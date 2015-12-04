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
Collection of routines that deals with zones (ie clumped images)
```
#!python
>>> from lcrimageutils import zones
>>> help(zones)
```

# Fortran 90 bindings for GDAL #

A subset of the GDAL functions are available from Fortran 90. See the [module](https://bitbucket.org/chchrsc/gdalutils/src/657f6300e9a17f46454ddaf7df2a0e9408dec399/libimgf90/src/libimgf90mod.f90?at=default&fileviewer=file-view-default) source. Also see the [example](https://bitbucket.org/chchrsc/gdalutils/src/7da720c6a690ca36fc15f9a7144052f1980f08cf/libimgf90/test.f90?at=default&fileviewer=file-view-default).

# Dr Shepherd's Imagine toolkit to GDAL compatibility layer #

Code to assist those porting from Imagine C toolkit to GDAL. Look at the [header functions](https://bitbucket.org/chchrsc/gdalutils/src/7da720c6a690ca36fc15f9a7144052f1980f08cf/utils/src/common/?at=default).