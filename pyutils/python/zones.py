
"""
Collection of routines that deals with zones
(ie clumped images)

"""
import numpy
from rios.imagereader import ImageReader

def zoneMeans(clumpFile, dataFile, clumpBand=1, dataBand=1):
    """
    Given a file of clumps and a file of data, calculates
    the mean and standard deviation for the area of each
    clump value in the data. 
    Returns two arrays, one with the mean values, one
    with the standard deviation values. The indices of these
    arrays go from zero to the maximum clump value and have values
    for each clump id, zero for other indices.
    """
    
    fileDict = {'clumps':clumpFile, 'data':dataFile}
    
    # use dictionaries for accumulated values
    # index is the clump id
    sumDict = {}
    sumsqDict = {}
    countDict = {}
    
    # red thru the images
    reader = ImageReader(fileDict)
    for (info, blocks) in reader:
        # get the data for the specified bands and flatten it
        clumps = blocks['clumps'][clumpBand-1].flatten()
        data = blocks['data'][dataBand-1].flatten()
        
        # for each clump id
        for value in numpy.unique(clumps):
            # get the data for that clump
            dataSubset = data.compress(clumps == value)
            # check we have data
            if dataSubset.size != 0:
                # calculate the values
                sum = dataSubset.sum()
                sq = dataSubset * dataSubset
                sumsq = sq.sum()

                # check if we encountered this value or not
                # and load into our dictioanaries
                if value in sumDict:
                    sumDict[value] += sum
                    sumsqDict[value] += sumsq
                    countDict[value] += dataSubset.size
                else:
                    sumDict[value] = sum
                    sumsqDict[value] = sumsq
                    countDict[value] = dataSubset.size
                
    # work out the length of the arrays and 
    # create some black arrays
    maxidx = max(sumDict.keys()) + 1
    meanArray = numpy.zeros((maxidx,), numpy.float)
    stdArray = numpy.zeros((maxidx,), numpy.float)
    
    # go thru each value
    for value in sumDict.keys():
        sum = sumDict[value]
        sumsq = sumsqDict[value]
        count = countDict[value]

        # do the calculations        
        mean = sum / count
        meanArray[value] = mean
        stdArray[value] = numpy.sqrt((sumsq / count) - (mean * mean))
            
    return meanArray, stdArray
        
    
                    
