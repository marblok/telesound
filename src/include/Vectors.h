//---------------------------------------------------------------------------
#ifndef VectorsH
#define VectorsH

#include <windef.h>
#include <math.h>

#include <DSP_lib.h>

//---------------------------------------------------------------------------
typedef float Tfloat;
typedef void * LPvoid;

class M_complex
{
  friend class cvector;
  friend class TFourier;
  Tfloat re;
  Tfloat im;
public:
  M_complex(void);
  M_complex(Tfloat);
  M_complex(Tfloat, Tfloat);
  Tfloat real(void);
  friend Tfloat real(const M_complex &);
  friend M_complex conj(const M_complex &);
  Tfloat imag(void);
  friend Tfloat imag(const M_complex &);
  friend M_complex operator *(Tfloat &, M_complex &);
  friend Tfloat abs2(const M_complex &); //kwadrat modu�u
  friend Tfloat abs(const M_complex &);  //modu�
  //friend Tfloat abs(M_complex);  //modu�
  M_complex operator *(Tfloat &);
  M_complex operator /(Tfloat);
  //M_complex operator /(const Tfloat &);
  M_complex operator /(DWORD &);
  //M_complex operator /(const DWORD &);
  M_complex operator *(M_complex &);
  M_complex operator +(const M_complex &);
  M_complex operator -(const M_complex &);
  void operator +=(const M_complex &);
  void operator *=(const M_complex &);
  void operator -=(const M_complex &);
  void operator /=(Tfloat &);
  void operator /=(DWORD &);
};

typedef M_complex * LPM_complex;


class bvector
{
  friend class T_Counters_Processor;

  DWORD N;
  char *data;
public:
  bvector(void);
  bvector(DWORD i);

  void operator =(const bvector &C);
  char& operator [](DWORD i);
  void resize(DWORD i);
  friend DWORD length(bvector &);
  DWORD size(void);

  virtual ~bvector();
};

class svector
{
  friend class TDemodDecym2;
  friend class T_Counters_Processor;

  DWORD N;
  short *data;
public:
  svector(void);
  svector(DWORD i);

//  rvector& operator =(const rvector &C);
  void operator =(const svector &C);
  short& operator [](DWORD i);
  void resize(DWORD i);
  friend DWORD length(svector &);
  DWORD size(void);

  virtual ~svector();
};

class dvector
{
  friend class TFourier;

  DWORD N;
  DWORD *data;
public:
  dvector(void);
  dvector(DWORD i);

//  rvector& operator =(const rvector &C);
  void operator =(const dvector &C);
  DWORD& operator [](DWORD i);
  void resize(DWORD i);
  friend DWORD length(dvector &);
  DWORD size(void);

  virtual ~dvector();
};

class rvector
{
  friend class cvector;
  friend class TFourier;
  friend class Tfilter;
  friend class TBoxFilter;
  friend class TDemodDecym2;
  friend class TDecym2;
  friend class T_Counters_Processor;

  DWORD N;
  Tfloat *data;
public:
  rvector(void);
  rvector(DWORD i);

//  rvector& operator =(const rvector &C);
  void operator =(const rvector &C);
  Tfloat& operator [](DWORD i);
  //void operator /=(DWORD &);
  void operator /=(const DWORD);
  void operator *=(const DWORD);
  void resize(DWORD i);
  friend DWORD length(rvector &);
  DWORD size(void);
  void sqrt(void);

  virtual ~rvector();
};

class cvector
{
  friend class rvector;
  friend class TFourier;
  friend class Tfilter;
  friend class TBoxFilter;
  friend class TDemodDecym2;
  friend class TDecym2;
  friend class T_Counters_Processor;

  DWORD N;
  M_complex *data;
public:
  cvector(void);
  cvector(DWORD i);
  cvector(cvector &C); // COPY CONSTRUCTOR

  void conj(void);
//  cvector& operator =(const cvector &C);
  void operator =(const cvector &C);
  void operator =(const rvector &C);
//  void operator =(cvector C);
  M_complex& operator [](DWORD i);
  cvector operator -(cvector &);
  void operator -=(cvector &);
  void operator /=(Tfloat &);
  void operator /=(DWORD &);
  void resize(DWORD i);
  friend void lin2dB(rvector &, cvector &);
  friend DWORD length(cvector &);

//  const M_complex& operator [](int i) const{ return data[i]; };

//  cvector operator +(cvector& A) {  // ADDITION OPERATOR
//    cvector result(A.size);  // DO NOT MODIFY THE ORIGINAL
//      for (int i = 0; i < size; i++)
//          result[i] = data[i] + A.data[i];
//      return result;
//      };
//   /* THE STREAM INSERTION OPERATOR. */
//   friend ostream& operator <<(ostream& out_data, cvector& C) {
//      for (int i = 0; i < C.size; i++)
//
//           out_data << "[" << i << "]=" << C.data[i] << "   ";
//      cout << std::endl;
//      return out_data;
//      };
//   };
  virtual ~cvector();
};

typedef cvector * LPcvector;

class TFourier
{
private:
  bool IsFFT;
  DWORD Kbit;  //Additional FFT parameters
  cvector fft;

  cvector cSin, cSinFFT;		//sinus zespolony
  cvector cSinConj; //sprz�ony sinus zespolony
  dvector RevBitTable, FFTshift_RevBitTable;
  DWORD K;   //liczba pr�bek dft dla sygna�u zespolonego
  DWORD K2;  //liczba pr�bek dft dla sygna�u rzeczywistego
  DWORD BitRev(DWORD x, DWORD s);
  void  CheckIsFFT(DWORD K);
public:
  TFourier(void);   //cSin puste
  TFourier(DWORD K); //inicjacja cSin przy tworzeniu
  void resize(DWORD K); //zmiana rozmiaru transformaty
  void resizeR(DWORD K2); //zmiana rozmiaru transformaty
  											 //na podstawie d�ugo�ci widma
                         //sygna�u rzeczywistego

//DFT length is equal cSin length (sSin.N)
  DWORD DFTlength(void);
  DWORD DFTdatalen(void);
  void  DFTR(cvector& dft, rvector& sygn); //dft.N points of DFT of real sygn
  cvector& DFT(cvector);   //DFT of M_complex sequence
  void  IDFTR(rvector& sygn, cvector& dft); //sygn.N points of IDFT
  cvector& IDFT(cvector);  //IDFT of M_complex sequence
  void  FFT(cvector& probki);
  void  absFFT(rvector& abs_fft, cvector& probki);
  void  abs2FFT(rvector& abs_fft, cvector& probki);
  void  FFTR(cvector& fft, rvector& sygn);
  void  IFFT(cvector& probki);
  void  IFFTR(rvector& sygn, cvector& fft);
};

class Tfilter
{
  private:
    enum Eh_type {EH_M_complex=0, EH_real=1, EH_even=2, EH_odd=4, EH_even_sym=8,
                  EH_odd_sym=16};

    cvector C_h;
    M_complex *C_buffer, *C_bufferN;
    DWORD C_N_1, C_bufferN_sizeof;

    rvector R_h;
    Tfloat *R_buffer, *R_bufferN;
    DWORD R_N_1, R_bufferN_sizeof;

    void filterC(LPM_complex &buffer_out, DWORD Ile, Tfloat *buffer_in);
  public:
    Tfilter(cvector &h, bool InputReal);
    ~Tfilter(void);
    void Execute(cvector &Out, rvector &In);
};

//Demodulator z dwukrotn� decymacja
//konversja sygna� rzeczysty na zespolony + decymacja przez 2
class TDemodDecym2
{
  private:
    DWORD beta, N;
    rvector h_re;
    rvector bufor;
    //svector bufor;
  public:
    TDemodDecym2(void);
    void ReInit(short beta);
    void filter(DWORD Ile, float *In, cvector &Out); //Ile musi byc liczb� podzieln� przez 4
};

//Dwukrotna decymacja
class TDecym2
{
  private:
    DWORD beta, N;
    rvector h_re;
    cvector bufor;
  public:
    TDecym2(void);
    void ReInit(short beta);
    void filter(DWORD Ile, cvector &In, cvector &Out); //Ile musi byc liczb� podzieln� przez 4
};

//Filtracja oknem prostok�tnym
class TBoxFilter
{
  private:
    DWORD N, M, k, segment1, segment2;
    void **PastInBuffers, **PastOutBuffers;
    M_complex  cPastOutValue;
    Tfloat   fPastOutValue;
  public:
    TBoxFilter(void);
    ~TBoxFilter(void);
    void ReInit(short FilterLength, short SegmentLength);
    void ResizeFilter(short FilterLength);
    void filter(cvector &In, cvector &Out); //Ile musi byc liczb� podzieln� przez 4
    void filter(cvector &In, cvector &Out, rvector &Out2); //Ile musi byc liczb� podzieln� przez 4
    void filter(rvector &In, rvector &Out); //Ile musi byc liczb� podzieln� przez 4
};

class T_Demodulator
{
  private:
    Tfloat omega;
    Tfloat faza; //! current phase
  public:
    //! K - Magnitude spectrum size (with zeropadding)
    void Demodulation(float k, cvector &In, cvector &Out, int K);
    void ReInit(void);
};

class T_CMPO
{
  private:
    M_complex last_conj;
  public:
    void Run(cvector &InOut);
    void Run2(cvector &InOut);
    T_CMPO(void);
};

inline Tfloat GetBlackman_h(DWORD n, DWORD N)
{ //Do okienkowania odpowiedzi impulsowych (odrzucamy skrajne zera)
  return (0.42 - 0.5*cos(2*DSP::M_PIx1*(n+1)/(N+1)) + 0.08*cos(4*DSP::M_PIx1*(n+1)/(N+1)));
}

inline Tfloat GetBlackman(DWORD n, DWORD N)
{ //Do okienkowania sygna�u przy widmie
  return (0.42 - 0.5*cos(2*DSP::M_PIx1*n/(N-1)) + 0.08*cos(4*DSP::M_PIx1*n/(N-1)));
}

inline unsigned int minimum(unsigned int a, unsigned int b)
{
  if (a<b)
    return a;
  return b;
}
#endif
