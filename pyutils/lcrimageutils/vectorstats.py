"""
Module for doing stats on rasters based on data from a vector
"""
import numpy
from rios import applier

IGNORE_NONE = 0     # use all values in the raster
IGNORE_INTERNAL = 1 # use internal value (if any)
IGNORE_VALUES = 2   # value(s) will be specified

def riosStats(info, inputs, output, otherargs):
    """
    Function that gets called from RIOS
    """
    if otherargs.bands is None:
        nbands = inputs.raster.shape[0]
        bands = numpy.arange(nbands) + 1 # 1 based
    else:
        bands = otherargs.bands

    bandIdx = 0 # for getting nodata values
    for band in bands:
        data = inputs.raster[band-1].flatten()
        mask = inputs.vector[0].flatten() != 0
        if otherargs.ignore_behaviour == IGNORE_INTERNAL:
            # we need to do more processing on 'mask'
            nodata = info.getNoDataValueFor(inputs.raster, int(band))
            if nodata is not None:
                mask = mask & (data != nodata)
        elif otherargs.ignore_behaviour == IGNORE_VALUES:
            try:
                nodata = otherargs.ignore_values[bandIdx]
            except TypeError:
                # not a sequence
                nodata = otherargs.ignore_values
            mask = mask & (data != nodata)

        # mask the data
        data = data.compress(mask)

        if band in otherargs.data:
            otherargs.data[band] = numpy.append(otherargs.data[band], data)
        else:
            otherargs.data[band] = data

        bandIdx += 1

def doStats(vector, raster, ignore_behaviour, ignore_values=None, 
                sql=None, alltouched=False, bands=None, layer=0):
    """
    Does the stats and returns a dictionary of dictionaries
    one for each band - keyed on the band index.
    Each dictionay for each band has values on the following keys:
        'mean', 'median', 'std', 'min', 'max', 'count'
    If no values are found for a given band there won't be a key for it.

    vector is the filename for the OGR supported vector source
    raster is the filename for the GDAL supported raster source
    ignore_behaviour should be one of:
        IGNORE_NONE - to use all values in the raster
        IGNORE_INTERNAL - to use internal value(s) (if any)
        IGNORE_VALUES - value(s) will be specified on ignore_values
    ignore_values needs to be specified with IGNORE_VALUES. Can be a single
        value to ignore the same on all bands or a list of values, one for
        each band
    sql can be a fragement of the SQL WHERE statement that restricts the 
        vectors that are processed
    alltouched - by default only pixels whose centres are completely within
        a polygon are processed. Setting this to True makes all pixels that
        are touched by the polygon get processed.
    bands - by default all bands are processed. This can be a list of bands
        to process.
    layer - by default the first layer in the vector is used. This can be
        set to either a number of a name of the vector layer to use.
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
    otherargs.ignore_behaviour = ignore_behaviour
    otherargs.ignore_values = ignore_values

    if ignore_behaviour == IGNORE_VALUES and ignore_values is None:
        raise ValueError('must specify ignore_values when '+
                        'ignore_behaviour = IGNORE_VALUES')

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
