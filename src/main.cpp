//#include <wx/mdi.h>
#include <wx/toolbar.h>
#include <wx/thread.h>
#include <wx/combobox.h>
#include <wx/glcanvas.h>
#include <wx/gbsizer.h>

//#include <DSP.h>
#include "Processing.h"
#include "main.h"
#include "MyGLCanvas.h"
#include "Branches.h"

#include "../bitmaps/new.xpm"

#include "../bitmaps/play_2_16.xpm"
#include "../bitmaps/pause_2_16.xpm"
#include "../bitmaps/stop_2_16.xpm"

#include "../bitmaps/mike_off.xpm"
#include "../bitmaps/mike_on.xpm"
#include "../bitmaps/local_off.xpm"
#include "../bitmaps/local_on.xpm"

#include "../bitmaps/open.xpm"
#include "../bitmaps/save.xpm"
#include "../bitmaps/copy.xpm"
#include "../bitmaps/cut.xpm"
#include "../bitmaps/paste.xpm"
#include "../bitmaps/print.xpm"
#include "../bitmaps/help.xpm"

//#include "../bitmaps/Ikony.xpm"

DEFINE_EVENT_TYPE(wxEVT_DRAW_NOW)
DEFINE_EVENT_TYPE(wxEVT_PROCESS_END)
DEFINE_EVENT_TYPE(wxEVT_BRANCH_END)
#ifdef __DEBUG__
DEFINE_EVENT_TYPE(wxEVT_STATUSBOX_UPDATE)
#endif // __DEBUG__

// (::wxPostEvent)
// wxEvtHandler::AddPendingEvent
//   wxEvent::Clone() must be implemented

// wxApp::Yield
// wxWindow::Update
// wxWindow::Refresh

// ::wxMutexGuiEnter
// ::wxMutexGuiLeave

IMPLEMENT_APP(MyApp)

// ---------------------------------------------------------------------------
// global variables
// ---------------------------------------------------------------------------

//MyFrame *frame = (MyFrame *) NULL;
//wxList my_children;

// ---------------------------------------------------------------------------
// event tables
// ---------------------------------------------------------------------------
BEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(MDI_ABOUT, MyFrame::OnAbout)

    EVT_MENU(ID_RUN_TASK, MyFrame::OnRunTask)
    EVT_MENU(ID_PAUSE_TASK, MyFrame::OnPauseTask)
    EVT_MENU(ID_STOP_TASK, MyFrame::OnStopTask)

    EVT_MENU(ID_MIKE_ON_OFF, MyFrame::OnSettingsInterfaceChange)
    EVT_MENU(ID_LOCAL_SIGNAL_ON_OFF, MyFrame::OnSettingsInterfaceChange)

    EVT_COMMAND(ID_BranchEnd, wxEVT_BRANCH_END, T_TaskElement::OnBranchEnd)
#ifdef __DEBUG__
    EVT_COMMAND(ID_StatusBox_AppendText, wxEVT_STATUSBOX_UPDATE, T_TaskElement::OnStatusBoxUpdate)
#endif // __DEBUG__

    EVT_MENU(MDI_QUIT, MyFrame::OnQuit)

    EVT_CLOSE(MyFrame::OnClose)
    EVT_SIZE(MyFrame::OnSize)

    EVT_COMMAND(ID_ProcessEnd, wxEVT_PROCESS_END, MyFrame::OnProcessEnd)

    EVT_SASH_DRAGGED(ID_NOTEBOOK_SASH, MyFrame::OnNotebookSashDragged)
    EVT_NOTEBOOK_PAGE_CHANGING(wxID_ANY, MyFrame::OnPageChanging)

    EVT_RADIOBUTTON(ID_work_as_server, MyFrame::OnSettingsInterfaceChange)
    EVT_RADIOBUTTON(ID_work_as_client, MyFrame::OnSettingsInterfaceChange)
    EVT_TEXT(ID_server_address, MyFrame::OnSettingsInterfaceChange)

    EVT_COMBOBOX(ID_SELECT_SAMPLING_RATE, MyFrame::OnSettingsInterfaceChange)
    EVT_COMBOBOX(ID_SELECT_MIXER_SOURCE_LINE, MyFrame::OnSettingsInterfaceChange)
    EVT_COMBOBOX(ID_SELECT_MIXER_DEST_LINE, MyFrame::OnSettingsInterfaceChange)
    EVT_COMMAND_SCROLL(ID_SourceLine_SLIDER, MyFrame::OnMixerVolumeChange)
    EVT_COMMAND_SCROLL(ID_DestLine_SLIDER, MyFrame::OnMixerVolumeChange)
    EVT_COMMAND_SCROLL(ID_MasterLine_SLIDER, MyFrame::OnMixerVolumeChange)

    //EVT_COMBOBOX(ID_SELECT_PROCESSING_TYPE, MyFrame::OnSelectProcessingType)
    EVT_COMBOBOX(ID_SELECT_DRAW_MODE, MyFrame::OnSettingsInterfaceChange)
    EVT_COMMAND_SCROLL(ID_PSD_SLOTS_SLIDER, MyFrame::OnPSDparamsChange)

    EVT_COMMAND_SCROLL(ID_SNR_SLIDER, MyFrame::OnChannelSNRChange)
//    EVT_COMMAND_SCROLL_CHANGED(ID_SNR_SLIDER, MyFrame::OnChannelSNRChange)
//    EVT_COMMAND_SCROLL_THUMBTRACK(ID_SNR_SLIDER, MyFrame::OnChannelSNRChange)
    EVT_COMMAND_SCROLL(ID_HPF_SLIDER, MyFrame::OnChannelFilterChange)
    EVT_COMMAND_SCROLL(ID_LPF_SLIDER, MyFrame::OnChannelFilterChange)

    EVT_BUTTON(ID_send_ascii_text, MyFrame::OnButtonPress)
    EVT_CHECKBOX(ID_morse_receiver_state, MyFrame::OnSettingsInterfaceChange)

    EVT_BUTTON(ID_select_voice_file, MyFrame::OnButtonPress)
    EVT_BUTTON(ID_open_wav_file,   MyFrame::OnButtonPress)
    EVT_BUTTON(ID_stop_wav_file,   MyFrame::OnButtonPress)
//    EVT_COMMAND_SCROLL_CHANGED(ID_WPM_SLIDER, MyFrame::OnWPMchange)
    EVT_COMMAND_SCROLL(ID_WPM_SLIDER, MyFrame::OnWPMchange)

    EVT_MENU(ID_draw_time_signal, MyFrame::OnDrawModeChange)
    EVT_MENU(ID_draw_histogram,   MyFrame::OnDrawModeChange)
    EVT_MENU(ID_draw_psd,         MyFrame::OnDrawModeChange)
    EVT_MENU(ID_draw_spectrogram, MyFrame::OnDrawModeChange)
    EVT_MENU(ID_draw_none,        MyFrame::OnDrawModeChange)

END_EVENT_TABLE()

// Note that MDI_NEW_WINDOW and MDI_ABOUT commands get passed
// to the parent window for processing, so no need to
// duplicate event handlers here.
/*
BEGIN_EVENT_TABLE(MyChild, wxMDIChildFrame)
    EVT_MENU(MDI_CHILD_QUIT, MyChild::OnQuit)
    //EVT_MENU(MDI_REFRESH, MyChild::OnRefresh)
    EVT_MENU(MDI_CHANGE_TITLE, MyChild::OnChangeTitle)
    EVT_MENU(MDI_CHANGE_POSITION, MyChild::OnChangePosition)
    EVT_MENU(MDI_CHANGE_SIZE, MyChild::OnChangeSize)

#if wxUSE_CLIPBOARD
    EVT_MENU(wxID_PASTE, MyChild::OnPaste)
    EVT_UPDATE_UI(wxID_PASTE, MyChild::OnUpdatePaste)
#endif // wxUSE_CLIPBOARD

    //EVT_PAINT(MyChild::OnPaint)
    EVT_SIZE(MyChild::OnSize)
//    EVT_MOVE(MyChild::OnMove)

    EVT_SET_FOCUS(MyChild::OnSetFocus)

    EVT_CLOSE(MyChild::OnClose)
END_EVENT_TABLE()
*/

BEGIN_EVENT_TABLE(MyGLCanvas, wxGLCanvas)
    EVT_SIZE(MyGLCanvas::OnSize)
    EVT_PAINT(MyGLCanvas::OnPaint)
    EVT_ERASE_BACKGROUND(MyGLCanvas::OnEraseBackground)
    //EVT_KEY_DOWN( MyGLCanvas::OnKeyDown )
    //EVT_KEY_UP( MyGLCanvas::OnKeyUp )
//! \Fixed Removed because it stole mouse capture from edit controls
//    EVT_ENTER_WINDOW( MyGLCanvas::OnEnterWindow )

    EVT_LEFT_DOWN( MyGLCanvas::OnMouseLeftDown )
    EVT_LEFT_UP( MyGLCanvas::OnMouseLeftUp )
    EVT_MOTION( MyGLCanvas::OnMouseMotion )

    EVT_COMMAND(ID_DrawNow, wxEVT_DRAW_NOW, MyGLCanvas::OnDrawNow)
END_EVENT_TABLE()


BEGIN_EVENT_TABLE(MyMorseKey, wxTextCtrl)
    EVT_KEY_DOWN( MyMorseKey::OnKeyDown )
    EVT_KEY_UP  ( MyMorseKey::OnKeyUp )

    //EVT_LEFT_DOWN( MyMorseKey::OnMouseLeftDown )
    //EVT_LEFT_UP  ( MyMorseKey::OnMouseLeftUp )
END_EVENT_TABLE()

/*
void MyMorseKey::OnMouseLeftDown(wxMouseEvent& event)
{
  mouse_captured = true;
  CaptureMouse();

  counter++;
  sprintf(text, "Down (%i)", counter);
  SetLabel(text);
  event.Skip();
}
void MyMorseKey::OnMouseLeftUp(wxMouseEvent& event)
{
  if (mouse_captured == true)
  {
    ReleaseMouse();
    mouse_captured = false;

    counter++;
    sprintf(text, "Up (%i)", counter);
    SetLabel(text);
  }
//  event.Skip();
}
*/
void MyMorseKey::OnKeyDown(wxKeyEvent& event)
{
  if (is_down == true)
    return;
  is_down = true;

//  counter++;
//  sprintf(text, "Key ON (%i)", counter);
  sprintf(text, "Key ON");
  SetLabel(text);

  if (Parent->parent_task != NULL)
    if (Parent->parent_task->MorseKey_temp != NULL)
      Parent->parent_task->MorseKey_temp->SetKeyState(true);
//  event.Skip();
}
void MyMorseKey::OnKeyUp(wxKeyEvent& event)
{
  is_down = false;
  //counter++;
  //sprintf(text, "Key Up (%i)", counter);
  sprintf(text, "Key Up");
  SetLabel(text);
//  event.Skip();
  if (Parent->parent_task != NULL)
    if (Parent->parent_task->MorseKey_temp != NULL)
      Parent->parent_task->MorseKey_temp->SetKeyState(false);
}

// ---------------------------------------------------------------------------
// MyApp
// ---------------------------------------------------------------------------
MyFrame *MyApp::frame = NULL;
string MyApp::HostAddress;
//wxCriticalSection CS_OnLOG;

bool MyApp::LogFunction(const string &source, const string &message, bool IsError)
{
  //CS_OnLOG.Enter();

  if (frame != NULL)
  {
    if (source.length() > 0)
      switch (source[0])
      {
        case 0x01:
          // remove \n from the end
          if (message.length() == 0)
          {
            //CS_OnLOG.Leave();
            return false;
          }

          *(frame->AsciiTextReceiver) << message;

          //CS_OnLOG.Leave();
          return true; // stop processing this message here
          break;

        case 0x02:
          if (message.length() == 0)
          {
            //CS_OnLOG.Leave();
            return false;
          }

          switch (message[0])
          {
            case 0x01:
              // locked
              frame->AsciiTextReceiver->SetBackgroundColour(wxColour(192, 255, 192));
              frame->AsciiTextReceiver->Refresh();
              break;

            case 0x02:
              // locking
              frame->AsciiTextReceiver->SetBackgroundColour(wxColour(255, 255, 192));
              frame->AsciiTextReceiver->Refresh();
              break;

            case 0x03:
              // unlocked
              frame->AsciiTextReceiver->SetBackgroundColour(wxColour(255, 192, 192));
              frame->AsciiTextReceiver->Refresh();
              break;

            default:
            case 0x00:
              // off
              //frame->AsciiTextReceiver->Clear(); ????
              //frame->AsciiTextReceiver->SetBackgroundColour(wxColour(255,255,255));
              break;
          }
          //wxGetApp().Yield(true);

          //CS_OnLOG.Leave();
          return true;
          break;

        default:
          break;
      }

    #ifdef __DEBUG__
      string MessageText;
      if (IsError == true)
        MessageText = DSPf_GetErrorMessage(source, message);
      else
        MessageText = DSPf_GetInfoMessage(source, message);

      //frame->StatusBox->AppendText(MessageText);
      // Send event to GUI thread
      wxCommandEvent event( wxEVT_STATUSBOX_UPDATE, ID_StatusBox_AppendText );
      if (IsError == true) {
        event.SetInt( 1 );
      }
      else {
        event.SetInt( 0 );
      }
      event.SetString( MessageText );
      frame->GetEventHandler()->AddPendingEvent(event);
    #endif
  }

  //CS_OnLOG.Leave();
  return false;
}

#ifdef __DEBUG__
void T_TaskElement::OnStatusBoxUpdate( wxCommandEvent &event ) {
  if (task_parent_window != NULL) {
    bool IsError = (event.GetInt() == 1);
    string MessageText = event.GetString().ToStdString();

    if (IsError == true)
    {
      task_parent_window->StatusBox->SetDefaultStyle(wxTextAttr(*wxRED));
    }
    else
    {
      task_parent_window->StatusBox->SetDefaultStyle(wxTextAttr(*wxBLACK));
    }
    task_parent_window->StatusBox->AppendText(MessageText);
  }
}
#endif // __DEBUG__

// Initialize this in OnInit, not statically
bool MyApp::OnInit()
{
  //char temp_str[1024];

    // Create the main frame window
#ifdef __DEBUG__
    DSPf_SetLogState(DSP_LS_file | DSP_LS_user_function);
    DSPf_SetLogFileName("log_file.log");
    DSPf_SetLogFunctionPtr(&(MyApp::LogFunction));
    DSPf_InfoMessage(DSP_lib_version_string());
#else
    DSPf_SetLogState(DSP_LS_user_function);
    //DSPf_SetLogFileName("log_file.log");
    DSPf_SetLogFunctionPtr(&(MyApp::LogFunction));
    DSPf_InfoMessage(DSP_lib_version_string());
#endif

#ifdef GLUT_API_VERSION
 	glutInit(&argc, argv);
#endif

    // http://tangentsoft.net/wskfaq/examples/ipaddr.html
    struct hostent *remoteHost;
    WSADATA wsaData;
    char HostName[1024];
    //struct in_addr addr;
    int iResult;

    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    gethostname(HostName, 1024);
    remoteHost = gethostbyname(HostName);
    //addr.s_addr = inet_addr("127.0.0.1");
    //remoteHost = gethostbyaddr((char *) &addr, 4, AF_INET);
    // h_addr_list[ind] != 0x00// h_addr_list[0] <- ip adress
    struct in_addr addr;
    memcpy(&addr, remoteHost->h_addr_list[0], sizeof(struct in_addr));
    HostAddress = inet_ntoa(addr);

    WSACleanup();

    frame = new MyFrame((wxFrame *)NULL, wxID_ANY, _T("TeleSound ver. 1.1 (Marek.Blok@eti.pg.edu.pl)"),
                        wxDefaultPosition, wxSize(800, 600),
                        wxDEFAULT_FRAME_STYLE); //  | wxHSCROLL | wxVSCROLL);
    //strcpy(frame->HostAddress, HostAddress);
//    frame->SetIcon(wxICON(MainIcon_xpm));
    frame->SetIcon(wxICON(TelesoundMainIcon));

#if 0
    // Experimental: change the window menu
    wxMenu* windowMenu = new wxMenu;
    windowMenu->Append(5000, _T("My menu item!"));
    frame->SetWindowMenu(windowMenu);
#endif

    // Give it an icon
    frame->SetIcon(wxICON(sample));

    //! \todo implement proper main frame menu bar
    /*
    // Make a menubar
    wxMenu *file_menu = new wxMenu;

    wxMenu *help_menu = new wxMenu;

    wxMenuBar *menu_bar = new wxMenuBar;

    menu_bar->Append(file_menu, _T("&File"));
    menu_bar->Append(help_menu, _T("&Help"));

    // Associate the menu bar with the frame
    frame->SetMenuBar(menu_bar);
    */

    //! \todo implement proper main frame status bar support
    frame->CreateStatusBar();

    frame->Show(true);
    SetTopWindow(frame);

    return true;
}

// ---------------------------------------------------------------------------
// T_ProcessingSpec
// ---------------------------------------------------------------------------
T_ProcessingSpec::T_ProcessingSpec(void)
{
  run_as_server = true;
  IP_address = "127.0.0.1";
  SamplingRate = 16000; //44100;

  no_of_psd_slots = 200;

  time_span = 2.0;
  WPM = 20;
  SNR_dB = 80;

  MikeIsOff = false;

  ChannelFilterON = false;
  ChannelFd = 0;
  ChannelFg = 0;

  morse_receiver_state = false;

  Next = NULL;
}

T_ProcessingSpec::~T_ProcessingSpec(void)
{
  Next = NULL;
}

void T_ProcessingSpec::DeleteList(void)
{
  if (Next != NULL)
    Next->DeleteList();

  delete this;
}

T_ProcessingSpec *T_ProcessingSpec::GetNext(void)
{ return Next; }


// ---------------------------------------------------------------------------
// T_TaskElement
// ---------------------------------------------------------------------------
// Class storing task list element data
T_TaskElement *T_TaskElement::First = NULL;
MyFrame *T_TaskElement::task_parent_window = NULL;
int T_TaskElement::NoOfTasks = 0;
int T_TaskElement::NewTaskID = 1; // start from 1

T_TaskElement::T_TaskElement(MyFrame *parent)
{
  T_TaskElement *current_task;

  MorseKey_temp = NULL;
  task_parent_window = parent;

  // ++++++++++++++++++++++++++++++++++++++++ //
  // initialize task data
  TaskID = NewTaskID;
  NewTaskID++;
  sprintf(task_name, "Task #%i", TaskID);
  strcpy(filename, "");
  strcpy(filedir, "");
  input_file_info = NULL;

  task_is_running = false;
  task_is_paused = false;
  //task_MDI_window = NULL;
  BranchFinished_semaphore = new wxSemaphore(0, 1);

  ProcessingBranch = NULL;
  FirstProcessingSpec = NULL;

  // ++++++++++++++++++++++++++++++++++++++++ //
  // add task to the list
  Next = NULL;
  if (First == NULL)
  {
    First = this; // this is the First task on the list
    NoOfTasks = 1;
  }
  else
  {
    // 1) find the last task
    current_task = First;
    while (current_task->Next != NULL)
    {
      current_task = current_task->Next;
    }

    // 2) add the new task at the end
    current_task->Next = this;

    // 3) update tasks counter
    NoOfTasks++;
  }
  // ++++++++++++++++++++++++++++++++++++++++ //
}

T_TaskElement::~T_TaskElement(void)
{
  T_TaskElement *current_task;

  // ++++++++++++++++++++++++++++++++++++++++ //
  // remove task from the list
  if (First != NULL)
  {
    current_task = First;

    if (current_task == this)
    {
      First = Next;
      if (NoOfTasks > 0)
        NoOfTasks--;
      else
        DSPf_ErrorMessage("T_TaskElement::~T_TaskElement", "NoOfTasks was already 0");
    }
    else
    {
      while (current_task != NULL)
      {
        if (current_task->Next == this)
        {
          current_task->Next = Next;
          if (NoOfTasks > 0)
            NoOfTasks--;
          else
            DSPf_ErrorMessage("T_TaskElement::~T_TaskElement", "NoOfTasks was already 0");
          break;
        }

        current_task = current_task->Next;
      }

      if (current_task == NULL)
      {
        DSPf_ErrorMessage("T_TaskElement::~T_TaskElement", "delete task was not on the list");
      }
    }
  }
  else
  {
    DSPf_ErrorMessage("T_TaskElement::~T_TaskElement", "tasks list already was empty");
  }

  // ++++++++++++++++++++++++++++++++++++++++ //
  // delete allocated data memory
  if (FirstProcessingSpec != NULL)
  {
    FirstProcessingSpec->DeleteList();
    FirstProcessingSpec = NULL;
  }

  if (input_file_info != NULL)
  {
    delete [] input_file_info;
    input_file_info = NULL;
  }
  if (BranchFinished_semaphore != NULL)
  {
    delete BranchFinished_semaphore;
    BranchFinished_semaphore = NULL;
  }
}

void T_TaskElement::OnBranchEnd( wxCommandEvent &event )
{
  if (task_parent_window != NULL)
  {
    //wxCommandEvent event( wxMenuEvent, ID_STOP_TASK );
    wxCommandEvent event2( wxEVT_COMMAND_TOOL_CLICKED, ID_STOP_TASK );
    //wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_STOP_TASK );
    task_parent_window->GetEventHandler()->AddPendingEvent( event2 );
  }
  /*
  if (ProcessingBranch != NULL)
  {
    DeleteBranch(ProcessingBranch);
    ProcessingBranch = NULL;
  }
  */
}

void T_TaskElement::DeleteBranch(MyProcessingBranch *BranchToClose)
{
  task_is_running = false;

  delete BranchToClose;

  BranchFinished_semaphore->Post();
}


char *T_TaskElement::GetTaskName(void)
{ return task_name; }

void T_TaskElement::SetTaskName(char *new_name)
{
  if (strlen(new_name) >= DSP_STR_MAX_LENGTH)
    strncpy(task_name, new_name, DSP_STR_MAX_LENGTH);
  else
    strcpy(task_name, new_name);
}

const char *T_TaskElement::GetInputFileInfo(void)
{
  return input_file_info;
}

const char *T_TaskElement::GetFilename(void)
{
  return filename;
}
const char *T_TaskElement::GetFiledir(void)
{
  return filedir;
}

/*! \note On parent MDI window close this function would be called twice.
 *    Second time task_MDI_window == NULL and ProcessingBranch == NULL.
 */
void T_TaskElement::StopTaskProcessing(void)
{
  MorseKey_temp  = NULL;
  // ++++++++++++++++++++++++++++++++++++++++++++++ //
  // 1. disable part of the GUI

  // ++++++++++++++++++++++++++++++++++++++++++++++ //
  // 2. remove branch from the processing queue

  // Tell thread branch to finish
  if (ProcessingBranch != NULL)
  {
    T_BranchCommand *temp;
    TCommandData *command_data;

    command_data = new TCommandData;
    command_data->BranchToClose = ProcessingBranch;
    temp = new T_BranchCommand(E_BC_closing, command_data);
    #ifdef __DEBUG__
      DSPf_InfoMessage("T_TaskElement::StopTaskProcessing", "PostCommandToBranch");
    #endif
    ProcessingBranch->PostCommandToBranch(temp);
    ProcessingBranch = NULL; // branch will be deleted in the processing thread
    // ++++++++++++++++++++++++
    // Wait for branch finish
    task_is_running = false;
    DSPf_ErrorMessage("T_TaskElement::StopTaskProcessing", "Waiting for thread branch to finish");
    //BranchFinished_semaphore->Wait();
    while (BranchFinished_semaphore->TryWait() == wxSEMA_BUSY)
    {
      wxGetApp().Yield(true);
    }
    DSPf_ErrorMessage("T_TaskElement::StopTaskProcessing", "!!! Thread branch finished");
  }
  task_is_running = false;
  task_is_paused = false;

  // ++++++++++++++++++++++++++++++++++++++++++++++ //
  // 3. destroy processing branch (branch is destroyed in the processing thread ???)

  // ++++++++++++++++++++++++++++++++++++++++++++++ //
  // 4. close MDI window

  // ++++++++++++++++++++++++++++++++++++++++++++++ //
  // 5. update GUI
  //if (task_parent_window != NULL)
  //  task_parent_window->UpdateGUI();

//! \todo implement task deletion
//  delete MDI_parent_task;
//  DSPf_ErrorMessage("MyChild::OnClose", "MDI_parent_task deleted");

}

bool T_TaskElement::PauseTaskProcessing(void)
{
  T_BranchCommand *temp;

  if (task_is_running == true)
  {
    // +++++++++++++++++++++++++++++++++++++++++++++ //
    // 1. disable part of the GUI

    // +++++++++++++++++++++++++++++++++++++++++++++ //
    // 2. pause processing
    if (ProcessingBranch != NULL)
    {
      if (task_is_paused == false)
      {
        task_is_paused = true;

        temp = new T_BranchCommand(E_BC_pause);
        #ifdef __DEBUG__
          DSPf_InfoMessage("T_TaskElement::PauseTaskProcessing", "PostCommandToBranch");
        #endif
        ProcessingBranch->PostCommandToBranch(temp);
      }
      else
      {
        task_is_paused = false;

        temp = new T_BranchCommand(E_BC_continue);
        #ifdef __DEBUG__
          DSPf_ErrorMessage("T_TaskElement::PauseTaskProcessing", "PostCommandToBranch");
        #endif
        ProcessingBranch->PostCommandToBranch(temp);
      }
    }

    // +++++++++++++++++++++++++++++++++++++++++++++ //
    // 3. update GUI
    //if (task_parent_window != NULL)
    //  task_parent_window->UpdateGUI();
  }
  return task_is_paused;
}

/*! \todo make use of SpecList
 * @param SpecList
 * @return
 *
 * \note stack elements must be allocated with new
 *   so the processing thread can take its ownership
 */
T_InputElement *T_TaskElement::CreateProcessingBranch(T_ProcessingSpec *SpecList)
{
  T_InputElement *input_object;

  input_object = new T_DSPlib_processing(SpecList);

  return input_object;
}

T_ProcessingSpec *T_TaskElement::GetProcessingSpec(int index)
{
  int ind;
  T_ProcessingSpec *current;

  ind = 0;
  current = FirstProcessingSpec;

  while (current != NULL)
  {
    if (ind == index)
      break;
    ind ++;
    current = current->GetNext();
  }
  return current;
}

void T_TaskElement::RunTaskProcessing(void)
{
  if (task_is_running == true)
    return;
  if (FirstProcessingSpec == NULL)
  {
    wxMessageBox(_("Processing specification not ready"),
        _("Run Task Problem"), wxOK | wxICON_ERROR);
    return;
  }

  task_is_running = true;
  task_is_paused = false;


  // +++++++++++++++++++++++++++++++++++++++++++++ //
  // 1. disable part of the GUI

  // +++++++++++++++++++++++++++++++++++++++++++++ //
  // 2. create processing branch
  T_InputElement *input_object;
  input_object = CreateProcessingBranch(FirstProcessingSpec);
  MorseKey_temp = ((T_DSPlib_processing *)input_object)->MorseKey;

  // +++++++++++++++++++++++++++++++++++++++++++++ //
  // 3. appends branch to the processing queue
  ProcessingBranch = MyProcessingThread::AddProcessingBranch(task_parent_window, input_object);

  // +++++++++++++++++++++++++++++++++++++++++++++ //
  // 4. update GUI
  if (task_parent_window != NULL)
    task_parent_window->UpdateGUI();

}

// ---------------------------------------------------------------------------
// T_InterfaceState
// ---------------------------------------------------------------------------
void T_InterfaceState::Reset(void)
{
  draw_mode = E_DM_none;
  no_of_psd_slots = 200;

  run_as_server = true;
  address = "127.0.0.1";

  task_is_running = false;
  task_is_paused = false;

  mike_is_off = true;
  local_signal_gain = 0.0;

  //sampling_rate = 8000;
  sampling_rate = 16000; //44100;
  WPM = 20;
  SNR_dB = 80;

  channel_filter_ON = false;
  channel_Fd = 0;
  channel_Fg = sampling_rate/2;

  ascii_text = "";
  morse_receiver_state = false;

  wav_filename[0] = 0x00;

  userdata_state = E_US_none;
}

void T_InterfaceState::Reset(T_TaskElement *selected_task)
{
  if (selected_task == NULL)
    Reset();
  else
  {
    //! \bug update other state data based on data in selected_task
    if (selected_task->task_parent_window == NULL)
      draw_mode = E_DM_none;
    else
      draw_mode = selected_task->task_parent_window->GetGLcanvas(0)->GetDrawMode();
  }
}

void T_InterfaceState::TransferDataToTask(
    wxNotebookPage *page,
    T_TaskElement *selected_task,
    bool AskUser)
{
  bool do_transfer;

  if (selected_task == NULL)
    return;

  do_transfer = false;
  // 1. Check if data has changed
  //do_transfer |= (sampling_rate != selected_task->GetSamplingRate());


  if (selected_task->FirstProcessingSpec != NULL)
  {
    //! \bug find better place for this initialization
    do_transfer |= (run_as_server != selected_task->FirstProcessingSpec->run_as_server);
    do_transfer |= (address.compare(selected_task->FirstProcessingSpec->IP_address) != 0);

    do_transfer |= (draw_mode != selected_task->task_parent_window->GetGLcanvas(0)->GetDrawMode());
    do_transfer |= (no_of_psd_slots != selected_task->FirstProcessingSpec->no_of_psd_slots);
    do_transfer |= (sampling_rate != selected_task->FirstProcessingSpec->SamplingRate);

    do_transfer |= (mike_is_off != selected_task->FirstProcessingSpec->MikeIsOff);
    do_transfer |= (local_signal_gain != selected_task->FirstProcessingSpec->local_signal_gain);

    do_transfer |= (channel_filter_ON != selected_task->FirstProcessingSpec->ChannelFilterON);
    do_transfer |= (channel_Fd != selected_task->FirstProcessingSpec->ChannelFd);
    do_transfer |= (channel_Fg != selected_task->FirstProcessingSpec->ChannelFg);

    do_transfer |= (WPM != selected_task->FirstProcessingSpec->WPM);
    do_transfer |= (SNR_dB != selected_task->FirstProcessingSpec->SNR_dB);

    do_transfer |= (morse_receiver_state != selected_task->FirstProcessingSpec->morse_receiver_state);

    do_transfer |= (wav_filename.compare(selected_task->FirstProcessingSpec->wav_filename) != 0);

    if (do_transfer == false)
    {
      switch (draw_mode)
      {
        case E_DM_signal:
          break;

        case E_DM_histogram:
          break;

        case E_DM_psd:
          break;

        case E_DM_spectrogram:
          //do_transfer |= (F_L_spec != selected_task->FirstProcessingSpec->F_L);
          //do_transfer |= (F_U_spec != selected_task->FirstProcessingSpec->F_U);
          break;

        case E_DM_none:
        default:
          break;
      }
    }
  }
  else
  { // there are no processing spec yet
    do_transfer = true;
  }

  // if no data changed return
  if (do_transfer == false)
    return;

  // 2. Ask user what to do
  if ((page != NULL) && (AskUser == true) &&
      (selected_task->FirstProcessingSpec != NULL)) // <== this is initialization not update
  {
    int answer;

    answer = wxMessageBox("Update task parameters?",
         _("Task parameters changed"),
         wxOK | wxCANCEL | wxICON_QUESTION, page);
    do_transfer = (answer == wxOK);
  }

  // 3. Transfer data
  if (do_transfer == true)
  {
    if (selected_task->FirstProcessingSpec == NULL)
    { // create processing spec list
      selected_task->FirstProcessingSpec =
        new T_ProcessingSpec();
    }
    else
    { // processing mode has changed
      // recreate processing spec list
      selected_task->FirstProcessingSpec->DeleteList();
      selected_task->FirstProcessingSpec =
        new T_ProcessingSpec();
    }
    selected_task->task_parent_window->GetGLcanvas(0)->SetDrawMode(draw_mode);

    /*! \bug implement support for other processing types
     *
     */
    selected_task->FirstProcessingSpec->run_as_server = run_as_server;
    selected_task->FirstProcessingSpec->IP_address = address;
    selected_task->FirstProcessingSpec->no_of_psd_slots = no_of_psd_slots;
    selected_task->FirstProcessingSpec->SamplingRate = sampling_rate;

    selected_task->FirstProcessingSpec->MikeIsOff = mike_is_off;
    selected_task->FirstProcessingSpec->local_signal_gain = local_signal_gain;

    selected_task->FirstProcessingSpec->ChannelFilterON = channel_filter_ON;
    selected_task->FirstProcessingSpec->ChannelFd = channel_Fd;
    selected_task->FirstProcessingSpec->ChannelFg = channel_Fg;

    selected_task->FirstProcessingSpec->WPM = WPM;
    selected_task->FirstProcessingSpec->SNR_dB = SNR_dB;

    selected_task->FirstProcessingSpec->morse_receiver_state = morse_receiver_state;

    selected_task->FirstProcessingSpec->wav_filename = wav_filename;

    switch (draw_mode)
    {
      case E_DM_signal:
        break;

      case E_DM_histogram:
        break;

      case E_DM_psd:
        break;

      case E_DM_spectrogram:
        //selected_task->FirstProcessingSpec->Set_F_L(F_L_spec);
        //selected_task->FirstProcessingSpec->Set_F_U(F_U_spec);
        break;

      case E_DM_none:
      default:
        break;
    }
  }
}

T_InterfaceState::T_InterfaceState(void)
{
  Reset();
}

T_InterfaceState::~T_InterfaceState(void)
{
  Reset();
}
// ---------------------------------------------------------------------------
// MyFrame
// ---------------------------------------------------------------------------

MyFrame::~MyFrame(void)
{
  if (AudioMixer != NULL)
  {
    AudioMixer->RestoreMixerSettings_WAVEIN();
    AudioMixer->RestoreMixerSettings_OUT();
    delete AudioMixer;
    AudioMixer = NULL;
  }
}
// Define my frame constructor
MyFrame::MyFrame(wxWindow *parent,
                 const wxWindowID id,
                 const wxString& title,
                 const wxPoint& pos,
                 const wxSize& size,
                 const long style)
       : wxFrame(parent, id, title, pos, size, style)
{
  frame_is_closing = false;
  task_is_stopping_now = false;
  AudioMixer = new TAudioMixer;
  AudioMixer->MemorizeMixerSettings_WAVEIN();
  AudioMixer->MemorizeMixerSettings_OUT();

  parent_task = NULL;
  int attribList[] = {
      WX_GL_RGBA,
      WX_GL_DOUBLEBUFFER,
      0
    };
  GLcanvas = new MyGLCanvas(0,
        this, wxID_ANY, attribList,
        wxDefaultPosition, wxDefaultSize,
        wxBORDER_NONE | wxFULL_REPAINT_ON_RESIZE | wxCLIP_CHILDREN);

  //MainSplitter = new wxSplitterWindow(this);
  //GetClientWindow();
  //SplitVertically(m_left, m_right, 100);
  //
  NotebookSash = new wxSashWindow(this, ID_NOTEBOOK_SASH);
  NotebookSash_width = 300;
  NotebookSash->SetSashVisible(wxSASH_LEFT, true);

    notebookWindow = new wxNotebook(NotebookSash, wxID_ANY,
                                wxDefaultPosition, wxDefaultSize,
                                wxNB_TOP,
                                _T("Parameters notebook"));
    wxWindow *page1, *page2, *page3;
    // Hide make page creation faster and warranties that no artifacts are seen during that operation
    notebookWindow->Hide();
    page1 = CreatePage(notebookWindow, E_PID_tasks);
    page2 = CreatePage(notebookWindow, E_PID_CHANNEL);
    page3 = CreatePage(notebookWindow, E_PID_MORSE);
  #ifdef __DEBUG__
    wxWindow *page5;
    page5 = CreatePage(notebookWindow, E_PID_status);
  #endif
    //page3 = CreatePage(notebookWindow, E_PID_general);
    // Without Show page adding should fail
    notebookWindow->Show();
    notebookWindow->AddPage(page1, _T("Konfiguracja"), true);
    notebookWindow->AddPage(page2, _T("Model kanału"), true);
    notebookWindow->AddPage(page3, _T("Telegraf"), true);
  #ifdef __DEBUG__
    notebookWindow->AddPage(page5, _T("Status programu"), true);
  #endif

    notebookWindow->ChangeSelection(0);

    wxCommandEvent tmp_cmd;
    tmp_cmd.SetId(ID_SELECT_DRAW_MODE);
    OnSettingsInterfaceChange(tmp_cmd);
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++ //
    Show(notebookWindow);



    //MainSplitter->SplitVertically(new wxScrolledWindow(MainSplitter), notebookWindow, 100);

    //! \todo implement proper main frame toolbar
    //CreateToolBar(wxNO_BORDER | wxTB_FLAT | wxTB_HORIZONTAL);
    //InitToolBar(GetToolBar());

    //MyProcessingThread::CreateAndRunThreads(this);
    MyProcessingThread::CreateAndRunThreads(this, 1); // tworzymy tylko jeden wątek, bo pozostałych i tak nie wykorzystywano

    FillSettingsInterface(NULL);
    UpdateGUI();

    // Accelerators
    wxAcceleratorEntry entries[8];
    entries[0].Set(wxACCEL_CTRL, (int) 'P', ID_PAUSE_TASK);
    entries[1].Set(wxACCEL_CTRL, (int) 'M', ID_MIKE_ON_OFF);
    entries[2].Set(wxACCEL_CTRL, (int) 'L', ID_LOCAL_SIGNAL_ON_OFF);
    entries[3].Set(wxACCEL_CTRL, (int) '1', ID_draw_time_signal);
    entries[4].Set(wxACCEL_CTRL, (int) '2', ID_draw_histogram);
    entries[5].Set(wxACCEL_CTRL, (int) '3', ID_draw_psd);
    entries[6].Set(wxACCEL_CTRL, (int) '4', ID_draw_spectrogram);
    entries[7].Set(wxACCEL_CTRL, (int) '0', ID_draw_none);
    wxAcceleratorTable accel(8, entries);
    SetAcceleratorTable(accel);
}

MyGLCanvas *MyFrame::GetGLcanvas(unsigned int CanvasInd)
{
  return GLcanvas;
}

T_TaskElement *MyFrame::GetParentTask(void)
{
  return parent_task;
}

wxPanel *MyFrame::CreatePage(wxNotebook *parent, E_PageIDs PageNo)
{
  wxPanel *panel = new wxPanel(parent);
  wxSize size;

  switch (PageNo)
  {
    case E_PID_tasks:
      {
        // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
        // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
        TasksToolBar = new wxToolBar(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize,
            wxTB_HORIZONTAL | wxNO_BORDER | wxTB_NODIVIDER | wxTB_FLAT);
        TasksToolBar->SetToolBitmapSize(wxSize(16,16));
        TasksToolBar->AddTool(ID_connection_state, _T("New"), wxBitmap( new_xpm ), _T("Status połączenia"));
        TasksToolBar->EnableTool(ID_connection_state, false);
        TasksToolBar->AddSeparator();
        TasksToolBar->AddTool(ID_RUN_TASK, _T("Run"), wxBitmap( play_xpm ), _T("Run processing"));
        TasksToolBar->AddTool(ID_PAUSE_TASK, _T("Pause"), wxBitmap( pause_xpm ), _T("Pause processing"));
        TasksToolBar->AddTool(ID_STOP_TASK, _T("Stop"), wxBitmap( stop_xpm ), _T("Stop processing"));
        TasksToolBar->AddSeparator();
        TasksToolBar->AddTool(ID_MIKE_ON_OFF, _T("Mike state"), wxBitmap( mike_on_xpm ), _T("Status mikrofonu (włączony)"));
        TasksToolBar->AddTool(ID_LOCAL_SIGNAL_ON_OFF, _T("Local signal state"), wxBitmap( local_on_xpm ), _T("Sygnał lokalny (włączony)"));
        TasksToolBar->Realize();
        MikeToolON = TasksToolBar->RemoveTool(ID_MIKE_ON_OFF);
        MikeToolOFF = TasksToolBar->AddTool(ID_MIKE_ON_OFF, _T("Mike state"), wxBitmap( mike_off_xpm ), _T("Status mikrofonu (wyłączony)"));
        LocalSignalToolON = TasksToolBar->RemoveTool(ID_LOCAL_SIGNAL_ON_OFF);
        LocalSignalToolOFF = TasksToolBar->AddTool(ID_LOCAL_SIGNAL_ON_OFF, _T("Local signal state"), wxBitmap( local_off_xpm ), _T("Sygnał lokalny (wyłączony)"));
        TasksToolBar->Realize();


        // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
        WorksAsServer = new wxRadioButton(panel,
            ID_work_as_server, _T("serwer"),
            wxDefaultPosition, wxDefaultSize, wxRB_GROUP); //wxRB_SINGLE);
        WorksAsClient = new wxRadioButton(panel,
            ID_work_as_client, _T("klient"),
            wxDefaultPosition, wxDefaultSize, 0);

        ServerAddressEdit = NULL;
        ServerAddressEdit = new wxTextCtrl(panel, ID_server_address, _T("xxx"),
            wxDefaultPosition, wxDefaultSize, wxTE_LEFT );

        // signal sampling rate
        wxStaticText *SamplingRateST;
        SamplingRateST = new wxStaticText(panel, wxID_ANY, _T("Sampling rate:"));
        wxString choises[7] = {
            _T("8000"), _T("11025"),_T("16000"), _T("22050"),
            _T("32000"), _T("44100"), _T("48000")};
        SamplingRateBox = new wxComboBox(panel, ID_SELECT_SAMPLING_RATE, _T("8000"),
            wxDefaultPosition, wxDefaultSize, 7, choises, wxCB_DROPDOWN | wxCB_READONLY);


        //ConnectionInfo = new wxStaticText(panel, wxID_ANY, "");
        wxString choises2[5] = {
            _T("Bez wykresów"),
            _T("Przebieg czasowy"),
            _T("Histogram"),
            _T("Periodogram"),
            _T("Spectrogram")};
        DrawModeBox = new wxComboBox(panel, ID_SELECT_DRAW_MODE,
            _T("Bez wykresów"), wxDefaultPosition, wxDefaultSize,
            5, choises2, wxCB_DROPDOWN | wxCB_READONLY);

        PSD_slots_text = new wxTextCtrl(panel, wxID_ANY, _T("200"),
            wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
        // 100, 150, 200, 250, 300, 350, 400
        PSD_slots_slider = new wxSlider(panel, ID_PSD_SLOTS_SLIDER, 2, 0, 6,
            wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);

        // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
        float val;
        int Active;
        DWORD mixer_state;

        wxStaticText *SourceLine_ST;
        SourceLine_ST = new wxStaticText(panel, wxID_ANY, _T("Nagrywanie:"));
        SourceLine_ComboBox = new wxComboBox(panel, ID_SELECT_MIXER_SOURCE_LINE, _T(""),
            wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN | wxCB_READONLY);
        Active = -1;
        SourceLine_ComboBox->Clear();
        for (int ind=0; ind<AudioMixer->GetNumberOfSourceLines(); ind++)
        {
          mixer_state = AudioMixer->GetSourceLineType(ind);
          if (mixer_state == MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE)
          {
            if (Active == -1)
              Active = ind;
          }
          SourceLine_ComboBox->Insert(AudioMixer->GetSourceLineName(ind), ind);
        }
        if (Active == -1)
          Active=AudioMixer->GetActiveSourceLine();
        AudioMixer->SetActiveSourceLine(Active);
        SourceLine_ComboBox->SetValue(AudioMixer->GetSourceLineName(Active));

        val = AudioMixer->GetSourceLineVolume(Active);
        SourceLine_slider = new wxSlider(panel, ID_SourceLine_SLIDER,
            (int)(val*MAX_SLIDER_VALUE), 0, MAX_SLIDER_VALUE,
            wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
        if (val < 0)
          SourceLine_slider->Enable(false);

        //DSPe_AM_MutedState mixer_state;
        wxStaticText *DestLine_ST;
        DestLine_ST = new wxStaticText(panel, wxID_ANY, _T("Odtwarzanie:"));
        DestLine_ComboBox = new wxComboBox(panel, ID_SELECT_MIXER_DEST_LINE, _T(""),
            wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN | wxCB_READONLY);
        DestLine_ComboBox->Clear();
        Active = -1; // GetDestLineType // GetMixerComponentType
        for (int ind=0; ind<AudioMixer->GetNumberOfDestLines(); ind++)
        {
          //mixer_state = AudioMixer->GetDestLineState(ind);
          //if (mixer_state == AM_MUTED_NO)
          mixer_state = AudioMixer->GetDestLineType(ind);
          if (mixer_state == MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT)
          {
            val = AudioMixer->GetDestLineVolume(ind);
            if ((Active == -1) && (val >= 0))
              Active = ind;
          }
          DestLine_ComboBox->Insert(AudioMixer->GetDestLineName(ind), ind);
        }
        if (Active == -1)
          Active = 0;
        DestLine_ComboBox->SetValue(AudioMixer->GetDestLineName(Active));
        for (int ind=0; ind<AudioMixer->GetNumberOfDestLines(); ind++)
        {
          if (ind != Active)
            AudioMixer->SetDestLineState(ind, AM_MUTED_YES);
          else
            AudioMixer->SetDestLineState(ind, AM_MUTED_NO);
        }
        val = AudioMixer->GetDestLineVolume(Active);
        DestLine_slider = new wxSlider(panel, ID_DestLine_SLIDER,
            (int)(val*MAX_SLIDER_VALUE), 0, MAX_SLIDER_VALUE,
            wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
        if (val < 0)
          DestLine_slider->Enable(false);

        wxStaticText *MasterLine_ST;
        MasterLine_ST = new wxStaticText(panel, wxID_ANY, _T("Głośność główna:"));
        val = AudioMixer->GetDestLineVolume(AM_MasterControl);
        MasterLine_slider = new wxSlider(panel, ID_MasterLine_SLIDER,
            (int)(val*MAX_SLIDER_VALUE), 0, MAX_SLIDER_VALUE,
            wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
        if (val < 0)
          MasterLine_slider->Enable(false);
        AudioMixer->SetDestLineState(AM_MasterControl, AM_MUTED_NO);


        // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
        // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //

        // +++++++++++++++++++++++++++++++++++++++++++++++++++++ //
        // tasks
        wxBoxSizer *sizerPanel;
        wxStaticBoxSizer *TaskActions_sizer;
        wxStaticBoxSizer *NetParams_sizer;
        wxStaticBoxSizer *AudioMixer_sizer;
        wxBoxSizer *subsizer;

        sizerPanel = new wxBoxSizer(wxVERTICAL);

        // Tasks action buttons
        TaskActions_sizer = new wxStaticBoxSizer(wxHORIZONTAL, panel, _T("Kontrola przetwarzania"));
        sizerPanel->Add(TaskActions_sizer, 0,wxEXPAND);
        TaskActions_sizer->Add(TasksToolBar, 0,wxEXPAND);

        // file parameters
        NetParams_sizer = new wxStaticBoxSizer(wxVERTICAL, panel, _T("Konfiguracja połączenia"));
        sizerPanel->Add(NetParams_sizer, 0,wxEXPAND);
          subsizer = new wxBoxSizer(wxHORIZONTAL);
          NetParams_sizer->Add(subsizer, 0,wxEXPAND);
            subsizer->Add(WorksAsServer, 1, 0 | wxBOTTOM, 5);
            subsizer->Add(WorksAsClient, 1, 0 | wxBOTTOM, 5);
          NetParams_sizer->Add(ServerAddressEdit, 0,wxEXPAND | wxBOTTOM, 5);
          subsizer = new wxBoxSizer(wxHORIZONTAL);
          NetParams_sizer->Add(subsizer, 0,wxEXPAND | wxBOTTOM, 5);
            subsizer->Add(SamplingRateST, 0, wxFIXED_MINSIZE | wxTOP | wxRIGHT, 3);
            subsizer->Add(SamplingRateBox, 1,wxEXPAND);
          //NetParams_sizer->Add(ConnectionInfo, 1,wxEXPAND, 0);

        // konfiguracja wykresów
        sizerPanel->AddSpacer(20);
        subsizer = new wxStaticBoxSizer(wxVERTICAL, panel, _T("Konfiguracja wykresów"));
          sizerPanel->Add(subsizer, 0, wxEXPAND);
          subsizer->Add(DrawModeBox, 0, wxEXPAND);
          subsizer->Add(PSD_slots_text, 0, wxTOP, 5);
          subsizer->Add(PSD_slots_slider, 0, wxEXPAND);

        // soundcard mixer
        sizerPanel->AddSpacer(20);
        AudioMixer_sizer = new wxStaticBoxSizer(wxVERTICAL, panel, _T("Ustawienia karty dźwiękowej"));
        sizerPanel->Add(AudioMixer_sizer, 0,wxEXPAND);
          subsizer = new wxBoxSizer(wxHORIZONTAL);
          AudioMixer_sizer->Add(subsizer, 0,wxEXPAND);
            subsizer->Add(SourceLine_ST, 0, 0 | wxBOTTOM, 5);
            subsizer->Add(SourceLine_ComboBox, 1, 0 | wxBOTTOM, 5);
          AudioMixer_sizer->Add(SourceLine_slider, 0,wxEXPAND);
          subsizer = new wxBoxSizer(wxHORIZONTAL);
          AudioMixer_sizer->Add(subsizer, 0,wxEXPAND);
            subsizer->Add(DestLine_ST, 0, 0 | wxBOTTOM, 5);
            subsizer->Add(DestLine_ComboBox, 1, 0 | wxBOTTOM, 5);
          AudioMixer_sizer->Add(DestLine_slider, 0,wxEXPAND);
          AudioMixer_sizer->Add(MasterLine_ST, 0,wxEXPAND);
          AudioMixer_sizer->Add(MasterLine_slider, 0,wxEXPAND);

        panel->SetSizer(sizerPanel);
      }
      break;

    case E_PID_CHANNEL:
      {
        SNR_text = new wxTextCtrl(panel, wxID_ANY, _T("80 dB"),
            wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
        SNR_slider = new wxSlider(panel, ID_SNR_SLIDER, MAX_SLIDER_VALUE, 0, MAX_SLIDER_VALUE,
            wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);

        HPF_text = new wxTextCtrl(panel, wxID_ANY, _T(""),
            wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
        HPF_slider = new wxSlider(panel, ID_HPF_SLIDER, 0, 0, MAX_SLIDER_VALUE,
            wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
        LPF_text = new wxTextCtrl(panel, wxID_ANY, _T(""),
            wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
        LPF_slider = new wxSlider(panel, ID_LPF_SLIDER, MAX_SLIDER_VALUE, 0, MAX_SLIDER_VALUE,
            wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);

        // Sygnał mowy
        UseLogatoms = new wxRadioButton(panel,
            ID_use_logatoms, _T("Logatomy"),
            wxDefaultPosition, wxDefaultSize, wxRB_GROUP); //wxRB_SINGLE);
        UseSentences = new wxRadioButton(panel,
            ID_use_sentences, _T("Zdania"),
            wxDefaultPosition, wxDefaultSize, 0); //wxRB_SINGLE);

        // voice type
        wxStaticText *VoiceTypeST;
        VoiceTypeST = new wxStaticText(panel, wxID_ANY, _T("Głos:"));
        wxString choises[5] = {
            _T("losowo"),
            _T("męski 1"),  _T("męski 2"),
            _T("żeński 1"), _T("żeński 2")};
        VoiceTypeBox = new wxComboBox(panel, ID_voice_type, _T("losowo"),
            wxDefaultPosition, wxDefaultSize, 5, choises, wxCB_DROPDOWN | wxCB_READONLY);

        SelectVoiceFile = new wxButton(panel, ID_select_voice_file, _T("Losuj"));
        VoiceFileIndex = new wxTextCtrl(panel, ID_select_voice_file, _T(""),
            wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
        OpenWAVEfile = new wxBitmapButton(panel, ID_open_wav_file, wxBitmap( play_xpm ));
        StopWAVEfile = new wxBitmapButton(panel, ID_stop_wav_file, wxBitmap( stop_xpm ));


        // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
        // sizers
        // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
        wxBoxSizer *sizerPanel;
        wxBoxSizer *subsizer, *subsubsizer;
        wxGridBagSizer *subsubsizer_grid;

        // main sizer
        sizerPanel = new wxBoxSizer(wxVERTICAL);
        subsizer = new wxStaticBoxSizer(wxVERTICAL, panel, _T("Model kanału"));
        sizerPanel->Add(subsizer, 0, wxEXPAND);
          subsizer->Add(SNR_text, 0, wxEXPAND);
          subsizer->Add(SNR_slider, 0, wxEXPAND);
          subsizer->AddSpacer(5);
          subsizer->Add(HPF_slider, 0, wxEXPAND);
          subsubsizer = new wxBoxSizer(wxHORIZONTAL);
            subsubsizer->Add(HPF_text, 0, wxEXPAND);
            subsubsizer->AddStretchSpacer(2);
            subsubsizer->Add(LPF_text, 0, wxEXPAND);
          subsizer->Add(subsubsizer, 0, wxEXPAND);
          subsizer->Add(LPF_slider, 0, wxEXPAND);

        // sygnał mowy
        sizerPanel->AddSpacer(20);
        subsizer = new wxStaticBoxSizer(wxVERTICAL, panel, _T("Sygnał mowy"));
        sizerPanel->Add(subsizer, 0, wxEXPAND);
          subsubsizer_grid = new wxGridBagSizer(7,3);
          subsubsizer_grid->SetCols(7);
          for (int ind = 0; ind < 7; ind++)
            subsubsizer_grid->AddGrowableCol(ind, 1);
          //subsizer->SetFlexibleDirection(wxHORIZONTAL);
          //subsizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_NONE);
          subsizer->Add(subsubsizer_grid, 0, wxEXPAND);
          //sizerPanel->AddStretchSpacer(1);

          subsubsizer_grid->Add(UseLogatoms, wxGBPosition(0, 0), wxGBSpan(1, 3), 0); //wxALIGN_LEFT | wxEXPAND);
          subsubsizer_grid->Add(UseSentences, wxGBPosition(0, 3), wxGBSpan(1, 4), wxALIGN_LEFT); //wxALIGN_LEFT | wxEXPAND);
          subsubsizer_grid->Add(VoiceTypeST, wxGBPosition(1, 0), wxGBSpan(1, 1), wxALIGN_RIGHT);
          subsubsizer_grid->Add(VoiceTypeBox, wxGBPosition(1, 1), wxGBSpan(1, 6), wxEXPAND);
          subsubsizer_grid->Add(SelectVoiceFile, wxGBPosition(2, 0), wxGBSpan(1, 2), wxEXPAND);
          subsubsizer_grid->Add(VoiceFileIndex, wxGBPosition(2, 2), wxGBSpan(1, 3), wxEXPAND);
          subsubsizer_grid->Add(OpenWAVEfile, wxGBPosition(2, 5), wxGBSpan(1, 1), wxEXPAND);
          subsubsizer_grid->Add(StopWAVEfile, wxGBPosition(2, 6), wxGBSpan(1, 1), wxEXPAND);

        panel->SetSizer(sizerPanel);
      }
      break;

    case E_PID_MORSE:
      {
        WPM_text = new wxTextCtrl(panel, wxID_ANY, _T("20 WPM"),
            wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
        WPM_slider = new wxSlider(panel, ID_WPM_SLIDER, WPM_20_SLIDER_VALUE, 0, WPM_MAX_SLIDER_VALUE,
            wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);

        wxStaticText *AsciiTextST;
        AsciiTextST = new wxStaticText(panel, wxID_ANY, _T("Nadawany tekst:"));
        AsciiTextEntry = new wxTextCtrl(panel, ID_ascii_text, _T(""),
            wxDefaultPosition, wxDefaultSize, wxTE_LEFT | wxTE_MULTILINE);

        MorseReceiverState = new wxCheckBox(panel, ID_morse_receiver_state, _T("włączony"));

        AsciiTextReceiver = new wxTextCtrl(panel, ID_received_ascii_text, _T(""),
            wxDefaultPosition, wxDefaultSize, wxTE_READONLY | wxTE_LEFT | wxTE_MULTILINE);

        wxStaticText *ManualST;
        ManualST = new wxStaticText(panel, wxID_ANY, _T("Ręczne nadawanie: "));
        KeyingCtrl = new MyMorseKey(this, panel, wxID_ANY, _T("Key OFF"));
        /*
        wxSize tmp_size = KeyingCtrl->GetClientSize(); //GetBestSize();
        //int w = GetCharWidth(); tmp_size.x = 7*w;
        KeyingCtrl->GetTextExtent(_T("Key OFF"), &(tmp_size.x), &(tmp_size.y));
        KeyingCtrl->SetClientSize(tmp_size);
         */
        SendAsciiText = new wxButton(panel, ID_send_ascii_text, _T("Wyślij"));

        // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
        // sizers
        // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
        wxBoxSizer *sizerPanel;
        wxBoxSizer *subsizer, *subsubsizer;
        // main sizer
        sizerPanel = new wxBoxSizer(wxVERTICAL);
        subsizer = new wxStaticBoxSizer(wxVERTICAL, panel, _T("Szybkość kluczowania"));
        sizerPanel->Add(subsizer, 0, wxEXPAND);
          subsizer->Add(WPM_text, 0, wxEXPAND);
          subsizer->Add(WPM_slider, 0, wxEXPAND);
          //subsizer->AddSpacer(5);
        subsizer = new wxStaticBoxSizer(wxVERTICAL, panel, _T("Nadajnik"));
        sizerPanel->Add(subsizer, 1, wxEXPAND);
          subsizer->Add(AsciiTextST, 0, 0);
          subsizer->Add(AsciiTextEntry, 1, wxEXPAND);
          subsubsizer = new wxBoxSizer(wxHORIZONTAL);
          subsizer->Add(subsubsizer, 0, wxEXPAND);
            subsubsizer->Add(ManualST, 0, wxTOP | wxEXPAND, 5);
            subsubsizer->Add(KeyingCtrl, 0, wxEXPAND);
            subsubsizer->AddStretchSpacer(2);
            subsubsizer->Add(SendAsciiText, 0, wxEXPAND);

        subsizer = new wxStaticBoxSizer(wxVERTICAL, panel, _T("Odbiornik"));
        sizerPanel->Add(subsizer, 1, wxEXPAND);
          subsizer->Add(MorseReceiverState, 0, wxALIGN_LEFT);
          subsizer->Add(AsciiTextReceiver, 1, wxEXPAND);

        panel->SetSizer(sizerPanel);
      }
      break;

#ifdef __DEBUG__
    case E_PID_status:
      {
        StatusBox = new wxTextCtrl(panel, wxID_ANY, _T(""),
            wxDefaultPosition, wxDefaultSize,
            wxTE_MULTILINE | wxTE_READONLY | wxTE_LEFT | wxTE_RICH2 );

        wxBoxSizer *sizerPanel;
        wxBoxSizer *subsizer;
        // main sizer
        sizerPanel = new wxBoxSizer(wxVERTICAL);
        subsizer = new wxBoxSizer(wxHORIZONTAL);
        sizerPanel->Add(subsizer, 1, wxEXPAND);
          subsizer->Add(StatusBox, 1, wxEXPAND);

        panel->SetSizer(sizerPanel);
      }
      break;
#endif

    case E_PID_default:
    default:
      {
        wxString computers[] = { wxT("Amiga"), wxT("Commodore 64"), wxT("PET"),
            wxT("Another") };
        wxRadioBox *radiobox2 = new wxRadioBox(panel, wxID_ANY,
            wxT("Choose your favourite"), wxDefaultPosition, wxDefaultSize,
            4, computers, 0, wxRA_SPECIFY_COLS);

        wxBoxSizer *sizerPanel = new wxBoxSizer(wxVERTICAL);
        sizerPanel->Add(radiobox2, 1, wxEXPAND);
        panel->SetSizer(sizerPanel);
      }
      break;
  }

  // make use of validators to fill proper data
  panel->TransferDataToWindow();

  return panel;
}

/*! \todo Move toolbar update into another function
 *   - implement more toolbar states (not just all on / all off)
 *   .
 */
void MyFrame::UpdateGUI(void)
{
  unsigned int ind;
  wxNotebookPage* tmp_page;
/*
  if (parent_task == NULL)
  {
    notebookWindow->ChangeSelection(0);

    // update toolbar
    TasksToolBar->EnableTool(ID_RUN_TASK, true);
    TasksToolBar->EnableTool(ID_PAUSE_TASK, false);
    TasksToolBar->EnableTool(ID_STOP_TASK, false);

    // Disable settings pages
    / *
    for (ind = 1; ind < notebookWindow->GetPageCount(); ind++)
    {
      tmp_page = notebookWindow->GetPage(ind);
      tmp_page->Enable(false);
    }
    * /
    return;
  }
  else
 */
  {
    // enable task related UI
    ////FileParams_sizer->Hide(FileParams_sizer, false);
    //FileParams_sizer->Show(FileParams_sizer, true, true);
    //FileParams_sizer->Layout();

    // update toolbar
    if (interface_state.task_is_running == true)
    {
      TasksToolBar->EnableTool(ID_RUN_TASK, false);
      TasksToolBar->EnableTool(ID_PAUSE_TASK, true);
      if (interface_state.task_is_paused == true)
        TasksToolBar->EnableTool(ID_STOP_TASK, false);
      else
        TasksToolBar->EnableTool(ID_STOP_TASK, true);

      SamplingRateBox->Enable(false);
      PSD_slots_text->Enable(false);
      PSD_slots_slider->Enable(false);
    }
    else
    {
      TasksToolBar->EnableTool(ID_RUN_TASK, true);
      TasksToolBar->EnableTool(ID_PAUSE_TASK, false);
      TasksToolBar->EnableTool(ID_STOP_TASK, false);

      SamplingRateBox->Enable(true);
      PSD_slots_text->Enable(true);
      PSD_slots_slider->Enable(true);
    }

    // Enable settings pages
    for (ind = 1; ind < notebookWindow->GetPageCount(); ind++)
    {
      tmp_page = notebookWindow->GetPage(ind);
      tmp_page->Enable(true);
    }
  }

  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
  // Enable / disable controls based on interface_state
  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++  //
  //  General settings page
  if (interface_state.run_as_server == true)
  {
    WorksAsServer->SetValue(true);
    WorksAsClient->SetValue(false);
    //ServerAddressEdit->ChangeValue(interface_state.address);
    ServerAddressEdit->ChangeValue(MyApp::HostAddress);
    ServerAddressEdit->Enable(false);
  }
  else
  {
    WorksAsServer->SetValue(false);
    WorksAsClient->SetValue(true);
    ServerAddressEdit->ChangeValue(interface_state.address);
    ServerAddressEdit->Enable(true);
  }

  if (interface_state.morse_receiver_state == true)
  {
    AsciiTextReceiver->Enable(true);
  }
  else
  {
    AsciiTextReceiver->Enable(false);
  }
}

void MyFrame::FillSettingsInterface(T_TaskElement *selected_task)
{
  if (selected_task == NULL)
  { // clear settings
    WorksAsServer->SetValue(true);
    WorksAsClient->SetValue(false);
    ServerAddressEdit->ChangeValue("127.0.0.1");
    //ConnectionInfo->SetLabel("");
  }
  else
  { // fill settings
    if (interface_state.run_as_server == true)
    {
      WorksAsServer->SetValue(true);
      WorksAsClient->SetValue(false);
      ServerAddressEdit->ChangeValue(MyApp::HostAddress);
      ServerAddressEdit->Enable(false);
    }
    else
    {
      WorksAsServer->SetValue(false);
      WorksAsClient->SetValue(true);
      ServerAddressEdit->ChangeValue(interface_state.address);
      ServerAddressEdit->Enable(true);
    }
    //ConnectionInfo->SetLabel("");


    DrawModeBox->SetSelection(wxNOT_FOUND);
    if (selected_task->GetProcessingSpec() != NULL)
    {
      switch (GetGLcanvas(0)->GetDrawMode())
      {
        case E_DM_signal:
          DrawModeBox->SetSelection(1);
          break;
        case E_DM_histogram:
          DrawModeBox->SetSelection(2);
          break;
        case E_DM_psd:
          DrawModeBox->SetSelection(3);
          break;
        case E_DM_spectrogram:
          DrawModeBox->SetSelection(4);
          break;
        case E_DM_none:
        default:
          DrawModeBox->SetSelection(0);
          break;
      }
    }
  }

  //SamplingRateEdit->SetValue(wxString::Format("%lld", (long long)(selected_task->GetSamplingRate())));
  SamplingRateBox->SetValue(wxString::Format("%ld", interface_state.sampling_rate));

  wxScrollEvent event;
  event.SetId(ID_SELECT_SAMPLING_RATE);
  OnChannelFilterChange(event);
  //LPF_slider->SetValue((int)(interface_state.channel_Fg/100));
  //HPF_slider->SetValue((int)(interface_state.channel_Fd/100));

  wxScrollEvent temp_event;
  temp_event.SetId(ID_SELECT_SAMPLING_RATE);
  OnChannelFilterChange(temp_event);
}

void MyFrame::OnClose(wxCloseEvent& event)
{
    DSPf_InfoMessage("MyFrame::OnClose", "start");
    /*
    if ( event.CanVeto() && (gs_nFrames > 0) )
    {
        wxString msg;
        msg.Printf(_T("%d windows still open, close anyhow?"), gs_nFrames);
        if ( wxMessageBox(msg, _T("Please confirm"),
                          wxICON_QUESTION | wxYES_NO) != wxYES )
        {
            event.Veto();

            return;
        }
    }
    */
    frame_is_closing = true;

    DSPf_InfoMessage("MyFrame::OnClose", "Calling FreeThreads");
    MyProcessingThread::FreeThreads();
    DSPf_InfoMessage("MyFrame::OnClose", "Finished FreeThreads");

    event.Skip();
}

void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    Close();
}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event) )
{
  string text, text2;

  text = DSP_lib_version_string();
  text2 = "TeleSound 2020\n\n";
  text2 += "Author: Marek Blok (c) 2020\n\n";
  text2 += text;

  wxMessageBox(text2,
               _T("About Zoom Analyzer"));
}

void MyFrame::OnRunTask(wxCommandEvent& event)
{
  if (parent_task == NULL)
  {
    if (task_is_stopping_now == true)
      return;

    if (interface_state.task_is_running == true)
    {
      DSPf_ErrorMessage("MyFrame::OnRunTask", "Task is still running");
      return;
    }
    parent_task = new T_TaskElement(this);
    interface_state.TransferDataToTask(NULL, parent_task, false); // do not ask user
    interface_state.task_is_running = true;
    interface_state.task_is_paused = false;
    parent_task->RunTaskProcessing();
  }
}

void MyFrame::OnPauseTask(wxCommandEvent& event)
{
  /*
  TCommandData *command_data;
  T_BranchCommand *temp;
  */

  if (parent_task != NULL)
  {
    if (task_is_stopping_now == true)
      return;

    interface_state.task_is_paused = parent_task->PauseTaskProcessing();
    UpdateGUI();

    if (interface_state.task_is_paused == true)
    {
      //parent_task->ProcessingBranch->ComputeHighResolutionSpectorgram();
      /*
      interface_state.userdata_state = E_US_high_res_psd;
      command_data = new TCommandData;
      command_data->UserData = (void *)(&interface_state);
      temp = new T_BranchCommand(E_BC_userdata, command_data);
      parent_task->ProcessingBranch->PostCommandToBranch(temp);
      \bug GLcanvas needs to be invalidated
      */
    }
  }
}

void MyFrame::OnStopTask(wxCommandEvent& event)
{
  if (parent_task != NULL)
  {
    if (task_is_stopping_now == true)
      return;
    task_is_stopping_now = true;
    //Enable(false);

    parent_task->StopTaskProcessing();
    //! \bug should wait until task finishes
    parent_task = NULL;

    interface_state.task_is_running = false;
    interface_state.task_is_paused = false;

    task_is_stopping_now = false;
    //Enable(true);
    UpdateGUI();

    GetGLcanvas(0)->Refresh(); // invalidate window
    GetGLcanvas(0)->Update();  // refresh canvas immediately
  }
}

void MyFrame::OnNotebookSashDragged(wxSashEvent& event)
{
  if (event.GetDragStatus() == wxSASH_STATUS_OK)
  {
    int w, h;
    wxRect pos2;

    //pos = NotebookSash->GetSize();
    pos2 = event.GetDragRect();
    pos2.y = 0;

    GetClientSize(&w, &h);
    GLcanvas->SetSize(0, 0, pos2.x, h);

    NotebookSash_width = pos2.width;
    NotebookSash->SetSize(pos2);

    //! wxListView is not refreshed correctly when the sash window is narrowed
    wxGetApp().Yield(true);
    notebookWindow->Refresh();
    notebookWindow->Update();
    //notebookWindow->UpdateWindowUI(wxUPDATE_UI_RECURSE);
  }
}

void MyFrame::OnPageChanging(wxNotebookEvent& event)
{
  int page_no;
  wxNotebookPage* current_page;

  page_no = event.GetOldSelection();
  if (page_no > -1)
  {
    current_page = notebookWindow->GetPage(page_no);
    current_page->SetExtraStyle(current_page->GetExtraStyle() | wxWS_EX_VALIDATE_RECURSIVELY);
    if (current_page->Validate() == false)
      event.Veto();
    else
    {
      current_page->TransferDataFromWindow();
      interface_state.TransferDataToTask(current_page, parent_task, false); // do not ask user
    }
  }
}

void MyFrame::OnSize(wxSizeEvent& event)
{
  int w, h;
  //int w2, h2;
  GetClientSize(&w, &h);

  //NotebookSash->GetSize(&w2, &h2);
  //! \todo leave some minimal space for MDI windows see also MyFrame::OnNotebookSashDragged
  if (w < NotebookSash_width)
    NotebookSash_width = w;

  //GetClientWindow()->SetSize(0, 0, w - NotebookSash_width, h);
  GLcanvas->SetSize(0, 0, w - NotebookSash_width, h);
//  GLcanvas->Refresh(true);
//  GLcanvas->Update();
  NotebookSash->SetSize(w-NotebookSash_width, 0, NotebookSash_width, h);
  //notebookWindow->SetSize(w-200, 0, 200, h);

  // FIXME: On wxX11, we need the MDI frame to process this
  // event, but on other platforms this should not
  // be done.

  //event.Skip();
}

void MyFrame::InitToolBar(wxToolBar* toolBar)
{
  wxBitmap bitmaps[8];

  bitmaps[0] = wxBitmap( new_xpm );
  bitmaps[1] = wxBitmap( open_xpm );
  bitmaps[2] = wxBitmap( save_xpm );
  bitmaps[3] = wxBitmap( copy_xpm );
  bitmaps[4] = wxBitmap( cut_xpm );
  bitmaps[5] = wxBitmap( paste_xpm );
  bitmaps[6] = wxBitmap( print_xpm );
  bitmaps[7] = wxBitmap( help_xpm );


  toolBar->AddTool(ID_connection_state, _T("New"), bitmaps[0], _T("Status połączenia"));
  toolBar->AddTool(1, _T("Open"), bitmaps[1], _T("Open file"));
  toolBar->AddTool(2, _T("Save"), bitmaps[2], _T("Save file"));
  toolBar->AddSeparator();
  toolBar->AddTool(3, _T("Copy"), bitmaps[3], _T("Copy"));
  toolBar->AddTool(4, _T("Cut"), bitmaps[4], _T("Cut"));
  toolBar->AddTool(5, _T("Paste"), bitmaps[5], _T("Paste"));
  toolBar->AddSeparator();
  toolBar->AddTool(6, _T("Print"), bitmaps[6], _T("Print"));
  toolBar->AddSeparator();
  toolBar->AddTool(MDI_ABOUT, _T("About"), bitmaps[7], _T("Help"));

  toolBar->Realize();
}

void MyFrame::OnProcessEnd( wxCommandEvent &event )
{
  DSPf_ErrorMessage("MyFrame::OnProcessEnd");
}

void MyFrame::OnSettingsInterfaceChange(wxCommandEvent& event)
{
  wxString t_address;
  long temp_long;
  bool refresh_GLcanvas;

  refresh_GLcanvas = false;
  switch (event.GetId())
  {
    case ID_work_as_server:
      interface_state.run_as_server = true;
      //get address from edit window
      t_address = ServerAddressEdit->GetValue();
      if (t_address.compare(MyApp::HostAddress) != 0)
        interface_state.address = t_address;
      break;
    case ID_work_as_client:
      interface_state.run_as_server = false;
      //get address from edit window
      t_address = ServerAddressEdit->GetValue();
      if (t_address.compare(MyApp::HostAddress) != 0)
        interface_state.address = t_address;
      break;
    case ID_server_address:
      if (ServerAddressEdit == NULL)
        return; // this is only text control initialization
      if (interface_state.run_as_server == false)
      {
        //get address from edit window
        t_address = ServerAddressEdit->GetValue();
        interface_state.address = t_address;
      }
      break;
    case ID_SELECT_SAMPLING_RATE:
      if (SamplingRateBox->GetValue().ToLong(&temp_long) == true)
        interface_state.sampling_rate = temp_long;
      {
        wxScrollEvent temp_event;
        temp_event.SetId(ID_SELECT_SAMPLING_RATE);
        OnChannelFilterChange(temp_event);
      }
      break;
    case ID_SELECT_MIXER_SOURCE_LINE:
      {
        int Active;
        float val;

        Active = SourceLine_ComboBox->GetSelection();
        AudioMixer->SetActiveSourceLine(Active);
        val = AudioMixer->GetActiveSourceLineVolume();
        if (val < 0)
          SourceLine_slider->Enable(false);
        else
        {
          SourceLine_slider->Enable(true);
          SourceLine_slider->SetValue((int)(val*MAX_SLIDER_VALUE));
        }
      }
      break;
    case ID_SELECT_MIXER_DEST_LINE:
      {
        int Active;
        float val;

        Active = DestLine_ComboBox->GetSelection();
        for (int ind=0; ind<AudioMixer->GetNumberOfDestLines(); ind++)
        {
          if (ind != Active)
            AudioMixer->SetDestLineState(ind, AM_MUTED_YES);
          else
            AudioMixer->SetDestLineState(ind, AM_MUTED_NO);
        }
        val = AudioMixer->GetDestLineVolume(Active);
        if (val < 0)
          DestLine_slider->Enable(false);
        else
        {
          DestLine_slider->Enable(true);
          DestLine_slider->SetValue((int)(val*MAX_SLIDER_VALUE));
        }
      }
      break;

    case ID_SELECT_DRAW_MODE:
      /*! \todo_later Add all sizers to panel before application close
       *   to make sure that all sizer will be cleared properly
       */
      {
        int val;
        //wxSizer *sizerPanel;
        bool do_transfer;

        val = DrawModeBox->GetSelection();

        do_transfer = false;
        switch (interface_state.draw_mode)
        {
          case E_DM_signal:
            break;

          case E_DM_histogram:
            break;

          case E_DM_psd:
            break;

          case E_DM_spectrogram:
            break;

          case E_DM_none:
          default:
            break;
        }

        switch (val)
        {
          case 1: // E_ST_SIGNAL
            interface_state.draw_mode = E_DM_signal;
            break;

          case 2: // E_DM_histogram
            interface_state.draw_mode = E_DM_histogram;
            break;

          case 3: // E_DM_psd
            interface_state.draw_mode = E_DM_psd;
            break;

          case 4: // E_DM_spectrogram
            interface_state.draw_mode = E_DM_spectrogram;
            break;

          case 0:
          default: // E_DM_none
            interface_state.draw_mode = E_DM_none;
            break;
        }

        refresh_GLcanvas = true;
      }
      break;

    case ID_MIKE_ON_OFF:
      if (interface_state.mike_is_off == true)
      {
        interface_state.mike_is_off = false;
        MikeToolOFF = TasksToolBar->RemoveTool(ID_MIKE_ON_OFF);
        TasksToolBar->InsertTool(6, MikeToolON);
        TasksToolBar->Realize();
      }
      else
      {
        interface_state.mike_is_off = true;
        MikeToolON = TasksToolBar->RemoveTool(ID_MIKE_ON_OFF);
        TasksToolBar->InsertTool(6, MikeToolOFF);
        TasksToolBar->Realize();
      }
      if (parent_task != NULL)
      {
        if (parent_task->ProcessingBranch != NULL)
        {
          T_BranchCommand *temp;
          TCommandData *command_data;

          interface_state.userdata_state = E_US_audio_in_gain;
          command_data = new TCommandData;
          command_data->UserData = (void *)(&interface_state);
          temp = new T_BranchCommand(E_BC_userdata, command_data);
          #ifdef __DEBUG__
            DSPf_InfoMessage("ID_MIKE_ON_OFF", "PostCommandToBranch");
          #endif
          parent_task->ProcessingBranch->PostCommandToBranch(temp);
        }
      }
      break;

    case ID_morse_receiver_state:
      interface_state.morse_receiver_state = MorseReceiverState->GetValue();

      if (parent_task != NULL)
      {
        if (parent_task->ProcessingBranch != NULL)
        {
          T_BranchCommand *temp;
          TCommandData *command_data;

          if (interface_state.morse_receiver_state == true)
            AsciiTextReceiver->Clear();
          else
            AsciiTextReceiver->SetBackgroundColour(wxColour(255,255,255));

          interface_state.userdata_state = E_US_morse_receiver_state;
          command_data = new TCommandData;
          command_data->UserData = (void *)(&interface_state);
          temp = new T_BranchCommand(E_BC_userdata, command_data);
          parent_task->ProcessingBranch->PostCommandToBranch(temp);
        }
      }
      break;

    case ID_LOCAL_SIGNAL_ON_OFF:
      if (interface_state.local_signal_gain == 0.0)
      {
        interface_state.local_signal_gain = 1.0;
        LocalSignalToolOFF = TasksToolBar->RemoveTool(ID_LOCAL_SIGNAL_ON_OFF);
        TasksToolBar->InsertTool(7, LocalSignalToolON);
        TasksToolBar->Realize();
      }
      else
      {
        interface_state.local_signal_gain = 0.0;
        LocalSignalToolON = TasksToolBar->RemoveTool(ID_LOCAL_SIGNAL_ON_OFF);
        TasksToolBar->InsertTool(7, LocalSignalToolOFF);
        TasksToolBar->Realize();
      }
      if (parent_task != NULL)
      {
        if (parent_task->ProcessingBranch != NULL)
        {
          T_BranchCommand *temp;
          TCommandData *command_data;

          interface_state.userdata_state = E_US_local_signal;
          command_data = new TCommandData;
          command_data->UserData = (void *)(&interface_state);
          temp = new T_BranchCommand(E_BC_userdata, command_data);
          #ifdef __DEBUG__
            DSPf_InfoMessage("ID_LOCAL_SIGNAL_ON_OFF", "PostCommandToBranch");
          #endif
          parent_task->ProcessingBranch->PostCommandToBranch(temp);
        }
      }
      break;

    default:
      return;
  }
  if (parent_task != NULL)
    interface_state.TransferDataToTask(NULL, parent_task, false);
  UpdateGUI();

  if (refresh_GLcanvas == true)
  {
    GetGLcanvas(0)->Refresh(); // invalidate window
    GetGLcanvas(0)->Update();  // refresh canvas immediately
  }
}


void MyGLCanvas::LockDrawingData(unsigned int GLcanvasIndex)
{
  //((MyChild *)GetParent())->LockDrawingData(GLcanvasIndex);
}

void MyGLCanvas::UnlockDrawingData(unsigned int GLcanvasIndex)
{
  //((MyChild *)GetParent())->UnlockDrawingData(GLcanvasIndex);
}

void MyGLCanvas::OnDrawNow( wxCommandEvent &event )
{
  OnDrawNow_();
}

bool MyGLCanvas::BlockDrawing = true;
bool MyGLCanvas::temporary_BlockDrawing = true;

void MyGLCanvas::OnDrawNow_(void)
{
  DSPe_SocketStatus status;
  E_DrawModes current_mode;

  if (temporary_BlockDrawing == true)
  {
    #ifdef __DEBUG__
      DSPf_ErrorMessage("Drawing block (temporary)- skipping");
    #endif
    return;
  }
  if (BlockDrawing == true)
  {
    #ifdef __DEBUG__
      DSPf_ErrorMessage("Drawing block - clearing");
    #endif
    current_mode = E_DM_none;
    //return;
  }

  CS_OnDraw.Enter();
  SetCurrent(*GLcontext);
  // Init OpenGL once, but after SetCurrent
  if (InitGL())
  {
    //! \bug should be reset for each new CurrentObject
    //! \bug CurrentObject might be NULL
    if (T_DSPlib_processing::CurrentObject != NULL)
      T_DSPlib_processing::CurrentObject->PSDs->SetWGLFont(m_bmf);
  }

  // ++++++++++++++++++++++++++++++++++++++++ //
  // ++++++++++++++++++++++++++++++++++++++++ //
  int w, h;
  GetClientSize(&w, &h);

  //if (current_mode != E_DM_none) {
  if (BlockDrawing == false) {
    if (T_DSPlib_processing::CurrentObject != NULL)
    {
      if (T_DSPlib_processing::CurrentObject->GraphInitialized == false)
      {
        long double Fp;
        unsigned int samples_per_PSD;

        Fp = T_DSPlib_processing::CurrentObject->Fp;
        samples_per_PSD = T_DSPlib_processing::CurrentObject->BufferStep;
        T_DSPlib_processing::CurrentObject->PSDs->InitialiseSpectrogram(
            0.0, Fp/2, Fp/2, samples_per_PSD,Fp, true);

        T_DSPlib_processing::CurrentObject->GraphInitialized = true;
      }
      current_mode = DrawMode;

      SocketsAreConnected = true;
      status = T_DSPlib_processing::CurrentObject->out_socket->GetSocketStatus();
      if ((status & DSP_socket_connected) == 0)
        SocketsAreConnected = false;
      status = T_DSPlib_processing::CurrentObject->in_socket->GetSocketStatus();
      if ((status & DSP_socket_connected) == 0)
        SocketsAreConnected = false;
    }
    else
    {
      current_mode = E_DM_none;
      SocketsAreConnected = false;
    }
  }

  switch (current_mode)
  {
    case E_DM_psd:
      DrawPSD(w, h);
      break;

    case E_DM_spectrogram:
      DrawSpectrogram(w, h);
      break;

    case E_DM_histogram:
      DrawHistogram(w, h);
      break;

    case E_DM_signal:
      DrawSignal(w, h);
      break;

    case E_DM_none:
    default:
      glViewport(0, 0, w, h);
      //glScissor (0,0, w, h);
      glDisable(GL_SCISSOR_TEST);

      //glClearColor(1.0,1.0,1.0,0);
      if (SocketsAreConnected == true)
      {
        glClearColor(1.0,1.0,1.0,0);
        //DSPf_InfoMessage("Connected");
      }
      else
      {
        glClearColor(1.0,1.0,0.0,0);
        //DSPf_InfoMessage("Not connected");
      }
      glClear(GL_COLOR_BUFFER_BIT);
      break;
  }
  glFlush();
  SwapBuffers();

  // ++++++++++++++++++++++++++++++++++++++++ //
  // ++++++++++++++++++++++++++++++++++++++++ //
  wxPoint poz;
  poz = ScreenToClient(::wxGetMousePosition());
  OnMouseMove(poz.x, poz.y, mouse_captured);

  CS_OnDraw.Leave();
}

bool MyGLCanvas::OnMouseDown(int x, int y)
{
  char tekst[1024];
  float xf, yf;
  bool in_subplot;

  if (T_DSPlib_processing::CurrentObject == NULL)
    return false;

  xf = x; yf = y;
  //! \bug also get units strings
  in_subplot = GetCords(T_DSPlib_processing::CurrentObject, xf, yf);
  if (in_subplot == true)
  {
    sprintf(tekst, "1. x = %i, y = %i, xf = %.2f , yf = %.2f", x, y, xf, yf);
    GetParent()->SetLabel(tekst);
  }
  else
  {
    sprintf(tekst, "2. x = %i, y = %i, xf = %.2f , yf = %.2f", x, y, xf, yf);
    GetParent()->SetLabel(tekst);
  }
  return in_subplot;
}

bool MyGLCanvas::OnMouseUp(int x, int y)
{
  //char tekst[1024];
  float xf, yf;
  bool in_subplot;

  if (T_DSPlib_processing::CurrentObject == NULL)
    return false;

  xf = x; yf = y;
  OnMouseMove(x, y, false);
  in_subplot = GetCords(T_DSPlib_processing::CurrentObject, xf, yf);
  /*
  if (in_subplot == true)
  {
    sprintf(tekst, "1. x = %i, y = %i, xf = %.2f , yf = %.2f", x, y, xf, yf);
    GetParent()->SetLabel(tekst);
  }
  else
  {
    sprintf(tekst, "2. x = %i, y = %i, xf = %.2f , yf = %.2f", x, y, xf, yf);
    GetParent()->SetLabel(tekst);
  }
  */
  return in_subplot;
}

void MyGLCanvas::OnMouseMove(int x, int y, bool captured)
{
  char tekst[1024];
  float xf, yf;
  bool in_subplot;

  if (T_DSPlib_processing::CurrentObject == NULL)
    return;

  xf = x; yf = y;
  //! \bug also get units strings
  in_subplot = GetCords(T_DSPlib_processing::CurrentObject, xf, yf);

  if (x_label != NULL)
  {
    if (in_subplot == true)
    {
      //sprintf(tekst, "1. x = %i, y = %i, xf = %.2f , yf = %.2f", x, y, xf, yf);
      sprintf(tekst, "%s = %.2f%s, %s = %.2f%s",
          x_label, xf, x_units,
          y_label, yf, y_units);
      GetParent()->SetLabel(tekst);
    }
    else
    {
      //sprintf(tekst, "2. x = %i, y = %i, xf = %.2f , yf = %.2f", x, y, xf, yf);
      sprintf(tekst, "%s = %.2f%s, %s = %.2f%s",
          x_label, xf, x_units,
          y_label, yf, y_units);
      GetParent()->SetLabel(tekst);
    }
  }
}

bool MyGLCanvas::GetCords(T_DSPlib_processing *Object, float &x_in, float &y_in)
{
  bool in_subplot;

  in_subplot = false;
  switch (DrawMode)
  {
    case E_DM_psd:
      x_label = const_cast<char *>(psd_x_label); x_units = const_cast<char *>(psd_x_units);
      y_label = const_cast<char *>(psd_y_label); y_units = const_cast<char *>(psd_y_units);

      Object->PSDs->GetSubPlotCords(1, 1, 1, x_in, y_in);
      in_subplot = ((fabs(x_in) > 1.0) && (fabs(y_in) > 1.0));

      //x_in = (x_in+1)/2 * Object->Fp / 2;
      x_in = (x_in+1) * Object->Fp / 4;
      // PSD_max_dB, PSD_range_dB;
      y_in = PSD_max_dB + (y_in-1)/2 * PSD_range_dB;
      break;

    case E_DM_spectrogram:
      x_label = const_cast<char *>(spec_x_label); x_units = const_cast<char *>(spec_x_units);
      y_label = const_cast<char *>(spec_y_label); y_units = const_cast<char *>(spec_y_units);

      Object->PSDs->GetSubPlotCords(1, 1, 1, x_in, y_in);
      in_subplot = ((fabs(x_in) > 1.0) && (fabs(y_in) > 1.0));

      //y_in = (y_in+1)/2 * Object->Fp / 2;
      y_in = (y_in+1) * Object->Fp / 4;
      x_in = (x_in-1)/2 * Object->specgram_time_span;
      break;

    case E_DM_histogram:
      x_label = const_cast<char *>(hist_x_label); x_units = const_cast<char *>(hist_x_units);
      y_label = const_cast<char *>(hist_y_label); y_units = const_cast<char *>(hist_y_units);

      Object->Histograms->GetSubPlotCords(1, 1, 1, x_in, y_in);
      in_subplot = ((fabs(x_in) > 1.0) && (fabs(y_in) > 1.0));

      y_in = (y_in+1)/2; // * (Object->NoOfHistBins * Object->BufferStep);
      //x_in = (x_in-1)/2 * Object->specgram_time_span;
      break;

    case E_DM_signal:
      x_label = const_cast<char *>(sig_x_label); x_units = const_cast<char *>(sig_x_units);
      y_label = const_cast<char *>(sig_y_label); y_units = const_cast<char *>(sig_y_units);

      Object->SignalSegments->GetSubPlotCords(1, 1, 1, x_in, y_in);
      in_subplot = ((fabs(x_in) > 1.0) && (fabs(y_in) > 1.0));

      //y_in = (y_in+1) * Object->Fp / 4;
      x_in = (x_in-1)/2 * Object->specgram_time_span;
      break;

    case E_DM_none:
    default:
      x_label = NULL; x_units = NULL;
      y_label = NULL; y_units = NULL;
      break;
  }
  return in_subplot;
}

void MyGLCanvas::DrawSpectrogram(int width, int height)
{
  if (T_DSPlib_processing::CurrentObject == NULL)
    return;

  if (T_DSPlib_processing::CurrentObject->PSDs != NULL)
  {
    T_PlotsStack *temp_plot_stack;
    int segment_size;
    glLoadIdentity();

    if (T_DSPlib_processing::CurrentObject->high_res_PSDs != NULL)
    {
      temp_plot_stack = T_DSPlib_processing::CurrentObject->high_res_PSDs;
      segment_size = T_DSPlib_processing::CurrentObject->PSD_high_size;

      temp_plot_stack->InitialiseSpectrogram(
          0.0, T_DSPlib_processing::CurrentObject->Fp/2,
          T_DSPlib_processing::CurrentObject->CurrentObject->Fp/2,
          T_DSPlib_processing::CurrentObject->NoOfSamplesPerAPSD,
          T_DSPlib_processing::CurrentObject->Fp, true);
    }
    else
    {
      temp_plot_stack = T_DSPlib_processing::CurrentObject->PSDs;
      segment_size = T_DSPlib_processing::CurrentObject->PSD_size;
    }

    // clear whole axis field
    //T_DSPlib_processing::CurrentObject->PSDs->SubPlot(1,1,-1, 0, 0, true);
    if (SocketsAreConnected == true)
      temp_plot_stack->SetBackgroundColor(0.4, CLR_gray);
    else
      temp_plot_stack->SetBackgroundColor(1.0,1.0,0.0);
    temp_plot_stack->SubPlot(1,1,-1, width, height, true);
    //  void SubPlot(int Rows, int Cols, int index, int WindowW, int WindowH, bool Clear);
    // ++++++++++++++++++++++++++++++++++++++++ //

    // select axes plot field for drawing
    temp_plot_stack->SubPlot(1,1,1, width, height, false);
    temp_plot_stack->DrawSpecgram2_dB(PSD_max_dB, PSD_range_dB, CLR_jet);

    // ++++++++++++++++++++++++++++++++++++++++ //
    // draw axis and labels
    int slot_no;
    //double to, dt;
    slot_no = 0;
    //to = SpectrogramStack->Get_SlotTime(slot_no);
    //dt = SpectrogramStack->Get_TimeWidth();
    //SpectrogramStack->PlotAxis(to, 0.0, dt, 0.0);

    // ++++++++++++++++++++++++++++++++++++++++ //
    // ++++++++++++++++++++++++++++++++++++++++ //
  }
}

void MyGLCanvas::DrawSignal(int width, int height)
{
  if (T_DSPlib_processing::CurrentObject == NULL)
    return;

  if (T_DSPlib_processing::CurrentObject->SignalSegments != NULL)
  {
    glLoadIdentity();
    // clear whole axis field
    if (SocketsAreConnected == true)
      T_DSPlib_processing::CurrentObject->SignalSegments->SetBackgroundColor(0.4, CLR_gray);
    else
      T_DSPlib_processing::CurrentObject->SignalSegments->SetBackgroundColor(1.0,1.0,0.0);
    T_DSPlib_processing::CurrentObject->SignalSegments->SubPlot(1,1,-1, width, height, true);
    //  void SubPlot(int Rows, int Cols, int index, int WindowW, int WindowH, bool Clear);
    // ++++++++++++++++++++++++++++++++++++++++ //

    // select axes plot field for drawing
    T_DSPlib_processing::CurrentObject->SignalSegments->SetBackgroundColor(1.0, CLR_gray);
    T_DSPlib_processing::CurrentObject->SignalSegments->SubPlot(1,1,1, width, height, true);
    //T_DSPlib_processing::CurrentObject->SignalSegments->SubPlot(1,1,1, width, height, false);
    SetColor(0.0, CLR_gray);
    T_DSPlib_processing::CurrentObject->SignalSegments->DrawSignal(1.0, DS_signed, 1.0);

    // ++++++++++++++++++++++++++++++++++++++++ //
    // ++++++++++++++++++++++++++++++++++++++++ //
  }
}

void MyGLCanvas::DrawHistogram(int width, int height)
{
  if (T_DSPlib_processing::CurrentObject == NULL)
    return;

  if (T_DSPlib_processing::CurrentObject->A_Histogram != NULL)
  {
    float factor;

    glLoadIdentity();
    // clear whole axis field
    if (SocketsAreConnected == true)
      T_DSPlib_processing::CurrentObject->Histograms->SetBackgroundColor(0.4, CLR_gray);
    else
      T_DSPlib_processing::CurrentObject->Histograms->SetBackgroundColor(1.0,1.0,0.0);
    T_DSPlib_processing::CurrentObject->Histograms->SubPlot(1,1,-1, width, height, true);
    //  void SubPlot(int Rows, int Cols, int index, int WindowW, int WindowH, bool Clear);
    // ++++++++++++++++++++++++++++++++++++++++ //

    // select axes plot field for drawing
    T_DSPlib_processing::CurrentObject->Histograms->SetBackgroundColor(1.0, CLR_gray);
    T_DSPlib_processing::CurrentObject->Histograms->SubPlot(1,1,1, width, height, true);
    //T_DSPlib_processing::CurrentObject->SignalSegments->SubPlot(1,1,1, width, height, false);
    SetColor(0.0, CLR_gray);
    factor = 1.0/(T_DSPlib_processing::CurrentObject->NoOfHistBins
        * T_DSPlib_processing::CurrentObject->BufferStep);
    T_DSPlib_processing::CurrentObject->Histograms->DrawSignal(
        T_DSPlib_processing::CurrentObject->NoOfHistBins,
        T_DSPlib_processing::CurrentObject->A_Histogram,
        factor, DS_unsigned, 1.0);

    // ++++++++++++++++++++++++++++++++++++++++ //
    // ++++++++++++++++++++++++++++++++++++++++ //
  }
}

void MyGLCanvas::DrawPSD(int width, int height)
{
  if (T_DSPlib_processing::CurrentObject == NULL)
    return;

  if (T_DSPlib_processing::CurrentObject->PSDs != NULL)
  {
    T_PlotsStack *temp_plot_stack;
    int segment_size;
    float factor;

    glLoadIdentity();
    // clear whole axis field
    if (T_DSPlib_processing::CurrentObject->high_res_PSDs != NULL)
    {
      temp_plot_stack = T_DSPlib_processing::CurrentObject->high_res_PSDs;
      segment_size = T_DSPlib_processing::CurrentObject->PSD_high_size;
    }
    else
    {
      temp_plot_stack = T_DSPlib_processing::CurrentObject->PSDs;
      segment_size = T_DSPlib_processing::CurrentObject->PSD_size;
    }

    if (SocketsAreConnected == true)
      temp_plot_stack->SetBackgroundColor(0.4, CLR_gray);
    else
      temp_plot_stack->SetBackgroundColor(1.0,1.0,0.0);
    temp_plot_stack->SubPlot(1,1,-1, width, height, true);
    //  void SubPlot(int Rows, int Cols, int index, int WindowW, int WindowH, bool Clear);
    // ++++++++++++++++++++++++++++++++++++++++ //

    // select axes plot field for drawing
    temp_plot_stack->SetBackgroundColor(1.0, CLR_gray);
    temp_plot_stack->SubPlot(1,1,1, width, height, true);
    //T_DSPlib_processing::CurrentObject->SignalSegments->SubPlot(1,1,1, width, height, false);
    SetColor(0.0, CLR_gray);
    factor = 1.0; ///(T_DSPlib_processing::CurrentObject->PSD_size);
    //! \bug A_PSD_dB ==> high_res_A_PSD ?????
    temp_plot_stack->DrawSignal_dB(
        segment_size,
        T_DSPlib_processing::CurrentObject->A_PSD_dB,
        PSD_max_dB, PSD_range_dB, 1.0);

    // ++++++++++++++++++++++++++++++++++++++++ //
    // ++++++++++++++++++++++++++++++++++++++++ //
    temp_plot_stack->InitAxis(T_DSPlib_processing::CurrentObject->Fp/2, PSD_range_dB,
    		T_DSPlib_processing::CurrentObject->Fp/2, 100.0, E_TM_black);
    temp_plot_stack->PlotAxis(0, PSD_max_dB-PSD_range_dB);
  }
}

// +++++++++++++++++++++++++++++++++++++++++++++++++ //
// +++++++++++++++++++++++++++++++++++++++++++++++++ //
const char *MyGLCanvas::psd_x_label = "F";
const char *MyGLCanvas::psd_x_units = " [Hz]";
const char *MyGLCanvas::psd_y_label = "|W(F)|";
const char *MyGLCanvas::psd_y_units = " [dB]";

const char *MyGLCanvas::spec_x_label = "t";
const char *MyGLCanvas::spec_x_units = " [s]";
const char *MyGLCanvas::spec_y_label = "F";
const char *MyGLCanvas::spec_y_units = " [Hz]";

const char *MyGLCanvas::hist_x_label = "x";
const char *MyGLCanvas::hist_x_units = "";
const char *MyGLCanvas::hist_y_label = "p(x)";
const char *MyGLCanvas::hist_y_units = "";

const char *MyGLCanvas::sig_x_label = "t";
const char *MyGLCanvas::sig_x_units = " [s]";
const char *MyGLCanvas::sig_y_label = "x";
const char *MyGLCanvas::sig_y_units = "";

MyGLCanvas::MyGLCanvas(unsigned int CanvasIndex,
    MyFrame *parent_in, wxWindowID id,
    int* attribList, const wxPoint& pos, const wxSize& size,
    long style, const wxString& name)
    : wxGLCanvas(parent_in, id, attribList, pos, size, style, name)
{
  ParentFrame = parent_in;
  BlockDrawing = true;
  temporary_BlockDrawing = false;

  SocketsAreConnected = false;
  DrawMode = E_DM_none; // E_DM_signal; //

  PSD_max_dB = -3.0; PSD_range_dB = 83.0;

  m_init = false;
  glc_context_id = -1;
  glc_font_id = -1;

  m_bmf = NULL;
  m_olf = NULL;
//    m_gllist = other->m_gllist; // share display list
//    m_rleft = WXK_LEFT;
//    m_rright = WXK_RIGHT;

  this_canvas_index = CanvasIndex;
  GLcontext = new wxGLContext(this);

  //! enable data access only for one process
  DrawData_semaphore = new wxSemaphore(1, 1);

  x_label = NULL; x_units = NULL;
  y_label = NULL; y_units = NULL;

  mouse_captured = false;
  //SpecDraw = NULL;
}

MyGLCanvas::~MyGLCanvas(void)
{
#ifdef _USE_GLC_
  glcDeleteFont( glc_font_id );
  glcDeleteContext( glc_context_id );
#endif

  delete GLcontext;

  if (DrawData_semaphore != NULL)
  {
    delete DrawData_semaphore;
    DrawData_semaphore = NULL;
  }
}

void MyGLCanvas::EnableDrawing(bool permament)
{
  if (permament == true) {
    BlockDrawing = false;
    temporary_BlockDrawing = false;
  }
  else
    temporary_BlockDrawing = false;
}

void MyGLCanvas::DisableDrawing(bool permament)
{
  if (permament == true)
    BlockDrawing = true;
  else
    temporary_BlockDrawing = true;
}

E_DrawModes MyGLCanvas::GetDrawMode(void)
{
  return DrawMode;
}

void MyGLCanvas::SetDrawMode(E_DrawModes draw_mode_in)
{
  DrawMode = draw_mode_in;
}

void MyGLCanvas::OnPaint(wxPaintEvent& event)
{
  wxPaintDC dc(this);

  OnDrawNow_();

  event.Skip(); // można chyba pominąæ
}

bool MyGLCanvas::InitGL(void)
{
  SetCurrent(*GLcontext);

  if (m_init == true)
    return false;

  m_olf = NULL;
  //m_olf = new COutlineFont(this, "Courier"); //"Arial");
  m_bmf = new CBitmapFont(this, _T("Courier"));

#ifdef _USE_GLC_
  glc_context_id = glcGenContext ();
  glcContext ( glc_context_id );

  glc_font_id = glcGenFontID();
  //glcNewFontFromFamily ( glc_font_id, "Times New Roman" );
  //glcNewFontFromFamily ( glc_font_id, "Arial" );
  glcNewFontFromFamily ( glc_font_id, "Courier New" );

  glcFont ( glc_font_id );
  glcFontFace(glc_font_id, "Regular"); // "Italic", "Regular", "Bold"
  //glcFontFace ( glc_font_id, "Bold");

  //glcStringType(GLC_UTF8_QSO);
  //glcRenderStyle(GLC_BITMAP);
  //glcRenderStyle(GLC_LINE);

  //glcRenderStyle(GLC_TRIANGLE); // <= requires fontconfig.dll and xmlparse.dll
  //; // <== GL scaling and rotation instead of GLC mones must be used
  // glcRenderStyle(GLC_TEXTURE); // needs textures

#endif // _USE_GLC_

    /*
      // set viewing projection
      glMatrixMode(GL_PROJECTION);
      glFrustum(-0.5f, 0.5f, -0.5f, 0.5f, 1.0f, 3.0f);

      // position viewer
      glMatrixMode(GL_MODELVIEW);
      glTranslatef(0.0f, 0.0f, -2.0f);

      // position object
      glRotatef(30.0f, 1.0f, 0.0f, 0.0f);
      glRotatef(30.0f, 0.0f, 1.0f, 0.0f);

      glEnable(GL_DEPTH_TEST);
      glEnable(GL_LIGHTING);
      glEnable(GL_LIGHT0);
     */

  m_init = true;

  int w, h;
  GetClientSize(&w, &h);

  /*
  if (SpecDraw != NULL)
  {
    SpecDraw->OnSize(w, h);
  }
  */
  return true;
}

void MyGLCanvas::OnSize(wxSizeEvent& event)
{
  //// this is also necessary to update the context on some platforms
  //wxGLCanvas::OnSize(event);

  // set GL viewport (not called by wxGLCanvas::OnSize on all platforms...)
  int w, h;
  GetClientSize(&w, &h);

  /*
  if (SpecDraw != NULL)
  {
    SpecDraw->OnSize(w, h);
  }
  */
}

void MyGLCanvas::OnEraseBackground(wxEraseEvent& WXUNUSED(event))
{
  if (ParentFrame == NULL)
    return;

  if (ParentFrame->GetParentTask() == NULL)
  {
    int w, h;
    GetClientSize(&w, &h);

    InitGL();

    glViewport(0, 0, w, h);
    //glScissor (0,0, w, h);
    glDisable(GL_SCISSOR_TEST);

    //if (SocketsAreConnected == true)
    //  glClearColor(1.0,1.0,1.0,0);
    //else
      glClearColor(1.0,1.0,0.0,0);
    glClear(GL_COLOR_BUFFER_BIT);

    glFlush();
    SwapBuffers();
  }
  else
  {
    // Do nothing, to avoid flashing.
  }
}

//! \Fixed Removed support because it stole mouse capture from edit controls
void MyGLCanvas::OnEnterWindow( wxMouseEvent& WXUNUSED(event) )
{
  SetFocus();
}

void MyGLCanvas::OnMouseLeftDown(wxMouseEvent& event)
{
  if (ParentFrame != NULL)
  {
    if (OnMouseDown(event.GetX(), event.GetY()));
    {
      mouse_captured = true;
      CaptureMouse();
    }
  }
  event.Skip();
}
void MyGLCanvas::OnMouseLeftUp(wxMouseEvent& event)
{
  if (mouse_captured == true)
  {
    ReleaseMouse();
    if (ParentFrame != NULL)
    {
      OnMouseUp(event.GetX(), event.GetY());
    }
    mouse_captured = false;
  }

  event.Skip();
}


void MyGLCanvas::OnMouseMotion( wxMouseEvent& event )
{
  //char tekst[1024];

  //sprintf(tekst, "x = %i, y = %i", event.GetX(), event.GetY());
  //GetParent()->SetLabel(tekst);
  if (ParentFrame != NULL)
  {
    OnMouseMove(event.GetX(), event.GetY(), mouse_captured);
  }
  event.Skip();
}

// +++++++++++++++++++++++++++++++++++++++ //
// +++++++++++++++++++++++++++++++++++++++ //
int MyProcessingThread::NoOfCPUs = 0;
int MyProcessingThread::NoOfProcessingThreads = 0;
MyProcessingThread **MyProcessingThread::ProcessingThreads = NULL;


MyProcessingThread::MyProcessingThread(MyFrame *Parent_in)
 : wxThread(wxTHREAD_DETACHED) //wxTHREAD_JOINABLE
{
  Parent = Parent_in;
  NoOfBranches = 0;
  Branches = NULL;

  new_branch = NULL;

  UpdateBranches_semaphore = new wxSemaphore(0, 1);
}

void MyProcessingThread::CreateAndRunThreads(MyFrame *Parent_in, int NoOfThreads)
{
  int ind;

  Initialize();

  if (NoOfThreads == -1)
    NoOfThreads = NoOfCPUs - 1;
  if (NoOfThreads <= 0)
    NoOfThreads = 1;

  NoOfProcessingThreads = NoOfThreads;
  ProcessingThreads = new MyProcessingThread *[NoOfProcessingThreads];
  ThreadFinished_semaphore = new wxSemaphore *[NoOfProcessingThreads];
  for (ind = 0; ind < NoOfProcessingThreads; ind++)
  {
    ThreadFinished_semaphore[ind] = new wxSemaphore(0,1);
    ProcessingThreads[ind] = new MyProcessingThread(Parent_in);
    ProcessingThreads[ind]->ThreadIndex = ind;
    ProcessingThreads[ind]->Create(100000000);

    //ProcessingThreads[0]->SetPriority(20);
    ProcessingThreads[ind]->Run();
  }
}

void MyProcessingThread::Initialize(void)
{
  char tekst[1024];

  /*! \todo_later Determine no of cores vs logical (HT) processors
   * GetSystemInfo http://msdn.microsoft.com/en-us/library/ms724381.aspx
   * GetLogicalProcessorInformation  http://msdn.microsoft.com/en-us/library/ms683194.aspx
   *
   * http://www.ureader.com/message/477694.aspx
   * http://or1cedar.intel.com/media/training/detect_ht_dt_v1/tutorial/index.htm
   */
  NoOfCPUs = GetCPUCount();
  if (NoOfCPUs <= 0)
    NoOfCPUs = 1;
  sprintf(tekst, "Number of CPUs detected = %i", NoOfCPUs);
  DSPf_InfoMessage(tekst);

  /*
  if (m_priority <= 20)
      win_priority = THREAD_PRIORITY_LOWEST;
  else if (m_priority <= 40)
      win_priority = THREAD_PRIORITY_BELOW_NORMAL;
  else if (m_priority <= 60)
      win_priority = THREAD_PRIORITY_NORMAL;
  else if (m_priority <= 80)
      win_priority = THREAD_PRIORITY_ABOVE_NORMAL;
  else if (m_priority <= 100)
      win_priority = THREAD_PRIORITY_HIGHEST;
  */
}

MyProcessingThread::~MyProcessingThread(void)
{
  unsigned int ind;

  FreeThread();

  if (Branches != NULL)
  {
    for (ind = 0; ind < NoOfBranches; ind++)
    {
      if (Branches[ind] != NULL)
      {
        if (Branches[ind]->Parent->GetParentTask() != NULL)
          Branches[ind]->Parent->GetParentTask()->DeleteBranch(Branches[ind]);
        else
        {
          DSPf_ErrorMessage("MyProcessingThread::~MyProcessingThread", "GetParentTask() == NULL");
          delete Branches[ind];
        }
        //delete Branches[ind];
        Branches[ind] = NULL;
      }
    }
    delete [] Branches;
    Branches = NULL;
  }
  if (UpdateBranches_semaphore != NULL)
  {
    delete UpdateBranches_semaphore;
    UpdateBranches_semaphore = NULL;
  }
}

wxSemaphore **MyProcessingThread::ThreadFinished_semaphore = NULL;
void MyProcessingThread::FreeThread(void)
{
  int ind, ind2;

  for (ind = 0; ind < NoOfProcessingThreads; ind++)
    if (ProcessingThreads[ind] == this)
    {
      if (ThreadFinished_semaphore[ind] != NULL)
        delete ThreadFinished_semaphore[ind];

      for (ind2 = ind+1; ind2 < NoOfProcessingThreads; ind2++)
      {
        ProcessingThreads[ind2-1] = ProcessingThreads[ind2];
        ThreadFinished_semaphore[ind2-1] = ThreadFinished_semaphore[ind2];
      }

      NoOfProcessingThreads--;
      if (NoOfProcessingThreads == 0)
      {
        delete [] ProcessingThreads;
        ProcessingThreads = NULL;
        delete [] ThreadFinished_semaphore;
        ThreadFinished_semaphore = NULL;
      }
      // else: just leave a bit to large memory allocation
      break;
    }
}

void MyProcessingThread::FreeThreads(void)
{
  int ind;
  int temp_NoOfThread;

  temp_NoOfThread = NoOfProcessingThreads;
  NoOfProcessingThreads = 0;

  ////! tell threads to finish
  for (ind = 0; ind < temp_NoOfThread; ind++)
    ProcessingThreads[ind]->Delete();
  //delete [] ProcessingThreads; // this also is delated in FreeThread
  //ProcessingThreads = NULL;
  NoOfCPUs = 0; // Forces initialization next time new thread is created
}

//unsigned int tread_wait_counter = 0;
wxThread::ExitCode MyProcessingThread::Entry(void)
{
  //unsigned int ind;
  bool DoFinish;
  unsigned int ind;
  int NoOfActiveBranches = 0;

  DoFinish = false;

  CS_OnDraw.Enter();
  Parent->GetGLcanvas(0)->EnableDrawing();
  CS_OnDraw.Leave();

  //for (ind=0 ; ind < 1000; ind++)
  while (1)
  {
    //! \bug Use number of active branches (NoOfBranches - NoOfPaused)
    if (NoOfActiveBranches == 0)
    {
      UpdateBranches_semaphore->WaitTimeout(10); // suspend process until new branches will be available
    }

    //DSPf_InfoMessage("Yield");
    //! \ bug check if it should be done here (maybe timer in main loop)
    //::wxGetApp().Yield(true);
    //DSPf_InfoMessage("Yield2");

    //DSPf_InfoMessage("UpdateBranches()");
    if (UpdateBranches() == false)
      DoFinish = true;


    //DSPf_InfoMessage("ProcessBranches()");
    // command can be post before thread start
    NoOfActiveBranches = 0;
    for (ind = 0; ind < NoOfBranches; ind++)
    {
      if (Branches[ind]->ProcessBranch() == true)
        NoOfActiveBranches++;
    }

    // Use number of active branches (NoOfBranches - NoOfPaused)
    if ((Parent != NULL) && (NoOfActiveBranches != 0))
    {
      //AddPendingEvent
      //DSPf_InfoMessage("GLcanvas->Refresh()");
      Parent->GetGLcanvas(0)->Refresh(); // invalidate window
      //! \bug na słabszym sprzęcie bez Update poniżej się potrafi wywalić
      //Parent->GetGLcanvas(0)->Update();  // refresh canvas immediately
    }

/*
    //DSPf_InfoMessage("DrawWait()");
    for (ind = 0; ind < NoOfBranches; ind++)
      if (Branches[ind]->DrawIsBlocked == false)
      {
        while (Branches[ind]->Parent->GUIready_semaphore->WaitTimeout(10) == wxSEMA_TIMEOUT)
        {
          if (TestDestroy())
            break;
          ::wxGetApp().Yield(true);
        }
        ::wxGetApp().Yield(true);
      }
*/
    //if ((TestDestroy() == true) || (DoFinish == true))
    if (TestDestroy() == true)
      break;

    //DSPf_InfoMessage("DoFinish()");
    if (DoFinish == true)
    {
      //DSPf_InfoMessage("DoFinish() == true");
      wxCommandEvent event( wxEVT_PROCESS_END, ID_ProcessEnd );
      event.SetClientData( this );
      Parent->GetEventHandler()->AddPendingEvent( event );
      break;
    }
  }

  CS_OnDraw.Enter();
  Parent->GetGLcanvas(0)->DisableDrawing();
  CS_OnDraw.Leave();

  DSPf_ErrorMessage("Thread loop ended");

  ////  not needed for wxTHREAD_JOINABLE
  DSPf_ErrorMessage("ThreadFinished is about to be posted");
  ThreadFinished_semaphore[ThreadIndex]->Post(); // signal finishing
  DSPf_ErrorMessage("ThreadFinished has been posted");

  return 0;
}

MyProcessingBranch *MyProcessingThread::AddProcessingBranch(MyFrame *Parent_in,
    T_InputElement *ProcessingStack)
{
  if (NoOfProcessingThreads > 0)
  {
    MyProcessingBranch *temp;

    //! \todo implement new branches creation
    temp = new MyProcessingBranch(ProcessingThreads[0], Parent_in, ProcessingStack);
    ProcessingThreads[0]->new_branch = temp;

    //! \todo Managed branches distribution if NoOfProcessingThreads > 1
    ProcessingThreads[0]->UpdateBranches_semaphore->Post();

    return temp;
  }

  return NULL;
}

// Updates branches
/*! \note Returns false if updated thread processing must be finished
 *
 * \note No Branches Update should be performed during drawing.
 *   The same with other critical Branches commands processing.
 */
bool MyProcessingThread::UpdateBranches(void)
{
  unsigned int ind, ind2, new_NoOfBranches;

  MyProcessingBranch::CS_CommandList.Enter();

  if (new_branch != NULL)
  {
    DSPf_InfoMessage("MyProcessingThread::UpdateBranches", "new_branch");

    MyProcessingBranch **new_Branches;
    new_Branches = new MyProcessingBranch *[NoOfBranches+1];
    if (Branches != NULL)
    {
      memcpy(new_Branches, Branches, sizeof(MyProcessingBranch *) * NoOfBranches);
      delete [] Branches;
    }
    // initialize start/end time tables
    new_branch->ProcessingStack->UpdateTimeTables(E_PD_forward, -1);
    new_Branches[NoOfBranches] = new_branch;
    Branches = new_Branches;
    NoOfBranches++;

    CS_OnDraw.Enter();
    Parent->GetGLcanvas(0)->EnableDrawing();
    CS_OnDraw.Leave();

    new_branch = NULL;
  }

  ind2 = 0;
  new_NoOfBranches = 0;
  for (ind = 0; ind < NoOfBranches; ind++)
  {
    if (Branches[ind]->DoFinish == true)
    {
      DSPf_InfoMessage("MyProcessingThread::UpdateBranches", "branch delete");

      CS_OnDraw.Enter();
      Parent->GetGLcanvas(0)->DisableDrawing();
      CS_OnDraw.Leave();

      if (Branches[ind]->Parent->GetParentTask() != NULL)
        Branches[ind]->Parent->GetParentTask()->DeleteBranch(Branches[ind]);
      else
      {
        DSPf_ErrorMessage("MyProcessingThread::UpdateBranches", "branch parent task is NULL");
        delete Branches[ind];
      }
      Branches[ind] = NULL;
    }
    else
    {
      if (ind > new_NoOfBranches)
      {
        DSPf_InfoMessage("MyProcessingThread::UpdateBranches", "branch relocated");

        Branches[new_NoOfBranches] = Branches[ind];
      }
      new_NoOfBranches++;
    }

  }
  NoOfBranches = new_NoOfBranches;
  if ((NoOfBranches == 0) && (Branches != NULL))
  {
    delete [] Branches;
    Branches = NULL;
  }

  //DSPf_InfoMessage("MyProcessingThread::UpdateBranches", "update finished");
  MyProcessingBranch::CS_CommandList.Leave();
  return true; // keep thread working
}


void MyFrame::OnChannelFilterChange(wxScrollEvent& event)
{
  float Fd, Fg;
  bool Fd_fix = false;
  bool Fg_fix = false;
  bool keep_Fg = false;

  switch (event.GetId())
  {
    case ID_LPF_SLIDER:
      Fg = LPF_slider->GetValue();
      Fg *= 100;
      Fd = interface_state.channel_Fd;
      keep_Fg = true;
      break;
    case ID_HPF_SLIDER:
      Fd = HPF_slider->GetValue();
      Fd *= 100;
      Fg = interface_state.channel_Fg;
      keep_Fg = false;
      break;

    case ID_SELECT_SAMPLING_RATE:
      // update sliders settings based on current sampling rate
      // interface_state.sampling_rate
      HPF_slider->SetRange(0, (int)(interface_state.sampling_rate/200));
      LPF_slider->SetRange(0, (int)(interface_state.sampling_rate/200));
      HPF_slider->SetValue(0);
      LPF_slider->SetValue((int)(interface_state.sampling_rate/200));
      interface_state.channel_Fd = 0;
      interface_state.channel_Fg = interface_state.sampling_rate/2;

      Fd = interface_state.channel_Fd;
      Fg = interface_state.channel_Fg;
      break;
  }

  if (keep_Fg == true)
  {
    //if (Fd > Fg-100)
    if (Fd > Fg)
    {
      //Fd = Fg - 100;
      Fd = Fg;
      Fd_fix = true;
    }
  }
  else
  {
    //if (Fd > Fg-100)
    if (Fd > Fg)
    {
      //Fg = Fd + 100;
      Fg = Fd;
      Fg_fix = true;
    }
  }
  //! \bug make use of filter design limits
  if (Fd < 100)
  {
    Fd = 0;
    Fd_fix = true;
  }
  if (Fg < 100)
  {
    Fg = 100;
    Fg_fix = true;
  }
  if (Fd >= interface_state.sampling_rate/2 - 100)
  {
    Fd = interface_state.sampling_rate/2 - 100;
    Fd_fix = true;
  }
  if (Fg >= interface_state.sampling_rate/2)
  {
    Fg = interface_state.sampling_rate/2;
    Fg_fix = true;
  }

  if (event.GetEventType() != wxEVT_SCROLL_CHANGED)
  { // only show current position but do not update
    HPF_text->ChangeValue(wxString::Format("%.0fHz", Fd));
    LPF_text->ChangeValue(wxString::Format("%.0fHz", Fg));
    return;
  }

  interface_state.userdata_state = E_US_none;
  if (interface_state.channel_Fd != Fd)
  {
    interface_state.channel_Fd = Fd;
    interface_state.userdata_state = E_US_channel_HPF_coefs;
    HPF_text->ChangeValue(wxString::Format("%.0fHz", Fd));
  }
  if (interface_state.channel_Fg != Fg)
  {
    interface_state.channel_Fg = Fg;
    interface_state.userdata_state |= E_US_channel_LPF_coefs;
    LPF_text->ChangeValue(wxString::Format("%.0fHz", Fg));
  }

  if (Fg_fix == true)
  {
    LPF_slider->SetValue((int)(Fg/100));
  }
  if (Fd_fix == true)
  {
    HPF_slider->SetValue((int)(Fd/100));
  }

  if (parent_task != NULL)
  {
    interface_state.TransferDataToTask(NULL, parent_task, false);

    if (parent_task->ProcessingBranch != NULL)
    {
      T_BranchCommand *temp;
      TCommandData *command_data;

      command_data = new TCommandData;
      command_data->UserData = (void *)(&interface_state);
      temp = new T_BranchCommand(E_BC_userdata, command_data);
      #ifdef __DEBUG__
        DSPf_InfoMessage("Channel filter settings change", "PostCommandToBranch");
      #endif
      parent_task->ProcessingBranch->PostCommandToBranch(temp);
    }
  }
  //UpdateGUI();
}


void MyFrame::OnChannelSNRChange(wxScrollEvent& event)
{
  float SNR_dB;

  switch (event.GetId())
  {
    case ID_SNR_SLIDER:
      //0..MAX_SLIDER_VALUE ==> -20 .. 80
      SNR_dB = SNR_slider->GetValue();
      SNR_dB /= MAX_SLIDER_VALUE;
      SNR_dB *= 100;
      SNR_dB -= 20;

      SNR_text->ChangeValue(wxString::Format("%.0f dB", SNR_dB));
      if (event.GetEventType() != wxEVT_SCROLL_CHANGED)
      { // only show current position but do not update
        return;
      }
      interface_state.SNR_dB = SNR_dB;
      break;

    default:
      return;
  }


  interface_state.userdata_state = E_US_noise_level;
  if (parent_task != NULL)
  {
    interface_state.TransferDataToTask(NULL, parent_task, false);

    if (parent_task->ProcessingBranch != NULL)
    {
      T_BranchCommand *temp;
      TCommandData *command_data;

      command_data = new TCommandData;
      command_data->UserData = (void *)(&interface_state);
      temp = new T_BranchCommand(E_BC_userdata, command_data);
      #ifdef __DEBUG__
        DSPf_InfoMessage("MyFrame::OnChannelSNRChange", "PostCommandToBranch");
      #endif
      parent_task->ProcessingBranch->PostCommandToBranch(temp);
    }
  }
  //UpdateGUI();
}

void MyFrame::OnPSDparamsChange(wxScrollEvent& event)
{
  int no_of_slots;

  switch (event.GetId())
  {
    case ID_PSD_SLOTS_SLIDER:
      //0..MAX_SLIDER_VALUE ==> -20 .. 80
      no_of_slots = PSD_slots_slider->GetValue();
      no_of_slots *= 50;
      no_of_slots += 100;

      PSD_slots_text->ChangeValue(wxString::Format("%i", no_of_slots));
      interface_state.no_of_psd_slots = no_of_slots;
      break;

    default:
      return;
  }

  if (parent_task != NULL)
  {
    interface_state.TransferDataToTask(NULL, parent_task, false);
  }
  //UpdateGUI();
}

void MyFrame::OnMixerVolumeChange(wxScrollEvent& event)
{
  float val;
  int Active;

  switch (event.GetId())
  {
    case ID_SourceLine_SLIDER:
      val = SourceLine_slider->GetValue();
      val /= MAX_SLIDER_VALUE;
      Active = SourceLine_ComboBox->GetSelection();
      AudioMixer->SetSourceLineVolume(Active, val);
      break;

    case ID_DestLine_SLIDER:
      val = DestLine_slider->GetValue();
      val /= MAX_SLIDER_VALUE;
      Active = DestLine_ComboBox->GetSelection();
      AudioMixer->SetDestLineVolume(Active, val);
      break;

    case ID_MasterLine_SLIDER:
      val = MasterLine_slider->GetValue();
      val /= MAX_SLIDER_VALUE;
      AudioMixer->SetDestLineVolume(AM_MasterControl, val);
      break;

    default:
      return;
  }
}


void MyFrame::OnWPMchange(wxScrollEvent& event)
{
  wxString text;
  int WPM;

  switch (event.GetId())
  {
    case ID_WPM_SLIDER:
      WPM = WPM_slider->GetValue();
      WPM = 5+WPM*5;

      WPM_text->SetValue(wxString::Format("%i WPM", WPM));
      break;

    default:
      return;
  }

  if (event.GetEventType() != wxEVT_SCROLL_CHANGED)
  { // only show current position but do not update
    return;
  }

  interface_state.WPM = WPM;
  interface_state.userdata_state = E_US_WPM_change;

  if (parent_task != NULL)
  {
    interface_state.TransferDataToTask(NULL, parent_task, false);

    if (parent_task->ProcessingBranch != NULL)
    {
      T_BranchCommand *temp;
      TCommandData *command_data;

      command_data = new TCommandData;
      command_data->UserData = (void *)(&interface_state);
      temp = new T_BranchCommand(E_BC_userdata, command_data);
      #ifdef __DEBUG__
        DSPf_InfoMessage("Myframe::OnSendAsciText", "PostCommandToBranch");
      #endif
      parent_task->ProcessingBranch->PostCommandToBranch(temp);
    }
  }
  //UpdateGUI();
}


void MyFrame::OnButtonPress(wxCommandEvent& event)
{
  wxString text;

  // TODO https://docs.wxwidgets.org/3.0/classwx_string.html

  // TODO => konwersja z wxString do ASCII => z uwzglenieniem polskich znaków (???)
  switch (event.GetId())
  {
    case ID_send_ascii_text:
      {
        text = AsciiTextEntry->GetValue().Upper();
        for (unsigned int ind = 0; ind < text.Length(); ind++)
        {
          // upper dla polskich znaków
          switch(text[ind].GetValue())
          {
            case _T('ą'):
              text[ind] = _T('Ą');
              break;
            case _T('ć'):
              text[ind] = _T('Ć');
              break;
            case _T('ę'):
              text[ind] = _T('Ę');
              break;
            case _T('ł'):
              text[ind] = _T('Ł');
              break;
            case _T('ń'):
              text[ind] = _T('Ń');
              break;
            case _T('ś'):
              text[ind] = _T('Ś');
              break;
            case _T('ź'):
              text[ind] = _T('Ź');
              break;
            case _T('ż'):
              text[ind] = _T('Ż');
              break;
            case _T('ó'):
              text[ind] = _T('Ó');
              break;
            default:
              break;
          }
        }
        interface_state.ascii_text = text;

        AsciiTextEntry->ChangeValue(_T(""));
        interface_state.userdata_state = E_US_ascii_text;
      }
      break;

    case ID_select_voice_file:
      {
        int voice_type, file_no;
        string index_text;
        bool is_logatom;

        is_logatom = (UseLogatoms->GetValue() == 1);
        // składamy nazwę pliku w interface_state.selected_wav_filename
        string &temp = interface_state.selected_wav_filename;
        if (is_logatom == true)
        {
          temp = "Logatomy\\";
          index_text = 'L';
        }
        else
        {
          temp, 2047, "Zdania\\";
          index_text = 'Z';
        }

        voice_type = VoiceTypeBox->GetCurrentSelection();
        if (voice_type == 0)
          voice_type = 1 + (rand() % 4);
        index_text += char('0' + voice_type-1);
        switch (voice_type)
        {
          case 1:
            temp += "male1\\";
            break;
          case 2:
            temp += "male2\\";
            break;
          case 3:
            temp += "female1\\";
            break;
          case 4:
            temp += "female2\\";
            break;
          default:
            break;
        }

        string file_no_str = to_string(file_no);
        while (file_no_str.length() < 3) { // "%03i"
          file_no_str = string("0") + file_no_str;
        }
        if (is_logatom == true)
        {
          file_no = 1 + (rand() % 3);
          //snprintf(temp, 2047-size, "lista %i.wav", file_no);
          temp += "lista ";
          temp += to_string(file_no);
          temp += ".wav";
        }
        else
        {
          file_no = 1 + (rand() % 500);
          //snprintf(temp, 2047-size, "%03i.wav", file_no);

          temp += file_no_str;
          temp += ".wav";
        }
        //snprintf(index_text+2, 1023-2, "%03i", file_no);
        index_text += file_no_str;

        VoiceFileIndex->ChangeValue(index_text);
      }
      return;
      break;

    case ID_stop_wav_file:
      interface_state.wav_filename = "";

      interface_state.userdata_state = E_US_wav_file_open;
      break;

    case ID_open_wav_file:
      interface_state.wav_filename = interface_state.selected_wav_filename;

      interface_state.userdata_state = E_US_wav_file_open;
      break;

    default:
      return;
  }

  if (parent_task != NULL)
  {
    interface_state.TransferDataToTask(NULL, parent_task, false);

    if (parent_task->ProcessingBranch != NULL)
    {
      T_BranchCommand *temp;
      TCommandData *command_data;

      command_data = new TCommandData;
      command_data->UserData = (void *)(&interface_state);
      temp = new T_BranchCommand(E_BC_userdata, command_data);
      #ifdef __DEBUG__
        DSPf_InfoMessage("Myframe::OnSendAsciText", "PostCommandToBranch");
      #endif
      parent_task->ProcessingBranch->PostCommandToBranch(temp);
    }
  }
  //UpdateGUI();
}

void MyFrame::OnDrawModeChange(wxCommandEvent& event)
{
  switch (event.GetId())
  {
    case ID_draw_time_signal:
      DrawModeBox->SetSelection(1);
      break;
    case ID_draw_histogram:
      DrawModeBox->SetSelection(2);
      break;
    case ID_draw_psd:
      DrawModeBox->SetSelection(3);
      break;
    case ID_draw_spectrogram:
      DrawModeBox->SetSelection(4);
      break;
    case ID_draw_none:
    default:
      DrawModeBox->SetSelection(0);
      break;
  }
  event.SetId(ID_SELECT_DRAW_MODE);
  OnSettingsInterfaceChange(event);
}



BEGIN_EVENT_TABLE(wxMorseValidator, wxValidator)
    EVT_CHAR(wxMorseValidator::OnChar)
    EVT_TEXT(wxID_ANY, wxMorseValidator::OnTextChange)
END_EVENT_TABLE()

wxMorseValidator::wxMorseValidator(int prec_in, E_NV_mode mode_in, long double *m_long_double_Value_in)
{
  current_morse_block = NULL;
}

wxMorseValidator::wxMorseValidator(const wxMorseValidator& val)
    : wxValidator()
{
  Copy(val);
}

bool wxMorseValidator::Copy(const wxMorseValidator& val)
{
  wxValidator::Copy(val);

  current_morse_block = val.current_morse_block;

  return true;
}

// Called when the value in the window must be validated.
// This function can pop up an error message.
bool wxMorseValidator::Validate(wxWindow *parent)
{
  bool ok;

  if( !CheckValidator() )
    return false;

  wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow;

  // If window is disabled, simply return
  if ( !control->IsEnabled() )
      return true;

  wxString val(control->GetValue());

  //! \todo ??? is there really something to do here
  ok = true;

  return ok;
}

// Called to transfer data to the window
bool wxMorseValidator::TransferToWindow(void)
{
    if( !CheckValidator() )
        return false;

    if ( current_morse_block != NULL )
    {
      wxString stringValue;

      wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow;

      //! \bug get internal ascii buffer of current_morse_block

      control->SetValue(stringValue);
    }

    return true;
}

// Called to transfer data to the window
bool wxMorseValidator::TransferFromWindow(void)
{
  if( !CheckValidator() )
      return false;

  if ( current_morse_block != NULL )
  {
    wxString stringValue;

    wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow;
    stringValue = control->GetValue();

    //! \bug set internal ascii buffer of current_morse_block
  }
  return true;
}

//! \todo implement EVT_TEXT to control - live text changes control
void wxMorseValidator::OnTextChange( wxCommandEvent &event )
{
  int id;

  id = event.GetId();

  if (m_validatorWindow->GetId() == id)
  {
    // 1. check if string is correct
    // 2. store cursor position
    // 3. update text with ChangeValue
    //((wxTextCtrl *)m_validatorWindow)->ChangeValue(_("11"));
    // 4. restore cursor position
  }
  event.Skip();
}

void wxMorseValidator::OnChar(wxKeyEvent& event)
{
  if ( m_validatorWindow )
  {
    bool discard_key;
    int keyCode = event.GetKeyCode();

    //! \bug implement sensible processing of characters
    discard_key = false;
    // we don't filter special keys and Delete
    if (
        !(keyCode < WXK_SPACE || keyCode == WXK_DELETE || keyCode > WXK_START)
         && !wxIsdigit(keyCode))
         //&& keyCode != wxT('.') && keyCode != wxT(',') && keyCode != wxT('-') )
         //&& keyCode != wxT('+') && keyCode != wxT('e') && keyCode != wxT('E'))
    {
      discard_key = true;
    }

    if (discard_key == true)
    {
      if ( !wxValidator::IsSilent() )
         wxBell();

      // eat message
      return;
    }

    //! \bug depending on mode send character to current_morse_block
  }

  event.Skip();
}

