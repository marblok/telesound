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
  std::string address;
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
  wave_in_resample_LPF_b.clear();
  wave_in_resample_LPF_a.clear();

  switch (Fp)
  {
    case 8000:
      Fp_wave_in = 48000;
      L = 1; M = 6;
      resample_LPF_order = 14;
      wave_in_resample_LPF_b.clear();
      wave_in_resample_LPF_a.clear();
      wave_in_resample_LPF_sos_b = wave_in_resample_LPF_b_48000_8000;
      wave_in_resample_LPF_sos_a = wave_in_resample_LPF_a_48000_8000;
      break;
    case 11025:
      Fp_wave_in = 44100;
      L = 1; M = 4;
      resample_LPF_order = 14;
      wave_in_resample_LPF_b.clear();
      wave_in_resample_LPF_a.clear();
      wave_in_resample_LPF_sos_b = wave_in_resample_LPF_b_44100_11025;
      wave_in_resample_LPF_sos_a = wave_in_resample_LPF_a_44100_11025;
      break;
    case 16000:
      Fp_wave_in = 48000;
      L = 1; M = 3;
      resample_LPF_order = 14;
      wave_in_resample_LPF_b.clear();
      wave_in_resample_LPF_a.clear();
      wave_in_resample_LPF_sos_b = wave_in_resample_LPF_b_48000_16000;
      wave_in_resample_LPF_sos_a = wave_in_resample_LPF_a_48000_16000;
      break;
    case 22050:
      Fp_wave_in = 44100;
      L = 1; M = 2;
      resample_LPF_order = 14;
      wave_in_resample_LPF_b = wave_in_resample_LPF_b_44100_22050;
      wave_in_resample_LPF_a = wave_in_resample_LPF_a_44100_22050;
      break;
    case 32000:
      Fp_wave_in = 48000;
      L = 2; M = 3;
      resample_LPF_order = 14;
      wave_in_resample_LPF_b.clear();
      wave_in_resample_LPF_a.clear();
      wave_in_resample_LPF_sos_b = wave_in_resample_LPF_b_48000_32000;
      wave_in_resample_LPF_sos_a = wave_in_resample_LPF_a_48000_32000;
      break;
    case 44100:
      Fp_wave_in = 44100;
      L =1; M = 1;
      resample_LPF_order = 0;
      wave_in_resample_LPF_b.clear();
      wave_in_resample_LPF_a.clear();
      break;
    case 48000:
      Fp_wave_in = 48000;
      L =1; M = 1;
      resample_LPF_order = 0;
      wave_in_resample_LPF_b.clear();
      wave_in_resample_LPF_a.clear();
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
  ModulatorState   = SpecList->modulator_state;
  DemodulatorState = SpecList->demodulator_state;
  ModulatorType = SpecList->modulator_type;
  ModulatorVariant = SpecList->modulator_variant;
  CarrierFreq  = SpecList->carrier_freq;
  DemodulatorCarrierFreq= SpecList->demodulator_carrier_freq;
  DemodulatorDelay= SpecList->demodulator_delay;

  current_constellation = DSP::Complex_vector(0);

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
  // OutSplitter=NULL;
  NoiseBuffer = NULL;
  NoiseGain = NULL;
  SignalGain = NULL;
  NoiseAdd = NULL;
  AudioOut = NULL;

  CurrentObject = NULL;
  //tmp_FFT_buffer = NULL;
  //tmp_FFT_out_buffer = NULL;
  //tmp_FFT_out_buffer2 = NULL;
  window = NULL;

  SignalSegments = NULL;
  SignalSegmentsMaps = NULL;
  Histograms = NULL;
  A_Histogram = NULL;
  PSDs = NULL;
  high_res_PSDs = NULL;
  constellation =NULL;
  eyediagrams=NULL;
  A_PSD = NULL;
  A_PSD_dB = NULL;
  PSDs_counter = 0;

  MorseDecoder = NULL;
  morse_decoder_slot = NULL;

  GraphInitialized = false;

  //! \bug put socket sample type into SpecList and let user select
  CreateAlgorithm(run_as_server, address, Fp, DSP::e::SampleType::ST_short);

  DSP::Component::CheckInputsOfAllComponents();
  DSP::Component::ListComponents(MasterClock);
  DSP::Clock::SchemeToDOTfile(MasterClock, "processing_scheme.dot");
}
void Modulator::create_branch(DSP::Clock_ptr Clock_in, DSP::input &Output_signal, E_ModulatorTypes Modulator_type, float Carrier_freq, unsigned short variant, bool Enable_output){ 
  CarrierFreq = Carrier_freq;
  switch (Modulator_type)
  {
  case E_MT_ASK:
    mod_type=DSP::e::ModulationType::ASK;
    switch (variant)
    {
    case 1:// ASK v1
      L1 = 5;
      M1 = 1;
      L2 = 8;
      M2 = 1;
      bits_per_symbol = 2;
      coef_name_stage1 = "ASK_PSK_1_stage1.coef";
      coef_name_stage2 = "ASK_PSK_1_stage2.coef";
      break;
    case 2: //ASK v2
      L1 = 5;
      M1 = 1;
      L2 = 8;
      M2 = 1;
      bits_per_symbol = 1;
      coef_name_stage1 = "ASK_PSK_2_stage1.coef";
      coef_name_stage2 = "ASK_PSK_2_stage2.coef";

      break;
    }
    break;

  case E_MT_PSK:
    mod_type=DSP::e::ModulationType::PSK;
    switch (variant)
    {
    case 1:// 8-PSK v1 - NSymb = 40,
      L1 = 5;
      M1 = 1;
      L2 = 8;
      M2 = 1;
      bits_per_symbol = 3;
      coef_name_stage1 = "ASK_PSK_1_stage1.coef";
      coef_name_stage2 = "ASK_PSK_1_stage2.coef";
      break;
    case 2: //QPSK v2   Nsymb = 80;
      L1 = 5;
      M1 = 1;
      L2 = 8;
      M2 = 1;
      bits_per_symbol = 2;
      coef_name_stage1 = "ASK_PSK_1_stage1.coef";
      coef_name_stage2 = "ASK_PSK_1_stage2.coef";
      break;
    }
    break;

  case E_MT_FSK:
    switch (variant)
    {
    case 1:
      L1 = 5;
      M1 = 1;
      L2 = 8;
      M2 = 1;
      bits_per_symbol = 1;
      coef_name_stage1 = "ASK_PSK_1_stage1.coef";
      coef_name_stage2 = "ASK_PSK_1_stage2.coef";
      break;
    case 2:
      L1 = 5;
      M1 = 1;
      L2 = 8;
      M2 = 1;
      bits_per_symbol = 1;
      coef_name_stage1 = "ASK_PSK_1_stage1.coef";
      coef_name_stage2 = "ASK_PSK_1_stage2.coef";
      break;
    }
    break;

  case E_MT_QAM:
    mod_type=DSP::e::ModulationType::QAM;
    switch (variant)
    {
    case 1:
      L1 = 5;
      M1 = 1;
      L2 = 8;
      M2 = 1;
      bits_per_symbol = 2;
      coef_name_stage1 = "ASK_PSK_1_stage1.coef";
      coef_name_stage2 = "ASK_PSK_1_stage2.coef";
      break;
    case 2:
      L1 = 5;
      M1 = 1;
      L2 = 8;
      M2 = 1;
      bits_per_symbol = 4;
      coef_name_stage1 = "ASK_PSK_1_stage1.coef";
      coef_name_stage2 = "ASK_PSK_1_stage2.coef";
      break;
    }
    break;
  }

  Interpol2Clock = Clock_in;
  Interpol1Clock = DSP::Clock::GetClock(Interpol2Clock, M2, L2);
  SymbolClock = DSP::Clock::GetClock(Interpol1Clock, M1, L1);
  BitClock = DSP::Clock::GetClock(SymbolClock, bits_per_symbol, 1);

  if (Modulator_type == E_MT_FSK)
  {   
  DSP::LoadCoef coef_info_stage1, coef_info_stage2, coef_info_stage3;
  int N_LPF_stage1, N_LPF_stage2;

    // filtr dla 1. stopnia
    coef_info_stage1.Open(coef_name_stage1, "matlab"); // TODO:change directory to /config.
    N_LPF_stage1 = coef_info_stage1.GetSize(0);
    if (N_LPF_stage1 < 1)
    {
      DSP::log << DSP::e::LogMode::Error << "No filter coeeficients: aborting" << std::endl;
      return;
    }
    else
    {
      coef_info_stage1.Load(h_LPF_stage1);
    }

    // filtr dla 2. stopnia
    coef_info_stage2.Open(coef_name_stage2, "matlab");
    N_LPF_stage2 = coef_info_stage2.GetSize(0);
    if (N_LPF_stage2 < 1)
    {
      DSP::log << DSP::e::LogMode::Error << "No filter coeeficients: aborting" << std::endl;
      return;
    }
    else
    {
      coef_info_stage2.Load(h_LPF_stage2);
    }
    ModS2P.reset(nullptr);
    ModMapper.reset(nullptr);
    ModDDS.reset(nullptr);
    ModMul.reset(nullptr);
    ModBits.reset(new DSP::u::BinRand(SymbolClock, DSP::M_PIx2 * Carrier_freq, DSP::M_PIx2 * (Carrier_freq + 0.2)));
    ModZeroInserter.reset(new DSP::u::Zeroinserter(true, SymbolClock, L1));
    ModZero.reset(new DSP::u::Const(BitClock, 0));
    ModVac.reset(new DSP::u::Vacuum(false, 1U));
    ModFIR.reset(new DSP::u::FIR(Interpol1Clock, h_LPF_stage1));
    ModConverter.reset(new DSP::u::SamplingRateConversion(true, Interpol1Clock, L2, M2, h_LPF_stage2));
    ModDDS.reset(new DSP::u::DDScos(Interpol2Clock));
    ModAmp.reset(new DSP::u::Amplifier((Enable_output) ? 1.0f : 0.0f));
    ModDDS->SetConstInput("ampl", 0.25);
    ModDDS->SetConstInput("phase", 0);
    ModZero->Output("out") >> ModZeroInserter->Input("in.im");
    ModBits->Output("out") >> ModZeroInserter->Input("in.re");
    ModZeroInserter->Output("out") >> ModFIR->Input("in");
    ModFIR->Output("out") >> ModConverter->Input("in");
    ModConverter->Output("out.re") >> ModDDS->Input("puls");
    ModConverter->Output("out.im") >> ModVac->Input("in");
    ModDDS->Output("out") >> ModAmp->Input("in");
    ModAmp->Output("out") >> Output_signal;
  }
  else
  {
    // wczytanie wspolczynników filtrow
    DSP::LoadCoef coef_info_stage1, coef_info_stage2, coef_info_stage3;
    int N_LPF_stage1, N_LPF_stage2, N_LPF_stage3;

    // filtr dla 1. stopnia
    coef_info_stage1.Open(coef_name_stage1, "matlab"); // TODO:change directory to /config.
    N_LPF_stage1 = coef_info_stage1.GetSize(0);
    if (N_LPF_stage1 < 1)
    {
      DSP::log << DSP::e::LogMode::Error << "No filter coeeficients: aborting" << std::endl;
      return;
    }
    else
    {
      coef_info_stage1.Load(h_LPF_stage1);
    }

    // filtr dla 2. stopnia
    coef_info_stage2.Open(coef_name_stage2, "matlab");
    N_LPF_stage2 = coef_info_stage2.GetSize(0);
    if (N_LPF_stage2 < 1)
    {
      DSP::log << DSP::e::LogMode::Error << "No filter coeeficients: aborting" << std::endl;
      return;
    }
    else
    {
      coef_info_stage2.Load(h_LPF_stage2);
    }

    // bloki
    ModBits.reset(new DSP::u::BinRand(BitClock, -1.0f, 1.0f));
    ModS2P.reset(new DSP::u::Serial2Parallel(BitClock, bits_per_symbol));
    ModS2P->SetName("S2P", false);
    ModMapper.reset(new DSP::u::SymbolMapper(mod_type, bits_per_symbol));
    ModMapper->SetName("SymbolMapper", false);

    bool are_symbols_real = ModMapper->isOutputReal();
    ModZero.reset(nullptr);
    if (are_symbols_real)
    {
      ModZero.reset(new DSP::u::Const(SymbolClock, 0.0));
    }

    ModZeroInserter.reset(new DSP::u::Zeroinserter(true, SymbolClock, L1));
    ModFIR.reset(new DSP::u::FIR(Interpol1Clock, h_LPF_stage1));
    ModConverter.reset(new DSP::u::SamplingRateConversion(true, Interpol1Clock, L2, M2, h_LPF_stage2));

    ModDDS.reset(new DSP::u::DDScos(Interpol2Clock, true, 1.0, DSP::M_PIx2 * Carrier_freq));
    ModMul.reset(new DSP::u::Multiplication(0U, 2U));
    ModVac.reset(new DSP::u::Vacuum(false));
    ModAmp.reset(new DSP::u::Amplifier((Enable_output) ? 1.0f : 0.0f));
    // polaczenia
    ModBits->Output("out") >> ModS2P->Input("in");
    ModS2P->Output("out") >> ModMapper->Input("in");
    if (are_symbols_real)
    {
      ModMapper->Output("out") >> ModZeroInserter->Input("in.re");
      ModZero->Output("out") >> ModZeroInserter->Input("in.im");
    }
    else
    {
      ModMapper->Output("out") >> ModZeroInserter->Input("in");
    }
    ModZeroInserter->Output("out") >> ModFIR->Input("in");
    ModFIR->Output("out") >> ModConverter->Input("in");
    ModConverter->Output("out") >> ModMul->Input("in1");
    ModDDS->Output("out") >> ModMul->Input("in2");
    ModMul->Output("out.re") >> ModAmp->Input("in");
    ModMul->Output("out.im") >> ModVac->Input("in");
    ModAmp->Output("out") >> Output_signal;
  }
}

void Modulator::clear_branch(){
  ModBits.reset(nullptr);
  ModS2P.reset(nullptr);
  ModMapper.reset(nullptr);
  ModZero.reset(nullptr);
  ModZeroInserter.reset(nullptr);
  ModFIR.reset(nullptr);
  ModConverter.reset(nullptr);
  ModDDS.reset(nullptr);
  ModMul.reset(nullptr);
  ModAmp.reset(nullptr);
  ModVac.reset(nullptr);
  Interpol2Clock = NULL;
  Interpol1Clock=NULL;
  SymbolClock=NULL;
  BitClock=NULL;
  
  // #ifdef __DEBUG__
  //   DSP::Component::ListOfAllComponents(true);
  // #endif
  }
  void Demodulator::create_branch(DSP::Clock_ptr Clock_in, DSP::input &Constellation, DSP::input &Eyediagram, DSP::output &Input_signal, Modulator &modulator, float carrier_freq, unsigned int input_delay, bool enable){
      Interpol2Clock = Clock_in;
      Interpol1Clock = DSP::Clock::GetClock(Clock_in,modulator.M2,modulator.L2);
      DemodAmp.reset(new DSP::u::Amplifier(((enable) ? 1.0f : 0.0f),1,false));
      DemodDelay.reset(new DSP::u::AdjustableDelay(50,input_delay));
      DemodDDS.reset(new DSP::u::DDScos(Clock_in, true, 1.0, -DSP::M_PIx2 *carrier_freq));
      DemodMul.reset(new DSP::u::Multiplication(1U, 1U));
      DemodConverter.reset(new DSP::u::SamplingRateConversion(true, Clock_in, modulator.M2, modulator.L2, modulator.h_LPF_stage2));
      DemodFIR.reset(new DSP::u::FIR(true, modulator.h_LPF_stage1));
      DemodDecimator.reset(new DSP::u::RawDecimator(true,Interpol1Clock,modulator.L1));



      Input_signal>>DemodAmp->Input("in");
      DemodAmp->Output("out")>> DemodDelay->Input("in");
      DemodDelay->Output("out")>>DemodMul->Input("in1");
      DemodDelay->SetName("max 50 samples");
      DemodDDS->Output("out")>>DemodMul->Input("in2");
      DemodMul->Output("out")>>DemodConverter->Input("in");
      DemodConverter->Output("out")>>DemodFIR->Input("in");
      DemodConverter->Output("out")>>Eyediagram;
      DemodFIR->Output("out")>>DemodDecimator->Input("in");
      DemodDecimator->Output("out")>>Constellation;
  }

  void Demodulator::clear_branch()
  {
      DemodAmp.reset(nullptr);
      DemodDelay.reset(nullptr);
      DemodDDS.reset(nullptr);
      DemodMul.reset(nullptr);
      DemodConverter.reset(nullptr);
      DemodFIR.reset(nullptr);
      DemodDecimator.reset(nullptr);
  }
  T_DSPlib_processing::~T_DSPlib_processing(void)
  {
  DestroyAlgorithm();

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
    GetIIR_LPF_coefs(ChannelFg, Fp, LPF_new_coefs_b, LPF_new_coefs_a);
    LPF_new_Nb = LPF_new_coefs_b.size();
    LPF_new_Na = LPF_new_coefs_a.size();
    if (LPF_new_Na <= 0)
    {
      LPF_new_coefs_a.resize(1);
      LPF_new_coefs_a[0] = 1.0;
    } 
    // coefs_b == NULL is e proper value
    //! \bug implement support for ChannelFilterON

    #ifdef __DEBUG__
    {
      char text[1024];
      int ind;

      sprintf(text, "LPF_new_Nb = %i", LPF_new_Nb);
      DSP::log << text<< std::endl;
      for (ind = 0; ind < LPF_new_Nb; ind++)
      {
        sprintf(text, "  LPF_new_coefs_b[%i] = %.6f", ind, LPF_new_coefs_b[ind]);
        DSP::log << text<< std::endl;
      }
      sprintf(text, "LPF_new_Na = %i", LPF_new_Na);
      DSP::log << text<< std::endl;
      for (ind = 0; ind < LPF_new_Na; ind++)
      {
        sprintf(text, "  LPF_new_coefs_a[%i] = %.6f", ind, LPF_new_coefs_a[ind]);
        DSP::log << text<< std::endl;
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
    GetIIR_HPF_coefs(ChannelFd, Fp, HPF_new_coefs_b, HPF_new_coefs_a);
    HPF_new_Na = HPF_new_coefs_a.size(); 
    HPF_new_Nb = HPF_new_coefs_b.size(); 
    if ( HPF_new_Na == 0)
    {
      HPF_new_coefs_a.resize(1);
      HPF_new_coefs_a[0] = 1.0;
    } // coefs_b == NULL is e proper value
    //! \bug implement support for ChannelFilterON

    #ifdef __DEBUG__
    {
      char text[1024];
      int ind;

      sprintf(text, "HPF_new_Nb = %i", HPF_new_Nb);
      DSP::log << text<< std::endl;
      for (ind = 0; ind < HPF_new_Nb; ind++)
      {
        sprintf(text, "  HPF_new_coefs_b[%i] = %.6f", ind, HPF_new_coefs_b[ind]);
         DSP::log << text<< std::endl;
      }
      sprintf(text, "HPF_new_Na = %i", HPF_new_Na);
      DSP::log << text<< std::endl;
      for (ind = 0; ind < HPF_new_Na; ind++)
      {
        sprintf(text, "  HPF_new_coefs_a[%i] = %.6f", ind, HPF_new_coefs_a[ind]);
        DSP::log << text<< std::endl;
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

    if ((temp_spec->userdata_state & E_US_modulator_state) != 0)
  {
    ModulatorState = temp_spec->modulator_state;
    UpdateState |= E_US_modulator_state;
    temp_spec->userdata_state ^= E_US_modulator_state;
  }   
  if ((temp_spec->userdata_state & E_US_demod_state) != 0)
  {
    DemodulatorState = temp_spec->demodulator_state;
    UpdateState |= E_US_demod_state;
    temp_spec->userdata_state ^= E_US_demod_state;
  }
    if ((temp_spec->userdata_state & E_US_carrier_freq) != 0)
  {
    CarrierFreq = temp_spec->carrier_freq;
    modulator.setCarrierFrequency(temp_spec->carrier_freq/temp_spec->sampling_rate);
    UpdateState |= E_US_carrier_freq;
    temp_spec->userdata_state ^= E_US_carrier_freq;
  }
  if ((temp_spec->userdata_state & E_US_demod_carrier_freq) != 0)
  {
    DemodulatorCarrierFreq = temp_spec->demodulator_carrier_freq;
    demodulator.setCarrierFrequency(temp_spec->demodulator_carrier_freq/temp_spec->sampling_rate);
    UpdateState |= E_US_demod_carrier_freq;
    temp_spec->userdata_state ^= E_US_demod_carrier_freq;
  }
    if ((temp_spec->userdata_state & E_US_demod_delay) != 0)
  {
    
    DemodulatorDelay= temp_spec->demodulator_delay;
    demodulator.setInputDelay(DemodulatorDelay);
    UpdateState |= E_US_demod_delay;
    temp_spec->userdata_state ^= E_US_demod_delay;
  }
    
    if ((temp_spec->userdata_state & E_US_modulator_type) != 0)
  {
    ModulatorType=temp_spec->modulator_type;
    ModulatorVariant=temp_spec->modulator_variant;
    reloadModulator = true;
    if (constellation_buffer != NULL)
    {
      //constellation_buffer->Reset();
      tmp_constellation_buffer.resize(constellation_buffer_size*2);
    }
    UpdateState |= E_US_modulator_type;
    temp_spec->userdata_state ^= E_US_modulator_type;
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
    DSP::log << DSP::e::LogMode::Error << "T_DSPlib_processing::ProcessUserData" << DSP::e::LogMode::second << "unsupported userdata state" << std::endl;
#endif
    temp_spec->userdata_state = E_US_none;
  }
  CS_UserData.Leave();
}

void T_DSPlib_processing::CreateAlgorithm(bool run_as_server, std::string address,
                                          long SamplingRate, DSP::e::SampleType sockets_sample_type)
{
  char tekst[1024];
  int ind;
  DSP::Clock_ptr WaveInClock, WaveInClock2;

  if (MasterClock != NULL)
    DestroyAlgorithm();

  Fp = SamplingRate;

  CurrentObject = this;
  if (run_as_server == true)
  { // server side
    //if (address == NULL)
      address = "0.0.0.0"; // always use this address to avoid problems

    //DSP::f::SetLogState(DSP_LS_file);
#ifdef __DEBUG__
    DSP::log.SetLogFileName("log_file_server.log");
#endif
  }
  else
  {
    if (address.length() == 0)
      address = "127.0.0.1";

    //DSP::f::SetLogState(DSP_LS_file);
#ifdef __DEBUG__
    DSP::log.SetLogFileName("log_file_client.log");
#endif
  }

  DSP::log << DSP::lib_version_string()<< std::endl<< std::endl;

  MasterClock=DSP::Clock::CreateMasterClock();

  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  // wejście 1: mikrofon // możliwośæ wyłączenia w trybie DEBUG
  AudioIn = new DSP::u::AudioInput(MasterClock, Fp, 1, 16);
  if (AudioInOff == true)
    AudioInGain = new DSP::u::Amplifier(0.0);
  else
    AudioInGain = new DSP::u::Amplifier(1.0);
  float factor = 1.99/2.0;
  DSP::Float_vector a_in{1.0f, -0.99f}, b_in{factor*1.0f, factor*-1.0f};


  DC_notcher = new DSP::u::IIR (a_in,b_in);
  DC_notcher->SetName("DC_notcher", false);
  AudioIn->Output("out")>>DC_notcher->Input("in");
  DC_notcher->Output("out")>> AudioInGain->Input("in");

  // wejście 2: kod MORSE'a
  if (run_as_server == true)
    CarrierIn = new DSP::u::DDScos(MasterClock, false, 0.5, DSP::Float(1200*DSP::M_PIx2/Fp));
    //CarrierIn = new DSP::u::DDScos(MasterClock, false, 0.5, DSP::Float(400*DSP::M_PIx2/Fp));
  else
    CarrierIn = new DSP::u::DDScos(MasterClock, false, 0.5, DSP::Float(800*DSP::M_PIx2/Fp));
  MorseKey = new DSP::u::MORSEkey(MasterClock, WPM, Fp);
  MorseKey->LoadCodeTable("config\\polish.mct");
  MorseMul = new DSP::u::RealMultiplication(2);
  CarrierIn->Output("out")>> MorseMul->Input("in1");
  MorseKey->Output("out")>> MorseMul->Input("in2");

  // wejście 3: plik audio ??? <== może tylko na starcie ??
  //! \bug problem z szybkością próbkowania
  //WaveIn = new DSP::u::WaveInput(MasterClock, "test.wav", ".");
  WaveInClock = DSP::Clock::GetClock(MasterClock, M, L);
  WaveInClock2 = DSP::Clock::GetClock(MasterClock, M, 1);
  WaveIn = new DSP::u::FileInput(WaveInClock, "", 1U, DSP::e::SampleType::ST_short, DSP::e::FileType::FT_wav);
  if (L > 1)
  {
    Zeroinserter = new DSP::u::Zeroinserter(WaveInClock, L);
    WaveIn->Output("out")>> Zeroinserter->Input("in");
  }
  if (resample_LPF_order > 0)
  {
    if (!wave_in_resample_LPF_a.empty())
    {
      WaveIn_LPF = new DSP::u::IIR(wave_in_resample_LPF_a, wave_in_resample_LPF_b);

      if (Zeroinserter == NULL)
        WaveIn->Output("out")>> WaveIn_LPF->Input("in");
      else
        Zeroinserter->Output("out")>> WaveIn_LPF->Input("in");
    }
    else
    {
      for (ind = 0; ind < no_of_sos_segments-1; ind++)
      {
        WaveIn_sos_LPF[ind] = new DSP::u::IIR(wave_in_resample_LPF_sos_a[ind], wave_in_resample_LPF_sos_b[ind]);
        if (ind >= 1)
          WaveIn_sos_LPF[ind-1]->Output("out")>> WaveIn_sos_LPF[ind]->Input("in");
      }
      if (L > 1)
      {
        DSP::Float_vector temp_float(3);
        for (ind = 0; ind < 3; ind++)
        {
          temp_float[ind] = L*wave_in_resample_LPF_sos_b[no_of_sos_segments-1][ind];
        }
        WaveIn_LPF = new DSP::u::IIR(wave_in_resample_LPF_sos_a[no_of_sos_segments-1], temp_float);
      }
      else
      {
        WaveIn_LPF = new DSP::u::IIR(wave_in_resample_LPF_sos_a[no_of_sos_segments-1], wave_in_resample_LPF_sos_b[no_of_sos_segments-1]);
      }
      WaveIn_sos_LPF[no_of_sos_segments-2]->Output("out")>> WaveIn_LPF->Input("in");

      if (Zeroinserter == NULL)
        WaveIn->Output("out")>>WaveIn_sos_LPF[0]->Input("in");
      else
        Zeroinserter->Output("out")>> WaveIn_sos_LPF[0]->Input("in");
    }
  }
  if (M > 1)
  {
    Decimator = new DSP::u::RawDecimator(WaveInClock2, M);
    if (WaveIn_LPF == NULL)
    {
      if (Zeroinserter == NULL)
      {
        WaveIn->Output("out")>> Decimator->Input("in");
      }
      else
        Zeroinserter->Output("out")>> Decimator->Input("in");
    }
    else
      WaveIn_LPF->Output("out")>> Decimator->Input("in");
  }

  DigitalSignalsAdd = new DSP::u::Addition(3U, 0U);
  DigitalSignalsAdd->SetName("Add(digital_signals)", false);
  MorseMul->Output("out")>> DigitalSignalsAdd->Input("in1");

  if (Decimator == NULL)
  {
    if (WaveIn_LPF == NULL)
    {
      if (Zeroinserter == NULL)
        WaveIn->Output("out")>> DigitalSignalsAdd->Input("in2");
      else
        Zeroinserter->Output("out")>> DigitalSignalsAdd->Input("in2");
    }
    else
      WaveIn_LPF->Output("out")>> DigitalSignalsAdd->Input("in2");
  }
  else
    Decimator->Output("out")>> DigitalSignalsAdd->Input("in2");

  AllSignalsAdd = new DSP::u::Addition(2U, 0U);
  AllSignalsAdd->SetName("Add(All_signals)", false);
  AudioInGain->Output("out")>> AllSignalsAdd->Input("in1");
  DigitalSignalsAdd->Output("out")>> AllSignalsAdd->Input("in2");

  if (run_as_server == true)
    out_socket = new DSP::u::SocketOutput(address, false, 0x00000002, 1, sockets_sample_type);
  else
    out_socket = new DSP::u::SocketOutput(address, true, 0x00000001, 1, sockets_sample_type);
  AllSignalsAdd->Output("out")>> out_socket->Input("in");

  // wejscie 4 : modulator

  
  modulator.create_branch(MasterClock, DigitalSignalsAdd->Input("in3"), CurrentObject->ModulatorType, CurrentObject->CarrierFreq/Fp, CurrentObject->ModulatorVariant, CurrentObject->ModulatorState);
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  if (run_as_server == true)
    in_socket = new DSP::u::SocketInput(MasterClock, address, false, 0x00000001, 1, sockets_sample_type);
  else
    in_socket = new DSP::u::SocketInput(MasterClock, address, true, 0x00000002, 1, sockets_sample_type);

  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  LocalSignalGain = new DSP::u::Amplifier(local_signal_gain);
  LocalSignalAdd = new DSP::u::Addition(2U, 0U);
  LocalSignalAdd->SetName("LocalSignalAdd", false);
  LocalSignalGain->SetName("Amplifier_(local_signal)", false);
  DigitalSignalsAdd->Output("out") >> LocalSignalGain->Input("in1");
  LocalSignalGain->Output("out") >> LocalSignalAdd->Input("in1");
  in_socket->Output("out") >> LocalSignalAdd->Input("in2");

  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  DSP::Float_vector coefs_b, coefs_a;

  GetIIR_HPF_coefs(ChannelFd, Fp, coefs_b, coefs_a);

  if (coefs_a.size() == 0)
  {
    coefs_a.resize(1);
    coefs_a[0] = 1.0;
  }
  // coefs_b == NULL is e proper value
  //! \bug implement support for ChannelFilterON
  ChannelFilter_HPF = new DSP::u::IIR(coefs_a, coefs_b);
  ChannelFilter_HPF->SetName("Channel_IIR_HPF", false);
#ifdef __DOUBLE_CHANNEL_FILTER__
  ChannelFilter_HPF2 = new DSP::u::IIR(coefs_a, coefs_b);
  ChannelFilter_HPF2->SetName("IIR HPF 2", false);
#endif

  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  GetIIR_LPF_coefs(ChannelFg, Fp, coefs_b, coefs_a);

  if (coefs_a.size() == 0)
  {
    coefs_a.resize(1);
    coefs_a[0] = 1.0;
  } // coefs_b == NULL is e proper value
  //! \bug implement support for ChannelFilterON
  ChannelFilter_LPF = new DSP::u::IIR(coefs_a, coefs_b);
  ChannelFilter_LPF->SetName("Channel_IIR_LPF", false);
#ifdef __DOUBLE_CHANNEL_FILTER__
  ChannelFilter_LPF2 = new DSP::u::IIR(*coefs_a, *coefs_b);
  ChannelFilter_LPF2->SetName("IIR LPF 2", false);
#endif

  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  int noise_len = (int)ceil(4 * Fp * specgram_time_span);
  NoiseBuffer = new DSP::u::InputBuffer(MasterClock, noise_len, 1, DSP::e::BufferType::cyclic);
  DSP::Float_ptr temp_noise = new DSP::Float[noise_len];
  DSP::Randomization::InitRandGenerator(false);
  DSP::Randomization::randn(noise_len, temp_noise);
  NoiseBuffer->WriteBuffer(temp_noise, noise_len * sizeof(DSP::Float));
  NoiseBuffer->SetName("Noise");
  NoiseGain = new DSP::u::Amplifier(alfa_n);
  NoiseGain->SetName("Amplifier_(noise)", false);
  NoiseBuffer->Output("out") >> NoiseGain->Input("in");

  SignalGain = new DSP::u::Amplifier(alfa_s);
  SignalGain->SetName("Amplifier(local_signal)");
  LocalSignalAdd->Output("out") >> SignalGain->Input("in");

  NoiseAdd = new DSP::u::Addition(2U, 0U);
  NoiseAdd->SetName("Add(noise)", false);
  // OutSplitter = new DSP::u::Splitter(3U);
  // OutSplitter->SetName("OutSplitter");
  AudioOut = new DSP::u::AudioOutput(Fp, 1, 16);
  // AudioOut = new DSP::u::Vacuum;

#ifdef __TEST_CHANNEL_FILTER__
  SignalGain->Output("out") >> NoiseAdd->Input("in1");
  NoiseGain->Output("out") >> NoiseAdd->Input("in2");
  NoiseAdd->Output("out") >> ChannelFilter_LPF->Input("in");
#ifdef __DOUBLE_CHANNEL_FILTER__
  ChannelFilter_LPF->Output("out") >> ChannelFilter_LPF2->Input("in");
  ChannelFilter_LPF2->Output("out") >> ChannelFilter_HPF2->Input("in");
  ChannelFilter_HPF2->Output("out") >> ChannelFilter_HPF->Input("in");
  // ChannelFilter_HPF->Output("out") >> OutSplitter->Input("in");
  ChannelFilter_HPF->Output("out") >> AudioOut->Input("in");
#else
  ChannelFilter_LPF->Output("out") >> ChannelFilter_HPF->Input("in");
  // ChannelFilter_HPF->Output("out") >> OutSplitter->Input("in");
  ChannelFilter_HPF->Output("out") >> AudioOut->Input("in");
#endif
#else
  SignalGain->Output("out") >> ChannelFilter_LPF->Input("in");
#ifdef __DOUBLE_CHANNEL_FILTER__
  ChannelFilter_LPF->Output("out") >> ChannelFilter_LPF2->Input("in");
  ChannelFilter_LPF2->Output("out") >> ChannelFilter_HPF2->Input("in");
  ChannelFilter_HPF2->Output("out") >> ChannelFilter_HPF->Input("in");
  ChannelFilter_HPF->Output("out") >> NoiseAdd->Input("in1");
#else
  ChannelFilter_LPF->Output("out") >> ChannelFilter_HPF->Input("in");
  ChannelFilter_HPF->Output("out") >> NoiseAdd->Input("in1");
#endif
  NoiseGain->Output("out") >> NoiseAdd->Input("in2");
  NoiseAdd->Output("out") >> AudioOut->Input("in");
#endif
  DSP::Clock_ptr symbolclock = modulator.getSymbolClock();//temporary solution
  DSP::Clock_ptr interpol1clock = modulator.getInterpol1Clock();//temporary solution
  constellation_buffer = new DSP::u::OutputBuffer(constellation_buffer_size, 2U, DSP::e::BufferType::stop_when_full, symbolclock, -1, T_DSPlib_processing::ConstellationBufferCallback);
  constellation_buffer->SetName("Constellation");
  eyediagram_buffer = new DSP::u::OutputBuffer(eyediagram_buffer_size, 2U, DSP::e::BufferType::stop_when_full, interpol1clock, -1, T_DSPlib_processing::EyeDiagramBufferCallback);
  eyediagram_buffer->SetName("Eye_diagram");
  tmp_constellation_buffer=DSP::Float_vector(constellation_buffer_size*2);
  tmp_eyediagram_buffer=DSP::Float_vector(eyediagram_buffer_size*2);
  demodulator.create_branch(MasterClock, constellation_buffer->Input("in"), eyediagram_buffer->Input("in"),ChannelFilter_HPF->Output("out"), modulator,CurrentObject->DemodulatorCarrierFreq/Fp,CurrentObject->DemodulatorDelay, CurrentObject->DemodulatorState);
  current_constellation = modulator.get_constellation();
  
  
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  // basic assumptions:
  // FFT_size = 1024;
  // NoOfPSDslots = 100;
  // NoOfHistBins = 101;
  // SignalMapSize = 2; //define line for each slot
  // NoOfSignalMAPslots = NoOfPSDslots * 8;
  // specgram_time_span = 2.0; // in [sek]
 
  PSD_size = (FFT_size / 2) + 1;
  PSD_high_size = 0;

  BufferStep = int(specgram_time_span * Fp / NoOfPSDslots);
  PSDs_per_APSD = 1;
  while (BufferStep >= FFT_size / zeropadding_factor)
  {
    PSDs_per_APSD++;
    BufferStep /= 2;
  }
  BufferSize = BufferStep;
  while (BufferSize + BufferStep <= FFT_size / zeropadding_factor)
    BufferSize += BufferStep;

  // precompute window of the length BufferSize
  if (window != NULL)
    delete[] window;
  window = new DSP::Float[BufferSize];
  DSP::f::Blackman(BufferSize, window, false);
  //! \bug properly select PSD_scaling_factor
  // PSD_scaling_factor = 1.0/BufferSize;
  if (standard_PSD_scaling == true)
  { // proper representation of noise density
    PSD_scaling_factor = 0;
    for (ind = 0; ind < BufferSize; ind++)
      PSD_scaling_factor += (window[ind] * window[ind]);
    PSD_scaling_factor = 1.0 / PSD_scaling_factor;
  }
  else
  { // proper representation of the amplitude of signal component
    PSD_scaling_factor = 0;
    for (ind = 0; ind < BufferSize; ind++)
      PSD_scaling_factor += window[ind];
    // note that constant component amplitude will be doubled
    PSD_scaling_factor = 2.0 / (PSD_scaling_factor * PSD_scaling_factor);
  }
  for (ind = 0; ind < BufferSize; ind++)
    window[ind] *= sqrt(PSD_scaling_factor);

  tmp_FFT_buffer = DSP::Float_vector(FFT_size, 0);

  
  // memset(tmp_FFT_buffer, 0, FFT_size*sizeof(float));

  SignalSegments = new T_PlotsStack(NoOfPSDslots, BufferStep);
  // SignalSegments.Reset(NoOfPSDslots, BufferStep);
  SignalSegmentsMaps = new T_PlotsStack(NoOfSignalMAPslots, SignalMapSize);

  // ++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  Histograms = new T_PlotsStack(NoOfPSDslots, NoOfHistBins);
  if (A_Histogram != NULL)
  {
    delete[] A_Histogram;
    A_Histogram = NULL;
  }
  A_Histogram = new float[NoOfHistBins];
  memset(A_Histogram, 0, sizeof(float) * NoOfHistBins);

  // ++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  constellation = new T_PlotsStack(constellation_buffer_size,0);
  eyediagrams = new T_PlotsStack(NoOfPSDslots, BufferStep);
  // ++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  PSDs = new T_PlotsStack(NoOfPSDslots, PSD_size);
  if (A_PSD != NULL)
  {
    delete[] A_PSD;
    A_PSD = NULL;
  }
  A_PSD = new float[PSD_size];
  memset(A_PSD, 0, sizeof(float) * PSD_size);
  if (A_PSD_dB != NULL)
  {
    delete[] A_PSD_dB;
    A_PSD_dB = NULL;
  }
  A_PSD_dB = new float[PSD_size];
  memset(A_PSD_dB, 0, sizeof(float) * PSD_size);

  // call FFT to initialize FFT block

  tmp_FFT_out_buffer = DSP::Float_vector(PSD_size);
  PSDs_counter = 0;
  FFT_block.absFFTR(FFT_size, tmp_FFT_out_buffer, tmp_FFT_buffer, true);
  tmp_FFT_out_buffer2 = DSP::Float_vector(PSD_size);

  //! BufferStep is to small use more segments together
  MorseDecoder_options = new TOptions(Fp);
  no_of_morse_decoder_slots = 8;
  morse_decoder_slot = new DSP::Float[no_of_morse_decoder_slots * BufferStep];
  morse_decoder_slot_ind = 0;
  MorseDecoder_options->audio_segment_size = no_of_morse_decoder_slots * BufferStep;
  //! \todo user should be able to select max WPM value
  MorseDecoder_options->MinDotLength = (int)(DSP::u::MORSEkey::GetDotLength(30, Fp / 2) + 0.5);

  morse_text_buffer[0] = 0x00;
  if (MorseDecoder_options->MinDotLength >= MorseDecoder_options->audio_segment_size)
  {
    //! \bug problem
    MorseDecoder = NULL;
  }
  else
  {
    // BufferStep
    //! \bug fill options
    // MorseDecoder = NULL;
    MorseDecoder = new T_MorseDecoder(Fp, MorseDecoder_options);
    LastLockState = E_none;
  }

  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  analysis_buffer = new DSP::u::OutputBuffer(BufferSize, 1U,
                                             DSP::e::BufferType::stop_when_full, MasterClock, BufferStep, // int NotificationsStep_in=,
                                             T_DSPlib_processing::AnalysisBufferCallback                  // DSP::u::notify_callback_ptr func_ptr=NULL,
  );
  analysis_buffer->SetName("Analysis");                                                                                                 // int CallbackIdentifier=0);
    
#ifdef __TEST_CHANNEL_FILTER__
  // OutSplitter->Output("out2") >> analysis_buffer->Input("in");
  ChannelFilter_HPF->Output("out") >> analysis_buffer->Input("in");
#else
  NoiseAdd->Output("out") >> analysis_buffer->Input("in");
#endif

  // { // diagnostics
  //   DSP::Clock::ListOfAllComponents(true);
  //   std::stringstream ss;
  //   ss << "scheme_" << rand() << ".dot";
  //   DSP::Clock::SchemeToDOTfile(MasterClock, ss.str());
  // }
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
}

void T_DSPlib_processing::AnalysisBufferCallback(DSP::Component_ptr Caller, unsigned int UserDefinedIdentifier)
{
  DSP::u::OutputBuffer *buffer;
  DSP::Float_ptr temp_slot;
  DSP::Float_ptr temp_hist_slot, temp_psd_slot;
  float hist_factor;
  int ind, index;

  if (UserDefinedIdentifier && DSP::CallbackID_signal_mask != 0)
  { // ignore start / stop signaling
    return;
  }

  buffer = (DSP::u::OutputBuffer *)(Caller->Convert2Block());

  // memset(CurrentObject->tmp_FFT_buffer, 0, CurrentObject->FFT_size*sizeof(float));
  CurrentObject->tmp_FFT_buffer.assign(CurrentObject->FFT_size, 0.0f);
  buffer->ReadBuffer(CurrentObject->tmp_FFT_buffer.data(), CurrentObject->BufferSize * sizeof(float), -2, DSP::e::SampleType::ST_float);
  // FFT_block.absFFTR(FFT_size, tmp_FFT_out_buffer, tmp_FFT_buffer, true);

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
                                      DSP::e::SampleType::ST_short, DSP::e::FileType::FT_wav);
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
          CurrentObject->LPF_new_coefs_a,
          CurrentObject->LPF_new_coefs_b);

      CurrentObject->UpdateState &= (~E_US_channel_LPF_coefs);
      CS_UserData.Leave();
    }

    if ((CurrentObject->UpdateState & E_US_channel_HPF_coefs) != 0)
    {
      CS_UserData.Enter();
      CurrentObject->ChannelFilter_HPF->SetCoefs(
          CurrentObject->HPF_new_coefs_a,
          CurrentObject->HPF_new_coefs_b);

      CurrentObject->UpdateState &= (~E_US_channel_HPF_coefs);
      CS_UserData.Leave();
    }

    if ((CurrentObject->UpdateState & E_US_morse_receiver_state) != 0)
    {
      if (CurrentObject->MorseReceiverState == false)
      { // reset window state
        // DSP::log << "\002"<< DSP::e::LogMode::second << "\000"<< std::endl;
        CurrentObject->LastLockState = E_none;
      }
      //! \todo reset MORSE decoder internal state

      CurrentObject->UpdateState &= (~E_US_morse_receiver_state);
    }
    
    if ((CurrentObject->UpdateState & E_US_modulator_state) != 0)
    {
      CurrentObject->modulator.enableOutput(CurrentObject->ModulatorState);
    }
    CurrentObject->UpdateState &= (~E_US_modulator_state);

    if ((CurrentObject->UpdateState & E_US_demod_state) != 0)
    {
      CurrentObject->demodulator.enableInput(CurrentObject->DemodulatorState);
    }
    CurrentObject->UpdateState &= (~E_US_demod_state);

    if ((CurrentObject->UpdateState & E_US_modulator_type) != 0)
    {
      if (CurrentObject->constellation_buffer_size != NULL)
      {
        CurrentObject->constellation_buffer->Reset();
        CurrentObject->tmp_constellation_buffer.assign(CurrentObject->constellation_buffer_size*2, 0);
      }
    }
    CurrentObject->UpdateState &= (~E_US_modulator_type);

    if ((CurrentObject->UpdateState & E_US_demod_carrier_freq) != 0)
    {
      CurrentObject->demodulator.setCarrierFrequency(CurrentObject->DemodulatorCarrierFreq / CurrentObject->Fp);
      
      //DSP::log << "New demodulator carrier frequency: " << CurrentObject->DemodulatorCarrierFreq / CurrentObject->Fp << std::endl;
    }
    CurrentObject->UpdateState &= (~E_US_demod_carrier_freq);

    if ((CurrentObject->UpdateState & E_US_demod_delay) != 0)
    {
      CurrentObject->demodulator.setInputDelay(CurrentObject->DemodulatorDelay);
    }
    CurrentObject->UpdateState &= (~E_US_demod_delay);
  }
  // -------------------------------------------------------- //

  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  // get time slot (BufferStep samples)
  temp_slot = CurrentObject->SignalSegments->GetSlot(true);
  memcpy(temp_slot, CurrentObject->tmp_FFT_buffer.data(), CurrentObject->BufferStep * sizeof(float));
  CurrentObject->SignalSegments->Set_SlotDataSize(CurrentObject->BufferStep);
  // MORSE decoder
  if ((CurrentObject->MorseDecoder != NULL) && (CurrentObject->MorseReceiverState == true))
  {
    memcpy(CurrentObject->morse_decoder_slot + CurrentObject->morse_decoder_slot_ind * CurrentObject->BufferStep, temp_slot, CurrentObject->BufferStep * sizeof(float));

    CurrentObject->morse_decoder_slot_ind++;
    CurrentObject->morse_decoder_slot_ind %= CurrentObject->no_of_morse_decoder_slots;

    if (CurrentObject->morse_decoder_slot_ind == 0)
    {
      // reset message buffer
      CurrentObject->morse_text_buffer[0] = 0x00;

      // DSP::log << "MorseDecoder->ProcessSegment"<< std::endl;
      CurrentObject->MorseDecoder->ProcessSegment(
          CurrentObject->morse_decoder_slot,
          CurrentObject->morse_text_buffer);

      // check status change
      if (CurrentObject->LastLockState != CurrentObject->MorseDecoder->LockState)
      {
        switch (CurrentObject->MorseDecoder->LockState)
        {
        case E_locked:
          DSP::log << "\002" << DSP::e::LogMode::second << "\001" << std::endl;
          break;
        case E_locking:
          DSP::log << "\002" << DSP::e::LogMode::second << "\002" << std::endl;
          break;
        case E_unlocked:
          DSP::log << "\002" << DSP::e::LogMode::second << "\003" << std::endl;
          break;
        default: // E_none
          DSP::log << "\002" << DSP::e::LogMode::second << "\000" << std::endl;
          break;
        }
        CurrentObject->LastLockState = CurrentObject->MorseDecoder->LockState;
      }

      if (strlen(CurrentObject->morse_text_buffer) > 0)
      {
        DSP::log << "\001" << DSP::e::LogMode::second << CurrentObject->morse_text_buffer << std::endl;
        // CurrentObject->morse_text_buffer[0] = 0x00;
      }
    }
  }
  CurrentObject->SignalSegments->NextSlot(false);

  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  //! \todo get histogram of time slot (always or just when it's needed)
  // --------------------------------------------------------- //
  // aktualizuj uśredniony histogram: usuń ostatni nowy segment
  // koryguj tylko gdy piewrszy będzie usunięty po dodaniu bieżącego histogramu
  // if (CurrentObject->Histograms->Get_SlotDataSize(0) == CurrentObject->NoOfHistBins)
  if (CurrentObject->Histograms->Get_SlotIndex() == (int)CurrentObject->NoOfPSDslots)
  {
    temp_hist_slot = CurrentObject->Histograms->GetSlot(0);
    for (ind = 0; ind < CurrentObject->NoOfHistBins; ind++)
      CurrentObject->A_Histogram[ind] -= temp_hist_slot[ind];
  }
  // --------------------------------------------------------- //
  temp_hist_slot = CurrentObject->Histograms->GetSlot(true);
  // for (ind = 0; ind < CurrentObject->NoOfHistBins; ind++)
  //   temp_hist_slot[ind] = 0;
  memset(temp_hist_slot, 0, CurrentObject->NoOfHistBins * sizeof(DSP::Float));
  hist_factor = CurrentObject->NoOfHistBins / 2;
  for (ind = 0; ind < CurrentObject->BufferStep; ind++)
  { // signal range [-1, +1]
    index = int(floor(hist_factor * (temp_slot[ind] + 1.0) + 0.5));
    if (index < 0)
      index = 0;
    if (index >= CurrentObject->NoOfHistBins)
      index = CurrentObject->NoOfHistBins - 1;
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
    CurrentObject->tmp_FFT_buffer[ind] *= CurrentObject->window[ind];
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
      CurrentObject->A_PSD[ind] += (1 - CurrentObject->A_PSD_factor) * CurrentObject->tmp_FFT_out_buffer[ind];

      if (CurrentObject->A_PSD[ind] > 0.000000000001) // -120 dB
        CurrentObject->A_PSD_dB[ind] = 10 * log10(CurrentObject->A_PSD[ind]);
      else
        CurrentObject->A_PSD_dB[ind] = -120.0;
    }

    // --------------------------------------------------------- //
    temp_psd_slot = CurrentObject->PSDs->GetSlot(true);
    // memcpy(temp_slot, CurrentObject->tmp_FFT_out_buffer, CurrentObject->PSD_size*sizeof(float));
    for (ind = 0; ind < CurrentObject->PSD_size; ind++)
    {
      if (CurrentObject->tmp_FFT_out_buffer[ind] > 0.000000000001) // -120 dB
        temp_psd_slot[ind] = 10 * log10(CurrentObject->tmp_FFT_out_buffer[ind]);
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
    DSP::log << text);
    */

    // --------------------------------------------------------- //
    CurrentObject->PSDs->Set_SlotDataSize(CurrentObject->PSD_size);
    CurrentObject->PSDs->NextSlot(false);
    CurrentObject->PSDs_counter = 0;
  }

  if (CurrentObject->AudioIn->GetNoOfFreeBuffers() == 0)
  {
    // #ifdef __DEBUG__
    //   DSP::log << DSP::e::LogMode::Error <<"DisableDrawing");
    // #endif
    MyGLCanvas::DisableDrawing(false);
  }
  else
  {
    MyGLCanvas::EnableDrawing(false);
  }

  //! \todo get time slot mask (for signal drawing)
  CS_OnDraw.Leave();
}
void T_DSPlib_processing::ConstellationBufferCallback(DSP::Component_ptr Caller, unsigned int UserDefinedIdentifier)
{
  DSP::u::OutputBuffer *buffer;
  if (UserDefinedIdentifier && DSP::CallbackID_signal_mask != 0)
  { // ignore start / stop signaling
    return;
  }
  buffer = (DSP::u::OutputBuffer *)(Caller->Convert2Block());
  buffer->ReadBuffer(CurrentObject->tmp_constellation_buffer.data(), CurrentObject->constellation_buffer_size *2* sizeof(DSP::Float),-1, DSP::e::SampleType::ST_float);
}

void T_DSPlib_processing::EyeDiagramBufferCallback(DSP::Component_ptr Caller, unsigned int UserDefinedIdentifier)
{
  DSP::u::OutputBuffer *buffer;
  if (UserDefinedIdentifier && DSP::CallbackID_signal_mask != 0)
  { // ignore start / stop signaling
    return;
  }
  buffer = (DSP::u::OutputBuffer *)(Caller->Convert2Block());
  buffer->ReadBuffer(CurrentObject->tmp_eyediagram_buffer.data(), CurrentObject->eyediagram_buffer_size *2* sizeof(DSP::Float),-1, DSP::e::SampleType::ST_float);
}


void T_DSPlib_processing::ComputeHighResolutionSpectorgram(void)
{
  DSP::Float_vector FFT_input_buffer;
  DSP::Float_vector FFT_output_buffer;
  DSP::Float *input_window = NULL;
  DSP::Float_vector temp_float;
  DSP::Float *temp_slot, *temp_psd_slot;
  DSP::Fourier high_res_FFT;
  int FFT_high_size, Window_size;
  int L, N, N_all;
  int ind, ind2, seg_no;

  // CS_OnDraw.Enter();

  // SignalSegments = new T_PlotsStack(NoOfPSDslots, BufferStep);

  //! compute FFT and window size
  L = 8;
  Window_size = L * BufferStep;
  FFT_high_size = 1024;
  while (FFT_high_size < 4 * Window_size)
    FFT_high_size *= 2;
  //! FFT buffer
  FFT_input_buffer.resize(FFT_high_size);
  PSD_high_size = (FFT_high_size / 2) + 1;
  FFT_output_buffer.resize(PSD_high_size);

  //! time window
  input_window = new DSP::Float[Window_size];
  DSP::f::Blackman(Window_size, input_window, false);

  //! number of processing segments
  seg_no = SignalSegments->Get_NoOfSlots() - (L - 1);
  if (high_res_PSDs != NULL)
    delete high_res_PSDs;
  high_res_PSDs = new T_PlotsStack(seg_no, PSD_high_size);

  //! compute spectrogram
  for (ind = 0; ind < seg_no; ind++)
  {
    //! get time segment
    temp_float = FFT_input_buffer;
    N_all = 0;
    N = 0;
    for (ind2 = 0; ind2 < L; ind2++)
    {
      temp_slot = SignalSegments->GetSlot(ind + ind2);
      N = SignalSegments->Get_SlotDataSize(ind + ind2);

      // memcpy(temp_float, temp_slot, N*sizeof(DSP::Float));
      temp_float.assign(temp_slot, temp_slot + N);

      // temp_float += N;
      N_all += N;
    }
    if (N_all < Window_size)
      // memset(FFT_input_buffer, 0x00, FFT_high_size*sizeof(DSP::Float));
      FFT_input_buffer.assign(FFT_high_size, 0.0);
    else
      // memset(temp_float, 0x00, (FFT_high_size-N_all)*sizeof(DSP::Float));
      temp_float.assign(FFT_high_size - N_all, 0.0);
    // windowing
    for (ind2 = 0; ind2 < N_all; ind2++)
      FFT_input_buffer[ind2] *= input_window[ind2];

    //! compute FFT
    high_res_FFT.absFFTR(FFT_high_size, FFT_output_buffer, FFT_input_buffer, true);

    temp_psd_slot = high_res_PSDs->GetSlot(ind);
    for (ind2 = 0; ind2 < PSD_high_size; ind2++)
    {
      if (FFT_output_buffer[ind2] > 0.000000000001) // -120 dB
        temp_psd_slot[ind2] = 10 * log10(FFT_output_buffer[ind2]);
      else
        temp_psd_slot[ind2] = -120.0;
    }
    high_res_PSDs->Set_SlotDataSize(PSD_high_size, ind);
    // high_res_PSDs->NextSlot(false);
  }

  if (input_window != NULL)
    delete[] input_window;

  // CS_OnDraw.Leave();
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
  for (int ind = 0; ind < no_of_sos_segments - 1; ind++)
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
// // if(OutSplitter!=NULL){
// // delete OutSplitter;

// }
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
  DSP::log << "\002" << DSP::e::LogMode::second << "\000" << std::endl;
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
  if (constellation_buffer != NULL)
  {
    delete constellation_buffer;
    constellation_buffer = NULL;
  }
    if (eyediagram_buffer != NULL)
  {
    delete eyediagram_buffer;
    eyediagram_buffer = NULL;
  }
  DSP::log << "T_DSPlib_processing::DestroyAlgorithm" << DSP::e::LogMode::second << "blocks deleted" << std::endl;

  DSP::Clock::FreeClocks();
  modulator.clear_branch();
  MasterClock = NULL;


  DSP::log << "T_DSPlib_processing::DestroyAlgorithm" << DSP::e::LogMode::second << "clocks deleted" << std::endl;
  DSP::Component::ListComponents();
  DSP::log << "T_DSPlib_processing::DestroyAlgorithm" << DSP::e::LogMode::second << "Components listed" << std::endl;

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
  if (constellation != NULL)
  {
    delete constellation;
    constellation = NULL;
  }
  if (eyediagrams != NULL)
  {
    delete eyediagrams;
    eyediagrams = NULL;
  }

  DSP::log << "T_DSPlib_processing::DestroyAlgorithm" << DSP::e::LogMode::second << "PlotsStacks deleted" << std::endl;

  CurrentObject = NULL;
}

unsigned int T_DSPlib_processing::ElementOverlapSegmentSize(void)
{
  return 0;
}
unsigned int T_DSPlib_processing::ElementOutputSegmentSize(unsigned int input_segment_size)
{
  return input_segment_size;
}
unsigned int T_DSPlib_processing::MaxNumberOfProtectedSamples(void)
{
  return 0;
}

bool T_DSPlib_processing::Process(E_processing_DIR processing_DIR)
{
  DSP::e::SocketStatus status;

#ifdef __DEBUG__
  // DSP::log << "Before Exec"<< std::endl;
#endif
  DSP::f::Sleep(0);
  DSP::f::Sleep(5);
  DSP::Clock::Execute(MasterClock, cycles_per_segment);
  if(reloadModulator){
    demodulator.clear_branch();
    // { // diagnostics
    //   DSP::Clock::ListOfAllComponents(true);
    //   std::stringstream ss;
    //   ss << "scheme_" << rand() << "-demodulator_clear.dot";
    //   DSP::Clock::SchemeToDOTfile(MasterClock, ss.str());
    // }    

    modulator.clear_branch();
    modulator.create_branch(MasterClock, DigitalSignalsAdd->Input("in3"), CurrentObject->ModulatorType, CurrentObject->CarrierFreq/Fp, CurrentObject->ModulatorVariant, CurrentObject->ModulatorState);
    // demodulator.create_branch(MasterClock, constellation_buffer->Input("in"), eyediagram_buffer->Input("in"), OutSplitter->Output("out3"), modulator, CurrentObject->DemodulatorState);
    demodulator.create_branch(MasterClock, constellation_buffer->Input("in"), eyediagram_buffer->Input("in"),ChannelFilter_HPF->Output("out"), modulator,CurrentObject->DemodulatorCarrierFreq/Fp,CurrentObject->DemodulatorDelay, CurrentObject->DemodulatorState);
    current_constellation = modulator.get_constellation();
    reloadModulator=false;

    // { // diagnostics
    //   DSP::Clock::ListOfAllComponents(true);
    //   std::stringstream ss;
    //   ss << "scheme_" << rand() << "-reload.dot";
    //   DSP::Clock::SchemeToDOTfile(MasterClock, ss.str());
    // }    
  }
  DSP::f::Sleep(0);
  DSP::f::Sleep(5);
#ifdef __DEBUG__
  // DSP::log << "After Exec"<< std::endl;
#endif

  // TODO: check if casting to int is correct and necessary
  if (out_socket != NULL)
  {
    status = out_socket->GetSocketStatus();
    if (((int)status & (int)DSP::e::SocketStatus::closed) != 0)
    {
      return false;
    }
  }
  if (in_socket != NULL)
  {
    status = in_socket->GetSocketStatus();
    if (((int)status & (int)DSP::e::SocketStatus::closed) != 0)
    {
      return false;
    }
  }
  return true;
}
