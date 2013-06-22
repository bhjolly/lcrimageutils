#!/usr/bin/env python
"""
Functions which are of use in PyModeller, and elsewhere. These are
mainly mimics/replacements for functions which are available in 
Imagine Spatial Modeller, but are not native to pymodeller/numpy. 

"""

import sys
import numpy
from scipy.ndimage import label
try:
    from scipy import weave
except ImportError:
    # not available under Python 3
    # ignore until I get around to re-writing in numba.
    pass

class MdlFuncError(Exception):
    pass
class ShapeMismatchError(MdlFuncError):
    pass
class ScalarMaskError(MdlFuncError):
    pass
class NotImageError(MdlFuncError):
    pass
class NonIntTypeError(MdlFuncError):
    pass

def stackwhere(mask, trueVal, falseVal):
    """
    Behaves like a numpy "where" function, but specifically for
    use with images and image stacks. Thus, the mask can be a single
    layer image (i.e. a 2-d array), but the true and false values can
    be multi-layer image stacks (i.e. 3-d arrays, where the first 
    dimension is layer number, and the other two dimensions must
    match those of the mask). 
    
    The output will be of the same shape as the true and false 
    input arrays. 
    
    When one or other of the true or false values is given as a scalar, or 
    a single image, it will attempt to promote the rank to match the other. 
    
    It seems to work fine for single layer inputs as well, although
    it wasn't really designed for it. They return as a 3-d array, but
    with only one layer. 
    
    """
    if numpy.isscalar(mask):
        raise ScalarMaskError("Mask is a scalar - this needs an array")
    
    if mask.ndim != 2:
        raise NotImageError("Function only works with images. Shape of mask is %s"%(mask.shape,))
    
    inputList = [trueVal, falseVal]
    nLayersList = [0, 0]
    valStr = ['trueVal', 'falseVal']        # For error messages
    for i in [0, 1]:
        val = inputList[i]
        if numpy.isscalar(val):
            nLayersList[i] = 0
        elif val.ndim == 2:
            nLayersList[i] = 1
        elif val.ndim == 3:
            nLayersList[i] = val.shape[0]
        else:
            raise NotImageError("%s is neither a scalar nor a 2-d or 3-d array. Its shape is %s" %
                    (valStr[i], val.shape))

    maxLayers = max(nLayersList)
    minLayers = min(nLayersList)
    if maxLayers not in [0, 1] and minLayers not in [0, 1] and maxLayers != minLayers:
        raise ShapeMismatchError("Stacks must have same number of layers: %s != %s"%(maxLayers, minLayers))
    
    stack = []
    for i in range(maxLayers):
        if numpy.isscalar(trueVal) or trueVal.ndim == 2:
            tVal = trueVal
        elif trueVal.ndim == 3:
            tVal = trueVal[i]
            
        if numpy.isscalar(falseVal) or falseVal.ndim == 2:
            fVal = falseVal
        elif falseVal.ndim == 3:
            fVal = falseVal[i]
        
        img = numpy.where(mask, tVal, fVal)
        stack.append(img)
        
    result = numpy.array(stack)
    return result


def and_list(conditionList):
    """
    Takes a list of condition arrays and does logical_and on the whole
    lot. 
    """
    return reduce(numpy.logical_and, conditionList)

def or_list(conditionList):
    """
    Takes a list of condition arrays and does logical_or on the whole
    lot. 
    """
    return reduce(numpy.logical_or, conditionList)


def pixinlist(img, valList):
    """
    Returns a mask where pixels are true if the corresponding 
    pixel values in the input img are in the given valList. 
    
    Most useful for lists of specific non-contiguous values. If values are really
    ranges between two values, probably easier to use a logical_and(). 
    """
    mask = numpy.zeros(img.shape, dtype=bool)
    for val in valList:
        mask[img==val] = True
    return mask


def makestack(inputList):
    """
    Makes a single stack of the various images and/or stacks given in 
    the list of inputs. Copes with some being single layer (i.e. 2-D) and some
    being multi-layer (i.e. 3-D). 
    """
    stack = []
    for img in inputList:
        if img.ndim == 2:
            stack.append(img[numpy.newaxis, ...])
        elif img.ndim == 3:
            stack.append(img)
    
    return numpy.concatenate(stack, axis=0)


def uniquevalues(a):
    """
    This function is entirely redundant, since there is an equivalent
    numpy function, numpy.unique(). I just didn't know it existed when 
    I wrote this. Please refrain from using this, use numpy.unique() 
    instead. 
    
    Returns a list of the unique values contained in the given array.
    Only works for integer-ish types, since float arrays (and other more 
    complicated types) would be too problematic. 
    
    """
    return list(numpy.unique(a))


def clump(img, nbrcount=4, connect=None, nullVal=0):
    """
    Mimics the Imagine modeller CLUMP function. Only works for
    single band images (i.e. img is a 2d array). 
    
    Needs to use the whole image at once, so if using from inside
    pymodeller, it should be used with doFullWindow(). 
    
    Returns an array of the same shape as img, with contiguous clumps 
    of equal values each given a unique clump id. Contiguousness is tested 
    against either the four direct neighbours or all eight surrounding 
    neighbours, depending on the value of nbrcount. This is the same
    controlling mechanism as used in Imagine. There is also a more general
    control using the connect argument (which over-rides nbrcount if given). 
    Connect is a 3x3 array which maps how the central pixel is tested
    against those surrounding. A zero in a surrounding pixel means no test
    against that pixel. 
    
    The nullVal is used to fill the output clump array, and pixels with this
    value in the input array are not counted as part of any clump. 
    
    Only works on an integer-like input array. 
    
    """
    shape = img.shape
    clumps = numpy.zeros(shape, dtype=numpy.uint32)
    clumps[...] = nullVal
    
    if connect is None:
        if nbrcount == 4:
            connect = numpy.array([[0,1,0],[1,1,1],[0,1,0]])
        elif nbrcount == 8:
            connect = numpy.ones((3,3))
    
    clumpid = numpy.uint32(0)
    imgvals = uniquevalues(img)
    for val in imgvals:
        if val != nullVal:
            mask = (img == val)
            labelledmask = numpy.zeros(shape, dtype=numpy.int32)
            numObj = label(mask, structure=connect, output=labelledmask)
            clumps = numpy.where(mask, labelledmask + clumpid, clumps)
            clumpid += numpy.uint32(numObj)
                
    return clumps.astype(numpy.uint32)


def clumpsizes(clumps, nullVal=0):
    """
    This function is almost entirely redundant, and should be replaced with 
    numpy.bincount(). This function now uses that instead. Please use
    that directly, instead of this. The only difference is the treatment 
    of null values (bincount() does not treat them in any special way, 
    whereas this function allows a nullVal to be set, and the count for 
    that value is zero). 
    
    Takes a clump id image as given by the clump() function and counts the 
    size of each clump, in pixels. Essentially this is doing a histogram
    of the clump img. Returns an array of pixel counts, where the array index
    is equal to the corresponding clump id. 
    
    """
    counts = numpy.bincount(clumps.flatten())
    counts[nullVal] = nullVal

    return counts


def clumpsizeimg(clumps, nullVal=0):
    """
    Takes a clump id image as given by the clump() function, and returns 
    an image of the same shape but with each clump pixel equal to the 
    number of pixels in its clump. This can then be used to contruct a 
    mask of pixels in clumps of a given size. 
    
    """
    sizes = clumpsizes(clumps, nullVal)
    sizeimg = sizes[clumps]
    return sizeimg


class ValueIndexes(object):
    """
    An object which contains the indexes for every value in a given array.
    This class is intended to mimic the reverse_indices clause in IDL,
    only nicer. 
    
    Takes an array, works out what unique values exist in this array. Provides
    a method which will return all the indexes into the original array for 
    a given value. 
    
    The array must be of an integer-like type. Floating point arrays will
    not work. If one needs to look at ranges of values within a float array,
    it is possible to use numpy.digitize() to create an integer array 
    corresponding to a set of bins, and then use ValueIndexes with that. 
    
    Example usage, for a given array a
        valIndexes = ValueIndexes(a)
        for val in valIndexes.values:
            ndx = valIndexes.getIndexes(val)
            # Do something with all the indexes
    
    
    This is a much faster and more efficient alternative to something like
        values = numpy.unique(a)
        for val in values:
            mask = (a == val)
            # Do something with the mask
    The point is that when a is large, and/or the number of possible values 
    is large, this becomes very slow, and can take up lots of memory. Each 
    loop iteration involves searching through a again, looking for a different 
    value. This class provides a much more efficient way of doing the same 
    thing, requiring only one pass through a. When a is small, or when the 
    number of possible values is very small, it probably makes little difference. 
    
    If one or more null values are given to the constructor, these will not 
    be counted, and will not be available to the getIndexes() method. This 
    makes it more memory-efficient, so it doesn't store indexes of a whole 
    lot of nulls. 
    
    A ValueIndexes object has the following attributes:
        values              Array of all values indexed
        counts              Array of counts for each value
        nDims               Number of dimensions of original array
        indexes             Packed array of indexes
        start               Starting points in indexes array for each value
        end                 End points in indexes for each value
        valLU               Lookup table for each value, to find it in 
                            the values array without explicitly searching. 
        nullVals            Array of the null values requested. 
    
    """
    def __init__(self, a, nullVals=[]):
        """
        Creates a ValueIndexes object for the given array a. 
        A sequence of null values can be given, and these will not be included
        in the results, so that indexes for these cannot be determined. 
        
        """
        integerTypes = [numpy.bool, numpy.int8, numpy.uint8, numpy.int16, numpy.uint16,
            numpy.int32, numpy.uint32, numpy.int64, numpy.uint64]
        if a.dtype not in integerTypes:
            raise NonIntTypeError("ValueIndexes only works on integer-like types. Array is %s"%a.dtype)
             
        if numpy.isscalar(nullVals):
            self.nullVals = [nullVals]
        else:
            self.nullVals = nullVals

        # Get counts of all values in a
        minval = a.min()
        maxval = a.max()
        (counts, binEdges) = numpy.histogram(a, range=(minval, maxval+1), 
            bins=(maxval-minval+1), new=True)
            
        # Mask counts for any requested null values. 
        maskedCounts = counts.copy()
        for val in self.nullVals:
            maskedCounts[binEdges[:-1]==val] = 0
        self.values = binEdges[maskedCounts>0].astype(a.dtype)
        self.counts = maskedCounts[maskedCounts>0]
        
        # Allocate space to store all indexes
        totalCounts = self.counts.sum()
        self.nDims = a.ndim
        self.indexes = numpy.zeros((totalCounts, a.ndim), dtype=int)
        self.end = self.counts.cumsum()
        self.start = self.end - self.counts
        
        # A lookup table to make searching for a value very fast.
        valrange = numpy.array([self.values.min(), self.values.max()])
        numLookups = valrange[1] - valrange[0] + 1
        self.valLU = numpy.zeros(numLookups, dtype=numpy.uint16)
        self.valLU.fill(-1)     # A value to indicate "not found"
        for i in range(len(self.values)):
            val = self.values[i]
            self.valLU[val - valrange[0]] = i
        
        # Array names to pass to the weave C code
        counts = self.counts
        values = self.values
        indexes = self.indexes
        start = self.start
        end = self.end
        nDims = self.nDims
        numVals = len(values)
        # For use within weave code. For each value, the current index 
        # into the indexes array. A given element is incremented whenever it finds
        # a new element of that value. 
        currentIndex = start.copy()
        valLU = self.valLU
        
        Ccode = """
            int a_ndx, n, j, m, i, found, stride;
            long int a_val;
            PyArrayIterObject *iter = NULL;
            
            /* Use generic iterator to iterate over all elements 
               in the array, regardless of how many dimensions it has.
            */
            iter = (PyArrayIterObject *)PyArray_IterNew((PyObject *)a_array);
            PyArray_ITER_RESET(iter);
            a_ndx = 0;
            while (PyArray_ITER_NOTDONE(iter)) {
                /* Extract the current array element into an integer. 
                   I am sure there must be a better way of doing the 
                   type casting, but I can't find it. */
                switch (PyArray_TYPE(a_array)) {
                    case NPY_BOOL: 
                    case NPY_BYTE: 
                        a_val = *((char *)PyArray_ITER_DATA(iter)); break;
                    case NPY_UBYTE: a_val = *((unsigned char *)PyArray_ITER_DATA(iter)); break;
                    case NPY_SHORT: a_val = *((short *)PyArray_ITER_DATA(iter)); break;
                    case NPY_USHORT: a_val = *((unsigned short *)PyArray_ITER_DATA(iter)); break;
                    case NPY_INT: 
                    case NPY_LONG: 
                    case NPY_LONGLONG: 
                        a_val = *((int *)PyArray_ITER_DATA(iter)); break;
                    case NPY_UINT: 
                    case NPY_ULONG: 
                    case NPY_ULONGLONG: 
                        a_val = *((unsigned int *)PyArray_ITER_DATA(iter)); break;
                }
                
                /* Find this value in the array of values */
                found = 0;  /* FALSE */
                if ((a_val >= valrange(0)) && (a_val <= valrange(1))) {
                    i = valLU(a_val - valrange(0));
                    found = (i >= 0);
                }
                
                if (found) {
                    /* Work out what the index values are in the various dimensions,
                       and store them in indexes array. */
                    n = a_ndx;
                    m = currentIndex(i);
                    for (j=0; j<Nindexes[1]; j++) {
                        stride = PyArray_STRIDES(a_array)[j] / PyArray_ITEMSIZE(a_array);
                        indexes(m, j) = n / stride;
                        n = n - indexes(m, j) * stride;
                    }
                    currentIndex(i) = m + 1;
                }
                
                a_ndx += 1;
                PyArray_ITER_NEXT(iter);
            }
        """
        
        variables = ['a', 'values', 'indexes', 'currentIndex', 'valLU', 'valrange']
        weave.inline(Ccode, variables)


    def getIndexes(self, val):
        """
        Return a set of indexes into the original array, for which the
        value in the array is equal to val. 
        
        """
        # Find where this value is listed. 
        valNdx = (self.values == val).nonzero()[0]
        
        # If this value is not actually in those listed, then we 
        # must return empty indexes
        if len(valNdx) == 0:
            start = 0
            end = 0
        else:
            # The index into counts, etc. for this value. 
            valNdx = valNdx[0]
            start = self.start[valNdx]
            end = self.end[valNdx]
            
        # Create a tuple of index arrays, one for each index of the original array. 
        ndx = ()
        for i in range(self.nDims):
            ndx += (self.indexes[start:end, i], )
        return ndx
            


def prewitt(img):
    """
    Implements a prewitt edge detection filter which behaves the same as the
    one in Imagine Spatial Modeller. 
    
    Note, firstly, that the one given in scipy.ndimage is broken, and 
    secondly, that the edge detection filter given in the Imagine Viewer is
    different to the one available as a graphic model. This function is the
    same as the one supplied as a graphic model. 
    
    Returns an edge array of the same shape as the input image. The kernel
    used is a 3x3 kernel, so when this function is used from within pymodeller
    the settings should have a window overlap of 1. The returned array is of type
    float, and care should be taken if truncating to an integer type. The magnitude 
    of the edge array scales with the magnitude of the input pixel values. 
    
    """
    kernel1 = numpy.array([1.0, 0.0, -1.0])
    kernel2 = numpy.array([1.0, 1.0, 1.0])
    Gx1 = convRows(img, kernel1)
    Gx = convCols(Gx1, kernel2)
    Gy1 = convCols(img, kernel1)
    Gy = convRows(Gy1, kernel2)
    
    G = numpy.sqrt(Gx*Gx + Gy*Gy)
    
    return G

def convRows(a, b):
    """
    Utility function to convolve b along the rows of a
    """
    out = numpy.zeros(a.shape)
    for i in range(a.shape[0]):
        out[i,1:-1] = numpy.correlate(a[i,:], b)
    return out

def convCols(a, b):
    """
    Utility function to convolve b along the cols of a
    """
    out = numpy.zeros(a.shape)
    for j in range(a.shape[1]):
        out[1:-1,j] = numpy.correlate(a[:,j], b)
    return out

def stretch(imgLayer, numStdDev, minVal, maxVal, ignoreVal, 
        globalMean, globalStdDev, outputNullVal=0):
    """
    Implements the Imagine Modeller STRETCH function. Takes
    a single band and applies a histogram stretch to it, returning
    the stretched image. The returned image is always a 2-d array. 
    
    """

    stretched = (minVal + 
        ((imgLayer - globalMean + globalStdDev * numStdDev) * (maxVal - minVal))/(globalStdDev * 2 *numStdDev))
        
    stretched = stretched.clip(minVal, maxVal)
    stretched = numpy.where(imgLayer == ignoreVal, outputNullVal, stretched)
    return stretched


def median_filter(imgLayer, size, ignore=None):
    """
    Performs a median filter on the input array (either 2 or 3d).
    size is the size of the filter in pixels and must be an 
    odd number. It is assumed that the edge ((size-1)/2) pixels
    are thrown away and are not filled in on the output. 
    ignore, if specified is the ignore value not to include
    in the calculations.
    This appears to be much faster than scipy.ndimage
    """

    origdims = imgLayer.ndim
    if origdims == 2:
        # code expects a 3d array so reshape with extra axis
        shape = imgLayer.shape
        imgLayer = imgLayer.reshape(1,shape[0],shape[1])
    elif origdims != 3:
        raise ShapeMismatchError("Can only handle 2 or 3d arrays")

    outLayer = numpy.empty_like(imgLayer)
    if size % 2 != 1:
        raise ShapeMismatchError("size must be odd")

    if ignore is None:
        # so it appears as a value for weave
        hasIgnore = 0
        ignore = 0
    else:
        hasIgnore = 1

    Ccode = """
        int offset = (size-1)/2;
        std::vector<float> tmpvector;
        float fResult;
        for( int band = 0; band < NimgLayer[0]; band++)
        {
            for( int y = offset; y < (NimgLayer[1]-offset); y++ )
            {
                for( int x = offset; x < (NimgLayer[2]-offset); x++ )
                {
                    if( hasIgnore && (IMGLAYER3(band, y, x) == ignore))
                    {
                        fResult = ignore;
                    }
                    else
                    {
                        tmpvector.clear();
                        for( int sy = y - offset; sy <= y + offset; sy++ )
                        {
                            for( int sx = x - offset; sx <= x + offset; sx++ )
                            {
                                float fVal = IMGLAYER3(band,sy,sx);
                                if( !hasIgnore || (fVal != ignore) )
                                    tmpvector.push_back(fVal);
                            }
                        }
                        if( tmpvector.size() == 0 )
                            fResult = ignore;
                        else
                        {
                            std::sort(tmpvector.begin(), tmpvector.end());
                            int middle = tmpvector.size() / 2;
                            if( ( middle % 2) == 1 )
                                fResult = tmpvector[middle];
                            else
                                fResult = (tmpvector[middle-1] + tmpvector[middle]) / 2.0;
                        }
                    }
                    OUTLAYER3(band,y,x) = fResult;
                }
            }
        }
    """
    variables = ['size','imgLayer','outLayer','ignore','hasIgnore']
    weave.inline(Ccode, variables, headers=['<vector>','<algorithm>'])

    if origdims == 2:
        # convert back to 2d for return
        outLayer = outLayer[0]

    return outLayer

def lowpass_filter(imgLayer, size, ignore=None):
    """
    Performs a lowpass filter on the input array (either 2 or 3d).
    size is the size of the filter in pixels and must be an 
    odd number. It is assumed that the edge ((size-1)/2) pixels
    are thrown away and are not filled in on the output. 
    ignore, if specified is the ignore value not to include
    in the calculations.
    This appears to be much faster than scipy.ndimage
    """

    origdims = imgLayer.ndim
    if origdims == 2:
        # code expects a 3d array so reshape with extra axis
        shape = imgLayer.shape
        imgLayer = imgLayer.reshape(1,shape[0],shape[1])
    elif origdims != 3:
        raise ShapeMismatchError("Can only handle 2 or 3d arrays")

    outLayer = numpy.empty_like(imgLayer)
    if size % 2 != 1:
        raise ShapeMismatchError("size must be odd")

    if ignore is None:
        # so it appears as a value for weave
        hasIgnore = 0
        ignore = 0
    else:
        hasIgnore = 1

    Ccode = """
        int offset = (size-1)/2;
        float fTotal, fResult;
        int count;
        for( int band = 0; band < NimgLayer[0]; band++)
        {
            for( int y = offset; y < (NimgLayer[1]-offset); y++ )
            {
                for( int x = offset; x < (NimgLayer[2]-offset); x++ )
                {
                    if( hasIgnore && (IMGLAYER3(band, y, x) == ignore))
                    {
                        fResult = ignore;
                    }
                    else
                    {
                        count = 0;
                        fTotal = 0;
                        for( int sy = y - offset; sy <= y + offset; sy++ )
                        {
                            for( int sx = x - offset; sx <= x + offset; sx++ )
                            {
                                float fVal = IMGLAYER3(band, sy, sx);
                                if( !hasIgnore || (fVal != ignore) )
                                {
                                    count++;
                                    fTotal += fVal;
                                }
                            }
                        }
                        if( count == 0 )
                            fResult = ignore;
                        else
                            fResult = fTotal / count;
                    }
                    OUTLAYER3(band, y, x) = fResult;
                }
            }
        }
    """
    variables = ['size','imgLayer','outLayer','ignore','hasIgnore']
    weave.inline(Ccode, variables)

    if origdims == 2:
        # convert back to 2d for return
        outLayer = outLayer[0]

    return outLayer
