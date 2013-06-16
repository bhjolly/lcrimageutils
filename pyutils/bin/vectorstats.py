#!/usr/bin/env python

import optparse
from lcrimageutils import vectorstats

class CmdArgs(object):
    """
    Class for processing command line arguments
    """
    def __init__(self):
        usage = "usage: %prog [options]"
        self.parser = optparse.OptionParser(usage)

        self.parser.add_option('-v', '--vector', dest='vector',
                    help="path to vector file")
        self.parser.add_option('-r', '--raster', dest='raster',
                    help="path to raster file")
        self.parser.add_option('-i', '--ignore', dest='ignore',
                    help="either: 'int' to use internal nodata values\n"+
                        "or 'none' to use all values, or a single value"+
                        " to use for all bands, or a comma seperated list"+
                        " of values - one per band.")
        self.parser.add_option('-l', '--layer', dest='layer',
                    default=0, type='int', 
                    help="Name or 0-based index of vector layer. Defaults"+
                        " to first layer")
        self.parser.add_option('-s', '--sql', dest='sql',
                    help="SQL to filter the features in the vector file")
        self.parser.add_option('-a', '--alltouched', action="store_true",
                    default=False, dest="alltouched", 
                    help="If set, all pixels touched are included in vector"+
                        " the default is that only pixels whose centres are"+
                        " in the polygon are included")
        self.parser.add_option('-b', '--bands', dest='bands',
                    help="comma separated list of 1-based bands to use. "+
                        "Default is all bands")

        (options, self.args) = self.parser.parse_args()
        self.__dict__.update(options.__dict__)


if __name__ == '__main__':
    cmdargs = CmdArgs()

    if (cmdargs.vector is None or cmdargs.raster is None or 
            cmdargs.ignore is None):
        raise SystemExit('Must specify raster, vector and ignore at least')

    bands = None
    if cmdargs.bands is not None:
        # convert to list of ints
        bands = cmdargs.bands.split(',')
        bands = [int(x) for x in bands]

    ignore_values = None
    if cmdargs.ignore == 'int':
        ignore_behaviour = vectorstats.IGNORE_INTERNAL
    elif cmdargs.ignore == 'none':
        ignore_behaviour = vectorstats.IGNORE_NONE
    else:
        ignore_behaviour = vectorstats.IGNORE_VALUES
        ignore_values = cmdargs.ignore.split(',')
        ignore_values = [float(x) for x in ignore_values]
        if len(ignore_values) == 1:
            ignore_values = ignore_values[0]

    results = vectorstats.doStats(cmdargs.vector, cmdargs.raster, 
                    ignore_behaviour, ignore_values, cmdargs.sql, 
                    cmdargs.alltouched, bands, cmdargs.layer)

    for band in sorted(results.keys()):
        print('%d %f %f %f %f %f %d' % (band, results[band]['mean'],
            results[band]['median'],  results[band]['std'],
             results[band]['min'],  results[band]['max'], 
                results[band]['count']))
