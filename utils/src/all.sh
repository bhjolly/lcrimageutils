#!/bin/sh

make -C common $*
make -C gdalcalcstats $*
make -C gdalclump $*
make -C gdaldegrade $*
make -C gdaldumpWKT $*
make -C gdalregiongrow $*
make -C ogrdissolve $*
make -C libimgf90 $*
