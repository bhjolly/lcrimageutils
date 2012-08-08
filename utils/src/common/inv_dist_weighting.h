#ifndef INV_DIST_WEIGHTING_H
#define INV_DIST_WEIGHTING_H

typedef struct
{
  float x, y;
} SIDWInfo;

float* idw_findWeights( SIDWInfo *pInfo, int numpoints, float pointx, float pointy, float N );

float* idw_findWeights_with_dist( float *dists, int numpoints, float N );

float idw_applyWeights( float *weights, float *values, int numpoints );

#endif //INV_DIST_WEIGHTING_H
