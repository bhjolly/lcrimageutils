#ifndef GINFO_H
#define GINFO_H

typedef enum
{
    EIMG_THEMATIC_LAYER = 0,
    EIMG_ATHEMATIC_LAYER,
    EIMG_REALFFT_LAYER
}
Eimg_LayerType;

typedef struct
{
  double minimum; /* Minimum value */
  double maximum; /* Maximum value */
  double mean;    /* Mean value */
  double median;  /* Median value */
  double mode;    /* Mode value */
  double stddev;  /* Standard deviation */
}
Esta_Statistics;

typedef struct
{
  double x; /* coordinate x-value */
  double y; /* coordinate y-value */
}
Eprj_Coordinate;

typedef struct
{
  double width;  /* pixelsize width */
  double height; /* pixelsize height */
}
Eprj_Size;

typedef struct {
  char *proName;                      /* projection name */
  Eprj_Coordinate *upperLeftCenter;   /* map coordinates of center of upper left pixel */
  Eprj_Coordinate *lowerRightCenter;  /* map coordinates of center of lower right pixel */
  Eprj_Size *pixelSize;               /* pixel size in map units */
  char *units;                        /* units of the map */
}
Eprj_MapInfo;

#endif
