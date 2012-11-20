
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
    # create some blank arrays
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
        
def zoneMajority(clumpFile, dataFile, clumpBand=1, dataBand=1):
    """
    Given a file of clumps and a file of data, calculates
    the most common data values for each clump and the histogram
    Returns an array with the most common values and a histogram. 
    The indices of this array go from zero to the maximum clump value 
    and have values for each clump id, zero for other indices.
    The histogram is a dictionary, keyed on the clump id. Each value
    in the dictionary is itself a dictionary keyed on the data value,
    with the count of that value.
    """
    
    fileDict = {'clumps':clumpFile, 'data':dataFile}
    
    # index is the clump id
    clumpDict = {}
    
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
                # do we have this value in histDict?
                if value in clumpDict:
                    # yes, retrieve dict
                    histDict = clumpDict[value]
                else:
                    # no, create it and set it
                    histDict = {}
                    clumpDict[value] = histDict

                # do the bincount
                bincount = numpy.bincount(dataSubset)
                # turn this into a dictionary
                bins = numpy.arange(bincount.size)
                # only interested in values where count != 0
                bins = numpy.compress(bincount != 0, bins)
                bincount = numpy.compress(bincount != 0, bincount)

                for count in range(bins.size):
                    binvalue = bins[count]
                    if binvalue in histDict:
                        histDict[binvalue] += bincount[count]
                    else:
                        histDict[binvalue] = bincount[count]
                
    # work out the length of the arrays and 
    # create a blank arrays
    maxidx = max(clumpDict.keys()) + 1
    modeArray = numpy.zeros((maxidx,), numpy.uint32)
    
    # go thru each value
    for value in clumpDict.keys():
        # find the mode
        histDict = clumpDict[value]
        maxValue, maxCount = max(histDict.iteritems(), key=lambda x:x[1])

        modeArray[value] = maxValue
            
    return modeArray, clumpDict
        
    
                    
