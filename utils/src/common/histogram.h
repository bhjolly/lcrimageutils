#ifndef HISTOGRAM_H
#define HISTOGRAM_H

class CHistogram
{
public: 
	CHistogram( double dMin, double dMax, int nBins );
	~CHistogram();
	
	void addValue( double dVal );

	double getStep() const
	{
		return m_dStep;
	}
	int getNBins() const
	{
	  return m_nBins;
	}
	double getMin() const
	{
	  return m_dMin;
	}
	double getMax() const
	{
	  return m_dMax;
	}
	int getBinCount(int bin) const
	{
	  return m_pBinData[bin];
	}
	double getValueForBin(int bin) const
	{
	  return m_dMin + ( (double)bin * m_dStep );
	}
	int getBinForValue(double dValue) const
	{
		return int( ( dValue - m_dMin ) / m_dStep);
	}
  int getNValuesAdded() const
  {
    return m_nValuesAdded;
  }
	
	void 		printBin() const;
	void 		makeCumulative();
	double 	findPercentile( double dPercentile ) const;
	int     getLargestBin() const;
		
protected:
	double 	m_dMin;
	double	m_dMax;
	double	m_dStep;
	int			m_nBins;
	int		 *m_pBinData;
  int     m_nValuesAdded;
};

#endif
