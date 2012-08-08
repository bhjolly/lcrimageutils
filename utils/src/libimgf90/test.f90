program test

  ! Access the GDAL functions and subroutines
  use libimgf90
  
  implicit none
  
  ! declare the 'pointers'  to the datasets and bands
  integer(kind=C_PTR_KIND) ds, bnd, outds, outbnd
  ! declare an array that will hold the data
  integer*1, allocatable :: buffer(:,:)
  ! normal integer return types
  integer(kind=C_INT_KIND) xsize,ysize
  
  ! You must call this before doing anything with GDAL
  call GDALAllRegister()
  
  ! Open test.img in readonly mode
  ds = GDALOpen("test.img",GA_ReadOnly)

  ! get the first band in the dataset
  bnd = GDALGetRasterBand(ds,1)

  ! get the size of the raster
  xsize = GDALGetRasterXSize(ds)
  ysize = GDALGetRasterYSize(ds)
  
  ! allocate enough memory to hold one band of the image
  allocate( buffer(xsize,ysize) )

  ! Create a new image based on the existing, but 
  ! only has one band.
  outds = GDALCreate(ds,"out.img",0,0,1,GDT_Unknown);
  
  ! get the first band of the output image
  outbnd = GDALGetRasterBand(outds,1)
  
  ! read in the whole of the layer 
  call GDALRasterIO(bnd,GF_Read,0,0,xsize,ysize,buffer,xsize,ysize,GDT_Byte,0,0);
  
  ! write the layer to the output image
  call GDALRasterIO(outbnd,GF_Write,0,0,xsize,ysize,buffer,xsize,ysize,GDT_Byte,0,0);
  
  ! change the layer name to 'bob'
  call GDALSetDescription(outbnd,"bob")

  ! close the datasets - note not the bands
  call GDALClose(ds)
  call GDALClose(outds)

  ! calculate stats and pyramid layers ignoring 0
  call GDALCalcStatsIgnore("out.img",0)
  
  ! free the memory allocated
  deallocate( buffer )
end program
