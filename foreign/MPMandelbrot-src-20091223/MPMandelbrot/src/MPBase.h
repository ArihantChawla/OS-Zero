// Basic multiprecision functions
// (c) EB Dec 2009

#ifndef MPBase_h
#define MPBase_h

#include <string.h>

#ifdef WIN32
typedef unsigned __int32 uint32;
typedef unsigned __int64 uint64;
#endif
#ifdef Linux
typedef unsigned int uint32;
typedef unsigned long long uint64;
#endif

// Zero unsigned array of N words.
inline void zeroWords(int n,uint32 * z)
{
  memset(z,0,n*sizeof(z[0]));
}

// Copy unsigned array of N words.
inline void copyWords(int n,uint32 * z,const uint32 * x)
{
  memcpy(z,x,n*sizeof(z[0]));
}

// Add unsigned arrays of N words.
// CARRY is input carry.
// Return value is output carry.
inline uint32 addWords(int n,uint32 * z,const uint32 * x,uint32 carry)
{
  uint64 c = carry;
  for (int i=n-1;i>=0;i--)
  {
    c += (uint64)z[i] + (uint64)x[i];
    z[i] = (uint32)(c & 0xFFFFFFFFULL);
    c >>= 32;
  }
  return (uint32)c;
}

// Sub unsigned arrays of N words.
// BORROW is input borrow.
// Return value is output borrow.
inline uint32 subWords(int n,uint32 * z,const uint32 * x,uint32 borrow)
{
  uint64 c = borrow;
  for (int i=n-1;i>=0;i--)
  {
    uint64 y = (uint64)z[i] - (uint64)x[i] - c;
    z[i] = (uint32)(y & 0xFFFFFFFFULL);
    c = (y>=0x100000000)?1:0;
  }
  return (uint32)c;
}

// Mul unsigned arrays of N words.
// CARRY is input carry.
// Return value is output carry.
inline uint32 mulWords(int n,uint32 k,uint32 * z,uint32 carry)
{
  uint64 c = carry;
  for (int i=n-1;i>=0;i--)
  {
    uint64 y = (uint64)z[i] * (uint64)k + (uint64)c;
    z[i] = (uint32)(y & 0xFFFFFFFFULL);
    c = y >> (uint64)32;
  }
  return (uint32)c;
}

// Div unsigned arrays of N words.
// REM is input remainder (must be <K).
// Return value is output remainder.
inline uint32 divWords(int n,uint32 k,uint32 * z,uint32 rem)
{
  uint64 c = rem;
  for (int i=0;i<n;i++)
  {
    uint64 y = (c<<(uint64)32) | (uint64)z[i];
    z[i] = (uint32)(y/(uint64)k);
    c = y % (uint64)k;
  }
  return (uint32)c;
}

// Shift left N words by K bits (0<K<32).
// BITS are input bits (K bits).
// Return value are output bits (K bits).
inline uint32 shlWords(int n,uint32 k,uint32 * z,uint32 bits)
{
  uint64 mask = ((uint64)1<<(uint64)k)-1;
  uint64 c = (uint64)bits & mask;
  for (int i=n-1;i>=0;i--)
  {
    uint64 y = ((uint64)z[i]<<(uint64)k) | c;
    z[i] = (uint32)(y & 0xFFFFFFFFULL);
    c = y >> (uint64)32;
  }
  return (uint32)c;
}

// Shift right N words by K bits (0<K<32).
// BITS are input bits (K bits).
// Return value are output bits (K bits).
inline uint32 shrWords(int n,uint32 k,uint32 * z,uint32 bits)
{
  uint64 mask = ((uint64)1<<(uint64)k)-1;
  uint64 c = (uint64)bits & mask;
  for (int i=0;i<n;i++)
  {
    uint64 y = (c<<(uint64)32) | (uint64)z[i];
    z[i] = (uint32)(y>>(uint64)k);
    c = y & mask;
  }
  return (uint32)c;
}

// Compare unsigned arrays of N words.
inline int cmpWords(int n,const uint32 * x,const uint32 * y)
{
  for (int i=0;i<n;i++)
  {
    if (x[i]>y[i]) return 1;
    if (x[i]<y[i]) return -1;
  }
  return 0;
}

// Check if all N words of X are 0
inline bool checkZeroWords(int n,const uint32 * x)
{
  for (int i=0;i<n;i++)
  {
    if (x[i] != 0) return false;
  }
  return true;
}

// Get index of most significant set bit in word X, or -1 if 0.
inline int msbWord(uint32 x)
{
  if (x == 0) return -1;
  int m = 0;
  if (x & 0xFFFF0000) { m |= 16; x >>= 16; }
  if (x & 0xFF00) { m |= 8; x >>= 8; }
  if (x & 0xF0) { m |= 4; x >>= 4; }
  if (x & 0xC) { m |= 2; x >>= 2; }
  if (x & 0x2) m |= 1;
  return m;
}

// Get index of most significant set bit in N words, or -1 if 0.
inline int msbWords(int n,const uint32 * x)
{
  for (int i=0;i<n;i++)
  {
    int m = msbWord(x[i]);
    if (m >= 0) { return m+((n-i-1)<<5); }
  }
  return -1;
}

#endif // #ifndef MPBase_h
