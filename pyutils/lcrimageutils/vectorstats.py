"""
Module for doing stats on rasters based on data from a vector
"""
import numpy
from rios import applier
from rios import pixelgrid
from osgeo import gdal
from osgeo import ogr
from osgeo import osr

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

ROUND_DOWN = 0
ROUND_UP = 1

def roundToRasterGridX(rastertransform, x, direction):
    xdiff = x - rastertransform[0]
    npix = xdiff / rastertransform[1]
    if direction == ROUND_DOWN:
        nwholepix = numpy.floor(npix)
    else:
        nwholepix = numpy.ceil(npix)
    xround = rastertransform[0] + nwholepix * rastertransform[1]
    return xround

def roundToRasterGridY(rastertransform, y, direction):
    xdiff = rastertransform[3] - y
    npix = xdiff / abs(rastertransform[5])
    if direction == ROUND_DOWN:
        nwholepix = numpy.floor(npix)
    else:
        nwholepix = numpy.ceil(npix)
    xround = rastertransform[0] + nwholepix * rastertransform[1]
    return xround
    

def calcWorkingExtent(vector, raster, layer):
    """
    Calculates the working extent of the vector
    in the coordinate system of the raster and returns a
    PixelGridDefn instance.
    Necessary since RIOS only calculates intersection
    based on rasters
    """
    vectords = ogr.Open(vector)
    vectorlyr = vectords.GetLayer(layer)

    vectorsr = vectorlyr.GetSpatialRef()
    vectorextent = vectorlyr.GetExtent()
    (xmin, xmax, ymin, ymax) = vectorextent
    

    rasterds = gdal.Open(raster)
    rasterproj = rasterds.GetProjection()
    if rasterproj is None or rasterproj == '':
        raise ValueError('Raster must have projection set')
    rastertransform = rasterds.GetGeoTransform()

    rastersr = osr.SpatialReference(rasterproj)
    transform = osr.CoordinateTransformation(vectorsr, rastersr)
    (tl_x, tl_y, z) = transform.TransformPoint(xmin, ymax)
    (tr_x, tr_y, z) = transform.TransformPoint(xmax, ymax)
    (bl_x, bl_y, z) = transform.TransformPoint(xmin, ymin)
    (br_x, br_y, z) = transform.TransformPoint(xmax, ymin)

    extent = (min(tl_x, bl_x), max(tr_x, br_x), min(bl_y, br_y), max(tl_y, tr_y))

    # round to pixels
    roundedextent = (roundToRasterGridX(rastertransform, extent[0], ROUND_DOWN),
                    roundToRasterGridX(rastertransform, extent[1], ROUND_UP),
                    roundToRasterGridX(rastertransform, extent[2], ROUND_DOWN),
                    roundToRasterGridX(rastertransform, extent[3], ROUND_UP))

    pixgrid = pixelgrid.PixelGridDefn(projection=rasterproj, xMin=roundedextent[0],
                        xMax=roundedextent[1], yMin=roundedextent[2],
                        yMax=roundedextent[3], xRes=rastertransform[1],
                        yRes=abs(rastertransform[5]))

    del vectords
    del rasterds

    return pixgrid
    

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

    # work out vector extent
    pixgrid = calcWorkingExtent(vector, raster, layer)
    controls.referencePixgrid = pixgrid

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
            results[int(band)] = stats # make sure not a numpy type

    return results
