// Multiprecision base functions
// (c) EB Dec 2009

const ulong WordMask = 0x00000000FFFFFFFFUL;

// Zero unsigned array of N words.
void zeroWords(int n,uint * z)
{
  for (int i=n-1;i>=0;i--) z[i] = 0;
}

// Copy unsigned array of N words.
void copyWords(int n,uint * z,const uint * x)
{
  for (int i=n-1;i>=0;i--) z[i] = x[i];
}

// Add unsigned arrays of N words.
// CARRY is input carry.
// Return value is output carry.
uint addWords(int n,uint * z,const uint * x,uint carry)
{
  ulong c = carry;
  for (int i=n-1;i>=0;i--)
  {
    c += (ulong)z[i] + (ulong)x[i];
    z[i] = (uint)(c & WordMask);
    c >>= 32;
  }
  return (uint)c;
}

// Sub unsigned arrays of N words.
// BORROW is input borrow.
// Return value is output borrow.
uint subWords(int n,uint * z,const uint * x,uint borrow)
{
  ulong c = borrow;
  for (int i=n-1;i>=0;i--)
  {
    ulong y = (ulong)z[i] - (ulong)x[i] - c;
    z[i] = (uint)(y & WordMask);
    c = (y>=0x100000000)?1:0;
  }
  return (uint)c;
}

// Mul unsigned arrays of N words.
// CARRY is input carry.
// Return value is output carry.
uint mulWords(int n,uint k,uint * z,uint carry)
{
  ulong c = carry;
  for (int i=n-1;i>=0;i--)
  {
    ulong y = (ulong)z[i] * (ulong)k + (ulong)c;
    z[i] = (uint)(y & WordMask);
    c = y >> (ulong)32;
  }
  return (uint)c;
}

// Div unsigned arrays of N words.
// REM is input remainder (must be <K).
// Return value is output remainder.
uint divWords(int n,uint k,uint * z,uint rem)
{
  ulong c = rem;
  for (int i=0;i<n;i++)
  {
    ulong y = (c<<(ulong)32) | (ulong)z[i];
    z[i] = (uint)(y/(ulong)k);
    c = y % (ulong)k;
  }
  return (uint)c;
}

// Shift left N words by K bits (0<K<32).
// BITS are input bits (K bits).
// Return value are output bits (K bits).
uint shlWords(int n,uint k,uint * z,uint bits)
{
  ulong mask = ((ulong)1<<(ulong)k)-1;
  ulong c = (ulong)bits & mask;
  for (int i=n-1;i>=0;i--)
  {
    ulong y = ((ulong)z[i]<<(ulong)k) | c;
    z[i] = (uint)(y & WordMask);
    c = y >> (ulong)32;
  }
  return (uint)c;
}

// Shift right N words by K bits (0<K<32).
// BITS are input bits (K bits).
// Return value are output bits (K bits).
uint shrWords(int n,uint k,uint * z,uint bits)
{
  ulong mask = ((ulong)1<<(ulong)k)-1;
  ulong c = (ulong)bits & mask;
  for (int i=0;i<n;i++)
  {
    ulong y = (c<<(ulong)32) | (ulong)z[i];
    z[i] = (uint)(y>>(ulong)k);
    c = y & mask;
  }
  return (uint)c;
}

// Compare unsigned arrays of N words.
int cmpWords(int n,const uint * x,const uint * y)
{
  for (int i=0;i<n;i++)
  {
    if (x[i]>y[i]) return 1;
    if (x[i]<y[i]) return -1;
  }
  return 0;
}

// Check if all N words of X are 0
bool checkZeroWords(int n,const uint * x)
{
  for (int i=0;i<n;i++)
  {
    if (x[i] != 0) return false;
  }
  return true;
}

// Get index of most significant set bit in word X, or -1 if 0.
int msbWord(uint x)
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
int msbWords(int n,const uint * x)
{
  for (int i=0;i<n;i++)
  {
    int m = msbWord(x[i]);
    if (m >= 0) { return m+((n-i-1)<<5); }
  }
  return -1;
}
