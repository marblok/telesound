#ifndef _WXADDONS_H_
#define _WXADDONS_H_

#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <wx/datectrl.h>
#include <wx/dateevt.h>


  #define _STDCALL_SUPPORTED
  #define GLUT_DISABLE_ATEXIT_HACK
  //#define _M_IX86
  //#include <glut.h>
  //#define OPENGLUT_STATIC
  //#include <openglut.h>
#ifdef _USE_GLC_
  #include <glc.h>
#endif
  #include <wx/string.h>

  #include "BitmapFont.h"

// *************************************************************** //
//  void wxString2string(char *out, wxString in);
  void wxString2string(char *out, wxString in, int max_len);
  int ExtractParam(wxString &param, char *data, int size);

  class T_PlotsStack;

  enum CLR_map_type
  {
    CLR_jet,
    CLR_hot,
    CLR_gray
  };
  //! sets default GL color
  void SetColor(float val, const CLR_map_type map_type);

  void GetMixerError(int rs, wxString &tempString);

  class T_ChannelParams
  {
    public:
      float BPSK_mark, QPSK_mark;
      float BPSK_SER_log, QPSK_SER_log;

    T_ChannelParams(void);
  };

  /*! index < 0 is the same as abd(index) but no border for labels is left
   */
  void SubPlot(int Rows, int Cols, int index, int WindowW, int WindowH, bool Clear);
  void SubPlot(int Rows, int Cols, int index, int WindowW, int WindowH, bool Clear,
               GLfloat &x_scale, GLfloat &y_scale); //normalized scale to pixels ratio

  void MoveImage_Left(float move_factor);

  enum DS_type {DS_unsigned, DS_signed};

  void DrawScatterPlot(int SegmentSize, DSP::Float *XYdata, float skala, float size,
                       T_ChannelParams *ChannelInfo);
  void DrawEyeDiagram(int SegmentSize, DSP::Float *Xdata, DSP::Float *Ydata,
                      bool IsDataComplex, int SymbolPeriod,
                      float skala_x, float skala_y,
                      T_ChannelParams *ChannelInfo);
  void DrawChar(GLfloat x, GLfloat y, char znak, GLfloat scale_, GLfloat width,
                GLfloat win_x_scale, GLfloat win_y_scale);

 enum E_DS_mode
 {
   DS_none = 0,
   #ifdef GLUT_API_VERSION
     DS_GLUT_bitmap = 1,
     DS_GLUT_stroke = 2,
   #endif // GLUT_API_VERSION
   #ifdef _USE_GLC_
     DS_GLC_line = 3,
     DS_GLC_bitmap = 4,
   #endif
   DS_WGL_bitmap = 5,

   DS_method_mask = 7,

   DS_unit_pixel = 0,       // just DS_unit_normalized off
   DS_unit_normalized = 8,
   DS_unit_mask = (DS_unit_pixel | DS_unit_normalized)
 };

 enum E_PS_ticks_mode{
   E_TM_ignore = 0,
   E_TM_white = 1,
   E_TM_black = 2
 };

//#define TICKS_TEXTURE_MAXSIZE 64
//#define TICKS_TEXTURE_MAXSIZE 16
#define TICKS_TEXTURE_MAXSIZE 10
 /*!
  *  \bug create function which will test how the current OpenGL implementation
  *   draws lines: position and width.
  *    - line of the width 3 - check if it is actually 3 pixel wide
  *    - line of width 1 check if its position in pixels is correct
  *    - line of width 3 check if its position in pixels is correct
  *    .
  *   Make use of the test results in axis drawing procedures.
  *
  * \bug design better support for MDI windows of not running tasks
  */
 class T_PlotsStack
 {
     int NoOfColors;
     float *ColorsTable;

     //! current free spectrum slot (-1 if unavailable)
     int currentSlot;
     //! number of available slots
     int NoOfSlots;

     //! current allocated size for the slots
     int K;

     DSP::Float_ptr *Slots;
     int *SlotDataSize;
     int *ColorIndex;
     long double *SlotTime;

     void InitColors(int No);
     void FreeSlots(void);
     //! Resets Slots state and reallocates them if neccessary
     /*! -1 value means: leave previous value
      */

     // +++++++++++++++++++++++++++++++++++++++++ //
     //  drawing functions variables
     GLuint GL_list_index;
     GLfloat *VertexTable;
     GLfloat *ColorTable;

     GLfloat BackgroundRGB[3];

     GLint client_W, client_H;
     GLint subplot_X, subplot_Y;
     GLint subplot_W, subplot_H;
     GLint axis_dx, axis_dy;
     GLint axis_dw, axis_dh;
     GLfloat subplot_x_scale, subplot_y_scale;
     //! WGL bitmap font
     CBitmapFont  *m_bmf;

     // spectrogram related variables
     float dx, dy, dy_offset;
     int min_slot_y, max_slot_y;

     long double F_min, F_max;
     long double CenterFrequency;
     //! no of input samples per slot
     unsigned int NoOfSamplesPerAPSD;
     //! input signal sampling rate
     long double sampling_rate;


     E_PS_ticks_mode ticks_mode;
     static unsigned char x_subsubticks_pixels_W[];
     static unsigned char x_subticks_pixels_W[];
     static unsigned char x_ticks_pixels_W[];
     static unsigned char y_subsubticks_pixels_W[];
     static unsigned char y_subticks_pixels_W[];
     static unsigned char y_ticks_pixels_W[];

     static unsigned char x_subsubticks_pixels_B[];
     static unsigned char x_subticks_pixels_B[];
     static unsigned char x_ticks_pixels_B[];
     static unsigned char y_subsubticks_pixels_B[];
     static unsigned char y_subticks_pixels_B[];
     static unsigned char y_ticks_pixels_B[];

     static unsigned char x_subsubticks_pixels[TICKS_TEXTURE_MAXSIZE*TICKS_TEXTURE_MAXSIZE*4];
     static unsigned char x_subticks_pixels[TICKS_TEXTURE_MAXSIZE*TICKS_TEXTURE_MAXSIZE*4];
     static unsigned char x_ticks_pixels[TICKS_TEXTURE_MAXSIZE*TICKS_TEXTURE_MAXSIZE*4];
     static unsigned char y_subsubticks_pixels[TICKS_TEXTURE_MAXSIZE*TICKS_TEXTURE_MAXSIZE*4];
     static unsigned char y_subticks_pixels[TICKS_TEXTURE_MAXSIZE*TICKS_TEXTURE_MAXSIZE*4];
     static unsigned char y_ticks_pixels[TICKS_TEXTURE_MAXSIZE*TICKS_TEXTURE_MAXSIZE*4];

     static bool ticks_textures_initialized;
     static GLuint ticks_x_textures[3];
     static GLuint ticks_y_textures[3];


   public:
     T_PlotsStack(int Initial_NoOfSlots, int Initial_K = 0);
     ~T_PlotsStack(void);

     void Reset(int new_K=-1, int New_NoOfSlot = -1);

     //! if IsCyclic == true resets after last slot
     void NextSlot(bool IsCyclic = false);
     //! Returns maximum allocated slots' size
     int Get_MaxSlotDataSize(void);
     //! Returns number of allocated slots
     int Get_NoOfSlots(void);
     //! return current slot index
     int Get_SlotIndex(void);
     //! Returns slot pointer
     /*! If the last slot was already used then
      *   - PushIfLast = true =>   all slots will be cyclically rotated
      *   - PushIfLast = false =>  return NULL
      *   .
      */
     DSP::Float_ptr GetSlot(bool PushIfLast);
     //! if No == -1 give current slot
     DSP::Float_ptr GetSlot(int No=-1);
     //! if No == -1 for current slot
     void Set_SlotDataSize(int new_DataSize, int No=-1);
     //! if No == -1 for current slot
     int Get_SlotDataSize(int No=-1);
     void Set_ColorIndex(int index, int No=-1);
     void Get_Color(float &R, float &G, float &B, int No=-1);
     void Set_SlotTime(long double new_time, int No=-1);
     long double Get_SlotTime(int &No);
     //! returns time width of the whole specgram
     long double Get_TimeWidth(void);
     long double Get_FrequencyWidth(void);
     long double Get_Fo(void);

     // +++++++++++++++++++++++++++++++++++++++++ //
     //  drawing functions
     //! sets default SubPlot background color
     void SetBackgroundColor(float val, const CLR_map_type map_type);
     //! sets default SubPlot background color
     void SetBackgroundColor(float R, float G, float B);

     void SubPlot(int Rows, int Cols, int index, int WindowW, int WindowH, bool Clear);
     //! Gets coordinates related to given subplot
     /*! Input:
      *  - x: GLcanvas mouse x coordinates
      *  - y: GLcanvas mouse y coordinates
      *  .
      *  Output:
      *  - x: axis x coordinates (-1, +1)
      *  - y: axis y coordinates (-1, +1)
      *  .
      */
     void GetSubPlotCords(int Rows, int Cols, int index, float &x, float &y);

     //! number of main units per x axis (should be > 0 but maight be < 1)
     double x_units_per_axis;
     double x_pixels_per_unit;
     double x_unit;
     double y_units_per_axis;
     double y_pixels_per_unit;
     double y_unit;
     void InitAxis(double dt, double dF,
                   double t_unit = 0.0, double F_unit = 0.0,
                   E_PS_ticks_mode ticks_mode_in = E_TM_black);
     void PlotAxis(double to, double Fo, bool use_XOR = false);

     //! WGL bitmap font
     void SetWGLFont(CBitmapFont *m_bmf = NULL);
     void DrawString(GLfloat x, GLfloat y, char *text, GLfloat height, GLfloat width,
         E_DS_mode mode);

     //! This implementation uses Display lists
     void DrawSpecgram(float C_skala, const CLR_map_type map_type = CLR_hot);

     //! Data are expected in linear scale in range [0, 1/C_scale]
     void DrawSpecgram2_lin(float C_scale, const CLR_map_type map_type = CLR_hot);
     //! Data are expected in dB scale in range [dB_max - dB_range, dB_max]
     void DrawSpecgram2_dB(float dB_max, float dB_range, const CLR_map_type map_type = CLR_hot);
     //! This implementation usesVertex arrays and implements scaling
     /*! \warning Data are expected in [0, 1] range
      */
     void DrawSpecgram2_base(const CLR_map_type map_type = CLR_hot);
     //! \bug implement this texture based drawing
     void DrawSpecgram3_dB(float dB_max, float dB_range, const CLR_map_type map_type = CLR_hot);
     void DrawSignal(int SegmentSize, DSP::Float *Ydata, float skala,
                     DS_type type, float width);
     void DrawSignal_dB(int SegmentSize, DSP::Float *Ydata,
                     float dB_max, float dB_range, float width);
     void DrawSignal(float skala, DS_type type, float width);
     //! sets sampling rate
     /*!
      * @param F_min_in
      * @param F_max_in
      * @param CenterFrequency_in
      * @param NoOfSamplesPerAPSD_in
      * @param sampling_rate_in
      * @param real_FFT - true if only positive frequency PSD bins are stored
      */
     void InitialiseSpectrogram(long double F_min_in, long double F_max_in,
                                //! signals 0 frequency actually corresponds to given center frequency
                                /*! This means that input signal is complex envelope of a signal
                                 * with given center frequency.
                                 *
                                 * eg. if CenterFrequency_in == 1000, then
                                 *    F_min_in == 200, is interpreted as 200-1000 = -800
                                 *    in input signal (complex envelope).
                                 */
                                long double CenterFrequency_in,
                                unsigned int NoOfSamplesPerAPSD_in,
                                long double sampling_rate_in,
                                bool real_FFT = false);

     //! Converts mouse cords to spectrogram time and frequency cords
     void SpectrogramMouseCordsToRealCords(int x, int y, long double &t, long double &F);
     bool IsMouseInAxis(int x, int y);
     //! returns true if mouse down in current subplot
     bool OnMouseDown(int x, int y);
     bool OnMouseUp(int x, int y);
     /*!
      * @param x mouse position in pixels
      * @param y mouse position in pixels
      * @param X mouse x position translated to analog scale
      * @param Y mouse y position translated to analog scale
      */
     bool OnMouseMove(int x, int y, long double &X, long double &Y);
 };
 //extern T_PlotsStack SpectrumPlotsStack;
 //extern T_PlotsStack SignalPlotsStack;

 class T_time_base
  {
    protected:
      //! real file beginning date & time
      short year;
      short month;
      short day;

      short real_hours;
      short real_minutes;
      short real_seconds;

      //! offset related to file beginning
      long double hours;
      long double minutes;
      long double seconds;
      long double milliseconds;

    public:
      void GetTimeString(char *time_str, int mode = 0);

      //! sets real date of file beginning (does not change time)
      void SetReferenceDate(const wxDateTime &date_tmp);
      //! sets real time of file beginning (does not change date)
      void SetReferenceTime(const wxDateTime &date_tmp);
      //! fills current object with date and time data from date_tmp
      void SetReferenceDateTime(const wxDateTime &date_tmp);

      //! sets time offset in seconds from file beginning
      void SetTimeOffset(long long overall_seconds);
      void SetTimeOffset(long double overall_seconds_d);
      //! gets time offset in seconds from file beginning
      long long GetTimeOffset(void);

      //! adds offset time to reference time and resets offset to zero
      void AddOffsetToReferenceTime(void);

      //! change date of reference time with offset (ignore time)
      void SetOffsetDate(const wxDateTime &date_tmp);
      //! change time of reference time with offset (ignore date)
      void SetOffsetTime(const wxDateTime &date_tmp);

      //! compares current object with src
      /*!
       * @param src
       * @return true if objects are the same
       */
      bool Cmp(const T_time_base &src);
      //! fills current object with data from  src
      void Copy(const T_time_base &src);
      //! fills current object with data from  src, but skips time offset
      void CopyReferenceDateTime(const T_time_base &src);

      T_time_base(void);
  };


enum E_NV_mode{
  //! >= 0
  E_NV_non_negative = 0,
  //! > 0
  E_NV_only_positive = 1,
  //! all numbers
  E_NV_any = 2
};
/*! \bug Use SetNumMin and SetNumMax to change validators
 *   limits when for example sampling rate changes.
 */
class wxNumValidator: public wxValidator
{
  public:
    //! prec_in - number of digits after decimal point (== 0 - simple integer number)
    /*! \todo implement allowed value range setting (eg. 0-59 for seconds)
     *    - test during edition
     *    - test only when validating
     *    .
     *
     *  \note if prec_in < 0 then add preceding zeroes to the integer number
     *    so the string is -prec_in characters long
     *
     *  \todo Implements support for forcing limited precision (prec_in > 0)
     */
    wxNumValidator(int prec_in, E_NV_mode mode_in, // = E_NV_any,
                   long double *m_long_double_Value_in = NULL);
    //! check if value is between given min and max values
    /*! mode is automatically determined based on min value
     */
    wxNumValidator(int prec_in, long double min_in, long double max_in,
                   long double *m_long_double_Value_in = NULL);
    wxNumValidator(const wxNumValidator& val);

    virtual ~wxNumValidator(){}

    // Make a clone of this validator (or return NULL) - currently necessary
    // if you're passing a reference to a validator.
    // Another possibility is to always pass a pointer to a new validator
    // (so the calling code can use a copy constructor of the relevant class).
    virtual wxObject *Clone() const { return new wxNumValidator(*this); }
    bool Copy(const wxNumValidator& val);

    // Called when the value in the window must be validated.
    // This function can pop up an error message.
    virtual bool Validate(wxWindow *parent);

    // Called to transfer data to the window
    virtual bool TransferToWindow();

    // Called to transfer data from the window
    virtual bool TransferFromWindow();

    //! returns false if new value collides with validator mode or new_min > max_val
    /*! \note new value is always set (even if false if returned)
     */
    bool SetNumMin(long double new_min);
    //! returns false if new value collides with validator mode or min_val > new_max
    /*! \note new value is always set (even if false if returned)
     */
    bool SetNumMax(long double new_max);

    //! Filter keystrokes
    void OnChar(wxKeyEvent& event);
    //! Monitors text changes
    void OnTextChange( wxCommandEvent &event );


    DECLARE_EVENT_TABLE();

  protected:
    int            prec;
    E_NV_mode      mode;
    long double min_val, max_val;
    long double *m_long_double_Value;

    bool CheckValidator() const
    {
      bool ret;

        wxCHECK_MSG( m_validatorWindow, false,
                     _T("No window associated with validator") );
        // ???
        ret = m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl));
        ret |= m_validatorWindow->IsKindOf(CLASSINFO(wxComboBox));
        wxCHECK_MSG( ret, false,
                     _T("wxNumValidator is only for wxTextCtrl's or wxComboBox's") );

        return true;
    }

  private:
    char dot_char;

    // Cannot use
    //  DECLARE_NO_COPY_CLASS(wxTextValidator)
    // because copy constructor is explicitly declared above;
    // but no copy assignment operator is defined, so declare
    // it private to prevent the compiler from defining it:
    wxNumValidator& operator=(const wxNumValidator&);
};


bool ToLongLong(wxString &str, long long *val_out);
bool ToULongLong(wxString &str, unsigned long long *val_out);

#endif //_WXADDONS_H_
