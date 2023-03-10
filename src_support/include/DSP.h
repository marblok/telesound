#ifndef DSP_H_
#define DSP_H_

#include <DSP_lib.h>
#include "MyGLcanvas.h"
#include "wxAddons.h"
#ifdef wxAddons_wxTimePickerCtrl
  #include "wxAddons_Time.h"
#endif

#define DSP_STR_MAX_LENGTH 4095

class T_OutputStackElement;
typedef T_OutputStackElement * T_OutputStackElement_ptr;
typedef T_OutputStackElement_ptr * T_OutputStackElement_ptr_table;

enum E_processing_DIR
{
  E_PD_forward = 0,
  E_PD_backward = 1
};
enum E_lock_status
{
  E_LS_none = 0,    //! not initialized yet
  E_LS_locked = 1,  //! currently locked // data status unpredictable
  E_LS_unlocked = 2 //! unlocked // current segment data correct
};

enum E_OutputBuffeType
{
  E_OBT_float = 0,
  E_OBT_complex = 1
};

typedef unsigned char * BYTE_ptr;

#define MAX_OUTPUT_SEGMENT_SIZE UINT_MAX

class T_InputElement;

//! Base class for processing tree implementation
/*! \todo add discrete / analog time indexes
 *    corresponding to segments' beginnings
 * - take into account processing block delay
 * - overlap size for FIR filters should be N_FIR-1
 *  (this allows to get full size output segment)
 * .
 *
 * \todo Support for multiple inputs.
 *    Multiple elements should be able to call process
 *    for single block. Output block should be
 *    able to distinguish between calling elements.
 *    Problem: how to do this without need for
 *    copying input segment.
 *  - store pointer to segment
 *  - do not process output segments until pointers to
 *   all input segments are collected.
 *  .
 *  \warning No feedback loops are allowed !!!
 *
 */
class T_OutputStackElement : public DSP_name
{
    friend class T_InputElement;

  protected:
    //! Type of the output buffer
    E_OutputBuffeType OutputBufferType;

    //! Number of allocated stack slots
    /*! The same value for each OutputStack is used
     */
    unsigned int OutputStackSize;
    //! Number of separate output stacks, one for each element's output
    unsigned int NoOfStacks;
    //! Number of filled stack slots
    /*! Table with entry for each output stack,
     *  one for each element's output
     */
    unsigned int *NoOfElementsOnStack;
    T_OutputStackElement_ptr_table *OutputStacks;
    //! Table storing info about number of unprocessed input samples
    /*! Each entry corresponds to entry in OutputStacks.
     */
    unsigned int **NoOfUnprocessedInputSamples;

    //! number of output buffer segments
    unsigned int no_of_output_segments;
    //! maximum size of input segment in samples
    unsigned int max_input_segment_size;
    //! maximum size of single output segment in samples
    unsigned int max_output_segment_size;
    //! size of segment overlap in samples
    /*! might differ from overlap size required by individual output elements
     *
     *  \note There are two overlap segments, at the beginning
     *   and in the end of the output buffer.
     *
     *  \warning values must be equal to maximum value returned by the
     *    output elemnt's T_OutputStackElement::ElementOverlapSegmentSize
     *    function
     */
    unsigned int output_segment_overlap;
    //! Table with pointers to starts of output buffer segments
    /*! \note Size of this table == no_of_output_segments + 3 (two overlap segments and end of the output buffer)
     */
    BYTE_ptr *output_segment_starts;
    //! Table sizes of output segments
    /*! \note Size of this table == no_of_output_segments + 2 (two overlap segments)
     */
    unsigned int *output_segment_sizes;
    //! Output buffer
    /*! Internal buffer structure:
     *  -# overlap in
     *  -# segment 0
     *  -# segment 1
     *  -# ...
     *  -# segment no_of_segments - 1
     *  -# overlap out
     *  .
     *
     *  \note [overlap in] is filled with the end of the last standard segment
     *  \note [overlap out] is filled with the beginning of the first standard segment
     */
    BYTE_ptr output_buffer;
    unsigned int output_buffer_size;
 /*! \todo check if this is a good idea
    / /! Table storing times of output segments starts in seconds
    / *! Time should also include Element's delay
     * /
 ???   long double *output_segment_starts_times;
    / /! Sets start time for the next inputs segment
    / *! Element must also update next input segments times for
     * it's output elements.
     *
     *  \note generally element should update
     *  next inputs segment input start automatically
     *  and this function should be call only for mayor
     *  updates (like segment skip).
     * /
 ???   void SetNextInputSegmentStart(long double seconds);
*/

    //! Index of currently locked or recently unlocked segment
    unsigned int current_output_segment;
    //! lock status of the current output segment
    E_lock_status current_output_segment_lock_status;
    //! remembers previous processing dir during output buffer lock
    E_processing_DIR previous_lock_processing_DIR;

    //! Increases numbers of slots on stack
    /*! Allocates twice as large stack as before.
     */
    void IncreaseStackSize(void);

    template <class T>
    void InitializeElement_(unsigned int InputSegmentSize,
                            unsigned int NoOfOutputSegments);

    //! Returns pointer to the input data segment and number of stored samples in NoOfInputSamples
    /*! \warning This is version for forward processing. For backward or skipping
     *   modes other function must be used.
     *
     * \note Actual pointer type depends on what data type
     * is used for output buffer in this processing element.
     *
     * \warning This function must be only used in ProcessOutputElements
     *   and is overwritten in T_InputElement class to support
     *   processing time ranges.
     */
    virtual void *GetInputData_forward
                              (unsigned int &NoOfInputSamples,
                               unsigned int current_segment,
                               unsigned int OutputIndex, unsigned int ElementIndex);
    //! see T_OutputStackElement::GetInputData_forward
    virtual void *GetInputData_backward
                              (unsigned int &NoOfInputSamples,
                               unsigned int current_segment,
                               unsigned int OutputIndex, unsigned int ElementIndex);

    //! Element's input sampling rate (0 if not specified)
    long double InputSamplingRate;
    //! Element's output sampling rate (0 if not specified)
    long double OutputSamplingRate;
    //! Set element's output sampling rate
    /*! If any output element exists update its input sampling rate
     */
    void SetOutputSamplingRate(long double output_sampling_rate);
  public:
    const long double GetOutputSamplingRate(void);

  public:
    T_OutputStackElement(E_OutputBuffeType OutputBufferType_in = E_OBT_complex,
                         unsigned int NoOfOutputs = 1, unsigned int InitialStackSize = 0);
    virtual ~T_OutputStackElement(void);

    //! Deletes the whole stack
    /*! This means that the object is recursively
     *  deleted with all its output elements.
     */
    void DeleteStack(void);

    //! Initializes this element
    /*! Function manages memory allocation for the output segments.
     * @param InputSegmentSize - maximum size of the input segment
     * @param NoOfOutputSegments - number of output segments the element should use
     *
     * \warning The function is recurrent, which means that
     *  it will be called automatically for elements
     *  from the T_OutputStackElement::OutputStacks table.
     *  Thus the user must only call this function for the
     *  parent element of the whole processing tree.
     *
     * \note User can overload this function but base class
     *    prototype should always be called in such an overloaded
     *    function.
     */
    virtual void InitializeElement(unsigned int InputSegmentSize,
                                   unsigned int NoOfOutputSegments);
    //! Element preinitialisation.
    /*! Called at the beginning of T_OutputStackElement::InitializeElement.
     *
     * \note user should always rather override this function
     *  instead of T_OutputStackElement::InitializeElement.
     */
    virtual void PreinitializeElement(unsigned int InputSegmentSize,
                                      unsigned int NoOfOutputSegments);
    //! Repeat segment initialization
    void ReinitializeElement(void);
    //! Set element's input sampling rate
    /*! This function must calculate and set element's output
     *  sampling rate.
     *
     * Default implementation:
     \code
    void T_OutputStackElement::SetInputSamplingRate(long double input_sampling_rate)
    {
      InputSamplingRate = input_sampling_rate;
      SetOutputSamplingRate(InputSamplingRate);
    }
     \endcode
     *
     */
    virtual void SetInputSamplingRate(long double input_sampling_rate);
    //! Returns size of output overlap segment required by the element
    /*! \warning This functions must be implemented in descendant class
     *
     * Returns overlap size required by T_OutputStackElement::Process function.
     * \note If required overlap size is not constant, this function must
     *   return maximum expected overlap segment size.
     * \note This size is in DSP_complex elements
     */
    virtual unsigned int ElementOverlapSegmentSize(void) = 0;
    //! Returns size of standard output segment
    /*! Returned segment size is calculated based on input segment size.
     *
     * \note function must return maximum size of the output buffer
     *       for given input segment size.
     * \note If input segment size varies, user must enter maximum
     *   expected input segment size.
     * \warning This functions must be implemented in descendant class
     */
    virtual unsigned int ElementOutputSegmentSize(unsigned int input_segment_size) = 0;
    //! Returns maximum number of protected samples the stack element might use
    /*! \note this is size of the output buffer segment memory, which can be allocated
     *    and protected between call to UnlockCurrentSegment and LockCurrentSegment
     *    without marking it as filled.
     */
    virtual unsigned int MaxNumberOfProtectedSamples(void) = 0;
    //! Returns size of inputs subsegment
    /*!  This value is used to determine possible maximum value of
     *   unprocessed input samples (it will be one less then the returned value).
     *
     * \note Size of the input segment must be integer multiple of returned value,
     *   otherwise not all input samples will be processed.
     *
     * @return default value is 1.
     *
     * \warning This function will be called before T_OutputStackElement::InitializeElement
     *   function call so it must not depend on any variable updated there.
     *   However, user can use variables initialized in T_OutputStackElement::PreinitializeElement.
     */
    virtual unsigned int ElementInputSubsegmentSize(void);

    //! Adds OutputStackElement on the stack
    /*! \note This function also calls T_OutputStackElement::InitializeElement.
     *
     * Indexing: 0,...,max_output_index - 1.
     *
     * \warning if overloaded in descendant class, this instance
     *   must be called from overloaded function
     *
     * @return Index of the added element on given output stack.
     *   Returns -1 if something went wrong.
     *
     * \todo change name => AttachOutputElement
     * \note several output elements can be connected to single output
     */
    virtual int AddOutput(T_OutputStackElement *Element, unsigned int OutputIndex = 0);
    //! Removes OutputStackElement from the stack
    /*! \warning if overloaded in descendant class, this instance
     *   must be called from overloaded function
     *
     * @return Index of the removed element on given output stack.
     *   Returns -1 if something went wrong.
     */
    virtual int RemoveOutput(T_OutputStackElement *Element, unsigned int OutputIndex = 0);
    //! Returns number of elements registered for given OutputIndex
    unsigned int GetNoOfElements(unsigned int OutputIndex = 0);

    //! Returns output segment size
    /*!
     * @param segment_index output segment index
     * @return size of given output segment or maximum output segment size
     *
     * \note For
     *   - segment_index  == -1 - returns current segment size
     *   - segment_index  == -2 - returns maximum expected segment size
     *   - segment_index  == no_of_output_segments - returns maximum overlap segment size
     *   .
     *
     *  \warning Between calls to LockCurrentSegment and UnlockCurrentSegment
     *    returned current segment size is <b>zero</b>.
     */
    unsigned int GetOutputSegmentSize(int segment_index = -1);
    //! Returns maximum expected input segment size
    unsigned int GetMaxInputSegmentSize(void);

    //! Locks next segment based on processing_DIR
    /*! This should be done only by this element
     *  to access buffer for its output and must be unlocked
     *  before T_OutputStackElement::ProcessOutputElements
     *
     *  // DSP_complex_ptr LockCurrentSegment(E_processing_DIR processing_DIR);
     */
    template<class T>
    T *LockCurrentSegment(E_processing_DIR processing_DIR);
    //! Unlocks segment locked with T_OutputStackElement::LockCurrentSegment
    /*!
     * @param segment_size unlocked segment size
     * @param no_of_protected_samples number of output samples not marked as outputed but still reserved
     * @return unlocked segment index
     *
     *  If segment_size == MAX_OUTPUT_SEGMENT_SIZE then default (maximum) output segment size is used.
     *  \note This function updates overlap segments if necessary.
     *
     *  \Fixed <b>07.05.2009</b> MAX_OUTPUT_SEGMENT_SIZE is no longed the default action.
     *   Problems occurred for variable block length.
     */
    template<class T>
    unsigned int UnlockCurrentSegment(unsigned int segment_size, // = MAX_OUTPUT_SEGMENT_SIZE,
                                      unsigned int no_of_protected_samples = 0);

    //! Calls Process function for output elements registered to output OutputIndex
    /*! Evaluates and passes NoOfSamples and InputData to
     *  the output element's Process function so the current
     *  output segment can be processed properly.
     *
     * \note Overlap segment is managed depending on
     *   processing_DIR and the output element's requirements
     *   (see T_OutputStackElement::InitializeElement)
     *
     * @return true if any of the output elements received
     *   samples to process.
     */
    template<class T>
    bool ProcessOutputElements(E_processing_DIR processing_DIR,
                               unsigned int current_segment,
                               unsigned int OutputIndex = 0);

    //! Element's processing function
    /*! User should call ProcessOutputElements at the end of processing
     *
     *  @return function returns number of unprocessed input samples.
     *
     * \warning Returned number of unprocessed input data must not be larger
     *  then the value returned by T_OutputStackElement::ElementInputSubsegment.
     *
     * \note Continuous InputData vector contains
     *   - overlap segment (depending on the value returned by this function
     *     in response to T_OutputStackElement::InitializeElement function call)
     *   - current segment data
     *   .
     *
     * \warning (NoOfSamples == Segment_size) but input data include
     *    overlap segment thus actuals input data size is (Overlap_segment_size + Segment_size).
     *
     * \note User must override either T_OutputStackElement::Process_cplx
     *   or T_OutputStackElement::Process_real depending on what input data
     *   type derived class is expecting.
     *
     * \warning Position of the overlap segment depends on the processing_DIR
     *  variable:
     *  - E_PD_forward - overlap segment is placed before actual segment data
     *  - E_PD_backward - overlap segment is placed after actual segment data
     *  .
     *
     * \code
unsigned int T_Test::Process_cplx(E_processing_DIR processing_DIR,
    unsigned int NoOfSamples, DSP_complex_ptr InputData)
{
  unsigned int current_segment_ind;
  DSP_complex_ptr cplx_buffer;

  cplx_buffer = LockCurrentSegment(processing_DIR);
  if (cplx_buffer == NULL)
  {
    DSPf_ErrorMessage("T_Test::Process", "output buffer lock failure");
    return;
  }

  //user processing start
  //user processing end

  current_segment_ind = UnlockCurrentSegment();

  ProcessOutputElements(processing_DIR, current_segment_ind); // output no 0

  return 0; // processed all input data
}
      \endcode
     */
    virtual unsigned int Process_cplx(E_processing_DIR processing_DIR,
        unsigned int NoOfSamples, DSP_complex_ptr InputData);
    //! The same as T_OutputStackElement::Process_cplx but with real input data
    virtual unsigned int Process_real(E_processing_DIR processing_DIR,
        unsigned int NoOfSamples, DSP_float_ptr InputData);
};

class T_InputElement : public T_OutputStackElement
{
  protected:
    //! Number of time subtables (ProcessingFinished, n_start, n_end)
    unsigned int NoOfSubtables;
    //! Number of slots if time subtables
    unsigned int *NoOfElementsInSubtables;

    //! tables storing information if processing got past the last sample to process
    /*! Table structure corresponds to the one of OutputStacks
     */
    bool **ProcessingFinished;
    //! tables storing processing input start sample
    /*! if entry < 0 then use default value
     */
    long long **n_start;
    //! tables processing input end sample (last sample to process)
    /*! if entry < 0 then use default value
     */
    long long **n_end;

    //! start times which also incorporate overlapping segments
    /*! \note n_start_with_overlap[ind][ind2] < 0 means that overlap segment must be cleared
     */
    long long **n_start_with_overlap;
    //! end times which also incorporate overlapping segments
    long long **n_end_with_overlap;
    //! start times without overlapping segments but with filled default values
    long long **n_start_with_defaults;
    //! end times without overlapping segments but with filled default values
    long long **n_end_with_defaults;

    //! default processing input start sample
    long long default_n_start;
    //! default processing input end sample (last sample to process)
    long long default_n_end;

    //! table storing discrete times of the first samples of segments
    long long *input_discrete_time;

    //! No of samples in input (eg. in file)
    long long NoOfInputSamples;

    //! changes sizes of ProcessingFinished, n_start and n_end tables
    /*!
     * @param element_added true if new element has been added
     * @param ElementIndex index of added or removed element on given output
     * @param OutputIndex index of the output where element has been added or removed
     *
     * \warning If ElementIndex == -1 just update size if necessary
     *   to prevent further problems (and report error).
     */
    void UpdateTablesSizes(bool element_added,
                           int ElementIndex, unsigned int OutputIndex);
    //! calculates actual start and stop times by taking into account the overlap segment sizes
    /*! a. forward processing
     *  b. backward processing ???
     *  c. <= in this tables set the default values to simplify further processing
     *  d. update ProcessingFinished table
     */
    bool FillTimeTables(E_processing_DIR process_dir = E_PD_forward);
    //! Deletes tables with start/end time information
    /*! Without and with overlap segments:
     *   - n_start_with_overlap
     *   - n_end_with_overlap
     *   - n_start_with_defaults
     *   - n_end_with_defaults
     *   .
     */
    void FreeTimeTables(void);
    //! Looks into time tables and searches for soonest input sample to read
    /*!
     * @param current_sample_to_read - look for soonest start time no smaller than this
     * @return
     *
     *  current_sample_to_read <= 0 means look for any soonest time.
     */
    bool FindNextSampleToRead(long long current_sample_to_read);

    //! returns number of samples to skip to the segment start
    /*! - zero if no skip is required
     *  - -1 if processing has ended
     *  .
     */
    long long GetNoOfSamplesToStart_forward(void);

    //! see T_OutputStackElement::GetInputData_forward
    void *GetInputData_forward
                              (unsigned int &NoOfInputSamples,
                               unsigned int current_segment,
                               unsigned int OutputIndex, unsigned int ElementIndex);
    //! see T_OutputStackElement::GetInputData_backward
    void *GetInputData_backward
                              (unsigned int &NoOfInputSamples,
                               unsigned int current_segment,
                               unsigned int OutputIndex, unsigned int ElementIndex);

    //! True if input signal is complex
    bool IsComplex;
    //! Center frequency
    long double Fo;
    //! Sampling rate
    long double Fs;

    char *input_info;

  public:
    //! returns center frequency
    const long double GetCenterFrequency(void);
    //! returns sampling rate
    const long double GetSamplingRate(void);
    //! returns text with info about input
    const char *GetInputInfo(void);

    //! \bug Must be called in processing initialization when all processing stack elements has been added
    /*! \warning output_segment_overlap  must be correctly set before
     *   call to this function.
     */
    bool UpdateTimeTables(E_processing_DIR process_dir = E_PD_forward, long long current_sample_to_read = -1);

    //! Adds OutputStackElement on the stack
    /*! Updates timing rage data and then redirects call to T_OutputStackElement
     */
    int AddOutput(T_OutputStackElement *Element, unsigned int OutputIndex = 0);
    //! Removes OutputStackElement from the stack
    /*! Updates timing rage data and then redirects call to T_OutputStackElement
     */
    int RemoveOutput(T_OutputStackElement *Element, unsigned int OutputIndex = 0);

    //! Searches for given output block and sets processing time range for it
    /*!
     * @param n_1 - discrete time (index) of the first input file sample to be processed
     * @param n_2 - discrete time (index) of the last input file sample to be processed
     * @param block == NULL means that default time range must be set.
     * @return false if failed
     *
     * \note additional samples might be read to fill the overlap segments
     *  and to allow for processing delay compensation.
     *  ??? transitional state versus processing delay.
     *   maybe processing delay should only be taken into
     *   account while drawing.
     *
     * \warning Changes won't be introduced in processing
     *    until the T_InputElement::UpdateTimeTables function will be call.
     *    Also processing dir change requires call to
     *    T_InputElement::UpdateTimeTables.
     *
     */
    bool SetInputTimeRange(long long n_1, long long n_2,
        T_OutputStackElement *output_block = NULL);

    //! This block cannot be an output block so it ignores NoOfSamples and InputData
    /*! \note This is the input block so for simplicity we
     *  use Process instead of Process_cplx or Process_real.
     *
     *  @return false if branch should stop processing
     */
    virtual bool Process(E_processing_DIR processing_DIR = E_PD_forward) = 0;

    virtual void ProcessUserData(void *userdata) = 0;

    /*! \warning no_of_output_segments_in must be equal to number
     *  of the output segments of the descendant object.
     */
    T_InputElement(E_OutputBuffeType OutputBufferType_in = E_OBT_complex,
                   unsigned int NoOfOutputs = 1, unsigned int InitialStackSize = 0,
                   unsigned int no_of_output_segments_in = 8);
    virtual ~T_InputElement(void);
};

/*! Block by block processing
 *
 * \todo check if T_FILEinput works for files larger than 2GB/4GB.
 */
class T_FILEinput : public T_InputElement
{
  private:
    DSPu_FILEinput *input_file;
    unsigned int raw_buffer_size;
    uint8_t *raw_buffer;
    // unsigned int buffer_size; <== this is managed in T_OutputStackElement
    // DSP_complex *cplx_buffer; <== this is managed in T_OutputStackElement

    bool input_file_is_mono;
    //! Type of the input file
    DSPe_FileType FileType;
  public:
    //T_FILEinput(char *filename, unsigned int buffer_size_in = 1024,
    //    unsigned int no_of_output_segments = 8);
    T_FILEinput(char *filename, char *filedir = NULL,
        unsigned int buffer_size_in = 1024,
        unsigned int no_of_output_segments = 8);
    ~T_FILEinput(void);

    //! returns number of samples
    const long long GetNoOfFileSamples(void);

    unsigned int ElementOverlapSegmentSize(void);
    unsigned int ElementOutputSegmentSize(unsigned int input_segment_size);
    unsigned int MaxNumberOfProtectedSamples(void);

    void ProcessUserData(void *userdata);

    bool Process(E_processing_DIR processing_DIR = E_PD_forward);
};

/*! Block by block processing
 *
 * \todo check if T_FILEoutput works for files larger than 2GB/4GB.
 */
class T_FILEoutput : public T_OutputStackElement
{
  private:
    DSPu_FILEoutput *output_file;
    unsigned int raw_buffer_size;
    uint8_t *raw_buffer;
    //! true if writes to mono file
    bool write_mono;

  public:
    /*! \todo if sampling_rate == 0 then sampling rate should be determined based
     * on the parent objects on the processing stack.
     */
    T_FILEoutput(const char *filename, unsigned int sampling_rate, bool write_mono_in);
    ~T_FILEoutput(void);

    unsigned int ElementOverlapSegmentSize(void);
    unsigned int ElementOutputSegmentSize(unsigned int input_segment_size);
    unsigned int MaxNumberOfProtectedSamples(void);

    //! This block cannot be an output block so it ignores NoOfSamples and InputData
    /*! \todo Implement both complex and real output file.
     *   Which means both Process_cplx and Process_real will be needed.
     */
    unsigned int Process_cplx(E_processing_DIR processing_DIR = E_PD_forward,
                      unsigned int NoOfSamples = 0, DSP_complex_ptr InputData = 0);
};

//! \todo_later change the name
/*! This is quadrature heterodine (not just complex sinusoid generator)
 */
class T_DDS : public T_OutputStackElement
{
  private:
    static wxCriticalSection CS_DDS;
    static int LUT_lock_index;

    //! should be power of 2 for improved performance
    static unsigned int LUT_size;
    //! log2(LUT_size)
    static unsigned int LUT_k;
    //! one period (LUT_size) of complex sinusoid
    static DSP_complex *LUT;
    static unsigned int LUT2_size;
    static unsigned int LUT2_k;
    //! corrections sinus table (sin(dw)) (LUT2_size)
    static DSP_float *LUT2a;
    //! corrections sinus table (sin(dw/2)) (LUT2_size)
    static DSP_float *LUT2b;

    //! normalized frequency of DDS
    double fo;
    //! LUT step size
    int k;
    //! frequency correction for LUT2
    double dfo;
    //! LUT2 step size
    int ko;
    //! frequency error - depends on sizes of LUT tables
    double Fo_err;

    unsigned int LUT_index;
    unsigned int LUT2_index;
    /*! LUT_index_mask = (2 << L_k) - 1;
     */
    static unsigned int LUT_index_mask;
    static unsigned int LUT2_index_mask;

    unsigned int log2_ceil(unsigned int L);


    //! Locks LUT tables: if this is first lock or size change has been detected then allocate and generate new tables
    /*! \note For Fp = 8*10^6 => 0.03Hz resolution
     */
    void LockLUTtables(unsigned int LUT_size_in = 1U << 14, unsigned int LUT2_size_in = 1U << 14);
    //! Releases LUT tables lock: if all locks has been released then delete LUT tables
    void ReleaseLUTtables(void);

  public:
    /*! Initializes class
     *
     * -# LUT_size_in should be power of 2 for improved performance
     * -# L_factor - LUT interpolation factor, number of output samples
     *   per one LUT value. (L_factor - 1) samples are interpolated.
     * .
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
    T_DDS(void);

    /*! cleaning up
     */
    ~T_DDS(void);

    /*! - Fo - QDDS frequency [Hz]
     *  - Fs - sampling rate [Hz]
     *  .
     *
     *  - determines interpolation step
     *  - resets discrete time reference indexes
     *  .
     */
    void SetFreq(double Fo, long double Fs);

    unsigned int ElementOverlapSegmentSize(void);
    unsigned int ElementOutputSegmentSize(unsigned int input_segment_size);
    unsigned int MaxNumberOfProtectedSamples(void);

    /*! \todo Implement also the real input version
     */
    unsigned int Process_cplx(E_processing_DIR processing_DIR,
        unsigned int NoOfSamples, DSP_complex_ptr InputData);
};

//! Quadrature decimation by 2 with bandpass antialiasing filter
/*! \note Decimation filter in general has complex coefficients.
 */
class T_QD2_BPF : public T_OutputStackElement
{
  private:
    //! decimation filter length
    unsigned int N_FIR;
    //! coefficients of the decimation filter
    DSP_float *h_LPF;
    DSP_complex *h_BPF;
    double fo, fp, fs;

    //! Returns LPF prototype filter length based on fp and fs.
    /*! \note forces halfband filter design scheme
     *
     * \bug forced minPE = -90, Weight = 1
     */
    unsigned int GetFIRlength(double &fp, double &fs);
  public:
    /*! - B - filter accurate approximation bandwidth
     *  - Fo - center frequency
     *  - Fs - sampling frequency
     *  .
     *
     * Lowpass prototype passband upper normalized frequency and
     * stopband lower normalized frequency.
     *  - fp = (B/2)/Fs;
     *  - fs = 1/2 - (B/2)/Fs;
     *  .
     */
    T_QD2_BPF(double B, double Fo, long double Fs);
    /*!
     * @param b - normalized bandwidth
     * @param fo  - normalized center frequency
     */
    T_QD2_BPF(double b, double fo);
    ~T_QD2_BPF(void);

    unsigned int ElementOverlapSegmentSize(void);
    unsigned int ElementOutputSegmentSize(unsigned int input_segment_size);
    unsigned int MaxNumberOfProtectedSamples(void);
//! Returns size of inputs subsegment
    unsigned int ElementInputSubsegmentSize(void);

    void SetInputSamplingRate(long double input_sampling_rate);

    /*! \todo Also real input version might be useful
     */
    unsigned int Process_cplx(E_processing_DIR processing_DIR,
        unsigned int NoOfSamples, DSP_complex_ptr InputData);
};

//! Quadrature decimation by 2 with lowpass antialiasing filter
/*! \note Decimation filter has real coefficients.
 */
class T_QD2_LPF : public T_OutputStackElement
{
  private:
    //! decimation filter length
    unsigned int N_FIR;
    //! coefficients of the decimation filter
    DSP_float *h_LPF;
    double fp, fs;

    //! Returns LPF prototype filter length based on fp and fs.
    /*! \note forces halfband filter design scheme
     *
     * \bug forced minPE = -90, Weight = 1
     */
    unsigned int GetFIRlength(double &fp, double &fs);
  public:
    /*! - B - filter accurate approximation bandwidth
     *  - Fs - sampling frequency
     *  .
     *
     * Lowpass filter upper normalized frequency and
     * stopband lower normalized frequency.
     *  - fp = (B/2)/Fs;
     *  - fs = 1/2 - (B/2)/Fs;
     *  .
     */
    T_QD2_LPF(long double B, long double Fs);
    /*!
     * @param b - normalized bandwidth
     */
    T_QD2_LPF(long double b);
    ~T_QD2_LPF(void);

    void Init(long double b);

    unsigned int ElementOverlapSegmentSize(void);
    unsigned int ElementOutputSegmentSize(unsigned int input_segment_size);
    unsigned int MaxNumberOfProtectedSamples(void);
//! Returns size of inputs subsegment
    unsigned int ElementInputSubsegmentSize(void);

    void SetInputSamplingRate(long double input_sampling_rate);

    /*! \todo Also real input version might be useful
     */
    unsigned int Process_cplx(E_processing_DIR processing_DIR,
        unsigned int NoOfSamples, DSP_complex_ptr InputData);
};


// +++++++++++++++++++++++++++++++++++++ //
enum E_window
{
  EW_rectangular = 0, EW_blackman = 1
};
//! Manages evaluation of spectrogram of the signal stored in T_QD input buffer
class T_Spectrogram : public T_OutputStackElement
{
  private:
    E_window window_type;
    unsigned int window_size;
    unsigned int FFT_size;
    unsigned int APSDs_per_segment;
    unsigned int NoOfOverlappedPSDs;

    unsigned int PSD_per_segment;
    DSP_float Overlap;
    unsigned int NoOfPSDsPerAPSD;
    //! number of samples to skip to next PSD input segment
    unsigned int overlap_step;

    //! FFT object
    DSP_Fourier SpecgramFFT;
    //! Table storing time window coefficients
    DSP_float_ptr   time_window;
    //! table for FFT input
    DSP_complex_ptr FFT_input;
    //! table for FFT output => squared magnitude
    DSP_float_ptr   FFT_output;
    //! number of PSD already used in current APSD calculation
    unsigned int PSD_2_APSD_index;

    unsigned long long NoOfAPSDsProcessed;

    void *parent_task;

  public:
    /*! Input parameters:
     *  -# window_type - window type
     *  -# window_size - windows_size - determines frequency resolution,
     *  -# FFT_size - size of FFT used to calculate PSD - determines smoothing (zeropadding size),
     *  -# APSDs_per_segment - number of averaged PSD (APSD) spectrums per segment
     *    \note this is the number of outputed PSDs per segment.
     *  -# NoOfOverlappedPSDs - number of PSD per window_size
     *  .
     *
     * Parameters derived form input parameters:
     * -# PSD_per_segment - number of non-overlapped PSD spectrums per segment
     *   \note PSD_per_segment = InputSegmentSize / window_size,
     *    thus segment_size must be multiple of windows_size.
     * -# Overlap - PSD overlap in percents
     *   \note Overlap = 100.0 - 100.0/NoOfOverlappedPSDs
     * -# NoOfPSDsPerAPSD - number of PSDs per one outputed APSD
     *   \note NoOfPSDsPerAPSD = (NoOfOverlappedPSDs * PSD_per_segment) / APSDs_per_segment
     * .
     *
     */
    T_Spectrogram(E_window window_type_in,
                  unsigned int window_size_in,
                  unsigned int FFT_size_in,
                  unsigned int NoOfPSDsPerAPSD_in, //APSD_per_segment_in,
                  unsigned int NoOfOverlappedPSDs_in,
                  void *parent_task_in);
    ~T_Spectrogram(void);

    void PreinitializeElement(unsigned int InputSegmentSize,
                           unsigned int NoOfOutputSegments);

    unsigned int ElementOverlapSegmentSize(void);
    unsigned int ElementOutputSegmentSize(unsigned int input_segment_size);
    unsigned int MaxNumberOfProtectedSamples(void);
    //! Returns size of inputs subsegment
    unsigned int ElementInputSubsegmentSize(void);

    /*! returns distance between two consecutive APSDs
     *  measured in input signal samples
     */
    unsigned int NoOfInputSamplesPerAPSD(void);

    /*! \todo Implement version for real input signals
     */
    unsigned int Process_cplx(E_processing_DIR processing_DIR,
        unsigned int NoOfSamples, DSP_complex_ptr InputData);
};

class MyGLCanvas;
class T_SpectrogramDraw : public T_OutputStackElement
{
  private:
    MyGLCanvas *GLcanvas;
    unsigned int GLcanvasIndex;

    unsigned int no_of_output_segments;

    //unsigned int APSD_per_segment;
    unsigned int NoOfDisplayedPSDs;
    unsigned int FFT_size;

    //! discrete time of the first sample used to compute APSD with index 0
    long long APSD_time_offset;
    //! index of the current APSD
    /*! APSD_discrete_start_time = APSD_time_offset + APSD_index * NoOfInputSamplesPerAPSD
     *  APSD_start_time = APSD_discrete_start_time / input_sampling_rate
     */
    long long APSD_index;

    //! number of samples per slot (used for time scaling)
    unsigned int NoOfSamplesPerAPSD;
    long double sampling_rate;
    long double F_L, F_U;
    long double CenterFrequency;

    DSP_float min, max;
    bool Use_dB;

    //! decides how coordinates are displayed
    /*! 0 - coordinates related to file beginning and center frequency
     *  1st bit: 1 - real time
     *  2nd bit: 1 - real frequency
     */
    unsigned char coordinates_mode;
    //! current time used in mouse cursor position calculations
    T_time_base current_time;

    T_PlotsStack *SpectrogramStack;

  public:
    T_SpectrogramDraw(MyGLCanvas *GLcanvas_in,
                      //! canvas index in Parent index
                      unsigned int GLcanvasIndex_in,
                      unsigned int FFT_size_in,
                      unsigned int NoOfDisplayedPSDs_in, // Number of PSD displayed on canvas
                      unsigned int NoOfSamplesPerAPSD_in,
                      long double sampling_rate_in,
                      long double F_L_in, long double F_U_in,
                      long double CenterFrequency_in,
                      //! date and time of the recording start
                      T_time_base recording_time_in,
                      //! time offset in samples (first sample used to compute spectrogram)
                      long long time_offset = 0,
                      DSP_float min_in = -20.0,
                      DSP_float max_in = +20.0,
                      bool Use_dB_in = true
                      );
    ~T_SpectrogramDraw(void);

    //! Sets coordinates are displayed
    /*! 0 - coordinates related to file beginning and center frequency
     *  1 - real time; frequency related to center frequency
     *  2 - time related to file beginning; real frequency
     *  3 - real time and frequency
     */
    void SetCoordinatesMode(unsigned char coordinates_mode_in)
    {
      coordinates_mode = coordinates_mode_in;
    }

    wxCriticalSection CS_OnDraw;
    //! drawing procedure, should be called from the main thread
    void Draw(void);
    /*!
     * @param w new GLcanvas width
     * @param h new GLcanvas height
     */
    void OnSize(int w, int h);
    //! returns true if mouse down in current subplot
    bool OnMouseDown(int x, int y);
    void OnMouseUp(int x, int y);
    //! captured == true if mouse is in captured mode
    void OnMouseMove(int x, int y, bool captured);

    //void PreinitializeElement(unsigned int InputSegmentSize,
    //                       unsigned int NoOfOutputSegments);

    unsigned int ElementOverlapSegmentSize(void);
    unsigned int ElementOutputSegmentSize(unsigned int input_segment_size);
    unsigned int MaxNumberOfProtectedSamples(void);
    //! Returns size of inputs subsegment
    unsigned int ElementInputSubsegmentSize(void);

    unsigned int Process_real(E_processing_DIR processing_DIR,
        unsigned int NoOfSamples, DSP_float_ptr InputData);
};

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //

//! Half band Sampling Rate Conversion algorithm based on Farrow structure
class T_HB_SRC_Farrow : public T_OutputStackElement
{
  private:
    //! Input and output sampling rates
    long double Fs1, Fs2;
    //! ratio of output and input sampling intervals
    double Ts2_to_Ts1;
    //! fractional part of Ts2_to_Ts1
    double       Ts2_to_Ts1_frac;
    //! integer part of Ts2_to_Ts1
    unsigned int Ts2_to_Ts1_int;

    //! current net delay
    double eps_m;
    //! number of input samples to skip for the current output sample
    unsigned int d_n;

    //! Fractional sample delay filter length
    unsigned int N_FSD;
    //! order of the Farrow structure polynomials
    unsigned int FarrowOrder;
    //! Farrow structure coefficients
    /*! - p_i_n - table of FarrowOrder vectors
     *  - p_i_n[i] - vector of i-th Farrow structure polynomial coefficients,
     *   one for each FSD filter response sample (size of this vector == N_FSD)
     *  .
     */
    DSP_float_ptr *p_i_n;
  public:
    /*!
     * @param Fs1  input sampling rate
     * @param Fs2  output sampling rate
     *
     * AdB  in band maximum approximation error
     * Fmax accurate approximation band maximum frequency
     */
    T_HB_SRC_Farrow(long double Fs1_in, long double Fs2_in);
    ~T_HB_SRC_Farrow(void);

    unsigned int ElementOverlapSegmentSize(void);
    unsigned int ElementOutputSegmentSize(unsigned int input_segment_size);
    unsigned int MaxNumberOfProtectedSamples(void);

    void SetInputSamplingRate(long double input_sampling_rate);

    /*! \todo Also real input version might be useful
     */
    unsigned int Process_cplx(E_processing_DIR processing_DIR,
        unsigned int NoOfSamples, DSP_complex_ptr InputData);
};

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
//!  FIR filter
/*! \note LPF filter has real coefficients.
 * \note BPF filter has complex coefficients.
 */
class T_FIR : public T_OutputStackElement
{
  private:
    //! decimation filter length
    unsigned int N_FIR;
    //! true if filter is the LPF one
    bool Is_LPF;
    //! coefficients of the decimation filter
    DSP_float *h_LPF;
    DSP_complex *h_BPF;
    double fo, fp, fs;

    //! Returns LPF prototype filter length based on fp and fs.
    /*! \bug forces halfband filter design scheme
     *
     * \bug forced minPE = -90, Weight = 1
     */
    unsigned int GetFIRlength(double &fp, double &fs);
  public:
    /*!
     * @param Fp Lowpass prototype passband upper frequency
     * @param Fs Lowpass prototype stopband lower frequency
     * @param Fo bandpass filter center frequency, for lowpass filter Fo == 0
     * @param Fs_rate sampling rate
     */
    T_FIR(double Fp, double Fs, double Fo, long double Fs_rate);
    ~T_FIR(void);

    unsigned int ElementOverlapSegmentSize(void);
    unsigned int ElementOutputSegmentSize(unsigned int input_segment_size);
    unsigned int MaxNumberOfProtectedSamples(void);

    /*! \todo Also real input version might be useful
     */
    unsigned int Process_cplx(E_processing_DIR processing_DIR,
        unsigned int NoOfSamples, DSP_complex_ptr InputData);
};


#endif /*DSP_H_*/
