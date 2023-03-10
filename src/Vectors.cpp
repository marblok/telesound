//---------------------------------------------------------------------------
//#include <vcl\vcl.h>
//#pragma hdrstop

#include "Vectors.h"
//---------------------------------------------------------------------------
M_complex::M_complex(void)
{ re=0; im=0; }

M_complex::M_complex(Tfloat r)
{ re=r; im=0; }

M_complex::M_complex(Tfloat r, Tfloat i)
{ re=r; im=i; }

Tfloat M_complex::real(void)
{ return re; };

Tfloat real(const M_complex &C)
{ return C.re; }

M_complex conj(const M_complex &C)
{return M_complex(C.re, -C.im);}


Tfloat M_complex::imag(void)
{ return im; };

Tfloat imag(const M_complex &C)
{ return C.im; }

Tfloat abs2(const M_complex &C)
{ return (C.re*C.re+C.im*C.im); }

Tfloat abs(const M_complex &C)
{ return sqrt(C.re*C.re+C.im*C.im); }

//Tfloat abs(M_complex C)
//{ return sqrt(C.re*C.re+C.im*C.im); }

M_complex M_complex::operator *(M_complex &C)
{ return M_complex (C.re*re-C.im*im, C.re*im+C.im*re); }

M_complex M_complex::operator *(Tfloat &scalar)
{ return M_complex (scalar*re, scalar*im); }

M_complex M_complex::operator /(Tfloat scalar)
{ return M_complex (re/scalar, im/scalar); }

//M_complex M_complex::operator /(const Tfloat &scalar)
//{ return M_complex (re/scalar, im/scalar); }

M_complex M_complex::operator /(DWORD &scalar)
{ return M_complex (re/scalar, im/scalar); }

//M_complex M_complex::operator /(const DWORD &scalar)
//{ return M_complex (re/scalar, im/scalar); }

M_complex operator *(Tfloat &scalar, M_complex &A)
{ // DO NOT MODIFY THE ORIGINAL
  return M_complex (scalar*A.re, scalar*A.im);
}

M_complex M_complex::operator +(const M_complex &A)
{ return M_complex(re+A.re,im+A.im); }

M_complex M_complex::operator -(const M_complex &A)
{ return M_complex(re-A.re,im-A.im); }

void M_complex::operator +=(const M_complex &A)
{ re+=A.re; im+=A.im; }

void M_complex::operator *=(const M_complex &A)
{ Tfloat temp;
  temp=re; re*=A.re;  re-=im*A.im;
  im*=A.re; im+=(temp*A.im); }

void M_complex::operator -=(const M_complex &A)
{ re-=A.re; im-=A.im; }

void M_complex::operator /=(Tfloat &A)
{ re/=A; im/=A; }

void M_complex::operator /=(DWORD &A)
{ re/=A; im/=A; }

//------------------------------------------------------------------//
//------------------------------------------------------------------//
//------------------------------------------------------------------//
bvector::bvector()
{ N = 0; data = NULL; };

bvector::bvector(DWORD i) : N(i)
{
  if (i==0)
  {
    data=NULL;
  }
  else
  {
    data = new char[N];
    memset(data, 0x00, N*sizeof(char));
  }
};

bvector::~bvector()
{
  if (data!=NULL)
    delete[] data;
};

DWORD length(bvector &C)
{ return C.N; };

//rvector& rvector::operator =(const rvector &C)
void bvector::operator =(const bvector &C)
{ // ASSIGNMENT OPERATOR.
  if (this == &C)
    return;
  if (data==NULL)
  {
    N = C.N;
    data = new char[N];
  }
  else
    if (N != C.N)
    {
      delete[] data;
      N = C.N;
      data = new char[N];
    }

  memcpy(data, C.data, N*sizeof(char));
//  return *this;
}

char& bvector::operator [](DWORD i)
{
  if (i>=N)
  { //Zmień rozmiar zmiennej
    char *pom;
    pom=data;
    data= new char[i+1];
    memset(data, 0x00, (i+1)*sizeof(char));
    if (N>0)
      memcpy(data, pom, N*sizeof(char));
    N=i+1;
    delete[] pom;
  }
  return data[i];
};

void bvector::resize(DWORD i)
{
  if (i<0) i=0;
  if (i!=N)
  {
    if (N==0)
    {
      data= new char[i];
      memset(data, 0x00, i*sizeof(char));
      N=i;
    }
    else
      if (i==0)
      {
        delete[] data;
        data=NULL;
        N=0;
      }
      else
      {
	      char *pom;
  	    pom=data;
        data= new char[i];
        if (i>N)
        {
          memcpy(data, pom, N*sizeof(char));
          memset(&(data[N]), 0x00, (i-N)*sizeof(char));
        }
        else
          memcpy(data, pom, i*sizeof(char));
        N=i;
        delete[] pom;
      }
  }
}

DWORD bvector::size(void)
{ return sizeof(DWORD)+N*sizeof(char); };

//------------------------------------------------------------------//
//------------------------------------------------------------------//
//------------------------------------------------------------------//
svector::svector()
{ N = 0; data = NULL; };

svector::svector(DWORD i) : N(i)
{
  if (i==0)
  {
    data=NULL;
  }
  else
  {
    data = new short[N];
    memset(data, 0x00, N*sizeof(short));
  }
};

svector::~svector()
{
  if (data!=NULL)
    delete[] data;
};

DWORD length(svector &C)
{ return C.N; };

//rvector& rvector::operator =(const rvector &C)
void svector::operator =(const svector &C)
{ // ASSIGNMENT OPERATOR.
  if (this == &C)
    return;
  if (data==NULL)
  {
    N = C.N;
    data = new short[N];
  }
  else
    if (N != C.N)
    {
      delete[] data;
      N = C.N;
      data = new short[N];
    }

  memcpy(data, C.data, N*sizeof(short));
//  return *this;
}

short& svector::operator [](DWORD i)
{
  if (i>=N)
  { //Zmień rozmiar zmiennej
    short *pom;
    pom=data;
    data= new short[i+1];
    memset(data, 0x00, (i+1)*sizeof(short));
    if (N>0)
      memcpy(data, pom, N*sizeof(short));
    N=i+1;
    delete[] pom;
  }
  return data[i];
};

void svector::resize(DWORD i)
{
  if (i<0) i=0;
  if (i!=N)
  {
    if (N==0)
    {
      data= new short[i];
      memset(data, 0x00, i*sizeof(short));
      N=i;
    }
    else
      if (i==0)
      {
        delete[] data;
        data=NULL;
        N=0;
      }
      else
      {
	      short *pom;
  	    pom=data;
        data= new short[i];
        if (i>N)
        {
          memcpy(data, pom, N*sizeof(short));
          memset(&(data[N]), 0x00, (i-N)*sizeof(short));
        }
        else
          memcpy(data, pom, i*sizeof(short));
        N=i;
        delete[] pom;
      }
  }
}

DWORD svector::size(void)
{ return sizeof(DWORD)+N*sizeof(short); };

//------------------------------------------------------------------//
//------------------------------------------------------------------//
//------------------------------------------------------------------//
dvector::dvector()
{ N = 0; data = NULL; };

dvector::dvector(DWORD i) : N(i)
{
  if (i==0)
  {
    data=NULL;
  }
  else
  {
    data = new DWORD[N];
    memset(data, 0x00, N*sizeof(DWORD));
  }
};

dvector::~dvector()
{
  if (data!=NULL)
    delete[] data;
};

DWORD length(dvector &C)
{ return C.N; };

//rvector& rvector::operator =(const rvector &C)
void dvector::operator =(const dvector &C)
{ // ASSIGNMENT OPERATOR.
  if (this == &C)
    return;
  if (data==NULL)
  {
    N = C.N;
    data = new DWORD[N];
  }
  else
    if (N != C.N)
    {
      delete[] data;
      N = C.N;
      data = new DWORD[N];
    }

  memcpy(data, C.data, N*sizeof(DWORD));
//  return *this;
}

DWORD& dvector::operator [](DWORD i)
{
  if (i>=N)
  { //Zmień rozmiar zmiennej
    DWORD *pom;
    pom=data;
    data= new DWORD[i+1];
    memset(data, 0x00, (i+1)*sizeof(DWORD));
    if (N>0)
      memcpy(data, pom, N*sizeof(DWORD));
    N=i+1;
    delete[] pom;
  }
  return data[i];
};

void dvector::resize(DWORD i)
{
  if (i<0) i=0;
  if (i!=N)
  {
    if (N==0)
    {
      data= new DWORD[i];
      memset(data, 0x00, i*sizeof(DWORD));
      N=i;
    }
    else
      if (i==0)
      {
        delete[] data;
        data=NULL;
        N=0;
      }
      else
      {
	      DWORD *pom;
  	    pom=data;
        data= new DWORD[i];
        if (i>N)
        {
          memcpy(data, pom, N*sizeof(DWORD));
          memset(&(data[N]), 0x00, (i-N)*sizeof(DWORD));
        }
        else
          memcpy(data, pom, i*sizeof(DWORD));
        N=i;
        delete[] pom;
      }
  }
}

DWORD dvector::size(void)
{ return sizeof(DWORD)+N*sizeof(DWORD); };

//------------------------------------------------------------------//
//------------------------------------------------------------------//
//------------------------------------------------------------------//
rvector::rvector()
{ N = 0; data = NULL; };

rvector::rvector(DWORD i) : N(i)
{
  if (i==0)
  {
    data=NULL;
  }
  else
  {
    data = new Tfloat[N];
    memset(data, 0x00, N*sizeof(Tfloat));
  }
};

rvector::~rvector()
{
  if (data!=NULL)
    delete[] data;
};

DWORD length(rvector &C)
{ return C.N; };

//rvector& rvector::operator =(const rvector &C)
void rvector::operator =(const rvector &C)
{ // ASSIGNMENT OPERATOR.
  if (this == &C)
    return;
  if (data==NULL)
  {
    N = C.N;
    data = new Tfloat[N];
  }
  else
    if (N != C.N)
    {
      delete[] data;
      N = C.N;
      data = new Tfloat[N];
    }

  memcpy(data, C.data, N*sizeof(Tfloat));
//  return *this;
}

Tfloat& rvector::operator [](DWORD i)
{
  if (i>=N)
  { //Zmień rozmiar zmiennej
    Tfloat *pom;
    pom=data;
    data= new Tfloat[i+1];
    memset(data, 0x00, (i+1)*sizeof(Tfloat));
    if (N>0)
      memcpy(data, pom, N*sizeof(Tfloat));
    N=i+1;
    delete[] pom;
  }
  return data[i];
};

/*
void rvector::operator /=(DWORD &A)
{
  DWORD i;
  Tfloat *pom;

  pom=data;
  for (i=0; i<N; i++)
  {
    *pom/=A;
    pom++;
  }
}
*/

void rvector::operator /=(const DWORD A)
{
  DWORD i;
  Tfloat *pom;

  pom=data;
  for (i=0; i<N; i++)
  {
    *pom/=A;
    pom++;
  }
}

void rvector::operator *=(const DWORD A)
{
  DWORD i;
  Tfloat *pom;

  pom=data;
  for (i=0; i<N; i++)
  {
    *pom*=A;
    pom++;
  }
}

void rvector::sqrt(void)
{
  DWORD ind;
  Tfloat *pom;

  pom=data;
  for (ind=0; ind<N; ind++)
  {
    if (*pom<0)
      *pom=::sqrt(-(*pom));
    else
      *pom=::sqrt(*pom);
    pom++;
  }
}

void rvector::resize(DWORD i)
{
  if (i<0) i=0;
  if (i!=N)
  {
    if (N==0)
    {
      data= new Tfloat[i];
      memset(data, 0x00, i*sizeof(Tfloat));
      N=i;
    }
    else
      if (i==0)
      {
        delete[] data;
        data=NULL;
        N=0;
      }
      else
      {
	      Tfloat *pom;
  	    pom=data;
        data= new Tfloat[i];
        if (i>N)
        {
          memcpy(data, pom, N*sizeof(Tfloat));
          memset(&(data[N]), 0x00, (i-N)*sizeof(Tfloat));
        }
        else
          memcpy(data, pom, i*sizeof(Tfloat));
        N=i;
        delete[] pom;
      }
  }
}

DWORD rvector::size(void)
{ return sizeof(DWORD)+N*sizeof(Tfloat); };

//------------------------------------------------------------------//
//------------------------------------------------------------------//
//------------------------------------------------------------------//
cvector::cvector()
{ N = 0; data = NULL; };

cvector::cvector(DWORD i) : N(i)
{
  data = new M_complex[N];
  memset(data, 0x00, N*sizeof(M_complex));
};

cvector::cvector(cvector &C)
{  // COPY CONSTRUCTOR
  N = C.N;
  data = new M_complex[N];
  memcpy(data, C.data, N*sizeof(M_complex));
}

cvector::~cvector()
{
  if (data!=NULL)
    delete[] data;
};

void cvector::conj(void)
{
  DWORD i;
  for (i=0; i<N; i++)
    data[i].im*=-1;
};

//cvector& cvector::operator =(const cvector &C)
void cvector::operator =(const cvector &C)
{ // ASSIGNMENT OPERATOR.
  if (this == &C)
    return;
  if (data==NULL)
  {
    N = C.N;
    data = new M_complex[N];
  }
  else
    if (N != C.N)
    {
      delete[] data;
      N = C.N;
      data = new M_complex[N];
    }

  memcpy(data, C.data, N*sizeof(M_complex));
};

void cvector::operator =(const rvector &C)
{
  if (data==NULL)
  {
    N = C.N;
    data = new M_complex[N];
  }
  else
    if (N != C.N)
    {
      delete[] data;
      N = C.N;
      data = new M_complex[N];
    }

  for (DWORD i=0; i<N; i++)
    data[i]=C.data[i];
};

/*
void cvector::operator =(cvector C)
{ // ASSIGNMENT OPERATOR.
  if (this == &C)
    return;
  if (data!=NULL)
    delete[] data;
  N = C.N;
  data = new M_complex[N];

  memcpy(data, C.data, N*sizeof(M_complex));
};
*/

cvector cvector::operator -(cvector &A)
{
  DWORD i;

  cvector result(*this);
  for (i = 0; i < N; i++)
    result[i] -= A.data[i];
  return result;
};

void cvector::operator -=(cvector &A)
{
  DWORD i;

  for (i = 0; i < N; i++)
    data[i] -= A.data[i];
};

void cvector::operator /=(Tfloat &A)
{
  DWORD i;

  for (i=0; i<N; i++)
  {
    data[i].re/=A;
    data[i].im/=A;
  }
}

void cvector::operator /=(DWORD &A)
{
  DWORD i;

  for (i=0; i<N; i++)
  {
    data[i].re/=A;
    data[i].im/=A;
  }
}

void cvector::resize(DWORD i)
{
  if (i<0) i=0;
  if (i!=N)
  {
    if (N==0)
    {
      data= new M_complex[i];
      memset(data, 0x00, i*sizeof(M_complex));
      N=i;
    }
    else
      if (i==0)
      {
        delete[] data;
        data=NULL;
        N=0;
      }
      else
      {
	      M_complex *pom;
  	    pom=data;
        data= new M_complex[i];
        if (i>N)
        {
          memcpy(data, pom, N*sizeof(M_complex));
          memset(&(data[N]), 0x00, (i-N)*sizeof(M_complex));
        }
        else
          memcpy(data, pom, i*sizeof(M_complex));
        N=i;
        delete[] pom;
      }
  }
};

M_complex& cvector::operator [](DWORD i)
{
  if (i>=N)
  { //Zmień rozmiar zmiennej
    M_complex *pom;
    pom=data;
    data= new M_complex[i+1];
    memset(data, 0x00, (i+1)*sizeof(M_complex));
    memcpy(data, pom, N*sizeof(M_complex));
    N=i+1;
    delete[] pom;
  }
  return data[i];
};

void lin2dB(rvector &R, cvector &C)
{
  DWORD i;

  R.resize(C.N);
  if (C.N > 0)
	  for (i=0; i<C.N; i++)
     	R[i]=10*log10(abs2(C[i]));
};

DWORD length(cvector &C)
{
  return C.N;
};

//******************************//
//* DFT functions, they used   *//
//* vector's length diferences *//
//* to minimalize computation   *//
//* time                       *//
//******************************//
TFourier::TFourier(void)
{ //cSin i cSinConj puste
  K=0;
  K2=0;
  fft.resize(0);
  IsFFT=false;

  //FFT parameters
  Kbit = 0; //Liczba bitów -1
};

TFourier::TFourier(DWORD K)
{
  resize(K);
};

void TFourier::resize(DWORD K)
{
  DWORD i, k;

  if (K != TFourier::K)
  {
    TFourier::K=K;
    K2=(K-K%2)/2+1;
  //  fft.resize(K>>1);
  //  fft.resize(K);
    CheckIsFFT(K);

    cSin.resize(K);
    cSinConj.resize(K);
    cSinFFT.resize(K>>1);
    RevBitTable.resize(K);
    FFTshift_RevBitTable.resize(K);

    //Przetworzenie parametrów wejœciowych for FFT
    Kbit = 0; //Liczba bitów -1 (czyli powyższe p)
    k = K>>1;
    while (k!=1)
    {
      Kbit++;
      k >>= 1;
    }

    for (i=0; i<K; i++)
    { //exp{j*2*pi/K*i}
      cSin[i]=M_complex(cos(2*M_PIx1*i/K),-sin(2*M_PIx1*i/K));
      if ((i%2)==0)
        cSinFFT[i>>1]=cSin[i];
      RevBitTable[i]=BitRev(i, Kbit+1);
      FFTshift_RevBitTable[(i+K/2)%K]=RevBitTable[i];
    }
    cSinConj=cSin;  cSinConj.conj();
    cSinConj/=K;
  }
};

void TFourier::resizeR(DWORD K2)
{
  resize((K2%2)?(2*(K2-1)):(2*K2-1));
};

void TFourier::DFTR(cvector& dft, rvector& sygn)
{  //DFT of real sequence
  if (IsFFT)
  {
    FFTR(dft, sygn);
  }
  else
  {
    DWORD k, n;

    //Warto wykorzystaæ symetrię DFT sygnału rzeczywistego
    dft.resize(K2);
    for (k=0; k<K2; k++)
    {
      dft[k]=0;
      for (n=0; n<sygn.N; n++)
        dft[k]+=sygn[n]*cSin[(k*n)%cSin.N];
    }
  }
};

cvector& TFourier::DFT(cvector)
{  //DFT of M_complex sequence
  return cSin;
};

void TFourier::IDFTR(rvector& sygn, cvector& dft)
{  //real part of IDFT
  if (IsFFT)
  {
    IFFTR(sygn, dft);
  }
  else
  {
    DWORD k, n;

    //Warto wykorzystaæ symetrię DFT sygnału rzeczywistego
    dft.resize(K2);
    for (n=0; n<sygn.N; n++)
    {
      sygn[n]=real(dft[0]*cSinConj[0]);
      sygn[n]+=real(dft[K2-1]*cSinConj[(K2*n)%cSinConj.N]);
      for (k=1; k<K2-1; k++)
        sygn[n]+=2*(real(dft[k]*cSinConj[(k*n)%cSinConj.N]));
    }
  }
};

DWORD TFourier::DFTlength(void)
{ return cSin.N; };

DWORD TFourier::DFTdatalen(void)
{ return K2; };

void TFourier::FFT(cvector& probki)
{  //FFT of real sequence
  DWORD Kbit_tmp, M, seg_no, ind1, ind2, ind2b, ind3;
  M_complex COStmp, temp;

  if (K != probki.N)
    resize(probki.N);

  M = K/2; seg_no=1;
  //Kbit - bits number - 1
  Kbit_tmp=Kbit+1;
  for (ind1=0; ind1<=Kbit; ind1++)
  {
    ind2=0; ind2b=ind2+M;
	  do
	  {
      COStmp=cSin[RevBitTable.data[ind2>>Kbit_tmp]>>1];
      for (ind3=0; ind3<M; ind3++)
      {
        temp=COStmp*probki.data[ind2b];
        probki.data[ind2b]=probki.data[ind2]-temp;
        probki.data[ind2]+=temp;

        ind2++; ind2b++;
      }
      ind2+= M; ind2b+= M;
		}
		while (ind2 < K);
    M=M>>1;
    seg_no=seg_no<<1;
    Kbit_tmp--;
  }
  for (ind1=0; ind1<K-1; ind1++)
  {
		ind2 = RevBitTable.data[ind1];
		//i = BitRev(k,Kbit0);
		if (ind2>ind1)
		{
		  COStmp = probki[ind1];
		  probki[ind1]=probki[ind2];
		  probki[ind2]=COStmp;
		}
	}
};

void TFourier::absFFT(rvector& abs_fft, cvector& probki)
{
  DWORD Kbit_tmp, M, seg_no, ind1, ind2, ind2b, ind3;
  M_complex COStmp, temp;

  if (K != probki.N)
    resize(probki.N);

  M = K/2; seg_no=1;
  //Kbit - bits number - 1
  Kbit_tmp=Kbit+1;
  for (ind1=0; ind1<=Kbit; ind1++)
  {
    ind2=0; ind2b=ind2+M;
	  do
	  {
      COStmp=cSin[RevBitTable.data[ind2>>Kbit_tmp]>>1];
      for (ind3=0; ind3<M; ind3++)
      {
        temp=COStmp*probki.data[ind2b];
        probki.data[ind2b]=probki.data[ind2]-temp;
        probki.data[ind2]+=temp;

        ind2++; ind2b++;
      }
      ind2+= M; ind2b+= M;
		}
		while (ind2 < K);
    M=M>>1;
    seg_no=seg_no<<1;
    Kbit_tmp--;
  }
/*
  for (ind1=0; ind1<K-1; ind1++)
  {
		ind2 = RevBitTable.data[ind1];
		//i = BitRev(k,Kbit0);
		if (ind2>ind1)
		{
		  abs_fft[ind1]=abs(probki[ind2]);
		  abs_fft[ind2]=abs(probki[ind1]);
		}
	}
*/
  //FFTshift
  for (ind1=0; ind1<K-1; ind1++)
  {
		ind2 = FFTshift_RevBitTable.data[ind1];
    abs_fft[ind1]=abs(probki[ind2]);
	}
};

void TFourier::abs2FFT(rvector& abs_fft, cvector& probki)
{  //FFT of real sequence
  DWORD Kbit_tmp, M, seg_no, ind1, ind2, ind2b, ind3;
  M_complex COStmp, temp;

  if (K != probki.N)
    resize(probki.N);

  M = K/2; seg_no=1;
  //Kbit - bits number - 1
  Kbit_tmp=Kbit+1;
  for (ind1=0; ind1<=Kbit; ind1++)
  {
    ind2=0; ind2b=ind2+M;
	  do
	  {
      COStmp=cSin[RevBitTable.data[ind2>>Kbit_tmp]>>1];
      for (ind3=0; ind3<M; ind3++)
      {
        temp=COStmp*probki.data[ind2b];
        probki.data[ind2b]=probki.data[ind2]-temp;
        probki.data[ind2]+=temp;

        ind2++; ind2b++;
      }
      ind2+= M; ind2b+= M;
		}
		while (ind2 < K);
    M=M>>1;
    seg_no=seg_no<<1;
    Kbit_tmp--;
  }
  for (ind1=0; ind1<K-1; ind1++)
  {
		ind2 = RevBitTable.data[ind1];
		//i = BitRev(k,Kbit0);
		if (ind2>ind1)
		{
		  abs_fft[ind1]=abs2(probki[ind2]);
		  abs_fft[ind2]=abs2(probki[ind1]);
		}
	}
};

/*
void TFourier::FFT(cvector& probki)
{  //FFT of real sequence
  //K - powinno byæ potęgą dwójki
  //DWORD K, N2,Kbit0,Kbit1, i,l,k;
  //DWORD Kpom, Pom;
  DWORD M, seg_no, ind1, ind2, ind2b, ind3;
  M_complex COStmp;


  if (K != probki.N)
    resize(probki.N);

  M = K/2; seg_no=1;
  //Kbit - bits number - 1
  for (ind1=0; ind1<=Kbit; ind1++)
  {
    ind2=0; ind2b=ind2+M;
	  do
	  {
      for (ind3=0; ind3<M; ind3++)
      {
        COStmp=probki.data[ind2];
//        probki.data[ind2]=probki.data[ind2b]+COStmp;
        probki.data[ind2]+=probki.data[ind2b];
        probki.data[ind2b]=COStmp-probki.data[ind2b];
        probki.data[ind2b]*=cSin[ind3*seg_no];
//        probki.data[ind2b]=probki.data[ind2b]*cSin[ind3*seg_no];

        ind2++; ind2b++;
      }
      ind2+= M; ind2b+= M;
		}
		while (ind2 < K);
    M=M>>1;
    seg_no=seg_no<<1;
  }
  for (ind1=0; ind1<K-1; ind1++)
  {
		ind2 = RevBitTable.data[ind1];
		//i = BitRev(k,Kbit0);
		if (ind2>ind1)
		{
		  COStmp = probki[ind1];
		  probki[ind1]=probki[ind2];
		  probki[ind2]=COStmp;
		}
	}
};
*/

void TFourier::IFFT(cvector& probki)
{  //FFT of real sequence
  FFT(probki);
  probki.conj();
  for (DWORD i=0; i<length(probki); i++)
    probki.data[i]=(probki.data[i])/K;
};

void  TFourier::FFTR(cvector& dft, rvector& sygn)
{
  DWORD i, k, new_FFT_size;
  M_complex pom1, pom2, pom;
  M_complex SINtmp;

//  new_FFT_size=(sygn.N+(sygn.N % 2))/2;
  new_FFT_size=(sygn.N+1)>>1;

  //Prepare input data
  resize(new_FFT_size);
  fft.resize(new_FFT_size);
  dft.resize(new_FFT_size+1);

  memset(fft.data, 0x00, fft.N*sizeof(M_complex));

//  for (i=0; i<sygn.N; i++)
//    if (i%2)
//     fft.data[i>>1].im=sygn.data[i];
//    else
//     fft.data[i>>1].re=sygn.data[i];
  memcpy(fft.data, sygn.data, sygn.N*sizeof(Tfloat));

  //compute M_complex FFT
  FFT(fft);

  //extract real signal DFT
  //i=0
  dft.data[0].re=fft.data[0].re+fft.data[0].im; //pom1+pom2
  k=fft.N-1;
  for (i=1; i<fft.N; i++,k--)
  {
    //pom1=(pom+conj(reverse(pom)))/2
    pom1.re=(fft.data[i].re+fft.data[k].re)/2;
    pom1.im=(fft.data[i].im-fft.data[k].im)/2;
    //pom2=(pom-conj(reverse(pom)))/2
    pom2.re=(fft.data[i].re-fft.data[k].re)/2;
    pom2.im=(fft.data[i].im+fft.data[k].im)/2;

    //pom2=pom2.*exp(-sqrt(-1)*(0:length(pom)-1)*pi/length(pom))*(-sqrt(-1));
    pom=pom2*cSin[i];

    dft.data[i].re=pom1.re+pom.im;
    dft.data[i].im=pom1.im-pom.re;
  }
  //i=K2-1 (fft.N)
  dft.data[fft.N].re=fft.data[0].re-fft.data[0].im; //pom1+pom2
};

void  TFourier::IFFTR(rvector& sygn, cvector& dft)
{
  DWORD i, k;
  M_complex pom1, pom2, pom;
  //ZeroMemory(fft.data, fft.N*sizeof(M_complex));
  //test=(dft2(1)+dft2(end))/2+(dft2(1)-dft2(end))/2*sqrt(-1)
  fft.data[0].re=(dft.data[0].re+dft.data[fft.N].re)/2;
  fft.data[0].im=(dft.data[0].re-dft.data[fft.N].re)/2;

  k=fft.N-1;
  for (i=1; i<fft.N; i++,k--)
  {
    //pom1=(dft(2:(end-1))+conj(dft((end-1):-1:2)))/2
    pom1.re=(dft.data[i].re+dft.data[k].re)/2;
    pom1.im=(dft.data[i].im-dft.data[k].im)/2;
    //pom2=(dft(2:(end-1))-conj(dft((end-1):-1:2)))/2
    pom2.re=(dft.data[i].re-dft.data[k].re)/2;
    pom2.im=(dft.data[i].im+dft.data[k].im)/2;

    //pom2=pom2.*(exp(sqrt(-1)*(1:length(pom)-1)*pi/length(pom)))*sqrt(-1)

    //pom=pom2.*(-exp(sqrt(-1)*(1:length(pom)-1)*pi/length(pom)))
    pom=pom2*cSin[k];

    //pom1-pom*sqrt(-1);
    fft.data[i].re=pom1.re+pom.im;
    fft.data[i].im=pom1.im-pom.re;
  }

  //ifft(test)
  FFT(fft);
  fft.data[0]/=fft.N;
  fft.data[fft.N>>1]/=fft.N;
  k=fft.N-1;
  for (i=1; i<(fft.N>>1); i++,k--)
  {
    pom=fft.data[k]/fft.N;
    fft.data[k]=fft.data[i]/fft.N;
    fft.data[i]=pom;
  }

  memcpy(sygn.data, fft.data, sygn.N*sizeof(Tfloat));
}

// FFT
DWORD TFourier::BitRev(DWORD x, DWORD s)
{
  DWORD Pom;
  DWORD i;

  Pom = 0;
  for (i=0; i<s; i++)
  {
    Pom <<= 1;
    if (x & 1)
      Pom++;
    x >>= 1;
  }
  return Pom;
}

void TFourier::CheckIsFFT(DWORD K)
{
  DWORD bits, k;

  bits = 0; //Liczba bitów -1 (czyli powyższe p)
  k = K;
  while (k!=1)
  {
    bits++;
	  k >>= 1;
  }
  IsFFT= (K == (DWORD(1) << bits));
}

//////////////////////////////////////////////////////////////////
// ------------------------------------------------------------ //
//////////////////////////////////////////////////////////////////

//Najpierw zrobic filtr potem poniższą funkcję
Tfilter::Tfilter(cvector &h, bool InputReal) //Impulse response
{
  //Filtr powinien zapamiętac końcówkę sygnału (bufor stanu) - nie potrzeba InPrevious
  //Odpowiedź impulsowa powinna byc zadana przy inicjacji filtru (czyli zbędne tutaj h)
  Tfilter::C_h=h;
  C_N_1=C_h.N-1;

  if (InputReal)
  {
    R_buffer = new Tfloat[2*C_N_1];
    C_buffer = NULL;

    R_bufferN = R_buffer+C_N_1; R_bufferN_sizeof=sizeof(Tfloat)*(C_N_1);
    memset(R_buffer, 0x00, sizeof(Tfloat)*(2*(C_N_1)));
  }
  else
  {
    C_buffer = new M_complex[2*C_N_1];
    R_buffer = NULL;

    C_bufferN = C_buffer+C_N_1; C_bufferN_sizeof=sizeof(M_complex)*(C_N_1);
    memset(C_buffer, 0x00, sizeof(M_complex)*(2*(C_N_1)));
  }
}

Tfilter::~Tfilter(void)
{
  if (R_buffer != NULL)
    delete [] R_buffer;
  if (C_buffer != NULL)
    delete [] C_buffer;
}

void Tfilter::Execute(cvector &Out, //Output vector
                      rvector &In) //rvector &InPrevious, //Input vector & previous input vector
{
  M_complex *buffer_out;
  buffer_out=Out.data;

// *******************************************
//  Odpowiedz impulsowa zespolona
//  Sygnał wejsciowy rzeczywisty
//  Sygnal wyjsciowy zespolony
// *******************************************

//pierwsze N-1 próbek wyjsciowych wyznacz na bazie bufora stanu
  //pierwsze N-1 próbek wejsciowych do tymczasowego bufora
  memcpy(R_bufferN, In.data, R_bufferN_sizeof);
  //oblicz pierwsze N-1 próbek wyjsciowych
  filterC(buffer_out, C_N_1, R_buffer);

//pozostałe na podstawie wektora wejsciowego
  filterC(buffer_out, In.N-C_N_1, In.data);

//zapamietaj ostatnie N-1 probek bufora wejsciowego
  memcpy(R_buffer, In.data+(In.N-C_N_1), R_bufferN_sizeof);
}

void Tfilter::filterC(LPM_complex &buffer_out, DWORD Ile,
                      Tfloat *buffer_in)
{
  M_complex *h_pom;
  Tfloat *state;
  DWORD ind, ind2;

  for (ind=0; ind<Ile; ind++)
  {
    h_pom=C_h.data+C_N_1;
    state=buffer_in+ind;
    *buffer_out=(*(state++))*(*(h_pom--));
    for (ind2=1; ind2<C_h.N; ind2++)
      *buffer_out+=(*(state++))*(*(h_pom--));
    buffer_out++;
  }
}

/*
Treal2M_complex_decimation::Execute(cvector &Out, //Output vector
                                  rvector &In, //rvector &InPrevious, //Input vector & previous input vector
                                  rvector &h) //Impulse response
{
  //Filtr powinien zapamiętac końcówkę sygnału (bufor stanu) - nie potrzeba InPrevious
  //Odpowiedx impulsowa powinna byc zadana przy inicjacji filtru (czyli zbędne tutaj h)

  // *exp(-jpi/2) <- w efekcjie realis i imaginaris co druga próbka zerowa
  // pozwala do zwiekszyc wydajnosc filtracji
}
*/

/////////////////////////////////////////////////
// ******************************************* //
//  Demodulacja z decymacja                    //
// ******************************************* //
/////////////////////////////////////////////////
TDemodDecym2::TDemodDecym2(void)
{
  ReInit(5);
};

void TDemodDecym2::ReInit(short beta)
{
  short ind;

  TDemodDecym2::beta=beta;
  N=4*beta-1; //Lowpass filter length

  h_re.resize(beta);

  for (ind=0; ind<beta; ind++)
  {
    h_re[ind]=sin(M_PIx1*(ind-beta+((Tfloat)0.5)))/
                 (M_PIx1*(ind-beta+((Tfloat)0.5)));
    h_re[ind]*=GetBlackman_h(ind*2, N);
  }
// /=MAXSHORT można tutaj przeniesc

  //parzyste + nieparzyste
  bufor.resize(0);
  bufor.resize(((2*beta-1) + (2*beta-1))*2);
};

//void TDemodDecym2::filter(DWORD Ile, short *In, cvector &Out)
void TDemodDecym2::filter(DWORD Ile, float *In, cvector &Out)
{ //Ile musi byc liczbą podzielną przez 4
  DWORD ind, ind2;
  float *OddInSample, *EvenInSample;
  Tfloat *OutSample;

//Zmiana znaków w ciągu wejsciowym
  ind=1;
  while (ind<Ile)
  {
    /*
    if (In[ind]==short(-32768))
      In[ind++]=short(32767);
    else
      In[ind++]*=(short)(-1);
    if (In[ind]==short(-32768))
      In[ind]=short(32767);
    else
      In[ind]*=(short)(-1);
    ind+=(short)(3);
    */
    In[ind++]*=-1;
    In[ind]*=-1;
    ind+=3;
  }

//Początek bufora wejsciowego na koniec bufora stanu
  memcpy(bufor.data+(2*beta-1)*2, In, (2*beta-1)*2*sizeof(float));

//Wyznacz początek: 2*beta-1 próbek na podstawie bufora stanu
  OutSample=((Tfloat *)(Out.data));
  EvenInSample=bufor.data;
//Sprawdzic !!!!!!!!!!!!!!!!!
  OddInSample=bufor.data+(2*beta-1);
//  OddInSample=bufor.data+(2*(beta-1)+1);
  for (ind=0; ind<(2*beta-1); ind++)
  {
    //składowa rzeczywista
    //filtracja z wykorzystaniem parzystej symetrii odpowiedzi impulsowej
    *OutSample=0;
    for (ind2=0; ind2<beta; ind2++)
      *OutSample+=h_re.data[ind2]*(EvenInSample[ind2*2]+EvenInSample[(2*beta-1-ind2)*2]);
    //*OutSample/=MAXSHORT;
    OutSample++;
    EvenInSample+=2;

    //składowa urojona
    *OutSample=((Tfloat)(*OddInSample));
    //*OutSample=((Tfloat)(*OddInSample))/MAXSHORT;
    OutSample++;
    OddInSample+=2;
  }

//Wyznacz resztę próbek na podstawie bufora wejsciowego
  EvenInSample=In;
//Sprawdzic !!!!!!!!!!!!!!!!!
  OddInSample=In+(2*beta-1);
  for (ind=0; ind<(Ile/2-(2*beta-1)); ind++)
  {
    //składowa rzeczywista
    //filtracja z wykorzystaniem parzystej symetrii odpowiedzi impulsowej
    *OutSample=0;
    for (ind2=0; ind2<beta; ind2++)
      *OutSample+=h_re.data[ind2]*(EvenInSample[ind2*2]+EvenInSample[(2*beta-1-ind2)*2]);
    //*OutSample/=MAXSHORT;
    OutSample++;
    EvenInSample+=2;

    //składowa urojona
    //*OutSample=((Tfloat)(*OddInSample))/MAXSHORT;
    *OutSample=((Tfloat)(*OddInSample));
    OutSample++;
    OddInSample+=2;
  }

//Uaktualnij bufor stanu
  //Koniec bufora wejsciowego na początek bufora stanu
  memcpy(bufor.data, In+Ile-2*(2*beta-1), (2*beta-1)*2*sizeof(float));
}

/////////////////////////////////////////////////
// ******************************************* //
//          decymacja                          //
// ******************************************* //
/////////////////////////////////////////////////
TDecym2::TDecym2(void)
{
  ReInit(5);
};

void TDecym2::ReInit(short beta)
{
  short ind;

  TDecym2::beta=beta;
  N=4*beta-1; //Lowpass filter length

  h_re.resize(beta);

  for (ind=0; ind<beta; ind++)
  {
    h_re[ind]=sin(M_PIx1*(ind-beta+((Tfloat)0.5)))/
                 (M_PIx1*(ind-beta+((Tfloat)0.5)));
    h_re[ind]*=GetBlackman_h(ind*2, N)/2;
  }
// /=MAXSHORT można tutaj przeniesc

  //parzyste + nieparzyste
  bufor.resize(0);
  bufor.resize(((2*beta-1) + (2*beta-1))*2);
};

void TDecym2::filter(DWORD Ile, cvector &In, cvector &Out)
{ //Ile musi byc liczbą podzielną przez 4
  DWORD ind, ind2;
  M_complex *OddInSample, *EvenInSample;
  M_complex *OutSample;

//Początek bufora wejsciowego na koniec bufora stanu
  memcpy(bufor.data+(2*beta-1)*2, In.data, (2*beta-1)*2*sizeof(M_complex));

//Wyznacz początek: 2*beta-1 próbek na podstawie bufora stanu
  OutSample=((M_complex *)(Out.data));
  EvenInSample=bufor.data;
//Sprawdzic !!!!!!!!!!!!!!!!!
  OddInSample=bufor.data+(2*beta-1);
//  OddInSample=bufor.data+(2*(beta-1)+1);
  for (ind=0; ind<(2*beta-1); ind++)
  {
    //składowa rzeczywista
    //filtracja z wykorzystaniem parzystej symetrii odpowiedzi impulsowej
    *OutSample=0;
    for (ind2=0; ind2<beta; ind2++)
    {
      *OutSample=*OutSample+h_re.data[ind2]*EvenInSample[ind2*2];
      *OutSample=*OutSample+h_re.data[ind2]*EvenInSample[(2*beta-1-ind2)*2];
    }
//      *OutSample+=h_re.data[ind2]*(EvenInSample[ind2*2]+EvenInSample[(2*beta-1-ind2)*2]);
//    OutSample++;
    EvenInSample+=2;

    //składowa urojona
    *OutSample+=*OddInSample/(DWORD)2;
    OutSample++;
    OddInSample+=2;
  }

//Wyznacz resztę próbek na podstawie bufora wejsciowego
  EvenInSample=In.data;
//Sprawdzic !!!!!!!!!!!!!!!!!
  OddInSample=In.data+(2*beta-1);
  for (ind=0; ind<(Ile/2-(2*beta-1)); ind++)
  {
    //składowa rzeczywista
    //filtracja z wykorzystaniem parzystej symetrii odpowiedzi impulsowej
    *OutSample=0;
    for (ind2=0; ind2<beta; ind2++)
    {
      *OutSample=*OutSample+h_re.data[ind2]*EvenInSample[ind2*2];
      *OutSample=*OutSample+h_re.data[ind2]*EvenInSample[(2*beta-1-ind2)*2];
    }
//      *OutSample+=h_re.data[ind2]*(EvenInSample[ind2*2]+EvenInSample[(2*beta-1-ind2)*2]);
//    OutSample++;
    EvenInSample+=2;

    //składowa urojona
    *OutSample+=(*OddInSample)/(DWORD)2;
    OutSample++;
    OddInSample+=2;
  }

//Uaktualnij bufor stanu
  //Koniec bufora wejsciowego na początek bufora stanu
  memcpy(bufor.data, In.data+Ile-2*(2*beta-1), (2*beta-1)*2*sizeof(M_complex));
}

/////////////////////////////////////////////////
// ******************************************* //
//  filtracja okne prostokatnym                //
// ******************************************* //
/////////////////////////////////////////////////
TBoxFilter::TBoxFilter(void)
{
  k=0;  PastInBuffers=NULL; PastOutBuffers=NULL;
  ReInit(5,512);
};

TBoxFilter::~TBoxFilter(void)
{
  if (PastInBuffers != NULL)
    delete [] PastInBuffers;
  if (PastOutBuffers != NULL)
    delete [] PastOutBuffers;
};

void TBoxFilter::ReInit(short FilterLength, short SegmentLength)
{
  DWORD ind;

  N=FilterLength;
  M=SegmentLength;
  segment1=((N-1)%M)+1;
  segment2=M-segment1;
  if ((N%M)!=0)
    k=N/M+1;
  else
    k=N/M;

  if (PastInBuffers!=NULL)
    delete [] PastInBuffers;
  if (PastOutBuffers!=NULL)
    delete [] PastOutBuffers;

  cPastOutValue=0;
  fPastOutValue=0;
  PastInBuffers=new LPvoid[k+1];
  for (ind=0; ind<=k; ind++)
    PastInBuffers[ind]=NULL;
  PastOutBuffers=new LPvoid[k+1];
  for (ind=0; ind<=k; ind++)
    PastOutBuffers[ind]=NULL;
};

void TBoxFilter::ResizeFilter(short FilterLength)
{
  DWORD ind;
  DWORD k_new;
  LPvoid *NewPastInBuffers;
  LPvoid *NewPastOutBuffers;

  N=FilterLength;
  segment1=((N-1)%M)+1;
  segment2=M-segment1;
  if ((N%M)!=0)
    k_new=N/M+1;
  else
    k_new=N/M;

  NewPastInBuffers=new LPvoid[k_new+1];
  for (ind=0; ind<k_new; ind++)
    NewPastInBuffers[ind]=NULL;

  NewPastOutBuffers=new LPvoid[k_new+1];
  for (ind=0; ind<k_new; ind++)
    NewPastOutBuffers[ind]=NULL;

  if (PastInBuffers!=NULL)
  {
    for (ind=0; ind<=minimum(k_new, k); ind++)
      NewPastInBuffers[ind]=PastInBuffers[ind];

    delete []PastInBuffers;
  }
  PastInBuffers=NewPastInBuffers;

  if (PastOutBuffers!=NULL)
  {
    for (ind=0; ind<=minimum(k_new, k); ind++)
      NewPastOutBuffers[ind]=PastOutBuffers[ind];

    delete []PastOutBuffers;
  }
  PastOutBuffers=NewPastOutBuffers;
};

void TBoxFilter::filter(cvector &In, cvector &Out)
{
  M_complex *In_data, *In_old_data, *Out_data;
  DWORD ind;

  /*
  for (ind=0; ind<M; ind++)
    In.data[ind]=0;
  In.data[510]=N/2;
  In.data[205]=N/4;
  */

  for (ind=k; ind>0; ind--)
    PastInBuffers[ind]=PastInBuffers[ind-1];
  PastInBuffers[0]=&In;

  In_data=In.data; Out_data=Out.data;
  if (PastInBuffers[k]!=NULL)
  {
    In_old_data=((cvector *)(PastInBuffers[k]))->data+M-segment1;
    for (ind=0; ind<segment1; ind++)
    {
      cPastOutValue+=((*(In_data++)-*(In_old_data++))/N);
      *(Out_data++)=cPastOutValue;
    }
  }
  else
  {
    for (ind=0; ind<segment1; ind++)
    {
      cPastOutValue+=(*(In_data++)/N);
      *(Out_data++)=cPastOutValue;
    }
  }

  if (PastInBuffers[k-1]!=NULL)
  {
    In_old_data=((cvector *)(PastInBuffers[k-1]))->data;
    for (ind=0; ind<segment2; ind++)
    {
      cPastOutValue+=((*(In_data++)-*(In_old_data++))/N);
      *(Out_data++)=cPastOutValue;
    }
  }
  else
  {
    for (ind=0; ind<segment2; ind++)
    {
      cPastOutValue+=(*(In_data++)/N);
      *(Out_data++)=cPastOutValue;
    }
  }
}

void TBoxFilter::filter(rvector &In, rvector &Out)
{
  Tfloat *In_data, *In_old_data, *Out_data;
  DWORD ind;

  /*
  for (ind=0; ind<M; ind++)
    In.data[ind]=0;
  In.data[510]=N/2;
  In.data[205]=N/4;
  */

  for (ind=k; ind>0; ind--)
    PastInBuffers[ind]=PastInBuffers[ind-1];
  PastInBuffers[0]=&In;

  In_data=In.data; Out_data=Out.data;
  if (PastInBuffers[k]!=NULL)
  {
    In_old_data=((rvector *)(PastInBuffers[k]))->data+M-segment1;
    for (ind=0; ind<segment1; ind++)
    {
      fPastOutValue+=((*(In_data++)-*(In_old_data++))/N);
      *(Out_data++)=fPastOutValue;
    }
  }
  else
  {
    for (ind=0; ind<segment1; ind++)
    {
      fPastOutValue+=(*(In_data++)/N);
      *(Out_data++)=fPastOutValue;
    }
  }

  if (PastInBuffers[k-1]!=NULL)
  {
    In_old_data=((rvector *)(PastInBuffers[k-1]))->data;
    for (ind=0; ind<segment2; ind++)
    {
      fPastOutValue+=((*(In_data++)-*(In_old_data++))/N);
      *(Out_data++)=fPastOutValue;
    }
  }
  else
  {
    for (ind=0; ind<segment2; ind++)
    {
      fPastOutValue+=(*(In_data++)/N);
      *(Out_data++)=fPastOutValue;
    }
  }
}

void TBoxFilter::filter(cvector &In, cvector &Out, rvector &Out2)
{
  M_complex *In_data, *In_old_data, *Out_data;
  Tfloat *Out2_data;
  DWORD ind;

  /*
  for (ind=0; ind<M; ind++)
    In.data[ind]=0;
  In.data[510]=N/2;
  In.data[205]=N/4;
  */

  for (ind=k; ind>0; ind--)
  {
    PastInBuffers[ind]=PastInBuffers[ind-1];
    PastOutBuffers[ind]=PastOutBuffers[ind-1];
  }
  PastInBuffers[0]=&In;
  PastOutBuffers[0]=&Out;

  //First Stage: HalfBoxFilter
  In_data=In.data; Out_data=Out.data;
  if (PastInBuffers[k]!=NULL)
  {
    In_old_data=((cvector *)(PastInBuffers[k]))->data+M-segment1;
    for (ind=0; ind<segment1; ind++)
    {
      cPastOutValue+=((*(In_data++)-*(In_old_data++))/N);
      *(Out_data++)=cPastOutValue;
    }
  }
  else
  {
    for (ind=0; ind<segment1; ind++)
    {
      cPastOutValue+=(*(In_data++)/N);
      *(Out_data++)=cPastOutValue;
    }
  }

  if (PastInBuffers[k-1]!=NULL)
  {
    In_old_data=((cvector *)(PastInBuffers[k-1]))->data;
    for (ind=0; ind<segment2; ind++)
    {
      cPastOutValue+=((*(In_data++)-*(In_old_data++))/N);
      *(Out_data++)=cPastOutValue;
    }
  }
  else
  {
    for (ind=0; ind<segment2; ind++)
    {
      cPastOutValue+=(*(In_data++)/N);
      *(Out_data++)=cPastOutValue;
    }
  }

  //Second Stage: BoxFilter
  In_data=Out.data; Out2_data=Out2.data;
  if (PastOutBuffers[k]!=NULL)
  {
    In_old_data=((cvector *)(PastOutBuffers[k]))->data+M-segment1;
    for (ind=0; ind<segment1; ind++)
    {
      *(Out2_data++)=(abs(*(In_data++))+abs(*(In_old_data++)))/2;
    }
  }
  else
  {
    for (ind=0; ind<segment1; ind++)
    {
      *(Out2_data++)=abs(*(In_data++))/2;
    }
  }

  if (PastInBuffers[k-1]!=NULL)
  {
    In_old_data=((cvector *)(PastOutBuffers[k-1]))->data;
    for (ind=0; ind<segment2; ind++)
    {
      *(Out2_data++)=(abs(*(In_data++))+abs(*(In_old_data++)))/2;
    }
  }
  else
  {
    for (ind=0; ind<segment2; ind++)
    {
      *(Out2_data++)=abs(*(In_data++))/2;
    }
  }
}

//----------------------------------------------------------------//
//----------------------------------------------------------------//
//----------------------------------------------------------------//
void T_Demodulator::ReInit(void)
{
  faza = 0.0;
}

//! k - indeks prążka DFT ==> float ze względu na udokładnianie
void T_Demodulator::Demodulation(float k, cvector &In, cvector &Out, int K)
{
  DWORD n;

////  omega=-M_PI+(2*M_PI*k)/length(Segment);

//  if (k<(int)(length(In)/2))
  if (k<(int)(K/2))
//    omega=M_PI+(2*M_PI*k)/length(In);
    omega=1+(2*k)/K;
  else
//    omega=-M_PI+(2*M_PI*k)/length(In);
    omega=-1+(2*k)/K;

  Out=In;

  for (n=0; n<length(Out); n++)
  {
//    Out[n]*=M_complex(cos(n*omega), -sin(n*omega));
    Out[n]*=M_complex(cos(M_PIx1*faza), -sin(M_PIx1*faza));
    faza += omega;

    if (faza > 1)
      faza -= 2;
    else
      if (faza < -1)
        faza += 2;
  }
}

T_CMPO::T_CMPO(void)
{ last_conj=0; }

void T_CMPO::Run(cvector &InOut)
{
  DWORD n;
  M_complex temp;

  for (n=0; n<length(InOut); n++)
  {
    temp=InOut[n];
    InOut[n]*=last_conj;
    last_conj=conj(temp);
  }
}

void T_CMPO::Run2(cvector &InOut)
{
  DWORD n;
  M_complex temp;

  for (n=0; n<length(InOut); n+=32)//n++)
  {
    temp=InOut[n];
    if (abs(last_conj)<=0.0001)
    {
      InOut[n]=abs(InOut[n]);
    }
    else
    {
      InOut[n]*=(last_conj/abs(last_conj));
    }
    last_conj=conj(temp);
  }
}

