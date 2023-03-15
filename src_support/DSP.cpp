#include <math.h>

#include <wx/wx.h>
#include <wx/mdi.h>
#include <wx/toolbar.h>
#include <wx/notebook.h>
#include <wx/glcanvas.h>
#include <wx/thread.h>


//#define wxAddons_wxTimePickerCtrl
#include "DSP.h"
#include "F_coefs.h"
//#include "main.h"

#include "GUIcalls.h"

/*! \todo_later Compare performance of file read/write files
 *   for standard fopen and windows CreateFile.
 */
T_OutputStackElement::T_OutputStackElement(
    E_OutputBuffeType OutputBufferType_in,
    unsigned int NoOfOutputs,
    unsigned int InitialStackSize)
{
  unsigned int ind, ind2;

  SetName("UNKNOWN", false);

  InputSamplingRate  = 0.0; // not specified yet
  OutputSamplingRate = 0.0; // not specified yet

  NoOfStacks = NoOfOutputs;
  OutputStackSize = InitialStackSize;

  NoOfElementsOnStack = new unsigned int[NoOfStacks];
  OutputStacks = new T_OutputStackElement_ptr_table[NoOfStacks];
  NoOfUnprocessedInputSamples = new unsigned int *[NoOfStacks];
  for (ind = 0; ind < NoOfStacks; ind++)
  {
    NoOfElementsOnStack[ind] = 0;
    OutputStacks[ind] = new T_OutputStackElement_ptr[OutputStackSize];
    NoOfUnprocessedInputSamples[ind] = new unsigned int[OutputStackSize];
    for (ind2 = 0; ind2 < OutputStackSize; ind2++)
    {
      OutputStacks[ind][ind2] = NULL;
      NoOfUnprocessedInputSamples[ind][ind2] = 0;
    }
  }

  // Index of currently locked or recently unlocked segment
  current_output_segment = 0;
  // lock status of the current output segment
  current_output_segment_lock_status = E_LS_none;

  max_input_segment_size = 0;
  no_of_output_segments = 0;
  max_output_segment_size = 0;
  output_buffer_size = 0;
  output_buffer = NULL;
  OutputBufferType = OutputBufferType_in;

  output_segment_starts = NULL;
  output_segment_sizes = NULL;
}

void T_OutputStackElement::PreinitializeElement(unsigned int InputSegmentSize,
                                          unsigned int NoOfOutputSegments)
{
}

void T_OutputStackElement::SetOutputSamplingRate(long double output_sampling_rate)
{
  unsigned int ind, ind2;

  OutputSamplingRate = output_sampling_rate;

  for (ind = 0; ind < NoOfStacks; ind++)
  {
    for (ind2 = 0; ind2 < NoOfElementsOnStack[ind]; ind2++)
    {
      OutputStacks[ind][ind2]->SetInputSamplingRate(output_sampling_rate);
    }
  }
}

const long double T_OutputStackElement::GetOutputSamplingRate(void)
{
  return OutputSamplingRate;
}

void T_OutputStackElement::SetInputSamplingRate(long double input_sampling_rate)
{
  InputSamplingRate = input_sampling_rate;
  SetOutputSamplingRate(InputSamplingRate);
}

void T_OutputStackElement::InitializeElement(
    unsigned int InputSegmentSize, unsigned int NoOfOutputSegments)
{
  DSP::log << "T_OutputStackElement::InitializeElement"<< DSP::e::LogMode::second << GetName()<<endl;

  switch (OutputBufferType)
  {
    case E_OBT_float:
      InitializeElement_<DSP::Float>(InputSegmentSize, NoOfOutputSegments);
      break;
    case E_OBT_complex:
      InitializeElement_<DSP::Complex>(InputSegmentSize, NoOfOutputSegments);
      break;
    default:
      DSP::log << DSP::e::LogMode::Error <<"T_OutputStackElement::InitializeElement"<< DSP::e::LogMode::second <<"Unsupported OutputBufferType"<<endl;
      break;
  }
}

unsigned int T_OutputStackElement::ElementInputSubsegmentSize(void)
{ return 1U; }

template <class T>
void T_OutputStackElement::InitializeElement_(
    unsigned int InputSegmentSize, unsigned int NoOfOutputSegments)
{
  unsigned int ind, ind2;
  unsigned int temp_overlap;

  max_input_segment_size = InputSegmentSize;
  no_of_output_segments = NoOfOutputSegments;
  max_output_segment_size = ElementOutputSegmentSize(max_input_segment_size);

  // Preinitialize output elements
  for (ind = 0; ind < NoOfStacks; ind++)
  {
    for (ind2 = 0; ind2 < NoOfElementsOnStack[ind]; ind2++)
      OutputStacks[ind][ind2]->PreinitializeElement(max_output_segment_size, no_of_output_segments);
  }

  output_segment_overlap = 0;
  for (ind = 0; ind < NoOfStacks; ind++)
  {
    for (ind2 = 0; ind2 < NoOfElementsOnStack[ind]; ind2++)
    {
      temp_overlap = OutputStacks[ind][ind2]->ElementOverlapSegmentSize()
                  + (OutputStacks[ind][ind2]->ElementInputSubsegmentSize() - 1);
      if (output_segment_overlap < temp_overlap)
        output_segment_overlap = temp_overlap;
    }
  }
  // correct max_output_segment_size otherwise overlapping might fail
  if (max_output_segment_size < output_segment_overlap)
  {
    max_output_segment_size = output_segment_overlap;
    DSP::log << DSP::e::LogMode::Error <<"T_OutputStackElement::InitializeElement_"<< DSP::e::LogMode::second << "max_output_segment_size < output_segment_overlap"<<endl;
  }

  if (output_buffer != NULL)
  {
    delete [] output_buffer;
    output_buffer = NULL;
  }

//  output_buffer_size = 2*output_segment_overlap
//                    + (max_output_segment_size * (no_of_output_segments));
/*  reserve memory for one additional samples
 *   to avoid troubles with protected memory (in T_Spectrogram)
 *   when protected memory is used continuously.
 */
  output_buffer_size = 2*output_segment_overlap
                    + (max_output_segment_size * (no_of_output_segments))
                    + MaxNumberOfProtectedSamples();
  if (output_buffer_size > 0)
  {
    //output_buffer = new DSP::Complex [output_buffer_size];
    output_buffer = (BYTE_ptr)new T[output_buffer_size];
    memset(output_buffer, 0, output_buffer_size * sizeof(T));
  }

  // Initialize segments
  if (output_segment_starts != NULL)
    delete [] output_segment_starts;
  output_segment_starts = (BYTE_ptr *)new T *[no_of_output_segments + 3];
  if (output_segment_sizes != NULL)
    delete [] output_segment_sizes;
  output_segment_sizes = new unsigned int[no_of_output_segments + 2];
  output_segment_starts[0] = output_buffer;
  output_segment_sizes [0] = output_segment_overlap;
  for (ind = 0; ind < no_of_output_segments; ind++)
  {
    output_segment_starts[ind+1] = (BYTE_ptr)( ((T *)output_buffer)
                                + output_segment_overlap
                                + (ind * max_output_segment_size) );
    output_segment_sizes [ind+1] = max_output_segment_size;
  }
  output_segment_starts[no_of_output_segments+1] = (BYTE_ptr)( ((T *)output_buffer)
                              + output_segment_overlap
                              + (no_of_output_segments * max_output_segment_size) );
  output_segment_sizes [no_of_output_segments+1] = output_segment_overlap;
  output_segment_starts[no_of_output_segments+2] = (BYTE_ptr)( ((T *)output_buffer)
                              + (2 * output_segment_overlap)
                              + (no_of_output_segments * max_output_segment_size) );

  // initialize current segment
  current_output_segment = no_of_output_segments - 1;
  current_output_segment_lock_status = E_LS_unlocked;
  previous_lock_processing_DIR = E_PD_forward;

  // Initialize output elements
  for (ind = 0; ind < NoOfStacks; ind++)
  {
    for (ind2 = 0; ind2 < NoOfElementsOnStack[ind]; ind2++)
      OutputStacks[ind][ind2]->InitializeElement(max_output_segment_size, no_of_output_segments);
  }
}

void T_OutputStackElement::ReinitializeElement(void)
{
  InitializeElement(max_input_segment_size, no_of_output_segments);
}

void T_OutputStackElement::DeleteStack(void)
{
  unsigned int ind, ind2;

  for (ind = 0; ind < NoOfStacks; ind++)
  {
    for (ind2 = 0; ind2 < NoOfElementsOnStack[ind]; ind2++)
    {
      if (OutputStacks[ind][ind2] != NULL)
        OutputStacks[ind][ind2]->DeleteStack();
    }
  }

  delete this;
}

T_OutputStackElement::~T_OutputStackElement(void)
{
  unsigned int ind;

  if (NoOfElementsOnStack != NULL)
  {
    delete [] NoOfElementsOnStack;
    NoOfElementsOnStack = NULL;
  }
  if (OutputStacks != NULL)
  {
    for (ind = 0; ind < NoOfStacks; ind++)
    {
      if (OutputStacks[ind] != NULL)
      {
        delete [] OutputStacks[ind];
        OutputStacks[ind] = NULL;
      }
    }
    delete [] OutputStacks;
    OutputStacks = NULL;
  }
  if (NoOfUnprocessedInputSamples != NULL)
  {
    for (ind = 0; ind < NoOfStacks; ind++)
    {
      if (NoOfUnprocessedInputSamples[ind] != NULL)
      {
        delete [] NoOfUnprocessedInputSamples[ind];
        NoOfUnprocessedInputSamples[ind] = NULL;
      }
    }
    delete [] NoOfUnprocessedInputSamples;
    NoOfUnprocessedInputSamples = NULL;
  }

  if (output_segment_starts != NULL)
  {
    delete [] output_segment_starts;
    output_segment_starts = NULL;
  }
  if (output_segment_sizes != NULL)
  {
    delete [] output_segment_sizes;
    output_segment_sizes = NULL;
  }

  output_segment_overlap = 0;
  no_of_output_segments = 0;
  max_input_segment_size = 0;
  max_output_segment_size = 0;
  if (output_buffer != NULL)
  {
    delete [] output_buffer;
    output_buffer = NULL;
    output_buffer_size = 0;
  }
}

unsigned int T_OutputStackElement::Process_cplx(E_processing_DIR processing_DIR,
    unsigned int NoOfSamples, DSP::Complex_ptr InputData)
{
  char tekst[1024];

  sprintf(tekst, "No processing for complex input signal has been implemented for: %s", GetName());
  DSP::log << DSP::e::LogMode::Error <<"T_OutputStackElement::Process_cplx"<< DSP::e::LogMode::second << tekst<<endl;;

  return 0;
}
unsigned int T_OutputStackElement::Process_real(E_processing_DIR processing_DIR,
    unsigned int NoOfSamples, DSP::Float_ptr InputData)
{
  char tekst[1024];

  sprintf(tekst, "No processing for real input signal has been implemented for: %s", GetName());
  DSP::log << DSP::e::LogMode::Error <<"T_OutputStackElement::Process_real"<< DSP::e::LogMode::second << tekst<<endl;

  return 0;
}

//DSP::Complex_ptr T_OutputStackElement::LockCurrentSegment(E_processing_DIR processing_DIR)
template<class T>
T *T_OutputStackElement::LockCurrentSegment(E_processing_DIR processing_DIR)
{
  switch (current_output_segment_lock_status)
  {
    case E_LS_unlocked:
      switch (processing_DIR)
      {
        case E_PD_backward:
          // If the processing_DIR has changed we should skip more.
          if (previous_lock_processing_DIR == processing_DIR)
            current_output_segment --;
          else
            // This way buffer always will contain continuous signal segments
            current_output_segment ++;
          current_output_segment %= no_of_output_segments;
          break;

        case E_PD_forward:
          // if the processing_DIR has changed we should skip more
          if (previous_lock_processing_DIR == processing_DIR)
            current_output_segment ++;
          else
            // This way buffer always will contain continuous signal segments
            current_output_segment --;
          current_output_segment %= no_of_output_segments;
          break;

        default:
          return NULL;
      }
      break;
    case E_LS_locked:
    case E_LS_none:
    default:
      return NULL;
      break;
  }

  current_output_segment_lock_status = E_LS_locked;
  previous_lock_processing_DIR = processing_DIR;
  //return (output_buffer
  //    + output_segment_overlap
  //    + (current_output_segment * max_output_segment_size));
  return (T *)(output_segment_starts[current_output_segment+1]);
}

/*! \bug Merge current_segment with previous one if sum of those segment sizes is less then
 *       max_output_segment_size. Though, obviously not on the edges (depending on processing direction).
 *       <== DONE for forward processing.
 */
template<class T>
unsigned int T_OutputStackElement::UnlockCurrentSegment(unsigned int segment_size,
                                                        unsigned int no_of_protected_samples)
{
  T *overlap_segment_start;
  T *segment_start;
  BYTE_ptr temp_segment_end;

  if (segment_size == MAX_OUTPUT_SEGMENT_SIZE)
    segment_size = max_output_segment_size;
  if (segment_size > max_output_segment_size)
    DSP::log << DSP::e::LogMode::Error <<"T_OutputStackElement::UnlockCurrentSegment"<< DSP::e::LogMode::second << "segment_size > max_output_segment_size"<<endl;

  // current_output_segment + 1 <== skip first overlap segment
  output_segment_sizes[current_output_segment+1] = segment_size;
  // update next segment start
  output_segment_starts[current_output_segment+2] = (BYTE_ptr)(
    ((T *)(output_segment_starts[current_output_segment+1])) + segment_size);
  output_segment_sizes[current_output_segment+2] = 0; // mark next segment as invalid

  switch (previous_lock_processing_DIR)
  {
    case E_PD_backward:
      if (current_output_segment == no_of_output_segments-1)
      { // fill the ending overlap segment
        //overlap_segment_start = output_buffer
        //     + output_segment_overlap
        //     + (no_of_output_segments * max_output_segment_size);
        overlap_segment_start = (T *)(output_segment_starts[no_of_output_segments+1]);// start of ending overlap segment
        //segment_start = output_buffer + output_segment_overlap;
        segment_start = (T *)(output_segment_starts[1]);// start of first segment
        memcpy((char *)overlap_segment_start, (char *)segment_start, output_segment_overlap*sizeof(T));
        output_segment_sizes[no_of_output_segments+1] = output_segment_overlap;
        /*! \todo Update output_segment_starts[no_of_output_segments+2]
         *  and check if it points no further then to first position pass
         *  allocated output buffer.
         */
      }
      else
      { // merge segment with previous one if it is possible

      }
      //! \todo  take care of protected samples
      break;
    case E_PD_forward:
      if (current_output_segment == 0)
      {
        temp_segment_end = (BYTE_ptr)(((T *)(output_segment_starts[0]))
            + output_segment_overlap);
        if (output_segment_starts[1] == temp_segment_end)
        { // fill the front overlap segment
          overlap_segment_start = (T *)(output_segment_starts[0]); // first overlap segment
          // point to the last output_segment_overlap samples of the last segment
          segment_start = ((T *)(output_segment_starts[no_of_output_segments+1])) - output_segment_overlap;
          memcpy((char *)overlap_segment_start, (char *)segment_start, output_segment_overlap*sizeof(T));
        }
      }
      else
      { // merge segment with previous one if it is possible
        // Actually not merging but replacing previous segment start with this segment
        // thus simulating that this is the same segment.
        temp_segment_end = (BYTE_ptr)(
            ((T *)(output_segment_starts[0]))
            + output_segment_overlap
            + (current_output_segment * max_output_segment_size)
            ); // <== points to predicted beginning of the current segment
        if (output_segment_starts[current_output_segment+2] < temp_segment_end)
        {
          // replace previous segment data with the data of the current segment
          output_segment_sizes[current_output_segment] = output_segment_sizes[current_output_segment+1];
          output_segment_starts[current_output_segment] = output_segment_starts[current_output_segment+1];
          // move back to previous segment
          current_output_segment--;
          // update next segment start
          output_segment_starts[current_output_segment+2] = (BYTE_ptr)(
              ((T *)(output_segment_starts[current_output_segment])) + output_segment_sizes[current_output_segment]);
          output_segment_sizes[current_output_segment+2] = 0; // mark next segment as invalid
        }
        else
        {
          if (current_output_segment == no_of_output_segments-1)
            output_segment_starts[1] = (BYTE_ptr)(((T *)(output_segment_starts[0]))
                + output_segment_overlap);
        }
      }

      // take care of protected samples
      if ((current_output_segment == no_of_output_segments-1) &&
          (no_of_protected_samples != 0))
      {
        overlap_segment_start = (T *)(output_segment_starts[1]); // first segment
        segment_start = ((T *)(output_segment_starts[no_of_output_segments+1])); // past the last segment
        memcpy((char *)overlap_segment_start, (char *)segment_start, no_of_protected_samples*sizeof(T));
      }
      break;
    default:
      DSP::log << DSP::e::LogMode::Error <<"T_OutputStackElement::UnlockCurrentSegment"<<DSP::e::LogMode::second<<"Unexpected state of previous_lock_processing_DIR variable"<<endl;
      break;
  }

  current_output_segment_lock_status = E_LS_unlocked;
  return current_output_segment;
}

unsigned int T_OutputStackElement::GetOutputSegmentSize(int segment_index)
{
  if (segment_index == -2)
    return max_output_segment_size;
  if (segment_index == -1)
    return output_segment_sizes[current_output_segment+1];

  return output_segment_sizes[segment_index+1];
}

unsigned int T_OutputStackElement::GetMaxInputSegmentSize(void)
{ return max_input_segment_size; }


// Allocates twice as large stack as before.
void T_OutputStackElement::IncreaseStackSize(void)
{
  unsigned int ind, ind2;
  T_OutputStackElement_ptr_table temp_stack;

  for (ind = 0; ind < NoOfStacks; ind++)
  {
    temp_stack = OutputStacks[ind];
    OutputStacks[ind] = new T_OutputStackElement_ptr[2 * OutputStackSize];
    for (ind2 = 0; ind2 < NoOfElementsOnStack[ind]; ind2++)
    {
      OutputStacks[ind][ind2] = temp_stack[ind2];
    }
    for (ind2 = NoOfElementsOnStack[ind]; ind2 < 2 * OutputStackSize; ind2++)
    {
      OutputStacks[ind][ind2] = NULL;
    }
    delete [] temp_stack;
  }
  OutputStackSize *= 2;
}

// Returns number of elements registered for given OutputIndex
unsigned int T_OutputStackElement::GetNoOfElements(unsigned int OutputIndex)
{
  return NoOfElementsOnStack[OutputIndex];
}

int T_OutputStackElement::AddOutput(T_OutputStackElement *Element, unsigned int OutputIndex)
{
  if (OutputIndex >= NoOfStacks)
  {
    DSP::log << DSP::e::LogMode::Error <<"T_OutputStackElement::AddOutput"<< DSP::e::LogMode::second <<"OutputIndex >= NoOfStacks"<<endl;
    return -1;
  }
  if (NoOfElementsOnStack[OutputIndex] >= OutputStackSize)
    IncreaseStackSize();

  if (OutputStacks[OutputIndex][NoOfElementsOnStack[OutputIndex]] != NULL)
  {
    DSP::log << DSP::e::LogMode::Error <<"T_OutputStackElement::AddOutput"<< DSP::e::LogMode::second << "Output already connected - try other OutputIndex"<<endl;
    return -1;
  }
  OutputStacks[OutputIndex][NoOfElementsOnStack[OutputIndex]] = Element;
  NoOfElementsOnStack[OutputIndex]++;

  ReinitializeElement();

  return NoOfElementsOnStack[OutputIndex]-1;
}

int T_OutputStackElement::RemoveOutput(T_OutputStackElement *Element, unsigned int OutputIndex)
{
  unsigned int element_index, ind2;

  if (OutputIndex >= NoOfStacks)
  {
    DSP::log << DSP::e::LogMode::Error <<"T_OutputStackElement::RemoveOutput"<< DSP::e::LogMode::second<< "OutputIndex >= NoOfStacks"<<endl;
    return -1;
  }

  // find element's index
  element_index = UINT_MAX;
  for (ind2 = 0; ind2 < NoOfElementsOnStack[OutputIndex]; ind2++)
    if (OutputStacks[OutputIndex][ind2] == Element)
    {
      element_index = ind2;
      break;
    }
  if (element_index == UINT_MAX)
  {
    DSP::log << DSP::e::LogMode::Error <<"T_OutputStackElement::RemoveOutput"<< DSP::e::LogMode::second<< "Element not on the stack"<<endl;
    return -1;
  }

  // remove element
  for (ind2 = element_index+1; ind2 < NoOfElementsOnStack[OutputIndex]; ind2++)
    OutputStacks[OutputIndex][ind2-1] = OutputStacks[OutputIndex][ind2];
  OutputStacks[OutputIndex][NoOfElementsOnStack[OutputIndex]-1] = NULL;
  NoOfElementsOnStack[OutputIndex]--;

  ReinitializeElement();

  return element_index;
}

void *T_OutputStackElement::GetInputData_forward
                          (unsigned int &NoOfInputSamples,
                           unsigned int current_segment,
                           unsigned int OutputIndex, unsigned int ElementIndex)
{
  // Calculate NoOfSamples and InputData
  // Additionally previously unprocessed input samples are also send as new input data
  // NoOfSamples must not include overlap segment, though,
  // the InputData will contain overlap segment.
  void *InputData = NULL;

  NoOfInputSamples = output_segment_sizes[current_segment+1]
                   + NoOfUnprocessedInputSamples[OutputIndex][ElementIndex];
  switch (OutputBufferType)
  {
    case E_OBT_float:
      InputData = (DSP::Float *)(output_segment_starts[current_segment+1])
                 - NoOfUnprocessedInputSamples[OutputIndex][ElementIndex]
      //// overlap segment before the actual current segment data
                 - OutputStacks[OutputIndex][ElementIndex]->ElementOverlapSegmentSize();
      break;
    case E_OBT_complex:
      InputData = (DSP::Complex *)(output_segment_starts[current_segment+1])
                 - NoOfUnprocessedInputSamples[OutputIndex][ElementIndex]
      //// overlap segment before the actual current segment data
                 - OutputStacks[OutputIndex][ElementIndex]->ElementOverlapSegmentSize();
      break;
    default:
      DSP::log << DSP::e::LogMode::Error <<"T_OutputStackElement::GetInputData_forward"<< DSP::e::LogMode::second<<"Unsupported output type"<<endl;
      break;
  }
  return InputData;
}

void *T_OutputStackElement::GetInputData_backward
                          (unsigned int &NoOfInputSamples,
                           unsigned int current_segment,
                           unsigned int OutputIndex, unsigned int ElementIndex)
{
  // Calculate NoOfSamples and InputData
  // Additionally previously unprocessed input samples are also send as new input data
  // NoOfSamples must not include overlap segment, though,
  // the InputData will contain overlap segment.
  void *InputData = NULL;

  NoOfInputSamples = output_segment_sizes[current_segment+1]
                   + NoOfUnprocessedInputSamples[OutputIndex][ElementIndex];
  switch (OutputBufferType)
  {
    case E_OBT_float:
      InputData = (DSP::Float *)(output_segment_starts[current_segment+1])
                 - NoOfUnprocessedInputSamples[OutputIndex][ElementIndex];
      //// overlap segment after the actual current segment data
      break;
    case E_OBT_complex:
      InputData = (DSP::Complex *)(output_segment_starts[current_segment+1])
                 - NoOfUnprocessedInputSamples[OutputIndex][ElementIndex];
      //// overlap segment after the actual current segment data
      break;
    default:
      DSP::log << DSP::e::LogMode::Error <<"T_OutputStackElement::GetInputData_backward"<< DSP::e::LogMode::second << "Unsupported output type"<<endl;
      break;
  }
  return InputData;
}

// Calls Process function for output elements registered to output OutputIndex
template<class T>
bool T_OutputStackElement::ProcessOutputElements(
    E_processing_DIR processing_DIR,
    unsigned int current_segment,
    unsigned int OutputIndex)
{
  unsigned int ind;
  unsigned int NoOfSamples;
  T *InputData;
  //! true if something has been processed
  bool result;

  result = false;
  if (OutputIndex >= NoOfStacks)
  {
    DSP::log << DSP::e::LogMode::Error <<"T_OutputStackElement::ProcessOutputElements"<< DSP::e::LogMode::second << "OutputIndex >= NoOfStacks"<<endl;
    return result;
  }

  for (ind = 0; ind < NoOfElementsOnStack[OutputIndex]; ind++)
  {
    switch (processing_DIR)
    {
      case E_PD_backward:
        InputData = (T *)GetInputData_backward(NoOfSamples,
            current_segment, OutputIndex, ind);
        break;

      case E_PD_forward:
      default:
        InputData = (T *)GetInputData_forward(NoOfSamples,
              current_segment, OutputIndex, ind);
        break;
    }
    //InputData += (current_segment * output_segment_size);

    if (NoOfSamples > 0)
    {
      result = true;

      //! \todo_later Use typeid instead of sizeof
      switch (OutputBufferType)
      {
        case E_OBT_complex:
          if (sizeof(DSP::Complex) != sizeof(*InputData))
            DSP::log << DSP::e::LogMode::Error <<"T_OutputStackElement::ProcessOutputElements"<< DSP::e::LogMode::second<< "DSP::Complex expected but not encountered"<<endl;
          else
            NoOfUnprocessedInputSamples[OutputIndex][ind] =
                OutputStacks[OutputIndex][ind]->Process_cplx(processing_DIR, NoOfSamples, (DSP::Complex_ptr)InputData);
          break;
        case E_OBT_float:
          if (sizeof(DSP::Float) != sizeof(*InputData))
            DSP::log << DSP::e::LogMode::Error <<"T_OutputStackElement::ProcessOutputElements"<< DSP::e::LogMode::second<< "DSP::Float expected but not encountered"<<endl;
          else
            NoOfUnprocessedInputSamples[OutputIndex][ind] =
              OutputStacks[OutputIndex][ind]->Process_real(processing_DIR, NoOfSamples, (DSP::Float_ptr)InputData);
          break;
        default:
          DSP::log << DSP::e::LogMode::Error <<"T_OutputStackElement::ProcessOutputElements"<< DSP::e::LogMode::second<< "Unrecognized output buffer type"<<endl;
          break;
      }
    }
    else
    {
      #ifdef __DEBUG__
        DSP::log << "T_OutputStackElement::ProcessOutputElements"<< DSP::e::LogMode::second << "Segment processing has ended"<<endl;
      #endif
      NoOfUnprocessedInputSamples[OutputIndex][ind] = 0;
      result = false;
    }
  }

  return result;
}

T_InputElement::T_InputElement(E_OutputBuffeType OutputBufferType_in,
                               unsigned int NoOfOutputs, unsigned int InitialStackSize,
                               unsigned int no_of_output_segments_in)
  : T_OutputStackElement(OutputBufferType_in, NoOfOutputs, InitialStackSize)
{
  unsigned int ind;

  IsComplex = true;
  // Fs must be > 0
  Fs = 1.0; Fo = 0.0;
  input_info = new char[DSP_STR_MAX_LENGTH+1]; input_info[0] = 0x00;

  NoOfSubtables = NoOfOutputs;
  NoOfElementsInSubtables = new unsigned int[NoOfSubtables];

  ProcessingFinished = new bool *[NoOfSubtables];
  n_start = new long long *[NoOfSubtables];
  n_end   = new long long *[NoOfSubtables];
  default_n_start = -1;
  default_n_end = -1;

  for (ind = 0; ind < NoOfSubtables; ind++)
  {
    NoOfElementsInSubtables[ind] = 0;

    ProcessingFinished[ind] = NULL;
    n_start[ind] = NULL;
    n_end[ind] = NULL;
  }

  n_start_with_overlap = NULL;
  n_end_with_overlap = NULL;

  n_start_with_defaults = NULL;
  n_end_with_defaults = NULL;

  input_discrete_time = new long long[no_of_output_segments_in];
  for (ind = 0; ind < no_of_output_segments_in; ind++)
    input_discrete_time[ind] = 0;
}

T_InputElement::~T_InputElement(void)
{
  unsigned int ind;

  if (input_info != NULL)
  {
    delete [] input_info;
    input_info = NULL;
  }

  FreeTimeTables();

  if (input_discrete_time != NULL)
  {
    delete [] input_discrete_time;
    input_discrete_time = NULL;
  }

  if (ProcessingFinished != NULL)
  {
    for (ind = 0; ind < NoOfStacks; ind++)
    {
      if (ProcessingFinished[ind] != NULL)
      {
        delete [] ProcessingFinished[ind];
        ProcessingFinished[ind] = NULL;
      }
    }
    delete [] ProcessingFinished;
    ProcessingFinished = NULL;
  }
  if (n_start != NULL)
  {
    for (ind = 0; ind < NoOfStacks; ind++)
    {
      if (n_start[ind] != NULL)
      {
        delete [] n_start[ind];
        n_start[ind] = NULL;
      }
    }
    delete [] n_start;
    n_start = NULL;
  }
  if (n_end != NULL)
  {
    for (ind = 0; ind < NoOfStacks; ind++)
    {
      if (n_end[ind] != NULL)
      {
        delete [] n_end[ind];
        n_end[ind] = NULL;
      }
    }
    delete [] n_end;
    n_end = NULL;
  }

  if (NoOfElementsInSubtables != NULL)
  {
    delete [] NoOfElementsInSubtables;
    NoOfElementsInSubtables = NULL;
  }
}

const long double T_InputElement::GetCenterFrequency(void)
{
  return Fo;
}

const char *T_InputElement::GetInputInfo(void)
{
  return input_info;
}

// returns sampling rate
const long double T_InputElement::GetSamplingRate(void)
{
  return Fs;
}


/* returns number of samples to skip to the segment start
 * - zero if no skip is required
 * - -1 if processing has ended
 * .
 */
long long T_InputElement::GetNoOfSamplesToStart_forward(void)
{
  long long input_start_time;
  long long n_start_min, n_end_max;
  unsigned int ind, ind2;

  n_start_min = n_start_with_defaults[0][0];
  n_end_max   = n_end_with_defaults[0][0];
  for (ind = 0; ind <NoOfStacks; ind++)
    for (ind2 = 0; ind2 <NoOfElementsOnStack[ind]; ind2++)
    {
      if (n_start_min > n_start_with_defaults[ind][ind2])
        n_start_min = n_start_with_defaults[ind][ind2];
      if (n_end_max > n_end_with_defaults[ind][ind2])
        n_end_max = n_end_with_defaults[ind][ind2];
    }


  //! \bug check if this should be current_output_segment or current_output_segment+1
  input_start_time = input_discrete_time[current_output_segment]; // (no overlap segments counted)

  if (n_start_min > input_start_time)
  {
    return (n_start_min - input_start_time);
  }
  else
    if (n_end_max < input_start_time)
      return -1;
  return 0;
}

/*! \bug Implement support for processing time ranges
 *  - input_discrete_time <- segment
 *  - n_start_with_overlap <- element
 *  - n_end_with_overlap   <- element
 *  - n_* - correct support at beginning and the end of the file
 */
void *T_InputElement::GetInputData_forward
                          (unsigned int &NoOfInputSamples_in,
                           unsigned int current_segment,
                           unsigned int OutputIndex, unsigned int ElementIndex)
{
  // Calculate NoOfSamples and InputData
  // Additionally previously unprocessed input samples are also send as new input data
  // NoOfSamples must not include overlap segment, though,
  // the InputData will contain overlap segment.
  void *InputData;
  long long input_start_time, input_end_time;
  long long start_correction;

  InputData = NULL;
  // segment start discrete time:
  input_start_time = input_discrete_time[current_segment]; // (no overlap segments counted)
  // segment end discrete time: (first sample past end of the segment)
  input_end_time = input_start_time + output_segment_sizes[current_segment+1];

  // ??? maybe better to use here n_start and n_end
  // ??? n_start_with_overlap and n_end_with_overlap
  // ??? should be rather used in T_InputElement::Process()
  // element start time:
  //   n_start_with_overlap[OutputIndex][ElementIndex]
  // element end time:
  //   n_end_with_overlap[OutputIndex][ElementIndex]
  //! \todo check if <= and >= should be used
  if ((n_start_with_defaults[OutputIndex][ElementIndex] < input_end_time) &&
      (n_end_with_defaults[OutputIndex][ElementIndex] >= input_start_time))
  {
    NoOfInputSamples_in = output_segment_sizes[current_segment+1]
                     + NoOfUnprocessedInputSamples[OutputIndex][ElementIndex];

    // check if segment should be processed till the end
    if (n_end_with_defaults[OutputIndex][ElementIndex] < input_end_time)
    { // and correct no of samples
      // \note input_end_time is one sample past the ending sample and
      // n_end[OutputIndex][ElementIndex] is the index of the ending sample
      NoOfInputSamples_in -= (input_end_time -
          (n_end_with_defaults[OutputIndex][ElementIndex]+1));
    }
    // check if the beginning of the segment should be discarded
    if (n_start_with_defaults[OutputIndex][ElementIndex]  > input_start_time)
    { // and correct InputData and NoOfInputSamples
      start_correction = n_start_with_defaults[OutputIndex][ElementIndex] - input_start_time;
      NoOfInputSamples_in -= start_correction;
    }
    else
      start_correction = 0;

    switch (OutputBufferType)
    {
      case E_OBT_float:
        InputData = (DSP::Float *)(output_segment_starts[current_segment+1])
                   - NoOfUnprocessedInputSamples[OutputIndex][ElementIndex]
        //// overlap segment before the actual current segment data
                   - OutputStacks[OutputIndex][ElementIndex]->ElementOverlapSegmentSize()
                   + start_correction;
        break;
      case E_OBT_complex:
        InputData = (DSP::Complex *)(output_segment_starts[current_segment+1])
                   - NoOfUnprocessedInputSamples[OutputIndex][ElementIndex]
        //// overlap segment before the actual current segment data
                   - OutputStacks[OutputIndex][ElementIndex]->ElementOverlapSegmentSize()
                   + start_correction;
        break;
      default:
        DSP::log << DSP::e::LogMode::Error <<"T_InputElement::GetInputData_forward"<< DSP::e::LogMode::second <<"Unsupported output type"<<endl;
        NoOfInputSamples_in = 0;
        break;
    }
  }
  else
    NoOfInputSamples_in = 0;

  return InputData;
}

/*! \bug Implement support for processing time ranges
 */
void *T_InputElement::GetInputData_backward
                          (unsigned int &NoOfInputSamples_in,
                           unsigned int current_segment,
                           unsigned int OutputIndex, unsigned int ElementIndex)
{
  // Calculate NoOfSamples and InputData
  // Additionally previously unprocessed input samples are also send as new input data
  // NoOfSamples must not include overlap segment, though,
  // the InputData will contain overlap segment.
  void *InputData = NULL;

  NoOfInputSamples_in = output_segment_sizes[current_segment+1]
                   + NoOfUnprocessedInputSamples[OutputIndex][ElementIndex];
  switch (OutputBufferType)
  {
    case E_OBT_float:
      InputData = (DSP::Float *)(output_segment_starts[current_segment+1])
                 - NoOfUnprocessedInputSamples[OutputIndex][ElementIndex];
      //// overlap segment after the actual current segment data
      break;
    case E_OBT_complex:
      InputData = (DSP::Complex *)(output_segment_starts[current_segment+1])
                 - NoOfUnprocessedInputSamples[OutputIndex][ElementIndex];
      //// overlap segment after the actual current segment data
      break;
    default:
      DSP::log << DSP::e::LogMode::Error <<"T_InputElement::GetInputData_backward"<< DSP::e::LogMode::second << "Unsupported output type"<<endl;
      break;
  }
  return InputData;
}

void T_InputElement::FreeTimeTables(void)
{
  unsigned int ind;

  if (n_start_with_overlap != NULL)
  {
    for (ind = 0; ind < NoOfStacks; ind++)
    {
      if (n_start_with_overlap[ind] != NULL)
      {
        delete [] n_start_with_overlap[ind];
        n_start_with_overlap[ind] = NULL;
      }
    }
    delete [] n_start_with_overlap;
    n_start_with_overlap = NULL;
  }
  if (n_end_with_overlap != NULL)
  {
    for (ind = 0; ind < NoOfStacks; ind++)
    {
      if (n_end_with_overlap[ind] != NULL)
      {
        delete [] n_end_with_overlap[ind];
        n_end_with_overlap[ind] = NULL;
      }
    }
    delete [] n_end_with_overlap;
    n_end_with_overlap = NULL;
  }

  if (n_start_with_defaults != NULL)
  {
    for (ind = 0; ind < NoOfStacks; ind++)
    {
      if (n_start_with_defaults[ind] != NULL)
      {
        delete [] n_start_with_defaults[ind];
        n_start_with_defaults[ind] = NULL;
      }
    }
    delete [] n_start_with_defaults;
    n_start_with_defaults = NULL;
  }
  if (n_end_with_defaults != NULL)
  {
    for (ind = 0; ind < NoOfStacks; ind++)
    {
      if (n_end_with_defaults[ind] != NULL)
      {
        delete [] n_end_with_defaults[ind];
        n_end_with_defaults[ind] = NULL;
      }
    }
    delete [] n_end_with_defaults;
    n_end_with_defaults = NULL;
  }
}

int T_InputElement::AddOutput(T_OutputStackElement *Element, unsigned int OutputIndex)
{
  unsigned int ElementIndex;

  ElementIndex = T_OutputStackElement::AddOutput(Element, OutputIndex);

  UpdateTablesSizes(true, ElementIndex, OutputIndex);

  return ElementIndex;
}
int T_InputElement::RemoveOutput(T_OutputStackElement *Element, unsigned int OutputIndex)
{
  unsigned int ElementIndex;

  ElementIndex = T_OutputStackElement::RemoveOutput(Element, OutputIndex);

  UpdateTablesSizes(false, ElementIndex, OutputIndex);

  return ElementIndex;
}

void T_InputElement::UpdateTablesSizes(bool element_added,
                                       int ElementIndex, unsigned int OutputIndex)
{
  bool size_has_changed;
  int ind, ind_2;
  int counter_t, counter_e;
  char text[1024];

  sprintf(text, "  ElementIndex = %i, OutputIndex = %i", ElementIndex, OutputIndex);
  DSP::log << "T_InputElement::UpdateTablesSizes"<< DSP::e::LogMode::second << text<<endl;

  /* 1. check if OutputStack size has changed properly
   *   if size change value is strange then set ElementIndex = -1;
   */
  // NoOfSubtables, NoOfElementsInSubtables
  // NoOfStacks, NoOfElementsOnStack
  size_has_changed = false;
  //! count number of entries in time table
  counter_t = 0;
  for (ind = 0; ind < (int)NoOfSubtables; ind++)
  {
    counter_t += NoOfElementsInSubtables[ind];

    sprintf(text, "  NoOfElementsInSubtables[%i] = %i", ind, NoOfElementsInSubtables[ind]);
    DSP::log << "T_InputElement::UpdateTablesSizes"<< DSP::e::LogMode::second << text<<endl;
  }
  //! count number of entries in elements stacks
  counter_e = 0;
  for (ind = 0; ind < (int)NoOfStacks; ind++)
  {
    counter_e += NoOfElementsOnStack[ind];

    sprintf(text, "  NoOfElementsOnStack[%i] = %i", ind, NoOfElementsOnStack[ind]);
    DSP::log << "T_InputElement::UpdateTablesSizes"<< DSP::e::LogMode::second << text<<endl;
  }

  sprintf(text, "tables sizes %i and %i", counter_e, counter_t);
  DSP::log << "T_InputElement::UpdateTablesSizes"<< DSP::e::LogMode::second << text<<endl;

  if (counter_e != counter_t)
  {
    size_has_changed = true;

    if (abs(counter_e - counter_t) != 1)
    {
      sprintf(text, "incorrect tables size change detected %i != %i", counter_e, counter_t);
      DSP::log << DSP::e::LogMode::Error <<"T_InputElement::UpdateTablesSizes"<< DSP::e::LogMode::second << text<<endl;
      ElementIndex = -1;
    }
    else
    {
      if (element_added == true)
      {
        if (counter_e - counter_t != 1)
        {
          DSP::log << DSP::e::LogMode::Error <<"T_InputElement::UpdateTablesSizes"<< DSP::e::LogMode::second <<"tables size should have increased by one element"<<endl;
          ElementIndex = -1;
        }
      }
      else
      {
        if (counter_e - counter_t != -1)
        {
          DSP::log << DSP::e::LogMode::Error <<"T_InputElement::UpdateTablesSizes"<< DSP::e::LogMode::second <<"tables size should have decreased by one element"<<endl;
          ElementIndex = -1;
        }
      }

      if (OutputIndex <= NoOfStacks)
      {
        // check if element added to or removed from the OutputIndex subtable
        if (element_added == true)
        {
          if (NoOfElementsInSubtables[OutputIndex] == NoOfElementsOnStack[ind]+1)
          {
            DSP::log << DSP::e::LogMode::Error <<"T_InputElement::UpdateTablesSizes"<< DSP::e::LogMode::second << "incorrect tables size: size change for wrong output"<<endl;
            ElementIndex = -1;
          }
        }
        else
        {
          if (NoOfElementsInSubtables[OutputIndex] == NoOfElementsOnStack[ind]-1)
          {
            DSP::log << DSP::e::LogMode::Error <<"T_InputElement::UpdateTablesSizes"<< DSP::e::LogMode::second << "incorrect tables size: size change for wrong output"<<endl;
            ElementIndex = -1;
          }
        }
      }
      else
      {
        DSP::log << "T_InputElement::UpdateTablesSizes"<< DSP::e::LogMode::second << "NoOfOutputStack has changed (this is rather unexpected)"<<endl;
        // though it might be correct ???
      }
    }
  }
  else
  {
    DSP::log << DSP::e::LogMode::Error <<"T_InputElement::UpdateTablesSizes"<< DSP::e::LogMode::second << "tables size did not change though it was expected to"<<endl;
    return;
  }

  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  //  update tables size
  if (ElementIndex == -1)
  {
    // if stack size change has been detected modify time tables too
    // crude size change and reset times to defaults
    if (NoOfSubtables != NoOfStacks)
    {
      // +++++++++++++++++++++++++++++++++++ //
      // free old tables
      if (ProcessingFinished != NULL)
      {
        for (ind = 0; ind < (int)NoOfSubtables; ind++)
        {
          if (ProcessingFinished[ind] != NULL)
            delete [] ProcessingFinished[ind];
        }
        delete [] ProcessingFinished;
      }

      if (n_start != NULL)
      {
        for (ind = 0; ind < (int)NoOfSubtables; ind++)
        {
          if (n_start[ind] != NULL)
            delete [] n_start[ind];
        }
        delete [] n_start;
      }

      if (n_end != NULL)
      {
        for (ind = 0; ind < (int)NoOfSubtables; ind++)
        {
          if (n_end[ind] != NULL)
            delete [] n_end[ind];
        }
        delete [] n_end;
      }

      // +++++++++++++++++++++++++++++++++++ //
      //  allocate new tables
      NoOfSubtables = NoOfStacks;

      if (NoOfElementsInSubtables != NULL)
        delete [] NoOfElementsInSubtables;
      NoOfElementsInSubtables = new unsigned int[NoOfSubtables];

      ProcessingFinished = new bool *[NoOfSubtables];
      n_start = new long long *[NoOfSubtables];
      n_end   = new long long *[NoOfSubtables];


      // +++++++++++++++++++++++++++++++++++ //
      //  fill then with defaults
      for (ind = 0; ind < (int)NoOfSubtables; ind++)
      {
        NoOfElementsInSubtables[ind] = NoOfElementsOnStack[ind];

        if (NoOfElementsInSubtables[ind] == 0)
        {
          ProcessingFinished[ind] = NULL;
          n_start[ind] = NULL;
          n_end[ind]   = NULL;
        }
        else
        {
          ProcessingFinished[ind] = new bool[NoOfElementsInSubtables[ind]];
          n_start[ind] = new long long[NoOfElementsInSubtables[ind]];
          n_end[ind]   = new long long[NoOfElementsInSubtables[ind]];
          for (ind_2 = 0; ind_2 < (int)(NoOfElementsInSubtables[ind]); ind_2++)
          {
            ProcessingFinished[ind][ind_2] = false;
            n_start[ind][ind_2] = -1;
            n_end[ind][ind_2]   = -1;
          }
        }
      }
    }
    return;
  }
  else
  {
    // update time tables size and move values accordingly
    if (element_added == true)
    { // one new element
      if (NoOfSubtables != NoOfStacks)
      {
        unsigned int temp_NoOfSubtables;
        unsigned int *temp_NoOfElementsInSubtables;
        bool **temp_ProcessingFinished;
        long long **temp_n_start;
        long long **temp_n_end;

        // 1a. new output added
        // \warning New outputs expected at the end of tables
        temp_NoOfSubtables = NoOfStacks;

        temp_NoOfElementsInSubtables = new unsigned int[temp_NoOfSubtables];
        temp_ProcessingFinished = new bool *[temp_NoOfSubtables];
        temp_n_start = new long long *[temp_NoOfSubtables];
        temp_n_end   = new long long *[temp_NoOfSubtables];

        // copy existing data
        for (ind = 0; ind < (int)NoOfSubtables; ind++)
        {
          temp_NoOfElementsInSubtables[ind] = NoOfElementsInSubtables[ind];

          // copy tables without reallocation
          temp_ProcessingFinished[ind] = ProcessingFinished[ind];
          temp_n_start[ind] = n_start[ind];
          temp_n_end[ind]   = n_end[ind];
        }

        // fill new slot with defaults
        for (ind = NoOfSubtables; ind < (int)temp_NoOfSubtables; ind++)
        {
          temp_NoOfElementsInSubtables[ind] = 0;

          temp_ProcessingFinished[ind] = NULL;
          temp_n_start[ind] = NULL;
          temp_n_end[ind]   = NULL;
        }

        if (ElementIndex != 0)
          DSP::log << DSP::e::LogMode::Error <<"T_InputElement::UpdateTablesSizes" << DSP::e::LogMode::second << "ElementIndex was expected to be 0"<<endl;

        temp_NoOfElementsInSubtables[OutputIndex] = 1;
        temp_ProcessingFinished[OutputIndex] = new bool[1];
        temp_ProcessingFinished[OutputIndex][0] = false;
        temp_n_start[OutputIndex] = new long long[1];
        temp_n_start[OutputIndex][0] = -1;
        temp_n_end[OutputIndex]   = new long long[1];
        temp_n_end[OutputIndex][0]   = -1;

        // replace tables with temporary ones and delete temporary tables
        delete [] NoOfElementsInSubtables;
        NoOfElementsInSubtables = temp_NoOfElementsInSubtables;
        delete [] ProcessingFinished;
        ProcessingFinished = temp_ProcessingFinished;
        delete [] n_start;
        n_start = temp_n_start;
        delete [] n_end;
        n_end = temp_n_end;

        NoOfStacks = temp_NoOfSubtables;
      }
      else
      {
        // 1b. no of outputs stays the same
        bool *temp_bool;
        long long *temp_long_long;

        NoOfElementsInSubtables[OutputIndex]++;

        if (ElementIndex != (int)(NoOfElementsInSubtables[OutputIndex])-1)
          DSP::log << DSP::e::LogMode::Error <<"T_InputElement::UpdateTablesSizes"<< DSP::e::LogMode::second << "ElementIndex was expected to be last in table"<<endl;

        temp_bool = ProcessingFinished[OutputIndex];
        ProcessingFinished[OutputIndex] = new bool[NoOfElementsInSubtables[OutputIndex]];
        if (temp_bool != NULL)
        {
          memcpy(ProcessingFinished[OutputIndex], temp_bool,
              (NoOfElementsInSubtables[OutputIndex]-1) * sizeof(bool));
          delete [] temp_bool;
        }
        ProcessingFinished[OutputIndex][NoOfElementsInSubtables[OutputIndex]-1] = false;

        temp_long_long = n_start[OutputIndex];
        n_start[OutputIndex] = new long long[NoOfElementsInSubtables[OutputIndex]];
        if (temp_long_long != NULL)
        {
          memcpy(n_start[OutputIndex], temp_long_long,
              (NoOfElementsInSubtables[OutputIndex]-1) * sizeof(long long));
          delete [] temp_long_long;
        }
        n_start[OutputIndex][NoOfElementsInSubtables[OutputIndex]-1] = -1;

        temp_long_long = n_end[OutputIndex];
        n_end[OutputIndex] = new long long[NoOfElementsInSubtables[OutputIndex]];
        if (temp_long_long != NULL)
        {
          memcpy(n_end[OutputIndex], temp_long_long,
              (NoOfElementsInSubtables[OutputIndex]-1) * sizeof(long long));
          delete [] temp_long_long;
        }
        n_end[OutputIndex][NoOfElementsInSubtables[OutputIndex]-1] = -1;
      }
    }
    else
    { // one element deleted
      if (NoOfSubtables != NoOfStacks)
      {
        unsigned int temp_NoOfSubtables;
        unsigned int *temp_NoOfElementsInSubtables;
        bool **temp_ProcessingFinished;
        long long **temp_n_start;
        long long **temp_n_end;

        // 2a. output removed
        temp_NoOfSubtables = NoOfStacks;

        temp_NoOfElementsInSubtables = new unsigned int[temp_NoOfSubtables];
        temp_ProcessingFinished = new bool *[temp_NoOfSubtables];
        temp_n_start = new long long *[temp_NoOfSubtables];
        temp_n_end   = new long long *[temp_NoOfSubtables];

        // copy existing data
        // A. below OutputIndex
        for (ind = 0; ind < (int)OutputIndex-1; ind++)
        {
          temp_NoOfElementsInSubtables[ind] = NoOfElementsInSubtables[ind];

          // copy tables without reallocation
          temp_ProcessingFinished[ind] = ProcessingFinished[ind];
          temp_n_start[ind] = n_start[ind];
          temp_n_end[ind]   = n_end[ind];
        }
        // B. after OutputIndex
        for (ind = OutputIndex; ind < (int)NoOfSubtables; ind++)
        {
          temp_NoOfElementsInSubtables[ind-1] = NoOfElementsInSubtables[ind];

          // copy tables without reallocation
          temp_ProcessingFinished[ind-1] = ProcessingFinished[ind];
          temp_n_start[ind-1] = n_start[ind];
          temp_n_end[ind-1]   = n_end[ind];
        }

        if (ElementIndex != 0)
          DSP::log << DSP::e::LogMode::Error <<"T_InputElement::UpdateTablesSizes"<< DSP::e::LogMode::second <<"ElementIndex was expected to be 0"<<endl;

        // replace tables with temporary ones and delete temporary tables
        delete [] NoOfElementsInSubtables;
        NoOfElementsInSubtables = temp_NoOfElementsInSubtables;
        delete [] ProcessingFinished;
        ProcessingFinished = temp_ProcessingFinished;
        delete [] n_start;
        n_start = temp_n_start;
        delete [] n_end;
        n_end = temp_n_end;

        NoOfStacks = temp_NoOfSubtables;
      }
      else
      {
        // 2b. no of outputs stays the same
        bool *temp_bool;
        long long *temp_long_long_start;
        long long *temp_long_long_end;

        NoOfElementsInSubtables[OutputIndex]--;

        temp_bool = ProcessingFinished[OutputIndex];
        temp_long_long_start = n_start[OutputIndex];
        temp_long_long_end   = n_end[OutputIndex];
        if (NoOfElementsInSubtables[OutputIndex] > 0)
        {
          ProcessingFinished[OutputIndex] = new bool[NoOfElementsInSubtables[OutputIndex]];
          n_start[OutputIndex] = new long long[NoOfElementsInSubtables[OutputIndex]];
          n_end[OutputIndex] = new long long[NoOfElementsInSubtables[OutputIndex]];

          // copy data before ElementIndex
          for (ind = 0; ind < ElementIndex-1; ind ++)
          {
            ProcessingFinished[OutputIndex][ind] = temp_bool[ind];
            n_start[OutputIndex][ind] = temp_long_long_start[ind];
            n_end[OutputIndex][ind]   = temp_long_long_end[ind];
          }

          // copy data after ElementIndex
          for (ind = ElementIndex; ind < (int)(NoOfElementsInSubtables[OutputIndex])+1; ind ++)
          {
            ProcessingFinished[OutputIndex][ind-1] = temp_bool[ind];
            n_start[OutputIndex][ind-1] = temp_long_long_start[ind];
            n_end[OutputIndex][ind-1]   = temp_long_long_end[ind];
          }
        }
        else
        {
          ProcessingFinished[OutputIndex] = NULL;
          n_start[OutputIndex] = NULL;
          n_end[OutputIndex]   = NULL;
        }

        if (temp_bool != NULL)
          delete [] temp_bool;
        if (temp_long_long_start != NULL)
          delete [] temp_long_long_start;
        if (temp_long_long_end != NULL)
          delete [] temp_long_long_end;
      }
    }
  }
}

bool T_InputElement::SetInputTimeRange(long long n_1, long long n_2,
                                       T_OutputStackElement *output_block)
{
  bool result;

  if (n_1 < 0)
    n_1 = 0;
  if (n_2 < 0)
    n_2 = NoOfInputSamples - 1;

  result = false;
  if (output_block == NULL)
  {
    // If output_block == NULL then

    // 1. update default variables
    default_n_start = n_1;
    default_n_end = n_2;
  }
  else
  {
    unsigned int ind_1, ind_2;

    ind_2 = 0;
    // If output_block != NULL
    // 1. find output_block indexes
    result = false;
    for (ind_1 = 0; ind_1 < NoOfStacks; ind_1++)
    {
      for (ind_2 = 0; ind_2 < NoOfElementsOnStack[ind_1]; ind_2++)
      {
        if (OutputStacks[ind_1][ind_2] == output_block)
        {
          result = true;
          break;
        }
      }
      if (result == true)
        break;
    }

    if (result == true)
    {
      // 2. update time values without overlapping segments taken into account
      n_start[ind_1][ind_2] = n_1;
      n_end[ind_1][ind_2] = n_2;
    }
  }

  // return false if something was wrong
  return result;
}

bool T_InputElement::UpdateTimeTables(E_processing_DIR process_dir, long long current_sample_to_read)
{
  bool result;

  // 2. and tables with times including overlapping segments
  result = FillTimeTables(process_dir);

  // 4. Calculate index of the first sample to read from input
  result &= FindNextSampleToRead(current_sample_to_read);

  return result;
}

// calculates actual start and stop times by taking into account the overlap segment sizes
bool T_InputElement::FillTimeTables(E_processing_DIR process_dir)
{
  int ind, ind2;

  FreeTimeTables();

  if ((default_n_start < 0) || (default_n_end < 0))
  {
    DSP::log << DSP::e::LogMode::Error <<"T_InputElement::UpdateTimeTablesWithOverlap"<< DSP::e::LogMode::second <<"Default start and end times must be with SetInputTimeRange"<<endl;
  }

  // allocate new overlap time tables
  n_start_with_overlap = new long long *[NoOfSubtables];
  n_end_with_overlap   = new long long *[NoOfSubtables];
  n_start_with_defaults = new long long *[NoOfSubtables];
  n_end_with_defaults   = new long long *[NoOfSubtables];
  for (ind = 0; ind < (int)NoOfSubtables; ind++)
  {
    if (NoOfElementsInSubtables[ind] == 0)
    {
      n_start_with_overlap[ind] = NULL;
      n_end_with_overlap[ind] = NULL;
      n_start_with_defaults[ind] = NULL;
      n_end_with_defaults[ind] = NULL;
    }
    else
    {
      n_start_with_overlap[ind] = new long long[NoOfElementsInSubtables[ind]];
      n_end_with_overlap[ind]   = new long long[NoOfElementsInSubtables[ind]];
      n_start_with_defaults[ind] = new long long[NoOfElementsInSubtables[ind]];
      n_end_with_defaults[ind]   = new long long[NoOfElementsInSubtables[ind]];
    }
  }

  // in this tables set the default values to simplify further processing
  switch (process_dir)
  {
    case E_PD_backward:
      for (ind = 0; ind < (int)NoOfSubtables; ind++)
      {
        for (ind2 = 0; ind2 < (int)(NoOfElementsInSubtables[ind]); ind2++)
        {
          if (n_start[ind][ind2] >= 0)
          {
            n_start_with_overlap[ind][ind2] = n_start[ind][ind2];
            n_start_with_defaults[ind][ind2] = n_start[ind][ind2];
          }
          else
          {
            n_start_with_overlap[ind][ind2] = default_n_start;
            n_start_with_defaults[ind][ind2] = default_n_start;
          }
          //! \note n_start_with_overlap[ind][ind2] < 0 means that overlap segment must be cleared
          if (n_start_with_defaults[ind][ind2] < 0)
            n_start_with_defaults[ind][ind2] = 0;

          if (n_end[ind][ind2] >= 0)
          {
            n_end_with_overlap[ind][ind2] = n_end[ind][ind2] +  output_segment_overlap;
            n_end_with_defaults[ind][ind2] = n_end[ind][ind2];
          }
          else
          {
            if (default_n_end < 0)
            {
              n_end_with_overlap[ind][ind2] = NoOfInputSamples-1;
              n_end_with_defaults[ind][ind2] = NoOfInputSamples-1;
            }
            else
            {
              n_end_with_overlap[ind][ind2] = default_n_end +  output_segment_overlap;
              n_end_with_defaults[ind][ind2] = default_n_end;
            }
          }
          if (n_end_with_overlap[ind][ind2] < 0)
            n_end_with_overlap[ind][ind2] = 0;
          if (n_end_with_defaults[ind][ind2] < 0)
            n_end_with_defaults[ind][ind2] = 0;
          if (n_end_with_overlap[ind][ind2] >= NoOfInputSamples)
            n_end_with_overlap[ind][ind2] = NoOfInputSamples-1;
          if (n_end_with_defaults[ind][ind2] >= NoOfInputSamples)
            n_end_with_defaults[ind][ind2] = NoOfInputSamples-1;
        }
      }
      break;

    case E_PD_forward:
    default:
      for (ind = 0; ind < (int)NoOfSubtables; ind++)
      {
        for (ind2 = 0; ind2 < (int)(NoOfElementsInSubtables[ind]); ind2++)
        {
          if (n_start[ind][ind2] >= 0)
          {
            n_start_with_overlap[ind][ind2] = n_start[ind][ind2] - output_segment_overlap;
            n_start_with_defaults[ind][ind2] = n_start[ind][ind2];
          }
          else
          {
            n_start_with_overlap[ind][ind2] = default_n_start - output_segment_overlap;
            n_start_with_defaults[ind][ind2] = default_n_start;
          }
          if (n_start_with_defaults[ind][ind2] < 0)
            n_start_with_defaults[ind][ind2] = 0;

          if (n_end[ind][ind2] >= 0)
          {
            n_end_with_overlap[ind][ind2] = n_end[ind][ind2];
            n_end_with_defaults[ind][ind2] = n_end[ind][ind2];
          }
          else
          {
            if (default_n_end < 0)
            {
              n_end_with_overlap[ind][ind2] = NoOfInputSamples-1;
              n_end_with_defaults[ind][ind2] = NoOfInputSamples-1;
            }
            else
            {
              n_end_with_overlap[ind][ind2] = default_n_end;
              n_end_with_defaults[ind][ind2] = default_n_end;
            }
          }
          if (n_end_with_overlap[ind][ind2] < 0)
            n_end_with_overlap[ind][ind2] = 0;
          if (n_end_with_defaults[ind][ind2] < 0)
            n_end_with_defaults[ind][ind2] = 0;
          if (n_end_with_overlap[ind][ind2] >= NoOfInputSamples)
            n_end_with_overlap[ind][ind2] = NoOfInputSamples-1;
          if (n_end_with_defaults[ind][ind2] >= NoOfInputSamples)
            n_end_with_defaults[ind][ind2] = NoOfInputSamples-1;
        }
      }
      break;
  }

  return false;
}

// Looks into time tables and searches for soonest input sample to read
bool T_InputElement::FindNextSampleToRead(long long current_sample_to_read)
{
  /*
   * @param current_sample_to_read - look for soonest start time no smaller than this
   * @return
   *
   *  current_sample_to_read <= 0 means look for any soonest time.

   *  !!! d. update ProcessingFinished table
   */
  return false;
}


// +++++++++++++++++++++++++++++++++++++ //
// +++++++++++++++++++++++++++++++++++++ //
/*! \todo implement mono/complex output selection
 *    for both mone and complex file inputs
 */
T_FILEinput::T_FILEinput(char *filename, char *filedir,
    unsigned int buffer_size_in,
    unsigned int no_of_output_segments_in)
  : T_InputElement(E_OBT_complex, 1, 2, no_of_output_segments_in) // QDD and PSD
{
  SetName("T_FILEinput", false);

  // determine file type on the basis of filename extension
  FileType = DSP::f::FileExtToFileType(filename);

  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  if (filedir == NULL)
  {
    input_file = new DSP::u::FileInput(NULL,
         filename,
         0, //autodetect
         DSP::e::SampleType::ST_short, FileType);
  }
  else
  {
    char *temp_filename;
    int temp_len;

    temp_filename = new char[strlen(filename) + strlen(filedir) + 2];
    strcpy(temp_filename, filedir);
    temp_len = strlen(temp_filename);
    temp_filename[temp_len] = '\\';
    strcpy(temp_filename+temp_len+1, filename);

    input_file = new DSP::u::FileInput(NULL,
         temp_filename,
         0, // autodetect
         DSP::e::SampleType::ST_short, FileType);

    delete [] temp_filename;
  }
  if (input_file->GetNoOfOutputs() == 1)
    input_file_is_mono = true;
  else
    input_file_is_mono = false;

  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  /* Fill the file_info
   *
   * - file type
   * - no of channels
   * - sampling rate
   * - sample type/size
   * - file size in samples and seconds
   * - center frequency
   * .
   */
  IsComplex = false;
  switch (FileType)
  {
    case DSP::e::FileType::FT_tape:
      {
        DSP::T_TAPE_header *tape_header;

        tape_header = input_file->GetHeader<DSP::T_TAPE_header>();
        if (tape_header->no_of_channels() == 2)
          IsComplex = true;
        Fo = tape_header->central_frequency * 1000000;
        // calculate Fs on the basis of CFG_filename
        Fs = 1; //
        if (strcmp(tape_header->CFG_filename, "REC_PLAY_1MHz.txt") == 0)
          Fs = 1000000;
        if (strcmp(tape_header->CFG_filename, "REC_PLAY_2MHz.txt") == 0)
          Fs = 2000000;
        if (strcmp(tape_header->CFG_filename, "REC_PLAY_4MHz.txt") == 0)
          Fs = 4000000;
        if (strcmp(tape_header->CFG_filename, "REC_PLAY_8MHz.txt") == 0)
          Fs = 8000000;
        //TotalSamples: 85196800
        NoOfInputSamples = tape_header->TotalSamples;

        //! \note MinGW printf has problems with long double
        sprintf(input_info, "TAPE file with %s signal\n"
              "Fo = %.3f MHz (%s)\n"
              "Fs = %.3f MSa/s\n"
              "No of samples = %ld"
              , (IsComplex == true) ? "complex" : "real"
              , double(Fo / 1000000), tape_header->CFG_filename,
              double(Fs / 1000000), (long int)NoOfInputSamples );
      }
      break;
    case DSP::e::FileType::FT_wav:
      {
        DSP::T_WAVEchunk   *wav_header;

        wav_header = input_file->GetHeader<DSP::T_WAVEchunk>();
        if (wav_header->nChannels == 2)
          IsComplex = true;
        Fs = wav_header->nSamplesPerSec;
        Fo = 0.0;
        NoOfInputSamples = wav_header->DataSize / (wav_header->wBitsPerSample / 8);

        //! \note MinGW printf has problems with long double
        sprintf(input_info, "WAV file with %s signal\n"
              "Fo = %.3f kHz\n"
              "Fs = %.3f kSa/s\n"
              "No of samples = %ld"
              , (IsComplex == true) ? "complex" : "real"
              , double(Fo / 1000), double(Fs / 1000), (long int)NoOfInputSamples );
      }
      break;
    case DSP::e::FileType::FT_flt:
      {
        DSP::T_FLT_header  *flt_header;

        flt_header = input_file->GetHeader<DSP::T_FLT_header>();
        if (flt_header->no_of_channels() == 2)
          IsComplex = true;
        Fs = flt_header->sampling_rate();
        Fo = 0.0;
        //! \bug detect actual input file size in samples
        NoOfInputSamples = -1;

        //! \note MinGW printf has problems with long double
        sprintf(input_info, "FLT file with %s signal\n"
              "Fo = %.3f MHz\n"
              "Fs = %.3f MSa/s\n"
              "No of samples = %ld"
              , (IsComplex == true) ? "complex" : "real"
              , double(Fo / 1000000), double(Fs / 1000000), (long int)NoOfInputSamples );
      }
      break;
    case DSP::e::FileType::FT_raw:
    default:
      sprintf(input_info, "Unsupported input file format");
      break;
  }

  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  InitializeElement(buffer_size_in, no_of_output_segments_in);

  SetInputSamplingRate(Fs);

  //! \bug this is max buffer size check if it is not used as buffer size
  raw_buffer_size = input_file->GetRawBufferSize(GetOutputSegmentSize(-2));
  raw_buffer = new uint8_t[raw_buffer_size];
  //cplx_buffer = new DSP::Complex[buffer_size];
}

T_FILEinput::~T_FILEinput(void)
{
  if (raw_buffer != NULL)
  {
    delete [] raw_buffer;
    raw_buffer = NULL;
  }
  /*
  if (cplx_buffer != NULL)
  {
    delete [] cplx_buffer;
    cplx_buffer = NULL;
  }
  */
  if (input_file != NULL)
  {
    delete input_file;
    input_file = NULL;
  }
}

void T_FILEinput::ProcessUserData(void *userdata)
{

}

// returns number of samples
const long long T_FILEinput::GetNoOfFileSamples(void)
{
  return NoOfInputSamples;
}

unsigned int T_FILEinput::ElementOverlapSegmentSize(void)
{
  return 0; // no overlap needed
}

unsigned int T_FILEinput::ElementOutputSegmentSize(unsigned int input_segment_size)
{
  return input_segment_size; // no segment size change during processing
}
unsigned int T_FILEinput::MaxNumberOfProtectedSamples(void)
{
  return 0;
}


bool T_FILEinput::Process(E_processing_DIR processing_DIR)
{
  unsigned int BytesRead, samples_read;
  unsigned int current_segment_ind;
  DSP::Complex_vector *cplx_buffer;
  long long no_to_skip, no_of_skipped;

  // +++++++++++++++++++++++++++++++++++++++++++++++++++ //
  // check if samples need to be skipped
  // lock segment again
  cplx_buffer = LockCurrentSegment<DSP::Complex_vector>(processing_DIR);
  if (cplx_buffer == NULL)
  {
    DSP::log << DSP::e::LogMode::Error <<"T_FILEinput::Process"<< DSP::e::LogMode::second <<"output buffer lock failure"<<endl;
    return false; // stop processing
  }

  no_to_skip = GetNoOfSamplesToStart_forward();
  if (no_to_skip > 0)
  {
    no_of_skipped = input_file->SkipSamples(no_to_skip);

    // zero próbek wczytanych do bufora
    current_segment_ind = UnlockCurrentSegment<DSP::Complex>(0);

    if (no_of_skipped != no_to_skip)
    {
      DSP::log << DSP::e::LogMode::Error <<"T_FILEinput::Process"<< DSP::e::LogMode::second << "samples skip failed"<<endl;
      return false;
    }
    input_discrete_time[(current_segment_ind+1) % no_of_output_segments]
                        = input_discrete_time[current_segment_ind] + no_to_skip;

    //! Lock segment again
    cplx_buffer = LockCurrentSegment<DSP::Complex_vector>(processing_DIR);
    if (cplx_buffer == NULL)
    {
      DSP::log << DSP::e::LogMode::Error <<"T_FILEinput::Process"<< DSP::e::LogMode::second << "output buffer lock failure"<<endl;
      return false; // stop processing
    }
  }
  else
    if (no_to_skip == -1)
    {
      // unlock segment with zero samples read
      current_segment_ind = UnlockCurrentSegment<DSP::Complex>(0);

      DSP::log << "T_FILEinput::Process"<< DSP::e::LogMode::second << "segment processing has ended"<<endl;
      return false;
    }
  // +++++++++++++++++++++++++++++++++++++++++++++++++++ //

  //! \bug 5. modify Process function so the time ranges will be used
  // n_start_with_overlap
  // n_end_with_overlap
  // notice: this variables might be NULL
  /*! a. fill input_discrete_time for segments
   *  b. if no output has been processed skip to next processing segment or finish
   *
   */
  //! \bug ProcessOutputElements must also be modified for full start/end support

DSP::Float_vector float_vector(GetMaxInputSegmentSize() * 2); // Allocate space for real and imaginary parts
for (int i = 0; i < GetMaxInputSegmentSize(); i++) {
  float_vector[i*2] = cplx_buffer->at(i).re;
  float_vector[i*2 + 1] = cplx_buffer->at(i).im;
}
  BytesRead = input_file->ReadSegmentToBuffer(
     //GetOutputSegmentSize(-1) == 0, // buffer_size in samples
     //GetMaxInputSegmentSize(),
     //raw_buffer, // raw_buffer_size == buffer_size * sample_size / 8
     float_vector, // size == buffer_size * no_of_channels
     (input_file_is_mono == true) ? 1 : 0
     );

  samples_read = BytesRead/((input_file->GetSampleSize()) / 8);
  if (BytesRead < raw_buffer_size)
  {
    // if last read sample is not complete, delete it too
    //memset(cplx_buffer+samples_read, 0x00, (GetOutputSegmentSize(-1) - samples_read)*sizeof(DSP_complex));
    memset(cplx_buffer+samples_read, 0x00, (GetMaxInputSegmentSize() - samples_read)*sizeof(DSP::Complex));
  }

  //// test
  //for (unsigned int ind = 0; ind < GetMaxInputSegmentSize(); ind++)
  //{
  //  cplx_buffer[ind].re = 1.0;
  //  cplx_buffer[ind].im = 0.0;
  //}
  current_segment_ind = UnlockCurrentSegment<DSP::Complex>(samples_read);

  switch (processing_DIR)
  {
    case E_PD_forward:
      // update discrete time of the first sample of the next segment
      input_discrete_time[(current_segment_ind+1) % no_of_output_segments]
                          = input_discrete_time[current_segment_ind] + samples_read;
      break;
    case E_PD_backward:
      // update discrete time of the first sample of the current segment
      //! \todo better support for backward processing would be nice
      input_discrete_time[current_segment_ind]
         = input_discrete_time[(current_segment_ind+1) % no_of_output_segments]
           - samples_read;
      break;
    default:
      DSP::log << DSP::e::LogMode::Error <<"T_FILEinput::Process" << DSP::e::LogMode::second <<"Unsupported processing dir"<<endl;
      break;
  }

  // output no 0
  if (ProcessOutputElements<DSP::Complex>(processing_DIR, current_segment_ind)
      == false)
  {
    //! \bug implement skipping to the next timing range sample
    // ??? <- start poszukiwania od input_discrete_time[(current_segment_ind+1) % no_of_output_segments]
    // FindNextSampleToRead();
  }

  return true; // continue processing
}


// +++++++++++++++++++++++++++++++++++++ //
// +++++++++++++++++++++++++++++++++++++ //
T_FILEoutput::T_FILEoutput(const char *filename, unsigned int sampling_rate, bool write_mono_in)
  : T_OutputStackElement(E_OBT_complex, 0, 0) // no outputs
{
  write_mono = write_mono_in;

  if (write_mono == true)
  {
    SetName("T_FILEoutput(mono)", false);

    //! \todo determine file type on the basis of filename extension
    output_file = new DSP::u::FileOutput(filename,
        DSP::e::SampleType::ST_short, 1, DSP::e::FileType::FT_wav, sampling_rate);
  }
  else
  {
    SetName("T_FILEoutput(cplx)", false);

    //! \todo determine file type on the basis of filename extension
    output_file = new DSP::u::FileOutput(filename,
        DSP::e::SampleType::ST_short, 2, DSP::e::FileType::FT_wav, sampling_rate);
  }

  raw_buffer_size = 0;
  raw_buffer = NULL;
  //cplx_buffer = new DSP::Complex[buffer_size];
}

T_FILEoutput::~T_FILEoutput(void)
{
  if (raw_buffer != NULL)
  {
    delete [] raw_buffer;
    raw_buffer = NULL;
  }
  if (output_file != NULL)
  {
    delete output_file;
    output_file = NULL;
  }
}

unsigned int T_FILEoutput::ElementOverlapSegmentSize(void)
{
  return 0; // no overlap needed
}

unsigned int T_FILEoutput::ElementOutputSegmentSize(unsigned int input_segment_size)
{
  return 0; // no output data
}
unsigned int T_FILEoutput::MaxNumberOfProtectedSamples(void)
{
  return 0;
}

unsigned int T_FILEoutput::Process_cplx(E_processing_DIR processing_DIR,
      unsigned int NoOfSamples, DSP::Complex_ptr InputData)
{
  unsigned int temp_size, BytesWritten;
  //DSP::Complex *cplx_buffer; // we don't use output buffer

  temp_size = output_file->GetRawBufferSize(NoOfSamples);
  if (raw_buffer_size < temp_size)
  {
    if (raw_buffer != NULL)
      delete [] raw_buffer;
    raw_buffer_size = temp_size;
    raw_buffer = new uint8_t[raw_buffer_size];
  }
DSP::Float_vector float_vector(GetMaxInputSegmentSize() * 2); // Allocate space for real and imaginary parts
for (int i = 0; i < GetMaxInputSegmentSize(); i++) {
  float_vector[i*2] = (InputData)->re;
  float_vector[i*2 + 1] = (InputData)->im;
}
  BytesWritten = output_file->WriteSegmentFromBuffer(
     //NoOfSamples, // buffer_size in samples
     //raw_buffer, // raw_buffer_size == buffer_size * sample_size / 8
     float_vector, // size == buffer_size * no_of_channels
     (write_mono==true) ? 1:0  // skip imaginary part for mono output file
     );

  // no outputs for this block
  // ProcessOutputElements(processing_DIR, current_segment_ind);

  return 0; // always writes all input data
}


// +++++++++++++++++++++++++++++++++++++ //
// +++++++++++++++++++++++++++++++++++++ //
unsigned int T_DDS::log2_ceil(unsigned int L)
{
  unsigned int temp_L;
  unsigned int korekta;
  unsigned int k_;

  temp_L = L;

  k_ = 0;
  korekta = 0;
  while (temp_L > 1)
  {
    //if (((temp_L&1U) != 0) && (temp_L > 1))
    if ((temp_L&1U) != 0)
    {
      DSP::log << DSP::e::LogMode::Error <<"T_DDS"<< DSP::e::LogMode::second << "L is not the power of 2"<<endl;
      korekta = 1;
    }

    temp_L >>= 1;
    k_++;
  }
  return k_+korekta;
}

// should be power of 2 for improved performance
unsigned int T_DDS::LUT_size = 0;
// log2(LUT_size)
unsigned int T_DDS::LUT_k = 0;
unsigned int T_DDS::LUT_index_mask = 0;
// one period (LUT_size) of complex sinusoid
DSP::Complex *T_DDS::LUT = NULL;
// ----------------------------------------- //
unsigned int T_DDS::LUT2_size = 0;
unsigned int T_DDS::LUT2_k = 0;
unsigned int T_DDS::LUT2_index_mask = 0;
// sin(dw)
DSP::Float *T_DDS::LUT2a = NULL;
// sin(dw/2)
DSP::Float *T_DDS::LUT2b = NULL;

/* Initializes class
 *
 * LUT_size should be power of 2 for improved performance
 *
 * LUT based frequency => w_LUT = k * 2*pi/LUT_size, where k is integer
 *
 * LUT interpolation based frequency => w = w_LUT / L,  where L is integer
 *  - generated by iteration - step by step
 *
 * Generally frequencies are k/L * 2*pi/LUT_size
 *
 *  - initializes LUT table
 *  .
 */
T_DDS::T_DDS(void)
  : T_OutputStackElement(E_OBT_complex, 1, 2) // QDD and PSD
{
  SetName("T_DDS", false);

  LockLUTtables();

  LUT_index = 0;
  LUT2_index = 0;

  SetFreq(0.0, 8000000);
}

wxCriticalSection T_DDS::CS_DDS;
int T_DDS::LUT_lock_index = 0;
void T_DDS::LockLUTtables(unsigned int LUT_size_in, unsigned int LUT2_size_in)
{
  unsigned int ind;

  CS_DDS.Enter();
  if ((LUT != NULL) && (LUT2a != NULL) && (LUT2b != NULL))
  { // just increase the lock index
    if ((LUT_size != LUT_size_in) || (LUT2_size != LUT2_size_in))
      DSP::log << DSP::e::LogMode::Error <<"T_DDS::LockLUTtables"<< DSP::e::LogMode::second <<"LUT tables already locked for different table sizes"<<endl;
    LUT_lock_index++;

    //char text[1024];
    //sprintf(text, "LUT_lock_index=%i", LUT_lock_index);
    //DSP::log << DSP::e::LogMode::Error <<"T_DDS::LockLUTtables", text);
    CS_DDS.Leave();
    return;
  }
  else
  {
    if (LUT != NULL)
    {
      delete [] LUT;
      LUT = NULL;
    }
    if (LUT2a != NULL)
    {
      delete [] LUT2a;
      LUT2a = NULL;
    }
    if (LUT2b != NULL)
    {
      delete [] LUT2b;
      LUT2b = NULL;
    }
  }

  // ++++++++++++++++++++++++++++++++++++++++++ //
  LUT_size = LUT_size_in;
  LUT_k = log2_ceil(LUT_size);
  LUT_size = (1 << LUT_k);
  LUT_index_mask = (1U << LUT_k)-1;

  LUT = new DSP::Complex[LUT_size];
  for (ind = 0; ind < LUT_size; ind++)
  {
    LUT[ind].re = COS((DSP::M_PIx2/LUT_size) * ind);
    LUT[ind].im = SIN((DSP::M_PIx2/LUT_size) * ind);
  }

  // ----------------------------------------- //
  LUT2_size = LUT2_size_in;
  LUT2_k = log2_ceil(LUT2_size);
  LUT2_size = (1 << LUT2_k);
  LUT2_index_mask = (1U << LUT2_k)-1;

  LUT2a = new DSP::Float[LUT2_size];
  LUT2b = new DSP::Float[LUT2_size];
  for (ind = 0; ind < LUT2_size; ind++)
  {
    LUT2a[ind] = SIN(((DSP::M_PIx2/LUT2_size) * ind) / LUT_size);
    LUT2b[ind] = SIN(((DSP::M_PIx1/LUT2_size) * ind) / LUT_size);
  }

  LUT_lock_index++;

  //char text[1024];
  //sprintf(text, "LUT_lock_index=%i", LUT_lock_index);
  //DSP::log << DSP::e::LogMode::Error <<"T_DDS::LockLUTtables", text);
  // ++++++++++++++++++++++++++++++++++++++++++ //
  CS_DDS.Leave();
}

void T_DDS::ReleaseLUTtables(void)
{
  CS_DDS.Enter();

  if (LUT_lock_index <= 0)
  {
    DSP::log << DSP::e::LogMode::Error <<"T_DDS::ReleaseLUTtables"<< DSP::e::LogMode::second << "cannot release: already fully unlocked"<<endl;
    CS_DDS.Leave();
    return;
  }
  LUT_lock_index--;

  if (LUT_lock_index == 0)
  {
    if (LUT != NULL)
    {
      delete [] LUT;
      LUT = NULL;
    }
    LUT_size = 0;
    if (LUT2a != NULL)
    {
      delete [] LUT2a;
      LUT2a = NULL;
    }
    if (LUT2b != NULL)
    {
      delete [] LUT2b;
      LUT2b = NULL;
    }
    LUT2_size = 0;
  }

  CS_DDS.Leave();
}

T_DDS::~T_DDS(void)
{
  ReleaseLUTtables();
}

void T_DDS::SetFreq(double Fo, long double Fs)
{
  fo = double(Fo / Fs);

  //LUT_size
  k = int(round(fo * LUT_size));
  dfo = (fo * LUT_size) - k;
  ko = int(round(dfo * LUT2_size));

  Fo_err = ((dfo * LUT2_size) - ko) * ((Fs / LUT_size) / LUT2_size);

  char tekst[1024];
  sprintf(tekst, "fo=%f, k=%i, dfo=%f, ko=%i, Fo_err=%f", fo, k, dfo, ko, Fo_err);
  DSP::log << tekst<<endl;
}


unsigned int T_DDS::ElementOverlapSegmentSize(void)
{
  return 0; // no need for overlap
}
unsigned int T_DDS::ElementOutputSegmentSize(unsigned int input_segment_size)
{
  return input_segment_size; // no segment size change during processing
}
unsigned int T_DDS::MaxNumberOfProtectedSamples(void)
{
  return 0;
}

unsigned int T_DDS::Process_cplx(E_processing_DIR processing_DIR,
    unsigned int NoOfSamples, DSP::Complex_ptr InputData)
{
  unsigned int current_segment_ind;
  unsigned int ind; //, size;
  DSP::Complex_ptr cplx_buffer;
  DSP::Float sin_corr, sin_half_corr;
  DSP::Complex_ptr phasor, tmp_input;
  DSP::Float temp_re, temp_im;

  cplx_buffer = LockCurrentSegment<DSP::Complex>(processing_DIR);
  if (cplx_buffer == NULL)
  {
    DSP::log << DSP::e::LogMode::Error <<"T_Test::Process"<< DSP::e::LogMode::second <<"output buffer lock failure"<<endl;
    return 0; // not processed but we'd better simulated that all has been done
  }

  // DDS processing implementation
  tmp_input = InputData;
  //size = GetSegmentSize();
  for (ind =0; ind < NoOfSamples; ind++)
  {
    phasor = LUT + LUT_index;

    // (a + jb)(c + jd) = (ac - bd) + j(ad + bc)
    temp_re = (tmp_input->re * phasor->re) - (tmp_input->im * phasor->im);
    temp_im = (tmp_input->re * phasor->im) + (tmp_input->im * phasor->re);
    //temp_re = phasor->re;
    //temp_im = phasor->im;

    sin_corr      = LUT2a[LUT2_index];
    sin_half_corr = LUT2b[LUT2_index];

    //cplx_buffer->re = 1-sin_half_corr;
    //cplx_buffer->im = sin_corr;
    //
    cplx_buffer->re = temp_re - cplx_buffer->re*sin_half_corr - cplx_buffer->im*sin_corr;
    cplx_buffer->im = temp_im - cplx_buffer->im*sin_half_corr + cplx_buffer->re*sin_corr;

    cplx_buffer++;
    tmp_input++;

    LUT2_index += ko;
    if (LUT2_index > LUT2_index_mask)
    {
      LUT2_index &= LUT2_index_mask;
      LUT_index++;
    }
    LUT_index += k;
    LUT_index &= LUT_index_mask;
  }

  //user processing end

  current_segment_ind = UnlockCurrentSegment<DSP::Complex>(NoOfSamples);

  ProcessOutputElements<DSP::Complex>(processing_DIR, current_segment_ind); // output no 0

  return 0; // processed all input samples
}

// +++++++++++++++++++++++++++++++++++++ //
// +++++++++++++++++++++++++++++++++++++ //
T_QD2_BPF::T_QD2_BPF(double B, double Fo, long double Fs)
  : T_OutputStackElement(E_OBT_complex, 1, 2) // DDS and PSD
{
  unsigned int n;

  SetName("T_QD2_BPF", false);
  fo = double(Fo/Fs);
  fp = double((B/2) / Fs);
  fs = 0.5 - fp;

  N_FIR = GetFIRlength(fp, fs);

  if (N_FIR == 0)
    N_FIR = 1;
  h_LPF = DSP::Float_vector(N_FIR);
  DSP::f::LPF_LS (N_FIR, DSP::Float(fp), DSP::Float(fs), h_LPF, 1.0);

  char tekst[1024];
  h_BPF = new DSP::Complex[N_FIR];
  for (n = 0; n < N_FIR; n++)
  {
    h_BPF[n] = DSP::Complex(COS(DSP::M_PIx2*fo*n), SIN(DSP::M_PIx2*fo*n));
    h_BPF[n].multiply_by(h_LPF[n]);

    sprintf(tekst, "h(%i) = %.10f + j*%.10f", n+1, h_BPF[n].re, h_BPF[n].im);
    DSP::log << tekst<<endl;
  }
}

T_QD2_BPF::~T_QD2_BPF(void)
{
  if (h_BPF != NULL)
  {
    delete [] h_BPF;
    h_BPF = NULL;
  }
  N_FIR = 0;
}

void T_QD2_BPF::SetInputSamplingRate(long double input_sampling_rate)
{
  InputSamplingRate = input_sampling_rate;
  SetOutputSamplingRate(InputSamplingRate / 2);
}

// Returns LPF prototype filter length based on fp and fs.
unsigned int T_QD2_BPF::GetFIRlength(double &fp_in, double &fs_in)
{
  double temp;
  int N_FIR_temp;
  unsigned int ind;

  // force fs == 0.5 - fp
  //  ==> fs + fp == 0.5
  // but safe the off-care band width: fs - fp
  //  ==> fp = 0.25 - (fs-fp)/2
  temp = fabs(fs_in-fp_in)/2;
  fp_in = 0.25 - temp;
  if (fp_in < N_FIR_half_band_90_min_fp)
    fp_in = N_FIR_half_band_90_min_fp;
  fs_in = 0.5 - fp_in;

  // Y = P(1)*X^N + P(2)*X^(N-1) + ... + P(N)*X + P(N+1)
  temp = N_FIR_half_band_90[0];
  for (ind = 1; ind <= N_FIR_half_band_90_poly_order; ind++)
  {
    temp *= fp_in;
    temp += N_FIR_half_band_90[ind];
  }
  temp = pow(2, temp);

  N_FIR_temp = 3 + 4*int(round((temp - 3)/4));

  return N_FIR_temp;
}

unsigned int T_QD2_BPF::ElementOverlapSegmentSize(void)
{
  return (N_FIR - 1);
}
unsigned int T_QD2_BPF::ElementOutputSegmentSize(unsigned int input_segment_size)
{
  if ((input_segment_size % 2) == 1)
  {
    DSP::log << DSP::e::LogMode::Error <<"T_QDD2::ElementOutputSegmentSize"<< DSP::e::LogMode::second <<"input segment size must be even"<<endl;
  }
  return (input_segment_size >> 1); // no segment size change during processing
}
unsigned int T_QD2_BPF::MaxNumberOfProtectedSamples(void)
{
  return 0;
}

unsigned int T_QD2_BPF::ElementInputSubsegmentSize(void)
{
  return 2U;
}

unsigned int T_QD2_BPF::Process_cplx(E_processing_DIR processing_DIR,
    unsigned int NoOfSamples, DSP::Complex_ptr InputData)
{
  unsigned int current_segment_ind, no_of_output_samples;
  unsigned int ind, n;
  DSP::Complex_ptr temp_input, cplx_buffer;

  cplx_buffer = LockCurrentSegment<DSP::Complex>(processing_DIR);

  //no_output_of_samples = GetOutputSegmentSize(-1);
  no_of_output_samples = NoOfSamples >> 1;
  if ((no_of_output_samples << 1) != NoOfSamples )
    DSP::log << DSP::e::LogMode::Error <<"T_QD2_BPF::Process"<< DSP::e::LogMode::second << "NoOfSamples must be even"<<endl;
  temp_input = InputData;

  for (ind = 0; ind < no_of_output_samples; ind++)
  {
    *cplx_buffer = (*temp_input) * h_BPF[N_FIR-1]; // n = 0
    for (n=1; n<N_FIR; n++)
    {
      *cplx_buffer += (temp_input[n]) * h_BPF[N_FIR-1-n];
    }
    temp_input+=2;
    cplx_buffer++;
  }

  current_segment_ind = UnlockCurrentSegment<DSP::Complex>(no_of_output_samples);

  ProcessOutputElements<DSP::Complex>(processing_DIR, current_segment_ind); // output no 0

  return (NoOfSamples - (no_of_output_samples << 1)); // not always all input samples are processed
}


// +++++++++++++++++++++++++++++++++++++ //
// +++++++++++++++++++++++++++++++++++++ //
T_QD2_LPF::T_QD2_LPF(long double B, long double Fs)
  : T_OutputStackElement(E_OBT_complex, 1, 2) // DDS and PSD
{
  Init(B/Fs);
}

T_QD2_LPF::T_QD2_LPF(long double b)
  : T_OutputStackElement(E_OBT_complex, 1, 2) // DDS and PSD
{
  Init(b);
}

void T_QD2_LPF::SetInputSamplingRate(long double input_sampling_rate)
{
  InputSamplingRate = input_sampling_rate;
  SetOutputSamplingRate(InputSamplingRate / 2);
}


void T_QD2_LPF::Init(long double b)
{
  unsigned int n;

  SetName("T_QD2_LPF", false);

  fp = double(b/2); // (B/2) / Fs;
  fs = 0.5 - fp;

  N_FIR = GetFIRlength(fp, fs);

  if (N_FIR == 0)
    N_FIR = 1;
  h_LPF = DSP::Float_vector(N_FIR);
  DSP::f::LPF_LS (N_FIR, DSP::Float(fp), DSP::Float(fs), h_LPF, 1.0);

  char tekst[1024];
  for (n = 0; n < N_FIR; n++)
  {
    sprintf(tekst, "h(%i) = %.10f", n+1, h_LPF[n]);
    DSP::log << tekst<<endl;
  }
}

T_QD2_LPF::~T_QD2_LPF(void)
{
  N_FIR = 0;
}

// Returns LPF prototype filter length based on fp and fs.
unsigned int T_QD2_LPF::GetFIRlength(double &fp_in, double &fs_in)
{
  double temp;
  int N_FIR_temp;
  unsigned int ind;
  //! \todo Implement this function

  // force fs == 0.5 - fp
  //  ==> fs + fp == 0.5
  // but safe the off-care band width: fs - fp
  //  ==> fp = 0.25 - (fs-fp)/2
  temp = fabs(fs_in-fp_in)/2;
  fp_in = 0.25 - temp;
  if (fp_in < N_FIR_half_band_90_min_fp)
    fp_in = N_FIR_half_band_90_min_fp;
  fs_in = 0.5 - fp_in;

  // Y = P(1)*X^N + P(2)*X^(N-1) + ... + P(N)*X + P(N+1)
  temp = N_FIR_half_band_90[0];
  for (ind = 1; ind <= N_FIR_half_band_90_poly_order; ind++)
  {
    temp *= fp_in;
    temp += N_FIR_half_band_90[ind];
  }
  temp = pow(2, temp);

  N_FIR_temp = 3 + 4*int(round((temp - 3)/4));

  return N_FIR_temp;
}

unsigned int T_QD2_LPF::ElementOverlapSegmentSize(void)
{
  return (N_FIR - 1);
}
unsigned int T_QD2_LPF::ElementOutputSegmentSize(unsigned int input_segment_size)
{
  if ((input_segment_size % 2) == 1)
  {
    DSP::log << DSP::e::LogMode::Error <<"T_QDD2::ElementOutputSegmentSize"<<DSP::e::LogMode::second<<"input segment size must be even"<<endl;
  }
  return (input_segment_size >> 1); // no segment size change during processing
}
unsigned int T_QD2_LPF::MaxNumberOfProtectedSamples(void)
{
  return 0;
}

unsigned int T_QD2_LPF::ElementInputSubsegmentSize(void)
{
  return 2U;
}

unsigned int T_QD2_LPF::Process_cplx(E_processing_DIR processing_DIR,
    unsigned int NoOfSamples, DSP::Complex_ptr InputData)
{
  unsigned int current_segment_ind, no_of_output_samples;
  unsigned int ind, n;
  DSP::Complex_ptr temp_input, cplx_buffer;

  cplx_buffer = LockCurrentSegment<DSP::Complex>(processing_DIR);

  //no_output_of_samples = GetOutputSegmentSize(-1);
  no_of_output_samples = NoOfSamples >> 1;
  if ((no_of_output_samples << 1) != NoOfSamples )
    DSP::log << DSP::e::LogMode::Error <<"T_QD2_BPF::Process"<< DSP::e::LogMode::second << "NoOfSamples must be even"<<endl;
  temp_input = InputData;

  for (ind = 0; ind < no_of_output_samples; ind++)
  {
    *cplx_buffer = (*temp_input) * h_LPF[N_FIR-1]; // n = 0
    for (n=1; n<N_FIR; n++)
    {
      *cplx_buffer += (temp_input[n]) * h_LPF[N_FIR-1-n];
    }
    temp_input+=2;
    cplx_buffer++;
  }

  current_segment_ind = UnlockCurrentSegment<DSP::Complex>(no_of_output_samples);

  ProcessOutputElements<DSP::Complex>(processing_DIR, current_segment_ind); // output no 0

  return (NoOfSamples - (no_of_output_samples << 1)); // not always all input samples are processed
}


// +++++++++++++++++++++++++++++++++++++ //
// +++++++++++++++++++++++++++++++++++++ //
/*! \todo Implement APSD overlapping beside PSD overlapping
 *
 */
T_Spectrogram::T_Spectrogram(E_window window_type_in,
              unsigned int window_size_in, unsigned int FFT_size_in,
              unsigned int NoOfPSDsPerAPSD_in, unsigned int NoOfOverlappedPSDs_in,
              void *parent_task_in)
: T_OutputStackElement(E_OBT_float, 1, 2) // QDD and PSD
{
  SetName("T_Spectrogram", false);

  parent_task = parent_task_in;
  NoOfAPSDsProcessed = 0;

  window_type = window_type_in;
  window_size = window_size_in;
  FFT_size = FFT_size_in;
  NoOfPSDsPerAPSD = NoOfPSDsPerAPSD_in;
  //APSDs_per_segment = APSD_per_segment_in;
  NoOfOverlappedPSDs = NoOfOverlappedPSDs_in;

  // derived parameters
  Overlap = DSP::Float(100.0 - 100.0/NoOfOverlappedPSDs);
  overlap_step = window_size / NoOfOverlappedPSDs;

  //SpecgramFFT.resize(FFT_size); <= private function
  SpecgramFFT.FFTshiftON(true);

  time_window = new DSP::Float[window_size];
  switch (window_type)
  {
    case EW_blackman:
      DSP::f::Blackman(window_size, time_window);
      break;
    case EW_rectangular:
    default:
      DSP::f::Rectangular(window_size, time_window);
      break;
  }

  FFT_input = DSP::Complex_vector(FFT_size);
  /*! NO (FFT_input is overwritten during FFT calculation)):
   *  filling with zeros also deals with zeropadding,
   *  no need to do it during processing. <== so must
   *  be done during processing
   */
  //memset(FFT_input, 0x00, FFT_size * sizeof(DSP::Complex));

  FFT_input.assign(FFT_size, DSP::Complex(0.0, 0.0));
  FFT_output = DSP::Float_vector(FFT_size);

  PSD_2_APSD_index = 0;
}

T_Spectrogram::~T_Spectrogram(void)
{
  if (time_window != NULL)
  {
    delete [] time_window;
    time_window = NULL;
  }
  /*if (FFT_input != NULL)
  {
    delete [] FFT_input;
    FFT_input = NULL;
  }
  if (FFT_output != NULL)
  {
    delete [] FFT_output;
    FFT_output = NULL;
  }*/
}

void T_Spectrogram::PreinitializeElement(unsigned int InputSegmentSize,
                                      unsigned int NoOfOutputSegments)
{
  DSP::log << "T_Spectrogram::InitializeElement"<<DSP::e::LogMode::second<<GetName()<<endl;

  //! \todo Check if input arguments are correct
  PSD_per_segment = InputSegmentSize / window_size;
  if ((InputSegmentSize % window_size) > 0)
    PSD_per_segment++;
  APSDs_per_segment = (NoOfOverlappedPSDs * PSD_per_segment) / NoOfPSDsPerAPSD;
  if ((NoOfOverlappedPSDs * PSD_per_segment) % NoOfPSDsPerAPSD > 0)
    APSDs_per_segment++;
  //NoOfPSDsPerAPSD = (NoOfOverlappedPSDs * PSD_per_segment) / APSDs_per_segment;

//  T_OutputStackElement::InitializeElement(InputSegmentSize, NoOfOutputSegments);

  NoOfAPSDsProcessed = 0;
}

unsigned int T_Spectrogram::NoOfInputSamplesPerAPSD(void)
{
  return NoOfPSDsPerAPSD * overlap_step;
}

unsigned int T_Spectrogram::ElementOverlapSegmentSize(void)
{
  /*! this allows for any overlapping scheme
   * if the user would like to change his mind
   * after the object creation
   *
   * \todo Redesign overlapping scheme.
   *   -# select optimal overlap size
   *   -# properly use overlapping during processing
   *   .
   */
  return (window_size - 1);
}
unsigned int T_Spectrogram::ElementOutputSegmentSize(unsigned int input_segment_size)
{
  //! spectrogram output buffer management
  /*! Output buffer should store PSDs.
   *   -# Number of stored PSDs == APSDs_per_segment.
   *   -# FFT_size - number of samples per PSD (sample type: DSP::Float)
   *
   *  DSP::Float ==> FFT_size * APSDs_per_segment
   *
   *  \warning FFT_size in T_SpectrogramDraw depends on this value
   */
  return (FFT_size * APSDs_per_segment);
}
unsigned int T_Spectrogram::MaxNumberOfProtectedSamples(void)
{
  return FFT_size;
}
unsigned int T_Spectrogram::ElementInputSubsegmentSize(void)
{
  return overlap_step;
}

/*! Complex input data, real output data
 */
unsigned int T_Spectrogram::Process_cplx(E_processing_DIR processing_DIR,
    unsigned int NoOfSamples, DSP::Complex_ptr InputData)
{
  unsigned int ind1, ind2, ind3, ind4;
  unsigned int current_segment_ind;
  unsigned int no_of_unprocessed_samples, no_of_output_samples;
  unsigned int no_of_protected_samples;
  DSP::Float_ptr real_buffer;
  DSP::Complex_ptr segment_start;
  DSP::Float_ptr current_APSD;

  real_buffer = LockCurrentSegment<DSP::Float>(processing_DIR);
  if (real_buffer == NULL)
  {
    DSP::log << DSP::e::LogMode::Error <<"T_Spectrogram::Process"<< DSP::e::LogMode::second << "output buffer lock failure"<<endl;
    return 0; // nothing done but simulate that all has been done
  }
  // GetSegmentSize()
  current_APSD = real_buffer;

  // PSD_per_segment * NoOfOverlappedPSDs
  segment_start = InputData;
  no_of_unprocessed_samples = NoOfSamples;
  no_of_output_samples = 0; no_of_protected_samples = 0;
  for (ind1=0; ind1 < PSD_per_segment; ind1++)
  {
    for (ind2=0; ind2 < NoOfOverlappedPSDs; ind2++)
    { //
      if (no_of_unprocessed_samples < overlap_step)
        break;

      //memcpy(FFT_input, segment_start, window_size*sizeof(DSP::Complex));
      std::copy(segment_start, segment_start + window_size, FFT_input.begin());
      //memset(FFT_input+window_size, 0x00, (FFT_size-window_size)*sizeof(DSP::Complex));
      std::fill(FFT_input.begin() + window_size, FFT_input.end(), DSP::Complex(0.0f, 0.0f));
      for (ind3=0; ind3 < window_size; ind3++)
        FFT_input[ind3].multiply_by(time_window[ind3]);

      SpecgramFFT.abs2FFT(FFT_size, FFT_output, FFT_input);

      // ++++++++++++++++++++++++++++++++++++++++++ //
      // spectrogram processing
      // ++++++++++++++++++++++++++++++++++++++++++ //
      // evaluate Averaged PSD (APSD)
      if (PSD_2_APSD_index == 0)
      { // initialize current APSD
        //memcpy(current_APSD, FFT_output, FFT_size*sizeof(DSP::Float));
        std::copy(FFT_output.begin(), FFT_output.end(), current_APSD);
        PSD_2_APSD_index++;
        no_of_protected_samples += FFT_size;
      }
      else
      { // add next PSD segment
        for (ind4 = 0; ind4 < FFT_size; ind4++)
          current_APSD[ind4] += FFT_output[ind4];

        PSD_2_APSD_index++;
      }

      // if it was the last PSD needed for this APSD
      if (PSD_2_APSD_index == NoOfPSDsPerAPSD)
      { // scale APSD segment
        for (ind4 = 0; ind4 < FFT_size; ind4++)
          current_APSD[ind4] /= (NoOfPSDsPerAPSD * window_size);

        // move to next APSD
        PSD_2_APSD_index = 0;
        // update processed APSDs counter
        NoOfAPSDsProcessed++;
        if (NoOfAPSDsProcessed == 100)
        {
          if (parent_task != NULL)
            CallPauseTaskProcessing(parent_task);
        }


        current_APSD += FFT_size;
        //! \todo checking if we do not move past the allocated memory
        no_of_output_samples += FFT_size;
        no_of_protected_samples = 0;
      }

      // skip to next segment
      segment_start += overlap_step;
      no_of_unprocessed_samples -= overlap_step;

    }
    if (no_of_unprocessed_samples < overlap_step)
      break;
  }

  current_segment_ind = UnlockCurrentSegment<DSP::Float>(no_of_output_samples, no_of_protected_samples);

  ProcessOutputElements<DSP::Float>(processing_DIR, current_segment_ind); // output no 0

  // not always all input samples are processed
  return (no_of_unprocessed_samples); //(NoOfSamples % overlap_step);
}


// +++++++++++++++++++++++++++++++++++++ //
// +++++++++++++++++++++++++++++++++++++ //
T_SpectrogramDraw::T_SpectrogramDraw(
    MyGLCanvas *GLcanvas_in, unsigned int GLcanvasIndex_in,
     unsigned int FFT_size_in, unsigned int NoOfDisplayedPSDs_in,
     unsigned int NoOfSamplesPerAPSD_in,
     long double sampling_rate_in,
     long double F_L_in, long double F_U_in,
     long double CenterFrequency_in,
     T_time_base recording_time_in,
     long long time_offset,
     DSP::Float min_in, DSP::Float max_in, bool Use_dB_in)
: T_OutputStackElement(E_OBT_complex, 0, 0) // no outputs
{
  SetName("T_SpectrogramDraw", false);

  coordinates_mode = 0;

  GLcanvas = GLcanvas_in;
  GLcanvasIndex = GLcanvasIndex_in;

  no_of_output_segments = 10000; //15; // start from drawing

  //APSD_per_segment = APSD_per_segment_in;
  NoOfDisplayedPSDs = NoOfDisplayedPSDs_in;
  FFT_size = FFT_size_in;

  APSD_time_offset = time_offset; //0;
  APSD_index = 0;

  min = min_in; max = max_in;
  Use_dB = Use_dB_in;

  NoOfSamplesPerAPSD = NoOfSamplesPerAPSD_in;
  sampling_rate = sampling_rate_in;
  F_L = F_L_in; F_U = F_U_in;
  CenterFrequency = CenterFrequency_in;

  //! ustaw czas początku rejestracji => globalnie
  current_time.Copy(recording_time_in); // date and time of recording start

  //SpectrogramStack = NULL;
  SpectrogramStack = new T_PlotsStack(NoOfDisplayedPSDs, FFT_size);
  SpectrogramStack->InitialiseSpectrogram(F_L, F_U, CenterFrequency, NoOfSamplesPerAPSD, sampling_rate);
}

T_SpectrogramDraw::~T_SpectrogramDraw(void)
{
  DSP::log << "T_SpectrogramDraw::~T_SpectrogramDraw"<< DSP::e::LogMode::second << "start"<<endl;
  GLcanvas->SpecDraw = NULL;
  if (SpectrogramStack != NULL)
  {
    delete SpectrogramStack;
    SpectrogramStack = NULL;
  }
  DSP::log << "T_SpectrogramDraw::~T_SpectrogramDraw"<< DSP::e::LogMode::second << "end"<<endl;
}

/*
void T_SpectrogramDraw::PreinitializeElement(unsigned int InputSegmentSize,
                                          unsigned int NoOfOutputSegments)
{
  DSP::log << "T_Spectrogram::InitializeElement", (char *)GetName());

  //! \todo check if InputSegmentSize is the multiple of APSD_per_segment
  // input segment size is for DSP::Complex
  // but we use it as DSP::Float
  //FFT_size = (2*InputSegmentSize) / APSD_per_segment;
  FFT_size = InputSegmentSize / APSD_per_segment;

  if (SpectrogramStack == NULL)
    SpectrogramStack = new T_PlotsStack(NoOfDisplayedPSDs, FFT_size);
  else
    SpectrogramStack->Reset(FFT_size, NoOfDisplayedPSDs);
  SpectrogramStack->InitialiseSpectrogram(F_L, F_U, sampling_rate);

  //T_OutputStackElement::InitializeElement(InputSegmentSize, NoOfOutputSegments);
}
*/
unsigned int T_SpectrogramDraw::ElementOverlapSegmentSize(void)
{
  return 0; // No need for overlapping
}
unsigned int T_SpectrogramDraw::ElementOutputSegmentSize(unsigned int input_segment_size)
{
  return 0; //! No output
}
unsigned int T_SpectrogramDraw::MaxNumberOfProtectedSamples(void)
{
  return 0;
}
unsigned int T_SpectrogramDraw::ElementInputSubsegmentSize(void)
{
  return FFT_size;
}

/*! \todo Implement user defined x, y and colour limits
 */
unsigned int T_SpectrogramDraw::Process_real(E_processing_DIR processing_DIR,
    unsigned int NoOfSamples, DSP::Float_ptr InputData)
{
  unsigned int ind2;
  DSP::Float_ptr input_segment;
  DSP::Float_ptr temp_slot;
  unsigned int no_of_unprocessed_samples;
  long double slot_time;

  // ++++++++++++++++++++++++++++++ //
  // user processing start

  //! Lock drawing data
  GLcanvas->LockDrawingData(GLcanvasIndex);

  //! \todo work based on processing_DIR

  // Preprocess plot data
  input_segment = InputData; no_of_unprocessed_samples = NoOfSamples;
  //no_of_output_segments = 0;
  if (Use_dB == true)
  {
    while (no_of_unprocessed_samples >= FFT_size)
    {
      slot_time = APSD_time_offset + APSD_index * NoOfSamplesPerAPSD;
      slot_time /= sampling_rate;

      temp_slot = SpectrogramStack->GetSlot(true);
      SpectrogramStack->Set_SlotTime(slot_time);
      SpectrogramStack->Set_SlotDataSize(FFT_size);
      for (ind2 = 0; ind2 < FFT_size; ind2++)
      {
        //temp_slot[ind2] = ((10*log10(input_segment[ind2]))-min)/(max-min);
        temp_slot[ind2] = 10*log10(input_segment[ind2]);
      }
      SpectrogramStack->NextSlot(false);
      APSD_index++;

      input_segment += FFT_size; no_of_unprocessed_samples -= FFT_size;
      no_of_output_segments++;
    }
  }
  else
  {
    while (no_of_unprocessed_samples >= FFT_size)
    {
      slot_time = APSD_time_offset + APSD_index * NoOfSamplesPerAPSD;
      slot_time /= sampling_rate;

      temp_slot = SpectrogramStack->GetSlot(true);
      SpectrogramStack->Set_SlotTime(slot_time);
      SpectrogramStack->Set_SlotDataSize(FFT_size);
      for (ind2 = 0; ind2 < FFT_size; ind2++)
        temp_slot[ind2] = (input_segment[ind2]-min)/(max-min);
      SpectrogramStack->NextSlot(false);
      APSD_index++;

      input_segment += FFT_size; no_of_unprocessed_samples -= FFT_size;
      no_of_output_segments++;
    }
  }

  //! Drawing data updated so unlock it
  GLcanvas->UnlockDrawingData(GLcanvasIndex);

  //DSP::log << "T_SpectrogramDraw::Process"<< DSP::e::LogMode::second << "LockGUI"<<endl;
//  if (no_of_output_segments > 0)
  if (no_of_output_segments > 5) // draw every 15 slots
  {
    no_of_output_segments = 0;
    ////! Inform Canvas to draw
    //// create event
    //wxCommandEvent event( wxEVT_DRAW_NOW, ID_DrawNow );
    //event.SetClientData( this );
    //// Send it
    //((MyChild *)GLcanvas->GetParent())->AddPendingEvent( event );
    GLcanvas->SpecDraw = this;
    GLcanvas->Refresh(); // invalidate window
    GLcanvas->Update();  // refresh canvas immediately
    /*! \bug bez Update tutaj lepsze wykorzystanie procesora dla HT ale przycina GUI.
     *   Przetestowaæ na pojedynczym procesorze bez HT. Należy opracowaæ
     *   jakieś rozwiązanie nie blokujące GUI.
    */
    //wxGetApp().Yield(true);???
  }

  // user processing end
  // ++++++++++++++++++++++++++++++ //

  // not always all input samples are processed
  // ??? probably all should be processed
  return no_of_unprocessed_samples; //(NoOfSamples % FFT_size);
}

void T_SpectrogramDraw::Draw(void)
{
  // ++++++++++++++++++++++++++++++++++++++++ //
  // Draw data
  //::wxMutexGuiEnter();
  CS_OnDraw.Enter();

  GLcanvas->SetCurrent(*(GLcanvas->GLcontext));
  // Init OpenGL once, but after SetCurrent
  if (GLcanvas->InitGL())
    SpectrogramStack->SetWGLFont(GLcanvas->m_bmf);


  /*
  glLoadIdentity();
  //glRotatef(45.0, 0.0, 0.0, 1.0);
  glTranslatef(0.0, 0.0, 0.0);
  glScalef(0.005, 0.05, 0.0);
  GLcanvas->m_olf->DrawString("test 2");
   */
  //glDisable(GL_LINE_SMOOTH);
  // ++++++++++++++++++++++++++++++++++++++++ //
  // ++++++++++++++++++++++++++++++++++++++++ //
  glLoadIdentity();
  // clear whole axis field
  SpectrogramStack->SubPlot(1,1,-1, 0, 0, true);
  //  void SubPlot(int Rows, int Cols, int index, int WindowW, int WindowH, bool Clear);
  // ++++++++++++++++++++++++++++++++++++++++ //
  //glPushClientAttrib( GL_CLIENT_ALL_ATTRIB_BITS  );
  //glPushMatrix();

  // select axes plot field for drawing
  SpectrogramStack->SubPlot(1,1,1, 0, 0, false);
  //SpectrogramStack->DrawSpecgram2_lin(1.0, CLR_hot);
  SpectrogramStack->DrawSpecgram2_dB(max, max-min, CLR_jet);
  //SpectrogramStack->DrawSpecgram3_dB(max, max-min, CLR_jet);

  //glPopMatrix();
  //glPopClientAttrib();

  // ++++++++++++++++++++++++++++++++++++++++ //
  // draw axis and labels
//  void PlotAxis(double to, double Fo, double dt, double dF);
  int slot_no;
  double to, dt, Fo, dF;
  slot_no = 0;
  to = SpectrogramStack->Get_SlotTime(slot_no);
  dt = SpectrogramStack->Get_TimeWidth();
  Fo = SpectrogramStack->Get_Fo();
  dF = SpectrogramStack->Get_FrequencyWidth();
  //! \todo Axis initialization can be done just once
  SpectrogramStack->InitAxis(dt, dF, 0, 0, E_TM_white);
  SpectrogramStack->PlotAxis(to, Fo);

//  glColor3f ( 1.f, 0.f, 0.f );
//  //glEnable(GL_LINE_SMOOTH);
//  //glLineWidth(0.5);
//  SpectrogramStack->DrawString(0.0, 0.0, "Proba mikrofonu ąśæ 0123456789",
//                20, 18,
//                DS_WGL_bitmap);
  // ++++++++++++++++++++++++++++++++++++++++ //
  // ++++++++++++++++++++++++++++++++++++++++ //
  glFlush();
  GLcanvas->SwapBuffers();

  // ++++++++++++++++++++++++++++++++++++++++ //
  // ++++++++++++++++++++++++++++++++++++++++ //
  wxPoint poz;
  poz = GLcanvas->ScreenToClient(::wxGetMousePosition());
  OnMouseMove(poz.x, poz.y, GLcanvas->mouse_captured);

  //::wxMutexGuiLeave();
  CS_OnDraw.Leave();
  // ++++++++++++++++++++++++++++++++++++++++ //
}

void T_SpectrogramDraw::OnSize(int w, int h)
{
  //glViewport(0, 0, (GLint) w, (GLint) h);
  SpectrogramStack->SubPlot(1,1,1,  w, h,  true);
}

bool T_SpectrogramDraw::OnMouseDown(int x, int y)
{
  return SpectrogramStack->OnMouseDown(x, y);
}
void T_SpectrogramDraw::OnMouseUp(int x, int y)
{
  SpectrogramStack->OnMouseUp(x, y);
}
void T_SpectrogramDraw::OnMouseMove(int x, int y, bool captured)
{
  char tekst[1024], time_str[1024];
  long double t, F;
  bool in_subplot;

  //! czas w sekundach od początku rejestracji i częstotliwośæ bezwzględna
  in_subplot = SpectrogramStack->OnMouseMove(x, y, t, F);
  if (((in_subplot == false) && (captured == false)) == false)
  {
    //! ustaw offset czasowy względem początku rejestracji
    current_time.SetTimeOffset(t);

    //sprintf(tekst, "x = %i, y = %i, t = %.2f , F = %.2f", x, y, double(t), double(F));
    switch (coordinates_mode & 0x01)
    {
      case 0: //! coordinates related to file beginning
        current_time.GetTimeString(time_str, 0);
        break;
      case 1:   //! get time string: (1) - real time
      default:  //! get time string: (1) - real time
        current_time.GetTimeString(time_str, 1);
        break;
    }
    switch ((coordinates_mode >> 1) & 0x01)
    {
      case 0: //! coordinates related to file beginning
        //sprintf(tekst, "x = %i, y = %i, t = %s , F = %.2f", x, y, time_str, double(F));
        sprintf(tekst, "t = %s, F = %.2f", time_str, double(F-CenterFrequency));
        break;
      case 1:   //! get time string: (1) - real time
      default:  //! get time string: (1) - real time
        //sprintf(tekst, "x = %i, y = %i, t = %s , F = %.2f", x, y, time_str, double(F));
        sprintf(tekst, "t = %s, F = %.2f", time_str, double(F));
        break;
    }
    GLcanvas->GetParent()->SetLabel(tekst);
  }
}

// +++++++++++++++++++++++++++++++++++++ //
// +++++++++++++++++++++++++++++++++++++ //
T_HB_SRC_Farrow::T_HB_SRC_Farrow(long double Fs1_in, long double Fs2_in)
  : T_OutputStackElement(E_OBT_complex, 1, 2) // DDS and PSD
{
  unsigned int ind;

  SetName("T_HB_SRC_Farrow", false);

  Fs1 = Fs1_in; Fs2 = Fs2_in;

  // ratio of output and input sampling intervals
  Ts2_to_Ts1 = Fs1 / Fs2;
  Ts2_to_Ts1_int = (unsigned int)floor(Ts2_to_Ts1);
  Ts2_to_Ts1_frac = Ts2_to_Ts1 - Ts2_to_Ts1_int;

  // current fractional delay
  /*! \todo Select initial delta_m such that overall filter
   *   is symmetric constant delay FIR filter.
   */
  eps_m = 0;
  // number of input samples to skip for the current output sample
  d_n = 0;

  N_FSD = Farrow_coefs_half_band_N_FSD;
  FarrowOrder = Farrow_coefs_half_band_Farrow_order;
  p_i_n = new DSP::Float_ptr[FarrowOrder+1];
  for (ind = 0; ind <= FarrowOrder; ind++)
    p_i_n[ind] = const_cast<DSP::Float_ptr>(Farrow_coefs_half_band[ind]);
}

T_HB_SRC_Farrow::~T_HB_SRC_Farrow(void)
{
  if (p_i_n != NULL)
  {
    delete [] p_i_n;
    p_i_n = NULL;
  }
}

void T_HB_SRC_Farrow::SetInputSamplingRate(long double input_sampling_rate)
{
  InputSamplingRate = input_sampling_rate;
  SetOutputSamplingRate(InputSamplingRate / Ts2_to_Ts1);
}

unsigned int T_HB_SRC_Farrow::ElementOverlapSegmentSize(void)
{
  return (N_FSD - 1);
}
unsigned int T_HB_SRC_Farrow::ElementOutputSegmentSize(unsigned int input_segment_size)
{
  // output segment size calculation
  return (unsigned int)ceil( Fs2/Fs1 * input_segment_size );
}
unsigned int T_HB_SRC_Farrow::MaxNumberOfProtectedSamples(void)
{
  return 0;
}
/*! \todo test if the resampling is done correctly
 */
unsigned int T_HB_SRC_Farrow::Process_cplx(E_processing_DIR processing_DIR,
    unsigned int NoOfSamples, DSP::Complex_ptr InputData)
{
  unsigned int current_segment_ind, no_of_output_samples;
  unsigned int ind_1, ind_2;
  double old_eps_m;
  DSP::Complex out_value;
  DSP::Complex_ptr input_buffer, input_buffer_end;
  DSP::Complex_ptr cplx_buffer;

  input_buffer = InputData;
  input_buffer_end = InputData + NoOfSamples;
  cplx_buffer = LockCurrentSegment<DSP::Complex>(processing_DIR);

  no_of_output_samples = 0;
  while (input_buffer < input_buffer_end)
  {
    // update input buffer
    if (d_n > 0)
    {
      input_buffer++;
      d_n--;
    }
    else
    {
      // generate output sample for the current delay
      // +++++++++++++++++++++++++++++++++++++++++++++++++++
      // Farrow structure is used for output sample calculation
      //*cplx_buffer = *input_buffer; // zero-order hold

      //*cplx_buffer = 0; out_value = 0;
      out_value = input_buffer[0] * (p_i_n[0][0]);
      for (ind_2 = 1; ind_2 < N_FSD; ind_2++)
        out_value += (input_buffer[ind_2] * (p_i_n[0][ind_2]));
      *cplx_buffer = out_value;

      for (ind_1 = 1; ind_1 <= FarrowOrder; ind_1++)
      {
        out_value = (input_buffer[0] * (p_i_n[ind_1][0]));
        for (ind_2 = 1; ind_2 < N_FSD; ind_2++)
          out_value += (input_buffer[ind_2] * (p_i_n[ind_1][ind_2]));

        (*cplx_buffer).multiply_by(eps_m);
        (*cplx_buffer) += out_value;
      }

      // move to next output slot
      cplx_buffer++;
      no_of_output_samples++;

      // update parameters for next input sample
      old_eps_m = eps_m;
      //delta_m += Ts2_to_Ts1; delta_m -= floor(delta_m);
      //d_n = round(Ts2_to_Ts1 - (delta_m - old_delta_m));
      eps_m += Ts2_to_Ts1_frac;
      d_n = Ts2_to_Ts1_int;
      if (eps_m >= 0.5)
      {
        eps_m--;
        d_n++;
      }
    }
  }

  current_segment_ind = UnlockCurrentSegment<DSP::Complex>(no_of_output_samples);

  // process output blocks
  ProcessOutputElements<DSP::Complex>(processing_DIR, current_segment_ind); // output no 0

  return 0; // this function should always process all input samples
}


// +++++++++++++++++++++++++++++++++++++ //
// +++++++++++++++++++++++++++++++++++++ //
/*! \todo implement version with real input signal
 */
T_FIR::T_FIR(double Fp, double Fs, double Fo, long double Fs_rate)
  : T_OutputStackElement(E_OBT_complex, 1, 2)  // one output - two elements attached: DDS and PSD
{
  unsigned int n;

  if (Fo == 0.0)
  {
    Is_LPF = true;
    SetName("T_FIR<LPF>", false);
  }
  else
  {
    Is_LPF = false;
    SetName("T_FIR<BPF>", false);
  }

  fo = Fo/Fs_rate;
  fp = Fp/Fs_rate;
  fs = Fs/Fs_rate;

  N_FIR = GetFIRlength(fp, fs);

  if (N_FIR == 0)
    N_FIR = 1;

  h_BPF = NULL;

  // design lowpass prototype
  h_LPF = DSP::Float_vector(N_FIR);
  DSP::f::LPF_LS (N_FIR, fp, fs, h_LPF, 1.0);
  if (Is_LPF == false)
  { // convert LPF to BPF
    h_BPF = new DSP::Complex[N_FIR];
    for (n = 0; n < N_FIR; n++)
    {
      h_BPF[n] = DSP::Complex(COS(DSP::M_PIx2*fo*n), SIN(DSP::M_PIx2*fo*n));
      h_BPF[n].multiply_by(h_LPF[n]);
    }
  }

}

T_FIR::~T_FIR(void)
{
  if (h_BPF != NULL)
  {
    delete [] h_BPF;
    h_BPF = NULL;
  }
  N_FIR = 0;
}

// Returns LPF prototype filter length based on fp and fs.
unsigned int T_FIR::GetFIRlength(double &fp_in, double &fs_in)
{
  double temp;
  int N_FIR_temp;
  unsigned int ind;

  // force fs == 0.5 - fp
  //  ==> fs + fp == 0.5
  // but safe the off-care band width: fs - fp
  //  ==> fp = 0.25 - (fs-fp)/2
  temp = fabs(fs_in-fp_in)/2;
  fp_in = 0.25 - temp;
  if (fp_in < N_FIR_half_band_90_min_fp)
    fp_in = N_FIR_half_band_90_min_fp;
  fs_in = 0.5 - fp_in;

  // Y = P(1)*X^N + P(2)*X^(N-1) + ... + P(N)*X + P(N+1)
  temp = N_FIR_half_band_90[0];
  for (ind = 1; ind <= N_FIR_half_band_90_poly_order; ind++)
  {
    temp *= fp_in;
    temp += N_FIR_half_band_90[ind];
  }
  temp = pow(2, temp);

  N_FIR_temp = 3 + 4*int(round((temp - 3)/4));

  return N_FIR_temp;
}

unsigned int T_FIR::ElementOverlapSegmentSize(void)
{
  return (N_FIR - 1);
}
unsigned int T_FIR::ElementOutputSegmentSize(unsigned int input_segment_size)
{
  return input_segment_size; // no segment size change during processing
}
unsigned int T_FIR::MaxNumberOfProtectedSamples(void)
{
  return 0;
}


unsigned int T_FIR::Process_cplx(E_processing_DIR processing_DIR,
    unsigned int NoOfSamples, DSP::Complex_ptr InputData)
{
  unsigned int current_segment_ind;
  unsigned int ind, n;
  DSP::Complex_ptr temp_input, cplx_buffer;

  cplx_buffer = LockCurrentSegment<DSP::Complex>(processing_DIR);

  temp_input = InputData;

  if (Is_LPF == true)
  {
    for (ind = 0; ind < NoOfSamples; ind++)
    {
      *cplx_buffer = (*temp_input) * h_LPF[N_FIR-1]; // n = 0
      for (n=1; n<N_FIR; n++)
      {
        *cplx_buffer += (temp_input[n]) * h_LPF[N_FIR-1-n];
      }
      temp_input++;
      cplx_buffer++;
    }
  }
  else
  {
    for (ind = 0; ind < NoOfSamples; ind++)
    {
      *cplx_buffer = (*temp_input) * h_BPF[N_FIR-1]; // n = 0
      for (n=1; n<N_FIR; n++)
      {
        *cplx_buffer += (temp_input[n]) * h_BPF[N_FIR-1-n];
      }
      temp_input++;
      cplx_buffer++;
    }
  }

  current_segment_ind = UnlockCurrentSegment<DSP::Complex>(NoOfSamples);

  ProcessOutputElements<DSP::Complex>(processing_DIR, current_segment_ind); // output no 0

  // always all input samples are processed
  return 0;
}


