
"""
Collection of routines that deals with zones
(ie clumped images)

"""
import numpy
from rios.imagereader import ImageReader

def zoneMeans(clumpFile, dataFile, clumpBand=1, dataBands=None, 
                    ignoreDataVals=None):
    """
    Given a file of clumps and a file of data, calculates
    the mean and standard deviation for the area of each
    clump value in the data. 
    If dataBands is None does all bands in the dataFile, otherwise
    pass list of 1-based band indices or a single integer
    If dataBands is None or a list, returns list of tuples. 
    Each tuple contains two arrays, one with the mean values, one
    with the standard deviation values. The indices of these
    arrays go from zero to the maximum clump value and have values
    for each clump id, zero for other indices.
    If dataBands is a single integer, returns a tuple with mean and
    standard deviation arrays as above.

    Ignore values(s) may be passed in with the ignoreDataVals parameter.
    This may be a single value in which case the same is used for all 
    dataBands, or a sequence the same length as dataValues.
    """
    
    fileDict = {'clumps':clumpFile, 'data':dataFile}

    origdataBands = dataBands # so we know whether to return list or tuple
    if isinstance(dataBands, int):
        dataBands = [dataBands] # treat as list for now

    if isinstance(ignoreDataVals, int):
        # make list same size as dataBands
        ignoreDataVals = [ignoreDataVals] * len(dataBands)

    # use dictionaries for accumulated values
    # index is the clump id
    # we have a list of these dictionaries one per dataBand
    sumDictList = []
    sumsqDictList = []
    countDictList = []
    if dataBands is not None:
        # if None, sorted below when we know how many bands
        # create the dictionaries for each band
        for dataBand in dataBands:
            sumDictList.append({})
            sumsqDictList.append({})
            countDictList.append({})
    
    # red thru the images
    reader = ImageReader(fileDict)
    for (info, blocks) in reader:
        # get the data for the specified bands and flatten it
        clumps = blocks['clumps'][clumpBand-1].flatten()

        if dataBands is None:
            # now we know how many bands there are for the default list
            dataBands = range(1, blocks['data'].shape[0]+1)
            # create the dictionaries for each band
            for dataBand in dataBands:
                sumDictList.append({})
                sumsqDictList.append({})
                countDictList.append({})

        for idx, dataBand in enumerate(dataBands):

            data = blocks['data'][dataBand-1].flatten()
            sumDict = sumDictList[dataBand-1]
            sumsqDict = sumsqDictList[dataBand-1]
            countDict = countDictList[dataBand-1]
        
            # for each clump id
            for value in numpy.unique(clumps):
                # get the data for that clump
                mask = (clumps == value)

                # if we are ignoring values then extend mask
                if ignoreDataVals is not None:
                    mask = mask & (data != ignoreDataVals[idx])

                dataSubset = data.compress(mask)
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

    resultList = []
    # go through each band    
    for dataBand in dataBands:
        sumDict = sumDictList[dataBand-1]
        sumsqDict = sumsqDictList[dataBand-1]
        countDict = countDictList[dataBand-1]

        # turn into arrays so we don't have to iterate
        idxs = numpy.fromiter(sumDict.keys(), numpy.integer)
        sums = numpy.zeros((maxidx,), numpy.float)
        sums[idxs] = numpy.fromiter(sumDict.values(), numpy.float)
        sumsqs = numpy.zeros((maxidx,), numpy.float)
        sumsqs[idxs] = numpy.fromiter(sumsqDict.values(), numpy.float)
        counts = numpy.zeros((maxidx,), numpy.integer)
        counts[idxs] = numpy.fromiter(countDict.values(), numpy.integer)

        # mask out invalid divides
        outInvalid = counts == 0
        counts[outInvalid] = 1

        means = sums / counts
        stds = numpy.sqrt((sumsqs / counts) - (means * means))

        means[outInvalid] = 0
        stds[outInvalid] = 0
        
        resultList.append((means, stds))
            
    if isinstance(origdataBands, int):
        return resultList[0] # only one item
    else:
        return resultList
        
def zoneMajority(clumpFile, dataFile, clumpBand=1, dataBands=None):
    """
    Given a file of clumps and a file of data, calculates
    the most common data values for each clump and the histogram
    If dataBands is None does all bands in the dataFile, otherwise
    pass list of 1-based band indices or a single integer
    If dataBands is None or a list, returns list of tuples. 
    Each tuple contains as array of the most common values and a histogram. 
    The indices of this array go from zero to the maximum clump value 
    and have values for each clump id, zero for other indices.
    The histogram is a dictionary, keyed on the clump id. Each value
    in the dictionary is itself a dictionary keyed on the data value,
    with the count of that value.
    If dataBands is a single integer, returns a tuple with the mode array and
    histogram dictionary as above.
    """

    origdataBands = dataBands # so we know whether to return list or tuple
    if isinstance(dataBands, int):
        dataBands = [dataBands] # treat as list for now
    
    fileDict = {'clumps':clumpFile, 'data':dataFile}
    
    # index is the clump id
    # list of dictionaries
    clumpDictList = []
    if dataBands is not None:
        # if None, sorted below when we know how many bands
        # create the dictionaries for each band
        for dataBand in dataBands:
            clumpDictList.append({})

    # red thru the images
    reader = ImageReader(fileDict)
    for (info, blocks) in reader:
        # get the data for the specified bands and flatten it
        clumps = blocks['clumps'][clumpBand-1].flatten()

        if dataBands is None:
            # now we know how many bands there are for the default list
            dataBands = range(1, blocks['data'].shape[0]+1)
            # create the dictionaries for each band
            for dataBand in dataBands:
                clumpDictList.append({})

        for dataBand in dataBands:

            data = blocks['data'][dataBand-1].flatten()
            clumpDict = clumpDictList[dataBand-1]
        
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
                
    resultList = []
    for dataBand in dataBands:
        # work out the length of the arrays and 
        # create a blank arrays
        maxidx = max(clumpDict.keys()) + 1
        modeArray = numpy.zeros((maxidx,), numpy.uint32)
    
        # go thru each value
        for value in clumpDict.keys():
            # find the mode
            histDict = clumpDict[value]
            maxValue, maxCount = max(histDict.items(), key=lambda x:x[1])

            modeArray[value] = maxValue

        resultList.append((modeArray, clumpDict))
            
    if isinstance(origdataBands, int):
        return resultList[0] # only one item
    else:
        return resultList
        
    
                    
