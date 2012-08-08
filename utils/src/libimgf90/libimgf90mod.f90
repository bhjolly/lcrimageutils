! This module is is allow proper type-checking and so forth when calling
! GDAL from Fortran 90, using Sam's wonderful interface package.
!
! Note there are two modules in the one file here, GDAL_Ctypes and GDALf90. 
! Needed to have GDAL_Ctypes as a separate module, because for some reason
! interface blocks don't follow the usual f90 scope rules.
! Fortunately, this is only needed internally, and outside of this
! file you don't need to know about Ctypes, you only need to 'use' GDALf90.
!
! Neil Flood. August 2005.
!
module GDAL_Ctypes
    ! To get the right sizes to match equivalent C types.
    ! For this, I think we will need to change the code between
    ! 32bit and 64bit. Bugger!
    ! Use 9 for 32bit, and 18 for 64bit. 
    ! see ifort Predefined Preprocessor Symbols
    integer, parameter :: C_INT_KIND = selected_int_kind(9)
    integer, parameter :: C_PTR_KIND = selected_int_kind(18)
    
    ! Also to match C types, but probably not so critical
    integer, parameter :: C_FLOAT_KIND = selected_real_kind(2, 30)
    integer, parameter :: C_DOUBLE_KIND = selected_real_kind(2, 300)
end module


module libimgf90
    ! this is to then make it available to things which 'use' libimgf90. 
    use GDAL_Ctypes
    
    
    ! GDALAccess
    integer(kind=C_INT_KIND), parameter :: GA_ReadOnly = 0, GA_Update = 1
    
    ! GDALDataType
    integer(kind=C_INT_KIND), parameter :: GDT_Unknown = 0, GDT_Byte = 1, GDT_UInt16 = 2, GDT_Int16 = 3, GDT_UInt32 = 4
    integer(kind=C_INT_KIND), parameter :: GDT_Int32 = 5, GDT_Float32 = 6, GDT_Float64 = 7, GDT_CInt16 = 8
    integer(kind=C_INT_KIND), parameter :: GDT_CInt32 = 9, GDT_CFloat32 = 10, GDT_CFloat64 = 11
    
    ! GDALRWFlag
    integer(kind=C_INT_KIND), parameter :: GF_Read = 0, GF_Write = 1
    
    ! Boolean
    integer(kind=C_INT_KIND), parameter :: GF_False = 0, GF_True = 1

    !  Can't make interfaces onto this as they have pointer types.       
    external GDALRasterIO
    external GDALReadBlock
    external GDALWriteBlock
    
    ! Interfaces for all the routines
    interface
        subroutine GDALAllRegister
        end subroutine
        
        subroutine GDALSetDescription(obj, descr)
            use GDAL_Ctypes
            integer(kind=C_PTR_KIND) obj
            character*(*) descr
        end subroutine
        
        subroutine GDALGetDescription(obj, descr)
            use GDAL_Ctypes
            integer(kind=C_PTR_KIND) obj
            character*(*) descr
        end subroutine
        
        function GDALOpen(fname,access)
            use GDAL_Ctypes
            integer(kind=C_PTR_KIND) GDALOpen
            integer(kind=C_INT_KIND) access
            character*(*) fname
        end function
        
        function GDALCreate(templ, fname, xsize, ysize, bands, pixeltype)
            use GDAL_Ctypes
            integer(kind=C_PTR_KIND) GDALCreate
            character*(*) fname
            integer(kind=C_PTR_KIND) templ
            integer(kind=C_INT_KIND) xsize, ysize, bands, pixeltype
        end function
        
        subroutine GDALClose(ds)
            use GDAL_Ctypes
            integer(kind=C_PTR_KIND) ds
        end subroutine
        
        function GDALGetRasterXsize(ds)
            use GDAL_Ctypes
            integer(kind=C_INT_KIND) GDALGetRasterXsize
            integer(kind=C_PTR_KIND) ds
        end function
        
        function GDALGetRasterYsize(ds)
            use GDAL_Ctypes
            integer(kind=C_INT_KIND) GDALGetRasterYsize
            integer(kind=C_PTR_KIND) ds
        end function
        
        function GDALGetRasterCount(ds)
            use GDAL_Ctypes
            integer(kind=C_INT_KIND) GDALGetRasterCount
            integer(kind=C_PTR_KIND) ds
        end function
        
        subroutine GDALGetGeoTransform(ds, transform)
            use GDAL_Ctypes
            integer(kind=C_PTR_KIND) ds
            real(kind=C_DOUBLE_KIND) transform(6)
        end subroutine
        
        subroutine GDALSetGeoTransform(ds, transform)
            use GDAL_Ctypes
            integer(kind=C_PTR_KIND) ds
            real(kind=C_DOUBLE_KIND) transform(6)
        end subroutine

        subroutine GDALwld2pix(transform, dwldx, dwldy, x, y)
            use GDAL_Ctypes
            real(kind=C_DOUBLE_KIND) transform(6), dwldx, dwldy
            integer(kind=C_INT_KIND) x, y
        end subroutine
        
        subroutine GDALpix2wld(transform, x, y, dwldx, dwldy)
            use GDAL_Ctypes
            real(kind=C_DOUBLE_KIND) transform(6), dwldx, dwldy
            integer(kind=C_INT_KIND) x, y
        end subroutine
        
        subroutine GDALSetProjection(ds, proj)
            use GDAL_Ctypes
            integer(kind=C_PTR_KIND) ds
            character*(*) proj
        end subroutine
        
        subroutine GDALGetProjection(ds, proj)
            use GDAL_Ctypes
            integer(kind=C_PTR_KIND) ds
            character*(*) proj
        end subroutine
        
        subroutine GDALCalcStats(file)
            use GDAL_Ctypes
            character*(*) file
        end subroutine
        
        subroutine GDALCalcStatsIgnore(file, ignore)
            use GDAL_Ctypes
            character*(*) file
            integer(kind=C_FLOAT_KIND) ignore
        end subroutine
        
        function GDALGetRasterBand(ds, band)
            use GDAL_Ctypes
            integer(kind=C_PTR_KIND) ds, GDALGetRasterBand
            integer(kind=C_INT_KIND) band
        end function
        
        function GDALGetRasterDataType(bnd)
            use GDAL_Ctypes
            integer(kind=C_INT_KIND) GDALGetRasterDataType
            integer(kind=C_PTR_KIND) bnd
        end function
        
        subroutine GDALGetBlockSize(bnd, xsize, ysize)
            use GDAL_Ctypes
            integer(kind=C_PTR_KIND) bnd
            integer(kind=C_INT_KIND) xsize, ysize
        end subroutine
        
        function GDALGetRasterBandXSize(bnd)
            use GDAL_Ctypes
            integer(kind=C_INT_KIND) GDALGetRasterBandXSize
            integer(kind=C_PTR_KIND) bnd
        end function
        
        function GDALGetRasterBandYSize(bnd)
            use GDAL_Ctypes
            integer(kind=C_INT_KIND) GDALGetRasterBandYSize
            integer(kind=C_PTR_KIND) bnd
        end function
        
        subroutine GDALFlushRasterCache(bnd)
            use GDAL_Ctypes
            integer(kind=C_PTR_KIND) bnd
        end subroutine
        
        subroutine GDALComputeRasterMinMax(bnd, approxOK, minmax)
            use GDAL_Ctypes
            integer(kind=C_PTR_KIND) bnd
            integer(kind=C_INT_KIND) approxOK
            real(kind=C_DOUBLE_KIND) minmax(2)
        end subroutine
        
        subroutine GDALGetRasterHistogram(bnd, min, max, buckets, histo,        &
                includeOutOfRange, approxOK)
            use GDAL_Ctypes
            integer(kind=C_PTR_KIND) bnd
            integer(kind=C_INT_KIND) buckets, histo(:), includeOutOfRange, approxOK
            real(kind=C_DOUBLE_KIND) min, max
        end subroutine
        
        subroutine GDALComputeBandStats(bnd, step, mean, stddev)
            use GDAL_Ctypes
            integer(kind=C_PTR_KIND) bnd
            integer(kind=C_INT_KIND) step
            real(kind=C_DOUBLE_KIND) mean, stddev
        end subroutine
    end interface
end module
