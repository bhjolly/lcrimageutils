"""
Module for doing stats on rasters based on data from a vector
"""
import numpy
from rios import applier

def riosStats(info, inputs, output, otherargs):
    """
    Function that gets called from RIOS
    """
    if otherargs.bands is None:
        nbands = inputs.raster.shape[0]
        bands = numpy.arange(nbands) + 1 # 1 based
    else:
        bands = otherargs.bands

    for band in bands:
        data = inputs.raster[band-1].flatten()
        mask = inputs.vector[0].flatten() != 0
        data = data.compress(mask)

        if band in otherargs.data:
            otherargs.data[band] = numpy.append(otherargs.data[band], data)
        else:
            otherargs.data[band] = data

def doStats(vector, raster, sql=None, alltouched=False, bands=None, layer=0):
    """
    Does the stats and returns a dictionary of dictionaries
    one for each band
    """
    infiles = applier.FilenameAssociations()
    infiles.raster = raster
    infiles.vector = vector

    outfiles = applier.FilenameAssociations()
    # empty

    controls = applier.ApplierControls()
    controls.setAlltouched(alltouched)
    controls.setVectorlayer(layer)
    if sql is not None:
        controls.setFilterSQL(sql)

    otherargs = applier.OtherInputs()
    otherargs.data = {} # dictionary, keyed on band
    otherargs.bands = bands

    # do the work
    applier.apply(riosStats, infiles, outfiles, otherargs, controls=controls)

    results = {}
    for band in sorted(otherargs.data.keys()):
        stats = {}
        data = otherargs.data[band]
        if data.size != 0:
            stats['mean'] = data.mean()
            stats['median'] = numpy.median(data)
            stats['std'] = data.std()
            stats['min'] = data.min()
            stats['max'] = data.max()
            stats['count'] = data.size
            results[band] = stats

    return results
