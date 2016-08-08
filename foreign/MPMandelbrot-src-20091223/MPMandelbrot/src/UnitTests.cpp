// Unit tests
// (c) EB Nov 2009

#include <QtCore/QDebug>
#include <QtGui/QApplication>
#include <math.h>
#include <time.h>
#ifdef Linux
#define _snprintf snprintf
#endif

#include <BealtoOpenCL.h>
#include "UnitTests.h"
#include "FPReal.h"
#include "Mandelbrot.h"

void report(const char * s,bool ok)
{
  if (ok)
    qDebug() << "ok:     " << s;
  else
    qDebug() << "FAILED: " << s;
}

// FPReal tests

// Random double in 0..1
double rnd()
{
  double x = 0;
  for (int i=0;i<4;i++)
  {
    x = (x+(rand() & 0xFFFF))/(double)0x10000;
  }
  return x;
}

bool testFPRealConversions()
{
  char aux[1000];
  std::string s;
  double sum;
  bool ok = true;

  sum = 0.0;
  for (int pass = 0;pass < 3 && ok;pass++)
  {
    for (int i=-1000;i<=1000 && ok;i++)
    {
      double x,y;
      FPReal<8> z;
      if (pass == 0) x = i;
      else if (pass == 1) x = (double)i/1000.0;
      else if (pass == 2) x = ldexp(rnd(),i-1000);
      z.set(x);
      y = (double)z;
      double u = fabs(x-y);
      if (u>1.0e-12)
      {
	z.toString(s);
	_snprintf(aux,1000,"%.15f %.15f %.6g %s",x,y,u,s.c_str());
	qDebug() << aux;
	ok = false;
      }
      sum += u;
    }
  }
  report("FPR conversions",ok);

  return ok;
}

bool testFPRealLog()
{
  bool ok = true;
  char aux[1000];
  std::string s;

  for (int i=FPReal<8>::Log2Min+2;i<=FPReal<8>::Log2Max && ok;i++)
  {
    FPReal<8> z;
    double x = ldexp(rnd(),i);
    while (x<0.5) x*=2.0; // Make sure X>=0.5, otherwisde test may fail for the smallest values of I
    z.set(x);
    int lg = z.logNorm();
    double y = ldexp(1.0,lg);
    if (x < y || x > 2.0*y)
    {
      z.toString(s);
      _snprintf(aux,1000,"%d %.15g %.15g %s",i,x,y,s.c_str());
      qDebug() << aux;
      ok = false;
    }
  }
  report("FPR logNorm",ok);

  return ok;
}

bool testFPRealAdd()
{
  bool ok = true;
  char aux[1000];
  std::string s;

  for (int pass=0;pass<8 && ok;pass++)
  {
    for (int i=FPReal<8>::Log2Min+40;i<=FPReal<8>::Log2Max && ok;i++)
      for (int j=FPReal<8>::Log2Min+40;j<=FPReal<8>::Log2Max && ok;j++)
      {
	double x,y,z,x2,zmax;

	x = ldexp(rnd(),i);
	if (pass&1) x = -x;
	y = ldexp(rnd(),j);
	if (pass&2) y = -y;

	FPReal<8> xx(x),yy(y);
	if (pass&4)
	{
	  xx.sub(yy);
	  z = (x-y);
	}
	else
	{
	  xx.add(yy);
	  z = (x+y);
	}
	x2 = (double)xx;
	z -= x2;
	zmax = ldexp(1.0e-10,std::max(i,j));
	if (fabs(z)>zmax)
	{
	  xx.toString(s);
	  _snprintf(aux,1000,"%d %d %.15g %s",i,j,z,s.c_str());
	  qDebug() << aux;
	  ok = false;
	}
      }
  }

  report("FPR add/sub",ok);
  return ok;
}

bool testFPRealParse()
{
  bool ok = true;
  char aux[1000];
  std::string s1,s2;

  FPReal<8> x,y;
  for (int i=0;i<100 && ok;i++)
  {
    double u = ldexp(rnd(),(int)(FPReal<8>::Log2Max+rnd()*FPReal<8>::Log2Min));
    x.set(u);
    x.toString(s1);
    y.set(s1.c_str());
    y.sub(x);
    double v = fabs((double)y); // Normally 0
    if (v > ldexp(1.0,FPReal<8>::Log2Min+5))
    {
      y.toString(s2);
      _snprintf(aux,1000,"%g %s %s",u,s1.c_str(),s2.c_str());
      qDebug() << aux;
      ok = false;
    }
  }
  report("FPR parse",ok);

  return ok;
}

bool testFPRealPi()
{
  bool ok = true;
  const char * PI_GROUND_TRUTH = "+3."
    "14159265358979323846264338327950288419716939937510582097494459230781640"
    "62862089986280348253421170679821480865132823066470938446095505822317253"
    "59408128481117450284102701938521105559644622948954930381964428810975665"
    "93344612847564823378678316527120190914564856692346034861045432664821339"
    "36072602491412737245870066063155881748815209209628292540917153643678925"
    "90360011330530548820466521384146951941511609433057270365759591953092186"
    "11738193261179310511854807446237996274956735188575272489122793818301194"
    "91298336733624406566430860213949463952247371907021798609437027705392171"
    "76293176752384674818467669405132000568127145263560827785771342757789609"
    "17363717872146844090122495343014654958537105079227968925892354201995611"
    "21290219608640344181598136297747713099605187072113499999983729780499510"
    "59731732816096318595024459455346908302642522308253344685035261931188171"
    "01000313783875288658753320838142061717766914730359825349042875546873115"
    "95628638823537875937519577818577805321712268066130019278766111959092164"
    "20198938095257201065485863278865936153381827968230301952035301852968995"
    "77362259941389124972177528347913151557485724245415069595082953311686172"
    "78558890750983817546374649393192550604009277016711390098488240128583616"
    "03563707660104710181942955596198946767837449448255379";

  // Compute Pi using John Machin's formula:
  // Pi/4 = 4*atan(1/5)-atan(1/239)
  const int N = 12;
  FPReal<N> pi(0);
  FPReal<N> term1(0),term2(0);
  for (int k=0; ;k++)
  {
    if (k == 0)
    {
      term1.set(16); term1.div(5);
      term2.set(4); term2.div(239);
    }
    else
    {
      term1.mul(2*k-1); term1.div(-5*5*(2*k+1));
      term2.mul(2*k-1); term2.div(-239*239*(2*k+1));
    }
    pi.add(term1);
    pi.sub(term2);

    if (term1.isZero()) break;

  }
  std::string s;
  pi.toString(s);
  size_t len = std::min(s.size() - 5,strlen(PI_GROUND_TRUTH)); // Allow error on the last digits
  ok = (strncmp(s.c_str(),PI_GROUND_TRUTH,len) == 0);
  // qDebug() << s.c_str();

  report("FPR pi",ok);
  return ok;
}

bool testFPRealMul()
{
  bool ok = true;

  // Compute sqrt2
  const int N = 12;
  FPReal<N> sqrt2(0);
  FPReal<N> term(0);
  std::string s;
  for (int k=0; ;k++)
  {
    if (k == 0)
    {
      term.set(1); term.mul2k(-1); // 1/2
    }
    else
    {
      term.mul((2*k+1)*2*k);
      term.div(8*k*k);
    }
    sqrt2.add(term);
    if (term.isZero()) break;
  }
  FPReal<N> z;
  mul(z,sqrt2,sqrt2);
  term.set(2);
  z.sub(term); // sqrt(2)^2-2 = 0
  // z.toString(s);
  // qDebug() << s.c_str();
  ok = (z.logNorm() < FPReal<N>::Log2Min + 32); // Allow last 32-bits wrong

  report("FPR mul",ok);
  return ok;
}

// Test C fixed-point reals
bool runFPRealTests()
{
  bool ok = true;
  ok &= testFPRealConversions();
  ok &= testFPRealLog();
  ok &= testFPRealAdd();
  ok &= testFPRealParse();
  ok &= testFPRealPi();
  ok &= testFPRealMul();
  return ok;
}

// Test OpenCL fp128
bool runFP128OpenCLTests()
{
  bool ok = true;
  cl::Context * c = 0;
  cl::Program * p = 0;
  cl::CommandQueue * q = 0;
  cl::Kernel * k = 0;
  cl::Buffer * buf = 0;
  const int n = 16; // Number of uint to alloc in buffer
  size_t bufSize = n*sizeof(cl_uint); // bytes
  std::string buildErrors;
  std::string s;
  cl_uint * xBuf;
  const int nIt = 100000;
  double t0;
  QString f = QApplication::applicationDirPath() + "/fp128.cl";

  c = cl::Context::create(CL_DEVICE_TYPE_GPU);
  if (c == 0) { ok = false; goto END; }
  p = c->createProgramWithFile(f.toAscii().data());
  if (p == 0) { ok = false; goto END; }
  ok &= p->build("-DUNIT_TESTS=1",buildErrors);
  if (!ok) { qDebug() << buildErrors.c_str(); goto END; }
  q = c->createCommandQueue(0,0);
  if (q == 0) { ok = false; goto END; }
  k = p->createKernel("test_fp128");
  if (k == 0) { ok = false; goto END; }
  buf = c->createBuffer(CL_MEM_READ_ONLY,bufSize,0);
  if (buf == 0) { ok = false; goto END; }

  // Run one kernel
  k->setArg(0,buf);
  k->setArg(1,nIt);
  t0 = -Mandelbrot::realTime();
  q->execKernel1(k,1,1);
  q->finish();
  t0 += Mandelbrot::realTime(); t0 = t0*1.0e6/(double)nIt;
  qDebug() << "Time/iteration: " << t0 << "us";
  // Display contents of buffer
  if (!q->mapBuffer(buf,xBuf,true,CL_MAP_READ).isValid()) { ok = false; goto END; }
  for (int i=0;i<n;i++)
  {
    char aux[30];
    _snprintf(aux,30," %08X",xBuf[i]);
    s.append(aux);
    if ( (i&3) == 3 ) s.append("\n");
  }
  qDebug() << s.c_str();
  q->unmapMemoryObject(buf,xBuf);

END:
  // Cleanup
  delete buf;
  delete k;
  delete q;
  delete p;
  delete c;

  report("OpenCL fp128",ok);

  return ok;
}

bool runTests()
{
  bool ok = true;
  srand(time(0));
  // ok &= runFPRealTests();
  ok &= runFP128OpenCLTests();
  return ok;
}
