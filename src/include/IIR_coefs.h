#ifndef IIR_coefs
#define IIR_coefs

#include <stddef.h> // NULL definition
#include <DSP_types.h>

//! IIR filter coeficients
class T_IIR_coefs
{
  public:
    long sampling_rate;
    int filter_order;

    int polynomial_order;
    //! approximation range frequency
    int approximation_Fp; //Hz
    int minimum_allowed_Fp; // Hz
    int maximum_allowed_Fp; // Hz

    float Rp;
    float Rs;

    float Freq_factor;

    /*! coef_b[x][y]:
     *   - x - indeks wsp�czynnika b_x filtru
     *   - y - indeks wsp�czynnika wielomianu aproksymuj�cego wsp�czynnik b_x filtru
     */
    DSP::Prec_Float_ptr *coef_b;
    /*! coef_a[x][y]:
     *   - x - indeks wsp�czynnika a_x filtru
     *   - y - indeks wsp�czynnika wielomianu aproksymuj�cego wsp�czynnik a_x filtru
     */
    DSP::Prec_Float_ptr *coef_a;

    /*! \warning All internal variables will be reset.
     *
     */
    void Init(int filter_order_in);
    void Free(void);

    T_IIR_coefs(void);
    ~T_IIR_coefs(void);
};

class T_HPF_IIR_coefs_8000 : public T_IIR_coefs
{
  public:
    T_HPF_IIR_coefs_8000(void);

  /*! \note symetria nieparzysta wzgl�dem wsp�czynnik�w filtru
   *
   *  coef_b[x][y]:
   *   - x - indeks wsp�czynnika b_x filtru
   *   - y - indeks wsp�czynnika wielomianu aproksymuj�cego wsp�czynnik b_x filtru
   */
  static const DSP::Prec_Float_ptr const_coef_b[];
  /*! coef_a[x][y]:
   *   - x - indeks wsp�czynnika a_x filtru
   *   - y - indeks wsp�czynnika wielomianu aproksymuj�cego wsp�czynnik a_x filtru
   */
  static const DSP::Prec_Float_ptr const_coef_a[];
};

class T_HPF_IIR_coefs_11025 : public T_IIR_coefs
{
  public:
    T_HPF_IIR_coefs_11025(void);

  /*! \note symetria nieparzysta wzgl�dem wsp�czynnik�w filtru
   *
   *  coef_b[x][y]:
   *   - x - indeks wsp�czynnika b_x filtru
   *   - y - indeks wsp�czynnika wielomianu aproksymuj�cego wsp�czynnik b_x filtru
   */
  static const DSP::Prec_Float_ptr const_coef_b[];
  /*! coef_a[x][y]:
   *   - x - indeks wsp�czynnika a_x filtru
   *   - y - indeks wsp�czynnika wielomianu aproksymuj�cego wsp�czynnik a_x filtru
   */
  static const DSP::Prec_Float_ptr const_coef_a[];
};

class T_HPF_IIR_coefs_16000 : public T_IIR_coefs
{
  public:
    T_HPF_IIR_coefs_16000(void);

  /*! \note symetria nieparzysta wzgl�dem wsp�czynnik�w filtru
   *
   *  coef_b[x][y]:
   *   - x - indeks wsp�czynnika b_x filtru
   *   - y - indeks wsp�czynnika wielomianu aproksymuj�cego wsp�czynnik b_x filtru
   */
  static const DSP::Prec_Float_ptr const_coef_b[];
  /*! coef_a[x][y]:
   *   - x - indeks wsp�czynnika a_x filtru
   *   - y - indeks wsp�czynnika wielomianu aproksymuj�cego wsp�czynnik a_x filtru
   */
  static const DSP::Prec_Float_ptr const_coef_a[];
};

class T_HPF_IIR_coefs_22050 : public T_IIR_coefs
{
  public:
    T_HPF_IIR_coefs_22050(void);

  /*! \note symetria nieparzysta wzgl�dem wsp�czynnik�w filtru
   *
   *  coef_b[x][y]:
   *   - x - indeks wsp�czynnika b_x filtru
   *   - y - indeks wsp�czynnika wielomianu aproksymuj�cego wsp�czynnik b_x filtru
   */
  static const DSP::Prec_Float_ptr const_coef_b[];
  /*! coef_a[x][y]:
   *   - x - indeks wsp�czynnika a_x filtru
   *   - y - indeks wsp�czynnika wielomianu aproksymuj�cego wsp�czynnik a_x filtru
   */
  static const DSP::Prec_Float_ptr const_coef_a[];
};

class T_HPF_IIR_coefs_32000 : public T_IIR_coefs
{
  public:
    T_HPF_IIR_coefs_32000(void);

  /*! \note symetria nieparzysta wzgl�dem wsp�czynnik�w filtru
   *
   *  coef_b[x][y]:
   *   - x - indeks wsp�czynnika b_x filtru
   *   - y - indeks wsp�czynnika wielomianu aproksymuj�cego wsp�czynnik b_x filtru
   */
  static const DSP::Prec_Float_ptr const_coef_b[];
  /*! coef_a[x][y]:
   *   - x - indeks wsp�czynnika a_x filtru
   *   - y - indeks wsp�czynnika wielomianu aproksymuj�cego wsp�czynnik a_x filtru
   */
  static const DSP::Prec_Float_ptr const_coef_a[];
};

class T_HPF_IIR_coefs_44100 : public T_IIR_coefs
{
  public:
    T_HPF_IIR_coefs_44100(void);

  /*! \note symetria nieparzysta wzgl�dem wsp�czynnik�w filtru
   *
   *  coef_b[x][y]:
   *   - x - indeks wsp�czynnika b_x filtru
   *   - y - indeks wsp�czynnika wielomianu aproksymuj�cego wsp�czynnik b_x filtru
   */
  static const DSP::Prec_Float_ptr const_coef_b[];
  /*! coef_a[x][y]:
   *   - x - indeks wsp�czynnika a_x filtru
   *   - y - indeks wsp�czynnika wielomianu aproksymuj�cego wsp�czynnik a_x filtru
   */
  static const DSP::Prec_Float_ptr const_coef_a[];
};

class T_HPF_IIR_coefs_48000 : public T_IIR_coefs
{
  public:
    T_HPF_IIR_coefs_48000(void);

  /*! \note symetria nieparzysta wzgl�dem wsp�czynnik�w filtru
   *
   *  coef_b[x][y]:
   *   - x - indeks wsp�czynnika b_x filtru
   *   - y - indeks wsp�czynnika wielomianu aproksymuj�cego wsp�czynnik b_x filtru
   */
  static const DSP::Prec_Float_ptr const_coef_b[];
  /*! coef_a[x][y]:
   *   - x - indeks wsp�czynnika a_x filtru
   *   - y - indeks wsp�czynnika wielomianu aproksymuj�cego wsp�czynnik a_x filtru
   */
  static const DSP::Prec_Float_ptr const_coef_a[];
};

// ----------------------------------------------------------- //
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
// ----------------------------------------------------------- //
class T_LPF_IIR_coefs_8000 : public T_IIR_coefs
{
  public:
    T_LPF_IIR_coefs_8000(void);

  /*! \note symetria nieparzysta wzgl�dem wsp�czynnik�w filtru
   *
   *  coef_b[x][y]:
   *   - x - indeks wsp�czynnika b_x filtru
   *   - y - indeks wsp�czynnika wielomianu aproksymuj�cego wsp�czynnik b_x filtru
   */
  static const DSP::Prec_Float_ptr const_coef_b[];
  /*! coef_a[x][y]:
   *   - x - indeks wsp�czynnika a_x filtru
   *   - y - indeks wsp�czynnika wielomianu aproksymuj�cego wsp�czynnik a_x filtru
   */
  static const DSP::Prec_Float_ptr const_coef_a[];
};

class T_LPF_IIR_coefs_11025 : public T_IIR_coefs
{
  public:
    T_LPF_IIR_coefs_11025(void);

  /*! \note symetria nieparzysta wzgl�dem wsp�czynnik�w filtru
   *
   *  coef_b[x][y]:
   *   - x - indeks wsp�czynnika b_x filtru
   *   - y - indeks wsp�czynnika wielomianu aproksymuj�cego wsp�czynnik b_x filtru
   */
  static const DSP::Prec_Float_ptr const_coef_b[];
  /*! coef_a[x][y]:
   *   - x - indeks wsp�czynnika a_x filtru
   *   - y - indeks wsp�czynnika wielomianu aproksymuj�cego wsp�czynnik a_x filtru
   */
  static const DSP::Prec_Float_ptr const_coef_a[];
};

class T_LPF_IIR_coefs_16000 : public T_IIR_coefs
{
  public:
    T_LPF_IIR_coefs_16000(void);

  /*! \note symetria nieparzysta wzgl�dem wsp�czynnik�w filtru
   *
   *  coef_b[x][y]:
   *   - x - indeks wsp�czynnika b_x filtru
   *   - y - indeks wsp�czynnika wielomianu aproksymuj�cego wsp�czynnik b_x filtru
   */
  static const DSP::Prec_Float_ptr const_coef_b[];
  /*! coef_a[x][y]:
   *   - x - indeks wsp�czynnika a_x filtru
   *   - y - indeks wsp�czynnika wielomianu aproksymuj�cego wsp�czynnik a_x filtru
   */
  static const DSP::Prec_Float_ptr const_coef_a[];
};

class T_LPF_IIR_coefs_22050 : public T_IIR_coefs
{
  public:
    T_LPF_IIR_coefs_22050(void);

  /*! \note symetria nieparzysta wzgl�dem wsp�czynnik�w filtru
   *
   *  coef_b[x][y]:
   *   - x - indeks wsp�czynnika b_x filtru
   *   - y - indeks wsp�czynnika wielomianu aproksymuj�cego wsp�czynnik b_x filtru
   */
  static const DSP::Prec_Float_ptr const_coef_b[];
  /*! coef_a[x][y]:
   *   - x - indeks wsp�czynnika a_x filtru
   *   - y - indeks wsp�czynnika wielomianu aproksymuj�cego wsp�czynnik a_x filtru
   */
  static const DSP::Prec_Float_ptr const_coef_a[];
};

class T_LPF_IIR_coefs_32000 : public T_IIR_coefs
{
  public:
    T_LPF_IIR_coefs_32000(void);

  /*! \note symetria nieparzysta wzgl�dem wsp�czynnik�w filtru
   *
   *  coef_b[x][y]:
   *   - x - indeks wsp�czynnika b_x filtru
   *   - y - indeks wsp�czynnika wielomianu aproksymuj�cego wsp�czynnik b_x filtru
   */
  static const DSP::Prec_Float_ptr const_coef_b[];
  /*! coef_a[x][y]:
   *   - x - indeks wsp�czynnika a_x filtru
   *   - y - indeks wsp�czynnika wielomianu aproksymuj�cego wsp�czynnik a_x filtru
   */
  static const DSP::Prec_Float_ptr const_coef_a[];
};

class T_LPF_IIR_coefs_44100 : public T_IIR_coefs
{
  public:
    T_LPF_IIR_coefs_44100(void);

  /*! \note symetria nieparzysta wzgl�dem wsp�czynnik�w filtru
   *
   *  coef_b[x][y]:
   *   - x - indeks wsp�czynnika b_x filtru
   *   - y - indeks wsp�czynnika wielomianu aproksymuj�cego wsp�czynnik b_x filtru
   */
  static const DSP::Prec_Float_ptr const_coef_b[];
  /*! coef_a[x][y]:
   *   - x - indeks wsp�czynnika a_x filtru
   *   - y - indeks wsp�czynnika wielomianu aproksymuj�cego wsp�czynnik a_x filtru
   */
  static const DSP::Prec_Float_ptr const_coef_a[];
};

class T_LPF_IIR_coefs_48000 : public T_IIR_coefs
{
  public:
    T_LPF_IIR_coefs_48000(void);

  /*! \note symetria nieparzysta wzgl�dem wsp�czynnik�w filtru
   *
   *  coef_b[x][y]:
   *   - x - indeks wsp�czynnika b_x filtru
   *   - y - indeks wsp�czynnika wielomianu aproksymuj�cego wsp�czynnik b_x filtru
   */
  static const DSP::Prec_Float_ptr const_coef_b[];
  /*! coef_a[x][y]:
   *   - x - indeks wsp�czynnika a_x filtru
   *   - y - indeks wsp�czynnika wielomianu aproksymuj�cego wsp�czynnik a_x filtru
   */
  static const DSP::Prec_Float_ptr const_coef_a[];
};

bool GetIIR_HPF_coefs(float Fp, long sampling_rate, int &Nb, DSP::Float *&b, int &Na, DSP::Float *&a);

bool GetIIR_LPF_coefs(float Fp, long sampling_rate, int &Nb, DSP::Float *&b, int &Na, DSP::Float *&a);


const int no_of_sos_segments = 7;

extern float wave_in_resample_LPF_b_48000_8000[no_of_sos_segments][3];
extern float wave_in_resample_LPF_a_48000_8000[no_of_sos_segments][3];

extern float wave_in_resample_LPF_b_44100_11025[no_of_sos_segments][3];
extern float wave_in_resample_LPF_a_44100_11025[no_of_sos_segments][3];

extern float wave_in_resample_LPF_b_48000_16000[no_of_sos_segments][3];
extern float wave_in_resample_LPF_a_48000_16000[no_of_sos_segments][3];

extern const float wave_in_resample_LPF_b_44100_22050[15];
extern const float wave_in_resample_LPF_a_44100_22050[15];

extern float wave_in_resample_LPF_b_48000_32000[no_of_sos_segments][3];
extern float wave_in_resample_LPF_a_48000_32000[no_of_sos_segments][3];

#endif //IIR_coefs
