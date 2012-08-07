#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "histogram.h"

CHistogram::CHistogram( double dMin, double dMax, int nBins )
{
	m_dMin 			= dMin;
	m_dMax			= dMax;	
	m_nBins			= nBins;
	m_dStep			= ( m_dMax - m_dMin ) / double( m_nBins );
	m_pBinData	= (int*)calloc(m_nBins, sizeof( int ));
  m_nValuesAdded = 0;
}

CHistogram::~CHistogram()
{
	free( m_pBinData );
}

void CHistogram::addValue( double dVal )
{
	assert( dVal <= m_dMax );
	dVal -= m_dMin;	// take away the min value so we can put in the right bin.
	int nBin = int( dVal / m_dStep );
	if( nBin >= m_nBins )
	{
		// hack for dVal being equal or close to m_dMax.
		nBin = m_nBins - 1;
	}
	assert( nBin < m_nBins );
	m_pBinData[ nBin ]++;
  m_nValuesAdded++;
}

void CHistogram::printBin() const
{
	for( int nCount = 0; nCount < m_nBins; nCount++ )
	{
		printf( "%d\n", m_pBinData[ nCount ] );
	}
}

void CHistogram::makeCumulative()
{
	for( int nCount = 1; nCount < m_nBins; nCount++ )
	{
		m_pBinData[ nCount ] += m_pBinData[ nCount - 1 ];
	}
}

double CHistogram::findPercentile( double dPercentile ) const
{
	// Find the frequency relating to the dPercentile.
	int nFreq = int( double( m_pBinData[ m_nBins - 1 ] ) * dPercentile );
	
	// Find where the frequency intersects with the culumulative histo.
	int nLastFreq = 0;
	int nCount;
	for( nCount = 0; ( nCount < m_nBins ) && ( nLastFreq < nFreq ); nCount++ )
	{
		nLastFreq = m_pBinData[ nCount ];
	}

  if( nCount >= 2 )
  {
  	int nVal1 = m_pBinData[ nCount - 2 ];
  	int nVal2 = m_pBinData[ nCount - 1 ];
  	
  	// which is closer?
  	if( abs( nFreq - nVal1 ) < abs( nFreq - nVal2 ) )
  	{
  		// nVal1 is.
  		nCount -= 2;
  	}
  	else
  	{
  		// nVal2 is.
  		nCount--;
  	}
  }	
	
	return m_dMin + ( double( nCount ) * m_dStep );
}

// Finds the bin with the largest number of items
int CHistogram::getLargestBin() const
{
	
	int maxbin = 0;
	int maxvals = m_pBinData[maxbin];
	
	for( int nCount = 1; nCount < m_nBins; nCount++ )
	{
		if( m_pBinData[nCount] > maxvals )
		{
			maxvals = m_pBinData[nCount];
			maxbin = nCount;
		}
	}
	
	return maxbin;
}

