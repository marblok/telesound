/*
 * Processing.h
 *
 *  Created on: 2008-10-16
 *      Author: Marek
 */

#ifndef PROCESSING_H_
#define PROCESSING_H_

//#define __DOUBLE_CHANNEL_FILTER__
#define __TEST_CHANNEL_FILTER__

#include <DSP_sockets.h>
#include <DSP_lib.h>
#include <DSP.h>
#include <wxAddons.h>
#include <DSP_modules_misc.h>
#include "MorseDecoder.h"

#include "Misc.h"
#include "IIR_coefs.h"

extern wxCriticalSection CS_OnDraw;
extern wxCriticalSection CS_UserData;

//typedef float sos_matrix[no_of_sos_segments][3];
typedef std::vector<std::vector<float>> sos_matrix;

class Modulator{
  
  private:
    unsigned int L1, M1, L2 , M2;
    DSP::e::ModulationType mod_type;
    unsigned int bits_per_symbol;
    std::string coef_name_stage1, coef_name_stage2;    
    DSP::Float_vector h_LPF_stage1, h_LPF_stage2;
    std::unique_ptr <DSP::u::Const> Const;
    float CarrierFreq;
    DSP::Clock_ptr BitClock, SymbolClock, Interpol1Clock, Interpol2Clock;
    std::unique_ptr <DSP::u::BinRand> ModBits;
    std::unique_ptr <DSP::u::Serial2Parallel> ModS2P;
    std::unique_ptr <DSP::u::SymbolMapper> ModMapper;
    std::unique_ptr <DSP::u::Const> ModZero;
    std::unique_ptr <DSP::u::Zeroinserter> ModZeroInserter;
    std::unique_ptr <DSP::u::FIR> ModFIR;
    std::unique_ptr <DSP::u::SamplingRateConversion> ModConverter;
    std::unique_ptr <DSP::u::DDScos> ModDDS;
    std::unique_ptr <DSP::u::Multiplication> ModMul;
    std::unique_ptr <DSP::u::Amplifier> ModAmp;
    std::unique_ptr <DSP::u::Vacuum> ModVac;
  public:
  void create_branch (DSP::Clock_ptr Clock_in, DSP::input &Output_signal, E_ModulatorTypes Modulator_type, float Carrier_freq, unsigned short variant = 1, bool Enable_output = false);
  void clear_branch(void);
  DSP::Complex_vector get_constellation(){
    DSP::Complex_vector tmp_constellation;
    tmp_constellation.resize(0);
    bool is_real=false;
    if(ModMapper!=nullptr){
      getConstellation(tmp_constellation,(DSP::e::ModulationType)mod_type,0,bits_per_symbol, is_real);
    }
    return tmp_constellation;
    }

  void enableOutput(bool enable){
    if(ModAmp!=nullptr){
      ModAmp->SetGain((enable)?1.0f:0.0f);
    }
  }
  
  void setCarrierFrequency(float New_frequency){
   if (ModDDS!=nullptr)
      ModDDS->SetAngularFrequency(DSP::M_PIx2*New_frequency);
      CarrierFreq =New_frequency;
  }
  friend class Demodulator;
};
class Demodulator{
  private:
    DSP::Clock_ptr  SymbolClock, Interpol1Clock, Interpol2Clock;
    
    std::unique_ptr <DSP::u::Amplifier> DemodAmp;
    std::unique_ptr <DSP::u::AdjustableDelay> DemodDelay;
    std::unique_ptr <DSP::u::DDScos> DemodDDS;
    std::unique_ptr <DSP::u::Multiplication> DemodMul;
    std::unique_ptr <DSP::u::SamplingRateConversion> DemodConverter;
    std::unique_ptr <DSP::u::FIR> DemodFIR;
    std::unique_ptr <DSP::u::RawDecimator> DemodDecimator;

  public:
  void create_branch(DSP::Clock_ptr Clock_in, DSP::input &Constellation, DSP::input &Eyediagram, DSP::output &Input_signal, Modulator &modulator, bool enable);
  void clear_branch(void);

  void enableInput(bool enable){
    if(DemodAmp!=nullptr){
      DemodAmp->SetGain((enable)?1.0f:0.0f);
    }
  }
  
  void setInputDelay(int delay){// execute between clock ticks
    if(DemodDelay!=nullptr){
      DemodDelay->SetDelay(delay);
    }
  } 

  void setCarrierFrequency(float New_frequency){
   if (DemodDDS!=nullptr)
      DemodDDS->SetAngularFrequency(-DSP::M_PIx2*New_frequency);
  }

};

class T_DSPlib_processing : public T_InputElement
{
  friend class MyGLCanvas;
  friend class T_TaskElement;

  private:
    DSP::Clock_ptr MasterClock;
    unsigned int cycles_per_segment;

    bool GraphInitialized;
    bool reloadModulator = false;
    bool reloadDelay = false;
    long Fp;
    //! sampling rate of wave source files
    long Fp_wave_in;
    //! interpolation and decimation factors for wave_in files
    int L, M;
    int resample_LPF_order;
    DSP::Float_vector wave_in_resample_LPF_b;
    sos_matrix wave_in_resample_LPF_sos_b;
    DSP::Float_vector wave_in_resample_LPF_a;
    sos_matrix wave_in_resample_LPF_sos_a;
    //! if != E_US_none processing state must be updated
    E_UpdateState UpdateState;

    DSP::u::AudioInput *AudioIn;
    DSP::u::Amplifier *AudioInGain;
    DSP::u::IIR *DC_notcher;
    bool AudioInOff;
    float local_signal_gain;

    DSP::u::DDScos *CarrierIn;
    int WPM;
    std::string ascii_text;
    DSP::u::MORSEkey *MorseKey;
    DSP::u::RealMultiplication *MorseMul;

    //DSP::u::WaveInput *WaveIn;
    DSP::u::FileInput *WaveIn;
    std::string wav_filename;
    DSP::u::Zeroinserter *Zeroinserter;
    DSP::u::IIR *WaveIn_LPF, *WaveIn_sos_LPF[6];
    DSP::u::RawDecimator *Decimator;

    DSP::u::Addition *DigitalSignalsAdd;
    DSP::u::Addition *AllSignalsAdd;
    DSP::u::SocketOutput *out_socket;


    DSP::u::SocketInput *in_socket;

    DSP::u::Amplifier *LocalSignalGain;
    DSP::u::Addition *LocalSignalAdd;

    DSP::u::IIR *ChannelFilter_LPF;
    DSP::u::IIR *ChannelFilter_LPF2;
    DSP::u::IIR *ChannelFilter_HPF;
    DSP::u::IIR *ChannelFilter_HPF2;


    // DSP::u::Splitter *OutSplitter;
    bool ChannelFilterON;
    float ChannelFd, ChannelFg;
    DSP::Float_vector LPF_new_coefs_b, LPF_new_coefs_a;
    int LPF_new_Na, LPF_new_Nb;
    DSP::Float_vector HPF_new_coefs_b, HPF_new_coefs_a;
    int HPF_new_Na, HPF_new_Nb;
    DSP::u::InputBuffer *NoiseBuffer;
    DSP::u::Amplifier *NoiseGain;
    DSP::u::Amplifier *SignalGain;
    float SNR_dB, alfa_n, alfa_s;
    DSP::u::Addition *NoiseAdd;
    DSP::u::AudioOutput *AudioOut;
    //DSP::Component *AudioOut;
    
    //        Modulator components           //

    Modulator modulator;
    Demodulator demodulator;
    bool ModulatorState;
    E_ModulatorTypes ModulatorType;
    unsigned short ModulatorVariant;
    float CarrierFreq;
    DSP::Complex_vector current_constellation;
    bool DemodulatorState;
    float DemodulatorCarrierFreq;
    int DemodulatorDelay;

    //**************************************//
    DSP::u::OutputBuffer *analysis_buffer, *constellation_buffer, *eyediagram_buffer;
    const unsigned int constellation_buffer_size = 3000;
    const unsigned int eyediagram_buffer_size = 12000;
    TOptions *MorseDecoder_options;
    bool MorseReceiverState;
  
    T_MorseDecoder *MorseDecoder;
    TE_LockState LastLockState;
    DSP::Float *morse_decoder_slot;
    int morse_decoder_slot_ind;
    int no_of_morse_decoder_slots;
    char morse_text_buffer[2048];
    DSP::Fourier FFT_block;
    int FFT_size;
    int zeropadding_factor;
    //! number of spectrogram APSD slots
    /*! \note this is also number of time signal
     * and histogram slots.
     */
    unsigned int NoOfPSDslots;
    int PSD_size;
    //! współczynnik pamiętania poprzednich wartości uśrednionego periodogramu
    /*! \note must be in range (0.0, 1.0)
     */
    float A_PSD_factor;
    //! uśredniony periodogram
    float *A_PSD, *A_PSD_dB;
    //! spectrogram time span in seconds
    float specgram_time_span;
    //! time length in samples per one APSD
    unsigned int NoOfSamplesPerAPSD;
    //! number of PSDs used to calculate one APSD
    unsigned int PSDs_per_APSD;
    //! this is buffer & window size
    int BufferSize;
    //! step between buffer callbacks and time signal segment size
    int BufferStep;
    //! number of histogram bins
    int NoOfHistBins;
    //! uśredniony histogram
    float *A_Histogram;
    //! number of entries in signal map
    int SignalMapSize;
    //! number of slots in signal map
    int NoOfSignalMAPslots;
    DSP::Float_vector tmp_FFT_buffer;
    DSP::Float_vector tmp_constellation_buffer, tmp_eyediagram_buffer;

    unsigned int PSDs_counter;
    bool standard_PSD_scaling;
    float PSD_scaling_factor;
    DSP::Float_ptr window;
    //! temporary buffer for APSD evaluation - stores PSD before final APSD is ready
    DSP::Float_vector tmp_FFT_out_buffer;
    DSP::Float_vector tmp_FFT_out_buffer2;
    static T_DSPlib_processing *CurrentObject;
    static void AnalysisBufferCallback(DSP::Component_ptr Caller, unsigned int UserDefinedIdentifier);
    static void ConstellationBufferCallback(DSP::Component_ptr Caller, unsigned int UserDefinedIdentifier);
    static void EyeDiagramBufferCallback(DSP::Component_ptr Caller, unsigned int UserDefinedIdentifier);
    //! przechowuje kolejne segmenty analizowanego sygnału
    T_PlotsStack *SignalSegments;
    //! przechowuje mapy kolejnych segmentów analizowanego sygnału
    /*! W celu przyśpieszenia wyświetlania dla kolejnych segmentów
     * jest obliczana mapa wyświetlania sygnału.
     * Dla wybranego rastru amplitud oraz czasu, zapisane
     * są binarne mapy zapalonych punktów.
     */
    T_PlotsStack *SignalSegmentsMaps;
    //! przechowuje histogramy kolejnych segmentów analizowanego sygnału
    T_PlotsStack *Histograms;
    //! przechowuje periodogramy kolejnych segmentów analizowanego sygnału
    T_PlotsStack *PSDs;
    //! size of PSD for high resolution
    int PSD_high_size;
    //! plots stack for high resolution spectrogram
    T_PlotsStack *high_res_PSDs;
    T_PlotsStack *constellation;
    T_PlotsStack *eyediagrams;

    //! created processing algorithm based on DSPlib
    void CreateAlgorithm(bool run_as_server, std::string address,
        long SamplingRate, DSP::e::SampleType sockets_sample_type = DSP::e::SampleType::ST_short);
    void DestroyAlgorithm(void);

  public:
    void ComputeHighResolutionSpectorgram(void);

    T_DSPlib_processing(T_ProcessingSpec *SpecList);
    ~T_DSPlib_processing(void);

    unsigned int ElementOverlapSegmentSize(void);
    unsigned int ElementOutputSegmentSize(unsigned int input_segment_size);
    unsigned int MaxNumberOfProtectedSamples(void);

    void ProcessUserData(void *userdata);

    bool Process(E_processing_DIR processing_DIR = E_PD_forward);
};

#endif /* PROCESSING_H_ */
