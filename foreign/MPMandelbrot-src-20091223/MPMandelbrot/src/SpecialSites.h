// Special sites in Mandelbrot Set
// (c) Eric Bainville Dec 2009

#ifndef SpecialSites_h
#define SpecialSites_h

const int NB_SPECIAL_SITES = 8;

// Special site data
struct SpecialSiteData
{
  const char * name;
  const char * x;
  const char * y;
  const char * step;
  int maxIt;
};

extern SpecialSiteData SpecialSites[NB_SPECIAL_SITES];

// Get name of special site. V is in 0..NB_SPECIAL_SITES-1.
inline const char * getSpecialSiteName(int v)
{
  return SpecialSites[v].name;
}


#endif // #ifndef SpecialSites_h

