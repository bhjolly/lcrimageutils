#!/usr/bin/env python

import os
import sys
import stat
import optparse
import tempfile
from rios import applier, cuiprogress
import numpy
from numba import autojit
from lcrimageutils import mdl, history

class CmdArgs(object):
    """
    Class for processing command line arguments
    """
    def __init__(self):
        self.parser = optparse.OptionParser()
        self.parser.add_option('-i', '--infile', dest='infile',
            help='Input image file to clump')
        self.parser.add_option('-o', '--output', dest='output',
            help="Output clumped image")
        self.parser.add_option('-t', '--tempdir', dest='tempdir',
            default='.',
            help="Temp directory to use (default=%default)")

        (options, self.args) = self.parser.parse_args()
        self.__dict__.update(options.__dict__)

        if (self.infile is None or self.output is None):
            self.parser.print_help()
            sys.exit(1)

def riosClump(info, inputs, outputs, otherinputs):
    """
    Called from RIOS - clumps each individual tile separately
    (but with globally unique ids)
    """
    # create valid mask that is True where data!=ignore
    ignore = info.getNoDataValueFor(inputs.infile)
    if ignore is not None:
        valid = inputs.infile[0] != ignore
    else:
        # no ignore val set - all valid
        valid = numpy.ones_like(inputs.infile[0], dtype=numpy.bool)

    out, clumpId = mdl.clump(inputs.infile[0], valid, otherinputs.clumpId)

    outputs.outfile = mdl.makestack([out])
    otherinputs.clumpId = clumpId

@autojit
def numbaMerge(infile, clump, recode, dontrecode, processBottom, processRight):
    """
    Merge the clumps using a slightly complicated algorithm
    """
    ysize, xsize = infile.shape
    nFailedRecodes = 0

    @jit('int_()')
    def merge():
        thisClumpId = clump[thisy, thisx]
        otherClumpId = clump[othery, otherx]
        # don't bother if they already have the same id 
        # and don't bother recoding 0's
        if (thisClumpId != otherClumpId and thisClumpId != 0 and 
                otherClumpId != 0):
            if not dontrecode[otherClumpId]:
                # only if this one isn't marked to be recoded
                recode[otherClumpId] = recode[thisClumpId]
                # recode while we go
                clump[clump == otherClumpId] = thisClumpId
                # mark as not to be recoded
                dontrecode[thisClumpId] = True
            elif not dontrecode[thisClumpId]:
                # ok try the other way around
                recode[thisClumpId] = recode[otherClumpId]
                clump[clump == thisClumpId] = otherClumpId
                dontrecode[otherClumpId] = True
            else:
                # need to re run the whole thing
                # both are marked as un-recodable
                return 1
        return 0
        

    if processBottom:
        # this isn't the bottom most block so run along the bottom
        # 2 lines and decide if clumps need to be merged
        thisy = ysize - 2
        othery = ysize - 1
        # start one pixel in and finish one pixel in
        for thisx in range(0, xsize-1):
            otherx = thisx
            if infile[thisy, thisx] == infile[othery, otherx]:
                # they should be the same clump - they have the same DNs
                nFailedRecodes += merge()

    # same for the right hand margin if we aren't the rightmost block
    if processRight:
        thisx = xsize - 2
        otherx = xsize - 1
        for thisy in range(0, ysize-1):
            othery = thisy
            if infile[thisy, thisx] == infile[othery, otherx]:
                # they should be the same clump
                nFailedRecodes += merge()

    return nFailedRecodes

def riosMerge(info, inputs, outputs, otherinputs):
    """
    Determine which clumps need to be merged
    and update otherinputs.recode
    """
    # overlap is set to one
    # we only need to inspect the right and bottom edge
    # and only if we aren't rightmost or bottom most
    xblock, yblock = info.getBlockCount()
    xtotalblocks, ytotalblocks = info.getTotalBlocks()
    processBottom = yblock != (ytotalblocks-1)
    processRight = xblock != (xtotalblocks-1)

    # recode this clump with the up to date recode table
    # so we are working with the right clumpids
    clump = otherinputs.recode[inputs.tileclump[0]]

    # merge the clumps (updates clump, otherinputs.recode
    # and otherinputs.dontrecode)
    nFailedRecodes = numbaMerge(inputs.infile[0], clump, 
            otherinputs.recode, otherinputs.dontrecode, processBottom, processRight)

    # keep a track
    otherinputs.nFailedRecodes += nFailedRecodes

    # output the merged clumps
    outputs.clump = mdl.makestack([clump])

def doClump(infile, outfile, tempDir):
    """
    Do the clumping
    """
    inputs = applier.FilenameAssociations()
    inputs.infile = infile

    # create temporary file with the clumps done on a per tile basis
    outputs = applier.FilenameAssociations()
    fileh, tmpClump = tempfile.mkstemp('.kea', dir=tempDir)
    os.close(fileh)
    outputs.outfile = tmpClump

    # start at clumpid 1 - will be zeros where no data
    otherinputs = applier.OtherInputs()
    otherinputs.clumpId = 1

    controls = applier.ApplierControls()
    controls.progress = cuiprogress.GDALProgressBar()
    # don't need stats for this since it is just temporary
    controls.calcStats = False

    applier.apply(riosClump, inputs, outputs, otherinputs, controls=controls)

    # run it on the input image again, but also read in the tile clumps
    inputs.tileclump = tmpClump

    # overlap of 1 so we can work out which neighbouring
    # clumps need to be merged
    controls.overlap = 1
    # make thematic
    # but still don't do stats - only when we finally know we succeeded
    controls.thematic = True

    outputs = applier.FilenameAssociations()

    finished = False
    while not finished:

        # it creates the output file as it goes
        # just create temp at this stage until we know it has succeeded
        fileh, tmpMerged = tempfile.mkstemp('.kea', dir=tempDir)
        os.close(fileh)

        outputs.clump = tmpMerged

        otherinputs.nFailedRecodes = 0
        # ok now we have to merge the clumps
        # create a recode table
        recode = numpy.arange(otherinputs.clumpId, dtype=numpy.uint32)    
        otherinputs.recode = recode

        # create a boolean array with clumps not to recode
        # obviously if you recode 39 -> 23 you don't want 23 being recoded to 
        # something else
        dontrecode = numpy.zeros_like(recode, dtype=numpy.bool)
        otherinputs.dontrecode = dontrecode

        applier.apply(riosMerge, inputs, outputs, otherinputs, controls=controls)

        # clobber the last temp input
        os.remove(inputs.tileclump)

        inputs.tileclump = tmpMerged

        dontrecodesum = dontrecode.sum()
        finished = dontrecodesum == 0
        if not finished:
            print('%d clumps failed to merge. %d recoded' % (
                        otherinputs.nFailedRecodes, dontrecodesum))
            print('having another go')

    # now we save the final output as the output name and calc stats
    cmd = 'gdalcalcstats %s -ignore 0' % tmpMerged
    os.system(cmd)

    os.rename(tmpMerged, outfile)

    # just be careful here since the permissions will be set strangely
    # for outfile since it was created by tempfile. Set to match current umask
    current_umask = os.umask(0)
    os.umask(current_umask) # can't get without setting!
    # need to convert from umask to mode and remove exe bits
    mode = 0o0777 & ~current_umask
    mode = (((mode ^ stat.S_IXUSR) ^ stat.S_IXGRP) ^ stat.S_IXOTH)

    os.chmod(outfile, mode)

    history.insertMetadataFilename(outfile, [infile], {})

if __name__ == '__main__':
    cmdargs = CmdArgs()

    doClump(cmdargs.infile, cmdargs.output, cmdargs.tempdir)
    
