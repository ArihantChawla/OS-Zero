// Fixed point reals
// (c) EB Nov 2009

#ifndef FPReal_h
#define FPReal_h

#include <string>
#include <math.h>
#include <limits.h>
#ifdef Linux
#define _snprintf snprintf
#endif
#include "MPBase.h"

// Generic implementation of fixed-point reals stored on N 32-bit words.
// The first word is the integer part.

template <int N> class FPReal
{
public:

  // Constants and types
  static const int Size = N; // Number of words
  static const int Log2Min = -((N-1)<<5); // Log2 of the least significant bit of a FPReal
  static const int Log2Max = 31; // Log2 of the most significant bit of a FPReal
  typedef uint32 word_t; // From MPBase

  // Constructor, initialized to 0.
  FPReal() { sign = 0; }

  // Constructors, initialized to X.
  FPReal(int x) { set(x); }
  FPReal(double x) { set(x); }
  FPReal(const FPReal & x) { set(x); }
  FPReal(const char * x) { if (!set(x)) set(0); }
  template <int M> FPReal(const FPReal<M> & x) { set(x); }

  // = operator
  FPReal & operator = (const FPReal & x)
  {
    if (&x != this) set(x);
    return *this;
  }

  // Set to 0
  void zero() { sign = 0; }

  // Set to X
  void set(int x)
  {
    if (x == 0) { zero(); return; }
    zeroWords(N,m);
    if (x < 0) { sign = -1; m[0] = -x; }
    else { sign = 1; m[0] = x; }
  }
  void set(double x)
  {
    if (x == 0) { sign = 0; return; }
    double p = ldexp(1.0,32); // 2^32
    if (x < 0) { x = -x; sign = -1; }
    else sign = 1;
    for (int i=0;i<N;i++)
    {
      double h = floor(x);
      m[i] = (word_t)h;
      x = (x-h)*p;
    }
    checkZero();
  }
  void set(const FPReal & x)
  {
    sign = x.sign;
    if (x.sign == 0) return;
    copyWords(N,m,x.m);
  }
  template <int M> void set(const FPReal<M> & x)
  {
    sign = x.sgn();
    if (sign == 0) return;
    const word_t * xm = x.words();
    if (M >= N) copyWords(N,m,xm);
    else
    {
      // N > M, we need to pad with 0
      zeroWords(N-M,m+M);
      copyWords(M,m,xm);
    }
  }
  bool set(const char * x)
  {
    if (x == 0) return false; // Invalid
    // Skip spaces
    while ((*x != 0) && isspace(*x)) x++;
    if (*x == 0) return false; // Empty
    // Process sign
    sign = 1;
    if (*x == '+') { sign = 1; x++; }
    else if (*x == '-') { sign = -1; x++; }
    // Forward pass
    word_t intPart = 0;
    const char * dotPos = 0;
    const char * endPos = 0;
    const char * s;
    for (s=x; ;s++)
    {
      char c = *s;
      if (isdigit(c))
      {
	if (dotPos == 0) intPart = intPart * 10 + (int)(c - '0');
	continue;
      }
      if (c == '.')
      {
	if (dotPos == 0) dotPos = s;
	continue;
      }
      endPos = s; break; // First invalid char
    }
    // No frac part?
    if (dotPos == 0)
    {
      set(sign*(int)intPart);
      return true; // OK
    }
    // Parse frac part
    zeroWords(N,m);
    int p = 1;
    int value = 0;
    for (s=endPos-1;s>=dotPos;s--)
    {
      char c = *s;
      if (isdigit(c)) { value += p*(int)(c-'0'); p *= 10; }
      if (p >= 100000 || s == dotPos)
      {
	// Insert in number
	m[0] = value;
	divWords(N,p,m,0);
	value = 0; p = 1;
      }
    }
    m[0] = intPart;
    checkZero();
    return true;
  }

  // Conversions

  // Return the nearest integer
  int round() const
  {
    if (sign == 0) return 0;
    int u = m[0];
    if (m[1] >= 0x80000000U) u++;
    return (sign>0)?u:-u;
  }
  // Return integer part
  int intPart() const
  {
    if (sign == 0) return 0;
    return sign * m[0];
  }
  // Convert to 'double'
  operator double () const { return toDouble(); }
  double toDouble() const
  {
    if (sign == 0) return 0; // OK
    double u = 0;
    for (int i=0;i<N;i++) u += ldexp((double)m[i],-(i<<5));
    return sign * u;
  }
  // Convert to decimal string. S cleared by the call.
  void toString(std::string & s) const
  {
    if (sign == 0) { s.assign("0"); return; }
    char aux[100];
    s.clear();
    s.reserve(8+10*N); // We have 9.632 decimal digits per word
    s.append((sign < 0)?"-":"+");
    word_t z[N];
    copyWords(N,z,m);
    int bits = 32 * N; // Bits to process
    // Integer part
    _snprintf(aux,100,"%d",z[0]);
    s.append(aux); s.append(".");
    bits -= 32;
    // Fractional part
    while (bits > 0)
    {
      z[0] = 0;
      mulWords(N,100000,z,0);
      _snprintf(aux,100,"%05d",z[0]);
      s.append(aux);
      bits -= 16; // we processed 16.61 bits
    }
  }
  // Copy M words of THIS into X[M]. Truncate or copy additional 0 when needed.
  void getWords(int M,uint32 * x) const
  {
    copyWords(std::min(M,N),x,m);
    if (M > N) zeroWords(M-N,x+N); // Pad extra output with 0
  }
  // Return a pointer to the words (needed for template set from FPReal<M>).
  const word_t * words() const { return m; }

  // Comparison operators
  friend bool operator < (const FPReal & x,const FPReal & y) { return (cmp(x,y)<0); }
  friend bool operator > (const FPReal & x,const FPReal & y) { return (cmp(x,y)>0); }
  friend bool operator == (const FPReal & x,const FPReal & y) { return (cmp(x,y)==0); }
  bool isZero() const { return (sign == 0); }
  friend int cmp(const FPReal & x,const FPReal & y)
  {
    if (x.sign < y.sign) return -1;
    if (x.sign > y.sign) return 1;
    // Here both signs are equal
    if (x.sign == 0) return 0; // Both 0
    if (x.sign > 0) return cmpWords(N,x.m,y.m);
    return cmpWords(N,y.m,x.m);
  }
  // Return K such that |THIS| is in 2^K,2^(K+1).
  // Return INT_MIN if THIS is 0.
  int logNorm() const
  {
    if (sign == 0) return INT_MIN;
    int p = msbWords(N,m);
    if (p < 0) return INT_MIN;
    return p + Log2Min;
  }
  // Return the sign of THIS (-1,0,+1)
  int sgn() const { return sign; }

  // Arithmetic operators

  // Negate THIS
  void neg() { sign = -sign; }

  // THIS = |THIS|
  void abs() { if (sign < 0) sign = 1; }

  // THIS += X
  void add(const FPReal & x)
  {
    if (x.sign == 0) return; // +0
    if (sign == 0) { set(x); return; } // 0+X

    // Same sign
    if (sign == x.sign)
    {
      addWords(N,m,x.m,0);
      return;
    }

    // Different signs
    int u = cmpWords(N,m,x.m);
    if (u>=0)
    {
      // |Z| >= |X|
      subWords(N,m,x.m,0);
    }
    else
    {
      // |X| > [Z|
      word_t y[N];
      copyWords(N,y,m);
      set(x); // We get the sign of X
      subWords(N,m,y,0);
    }
    checkZero();
  }

  // THIS -= X
  void sub(const FPReal & x)
  {
    FPReal<N> y(x);
    y.neg();
    add(y);
  }

  // THIS *= K
  void mul(int k)
  {
    if (k == 0) { zero(); return; } // Z *= 0
    if (sign == 0) return; // Z = 0*K
    if (k<0) { neg(); k = -k; }
    if (k == 1) return; // K was 1 or -1
    mulWords(N,k,m,0);
  }

  // THIS /= K
  void div(int k)
  {
    if (k == 0) return; // DIV0!
    if (sign == 0) return; // 0/K
    if (k<0) { neg(); k = -k; }
    if (k == 1) return; // K was 1 or -1
    divWords(N,k,m,0);
    checkZero();
  }

  // THIS *= 2^K
  // K may be negative, but is limited to -31,+31
  void mul2k(int k)
  {
    if (k == 0) return; // Z*1
    if (sign == 0) return; // 0*2^K
    if (k>0) shlWords(N,k,m,0);
    else
    {
      shrWords(N,-k,m,0);
      checkZero();
    }
  }

  // Z = X * Y (truncated to N words)
  friend void mul(FPReal & z,const FPReal & x,const FPReal & y)
  {
    if (x.sign == 0 || y.sign == 0) { z.zero(); return; }
    z.sign = x.sign * y.sign;

    // Multiply (the trivial way) and accumulate in AUX
    uint64 aux[N];
    memset(aux,0,N*sizeof(aux[0]));
    for (int i=0;i<N;i++) for (int j=0;j<N;j++)
    {
      int k = i+j;
      if (k > N) continue; // ignored
      uint64 u1 = (uint64)(x.m[i]) * (uint64)(y.m[j]);
      uint64 u0 = u1 & 0xFFFFFFFFULL; // lower 32 bits, index K
      u1 >>= (uint64)32; // higher 32 bits, index K-1
      if (k < N) aux[k] += u0;
      if (k > 0) aux[k-1] += u1;
    }
    // Propagate carry into result
    uint64 c = 0;
    for (int i=N-1;i>=0;i--)
    {
      c += aux[i];
      z.m[i] = (uint32)(c & 0xFFFFFFFFULL);
      c >>= (uint64)32;
    }
  }

private:

  // Check if all words are 0, and then set sign to 0
  void checkZero()
  {
    if (sign == 0) return; // Already 0
    if (checkZeroWords(N,m)) sign = 0;
  }

  // Sign and digits
  int sign;
  word_t m[N];
};

#endif // #ifndef FPReal_h
