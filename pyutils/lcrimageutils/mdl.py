#!/usr/bin/env python
"""
Functions which are of use in PyModeller, and elsewhere. These are
mainly mimics/replacements for functions which are available in 
Imagine Spatial Modeller, but are not native to pymodeller/numpy. 

"""

import sys
import numpy
import functools
try:
    from numba import autojit, int_
    HAVE_NUMBA = True
except ImportError:
    # numba not available - expect some functions to run very slow...
    HAVE_NUMBA = False
    # have to define our own autojit so Python doesn't complain
    def autojit(func):
        print("Warning: Numba not available - current function will run very slowly...")
        return func

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
    return functools.reduce(numpy.logical_and, conditionList)

def or_list(conditionList):
    """
    Takes a list of condition arrays and does logical_or on the whole
    lot. 
    """
    return functools.reduce(numpy.logical_or, conditionList)


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


@autojit
def clump(input, valid, clumpId=0):
    """
    Implementation of clump using Numba
    Uses the 4 connected algorithm.

    Input should be an integer 2 d array containing the data to be clumped.
    Valid should be a boolean 2 d array containing True where data to be 
        processed
    clumpId is the start clump id to use

    returns a 2d uint32 array containing the clump ids
    and the highest clumpid used + 1
    """
    (ysize, xsize) = input.shape
    output = numpy.zeros_like(input, dtype=numpy.uint32)

    # lists slow from Numba - use an array since
    # we know the maximum size
    searchIdx = 0
    search_list = numpy.empty((xsize*ysize, 2), dtype=numpy.int)

    # run through the image
    for y in range(ysize):
        for x in range(xsize):
            # check if we have visited this one before
            if valid[y, x] and output[y, x] == 0:
                val = input[y, x]
                searchIdx = 0
                search_list[searchIdx, 0] = y
                search_list[searchIdx, 1] = x
                searchIdx += 1
                output[y, x] = clumpId # marked as visited

                while searchIdx > 0:
                    # search the last one
                    searchIdx -= 1
                    sy = search_list[searchIdx, 0]
                    sx = search_list[searchIdx, 1]

                    # work out the 3x3 window to vist
                    tlx = sx - 1
                    if tlx < 0:
                        tlx = 0
                    tly = sy - 1
                    if tly < 0:
                        tly = 0
                    brx = sx + 1
                    if brx > xsize - 1:
                        brx = xsize - 1
                    bry = sy + 1
                    if bry > ysize - 1:
                        bry = ysize - 1

                    for cx in range(tlx, brx+1):
                        for cy in range(tly, bry+1):
                            # do a '4 neighbour search'
                            # don't have to check we are the middle
                            # cell since output will be != 0
                            # since we do that before we add it to search_list
                            if (cy == sy or cx == sx) and (valid[cy, cx] and 
                                    output[cy, cx] == 0 and 
                                    input[cy, cx] == val):
                                output[cy, cx] = clumpId # mark as visited
                                # add this one to the ones to search the neighbours
                                search_list[searchIdx, 0] = cy
                                search_list[searchIdx, 1] = cx
                                searchIdx += 1
                clumpId += 1

    return output, clumpId

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
    
    Limtations:
        The array index values are handled using unsigned 32bit int values, so 
        it won't work if the data array is larger than 4Gb. I don't think it would
        fail very gracefully, either. 
    
    """
    def __init__(self, a, nullVals=[]):
        """
        Creates a ValueIndexes object for the given array a. 
        A sequence of null values can be given, and these will not be included
        in the results, so that indexes for these cannot be determined. 
        
        """
        integerTypes = [numpy.int8, numpy.uint8, numpy.int16, numpy.uint16,
            numpy.int32, numpy.uint32]
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
            bins=(maxval-minval+1))
            
        # Mask counts for any requested null values. 
        maskedCounts = counts.copy()
        for val in self.nullVals:
            maskedCounts[binEdges[:-1]==val] = 0
        self.values = binEdges[maskedCounts>0].astype(a.dtype)
        self.counts = maskedCounts[maskedCounts>0]
        
        # Allocate space to store all indexes
        totalCounts = self.counts.sum()
        self.nDims = a.ndim
        self.indexes = numpy.zeros((totalCounts, a.ndim), dtype=numpy.uint32)
        self.end = self.counts.cumsum()
        self.start = self.end - self.counts
        
        if len(self.values) > 0:
            # A lookup table to make searching for a value very fast.
            valrange = numpy.array([self.values.min(), self.values.max()])
            numLookups = valrange[1] - valrange[0] + 1
            self.valLU = numpy.zeros(numLookups, dtype=numpy.uint16)
            self.valLU.fill(-1)     # A value to indicate "not found"
            for i in range(len(self.values)):
                val = self.values[i]
                self.valLU[val - valrange[0]] = i

            # For use within C code. For each value, the current index 
            # into the indexes array. A given element is incremented whenever it finds
            # a new element of that value. 
            currentIndex = self.start.copy().astype(numpy.uint32)

            if not numpy.issubdtype(a.dtype, numpy.integer):
                raise NonIntTypeError('Array must be of integer type')

            _valndxFunc(a, self.indexes, valrange[0], valrange[1], self.valLU, currentIndex)

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

@autojit
def _valndxFunc(a, indexes, minVal, maxVal, valLU, currentIndex):
    """
    To be called by ValueIndexes. An implementation using Numba of Neil's
    C code. This has the advantage of being able to handle any integer
    type passed. However it does currently run much slower than the C code
    (although faster than plain Python) because of the reasons noted below.
    """
    # need to override the type otherwise numba creates an array of
    # type 'object' for some reason
    shape = numpy.array(a.shape, dtype=numpy.int)
    # our array that contains the current index in each of the dims
    curridx = numpy.zeros_like(shape)
    done = False
    lastidx = a.ndim - 1

    while not done:
        # this is currently slow since numba must create a Python
        # tuple each time arount the loop. There seems to be no
        # other way of getting the value at the specified indexes.
        arrVal = int_(a[tuple(curridx)])
        
        found = False
        j = 0
        if arrVal >= minVal and arrVal <= maxVal:
            j = valLU[arrVal - minVal]
            found = j >= 0

        if found:
            m = currentIndex[j]
            # I believe this next line is also quite slow
            # in numba since it is optimised for single
            # element access
            indexes[m] = curridx
            currentIndex[j] = m + 1        
    
        # code that updates curridx - incs the next dim
        # if we have done all the elements in the current 
        # dim
        idx = lastidx
        while idx >= 0:
            curridx[idx] = curridx[idx] + 1
            if curridx[idx] >= shape[idx]:
                curridx[idx] = 0
                idx -= 1
            else:
                break

        # if we are done we have run out of dims
        done = idx < 0

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

def makeBufferKernel(buffsize):
    """
    Make a 2-d array for buffering. It represents a circle of 
    radius buffsize pixels, with 1 inside the circle, and zero outside.
    """
    bufferkernel = None
    if buffsize > 0:
        n = 2 * buffsize + 1
        (r, c) = numpy.mgrid[:n, :n]
        radius = numpy.sqrt((r-buffsize)**2 + (c-buffsize)**2)
        bufferkernel = (radius <= buffsize).astype(numpy.uint8)
    return bufferkernel
