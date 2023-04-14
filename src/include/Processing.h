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

class T_DSPlib_processing : public T_InputElement
{
  friend class MyGLCanvas;
  friend class T_TaskElement;

  private:
    DSP::Clock_ptr MasterClock;
    unsigned int cycles_per_segment;

    bool GraphInitialized;

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

    DSP::u::OutputBuffer *analysis_buffer;
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
    unsigned int PSDs_counter;
    bool standard_PSD_scaling;
    float PSD_scaling_factor;
    DSP::Float_ptr window;
    //! temporary buffer for APSD evaluation - stores PSD before final APSD is ready
    DSP::Float_vector tmp_FFT_out_buffer;
    DSP::Float_vector tmp_FFT_out_buffer2;
    static T_DSPlib_processing *CurrentObject;
    static void AnalysisBufferCallback(DSP::Component_ptr Caller, unsigned int UserDefinedIdentifier);

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
