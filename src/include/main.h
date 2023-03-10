#ifndef MAIN_H_
#define MAIN_H_

#include <wx/listctrl.h>
//#include <wx/splitter.h>
#include <wx/sashwin.h>
#include <wx/datectrl.h>
#include <wx/notebook.h>

#include "BitmapFont.h"
#include "OutlineFont.h"
#include "Misc.h"
#include <DSPmodules_misc.h>

#include "MyGLCanvas.h"

const int ID_DrawNow    = wxID_HIGHEST+1;
const int ID_ProcessEnd = wxID_HIGHEST+2;
const int ID_BranchEnd  = wxID_HIGHEST+3;
DECLARE_EVENT_TYPE(wxEVT_DRAW_NOW, -1)
DECLARE_EVENT_TYPE(wxEVT_PROCESS_END, -1)
DECLARE_EVENT_TYPE(wxEVT_BRANCH_END, -1)
#ifdef __DEBUG__
const int ID_StatusBox_AppendText  = wxID_HIGHEST+4;
DECLARE_EVENT_TYPE(wxEVT_STATUSBOX_UPDATE, -1)
#endif // __DEBUG__

class T_DSPlib_processing;
class MyProcessingThread;
class MyProcessingBranch;
class MyFrame;
class MyMorseKey;

// Define a new application
class MyApp : public wxApp
{
public:
  static string HostAddress;

private:
  static MyFrame *frame;

  static bool LogFunction(const string &source, const string &message, bool IsError);

public:
  bool OnInit();
};


// processing subtask types
/* Any combination of subtasks can be used
enum E_SubtaskType
{
  E_ST_NONE = 0,       //!< E_ST_NONE
  //! draw PSD
  E_ST_PSD = 1,        //!< E_ST_PSD
  //! draw SPECTROGRAM
  E_ST_SPECTROGRAM = 2, //!< E_ST_SPECTROGRAM
  //! draw time signal
  E_ST_SIGNAL = 4     //!< E_ST_SIGNAL

//  E_ST_SPECTROGRAM_and_WRITE = (E_ST_SPECTROGRAM | E_ST_WRITE)
};
inline E_SubtaskType operator|(E_SubtaskType __a, E_SubtaskType __b)
  { return E_SubtaskType(static_cast<int>(__a) | static_cast<int>(__b)); }
 */

//! Class storing task list element data
class T_TaskElement : public wxEvtHandler
{
  friend class T_InterfaceState;

  private:
    //! First task list element
    static T_TaskElement *First;
    //! no of task list elements
    static int NoOfTasks;
    //! ID number for the new task
    static int NewTaskID;

    T_TaskElement *Next;

    unsigned int TaskID;
    char task_name[DSP_STR_MAX_LENGTH+1];

    //! true if task is already running
    bool task_is_running;
    //! true if task is in pause state
    bool task_is_paused;

  public:
    //! Pointer to the processing thread
    /*! \warning Thread will delete itself on exit
     *    so this variable can point to non-existing object.
     *
     *  \note This variable might not be valid while ProcessEndReported is still true.
     */
    MyProcessingBranch *ProcessingBranch;
    //!
    DSPu_MORSEkey *MorseKey_temp;

  private:
    //! Pointer to the first processing specification on the list
    T_ProcessingSpec *FirstProcessingSpec;
    //! MDI window related to this task
    static MyFrame *task_parent_window;

    //! Creates processing branch based on processing specification
    /*!
     * @param SpecList list of processing specifications
     * @return top most list object (must be T_FILEinput object)
     */
    T_InputElement *CreateProcessingBranch(T_ProcessingSpec *SpecList);

  public:
    //! Called by processing thread when branch processing has just ended
    void OnBranchEnd( wxCommandEvent &event );
#ifdef __DEBUG__
    //! Called by processing thread when status box needs to be updated
    void OnStatusBoxUpdate( wxCommandEvent &event );
#endif // __DEBUG__

    //!Task calls Post from DeleteBranch function
    wxSemaphore *BranchFinished_semaphore;
    //! just deletes branch object
    void DeleteBranch(MyProcessingBranch *BranchToClose);

  private:
    // processing task data
    //! stores info string for the input file
    char *input_file_info;
    //! input file name
    char filename[DSP_STR_MAX_LENGTH+1];
    //! input file directory
    char filedir[DSP_STR_MAX_LENGTH+1];
  public:
    const char *GetInputFileInfo(void);
    const char *GetFilename(void);
    const char *GetFiledir(void);
    const long double GetSamplingRate(void);
    void SetSamplingRate(long double Fs);
    const long double GetCenterFrequency(void);
    void SetCenterFrequency(long double Fo);

    T_ProcessingSpec *GetProcessingSpec(int index = 0);

  public:
    char *GetTaskName(void);
    void SetTaskName(char *);

  public:
    //! Run task processing
    /*! \todo implement all elements
     *  -# disable part of the GUI
     *  -# open MDI window
     *  -# create processing branch
     *  -# appends branch to the processing queue
     *  -# update GUI
     *  .
     */
    void RunTaskProcessing(void);
    //! Stops task processing
    /*! \todo implement all elements
     *  -# disable part of the GUI
     *  -# remove branch from the processing queue
     *  -# destroy processing branch (???)
     *  -# close MDI window
     *  -# update GUI
     *  .
     */
    void StopTaskProcessing(void);
    //! Pause task processing
    bool PauseTaskProcessing(void);

    T_TaskElement(MyFrame *parent);
    ~T_TaskElement(void);
};

enum E_PageIDs
{
  E_PID_tasks = 0,
  E_PID_CHANNEL = 1,
  E_PID_MORSE = 2,
  E_PID_status = 3,
  E_PID_default = 1000
};
class T_InterfaceState
{
  public:
    // +++++++++++++++++++++++++ //
    E_DrawModes draw_mode;
    int no_of_psd_slots;

    // +++++++++++++++++++++++++ //
    // temporary variables storing current user edited values
    bool run_as_server;
    string address;

    bool task_is_running;
    bool task_is_paused;

    int WPM;

    string selected_wav_filename;
    string wav_filename;

    bool mike_is_off;
    float local_signal_gain;

    float SNR_dB;

    long sampling_rate;

    bool channel_filter_ON;
    float channel_Fd;
    float channel_Fg;

    string ascii_text;
    bool morse_receiver_state;

    E_UpdateState userdata_state;

    // +++++++++++++++++++++++++ //
    void Reset(void);
    void Reset(T_TaskElement *selected_task);
    void TransferDataToTask(wxNotebookPage *page,
        T_TaskElement *selected_task, bool AskUser = true);

    T_InterfaceState(void);
    ~T_InterfaceState(void);
};
// Define a new frame
class MyFrame : public wxFrame
{
  public:
    //char HostAddress[1024];

  public:
    wxPanel *CreatePage(wxNotebook *parent, E_PageIDs PageNo = E_PID_tasks);
    void SelectPage(E_PageIDs PageNo = E_PID_tasks);

    //// Splitter window which allows managing MDIclient area versus notebook area
    //wxSplitterWindow *MainSplitter;
    wxSashWindow *NotebookSash;
    //! width of the notebook sash
    int NotebookSash_width;
    wxNotebook *notebookWindow;

    //! GLcanvas used to draw signals
    MyGLCanvas *GLcanvas;
    MyGLCanvas *GetGLcanvas(unsigned int CanvasInd);
    //! current task working with parent window
    T_TaskElement *parent_task;
    T_TaskElement *GetParentTask(void);


    // --------------------------------------- //
    // Panel #1 : Task management / file info
        // Tasks action controls
    wxToolBar *TasksToolBar;
    wxToolBarToolBase *MikeToolOFF;
    wxToolBarToolBase *MikeToolON;
    wxToolBarToolBase *LocalSignalToolOFF;
    wxToolBarToolBase *LocalSignalToolON;
    // input file parameters
    wxRadioButton *WorksAsServer;
    wxRadioButton *WorksAsClient;
    wxTextCtrl *ServerAddressEdit;
    wxComboBox *SamplingRateBox;
    //wxStaticText *ConnectionInfo;

    TAudioMixer *AudioMixer;
    wxComboBox *SourceLine_ComboBox;
    wxSlider *SourceLine_slider;
    wxComboBox *DestLine_ComboBox;
    wxSlider *DestLine_slider;
    wxSlider *MasterLine_slider;

    // --------------------------------------- //
    // Panel #2 : processing setting controls
    wxComboBox *DrawModeBox;
    wxTextCtrl *PSD_slots_text;
    wxSlider *PSD_slots_slider;

    wxTextCtrl *SNR_text;
    wxSlider *SNR_slider;

    wxTextCtrl *LPF_text;
    wxSlider *LPF_slider;
    wxTextCtrl *HPF_text;
    wxSlider *HPF_slider;

    // --------------------------------------- //
    // Panel #3 : MORSE code
    wxTextCtrl *WPM_text;
    wxSlider   *WPM_slider;
    wxTextCtrl *AsciiTextEntry;
    MyMorseKey *KeyingCtrl;
    wxButton   *SendAsciiText;

    wxCheckBox *MorseReceiverState;
    wxTextCtrl *AsciiTextReceiver;

    // --------------------------------------- //
    // Panel #4 : speech signal
    wxRadioButton  *UseLogatoms;
    wxRadioButton  *UseSentences;
    wxComboBox     *VoiceTypeBox;
    wxButton       *SelectVoiceFile;
    wxTextCtrl     *VoiceFileIndex;
    wxBitmapButton *OpenWAVEfile;
    wxBitmapButton *StopWAVEfile;

#ifdef __DEBUG__
    // --------------------------------------- //
    // Panel #5 : status controls
    wxTextCtrl *StatusBox;
#endif

    // --------------------------------------- //
    // --------------------------------------- //
    MyFrame(wxWindow *parent, const wxWindowID id, const wxString& title,
            const wxPoint& pos, const wxSize& size, const long style);
    virtual ~MyFrame(void);


    void OnSize(wxSizeEvent& event);
    void OnAbout(wxCommandEvent& event);

    void OnRunTask(wxCommandEvent& event);
    void OnPauseTask(wxCommandEvent& event);
    void OnStopTask(wxCommandEvent& event);
    bool task_is_stopping_now;

    //! true if window if about to close
    bool frame_is_closing;
    void OnQuit(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);

    void OnPickFileStartTime(wxCommandEvent& event);

    void OnNotebookSashDragged(wxSashEvent& event);

    void OnPageChanging(wxNotebookEvent& event);

    void OnDrawModeChange(wxCommandEvent& event);

    //! Called when radio buttons or checkboxes, etc. in settings pages changed
    void OnSettingsInterfaceChange(wxCommandEvent& event);
    void OnMixerVolumeChange(wxScrollEvent& event);
    //void OnSelectProcessingType(wxCommandEvent& event);
    void OnChannelFilterChange(wxScrollEvent& event);
    void OnChannelSNRChange(wxScrollEvent& event);
    void OnButtonPress(wxCommandEvent& event);
    void OnWPMchange(wxScrollEvent& event);
    void OnPSDparamsChange(wxScrollEvent& event);

    void OnProcessEnd(wxCommandEvent &event );

    // ++++++++++++++++++++++++++++++++++++++++++++++ //
    // + GUI procedures                             + //
    // ++++++++++++++++++++++++++++++++++++++++++++++ //
    T_InterfaceState interface_state;

    void InitToolBar(wxToolBar* toolBar);
    //! updates GUI based on parent_task state
    /*! task_is_paused, task_is_running
     */
    void UpdateGUI(void);
    //! Fills settings interface with data based on selected task
    void FillSettingsInterface(T_TaskElement *selected_task);

    // ++++++++++++++++++++++++++++++++++++++++++++++ //
    DECLARE_EVENT_TABLE()
};
//extern MyFrame *frame;

class MyMorseKey : public wxTextCtrl
{
  private:
    //bool mouse_captured;
    bool is_down;
    //int counter;
    char text[1024];
    MyFrame *Parent;

  public:
    void OnKeyDown(wxKeyEvent& event);
    void OnKeyUp(wxKeyEvent& event);

    //void OnMouseLeftDown(wxMouseEvent& event);
    //void OnMouseLeftUp(wxMouseEvent& event);

    MyMorseKey(MyFrame* frame, wxWindow* parent, wxWindowID id, const wxString& label,
        const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
        long style = 0, const wxValidator& validator = wxDefaultValidator)
     : wxTextCtrl(parent, id, label, pos, size, style, validator)
    {
      Parent = frame;
      //mouse_captured = false;
      is_down = false;
      //counter = 0;
    }

  DECLARE_EVENT_TABLE()
};

/*
class MyGLCanvas;

class MyChild: public wxMDIChildFrame
{
  friend class MyGLCanvas;
  private:
    //! Task to which this MDI window belongs
    T_TaskElement *MDI_parent_task;

  public:
    MyChild(wxMDIParentFrame *parent, T_TaskElement *MDI_task);
    ~MyChild();

    void InitToolBar(wxToolBar* toolBar);

    MyGLCanvas  *GLcanvas;
    //bool m_init;
    //void InitGL(void);
    MyGLCanvas *GetGLcanvas(unsigned int CanvasInd);
    T_TaskElement *GetParentTask(void);

  private:
    //unsigned int GUI_lock_counter;
    wxCriticalSection CS_OnGUI_LockUnlock;

  public:
    void LockDrawingData(unsigned int GLcanvas_index);
    //void Full_UnlockGUI(unsigned int GLcanvas_index = 0);
    void UnlockDrawingData(unsigned int GLcanvas_index);

    void OnActivate(wxActivateEvent& event);

    //void OnRefresh(wxCommandEvent& event);
    //void OnUpdateRefresh(wxUpdateUIEvent& event);
    //void OnPaint(wxPaintEvent& event);
    void OnChangeTitle(wxCommandEvent& event);
    void OnChangePosition(wxCommandEvent& event);
    void OnChangeSize(wxCommandEvent& event);
    void OnQuit(wxCommandEvent& event);
    void OnSize(wxSizeEvent& event);
//    void OnMove(wxMoveEvent& event);

    void OnSetFocus(wxFocusEvent& event);

    void OnClose(wxCloseEvent& event);

#if wxUSE_CLIPBOARD
    void OnPaste(wxCommandEvent& event);
    void OnUpdatePaste(wxUpdateUIEvent& event);
#endif // wxUSE_CLIPBOARD

    DECLARE_EVENT_TABLE()
};
*/


//! Represents processing thread container
/*! \todo implemented multiple threads management on multiple core CPUs
 *
 * \note AddProcessingBranch command takes as an input
 *   the stack of objects T_OutputStackElement
 *  - top level object must be input object like: T_FILEinput
 *  - ProcessingBranch takes ownership of the stack,
 *    which means it must delete the stack on its own
 *    using DeleteStack.
 *  - ProcessingBranch must delete stack if
 *    it is replaced processing stack using DeleteStack
 *  .
 */
class MyProcessingThread : public wxThread
{
  private:
    //! Number of CPUs
    static int NoOfCPUs;
    //! Number of active processing threads (size of ProcessingThreads table)
    static int NoOfProcessingThreads;
    //! Table storing pointers to processing threads
    static MyProcessingThread **ProcessingThreads;

    static void Initialize(void);

  public:
    wxThread::ExitCode Entry(void);
    //  not needed for wxTHREAD_JOINABLE
    static wxSemaphore **ThreadFinished_semaphore;

  private:
    int ThreadIndex;
    MyFrame *Parent;

    unsigned int NoOfBranches;
    MyProcessingBranch **Branches;

  private:
    MyProcessingThread(MyFrame *Parent);
  public:
    ~MyProcessingThread(void);
    //! checks if thread is on ProcessingThreads table and remove it from there
    void FreeThread(void);

  private:
    //! indicates whether changes to processing branches must be processed
    wxSemaphore *UpdateBranches_semaphore;
    MyProcessingBranch *new_branch;
    //! Updates branches
    /*! \note Returns false if updated thread processing must be finished
     */
    bool UpdateBranches(void);
  public:
    //! Adds processing branch to one of the active threads
    /*! \note Only one branch should have given MyChild window as a Parent
     */
    static MyProcessingBranch *AddProcessingBranch(MyFrame *Parent_in,
        T_InputElement *ProcessingStack_in);

    //! Deletes all threads
    static void FreeThreads(void);
    //! Creates working threads
    /*! Created threads will be stored in ProcessingThreads table
     *  and used in MyProcessingThread::AddProcessingBranch
     *  for branches distribution and processing.
     */
    static void CreateAndRunThreads(MyFrame *Parent_in, int NoOfThreads = -1);
};

// menu items ids
#define MAX_SLIDER_VALUE 100
// 5, 10, 15, ...
#define WPM_20_SLIDER_VALUE  3
#define WPM_MAX_SLIDER_VALUE 5
enum
{
    MDI_QUIT = wxID_EXIT,

    ID_connection_state = 101,
    ID_work_as_server = 102,
    ID_work_as_client = 103,
    ID_server_address = 104,
    ID_SELECT_SAMPLING_RATE = 105,
    ID_SELECT_MIXER_SOURCE_LINE = 106,
    ID_SELECT_MIXER_DEST_LINE = 107,
    ID_SourceLine_SLIDER = 108,
    ID_DestLine_SLIDER = 109,
    ID_MasterLine_SLIDER = 110,

    ID_SELECT_DRAW_MODE = 121,
    ID_PSD_SLOTS_SLIDER = 122,

    ID_SNR_SLIDER = 123,
    ID_LPF_SLIDER = 124,
    ID_HPF_SLIDER = 125,

    ID_WPM_SLIDER = 126,
    ID_ascii_text = 127,
    ID_morse_receiver_state = 128,
    ID_received_ascii_text = 129,
    ID_send_ascii_text = 130,

    ID_use_logatoms = 131,
    ID_use_sentences = 132,
    ID_voice_type = 133,
    ID_select_voice_file = 134,
    ID_voice_file_index = 135,
    ID_open_wav_file = 136,
    ID_stop_wav_file = 137,

    ID_RUN_TASK = 201,
    ID_PAUSE_TASK = 202,
    ID_STOP_TASK = 203,
    ID_MIKE_ON_OFF = 204,
    ID_LOCAL_SIGNAL_ON_OFF = 205,

    ID_NOTEBOOK_SASH = 301,
    ID_draw_time_signal = 302,
    ID_draw_histogram = 303,
    ID_draw_psd = 304,
    ID_draw_spectrogram = 305,
    ID_draw_none = 306,

    MDI_REFRESH,
    MDI_CHANGE_TITLE,
    MDI_CHANGE_POSITION,
    MDI_CHANGE_SIZE,
    MDI_CHILD_QUIT,

    MDI_ABOUT = wxID_ABOUT
};

#endif /*MAIN_H_*/
