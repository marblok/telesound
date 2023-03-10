/*
 * MorseDecoder.cpp
 *
 *  Created on: 2008-11-06
 *      Author: Marek
 */

#include "MorseDecoder.h"

Tfloat T_ProcessingBuffer::ConstThreshold = 0;
Tfloat T_ProcessingBuffer::max_ = 0;
cvector T_ProcessingBuffer::FFT_temp;
TFourier T_ProcessingBuffer::FFT;
T_ProcessingBuffer *T_ProcessingBuffer::Last;
TBoxFilter T_ProcessingBuffer::NoiseSmoother;

TOptions::TOptions(long Fp)
{
  audio_segment_size = 1024;
  sampling_rate = Fp;

  /*
  UseMultipleSpaces=false;
  UseHardSpace=true;
  WriteMorseTable=false;
  WriteSpeedChange=true;
  UnrecognizedCodeTimeStamp=true;
  PeriodicTimeStamp=false;
  TimeStampPeriod=22.5;
  */
  MinDotLength=160; // 30 WPM
  F_lock_min=600;
  F_lock_max=1600;

  group_pause_factor=1.25;
  group_text_space_factor=0.8;
}

void TOptions::Process(DWORD K, DWORD K_with_zeropadding)
{
  if (F_lock_min<0)
    F_lock_min = 0;
  if (F_lock_min>(sampling_rate-100))
    F_lock_min=(sampling_rate-100);
  if (F_lock_max<(F_lock_min+100))
    F_lock_max=F_lock_min+100;

  //void T_ProcessingBuffer::LockRange2k_range(DWORD K, int SamplingFrequency)
  // k = 2*N/Fp*Fk
  k_lock_min = (int)floor((Tfloat)((F_lock_min*2*K_with_zeropadding))/sampling_rate);
  k_lock_max = (int)ceil((Tfloat)((F_lock_max*2*K_with_zeropadding))/sampling_rate);

  dk_blind=(int)(1+(k_lock_max-k_lock_min)/20); // +/-5%
}

TOptions *T_MorseDecoder::Options = NULL;

T_MorseDecoder::T_MorseDecoder(long SamplingRate, TOptions *Options_in)
{
  int decym_ind, ind;
  int CurrentDecimatedSegmentSize;
  long temp_SamplingRate;

  Options = Options_in;
  Counters_Processor.Parent=this;

  //! change if Fp != 8000 and input segment length != 1024
  LockingRange=5; LockThreshold=2; UnlockRange=64; //32;

  //! \bug this seems not enough
  NoOfDecimationStages = 0;
  temp_SamplingRate = SamplingRate;
  while (temp_SamplingRate >= 2*8000)
  {
    temp_SamplingRate /= 2;
    NoOfDecimationStages++;
  }
  BoxFilterLength = Options->MinDotLength;
  DecimatedSegmentSize = Options->audio_segment_size / 2;
  for (ind=0; ind<NoOfDecimationStages; ind++)
  {
    BoxFilterLength=((BoxFilterLength+1)/2);
    DecimatedSegmentSize/=2;
  }

  MinPulseLength=(short)((BoxFilterLength*3)/4);
  MaxPauseLength=BoxFilterLength*100;

  for (ind=0; ind<ProcessingSegmentsNo; ind++)
  {
    ProcessingBuffers[ind].Resize(Options->audio_segment_size / 2, DecimatedSegmentSize);
  }

  // ++++++++++++++++++++++++++++++++++++++++++ //
  CurrentBufferNo = 0;
  CurrentBuffer = ProcessingBuffers + CurrentBufferNo;
  RecalculationBufferNo = 0;
  RecalculationBuffer = NULL;

  LockState=E_unlocked;

  //DrawFFT_In.resize(Options->audio_segment_size/2);
  // ++++++++++++++++++++++++++++++++++++++++++ //

  if (NoOfDecimationStages>0)
    Decym2= new TDecym2[NoOfDecimationStages];
  else
    Decym2= NULL;
  if (NoOfDecimationStages>1)
    pom_decym= new cvector[NoOfDecimationStages-1];
  else
    pom_decym= NULL;

  //  T_ProcessingBuffer::NoiseSmoother.ReInit(BoxFilterLength, DecimatedSegmentSize);
  T_ProcessingBuffer::NoiseSmoother.ReInit(short(BoxFilterLength/2), DecimatedSegmentSize);

  Demodulator.ReInit();
  Decymacja2.ReInit((short)(pow(2,NoOfDecimationStages)* 5));

  //  BoxFilter1.ReInit(100, 512);
  for (decym_ind=0; decym_ind<NoOfDecimationStages; decym_ind++)
    Decym2[decym_ind].ReInit(10);
  CurrentDecimatedSegmentSize=DecimatedSegmentSize;
  for (decym_ind=short(NoOfDecimationStages-2); decym_ind>=0; decym_ind--)
  {
    CurrentDecimatedSegmentSize*=short(2);
    pom_decym[decym_ind].resize(CurrentDecimatedSegmentSize);
  }
  BoxFilter1.ReInit(BoxFilterLength, DecimatedSegmentSize);
  BoxFilter2.ReInit(short(BoxFilterLength/2), DecimatedSegmentSize);

  // ChangeParameters();
  Options->Process(length(ProcessingBuffers[0].Segment), ProcessingBuffers[0].K_with_zeropadding);
}

//---------------------------------------------------------------------------
void T_MorseDecoder::NextProcessingBuffer(void)
{
  CurrentBufferNo++;
  CurrentBufferNo%=(short)ProcessingSegmentsNo;
  CurrentBuffer=ProcessingBuffers+CurrentBufferNo;
//  LockedProcessingBufferNo++;
//  LockedProcessingBufferNo%=(short)ProcessingSegmentsNo;
//  LockedProcessingBuffer=ProcessingBuffers+LockedProcessingBufferNo;
}

//void T_MorseDecoder::ProcessSegment(short *input_segment, int input_segment_size)
void T_MorseDecoder::ProcessSegment(float *input_segment, char *text_buffer)
{
  short ind, decym_ind;
//  T_CMPO CMPO;

  //AnsiString temp_str;

  if (input_segment == NULL)
    return;

  Decymacja2.filter(Options->audio_segment_size, input_segment, CurrentBuffer->Segment);

  CurrentBuffer->EvaluateFFT();
  CurrentBuffer->FindSpectrumMax(Options->k_lock_min, Options->k_lock_max);
  CurrentBuffer->VerifySpectrumMax(Options->k_lock_min, Options->k_lock_max, Options->dk_blind);

  if (VerifyLockState())
  {
    //recalculation of segments in locking range
    //CurrentProce -= (LockRange+1)
    RecalculationBufferNo=(short)(CurrentBufferNo-LockingRange);
    RecalculationBufferNo+=(short)ProcessingSegmentsNo;
    RecalculationBufferNo%=(short)ProcessingSegmentsNo;

    /* All processing blocks should be reset */
//          BoxFilter1.ReInit(100, 512);
    BoxFilter1.ReInit(BoxFilterLength, DecimatedSegmentSize);
    BoxFilter2.ReInit(short(BoxFilterLength/2), DecimatedSegmentSize);

    for (decym_ind=0; decym_ind<NoOfDecimationStages; decym_ind++)
      Decym2[decym_ind].ReInit(10);
    Counters_Processor.ReInit(MinPulseLength, MaxPauseLength); //??

    for (ind=0; ind<LockingRange; ind++)
    {
      RecalculationBuffer=ProcessingBuffers+RecalculationBufferNo;

      Demodulator.Demodulation(LockedFrequency,
            RecalculationBuffer->Segment,RecalculationBuffer->DemodulatedSegment,
            length(CurrentBuffer->MagnitudeSpectrum));

      if (NoOfDecimationStages>0)
      {
        if (NoOfDecimationStages==1)
          Decym2[0].filter(length(RecalculationBuffer->DemodulatedSegment),
                          RecalculationBuffer->DemodulatedSegment,
                          RecalculationBuffer->DecimatedSegment);
        else
        {
          Decym2[0].filter(length(RecalculationBuffer->DemodulatedSegment),
                          RecalculationBuffer->DemodulatedSegment,
                          pom_decym[0]);

          for (decym_ind=1; decym_ind<NoOfDecimationStages-1; decym_ind++)
            Decym2[decym_ind].filter(length(pom_decym[decym_ind-1]),
                          pom_decym[decym_ind-1],
                          pom_decym[decym_ind]);

          Decym2[NoOfDecimationStages-1].filter(length(pom_decym[NoOfDecimationStages-2]),
                        pom_decym[NoOfDecimationStages-2],
                        RecalculationBuffer->DecimatedSegment);
        }
      }
      else
      {
        RecalculationBuffer->DecimatedSegment=
          RecalculationBuffer->DemodulatedSegment;
      }

//            BoxFilter1.filter(RecalculationBuffer->DecimatedSegment,
//                              RecalculationBuffer->BoxFilter1Segment);
//            BoxFilter2.filter(RecalculationBuffer->DecimatedSegment,
//                              RecalculationBuffer->HalfBoxFilter1Segment);
      BoxFilter2.filter(RecalculationBuffer->DecimatedSegment,
                        RecalculationBuffer->HalfBoxFilter1Segment  ,
                        RecalculationBuffer->BoxFilter1Segment);

//            CMPO.Run2(RecalculationBuffer->DecimatedSegment);

      RecalculationBuffer->Thresholding3(BoxFilterLength, DecimatedSegmentSize); //Thresholding2();
      if (LockState==E_locked)
      {
        /*
        if (MorseCodeTableChange)
        {
          WritePlainTextMorseCodeChange();
          MorseCodeTableChange=false;
        }
        */
        Counters_Processor.ProcessDecitions(RecalculationBuffer->Decitions,
                                      RecalculationBuffer->BoxFilter1Segment);
        Counters_Processor.ProcessLengths(text_buffer);
      }
      RecalculationBufferNo++;
      RecalculationBufferNo%=(short)ProcessingSegmentsNo;
    }
  }
  Demodulator.Demodulation(LockedFrequency,
            CurrentBuffer->Segment,CurrentBuffer->DemodulatedSegment,
            length(CurrentBuffer->MagnitudeSpectrum));

  if (NoOfDecimationStages>0)
  {
    if (NoOfDecimationStages==1)
      Decym2[0].filter(length(CurrentBuffer->DemodulatedSegment),
                      CurrentBuffer->DemodulatedSegment,
                      CurrentBuffer->DecimatedSegment);
    else
    {
      Decym2[0].filter(length(CurrentBuffer->DemodulatedSegment),
                      CurrentBuffer->DemodulatedSegment,
                      pom_decym[0]);

      for (decym_ind=1; decym_ind<NoOfDecimationStages-1; decym_ind++)
        Decym2[decym_ind].filter(length(pom_decym[decym_ind-1]),
                      pom_decym[decym_ind-1],
                      pom_decym[decym_ind]);

      Decym2[NoOfDecimationStages-1].filter(length(pom_decym[NoOfDecimationStages-2]),
                    pom_decym[NoOfDecimationStages-2],
                    CurrentBuffer->DecimatedSegment);
    }
  }
  else
  {
    CurrentBuffer->DecimatedSegment=
      CurrentBuffer->DemodulatedSegment;
  }

//        BoxFilter1.filter(CurrentBuffer->DecimatedSegment,
//                          CurrentBuffer->BoxFilter1Segment);
//        BoxFilter2.filter(CurrentBuffer->DecimatedSegment,
//                          CurrentBuffer->HalfBoxFilter1Segment);
  BoxFilter2.filter(CurrentBuffer->DecimatedSegment,
                    CurrentBuffer->HalfBoxFilter1Segment,
                    CurrentBuffer->BoxFilter1Segment);

//        CMPO.Run2(CurrentBuffer->DecimatedSegment);

  CurrentBuffer->Thresholding3(BoxFilterLength, DecimatedSegmentSize); //Thresholding2();
  if (LockState==E_locked)
  {
    /*
    if (MorseCodeTableChange)
    {
      WritePlainTextMorseCodeChange();
      MorseCodeTableChange=false;
    }
    */
    Counters_Processor.ProcessDecitions(CurrentBuffer->Decitions,
                                  CurrentBuffer->BoxFilter1Segment);
    Counters_Processor.ProcessLengths(text_buffer);
  }

  //Grafika
  /*
  if (AudioInput->DrawingAllowed())
  {
    if (Terminated!=true)
    {
      MainForm->NowDrawing=true;
      Synchronize(Draw_MAIN);
      MainForm->NowDrawing=false;
    }
  }
  */


  NextProcessingBuffer();
}

T_MorseDecoder::~T_MorseDecoder(void)
{
  if (Decym2 != NULL)
    delete [] Decym2;
  if (pom_decym != NULL)
    delete []pom_decym;
}


//**************************************************************//
//**                T_Counters_Processor                      **//
//**************************************************************//
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void T_Counters_Processor::InitHistograms(void)
{
  int ind;

  HistBase.resize(HistogramNoOfBins);
  HistBase[0]=MinHistBase; //1;
  for (ind=1; ind<HistogramNoOfBins; ind++)
  {
    HistBase[ind]= (unsigned long)ceil(HistogramAlfa*HistBase[ind-1]);
  }

  HistNo=2;  //Number of histograms
  HistN=new DWORD[HistNo];          //Number of elements used to create HistBins
  HistBins=new svector[HistNo];
  HistBufLen=new short[HistNo]; //Histogram entry signal buffer length
  HistBuffer=new svector[HistNo]; //Histogram entry signal buffer

  for (ind=0; ind<HistNo; ind++)
  {
    HistN[ind]=0;
    HistBins[ind].resize(HistogramNoOfBins);
    HistBufLen[ind]=20;
    HistBuffer[ind].resize(HistBufLen[ind]);
  }

  ReInitHistograms();
}

void T_Counters_Processor::FreeHistograms(void)
{
  HistNo=0;  //Number of histograms
  delete[] HistN; HistN=NULL;
  delete[] HistBins; HistBins=NULL;
  delete[] HistBufLen; HistBufLen=NULL;
  delete[] HistBuffer; HistBuffer=NULL;
  HistBufLen=new short[HistNo]; //Histogram entry signal buffer length
  HistBuffer=new svector[HistNo]; //Histogram entry signal buffer
}

void T_Counters_Processor::ReInitHistograms(void)
{
  int ind;

  for (ind=0; ind<HistNo; ind++)
  {
    HistN[ind]=0;
    HistBins[ind].resize(0);
    HistBins[ind].resize(HistogramNoOfBins);
    HistBuffer[ind].resize(0);
    HistBuffer[ind].resize(HistBufLen[ind]);
  }
}

void T_Counters_Processor::UpDateHist(short hist, short value)
{
  int ind;
  short temp_value;
  short abs_differ, abs_differ_LAST;

  //remove from the histogram the value HistBuffer[hist].data[0]
  temp_value=HistBuffer[hist].data[0];
  if (temp_value!=0)
  {
    abs_differ_LAST=(short)(abs(long(HistBase[0])-temp_value));
    for (ind=1; ind<HistogramNoOfBins; ind++)
    {
      abs_differ=(short)(abs(long(HistBase[ind])-temp_value));
      if (abs_differ>abs_differ_LAST)
        break;
      abs_differ_LAST=abs_differ;
    }
    HistBins[hist].data[ind-1]--;
    HistN[hist]--;
  }

  memcpy(HistBuffer[hist].data, HistBuffer[hist].data+1, sizeof(short)*(HistBufLen[hist]-1));
  HistBuffer[hist].data[HistBufLen[hist]-1]=value;

  //add to the histogram "value"
  if (value!=0)
  {
    abs_differ_LAST=short(abs(long(HistBase[0])-value));
    for (ind=1; ind<HistogramNoOfBins; ind++)
    {
      abs_differ=short(abs(long(HistBase[ind])-value));
      if (abs_differ>abs_differ_LAST)
        break;
      abs_differ_LAST=abs_differ;
    }
    HistBins[hist].data[ind-1]++;
    HistN[hist]++;
  }

  //Process histogram
  if (hist==1)
  { //histogram of pulse lengths
    int group_state, group_empty_bins;

    //Starting from the end find groups of bins
    //  group of bins:
    //    nonempty bins separated only by HistogramGroupBinsMaxSeparation (before: single)
    //    empty bins

    Hist_group_no=0;
    group_state=0; //looking for group beginning
    group_empty_bins = 0;
    for (ind=HistogramNoOfBins-1; ind>=0; ind--)
    {
      switch (group_state)
      {
        case 0:
          if (HistBins[hist][ind]!=0)
          {
            group_state=1;
            group_empty_bins=0;
            Hist_group_end[Hist_group_no]=short(ind);
            Hist_group_val[Hist_group_no]=HistBins[hist][ind];
          }
          break;
        case 1:
          if (HistBins[hist][ind]==0)
          {
            group_empty_bins++;
            if (group_empty_bins==HistogramGroupBinsMaxSeparation+1) //2)
            {
              //Find last nonempty ??
              Hist_group_start[Hist_group_no]=short(ind+HistogramGroupBinsMaxSeparation+1); //2);
              group_state=0;
              //Ignore groups with less than three pulses
              if (Hist_group_val[Hist_group_no]>2)
                Hist_group_no++;
            }
          }
          else
          {
            Hist_group_val[Hist_group_no]+=HistBins[hist][ind];
            group_empty_bins=0;
          }
          break;
      }
    }
    if (group_state==1)
    {
      Hist_group_start[Hist_group_no]=short(ind+group_empty_bins);
      group_state=0;
      Hist_group_no++;
    }


    //////////////////////////////////////////
    int BinsNo;

    MaxN=0;
    BinsNo=0;
    for (ind=1; ind<HistogramNoOfBins-2; ind++)
//    for (ind=2; ind<HistogramNoOfBins-3; ind++)
    {
      BinsNo+=(HistBins[hist][ind]>0);
      if ((HistBins[hist][ind]+HistBins[hist][ind+1])
        > (4+HistBins[hist][ind-1]+HistBins[hist][ind+2]))
//      if ((HistBins[hist][ind]+HistBins[hist][ind+1])
//        > (4+HistBins[hist][ind-1]+HistBins[hist][ind+2]
//            +HistBins[hist][ind-2]+HistBins[hist][ind+3]))
      {
        Maximas[MaxN]=short(HistBase[ind]);
        MaxN++;
        ind++;
      }
    }
//    MainForm->RadioGroup1->Caption=MaxN;

    if (SpeedInitialized)
    {
      if (Hist_group_no==2)
      {
        dash_len=short((HistBase[Hist_group_start[0]]+HistBase[Hist_group_end[0]])/2);
        dot_len=short((HistBase[Hist_group_start[1]]+HistBase[Hist_group_end[1]])/2);
  //      pulse_threshold=(dot_len+dash_len)/2;

        /*
        if ((pulse_threshold<0.8*short((dot_len*2+dash_len)/3)) ||
            (pulse_threshold>1.2*short((dot_len*2+dash_len)/3)))
          Parent->WritePlainMorseTextSpeed(false);
        */
        pulse_threshold=short((dot_len*2+dash_len)/3);
  //      pulse_threshold=(dot_len*3+dash_len)/4;
        inchar_pause_len=dot_len;
        char_pause_len=dash_len;
        inchar_pause_threshold=short((inchar_pause_len+char_pause_len)/2);
        group_pause_len=short(char_pause_len*T_MorseDecoder::Options->group_pause_factor);
  //      group_pause_threshold=(3*group_pause_len+char_pause_len)/4;
        group_pause_threshold=group_pause_len;
      }
      /*
      if ((MaxN==2) && (BinsNo<7))
      {
        dot_len=Maximas[0];
        dash_len=Maximas[1];
  //      pulse_threshold=(dot_len+dash_len)/2;
        if ((pulse_threshold<0.8*short((dot_len*2+dash_len)/3)) ||
            (pulse_threshold>1.2*short((dot_len*2+dash_len)/3)))
          Parent->WritePlainMorseTextSpeed(false);
        pulse_threshold=short((dot_len*2+dash_len)/3);
  //      pulse_threshold=(dot_len*3+dash_len)/4;
        inchar_pause_len=dot_len;
        char_pause_len=dash_len;
        inchar_pause_threshold=short((inchar_pause_len+char_pause_len)/2);
        group_pause_len=short(char_pause_len*group_pause_factor);
  //      group_pause_threshold=(3*group_pause_len+char_pause_len)/4;
        group_pause_threshold=group_pause_len;
      }
      */
    }
  }
}

void T_Counters_Processor::ReInit(short MinLength, int MaxLength)
{
  MinPulseLength=MinLength;
  MaxPauseLength=MaxLength;

  Counters.N=0;
//  Buffer.State.resize(0);
//  Buffer.Length.resize(0);
  Mode=E_LEM_searching;

  CurrentState=0;
  CurrentLength=0;
  CurrentMax=0;

  M=6;  N=3;
//  min_p=3;   n=2;
  min_p=3;   n=2;
  a=0.2;
  SingleMorseCode[0]=0x00;

  ReInitHistograms();
  SpeedInitialized=false;
}

void  T_Counters_Processor::ProcessBuffer(void)
{
  int ind;

  ind=0;
  while (ind<Buffer.N)
  {
    if (ind == Buffer.N-1)
    {
      if (Buffer.Length[ind]>MaxPauseLength)
      {
        DecodedMorse.Length[DecodedMorse.Counter]=Buffer.Length[ind];
        DecodedMorse.Max[DecodedMorse.Counter]=Buffer.Max[ind];
        DecodedMorse.Morse[DecodedMorse.Counter]=E_unknown_pause;
        DecodedMorse.Counter++;
      }
      break;
    }

    DecodedMorse.Length[DecodedMorse.Counter]=Buffer.Length[ind];
    DecodedMorse.Max[DecodedMorse.Counter]=Buffer.Max[ind];
    if (Buffer.State[ind]==1)
    {
      if (Buffer.Length[ind]<pulse_threshold)
      {
        DecodedMorse.Morse[DecodedMorse.Counter]=E_dot;
        // !!! too short impulses should be added to nearby pauses
        if (Buffer.Length[ind]<dot_len/2)
        {
          if (DecodedMorse.Counter>0)
          {
            if ((DecodedMorse.Morse[DecodedMorse.Counter-1]!=E_dot) &&
                (DecodedMorse.Morse[DecodedMorse.Counter-1]!=E_dash))
            {
              DecodedMorse.Counter--;
              DecodedMorse.Length[DecodedMorse.Counter]+=Buffer.Length[ind];
              if (DecodedMorse.Max[DecodedMorse.Counter]<Buffer.Max[ind])
                DecodedMorse.Max[DecodedMorse.Counter]=Buffer.Max[ind];
            }
          }
          DecodedMorse.Morse[DecodedMorse.Counter]=E_unknown_pause;
        }
        else
        {
/*
          if ((Buffer.Length[ind]>0.8*dot_len) && (Buffer.Length[ind]<1.2*dot_len))
          {
            dot_len*=0.95;
            dot_len+=(0.05*Buffer.Length[ind]);
            pulse_threshold=(dot_len+dash_len)/2;
          }
          else
            dot_len_out++;
*/
        }
      }
      else
      {
        DecodedMorse.Morse[DecodedMorse.Counter]=E_dash;
/*
        if ((Buffer.Length[ind]>0.8*dash_len) && (Buffer.Length[ind]<1.2*dash_len))
        {
          dash_len*=0.95;
          dash_len+=(0.05*Buffer.Length[ind]);
          pulse_threshold=(dot_len+dash_len)/2;
        }
        else
          dash_len_out++;
*/
      }
    }
    else
    {
      if (DecodedMorse.Counter>0)
        if ((DecodedMorse.Morse[DecodedMorse.Counter-1]!=E_dot) &&
            (DecodedMorse.Morse[DecodedMorse.Counter-1]!=E_dash))
        {
          DecodedMorse.Counter--;
          DecodedMorse.Length[DecodedMorse.Counter]+=Buffer.Length[ind];
        }
      DecodedMorse.Morse[DecodedMorse.Counter]=E_unknown_pause;
    }

    if (DecodedMorse.Morse[DecodedMorse.Counter]==E_unknown_pause)
    {
      if (short(DecodedMorse.Length[DecodedMorse.Counter])<inchar_pause_threshold)
      {
        DecodedMorse.Morse[DecodedMorse.Counter]=E_inchar_pause;
/*
        if ((Buffer.Length[ind]>0.6*inchar_pause_len) && (Buffer.Length[ind]<1.2*inchar_pause_len))
        {
          inchar_pause_len*=0.95;
          inchar_pause_len+=(0.05*Buffer.Length[ind]);
//          char_pause_len=dash_len;
          inchar_pause_threshold=(inchar_pause_len+char_pause_len)/2;
        }
        else
          inchar_pause_len_out++;
*/
      }
      else
        if ((short)DecodedMorse.Length[DecodedMorse.Counter]<group_pause_threshold)
        {
          DecodedMorse.Morse[DecodedMorse.Counter]=E_char_pause;
/*
          if ((Buffer.Length[ind]>0.8*char_pause_len) && (Buffer.Length[ind]<1.5*char_pause_len))
          {
            char_pause_len*=0.95;
            char_pause_len+=(0.05*Buffer.Length[ind]);
            inchar_pause_threshold=(inchar_pause_len+char_pause_len)/2;
            group_pause_threshold=(3*group_pause_len+char_pause_len)/4;
          }
          else
            char_pause_len_out++;
*/
        }
        else
        {
/*
          if (DecodedMorse.Length[DecodedMorse.Counter]<2*group_pause_threshold)
          {
            group_pause_len*=0.95;
            group_pause_len+=(0.05*Buffer.Length[ind]);
            group_pause_threshold=(3*group_pause_len+char_pause_len)/4;
          }
*/
        }
    }

    DecodedMorse.Counter++;
    ind++;
  }

  if (Buffer.N>1)
  {
    Buffer.State[0]=Buffer.State[Buffer.N-1];
    Buffer.Length[0]=Buffer.Length[Buffer.N-1];
    Buffer.Max[0]=Buffer.Max[Buffer.N-1];
    Buffer.N=1;
  }

}

//! \todo text_buffer length checking
void T_Counters_Processor::DotDash2Text(char znak, char *text_buffer)
{
  int ind, len;
  int text_len;

  switch (znak)
  {
    case '.':
    case '-':
      len = strlen(SingleMorseCode);
      SingleMorseCode[len] = znak;
      SingleMorseCode[len+1] = 0x00;
      break;

    case 'c':
      len = strlen(SingleMorseCode);
      if (len==0)
      {
        break;
      }
      ind=MorseTable.MorseCodeText2LetterInd(SingleMorseCode);
      if (ind!=-1)
      {
        text_len = strlen(text_buffer);
        text_buffer[text_len] = (char)(MorseTable.CharCode[ind]);
        text_len++;
        if (MorseTable.CharBCode[ind])
        {
          text_buffer[text_len] = (char)(MorseTable.CharBCode[ind]);
          text_len++;
        }
        text_buffer[text_len] = 0x00;
      }
      else
      {
        for (ind=0; ind < len; ind++)
          if (SingleMorseCode[ind]=='-')
            SingleMorseCode[ind]=(char)173;

        text_len = strlen(text_buffer);
        text_buffer[text_len] = '{';
        text_len++;
        strcpy(text_buffer+text_len, SingleMorseCode);
        text_len = strlen(text_buffer);
        text_buffer[text_len] = '}';
        text_len++;
        text_buffer[text_len] = 0x00;
      }
      SingleMorseCode[0]=0x00;
      break;

    case ' ':
      len = strlen(SingleMorseCode);
      if (len==0)
      {
        if ((last_znak == ' ') && (last_znak_counter == 2))
        {
          text_len = strlen(text_buffer);
          text_buffer[text_len] = '\r';
          text_len++;
          text_buffer[text_len] = '\n';
          text_len++;
        }
        break;
      }
      ind=MorseTable.MorseCodeText2LetterInd(SingleMorseCode);
      if (ind!=-1)
      {
        text_len = strlen(text_buffer);
        text_buffer[text_len] = (char)(MorseTable.CharCode[ind]);
        text_len++;
        if (MorseTable.CharBCode[ind])
        {
          text_buffer[text_len] = (char)(MorseTable.CharBCode[ind]);
          text_len++;
        }
      }
      else
      {
        for (ind=0; ind < len; ind++)
          if (SingleMorseCode[ind]=='-')
            SingleMorseCode[ind]=(char)173;

        text_len = strlen(text_buffer);
        text_buffer[text_len] = '{';
        text_len++;
        strcpy(text_buffer+text_len, SingleMorseCode);
        text_len = strlen(text_buffer);
        text_buffer[text_len] = '}';
        text_len++;
      }

      text_buffer[text_len] = ' ';
      text_len++;
      text_buffer[text_len] = 0x00;

      SingleMorseCode[0]=0x00;
      break;
  }

  if (last_znak == znak)
    last_znak_counter++;
  else
    last_znak_counter = 0;
  last_znak = znak;
}

void T_Counters_Processor::RemoveToShort(svector &HistVal, svector &Hist, int Min)
{
  int ind;

  for (ind=0; ind<(short)length(HistVal); ind++)
    if (Hist[ind]<Min)
      HistVal[ind]=0;
}

int T_Counters_Processor::SortHist(svector &HistVal, svector &Hist)
{
  int ind, ind2, ind_maks, max_current_len;
  short maks;

  Hist_temp.resize(0);
  HistVal_temp.resize(0);
  max_current_len=0;
  for (ind=0; ind<(short)length(HistVal); ind++)
  {
    ind_maks=0; maks=HistVal[ind_maks];
    for (ind2=0; ind2<(short)length(HistVal); ind2++)
      if (maks<HistVal[ind2])
      {
        ind_maks=ind2;
        maks=HistVal[ind_maks];
      }
    HistVal_temp[length(HistVal_temp)]=HistVal[ind_maks];
    Hist_temp[length(Hist_temp)]=Hist[ind_maks];
    HistVal[ind_maks]=-1;
    if (max_current_len<Hist[ind])
      max_current_len=Hist[ind];
  }
  Hist=Hist_temp;
  HistVal=HistVal_temp;
  return max_current_len;
}

//============================================================//
void T_Counters_Processor::ProcessLengths(char *text_buffer)
{
  int ind, ind2, MaxCurrentLen;
//  int CurrentPulseInd;
//  short CurrentPulseLength;
  bool found;
  int PulseCounter=0;

  if (Counters.N==0)
  {
    if (CurrentLength>MaxPauseLength)
    {
      Counters.State[0]=0; //CurrentState;
      Counters.Length[0]=(short)CurrentLength;
      Counters.Max[0]=CurrentMax;
      Counters.N=1;
    }
    else
      return;
  }

  //Copy current block into buffer
  //Too short pulses should be eliminated on that stage
//  for (ind=length(Counters.State)-1; ind>=0; ind--)
  for (ind=0; ind<Counters.N; ind++)
  {
    if (Counters.Length[ind]>MaxPauseLength)
    {
      Buffer.State[Buffer.N]=0;
      Buffer.Length[Buffer.N]=Counters.Length[ind];
      Buffer.Max[Buffer.N]=Counters.Max[ind];
    }
    else
    {
      Buffer.State[Buffer.N]=Counters.State[ind];
      Buffer.Length[Buffer.N]=Counters.Length[ind];
      Buffer.Max[Buffer.N]=Counters.Max[ind];
    }

    if (Buffer.N!=0)
    {
      if (Counters.State[ind]==1)
      {
        if (Counters.Length[ind]<MinPulseLength)
        {
          Buffer.N--;
          Buffer.Length[Buffer.N]+=Counters.Length[ind];
          Buffer.Max[Buffer.N]=
            (Buffer.Max[Buffer.N] > Counters.Max[ind])? Buffer.Max[Buffer.N] : Counters.Max[ind];
        }
      }
      else
      {
        if (Buffer.State[Buffer.N-1]==0)
        {
          Buffer.N--;
          Buffer.Length[Buffer.N]+=Counters.Length[ind];
          Buffer.Max[Buffer.N]=
            (Buffer.Max[Buffer.N] > Counters.Max[ind])? Buffer.Max[Buffer.N] : Counters.Max[ind];
        }
      }
    }

    Buffer.N++;
  }

  for (ind=0; ind<Buffer.N; ind++)
  {
    UpDateHist(Buffer.State[ind], Buffer.Length[ind]);
  }

//0) !!! the previous lengths should be considered to allow fast speed change detection
// - check whether most of the pulses in the following N comply with previous
//   dot/dash speeds (within a% range)

  /* Process counters in buffer*/
  switch (Mode)
  {
    case E_LEM_searching:
      if (Buffer.N>=2*M)
      { //We have at least M pulses
        HistVal1.resize(0); Hist1.resize(0);
        ind=0;
    //    while (ind<=2*M)
        while (ind<Buffer.N)
        {
          found=false;
          if ((Buffer.State[ind]==1) && (Buffer.Length[ind]>MinPulseLength))
          {
            for (ind2=0; ind2<(short)length(HistVal1); ind2++)
            {
              if ((Buffer.Length[ind]<Hist1[ind2]*(a+1)) &&
                  (Buffer.Length[ind]>Hist1[ind2]/(a+1)))
              {
                Hist1[ind2]=(short)((Hist1[ind2]*HistVal1[ind2]+Buffer.Length[ind])/(HistVal1[ind2]+1));
                HistVal1[ind2]++;
                found=true;
                break; //ind2=length(HistVal);
              }
            }
            if (found==false)
            {
              HistVal1[length(Hist1)]=1;
              Hist1[length(Hist1)]=Buffer.Length[ind];
            }
            PulseCounter++;
          }
          ind++;
        }

        MaxCurrentLen=SortHist(HistVal1, Hist1);
        RemoveToShort(HistVal1, Hist1, MaxCurrentLen/6);
        MaxCurrentLen=SortHist(HistVal1, Hist1);

        // Get the dot and dash lengths
        if (length(HistVal1)>1)
        {
          //- at least two entries
          // (the two most popular might be the dot and dash lengths)
          // * check whether the sum of HistVal[0] and HistVal[1] is
          //   large enough (low error level)
          // * check whether the each of HistVal[0] and HistVal[1] is
          //   large enough (high certainty level)
          if ((HistVal1[0]+HistVal1[1])>=(min_p*2))
          {
            if ((HistVal1[0]>=min_p) && (HistVal1[1]>=min_p))
            {
              // so we have dot and dash lengths
              dot_len=(Hist1[0]<Hist1[1])?Hist1[0]:Hist1[1];
              dash_len=(Hist1[0]>Hist1[1])?Hist1[0]:Hist1[1];
//              pulse_threshold=(dot_len+dash_len)/2;
              pulse_threshold=(short)((dot_len*2+dash_len)/3);
//              pulse_threshold=(dot_len*3+dash_len)/4;
              inchar_pause_len=dot_len;
              char_pause_len=dash_len;
              inchar_pause_threshold=(short)((inchar_pause_len+char_pause_len)/2);
              group_pause_len=(short)(char_pause_len*T_MorseDecoder::Options->group_pause_factor);
//              group_pause_threshold=(3*group_pause_len+char_pause_len)/4;
              group_pause_threshold=group_pause_len;

              //! \bug
              /*
              Parent->WritePlainMorseTextSpeed(true);
              */

              dot_len_out=0; dash_len_out=0;
              inchar_pause_len_out=0; char_pause_len_out=0;

              SpeedInitialized=true;
              Mode=E_LEM_correcting;
            }
            else
            {
              // - only one entry in HistVal1 is certain
              //  (it's dot or dash length we must wait to get to know that)
            }
          }
          else
          {
            // !!! here should be cut off the first part of the buffer
            //in fact it should be converted into pause

            //This reinit might be in wrong place
            //and if buffer is too long the first part should be discarded
            //ReInit(MinPulseLength, MaxPauseLength);

            if (Buffer.N>40)
            {
              memcpy(Buffer.State.data, Buffer.State.data+(Buffer.N-40), sizeof(char)*(40));
              memcpy(Buffer.Length.data, Buffer.Length.data+(Buffer.N-40), sizeof(short)*(40));
              memcpy(Buffer.Max.data, Buffer.Max.data+(Buffer.N-40), sizeof(Tfloat)*(40));
            }
          }
        }
        else
        {
          // - only one entry in HistVal1
          //  (it's dot or dash length we must wait to get to know that)
        }

        if (Mode==E_LEM_correcting)
        {
          //The dot and Dash lengths have been found so we must process buffer
          ProcessBuffer();
        }
      }
      break;
    case E_LEM_correcting:
      ProcessBuffer();
      break;
  }
/*
  MainForm->Label2->Caption=AnsiString("");
  MainForm->Label2->Caption=MainForm->Label2->Caption+AnsiString(DecodedMorse.Counter);
  MainForm->Label2->Caption=MainForm->Label2->Caption+AnsiString(": ");
  MainForm->Label2->Caption=MainForm->Label2->Caption+AnsiString(pulse_threshold);
  MainForm->Label2->Caption=MainForm->Label2->Caption+AnsiString(": ");
*/

//SingleMorseCode="";
//MainForm->RichEdit1->Clear();
//MainForm->RichEdit1->Lines->Add("x");
  for (ind=0; ind<DecodedMorse.Counter; ind++)
  {
    if (DecodedMorse.Morse[ind]!=E_inchar_pause)
    {
//      MainForm->Label2->Caption=MainForm->Label2->Caption+AnsiString(DecodedMorse.Morse[ind]);

      switch (DecodedMorse.Morse[ind])
      {
        case E_dot:
          DotDash2Text('.', text_buffer);
          break;
        case E_dash:
          DotDash2Text('-', text_buffer);
          break;
        case E_unknown_pause:
          DotDash2Text(' ', text_buffer);
          //! \bug
          /*
          if (T_MorseDecoder::Options->UseMultipleSpaces)
          {
//          space_no=(int)ceil((float)((DecodedMorse.Length[ind]-group_pause_threshold/2)*group_text_space_factor)/group_pause_threshold);
            space_no=(int)ceil((float)((DecodedMorse.Length[ind]-dot_len)*T_MorseDecoder::Options->group_text_space_factor)/group_pause_threshold);
            for (ind_p=1; ind_p<space_no; ind_p++)
              Parent->WriteString2PlainText(Ansi2RTF(' ', Parent->Options->UseHardSpace));
          }
              */
          break;
        case E_char_pause:
          DotDash2Text('c', text_buffer);
          break;
      }
    }

  }
DecodedMorse.Counter=0;

/*
  MainForm->Label3->Caption=AnsiString("");
  MainForm->Label3->Caption=MainForm->Label3->Caption+AnsiString(Buffer.N);
  MainForm->Label3->Caption=MainForm->Label3->Caption+AnsiString(": ");
  MainForm->Label3->Caption=MainForm->Label3->Caption+AnsiString(PulseCounter);
  for (ind=0; ind<(short)length(Hist1); ind++)
  {
    MainForm->Label3->Caption=MainForm->Label3->Caption+AnsiString(" {");
    MainForm->Label3->Caption=MainForm->Label3->Caption+AnsiString(HistVal1[ind]);
    MainForm->Label3->Caption=MainForm->Label3->Caption+AnsiString("/");
    MainForm->Label3->Caption=MainForm->Label3->Caption+AnsiString(Hist1[ind]);
    MainForm->Label3->Caption=MainForm->Label3->Caption+AnsiString("}");
  }
*/
}

void T_Counters_Processor::ProcessDecitions(bvector &decitions, cvector &signal)
{
  DWORD ind;
  char *pom;
  M_complex *envelope;

  Counters.N=0;

  pom=decitions.data;
  envelope=signal.data;

  for (ind=0; ind<decitions.N; ind++)
  {
    if (CurrentState==*pom)
    {
      CurrentLength++;
      if (abs(*envelope)<CurrentMax)
        CurrentMax=abs(*envelope);
    }
    else
    {
      Counters.State[Counters.N]=CurrentState;
      if (CurrentLength>MAXSHORT)
        Counters.Length[Counters.N]=MAXSHORT;
      else
        Counters.Length[Counters.N]=(short)CurrentLength;
      Counters.Max[Counters.N]=CurrentMax;
      Counters.N++;

      CurrentState=*pom;
      CurrentLength=1;
      CurrentMax=abs(*envelope);
    }
    pom++;
    envelope++;
  }
}

void T_Counters_Processor::ProcessDecitions(bvector &decitions, rvector &signal)
{
  DWORD ind;
  char *pom;
  Tfloat *envelope;

  Counters.N=0;

  pom=decitions.data;
  envelope=signal.data;

  for (ind=0; ind<decitions.N; ind++)
  {
    if (CurrentState==*pom)
    {
      CurrentLength++;
      if (*envelope<CurrentMax)
        CurrentMax=*envelope;
    }
    else
    {
      Counters.State[Counters.N]=CurrentState;
      if (CurrentLength>MAXSHORT)
        Counters.Length[Counters.N]=MAXSHORT;
      else
        Counters.Length[Counters.N]=(short)CurrentLength;
      Counters.Max[Counters.N]=CurrentMax;
      Counters.N++;

      CurrentState=*pom;
      CurrentLength=1;
      CurrentMax=*envelope;
    }
    pom++;
    envelope++;
  }
}

//---------------------------------------------------------------------------
void T_ProcessingBuffer::FindSpectrumMax(short k_min, short k_max)
{
  maks_val=0;
  for (short ind=k_min; ind<=k_max; ind++)
  {
    if (MagnitudeSpectrum[ind]>maks_val)
    {
      maks_val=MagnitudeSpectrum[ind];
      maks_ind=ind;
      //! \todo zaimplementowaæ uściślanie częstotliwości (max_ind ==> float)

      float X_0, X_1, X_2;
      X_1 = maks_val;
      if ((ind > k_min) && (ind < k_max))
      {
        //X_0 = MagnitudeSpectrum[(int)maks_ind-1];
        X_0 = MagnitudeSpectrum[ind-1];
        X_2 = MagnitudeSpectrum[ind+1];
      }
      else
      {
        X_0 = 0;
        X_2 = 0;
      }

      // udokładnianie częstotliwości
      X_0 = sqrt(X_0); X_1 = sqrt(X_1); X_2 = sqrt(X_2);
      float df;
      //df = -0.5*beta*(X_2 - X_0)/(X_0 + 2*X_1 + X_2);
      if (X_0 - 2*X_1 + X_2 == 0)
        df = 0;
      else
        df = 0.5*(X_0 - X_2)/(X_0 - 2*X_1 + X_2); // ???
      //df = 0;
      maks_ind += df;
    }
  }
}

void T_ProcessingBuffer::EvaluateFFT(void)
{
  unsigned int ind;

  FFT_temp.resize(length(Segment));
  FFT.resize(length(Segment));

  FFT_temp=Segment;
  for (ind=0; ind<length(Segment); ind++)
    FFT_temp[ind]*=GetBlackman(ind,length(Segment));
  FFT_temp.resize(K_with_zeropadding);

//  TestFFT.FFTR(Out, In);
  FFT.absFFT(MagnitudeSpectrum,FFT_temp);
}

bool T_MorseDecoder::VerifyLockState(void)
{
  int temp_ind;
  int pom_ile;
  int ind_temp;

  switch (LockState)
  {
    case E_unlocked:
      if (ProcessingBuffers[CurrentBufferNo].maks_val>1)
      {
        LockingFrequency=ProcessingBuffers[CurrentBufferNo].maks_ind;
        LockState=E_locking;
//        VerifyLockState();
      }
      break;
    case E_locking:
//      temp_ind=LockProcessingBuffer->maks_ind;
//      temp_ind=(CurrentBufferNo - LockingRange +1)% ProcessingSegmentsNo;
      temp_ind=(CurrentBufferNo + ProcessingSegmentsNo - LockingRange +1)% ProcessingSegmentsNo;
      pom_ile=0;

      for (ind_temp=0; ind_temp<LockingRange; ind_temp++)
      {
        pom_ile+=((abs((int)(LockingFrequency-ProcessingBuffers[temp_ind].maks_ind))<3)
          && (ProcessingBuffers[temp_ind].maks_val>0));
        temp_ind++;
        temp_ind %= ProcessingSegmentsNo;
      }
      if (pom_ile>=LockThreshold)
      {
        LockState=E_locked;
        LockedFrequency=LockingFrequency;
        UnlockCounter=0;
        return true; //recalculation necessary
      }
      else
        if (pom_ile==0)
        {
          LockState=E_unlocked;
          VerifyLockState();
        }
      break;
    case E_locked:
      if ((ProcessingBuffers[CurrentBufferNo].maks_val<0) ||
          (abs((int)(LockedFrequency-ProcessingBuffers[CurrentBufferNo].maks_ind))> (3 * 4)))
//          (abs((int)(LockedFrequency-ProcessingBuffers[CurrentBufferNo].maks_ind))> (3 * ZeropaddingFactor)))
      {
        UnlockCounter++;
        if (UnlockCounter>UnlockRange)
        {
          /*
          WritePlainLostLockFrequency();
          */
          LockState=E_unlocked;
          LockedFrequency=0;
        }
      }
      else
      {
        LockedFrequency=ProcessingBuffers[CurrentBufferNo].maks_ind;
        UnlockCounter=0;
      }
      break;
  }
  return false;
}

void T_ProcessingBuffer::VerifySpectrumMax(short k_min, short k_max, short dk_blind)
{
  short ind;
  Tfloat Tlo_val=0;

  for (ind=k_min; ind<(maks_ind-dk_blind); ind++)
    if (Tlo_val<MagnitudeSpectrum[ind])
      Tlo_val=MagnitudeSpectrum[ind];

  for (ind=k_max; ind>(maks_ind+dk_blind); ind--)
    if (Tlo_val<MagnitudeSpectrum[ind])
      Tlo_val=MagnitudeSpectrum[ind];

  if (Tlo_val>=maks_val/2)
    maks_val*=-1;
}

void T_ProcessingBuffer::Thresholding3(short N, short SegmentSize)
{
  short n;
  short N2, N4;

  N2=short((N+1)/2);
  N4=short((N2+1)/2);

//  SegmentSize=length(BoxFilter1Segment);
  for (n=0; n<SegmentSize; n++)
  {
    //Current HalfBoxFilter1Segment
    Threshold[n]=abs(HalfBoxFilter1Segment[n])/2;

    //HalfBoxFilter1Segment delayed by N
    if (n<N)
    {
      if (Last != NULL)
        if (Threshold[n]<abs(Last->HalfBoxFilter1Segment[SegmentSize+n-N])/2)
          Threshold[n]=abs(Last->HalfBoxFilter1Segment[SegmentSize+n-N])/2;
    }
    else
    {
      if (Threshold[n]<abs(HalfBoxFilter1Segment[n-N])/2)
        Threshold[n]=abs(HalfBoxFilter1Segment[n-N])/2;
    }

    //Noise level
    //noise extraction
    if (n<N4)
    {
      if (Last != NULL)
//        noise[n]=Last->DecimatedSegment[SegmentSize+n-N4]-HalfBoxFilter1Segment[n];
        noise[n]=abs(Last->DecimatedSegment[SegmentSize+n-N4]-HalfBoxFilter1Segment[n]);
      else
        noise[n]=abs(HalfBoxFilter1Segment[n]);
    }
    else
    {
//      noise[n]=DecimatedSegment[n-N4]-HalfBoxFilter1Segment[n];
      noise[n]=abs(DecimatedSegment[n-N4]-HalfBoxFilter1Segment[n]);
    }
  }

  NoiseSmoother.filter(noise, smoothed_noise2);
//  NoiseSmoother.filter(noise, smoothed_noise, smoothed_noise2);
//  smoothed_noise2*=3;


/*
  Tfloat MaxNoise;

  MaxNoise=0;
  for (n=0; n<SegmentSize; n++)
    if (MaxNoise<2*smoothed_noise2[n])
      MaxNoise=2*smoothed_noise2[n];
*/
  Tfloat MeanNoise;

  MeanNoise=0;
  for (n=0; n<SegmentSize; n++)
    MeanNoise+=smoothed_noise2[n];
  MeanNoise/=SegmentSize;

  for (n=0; n<SegmentSize; n++)
  {

//    if (Threshold[n]<MaxNoise)
//      Threshold[n]=MaxNoise;

    if (N<75)
    {
      if (Threshold[n]<sqrt(200/N)*MeanNoise)
        Threshold[n]=sqrt(200/N)*MeanNoise;
    }
    else
    {
      if (Threshold[n]<smoothed_noise2[n])
        Threshold[n]=smoothed_noise2[n];
    }

//    if (Threshold[n]<=0.001)
//      Threshold[n]=0.001;
    Threshold[n]+=0.001;
//    Threshold[n]+=200/N*MeanNoise;


    //Thresholding should be done on the signal delayed by N/4 samples
    if (n<N4)
    {
      if (Last != NULL)
        if (Last->BoxFilter1Segment[SegmentSize+n-N4]>Threshold[n])
//        if (Last->BoxFilter1Segment[SegmentSize+n-N4]-Threshold[n]>0.25*Last->BoxFilter1Segment[SegmentSize+n-N4])
          Decitions[n]=1;
        else
          Decitions[n]=0;
    }
    else
    {
      if (BoxFilter1Segment[n-N4]>Threshold[n])
//      if (BoxFilter1Segment[n-N4]-Threshold[n]>0.25*BoxFilter1Segment[n-N4])
        Decitions[n]=1;
      else
        Decitions[n]=0;
    }

  }

  Last=this;
}

void T_ProcessingBuffer::Resize(short SegmentSize, short DecimatedSegmentSize)
{
  K_with_zeropadding = 128;
  while (K_with_zeropadding < SegmentSize * ZeropaddingFactor)
    K_with_zeropadding *= 2;

  Segment.resize(SegmentSize);
//  BoxFilter1Segment.resize(SegmentSize);
  MagnitudeSpectrum.resize(K_with_zeropadding);
  DemodulatedSegment.resize(SegmentSize);
  DecimatedSegment.resize(DecimatedSegmentSize);
  BoxFilter1Segment.resize(DecimatedSegmentSize);
  HalfBoxFilter1Segment.resize(DecimatedSegmentSize);
  Decitions.resize(DecimatedSegmentSize);
  Threshold.resize(DecimatedSegmentSize);
  noise.resize(DecimatedSegmentSize);

  c_noise.resize(DecimatedSegmentSize);
  smoothed_noise.resize(DecimatedSegmentSize);
  smoothed_noise2.resize(DecimatedSegmentSize);
  smoothed_noise3.resize(DecimatedSegmentSize);

  ConstThreshold=0.25;
  max_=0;
  Last=NULL;
}

