/*
 * Processing.cpp
 *
 *  Created on: 2008-10-16
 *      Author: Marek Blok
 */
#include "Processing.h"
#include "main.h"

wxCriticalSection CS_UserData;

E_UpdateState& operator|= (E_UpdateState& left, const E_UpdateState& right)
{
  left = E_UpdateState(left | right);
  return left;
}
E_UpdateState& operator&= (E_UpdateState& left, const E_UpdateState& right)
{
  left = E_UpdateState(left & right);
  return left;
}
E_UpdateState& operator^= (E_UpdateState& left, const E_UpdateState& right)
{
  left = E_UpdateState(left ^ right);
  return left;
}
E_UpdateState operator~ (const E_UpdateState& right)
{
  E_UpdateState left;

  left = (E_UpdateState)(~((DWORD)right));
  return left;
}

T_DSPlib_processing *T_DSPlib_processing::CurrentObject = NULL;
wxCriticalSection CS_OnDraw;

T_DSPlib_processing::T_DSPlib_processing(T_ProcessingSpec *SpecList)
  : T_InputElement(E_OBT_float, 0, 0) // no output elements
{
  bool run_as_server;
  string address;
  float SNR_lin;

  //cycles_per_segment = 1024;
  standard_PSD_scaling = false;
  UpdateState = E_US_none;

//#define __DEMO__
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  Fp = SpecList->SamplingRate;
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  // FFT_size = 512 to small time resolution for Fp = 48000
  FFT_size = 1024; // window size is about half of FFT_size
#ifdef __DEMO__
  FFT_size = 512;
#endif
  if (Fp <= 16000)
    zeropadding_factor = 2;
  else
    zeropadding_factor = 1;
  NoOfPSDslots = SpecList->no_of_psd_slots;
  A_PSD_factor = 0.95;
  NoOfHistBins = 101;
  SignalMapSize = 2; //define line for each slot
  //NoOfSignalMAPslots = NoOfPSDslots * 8;
  NoOfSignalMAPslots = NoOfPSDslots * 1;
#ifdef __DEMO__
  NoOfSignalMAPslots = NoOfPSDslots * 1;
#endif

  // +++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  run_as_server = SpecList->run_as_server;
  address = SpecList->IP_address;
  wave_in_resample_LPF_b = NULL;
  wave_in_resample_LPF_a = NULL;
  switch (Fp)
  {
    case 8000:
      Fp_wave_in = 48000;
      L = 1; M = 6;
      resample_LPF_order = 14;
      wave_in_resample_LPF_b = NULL;
      wave_in_resample_LPF_a = NULL;
      wave_in_resample_LPF_sos_b = &wave_in_resample_LPF_b_48000_8000;
      wave_in_resample_LPF_sos_a = &wave_in_resample_LPF_a_48000_8000;
      break;
    case 11025:
      Fp_wave_in = 44100;
      L = 1; M = 4;
      resample_LPF_order = 14;
      wave_in_resample_LPF_b = NULL;
      wave_in_resample_LPF_a = NULL;
      wave_in_resample_LPF_sos_b = &wave_in_resample_LPF_b_44100_11025;
      wave_in_resample_LPF_sos_a = &wave_in_resample_LPF_a_44100_11025;
      break;
    case 16000:
      Fp_wave_in = 48000;
      L = 1; M = 3;
      resample_LPF_order = 14;
      wave_in_resample_LPF_b = NULL;
      wave_in_resample_LPF_a = NULL;
      wave_in_resample_LPF_sos_b = &wave_in_resample_LPF_b_48000_16000;
      wave_in_resample_LPF_sos_a = &wave_in_resample_LPF_a_48000_16000;
      break;
    case 22050:
      Fp_wave_in = 44100;
      L = 1; M = 2;
      resample_LPF_order = 14;
      wave_in_resample_LPF_b = const_cast<DSP_float_ptr>(wave_in_resample_LPF_b_44100_22050);
      wave_in_resample_LPF_a = const_cast<DSP_float_ptr>(wave_in_resample_LPF_a_44100_22050);
      break;
    case 32000:
      Fp_wave_in = 48000;
      L = 2; M = 3;
      resample_LPF_order = 14;
      wave_in_resample_LPF_b = NULL;
      wave_in_resample_LPF_a = NULL;
      wave_in_resample_LPF_sos_b = &wave_in_resample_LPF_b_48000_32000;
      wave_in_resample_LPF_sos_a = &wave_in_resample_LPF_a_48000_32000;
      break;
    case 44100:
      Fp_wave_in = 44100;
      L =1; M = 1;
      resample_LPF_order = 0;
      wave_in_resample_LPF_b = NULL;
      wave_in_resample_LPF_a = NULL;
      break;
    case 48000:
      Fp_wave_in = 48000;
      L =1; M = 1;
      resample_LPF_order = 0;
      wave_in_resample_LPF_b = NULL;
      wave_in_resample_LPF_a = NULL;
      break;
    default:
      Fp_wave_in = 1;
      break;
  }
  specgram_time_span = SpecList->time_span; // in [sek]
  NoOfSamplesPerAPSD = (unsigned int)(specgram_time_span*Fp)/NoOfPSDslots;

  // 100 slotów na 2 sek ==> 2*Fp próbek
  // 8000Sa/s : 16000 / 100 ==> 160 Sa na slot
  // 48000Sa/s : 96000 / 100 ==> 960 Sa na slot
  // ? 5 sloty na Execute (10 razy na sek)
  //cycles_per_segment = (unsigned int)(5*specgram_time_span*Fp)/NoOfPSDslots;
  //cycles_per_segment = (unsigned int)(5*(specgram_time_span*Fp/10000)*NoOfPSDslots);
  // co 5% szerokości okna czasowego
  cycles_per_segment = (unsigned int)(0.05*(specgram_time_span*Fp));
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++ //

  AudioInOff = SpecList->MikeIsOff;
  local_signal_gain = SpecList->local_signal_gain;

  ChannelFilterON = SpecList->ChannelFilterON;
  ChannelFd = SpecList->ChannelFd;
  ChannelFg = SpecList->ChannelFg;

  wav_filename = SpecList->wav_filename;

  WPM = SpecList->WPM;
  SNR_dB = SpecList->SNR_dB; //, alfa_n, alfa_s
  // przelicz SNR_dB na NoiseGain i SignalGain
  //SNR_lin = pow(10, SNR_dB/20); // odniesione do fali prostokątnej o amplitudzie 1
  SNR_lin = sqrt(2)*pow(10, SNR_dB/20); // Odniesione do sygnału sinusoidalnego o amplitudzie 1
  alfa_n = 1/(SNR_lin+3);
  alfa_s = 1 - 3*alfa_n;

  MorseReceiverState = SpecList->morse_receiver_state;

  // +++++++++++++++++++++++++++++++++++++++++++++++++++++ //

  MasterClock = NULL;

  AudioIn = NULL;
  DC_notcher = NULL;
  AudioInGain = NULL;

  CarrierIn = NULL;
  MorseKey = NULL;
  MorseMul = NULL;

  WaveIn = NULL;
  Zeroinserter = NULL;
  WaveIn_LPF = NULL;
  for (int ind = 0; ind < no_of_sos_segments-1; ind++)
    WaveIn_sos_LPF[ind] = NULL;
  Decimator = NULL;

  DigitalSignalsAdd = NULL;
  AllSignalsAdd = NULL;
  out_socket = NULL;

  in_socket = NULL;
  LocalSignalGain = NULL;
  LocalSignalAdd = NULL;

  ChannelFilter_LPF = NULL;
  ChannelFilter_HPF = NULL;
#ifdef __DOUBLE_CHANNEL_FILTER__
  ChannelFilter_LPF2 = NULL;
  ChannelFilter_HPF2 = NULL;
#endif
  NoiseBuffer = NULL;
  NoiseGain = NULL;
  SignalGain = NULL;
  NoiseAdd = NULL;
  AudioOut = NULL;


  CurrentObject = NULL;
  tmp_FFT_buffer = NULL;
  tmp_FFT_out_buffer = NULL;
  tmp_FFT_out_buffer2 = NULL;
  window = NULL;

  SignalSegments = NULL;
  SignalSegmentsMaps = NULL;
  Histograms = NULL;
  A_Histogram = NULL;
  PSDs = NULL;
  high_res_PSDs = NULL;
  A_PSD = NULL;
  A_PSD_dB = NULL;
  PSDs_counter = 0;

  MorseDecoder = NULL;
  morse_decoder_slot = NULL;

  GraphInitialized = false;

  //! \bug put socket sample type into SpecList and let user select
  CreateAlgorithm(run_as_server, address, Fp, DSP_ST_short);

  DSP_component::CheckInputsOfAllComponents();
  DSP_component::ListComponents(MasterClock);
  DSP_clock::SchemeToDOTfile(MasterClock, "processing_scheme.dot");
}

T_DSPlib_processing::~T_DSPlib_processing(void)
{
  DestroyAlgorithm();

  if (tmp_FFT_buffer != NULL)
  {
    delete [] tmp_FFT_buffer;
    tmp_FFT_buffer = NULL;
  }
  if (tmp_FFT_out_buffer != NULL)
  {
    delete [] tmp_FFT_out_buffer;
    tmp_FFT_out_buffer = NULL;
  }
  if (tmp_FFT_out_buffer2 != NULL)
  {
    delete [] tmp_FFT_out_buffer2;
    tmp_FFT_out_buffer2 = NULL;
  }
  if (window != NULL)
  {
    delete [] window;
    window = NULL;
  }
  if (A_Histogram != NULL)
  {
    delete [] A_Histogram;
    A_Histogram = NULL;
  }
  if (A_PSD != NULL)
  {
    delete [] A_PSD;
    A_PSD = NULL;
  }
  if (A_PSD_dB != NULL)
  {
    delete [] A_PSD_dB;
    A_PSD_dB = NULL;
  }
}

void T_DSPlib_processing::ProcessUserData(void *userdata)
{
  T_InterfaceState *temp_spec;

  CS_UserData.Enter();
  temp_spec = (T_InterfaceState *)userdata;
  if ((temp_spec->userdata_state & E_US_audio_in_gain) != 0)
  {
    AudioInOff = temp_spec->mike_is_off;

    UpdateState |= E_US_audio_in_gain;
    temp_spec->userdata_state ^= E_US_audio_in_gain;
  };

  if ((temp_spec->userdata_state & E_US_local_signal) != 0)
  {
    local_signal_gain = temp_spec->local_signal_gain;

    UpdateState |= E_US_local_signal;
    temp_spec->userdata_state ^= E_US_local_signal;
  }

  if ((temp_spec->userdata_state & E_US_ascii_text) != 0)
  {
    ascii_text = temp_spec->ascii_text;

    UpdateState |= E_US_ascii_text;
    temp_spec->userdata_state ^= E_US_ascii_text;
  }

  if ((temp_spec->userdata_state & E_US_WPM_change) != 0)
  {
    WPM = temp_spec->WPM;

    UpdateState |= E_US_WPM_change;
    temp_spec->userdata_state ^= E_US_WPM_change;
  }

  if ((temp_spec->userdata_state & E_US_wav_file_open) != 0)
  {
    if (Fp_wave_in == 44100)
      wav_filename = "./audio_wzor_44100/";
    else
      wav_filename = "./audio_wzor_48000/";
    wav_filename += temp_spec->wav_filename;

    UpdateState |= E_US_wav_file_open;
    temp_spec->userdata_state ^= E_US_wav_file_open;
  }

  if ((temp_spec->userdata_state & E_US_noise_level) != 0)
  {
    float SNR_lin;

    SNR_dB = temp_spec->SNR_dB;
    // przelicz SNR_dB na NoiseGain i SignalGain
    //SNR_lin = pow(10, SNR_dB/20); // odniesione do fali prostokątnej o amplitudzie 1
    SNR_lin = sqrt(2)*pow(10, SNR_dB/20); // Odniesione do sygnału sinusoidalnego o amplitudzie 1
    alfa_n = 1/(SNR_lin+3);
    alfa_s = 1 - 3*alfa_n;

    UpdateState |= E_US_noise_level;
    temp_spec->userdata_state ^= E_US_noise_level;
  }

  if ((temp_spec->userdata_state & E_US_channel_LPF_coefs) != 0)
  {
    // compute new LPF filter coefs
    ChannelFg = temp_spec->channel_Fg;
    GetIIR_LPF_coefs(ChannelFg, Fp, LPF_new_Nb, LPF_new_coefs_b, LPF_new_Na, LPF_new_coefs_a);
    if (LPF_new_Na <= 0)
    {
      LPF_new_coefs_a = new DSP_float[1];
      LPF_new_coefs_a[0] = 1.0;
    } // coefs_b == NULL is e proper value
    //! \bug implement support for ChannelFilterON

    #ifdef __DEBUG__
    {
      char text[1024];
      int ind;

      sprintf(text, "LPF_new_Nb = %i", LPF_new_Nb);
      DSPf_InfoMessage(text);
      for (ind = 0; ind < LPF_new_Nb; ind++)
      {
        sprintf(text, "  LPF_new_coefs_b[%i] = %.6f", ind, LPF_new_coefs_b[ind]);
        DSPf_InfoMessage(text);
      }
      sprintf(text, "LPF_new_Na = %i", LPF_new_Na);
      DSPf_InfoMessage(text);
      for (ind = 0; ind < LPF_new_Na; ind++)
      {
        sprintf(text, "  LPF_new_coefs_a[%i] = %.6f", ind, LPF_new_coefs_a[ind]);
        DSPf_InfoMessage(text);
      }
    }
    #endif

    UpdateState |= E_US_channel_LPF_coefs;
    temp_spec->userdata_state ^= E_US_channel_LPF_coefs;
  }

  if ((temp_spec->userdata_state & E_US_channel_HPF_coefs) != 0)
  {
    // compute new HPF filter coefs
    ChannelFd = temp_spec->channel_Fd;
    GetIIR_HPF_coefs(ChannelFd, Fp, HPF_new_Nb, HPF_new_coefs_b, HPF_new_Na, HPF_new_coefs_a);
    if (HPF_new_Na <= 0)
    {
      HPF_new_coefs_a = new DSP_float[1];
      HPF_new_coefs_a[0] = 1.0;
    } // coefs_b == NULL is e proper value
    //! \bug implement support for ChannelFilterON

    #ifdef __DEBUG__
    {
      char text[1024];
      int ind;

      sprintf(text, "HPF_new_Nb = %i", HPF_new_Nb);
      DSPf_InfoMessage(text);
      for (ind = 0; ind < HPF_new_Nb; ind++)
      {
        sprintf(text, "  HPF_new_coefs_b[%i] = %.6f", ind, HPF_new_coefs_b[ind]);
        DSPf_InfoMessage(text);
      }
      sprintf(text, "HPF_new_Na = %i", HPF_new_Na);
      DSPf_InfoMessage(text);
      for (ind = 0; ind < HPF_new_Na; ind++)
      {
        sprintf(text, "  HPF_new_coefs_a[%i] = %.6f", ind, HPF_new_coefs_a[ind]);
        DSPf_InfoMessage(text);
      }
    }
    #endif

    UpdateState |= E_US_channel_HPF_coefs;
    temp_spec->userdata_state ^= E_US_channel_HPF_coefs;
  }

  if ((temp_spec->userdata_state & E_US_morse_receiver_state) != 0)
  {
    MorseReceiverState = temp_spec->morse_receiver_state;

    UpdateState |= E_US_morse_receiver_state;
    temp_spec->userdata_state ^= E_US_morse_receiver_state;
  }

  if ((temp_spec->userdata_state & E_US_high_res_psd) != 0)
  {
    ComputeHighResolutionSpectorgram();

    UpdateState |= E_US_high_res_psd;
    temp_spec->userdata_state ^= E_US_high_res_psd;
  };

  if (temp_spec->userdata_state != E_US_none)
  {
    #ifdef __DEBUG__
      DSPf_ErrorMessage("T_DSPlib_processing::ProcessUserData", "unsupported userdata state");
    #endif
    temp_spec->userdata_state = E_US_none;
  }
  CS_UserData.Leave();
}

void T_DSPlib_processing::CreateAlgorithm(bool run_as_server, string address,
                                          long SamplingRate, DSPe_SampleType sockets_sample_type)
{
  char tekst[1024];
  int ind;
  DSP_clock_ptr WaveInClock, WaveInClock2;

  if (MasterClock != NULL)
    DestroyAlgorithm();

  Fp = SamplingRate;

  CurrentObject = this;
  if (run_as_server == true)
  { // server side
    //if (address == NULL)
      address = "0.0.0.0"; // always use this address to avoid problems

    //DSPf_SetLogState(DSP_LS_file);
#ifdef __DEBUG__
    DSPf_SetLogFileName("log_file_server.log");
#endif
  }
  else
  {
    if (address.length() == 0)
      address = "127.0.0.1";

    //DSPf_SetLogState(DSP_LS_file);
#ifdef __DEBUG__
    DSPf_SetLogFileName("log_file_client.log");
#endif
  }

  DSPf_InfoMessage(DSP_lib_version_string());
  DSPf_InfoMessage();

  MasterClock=DSP_clock::CreateMasterClock();


  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  // wejście 1: mikrofon // możliwośæ wyłączenia w trybie DEBUG
  AudioIn = new DSPu_AudioInput(MasterClock, Fp, 1, 16);
  if (AudioInOff == true)
    AudioInGain = new DSPu_Amplifier(0.0);
  else
    AudioInGain = new DSPu_Amplifier(1.0);
  DSP_float a_in[2] = {1.0, -0.99};
  float factor = 1.99/2.0;
  DSP_float b_in[2] = {factor*1.0, factor*-1.0};
  DC_notcher = new DSPu_IIR (2, a_in, 2, b_in);
  DC_notcher->SetName("DC_notcher", false);
  DSP_connect(AudioIn->Output("out"), DC_notcher->Input("in"));
  DSP_connect(DC_notcher->Output("out"), AudioInGain->Input("in"));

  // wejście 2: kod MORSE'a
  if (run_as_server == true)
    CarrierIn = new DSPu_DDScos(MasterClock, false, 0.5, DSP_float(1200*M_PIx2/Fp));
    //CarrierIn = new DSPu_DDScos(MasterClock, false, 0.5, DSP_float(400*M_PIx2/Fp));
  else
    CarrierIn = new DSPu_DDScos(MasterClock, false, 0.5, DSP_float(800*M_PIx2/Fp));
  MorseKey = new DSPu_MORSEkey(MasterClock, WPM, Fp);
  MorseKey->LoadCodeTable("polish.mct");
  MorseMul = new DSPu_RealMultiplication(2);
  DSP_connect(CarrierIn->Output("out"), MorseMul->Input("in1"));
  DSP_connect(MorseKey->Output("out"), MorseMul->Input("in2"));

  // wejście 3: plik audio ??? <== może tylko na starcie ??
  //! \bug problem z szybkością próbkowania
  //WaveIn = new DSPu_WaveInput(MasterClock, "test.wav", ".");
  WaveInClock = DSP_clock::GetClock(MasterClock, M, L);
  WaveInClock2 = DSP_clock::GetClock(MasterClock, M, 1);
  WaveIn = new DSPu_FILEinput(WaveInClock, "", 1U, DSP_ST_short, DSP_FT_wav);
  if (L > 1)
  {
    Zeroinserter = new DSPu_Zeroinserter(WaveInClock, L);
    DSP_connect(WaveIn->Output("out"), Zeroinserter->Input("in"));
  }
  if (resample_LPF_order > 0)
  {
    if (wave_in_resample_LPF_a != NULL)
    {
      WaveIn_LPF = new DSPu_IIR(resample_LPF_order+1, wave_in_resample_LPF_a, resample_LPF_order+1, wave_in_resample_LPF_b);

      if (Zeroinserter == NULL)
        DSP_connect(WaveIn->Output("out"), WaveIn_LPF->Input("in"));
      else
        DSP_connect(Zeroinserter->Output("out"), WaveIn_LPF->Input("in"));
    }
    else
    {
      for (ind = 0; ind < no_of_sos_segments-1; ind++)
      {
        WaveIn_sos_LPF[ind] = new DSPu_IIR(3, (*wave_in_resample_LPF_sos_a)[ind], 3, (*wave_in_resample_LPF_sos_b)[ind]);
        if (ind >= 1)
          DSP_connect(WaveIn_sos_LPF[ind-1]->Output("out"), WaveIn_sos_LPF[ind]->Input("in"));
      }
      if (L > 1)
      {
        DSP_float temp_float[3];
        for (ind = 0; ind < 3; ind++)
        {
          temp_float[ind] = L*(*wave_in_resample_LPF_sos_b)[no_of_sos_segments-1][ind];
        }
        WaveIn_LPF = new DSPu_IIR(3, (*wave_in_resample_LPF_sos_a)[no_of_sos_segments-1],
                                  3, temp_float);
      }
      else
      {
        WaveIn_LPF = new DSPu_IIR(3, (*wave_in_resample_LPF_sos_a)[no_of_sos_segments-1],
                                  3, (*wave_in_resample_LPF_sos_b)[no_of_sos_segments-1]);
      }
      DSP_connect(WaveIn_sos_LPF[no_of_sos_segments-2]->Output("out"), WaveIn_LPF->Input("in"));

      if (Zeroinserter == NULL)
        DSP_connect(WaveIn->Output("out"), WaveIn_sos_LPF[0]->Input("in"));
      else
        DSP_connect(Zeroinserter->Output("out"), WaveIn_sos_LPF[0]->Input("in"));
    }
  }
  if (M > 1)
  {
    Decimator = new DSPu_RawDecimator(WaveInClock2, M);
    if (WaveIn_LPF == NULL)
    {
      if (Zeroinserter == NULL)
      {
        DSP_connect(WaveIn->Output("out"), Decimator->Input("in"));
      }
      else
        DSP_connect(Zeroinserter->Output("out"), Decimator->Input("in"));
    }
    else
      DSP_connect(WaveIn_LPF->Output("out"), Decimator->Input("in"));
  }

  DigitalSignalsAdd = new DSPu_Addition(2U, 0U);
  DigitalSignalsAdd->SetName("Add", false);
  DSP_connect(MorseMul->Output("out"), DigitalSignalsAdd->Input("in1"));

  if (Decimator == NULL)
  {
    if (WaveIn_LPF == NULL)
    {
      if (Zeroinserter == NULL)
        DSP_connect(WaveIn->Output("out"), DigitalSignalsAdd->Input("in2"));
      else
        DSP_connect(Zeroinserter->Output("out"), DigitalSignalsAdd->Input("in2"));
    }
    else
      DSP_connect(WaveIn_LPF->Output("out"), DigitalSignalsAdd->Input("in2"));
  }
  else
    DSP_connect(Decimator->Output("out"), DigitalSignalsAdd->Input("in2"));

  AllSignalsAdd = new DSPu_Addition(2U, 0U);
  AllSignalsAdd->SetName("Add", false);
  DSP_connect(AudioInGain->Output("out"), AllSignalsAdd->Input("in1"));
  DSP_connect(DigitalSignalsAdd->Output("out"), AllSignalsAdd->Input("in2"));

  if (run_as_server == true)
    out_socket = new DSPu_SOCKEToutput(address, false, 0x00000002, 1, sockets_sample_type);
  else
    out_socket = new DSPu_SOCKEToutput(address, true, 0x00000001, 1, sockets_sample_type);
  DSP_connect(AllSignalsAdd->Output("out"), out_socket->Input("in"));

  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  if (run_as_server == true)
    in_socket = new DSPu_SOCKETinput(MasterClock, address, false, 0x00000001, 1, sockets_sample_type);
  else
    in_socket = new DSPu_SOCKETinput(MasterClock, address, true, 0x00000002, 1, sockets_sample_type);

  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  LocalSignalGain = new DSPu_Amplifier(local_signal_gain);
  LocalSignalAdd = new DSPu_Addition(2U,0U);
  LocalSignalAdd->SetName("Add", false);
  DSP_connect(DigitalSignalsAdd->Output("out"), LocalSignalGain->Input("in1"));
  DSP_connect(LocalSignalGain->Output("out"), LocalSignalAdd->Input("in1"));
  DSP_connect(in_socket->Output("out"),       LocalSignalAdd->Input("in2"));


  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  DSP_float_ptr coefs_b, coefs_a;
  int Na, Nb;
  GetIIR_HPF_coefs(ChannelFd, Fp, Nb, coefs_b, Na, coefs_a);
  if (Na <= 0)
  {
    coefs_a = new DSP_float[1];
    coefs_a[0] = 1.0;
  } // coefs_b == NULL is e proper value
  //! \bug implement support for ChannelFilterON
  ChannelFilter_HPF = new DSPu_IIR(Na, coefs_a, Nb, coefs_b);
  ChannelFilter_HPF->SetName("IIR HPF", false);
#ifdef __DOUBLE_CHANNEL_FILTER__
  ChannelFilter_HPF2 = new DSPu_IIR(Na, coefs_a, Nb, coefs_b);
  ChannelFilter_HPF2->SetName("IIR HPF 2", false);
#endif
  if (coefs_a != NULL)
    delete [] coefs_a;
  if (coefs_b != NULL)
    delete [] coefs_b;

  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  GetIIR_LPF_coefs(ChannelFg, Fp, Nb, coefs_b, Na, coefs_a);
  if (Na <= 0)
  {
    coefs_a = new DSP_float[1];
    coefs_a[0] = 1.0;
  } // coefs_b == NULL is e proper value
  //! \bug implement support for ChannelFilterON
  ChannelFilter_LPF = new DSPu_IIR(Na, coefs_a, Nb, coefs_b);
  ChannelFilter_LPF->SetName("IIR LPF", false);
#ifdef __DOUBLE_CHANNEL_FILTER__
  ChannelFilter_LPF2 = new DSPu_IIR(Na, coefs_a, Nb, coefs_b);
  ChannelFilter_LPF2->SetName("IIR LPF 2", false);
#endif
  if (coefs_a != NULL)
    delete [] coefs_a;
  if (coefs_b != NULL)
    delete [] coefs_b;

  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  int noise_len = (int)ceil(4*Fp*specgram_time_span);
  NoiseBuffer = new DSPu_InputBuffer(MasterClock, noise_len, 1, DSP_cyclic);
  DSP_float_ptr  temp_noise = new DSP_float[noise_len];
  DSP_rand::InitRandGenerator(false);
  DSP_rand::randn(noise_len, temp_noise);
  NoiseBuffer->WriteBuffer(temp_noise, noise_len*sizeof(DSP_float));
  NoiseBuffer->SetName("randn");
  NoiseGain = new DSPu_Amplifier(alfa_n);
  DSP_connect(NoiseBuffer->Output("out"), NoiseGain->Input("in"));

  SignalGain = new DSPu_Amplifier(alfa_s);
  DSP_connect(LocalSignalAdd->Output("out"), SignalGain->Input("in"));

  NoiseAdd = new DSPu_Addition(2U, 0U);
  NoiseAdd->SetName("Add", false);

  AudioOut = new DSPu_AudioOutput(Fp, 1, 16);
  //AudioOut = new DSPu_Vacuum;

#ifdef __TEST_CHANNEL_FILTER__
  DSP_connect(SignalGain->Output("out"), NoiseAdd->Input("in1"));
  DSP_connect(NoiseGain->Output("out"), NoiseAdd->Input("in2"));
  DSP_connect(NoiseAdd->Output("out"), ChannelFilter_LPF->Input("in"));
  #ifdef __DOUBLE_CHANNEL_FILTER__
    DSP_connect(ChannelFilter_LPF->Output("out"), ChannelFilter_LPF2->Input("in"));
    DSP_connect(ChannelFilter_LPF2->Output("out"), ChannelFilter_HPF2->Input("in"));
    DSP_connect(ChannelFilter_HPF2->Output("out"), ChannelFilter_HPF->Input("in"));
    DSP_connect(ChannelFilter_HPF->Output("out"), AudioOut->Input("in"));
  #else
    DSP_connect(ChannelFilter_LPF->Output("out"), ChannelFilter_HPF->Input("in"));
    DSP_connect(ChannelFilter_HPF->Output("out"), AudioOut->Input("in"));
  #endif
#else
  DSP_connect(SignalGain->Output("out"), ChannelFilter_LPF->Input("in"));
  #ifdef __DOUBLE_CHANNEL_FILTER__
    DSP_connect(ChannelFilter_LPF->Output("out"), ChannelFilter_LPF2->Input("in"));
    DSP_connect(ChannelFilter_LPF2->Output("out"), ChannelFilter_HPF2->Input("in"));
    DSP_connect(ChannelFilter_HPF2->Output("out"), ChannelFilter_HPF->Input("in"));
    DSP_connect(ChannelFilter_HPF->Output("out"), NoiseAdd->Input("in1"));
  #else
    DSP_connect(ChannelFilter_LPF->Output("out"), ChannelFilter_HPF->Input("in"));
    DSP_connect(ChannelFilter_HPF->Output("out"), NoiseAdd->Input("in1"));
  #endif
  DSP_connect(NoiseGain->Output("out"), NoiseAdd->Input("in2"));
  DSP_connect(NoiseAdd->Output("out"), AudioOut->Input("in"));
#endif


  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  // basic assumptions:
  //FFT_size = 1024;
  //NoOfPSDslots = 100;
  //NoOfHistBins = 101;
  //SignalMapSize = 2; //define line for each slot
  //NoOfSignalMAPslots = NoOfPSDslots * 8;
  //specgram_time_span = 2.0; // in [sek]
  PSD_size = (FFT_size / 2) + 1;
  PSD_high_size = 0;

  BufferStep = int(specgram_time_span * Fp / NoOfPSDslots);
  PSDs_per_APSD = 1;
  while (BufferStep >= FFT_size/zeropadding_factor)
  {
    PSDs_per_APSD++;
    BufferStep /= 2;
  }
  BufferSize = BufferStep;
  while (BufferSize+BufferStep <= FFT_size/zeropadding_factor)
    BufferSize += BufferStep;

  // precompute window of the length BufferSize
  if (window != NULL)
    delete [] window;
  window = new DSP_float[BufferSize];
  DSPf_Blackman(BufferSize, window,false);
  //! \bug properly select PSD_scaling_factor
  //PSD_scaling_factor = 1.0/BufferSize;
  if (standard_PSD_scaling == true)
  { // proper representation of noise density
    PSD_scaling_factor = 0;
    for (ind = 0; ind < BufferSize; ind++)
      PSD_scaling_factor += (window[ind] * window[ind]);
    PSD_scaling_factor = 1.0/PSD_scaling_factor;
  }
  else
  { // proper representation of the amplitude of signal component
    PSD_scaling_factor = 0;
    for (ind = 0; ind < BufferSize; ind++)
      PSD_scaling_factor += window[ind];
    // note that constant component amplitude will be doubled
    PSD_scaling_factor = 2.0/(PSD_scaling_factor*PSD_scaling_factor);
  }
  for (ind = 0; ind < BufferSize; ind++)
    window[ind] *= sqrt(PSD_scaling_factor);


  if (tmp_FFT_buffer != NULL)
    delete [] tmp_FFT_buffer;
  tmp_FFT_buffer = new DSP_float[FFT_size];
  memset(tmp_FFT_buffer, 0, FFT_size*sizeof(float));

  SignalSegments = new T_PlotsStack(NoOfPSDslots, BufferStep);
  //SignalSegments.Reset(NoOfPSDslots, BufferStep);
  SignalSegmentsMaps = new T_PlotsStack(NoOfSignalMAPslots, SignalMapSize);

  // ++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  Histograms = new T_PlotsStack(NoOfPSDslots, NoOfHistBins);
  if (A_Histogram != NULL)
  {
    delete [] A_Histogram;
    A_Histogram = NULL;
  }
  A_Histogram = new float[NoOfHistBins];
  memset(A_Histogram, 0, sizeof(float)*NoOfHistBins);

  // ++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  PSDs = new T_PlotsStack(NoOfPSDslots, PSD_size);
  if (A_PSD != NULL)
  {
    delete [] A_PSD;
    A_PSD = NULL;
  }
  A_PSD = new float[PSD_size];
  memset(A_PSD, 0, sizeof(float)*PSD_size);
  if (A_PSD_dB != NULL)
  {
    delete [] A_PSD_dB;
    A_PSD_dB = NULL;
  }
  A_PSD_dB = new float[PSD_size];
  memset(A_PSD_dB, 0, sizeof(float)*PSD_size);

  // call FFT to initialize FFT block
  if (tmp_FFT_out_buffer != NULL)
    delete [] tmp_FFT_out_buffer;
  tmp_FFT_out_buffer = new DSP_float[PSD_size];
  PSDs_counter = 0;
  FFT_block.absFFTR(FFT_size, tmp_FFT_out_buffer, tmp_FFT_buffer, true);
  if (tmp_FFT_out_buffer2 != NULL)
    delete [] tmp_FFT_out_buffer2;
  tmp_FFT_out_buffer2 = new DSP_float[PSD_size];


  //! BufferStep is to small use more segments together
  MorseDecoder_options = new TOptions(Fp);
  no_of_morse_decoder_slots = 8;
  morse_decoder_slot = new DSP_float[no_of_morse_decoder_slots*BufferStep];
  morse_decoder_slot_ind = 0;
  MorseDecoder_options->audio_segment_size = no_of_morse_decoder_slots*BufferStep;
  //! \todo user should be able to select max WPM value
  MorseDecoder_options->MinDotLength=(int)(DSPu_MORSEkey::GetDotLength(30, Fp/2) + 0.5);

  morse_text_buffer[0] = 0x00;
  if (MorseDecoder_options->MinDotLength >= MorseDecoder_options->audio_segment_size)
  {
    //! \bug problem
    MorseDecoder = NULL;
  }
  else
  {
    //BufferStep
    //! \bug fill options
    //MorseDecoder = NULL;
    MorseDecoder = new T_MorseDecoder(Fp, MorseDecoder_options);
    LastLockState = E_none;
  }

  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  analysis_buffer = new DSPu_OutputBuffer (BufferSize, 1U,
      DSP_stop_when_full, MasterClock, BufferStep,  // int NotificationsStep_in=,
      T_DSPlib_processing::AnalysisBufferCallback//DSPu_notify_callback_ptr func_ptr=NULL,
      );//int CallbackIdentifier=0);

#ifdef __TEST_CHANNEL_FILTER__
  DSP_connect(ChannelFilter_HPF->Output("out"), analysis_buffer->Input("in"));
#else
  DSP_connect(NoiseAdd->Output("out"), analysis_buffer->Input("in"));
#endif

  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
}

void T_DSPlib_processing::AnalysisBufferCallback(DSP_component_ptr Caller, unsigned int UserDefinedIdentifier)
{
  DSPu_OutputBuffer *buffer;
  DSP_float_ptr temp_slot, temp_hist_slot, temp_psd_slot;
  float hist_factor;
  int ind, index;

  if (UserDefinedIdentifier && CallbackID_signal_mask != 0)
  { // ignore start / stop signaling
    return;
  }

  buffer = (DSPu_OutputBuffer *)(Caller->Convert2Block());

  memset(CurrentObject->tmp_FFT_buffer, 0, CurrentObject->FFT_size*sizeof(float));
  buffer->ReadBuffer(CurrentObject->tmp_FFT_buffer, CurrentObject->BufferSize*sizeof(float), -2, DSP_ST_float);
  //FFT_block.absFFTR(FFT_size, tmp_FFT_out_buffer, tmp_FFT_buffer, true);

  CS_OnDraw.Enter();

  // -------------------------------------------------------- //
  if (CurrentObject->UpdateState != 0)
  {
    if ((CurrentObject->UpdateState & E_US_audio_in_gain) != 0)
    {
      if (CurrentObject->AudioInOff == true)
      {
        if (CurrentObject->AudioInGain != NULL)
          CurrentObject->AudioInGain->SetGain(0.0);
      }
      else
      {
        if (CurrentObject->AudioInGain != NULL)
          CurrentObject->AudioInGain->SetGain(1.0);
      }
      CurrentObject->UpdateState &= (~E_US_audio_in_gain);
    }

    if ((CurrentObject->UpdateState & E_US_ascii_text) != 0)
    {
      CurrentObject->MorseKey->AddChar(' ');
      CurrentObject->MorseKey->AddString(CurrentObject->ascii_text);
      CurrentObject->UpdateState &= (~E_US_ascii_text);
    }

    if ((CurrentObject->UpdateState & E_US_WPM_change) != 0)
    {
      CurrentObject->MorseKey->SetKeyingSpeed(CurrentObject->WPM, CurrentObject->Fp);
      CurrentObject->UpdateState &= (~E_US_WPM_change);
    }

    if ((CurrentObject->UpdateState & E_US_wav_file_open) != 0)
    {
      //! \todo 2011.11.28 - measure time on file opening (does these causes audio of socket buffers stutter)
      CurrentObject->WaveIn->OpenFile(CurrentObject->wav_filename,
          DSP_ST_short, DSP_FT_wav);
      CurrentObject->UpdateState &= (~E_US_wav_file_open);
    }

    if ((CurrentObject->UpdateState & E_US_noise_level) != 0)
    {
      // alfa_n = 1/(SNR_lin+3);
      CurrentObject->NoiseGain->SetGain(CurrentObject->alfa_n);
      // alfa_s = 1 - 3*alfa_n;
      CurrentObject->SignalGain->SetGain(CurrentObject->alfa_s);

      CurrentObject->UpdateState &= (~E_US_noise_level);
    }

    if ((CurrentObject->UpdateState & E_US_local_signal) != 0)
    {
      CurrentObject->LocalSignalGain->SetGain(CurrentObject->local_signal_gain);

      CurrentObject->UpdateState &= (~E_US_local_signal);
    }

    if ((CurrentObject->UpdateState & E_US_channel_LPF_coefs) != 0)
    {
      CS_UserData.Enter();
      CurrentObject->ChannelFilter_LPF->SetCoefs(
          CurrentObject->LPF_new_Na, CurrentObject->LPF_new_coefs_a,
          CurrentObject->LPF_new_Nb, CurrentObject->LPF_new_coefs_b);
      if (CurrentObject->LPF_new_coefs_a != NULL)
      {
        delete [] CurrentObject->LPF_new_coefs_a;
        CurrentObject->LPF_new_coefs_a = NULL;
      }
      if (CurrentObject->LPF_new_coefs_b != NULL)
      {
        delete [] CurrentObject->LPF_new_coefs_b;
        CurrentObject->LPF_new_coefs_b = NULL;
      }
      CurrentObject->UpdateState &= (~E_US_channel_LPF_coefs);
      CS_UserData.Leave();
    }

    if ((CurrentObject->UpdateState & E_US_channel_HPF_coefs) != 0)
    {
      CS_UserData.Enter();
      CurrentObject->ChannelFilter_HPF->SetCoefs(
          CurrentObject->HPF_new_Na, CurrentObject->HPF_new_coefs_a,
          CurrentObject->HPF_new_Nb, CurrentObject->HPF_new_coefs_b);
      if (CurrentObject->HPF_new_coefs_a != NULL)
      {
        delete [] CurrentObject->HPF_new_coefs_a;
        CurrentObject->HPF_new_coefs_a = NULL;
      }
      if (CurrentObject->HPF_new_coefs_b != NULL)
      {
        delete [] CurrentObject->HPF_new_coefs_b;
        CurrentObject->HPF_new_coefs_b = NULL;
      }
      CurrentObject->UpdateState &= (~E_US_channel_HPF_coefs);
      CS_UserData.Leave();
    }

    if ((CurrentObject->UpdateState & E_US_morse_receiver_state) != 0)
    {
      if (CurrentObject->MorseReceiverState == false)
      { // reset window state
        //DSPf_InfoMessage("\002", "\000");
        CurrentObject->LastLockState = E_none;
      }
      //! \todo reset MORSE decoder internal state

      CurrentObject->UpdateState &= (~E_US_morse_receiver_state);
    }

  }
  // -------------------------------------------------------- //

  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  // get time slot (BufferStep samples)
  temp_slot = CurrentObject->SignalSegments->GetSlot(true);
  memcpy(temp_slot, CurrentObject->tmp_FFT_buffer, CurrentObject->BufferStep*sizeof(float));
  CurrentObject->SignalSegments->Set_SlotDataSize(CurrentObject->BufferStep);
  // MORSE decoder
  if ((CurrentObject->MorseDecoder != NULL) && (CurrentObject->MorseReceiverState == true))
  {
    memcpy(CurrentObject->morse_decoder_slot + CurrentObject->morse_decoder_slot_ind*CurrentObject->BufferStep,
        temp_slot, CurrentObject->BufferStep*sizeof(float));
    CurrentObject->morse_decoder_slot_ind++;
    CurrentObject->morse_decoder_slot_ind %= CurrentObject->no_of_morse_decoder_slots;

    if (CurrentObject->morse_decoder_slot_ind == 0)
    {
      // reset message buffer
      CurrentObject->morse_text_buffer[0] = 0x00;

      //DSPf_InfoMessage("MorseDecoder->ProcessSegment");
      CurrentObject->MorseDecoder->ProcessSegment(
          CurrentObject->morse_decoder_slot,
          CurrentObject->morse_text_buffer);

      //check status change
      if (CurrentObject->LastLockState != CurrentObject->MorseDecoder->LockState)
      {
        switch (CurrentObject->MorseDecoder->LockState)
        {
          case E_locked:
            DSPf_InfoMessage("\002", "\001");
            break;
          case E_locking:
            DSPf_InfoMessage("\002", "\002");
            break;
          case E_unlocked:
            DSPf_InfoMessage("\002", "\003");
            break;
          default: // E_none
            DSPf_InfoMessage("\002", "\000");
            break;
        }
        CurrentObject->LastLockState = CurrentObject->MorseDecoder->LockState;
      }

      if (strlen(CurrentObject->morse_text_buffer) > 0)
      {
        DSPf_InfoMessage("\001", CurrentObject->morse_text_buffer);
        //CurrentObject->morse_text_buffer[0] = 0x00;
      }
    }
  }
  CurrentObject->SignalSegments->NextSlot(false);

  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  //! \todo get histogram of time slot (always or just when it's needed)
  // --------------------------------------------------------- //
  // aktualizuj uśredniony histogram: usuń ostatni nowy segment
  // koryguj tylko gdy piewrszy będzie usunięty po dodaniu bieżącego histogramu
  //if (CurrentObject->Histograms->Get_SlotDataSize(0) == CurrentObject->NoOfHistBins)
  if (CurrentObject->Histograms->Get_SlotIndex() == (int)CurrentObject->NoOfPSDslots)
  {
    temp_hist_slot = CurrentObject->Histograms->GetSlot(0);
    for (ind = 0; ind < CurrentObject->NoOfHistBins; ind++)
      CurrentObject->A_Histogram[ind] -= temp_hist_slot[ind];
  }
  // --------------------------------------------------------- //
  temp_hist_slot = CurrentObject->Histograms->GetSlot(true);
  //for (ind = 0; ind < CurrentObject->NoOfHistBins; ind++)
  //  temp_hist_slot[ind] = 0;
  memset(temp_hist_slot, 0, CurrentObject->NoOfHistBins*sizeof(DSP_float));
  hist_factor = CurrentObject->NoOfHistBins / 2;
  for (ind = 0; ind < CurrentObject->BufferStep; ind++)
  { // signal range [-1, +1]
    index = int(floor(hist_factor*(temp_slot[ind]+1.0) + 0.5));
    if (index < 0)
      index = 0;
    if (index >= CurrentObject->NoOfHistBins)
      index = CurrentObject->NoOfHistBins-1;
    temp_hist_slot[index]++;
  }
  // --------------------------------------------------------- //
  // aktualizuj uśredniony histogram: dodaj nowy segment
  for (ind = 0; ind < CurrentObject->NoOfHistBins; ind++)
    CurrentObject->A_Histogram[ind] += temp_hist_slot[ind];
  // --------------------------------------------------------- //
  CurrentObject->Histograms->Set_SlotDataSize(CurrentObject->NoOfHistBins);
  CurrentObject->Histograms->NextSlot(false);

  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  // get buffer PSD (windowing + scaling <= in window)
  // --------------------------------------------------------- //
  // windowing
  for (ind = 0; ind < CurrentObject->BufferSize; ind++)
    CurrentObject->tmp_FFT_buffer[ind] *=  CurrentObject->window[ind];
  if (CurrentObject->PSDs_counter == 0)
  {
    CurrentObject->FFT_block.absFFTR(CurrentObject->FFT_size,
        CurrentObject->tmp_FFT_out_buffer, CurrentObject->tmp_FFT_buffer, true);
    // power 2
    for (ind = 0; ind < CurrentObject->PSD_size; ind++)
      CurrentObject->tmp_FFT_out_buffer[ind] *= CurrentObject->tmp_FFT_out_buffer[ind];
  }
  else
  {
    CurrentObject->FFT_block.absFFTR(CurrentObject->FFT_size,
        CurrentObject->tmp_FFT_out_buffer2, CurrentObject->tmp_FFT_buffer, true);
    // power 2
    for (ind = 0; ind < CurrentObject->PSD_size; ind++)
      CurrentObject->tmp_FFT_out_buffer2[ind] *= CurrentObject->tmp_FFT_out_buffer2[ind];
    // average PSD
    for (ind = 0; ind < CurrentObject->PSD_size; ind++)
      CurrentObject->tmp_FFT_out_buffer[ind] += CurrentObject->tmp_FFT_out_buffer2[ind];
  }
  CurrentObject->PSDs_counter++;

  if (CurrentObject->PSDs_counter == CurrentObject->PSDs_per_APSD)
  {
    // --------------------------------------------------------- //
    // aktualizuj uśredniony PSD: akumulacja z przeciekiem
    for (ind = 0; ind < CurrentObject->PSD_size; ind++)
    {
      CurrentObject->A_PSD[ind] *= CurrentObject->A_PSD_factor;
      CurrentObject->A_PSD[ind] += (1-CurrentObject->A_PSD_factor)
                                 * CurrentObject->tmp_FFT_out_buffer[ind];

      if (CurrentObject->A_PSD[ind] > 0.000000000001) // -120 dB
        CurrentObject->A_PSD_dB[ind] = 10*log10(CurrentObject->A_PSD[ind]);
      else
        CurrentObject->A_PSD_dB[ind] = -120.0;
    }

    // --------------------------------------------------------- //
    temp_psd_slot = CurrentObject->PSDs->GetSlot(true);
    //memcpy(temp_slot, CurrentObject->tmp_FFT_out_buffer, CurrentObject->PSD_size*sizeof(float));
    for (ind = 0; ind < CurrentObject->PSD_size; ind++)
    {
      if (CurrentObject->tmp_FFT_out_buffer[ind] > 0.000000000001) // -120 dB
        temp_psd_slot[ind] = 10*log10(CurrentObject->tmp_FFT_out_buffer[ind]);
      else
        temp_psd_slot[ind] = -120.0;
    }

    /*
    float max_, min_;
    for (ind = 0; ind < CurrentObject->PSD_size; ind++)
    {
      if (ind == 0)
      {
        max_ = temp_slot[ind];
        min_ = temp_slot[ind];
      }
      else
      {
        if (max_ < temp_slot[ind])
          max_ = temp_slot[ind];
        if (min_ > temp_slot[ind])
          min_ = temp_slot[ind];
      }
    }
    char text[1024];
    sprintf(text, "PSD_min = %.3f, PSD_max = %.3f", min_, max_);
    DSPf_InfoMessage(text);
    */

    // --------------------------------------------------------- //
    CurrentObject->PSDs->Set_SlotDataSize(CurrentObject->PSD_size);
    CurrentObject->PSDs->NextSlot(false);
    CurrentObject->PSDs_counter = 0;
  }


  if (CurrentObject->AudioIn->GetNoOfFreeBuffers() == 0)
  {
    //#ifdef __DEBUG__
    //  DSPf_ErrorMessage("DisableDrawing");
    //#endif
    MyGLCanvas::DisableDrawing(false);
  }
  else
  {
    MyGLCanvas::EnableDrawing(false);
  }

  //! \todo get time slot mask (for signal drawing)
  CS_OnDraw.Leave();
}

void T_DSPlib_processing::ComputeHighResolutionSpectorgram(void)
{
  DSP_float *FFT_input_buffer = NULL;
  DSP_float *FFT_output_buffer = NULL;
  DSP_float *input_window = NULL;
  DSP_float *temp_float, *temp_slot, *temp_psd_slot;
  DSP_Fourier high_res_FFT;
  int FFT_high_size, Window_size;
  int L, N, N_all;
  int ind, ind2, seg_no;

  //CS_OnDraw.Enter();

  //SignalSegments = new T_PlotsStack(NoOfPSDslots, BufferStep);

  //! compute FFT and window size
  L = 8;
  Window_size = L*BufferStep;
  FFT_high_size = 1024;
  while (FFT_high_size < 4*Window_size)
    FFT_high_size *= 2;
  //! FFT buffer
  FFT_input_buffer = new DSP_float[FFT_high_size];
  PSD_high_size = (FFT_high_size / 2) + 1;
  FFT_output_buffer = new DSP_float[PSD_high_size];

  //! time window
  input_window = new DSP_float[Window_size];
  DSPf_Blackman(Window_size, input_window, false);

  //! number of processing segments
  seg_no = SignalSegments->Get_NoOfSlots() - (L-1);
  if (high_res_PSDs != NULL)
    delete high_res_PSDs;
  high_res_PSDs = new T_PlotsStack(seg_no, PSD_high_size);

  //! compute spectrogram
  for (ind = 0; ind < seg_no; ind++)
  {
    //! get time segment
    temp_float = FFT_input_buffer;
    N_all = 0; N = 0;
    for (ind2 = 0; ind2 < L; ind2++)
    {
      temp_slot = SignalSegments->GetSlot(ind+ind2);
      N = SignalSegments->Get_SlotDataSize(ind+ind2);

      memcpy(temp_float, temp_slot, N*sizeof(DSP_float));
      temp_float += N; N_all += N;
    }
    if (N_all < Window_size)
      memset(FFT_input_buffer, 0x00, FFT_high_size*sizeof(DSP_float));
    else
      memset(temp_float, 0x00, (FFT_high_size-N_all)*sizeof(DSP_float));

    // windowing
    for (ind2 = 0; ind2 < N_all; ind2++)
      FFT_input_buffer[ind2] *=  input_window[ind2];

    //! compute FFT
    high_res_FFT.absFFTR(FFT_high_size, FFT_output_buffer, FFT_input_buffer, true);

    temp_psd_slot = high_res_PSDs->GetSlot(ind);
    for (ind2 = 0; ind2 < PSD_high_size; ind2++)
    {
      if (FFT_output_buffer[ind2] > 0.000000000001) // -120 dB
        temp_psd_slot[ind2] = 10*log10(FFT_output_buffer[ind2]);
      else
        temp_psd_slot[ind2] = -120.0;
    }
    high_res_PSDs->Set_SlotDataSize(PSD_high_size, ind);
    //high_res_PSDs->NextSlot(false);
  }

  if (FFT_input_buffer != NULL)
    delete [] FFT_input_buffer;
  if (FFT_output_buffer != NULL)
    delete [] FFT_output_buffer;
  if (input_window != NULL)
    delete [] input_window;

  //CS_OnDraw.Leave();
}

void T_DSPlib_processing::DestroyAlgorithm(void)
{
  if (AudioIn != NULL)
  {
    delete AudioIn;
    AudioIn = NULL;
  }
  if (DC_notcher != NULL)
  {
    delete DC_notcher;
    DC_notcher = NULL;
  }
  if (AudioInGain != NULL)
  {
    delete AudioInGain;
    AudioInGain = NULL;
  }

  if (CarrierIn != NULL)
  {
    delete CarrierIn;
    CarrierIn = NULL;
  }
  if (MorseKey != NULL)
  {
    delete MorseKey;
    MorseKey = NULL;
  }
  if (MorseMul != NULL)
  {
    delete MorseMul;
    MorseMul = NULL;
  }

  if (WaveIn != NULL)
  {
    delete WaveIn;
    WaveIn = NULL;
  }
  if (Zeroinserter != NULL)
  {
    delete Zeroinserter;
    Zeroinserter = NULL;
  }
  if (WaveIn_LPF != NULL)
  {
    delete WaveIn_LPF;
    WaveIn_LPF = NULL;
  }
  for (int ind = 0; ind < no_of_sos_segments-1; ind++)
  {
    if (WaveIn_sos_LPF[ind] != NULL)
    {
      delete WaveIn_sos_LPF[ind];
      WaveIn_sos_LPF[ind] = NULL;
    }
  }
  if (Decimator != NULL)
  {
    delete Decimator;
    Decimator = NULL;
  }

  if (DigitalSignalsAdd != NULL)
  {
    delete DigitalSignalsAdd;
    DigitalSignalsAdd = NULL;
  }
  if (AllSignalsAdd != NULL)
  {
    delete AllSignalsAdd;
    AllSignalsAdd = NULL;
  }
  if (out_socket != NULL)
  {
    delete out_socket;
    out_socket = NULL;
  }

  if (in_socket != NULL)
  {
    delete in_socket;
    in_socket = NULL;
  }
  if (LocalSignalGain != NULL)
  {
    delete LocalSignalGain;
    LocalSignalGain = NULL;
  }
  if (LocalSignalAdd != NULL)
  {
    delete LocalSignalAdd;
    LocalSignalAdd = NULL;
  }
  if (ChannelFilter_LPF != NULL)
  {
    delete ChannelFilter_LPF;
    ChannelFilter_LPF = NULL;
  }
  if (ChannelFilter_HPF != NULL)
  {
    delete ChannelFilter_HPF;
    ChannelFilter_HPF = NULL;
  }
#ifdef __DOUBLE_CHANNEL_FILTER__
  if (ChannelFilter_LPF2 != NULL)
  {
    delete ChannelFilter_LPF2;
    ChannelFilter_LPF2 = NULL;
  }
  if (ChannelFilter_HPF2 != NULL)
  {
    delete ChannelFilter_HPF2;
    ChannelFilter_HPF2 = NULL;
  }
#endif
  if (NoiseBuffer != NULL)
  {
    delete NoiseBuffer;
    NoiseBuffer = NULL;
  }
  if (NoiseGain != NULL)
  {
    delete NoiseGain;
    NoiseGain = NULL;
  }
  if (SignalGain != NULL)
  {
    delete SignalGain;
    SignalGain = NULL;
  }
  if (NoiseAdd != NULL)
  {
    delete NoiseAdd;
    NoiseAdd = NULL;
  }
  if (AudioOut != NULL)
  {
    delete AudioOut;
    AudioOut = NULL;
  }

  if (MorseDecoder != NULL)
  {
    delete MorseDecoder;
    MorseDecoder = NULL;
  }
  LastLockState = E_none;
  DSPf_InfoMessage("\002", "\000");
  if (MorseDecoder_options != NULL)
  {
    delete MorseDecoder_options;
    MorseDecoder_options = NULL;
  }
  if (morse_decoder_slot != NULL)
  {
    delete morse_decoder_slot;
    morse_decoder_slot = NULL;
  }

  if (analysis_buffer != NULL)
  {
    delete analysis_buffer;
    analysis_buffer = NULL;
  }

  DSPf_InfoMessage("T_DSPlib_processing::DestroyAlgorithm", "blocks deleted");
  DSP_clock::FreeClocks();
  MasterClock = NULL;
  DSPf_InfoMessage("T_DSPlib_processing::DestroyAlgorithm", "clocks deleted");
  DSP_component::ListComponents();
  DSPf_InfoMessage("T_DSPlib_processing::DestroyAlgorithm", "Components listed");

  if (SignalSegments != NULL)
  {
    delete SignalSegments;
    SignalSegments = NULL;
  }
  if (SignalSegmentsMaps != NULL)
  {
    delete SignalSegmentsMaps;
    SignalSegmentsMaps = NULL;
  }
  if (Histograms != NULL)
  {
    delete Histograms;
    Histograms = NULL;
  }
  if (PSDs != NULL)
  {
    delete PSDs;
    PSDs = NULL;
  }
  if (high_res_PSDs != NULL)
  {
    delete high_res_PSDs;
    high_res_PSDs = NULL;
  }

  DSPf_InfoMessage("T_DSPlib_processing::DestroyAlgorithm", "PlotsStacks deleted");

  CurrentObject = NULL;
}

unsigned int T_DSPlib_processing::ElementOverlapSegmentSize(void)
{ return 0; }
unsigned int T_DSPlib_processing::ElementOutputSegmentSize(unsigned int input_segment_size)
{ return input_segment_size; }
unsigned int T_DSPlib_processing::MaxNumberOfProtectedSamples(void)
{ return 0; }

bool T_DSPlib_processing::Process(E_processing_DIR processing_DIR)
{
  DSPe_SocketStatus status;

#ifdef __DEBUG__
  //DSPf_InfoMessage("Before Exec");
#endif
  DSPf_Sleep(0);
  DSPf_Sleep(5);
  DSP_clock::Execute(MasterClock, cycles_per_segment);
  DSPf_Sleep(0);
  DSPf_Sleep(5);
#ifdef __DEBUG__
  //DSPf_InfoMessage("After Exec");
#endif


  if (out_socket != NULL)
  {
    status = out_socket->GetSocketStatus();
    if ((status & DSP_socket_closed) != 0)
    {
      return false;
    }
  }
  if (in_socket != NULL)
  {
    status = in_socket->GetSocketStatus();
    if ((status & DSP_socket_closed) != 0)
    {
      return false;
    }
  }
  return true;
}

