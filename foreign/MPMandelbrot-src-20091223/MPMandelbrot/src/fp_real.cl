// Fixed-point reals
// (c) EB Dec 2009

#ifndef FPWords  // compile with -DFPWords=<value>
#define FPWords 4 // default value if undefined
#endif

// fp reals are stored on FPWords uint + one int for the sign,
// the first word m[0] is the integral part. The last word is
// the least significant word.
typedef struct sFPReal
{
  int sign;
  uint m[FPWords];
} FPReal;

// Set Z to 0
void set0(FPReal * z)
{
  zeroWords(FPWords,z->m);
  z->sign = 0;
}

// Set Z to signed integer X
void seti(FPReal * z,int x)
{
  zeroWords(FPWords,z->m);
  if (x == 0) { z->sign = 0; return; }
  if (x > 0) { z->sign = 1; z->m[0] = (uint)x; }
  else { z->sign = -1; z->m[0] = (uint)(-x); }
}

// Set Z to fp real X
void setfp(FPReal * z,const FPReal * x)
{
  if (z == x) return;
  z->sign = x->sign;
  copyWords(FPWords,z->m,x->m);
}

// Internal function: check if all digits of Z are 0,
// and if they are set Z sign to 0.  This function is
// called to update the sign after an operation which
// may have a 0 result.
void checkfp(FPReal * z)
{
  if (z->sign == 0) return; // Already 0
  if (checkZeroWords(FPWords,z->m)) z->sign = 0;
}

// Negate Z
void negfp(FPReal * z)
{
  z->sign = -z->sign;
}

// Z += X
void addfp(FPReal * z,const FPReal * x)
{
  if (x->sign == 0) return; // +0
  if (z->sign == 0) { setfp(z,x); return; } // 0+X

  // Same sign
  if (z->sign == x->sign)
  {
    addWords(FPWords,z->m,x->m,0);
    return;
  }

  // Different signs, need to subtract
  int u = cmpWords(FPWords,z->m,x->m);
  if (u>=0)
  {
    // |Z| >= |X|
    subWords(FPWords,z->m,x->m,0);
  }
  else
  {
    // |X| > [Z|
    uint y[FPWords];
    copyWords(FPWords,y,z->m);
    setfp(z,x); // We get the sign of X
    subWords(FPWords,z->m,y,0);
  }
  checkfp(z);
}

// Z -= X
void subfp(FPReal * z,const FPReal * x)
{
  // Implemented as Z += (-X) for simplicity
  FPReal y;
  setfp(&y,x);
  negfp(&y);
  addfp(z,&y);
}

// Z *= K
void mulk(FPReal * z,int k)
{
  if (k == 0) { set0(z); return; } // Z *= 0
  if (z->sign == 0) return; // Z = 0*K
  if (k < 0) { negfp(z); k = -k; }
  if (k == 1) return; // K was 1 or -1
  mulWords(FPWords,k,z->m,0);
}

// Z *= 2^K
void mul2k(FPReal * z,int k)
{
  if (k == 0) return; // Z*1
  if (z->sign == 0) return; // 0*2^K
  if (k > 0) shlWords(FPWords,k,z->m,0);
  else
  {
    shrWords(FPWords,-k,z->m,0);
    checkfp(z);
  }
}

// Z = X*Y (truncated to FPWords)
void mulfp(FPReal * z,const FPReal * x,const FPReal * y)
{
  if (x->sign == 0 || y->sign == 0) { set0(z); return; }
  z->sign = x->sign * y->sign;

  // Multiply (the trivial way) and accumulate in AUX
  uint64 aux[FPWords];
  for (int i=0;i<FPWords;i++) aux[i] = 0;
  for (int i=0;i<FPWords;i++) for (int j=0;j<FPWords;j++)
  {
    int k = i+j;
    if (k > FPWords) continue; // ignored
    uint64 u1 = (uint64)(x->m[i]) * (uint64)(y->m[j]);
    uint64 u0 = u1 & WordMask; // lower 32 bits, index K
    u1 >>= (uint64)32; // higher 32 bits, index K-1
    if (k < FPWords) aux[k] += u0;
    if (k > 0) aux[k-1] += u1;
  }
  // Propagate carry into result
  uint64 c = 0;
  for (int i=FPWords-1;i>=0;i--)
  {
    c += aux[i];
    z->m[i] = (uint)(c & WordMask);
    c >>= (uint64)32;
  }
}

// Convert to float (approx for debug)
float toFloat(FPReal * z)
{
  if (z->sign == 0) return (float)0;
  return (float)(z->sign) * ((float)(z->m[0])+1.0e-32f*(float)(z->m[1]));
}
