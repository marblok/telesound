/*
 * MorseDecoder.h
 *
 *  Created on: 2008-11-06
 *      Author: Marek
 */

#ifndef MORSEDECODER_H_
#define MORSEDECODER_H_

#include "Vectors.h"
#include <DSP_modules_misc.h>

enum TE_LockState {E_none, E_unlocked, E_locking, E_locked};
enum TE_LengthEstimationMode {E_LEM_searching, E_LEM_correcting};

class T_MorseDecoder;

#define ZeropaddingFactor 2

#define HistogramNoOfBins 256 //128
//! number of empty bins separating bins in the same group
#define HistogramGroupBinsMaxSeparation 7 // 1
#define HistogramAlfa 1.02 // 1.05 //1.1
#define MinHistBase 20

class T_Counters
{
    friend class T_Counters_Processor;
  public:
    int N;
    bvector State;
    svector Length;
    rvector Max;

    void ReInit(void)
    {
      N=0;
      //Prealloc some memory
      State.resize(20);
      Length.resize(20);
      Max.resize(20);
    }
    T_Counters(void)
    { ReInit(); }
};

enum TE_DecodedMorse {E_dot='.', E_dash='-', E_unknown_pause='\?', E_end_pause='#',
                      E_inchar_pause=' ', E_char_pause='/', E_group_pause='\\'};
class T_DecodedMorse
{
  public:
    bvector Morse; // TE_DecodedMorse
    dvector Length;
    rvector Max;
    short Counter;

    T_DecodedMorse(void) { Counter=0; };
};

class T_Counters_Processor
{
  public:
    DSP::TMorseTable MorseTable;
    T_MorseDecoder *Parent;

    T_Counters Counters;
  private:
    T_Counters Buffer;

    short MinPulseLength;
    int MaxPauseLength;
    char CurrentState;
    int CurrentLength;
    Tfloat CurrentMax;

    bool SpeedInitialized;

  public:
    dvector HistBase; //center value of the histogram bin
    char HistNo;  //Number of histograms
    int Hist_group_no;
    svector Hist_group_end, Hist_group_start, Hist_group_val;

    DWORD   *HistN;          //Number of elements used to create HistBins
    svector *HistBins; //Histogram bins
    short   *HistBufLen; //Histogram entry signal buffer length
  private:
    DWORD   MaxN;          //Number of detected maximas
    svector Maximas;
    svector *HistBuffer; //Histogram entry signal buffer
    void InitHistograms(void);
    void ReInitHistograms(void);
    void FreeHistograms(void);
    void UpDateHist(short hist, short value);


    svector HistVal1, HistVal_temp;
    svector Hist1, Hist_temp;
    TE_LengthEstimationMode Mode;

    short M;  //Minimum number of pulses needed to start processing
    short N;  //Minimum number of pulses needed to start speed verification
    short min_p;  //Minimum number of pulses necessary to accept pulse length
    short n;  //Minimum number of pulses needed to accept verification

    Tfloat a; //Length precision
    //! \todo check how long this must be
    char SingleMorseCode[1024];
    //__int64 MorseCode_AudioSegmentNo;

    void ProcessBuffer(void);
    int  SortHist(svector &HistVal, svector &Hist);
    void RemoveToShort(svector &HistVal, svector &Hist, int Min);

  private:
    //! last char decoded in DotDash2Text
    char last_znak;
    //! counter of repetition of the last character
    int last_znak_counter;
  public:
    void DotDash2Text(char, char *);

    short dot_len, dash_len, pulse_threshold;
    short dot_len_out, dash_len_out, inchar_pause_len_out, char_pause_len_out;
    short inchar_pause_len, char_pause_len, group_pause_len, inchar_pause_threshold, group_pause_threshold;
    T_DecodedMorse DecodedMorse;

    void ProcessDecitions(bvector &decitions, cvector &signal);
    void ProcessDecitions(bvector &decitions, rvector &signal);

    void ProcessLengths(char *);
    void ReInit(short MinLength, int MaxLength);

    T_Counters_Processor(void)
    {
      MorseTable.LoadFromFile("config\\polish.mct");

//  MorseTable.MorseCode[MorseTable.MorseCodeEntriesNo]=MorseTable.MorseCodeText2Number("--...-"); //Converter to number
//  MorseTable.CharCode[MorseTable.MorseCodeEntriesNo]=-113; // 'Ź'
//  MorseTable.CharBCode[MorseTable.MorseCodeEntriesNo]=0;
//  MorseTable.FontNo[MorseTable.MorseCodeEntriesNo++]=0;

      last_znak = 'u'; //unspecified
      last_znak_counter = 0;
      Parent=NULL;
      InitHistograms();
    }
    ~T_Counters_Processor(void)
    {
      FreeHistograms();
    }
};

class T_ProcessingBuffer
{
  friend class T_MorseDecoder;
  private:
    static cvector FFT_temp;
    static TFourier FFT;
    int K_with_zeropadding;

  public:
    static Tfloat ConstThreshold;
    static T_ProcessingBuffer *Last;
    static Tfloat max_;
    static TBoxFilter NoiseSmoother;

    __int64 AudioSegmentNo;
    //short maks_ind;
    float maks_ind;
    Tfloat maks_val;

    static Tfloat maks_val_threshold;

    cvector Segment;
    cvector DemodulatedSegment;
    cvector DecimatedSegment;
    cvector HalfBoxFilter1Segment;
    rvector BoxFilter1Segment;
    bvector Decitions;
    rvector noise;
    cvector c_noise, smoothed_noise;
    rvector smoothed_noise2, smoothed_noise3;
    rvector Threshold;
//    T_Counters Counters;
    rvector MagnitudeSpectrum;

    void Resize(short SegmentSize, short DecimatedSegmentSize);

    void EvaluateFFT(void);
    void FindSpectrumMax(short k_min, short k_max);
    void VerifySpectrumMax(short k_min, short k_max, short dk_blind);

    void Thresholding(void);
    void Thresholding2(void);
    void Thresholding3(short N, short SegmentSize);
    void Thresholding4(short N, short SegmentSize);

    static void LockRange2k_range(DWORD, int);
};

//---------------------------------------------------------------------------
class TOptions
{
public:
  int audio_segment_size;
  long sampling_rate;

  //bool UseHardSpace;
  //bool UseMultipleSpaces;

  //bool UnrecognizedCodeTimeStamp;
  //bool WriteMorseTable;
  //bool WriteSpeedChange;
  //bool PeriodicTimeStamp;
  //double TimeStampPeriod;

  int   MinDotLength;
  float F_lock_min;
  float F_lock_max;
  double group_pause_factor;
  double group_text_space_factor;

  //below are option data after some processing
  //int TimeStampPeriod_segmentNo;
  int k_lock_min, k_lock_max, dk_blind;

  void Process(DWORD K, DWORD K_with_zeropadding);

  TOptions(long Fp);
};

#define ProcessingSegmentsNo 16
class T_MorseDecoder
{
  public:
    static TOptions *Options;

  private:
    int NoOfDecimationStages;
    int BoxFilterLength;
    int DecimatedSegmentSize;

    int MinPulseLength;
    int MaxPauseLength;

    TDemodDecym2 Decymacja2;
    TBoxFilter   BoxFilter1, BoxFilter2;
    T_Demodulator Demodulator;

    TDecym2 *Decym2;
    cvector *pom_decym;

    short LockingRange, LockThreshold, UnlockRange;

    T_ProcessingBuffer ProcessingBuffers[ProcessingSegmentsNo];
    short CurrentBufferNo;
    T_ProcessingBuffer *CurrentBuffer;
    short RecalculationBufferNo;
    void NextProcessingBuffer(void);
    T_ProcessingBuffer *RecalculationBuffer;

    T_Counters_Processor Counters_Processor;

  public:
    //! \bug should use function to read it instead of making it public
    TE_LockState LockState; //state of the locking module
  private:
    float LockingFrequency; //suspected signal frequency
    float LockedFrequency;  //locked signal frequency
    int UnlockCounter;      //length of the unlocked segment
    bool VerifyLockState(void); //function which checks the lock state of the signal
      //returns true when recalculation of segments in locking range is necessary

  public:
    T_MorseDecoder(long SamplingRate, TOptions *Options_in);
    ~T_MorseDecoder(void);

    //void ProcessSegment(short *input_segment, int input_segment_size);
    void ProcessSegment(float *input_segment, char *text_buffer);
};


#endif /* MORSEDECODER_H_ */
