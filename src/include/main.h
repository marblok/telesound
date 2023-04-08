#ifndef MAIN_H_
#define MAIN_H_

#include "IDs.h"
#include "../TeleSound_wxGlade.h"

#include <wx/listctrl.h>
//#include <wx/splitter.h>
#include <wx/sashwin.h>
#include <wx/datectrl.h>
#include <wx/notebook.h>

#include "BitmapFont.h"
#include "OutlineFont.h"
#include "Misc.h"
#include <DSP_modules_misc.h>
#include "DSP.h"
#include "MyGLCanvas.h"

wxDECLARE_EVENT(wxEVT_DRAW_NOW, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_PROCESS_END, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_BRANCH_END, wxCommandEvent);
#ifdef __DEBUG__
const int ID_StatusBox_AppendText  = wxID_HIGHEST+4;
//DECLARE_EVENT_TYPE(wxEVT_STATUSBOX_UPDATE, -1);
wxDECLARE_EVENT(wxEVT_STATUSBOX_UPDATE, wxCommandEvent);
#endif // __DEBUG__

class T_DSPlib_processing;
class MyProcessingThread;
class MyProcessingBranch;
class MainFrame;
class MyMorseKey;

// Define a new application
class MainApp : public wxApp
{
public:
  static string HostAddress;

private:
  static MainFrame *frame;

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
    DSP::u::MORSEkey *MorseKey_temp;

  private:
    //! Pointer to the first processing specification on the list
    T_ProcessingSpec *FirstProcessingSpec;
    //! MDI window related to this task
    static MainFrame *task_parent_window;

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

    T_TaskElement(MainFrame *parent);
    ~T_TaskElement(void);
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
class MainFrame : public GUIFrame
{
  public:
    //char HostAddress[1024];

  public:
    wxPanel *CreatePage(wxNotebook *parent, E_PageIDs PageNo = E_PID_tasks);
    void SelectPage(E_PageIDs PageNo = E_PID_tasks);
    void SetStatusBoxMessage(std::string MessageText, bool isError=false);

    MyGLCanvas *GetGLcanvas(unsigned int CanvasInd);
    //! current task working with parent window
    T_TaskElement *parent_task;
    T_TaskElement *GetParentTask(void);


    TAudioMixer *AudioMixer;
      MyMorseKey *KeyingCtrl;


    MainFrame(wxWindow *parent, const wxWindowID id, const wxString& title,
            const wxPoint& pos, const wxSize& size, const long style);
    virtual ~MainFrame(void);


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
//extern MainFrame *frame;

class MyMorseKey : public wxTextCtrl
{
  private:
    //bool mouse_captured;
    bool is_down;
    //int counter;
    char text[1024];
    MainFrame *Parent;

  public:
    void OnKeyDown(wxKeyEvent& event);
    void OnKeyUp(wxKeyEvent& event);

    //void OnMouseLeftDown(wxMouseEvent& event);
    //void OnMouseLeftUp(wxMouseEvent& event);

    MyMorseKey(MainFrame* frame, wxWindow* parent, wxWindowID id, const wxString& label,
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
    MainFrame *Parent;

    unsigned int NoOfBranches;
    MyProcessingBranch **Branches;

  private:
    MyProcessingThread(MainFrame *Parent);
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
    static MyProcessingBranch *AddProcessingBranch(MainFrame *Parent_in,
        T_InputElement *ProcessingStack_in);

    //! Deletes all threads
    static void FreeThreads(void);
    //! Creates working threads
    /*! Created threads will be stored in ProcessingThreads table
     *  and used in MyProcessingThread::AddProcessingBranch
     *  for branches distribution and processing.
     */
    static void CreateAndRunThreads(MainFrame *Parent_in, int NoOfThreads = -1);
};




#endif /*MAIN_H_*/
