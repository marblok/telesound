#include "main.h"
// #include <wx/mdi.h>
#include <wx/toolbar.h>
#include <wx/thread.h>
#include <wx/combobox.h>
#include <wx/glcanvas.h>
#include <wx/gbsizer.h>

// #include <DSP.h>
#include "Processing.h"
#include "MyGLCanvas.h"
#include "Branches.h"

#include "Icons.h"

wxDEFINE_EVENT(wxEVT_DRAW_NOW, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_PROCESS_END, wxCommandEvent);
wxDEFINE_EVENT(wxEVT_BRANCH_END, wxCommandEvent);

#ifdef __DEBUG__
wxDEFINE_EVENT(wxEVT_STATUSBOX_UPDATE, wxCommandEvent);
#endif // __DEBUG__

IMPLEMENT_APP(MainApp)

// ---------------------------------------------------------------------------
// event tables
// ---------------------------------------------------------------------------
BEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_BUTTON(MDI_ABOUT, MainFrame::OnAbout)

EVT_BUTTON(ID_RUN_TASK, MainFrame::OnRunTask)
EVT_BUTTON(ID_PAUSE_TASK, MainFrame::OnPauseTask)
EVT_BUTTON(ID_STOP_TASK, MainFrame::OnStopTask)

EVT_BUTTON(ID_MIKE_ON_OFF, MainFrame::OnSettingsInterfaceChange)
EVT_BUTTON(ID_LOCAL_SIGNAL_ON_OFF, MainFrame::OnSettingsInterfaceChange)
EVT_BUTTON(ID_RESET_SETTINGS, MainFrame::OnSettingsInterfaceChange)

EVT_COMMAND(ID_BranchEnd, wxEVT_BRANCH_END, T_TaskElement::OnBranchEnd)
#ifdef __DEBUG__
EVT_COMMAND(ID_StatusBox_AppendText, wxEVT_STATUSBOX_UPDATE, T_TaskElement::OnStatusBoxUpdate)
#endif // __DEBUG__

EVT_MENU(MDI_QUIT, MainFrame::OnQuit)

EVT_CLOSE(MainFrame::OnClose)
// EVT_SIZE(MainFrame::OnSize)

EVT_COMMAND(ID_ProcessEnd, wxEVT_PROCESS_END, MainFrame::OnProcessEnd)

// EVT_NOTEBOOK_PAGE_CHANGING(wxID_ANY, MainFrame::OnPageChanging)

EVT_RADIOBUTTON(ID_work_as_server, MainFrame::OnSettingsInterfaceChange)
EVT_RADIOBUTTON(ID_work_as_client, MainFrame::OnSettingsInterfaceChange)

EVT_RADIOBUTTON(ID_use_sentences, MainFrame::OnSettingsInterfaceChange)
EVT_RADIOBUTTON(ID_use_logatoms, MainFrame::OnSettingsInterfaceChange)

EVT_TEXT(ID_server_address, MainFrame::OnSettingsInterfaceChange)

EVT_COMBOBOX(ID_SELECT_SAMPLING_RATE, MainFrame::OnSettingsInterfaceChange)
EVT_COMBOBOX(ID_SELECT_MIXER_SOURCE_LINE, MainFrame::OnSettingsInterfaceChange)
EVT_COMBOBOX(ID_SELECT_MIXER_DEST_LINE, MainFrame::OnSettingsInterfaceChange)
EVT_COMBOBOX(ID_SELECT_MODULATOR_TYPE, MainFrame::OnSettingsInterfaceChange)

EVT_COMMAND_SCROLL(ID_SourceLine_SLIDER, MainFrame::OnMixerVolumeChange)
EVT_COMMAND_SCROLL(ID_DestLine_SLIDER, MainFrame::OnMixerVolumeChange)
EVT_COMMAND_SCROLL(ID_MasterLine_SLIDER, MainFrame::OnMixerVolumeChange)

// EVT_COMBOBOX(ID_SELECT_PROCESSING_TYPE, MainFrame::OnSelectProcessingType)
EVT_COMBOBOX(ID_SELECT_DRAW_MODE, MainFrame::OnSettingsInterfaceChange)
EVT_COMMAND_SCROLL(ID_PSD_SLOTS_SLIDER, MainFrame::OnPSDparamsChange)

EVT_COMMAND_SCROLL(ID_SNR_SLIDER, MainFrame::OnChannelSNRChange)
//    EVT_COMMAND_SCROLL_CHANGED(ID_SNR_SLIDER, MainFrame::OnChannelSNRChange)
//    EVT_COMMAND_SCROLL_THUMBTRACK(ID_SNR_SLIDER, MainFrame::OnChannelSNRChange)
EVT_COMMAND_SCROLL(ID_HPF_SLIDER, MainFrame::OnChannelFilterChange)
EVT_COMMAND_SCROLL(ID_LPF_SLIDER, MainFrame::OnChannelFilterChange)
EVT_COMMAND_SCROLL(ID_carrier_freq_SLIDER, MainFrame::OnCarrierFreqChange)


EVT_BUTTON(ID_send_ascii_text, MainFrame::OnButtonPress)
EVT_CHECKBOX(ID_morse_receiver_state, MainFrame::OnSettingsInterfaceChange)

EVT_CHECKBOX(ID_show_text_checkbox, MainFrame::OnSettingsInterfaceChange)
EVT_CHECKBOX(ID_modulator_state, MainFrame::OnSettingsInterfaceChange)


EVT_COMBOBOX(ID_voice_type, MainFrame::OnSelectVoiceType)
EVT_BUTTON(ID_select_voice_file, MainFrame::OnButtonPress)
EVT_BUTTON(ID_open_wav_file, MainFrame::OnButtonPress)
EVT_BUTTON(ID_stop_wav_file, MainFrame::OnButtonPress)
//    EVT_COMMAND_SCROLL_CHANGED(ID_WPM_SLIDER, MainFrame::OnWPMchange)
EVT_COMMAND_SCROLL(ID_WPM_SLIDER, MainFrame::OnWPMchange)

EVT_MENU(ID_draw_time_signal, MainFrame::OnDrawModeChange)
EVT_MENU(ID_draw_histogram, MainFrame::OnDrawModeChange)
EVT_MENU(ID_draw_psd, MainFrame::OnDrawModeChange)
EVT_MENU(ID_draw_spectrogram, MainFrame::OnDrawModeChange)
EVT_MENU(ID_draw_none, MainFrame::OnDrawModeChange)

END_EVENT_TABLE()

BEGIN_EVENT_TABLE(MyGLCanvas, wxGLCanvas)
EVT_SIZE(MyGLCanvas::OnSize)
EVT_PAINT(MyGLCanvas::OnPaint)
EVT_ERASE_BACKGROUND(MyGLCanvas::OnEraseBackground)
// EVT_KEY_DOWN( MyGLCanvas::OnKeyDown )
// EVT_KEY_UP( MyGLCanvas::OnKeyUp )
//! \Fixed Removed because it stole mouse capture from edit controls
//    EVT_ENTER_WINDOW( MyGLCanvas::OnEnterWindow )

EVT_LEFT_DOWN(MyGLCanvas::OnMouseLeftDown)
EVT_LEFT_UP(MyGLCanvas::OnMouseLeftUp)
EVT_MOTION(MyGLCanvas::OnMouseMotion)

EVT_COMMAND(ID_DrawNow, wxEVT_DRAW_NOW, MyGLCanvas::OnDrawNow)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(MyMorseKey, wxTextCtrl)
EVT_KEY_DOWN(MyMorseKey::OnKeyDown)
EVT_KEY_UP(MyMorseKey::OnKeyUp)

// EVT_LEFT_DOWN( MyMorseKey::OnMouseLeftDown )
// EVT_LEFT_UP  ( MyMorseKey::OnMouseLeftUp )
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
void MyMorseKey::OnKeyDown(wxKeyEvent &event)
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
void MyMorseKey::OnKeyUp(wxKeyEvent &event)
{
  is_down = false;
  // counter++;
  // sprintf(text, "Key Up (%i)", counter);
  sprintf(text, "Key Up");
  SetLabel(text);
  //  event.Skip();
  if (Parent->parent_task != NULL)
    if (Parent->parent_task->MorseKey_temp != NULL)
      Parent->parent_task->MorseKey_temp->SetKeyState(false);
}

// ---------------------------------------------------------------------------
// MainApp
// ---------------------------------------------------------------------------
MainFrame *MainApp::frame = NULL;
std::string MainApp::HostAddress;
// wxCriticalSection CS_OnLOG;

bool MainApp::LogFunction(const std::string &source, const std::string &message, bool IsError)
{
  // CS_OnLOG.Enter();

  if (frame != NULL)
  {
    if (source.length() > 0)
      switch (source[0])
      {
      case 0x01:
        // remove \n from the end
        if (message.length() == 0)
        {
          // CS_OnLOG.Leave();
          return false;
        }

        ((wxTextCtrl *)(frame->FindWindowById(ID_received_ascii_text)))->AppendText(message);

        // CS_OnLOG.Leave();
        return true; // stop processing this message here
        break;

      case 0x02:
        if (message.length() == 0)
        {
          // CS_OnLOG.Leave();
          return false;
        }

        switch (message[0])
        {
        case 0x01:
          // locked
          frame->FindWindowById(ID_received_ascii_text)->SetBackgroundColour(wxColour(192, 255, 192));
          frame->FindWindowById(ID_received_ascii_text)->Refresh();
          break;

        case 0x02:
          // locking
          frame->FindWindowById(ID_received_ascii_text)->SetBackgroundColour(wxColour(255, 255, 192));
          frame->FindWindowById(ID_received_ascii_text)->Refresh();
          break;

        case 0x03:
          // unlocked
          frame->FindWindowById(ID_received_ascii_text)->SetBackgroundColour(wxColour(255, 192, 192));
          frame->FindWindowById(ID_received_ascii_text)->Refresh();
          break;

        default:
        case 0x00:
          // off
          // frame->AsciiTextReceiver->Clear(); ????
          // frame->AsciiTextReceiver->SetBackgroundColour(wxColour(255,255,255));
          break;
        }
        // wxGetApp().Yield(true);

        // CS_OnLOG.Leave();
        return true;
        break;

      default:
        break;
      }
#ifdef __DEBUG__
    std::string MessageText;
    if (message.length() > 0)
      MessageText = "\n" + source + ": " + message;
    else
      MessageText = "\n" + source;

    // frame->SetStatusBoxMessage(message,IsError);
    //  Send event to GUI thread
    wxCommandEvent event(wxEVT_STATUSBOX_UPDATE, ID_StatusBox_AppendText);
    if (IsError == true)
    {
      event.SetInt(1);
    }
    else
    {
      event.SetInt(0);
    }
    event.SetString(MessageText);
    // frame->GetEventHandler()->AddPendingEvent(event);
    wxPostEvent(frame, event);
#endif
  }

  // CS_OnLOG.Leave();
  return false;
}

#ifdef __DEBUG__
void T_TaskElement::OnStatusBoxUpdate(wxCommandEvent &event)
{
  bool IsError = (event.GetInt() == 1);
  std::string MessageText = event.GetString().ToStdString();
  task_parent_window->SetStatusBoxMessage(MessageText, IsError);
}
#endif // __DEBUG__

// Initialize this in OnInit, not statically
bool MainApp::OnInit()
{
  // char temp_str[1024];

  // Create the main frame window
#ifdef __DEBUG__
  DSP::log.SetLogState(DSP::e::LogState::file | DSP::e::LogState::user_function);
  DSP::log.SetLogFileName("log_file.log");
  DSP::log.SetLogFunctionPtr(&(MainApp::LogFunction));
  DSP::log << DSP::lib_version_string() << std::endl
           << std::endl;
#else
  DSP::log.SetLogState(DSP::e::LogState::user_function);
  // DSP::f::SetLogFileName("log_file.log");
  DSP::log.SetLogFunctionPtr(&(MainApp::LogFunction));
  DSP::log << DSP::lib_version_string() << std::endl
           << std::endl;
#endif

#ifdef GLUT_API_VERSION
  glutInit(&argc, argv);
#endif
  wxInitAllImageHandlers();
  // http://tangentsoft.net/wskfaq/examples/ipaddr.html
  struct hostent *remoteHost;
  WSADATA wsaData;
  char HostName[1024];
  // struct in_addr addr;
  int iResult;

  iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
  gethostname(HostName, 1024);
  remoteHost = gethostbyname(HostName);
  // addr.s_addr = inet_addr("127.0.0.1");
  // remoteHost = gethostbyaddr((char *) &addr, 4, AF_INET);
  //  h_addr_list[ind] != 0x00// h_addr_list[0] <- ip adress
  struct in_addr addr;
  memcpy(&addr, remoteHost->h_addr_list[0], sizeof(struct in_addr));
  HostAddress = inet_ntoa(addr);

  WSACleanup();

  frame = new MainFrame((wxFrame *)NULL, wxID_ANY, _T("TeleSound ver. 1.1 (Marek.Blok@eti.pg.edu.pl)"),
                        wxDefaultPosition, wxSize(800, 600),
                        wxDEFAULT_FRAME_STYLE); //  | wxHSCROLL | wxVSCROLL);
  // strcpy(frame->HostAddress, HostAddress);
  frame->SetIcon(wxICON(TelesoundMainIcon));

  SetTopWindow(frame);
  frame->Show(true);

  return true;
}

// ---------------------------------------------------------------------------
// T_ProcessingSpec
// ---------------------------------------------------------------------------
T_ProcessingSpec::T_ProcessingSpec(void)
{
  run_as_server = true;
  IP_address = "127.0.0.1";
  SamplingRate = 16000; // 44100;

  no_of_psd_slots = 200;

  time_span = 2.0;
  WPM = 20;
  SNR_dB = 80;

  MikeIsOff = false;

  ChannelFilterON = false;
  ChannelFd = 0;
  ChannelFg = 0;
  modulator_state = false;
  modulator_type= E_MT_PSK;
  carrier_freq = 4000;
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
{
  return Next;
}

// ---------------------------------------------------------------------------
// T_TaskElement
// ---------------------------------------------------------------------------
// Class storing task list element data
T_TaskElement *T_TaskElement::First = NULL;
MainFrame *T_TaskElement::task_parent_window = NULL;
int T_TaskElement::NoOfTasks = 0;
int T_TaskElement::NewTaskID = 1; // start from 1

T_TaskElement::T_TaskElement(MainFrame *parent)
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
  // task_MDI_window = NULL;
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
        DSP::log << DSP::e::LogMode::Error << "T_TaskElement::~T_TaskElement" << DSP::e::LogMode::second << "NoOfTasks was already 0" << std::endl;
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
            DSP::log << DSP::e::LogMode::Error << "T_TaskElement::~T_TaskElement" << DSP::e::LogMode::second << "NoOfTasks was already 0" << std::endl;
          break;
        }

        current_task = current_task->Next;
      }

      if (current_task == NULL)
      {
        DSP::log << DSP::e::LogMode::Error << "T_TaskElement::~T_TaskElement" << DSP::e::LogMode::second << "delete task was not on the list" << std::endl;
      }
    }
  }
  else
  {
    DSP::log << DSP::e::LogMode::Error << "T_TaskElement::~T_TaskElement" << DSP::e::LogMode::second << "tasks list already was empty" << std::endl;
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
    delete[] input_file_info;
    input_file_info = NULL;
  }
  if (BranchFinished_semaphore != NULL)
  {
    delete BranchFinished_semaphore;
    BranchFinished_semaphore = NULL;
  }
}

void T_TaskElement::OnBranchEnd(wxCommandEvent &event)
{
  if (task_parent_window != NULL)
  {
    // wxCommandEvent event( wxMenuEvent, ID_STOP_TASK );
    wxCommandEvent event2(wxEVT_COMMAND_BUTTON_CLICKED, ID_STOP_TASK);
    // wxCommandEvent event( wxEVT_COMMAND_MENU_SELECTED, ID_STOP_TASK );
    // task_parent_window->GetEventHandler()->AddPendingEvent( event2 );
    wxPostEvent(task_parent_window, event2);
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
{
  return task_name;
}

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
  MorseKey_temp = NULL;
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
    DSP::log << "T_TaskElement::StopTaskProcessing" << DSP::e::LogMode::second << "PostCommandToBranch" << std::endl;
#endif
    ProcessingBranch->PostCommandToBranch(temp);
    ProcessingBranch = NULL; // branch will be deleted in the processing thread
    // ++++++++++++++++++++++++
    // Wait for branch finish
    task_is_running = false;
    DSP::log << DSP::e::LogMode::Error << "T_TaskElement::StopTaskProcessing" << DSP::e::LogMode::second << "Waiting for thread branch to finish" << std::endl;
    // BranchFinished_semaphore->Wait();
    while (BranchFinished_semaphore->TryWait() == wxSEMA_BUSY)
    {
      wxGetApp().Yield(true);
    }
    DSP::log << DSP::e::LogMode::Error << "T_TaskElement::StopTaskProcessing" << DSP::e::LogMode::second << "!!! Thread branch finished" << std::endl;
  }
  task_is_running = false;
  task_is_paused = false;

  // ++++++++++++++++++++++++++++++++++++++++++++++ //
  // 3. destroy processing branch (branch is destroyed in the processing thread ???)

  // ++++++++++++++++++++++++++++++++++++++++++++++ //
  // 4. close MDI window

  // ++++++++++++++++++++++++++++++++++++++++++++++ //
  // 5. update GUI
  // if (task_parent_window != NULL)
  //  task_parent_window->UpdateGUI();

  //! \todo implement task deletion
  //  delete MDI_parent_task;
  //  DSP::log << DSP::e::LogMode::Error <<"MyChild::OnClose"<< DSP::e::LogMode::second << "MDI_parent_task deleted"<< std::endl;
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
        DSP::log << "T_TaskElement::PauseTaskProcessing" << DSP::e::LogMode::second << "PostCommandToBranch" << std::endl;
#endif
        ProcessingBranch->PostCommandToBranch(temp);
      }
      else
      {
        task_is_paused = false;

        temp = new T_BranchCommand(E_BC_continue);
#ifdef __DEBUG__
        DSP::log << DSP::e::LogMode::Error << "T_TaskElement::PauseTaskProcessing" << DSP::e::LogMode::second << "PostCommandToBranch" << std::endl;
#endif
        ProcessingBranch->PostCommandToBranch(temp);
      }
    }

    // +++++++++++++++++++++++++++++++++++++++++++++ //
    // 3. update GUI
    // if (task_parent_window != NULL)
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
    ind++;
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

  // sampling_rate = 8000;
  sampling_rate = 16000; // 44100;
  WPM = 20;
  SNR_dB = 80;

  channel_filter_ON = false;
  channel_Fd = 0;
  channel_Fg = sampling_rate / 2;

  ascii_text = "";
  morse_receiver_state = false;
  modulator_state = false;
  modulator_type = E_MT_PSK;
  carrier_freq = sampling_rate/4;
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
  // do_transfer |= (sampling_rate != selected_task->GetSamplingRate());

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

    do_transfer |= (modulator_state != selected_task->FirstProcessingSpec->modulator_state);
    do_transfer |= (modulator_type != selected_task->FirstProcessingSpec->modulator_type);
    do_transfer |= (carrier_freq != selected_task->FirstProcessingSpec->carrier_freq);
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
        // do_transfer |= (F_L_spec != selected_task->FirstProcessingSpec->F_L);
        // do_transfer |= (F_U_spec != selected_task->FirstProcessingSpec->F_U);
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
    selected_task->FirstProcessingSpec->modulator_state = modulator_state;
    selected_task->FirstProcessingSpec->modulator_type = modulator_type;
    selected_task->FirstProcessingSpec->carrier_freq = carrier_freq;
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
      // selected_task->FirstProcessingSpec->Set_F_L(F_L_spec);
      // selected_task->FirstProcessingSpec->Set_F_U(F_U_spec);
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
// MainFrame
// ---------------------------------------------------------------------------

MainFrame::~MainFrame(void)
{
  if (fileManager != NULL)
  {
    delete fileManager;
    fileManager = NULL;
  }
  if (AudioMixer != NULL)
  {
    AudioMixer->RestoreMixerSettings_WAVEIN();
    AudioMixer->RestoreMixerSettings_OUT();
    delete AudioMixer;
    AudioMixer = NULL;
  }
  if (KeyingCtrl != NULL)
  {
    delete KeyingCtrl;
    KeyingCtrl = NULL;
  }
}
// Define my frame constructor
MainFrame::MainFrame(wxWindow *parent,
                     const wxWindowID id,
                     const wxString &title,
                     const wxPoint &pos,
                     const wxSize &size,
                     const long style)
    : GUIFrame(parent, id, title, pos, size, style)
{
  frame_is_closing = false;
  task_is_stopping_now = false;

  int waveInDevNumber = TAudioMixer::GetNoOfWaveInDevices();
  int waveOutDevNumber = TAudioMixer::GetNoOfWaveOutDevices();

  if (waveInDevNumber == 0 || waveOutDevNumber == 0)
  {
    std::string ErrorMessage = "Error: ";
    if (waveInDevNumber == 0)
    {
      ErrorMessage += "\nNo input audio device found.";
    }
    if (waveOutDevNumber == 0)
    {
      ErrorMessage += "\nNo output audio device found.";
    }

    DSP::log << DSP::e::LogMode::Error << ErrorMessage << std::endl;
    wxMessageBox(wxString::FromUTF8("Sprawdź połączenie głośników i mikrofonu, następnie uruchom program ponownie. \n\n") + ErrorMessage, wxString::FromUTF8("Błąd urządzeń we/wy"), wxOK | wxICON_ERROR);
    exit(1);
  }
  //Check if config folder and all required files exist.
  bool configFileError = false;
  const std::vector<fs::path> requiredFolders = {"./config","./matlab"};
  const std::vector<fs::path> requiredFiles = {"./config/Polish.mct","./matlab/srRC_stage1.coef","./matlab/srRc_stage2.coef"};

    for (const fs::path& folder : requiredFolders) {//loop through folder list
      if (!fs::exists(folder)) {
        configFileError = true;
        wxMessageBox(wxString::FromUTF8("Nie znaleziono katalogu " + folder.filename().string() + " zawierającego pliki wymagane do poprawnej pracy programu."), wxString::FromUTF8("Błąd"), wxOK | wxICON_ERROR);   
    };}

  if(!configFileError){
    //loop through file list and check for existence
     for (const fs::path& file : requiredFiles) {
      if(!fs::exists(file)){
        wxMessageBox(wxString::FromUTF8("Nie znaleziono pliku " + file.filename().string() + " wymaganego do poprawnej pracy programu."), wxString::FromUTF8("Błąd"), wxOK | wxICON_ERROR);
        configFileError = true;
      }
     }
  };

  AudioMixer = new TAudioMixer;
  AudioMixer->MemorizeMixerSettings_WAVEIN();
  AudioMixer->MemorizeMixerSettings_OUT();

  parent_task = NULL;
  MyProcessingThread::CreateAndRunThreads(this, 1); // tworzymy tylko jeden wątek, bo pozostałych i tak nie wykorzystywano

  FillSettingsInterface(NULL);
  UpdateGUI();

  // Create Voice File Manager object, lock the file selection if the directories do not exist
  try
  {
    fileManager = new VoiceFileManager("audio_wzor_44100", "Logatomy", "Zdania");
    DSP::log << DSP::e::LogMode::Info << "VoiceFileManager" << DSP::e::LogMode::second << "created successfully" << std::endl;
    std::vector<wxString> voiceTypes = fileManager->listVoiceTypes(VoiceFileTypes::Logatoms);
    voiceTypes.push_back(_T("losowy"));
    VoiceTypeBox->Set(voiceTypes);
    VoiceTypeBox->Select(voiceTypes.size() - 1);
  }
  catch (std::invalid_argument const &e)
  {
    fileManager=nullptr;
    DSP::log << DSP::e::LogMode::Error << "VoiceFileManager" << DSP::e::LogMode::second << e.what() << "; disabling GUI elements. " << std::endl;
    UseLogatoms->Disable();
    UseSentences->Disable();
    VoiceTypeBox->Disable();
    VoiceFileIndex->Disable();
    SelectVoiceFile->Disable();
    OpenWAVEfile->Disable();
    StopWAVEfile->Disable();
  }

  // Accelerators
  wxAcceleratorEntry entries[8];
  entries[0].Set(wxACCEL_CTRL, (int)'P', ID_PAUSE_TASK);
  entries[1].Set(wxACCEL_CTRL, (int)'M', ID_MIKE_ON_OFF);
  entries[2].Set(wxACCEL_CTRL, (int)'L', ID_LOCAL_SIGNAL_ON_OFF);
  entries[3].Set(wxACCEL_CTRL, (int)'1', ID_draw_time_signal);
  entries[4].Set(wxACCEL_CTRL, (int)'2', ID_draw_histogram);
  entries[5].Set(wxACCEL_CTRL, (int)'3', ID_draw_psd);
  entries[6].Set(wxACCEL_CTRL, (int)'4', ID_draw_spectrogram);
  entries[7].Set(wxACCEL_CTRL, (int)'0', ID_draw_none);
  wxAcceleratorTable accel(8, entries);
  SetAcceleratorTable(accel);

  float val;
  int Active;
  DWORD mixer_state;

  Active = -1;
  SourceLine_ComboBox->Clear();
  for (int ind = 0; ind < AudioMixer->GetNumberOfSourceLines(); ind++)
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
    Active = AudioMixer->GetActiveSourceLine();
  AudioMixer->SetActiveSourceLine(Active);
  SourceLine_ComboBox->SetValue(AudioMixer->GetSourceLineName(Active));

  val = AudioMixer->GetSourceLineVolume(Active);
  SourceLine_slider->SetValue((val * MAX_SLIDER_VALUE));
  if (val < 0)
    SourceLine_slider->Enable(false);

  DestLine_ComboBox->Clear();
  Active = -1;
  for (int ind = 0; ind < AudioMixer->GetNumberOfDestLines(); ind++)
  {
    // mixer_state = AudioMixer->GetDestLineState(ind);
    // if (mixer_state == AM_MUTED_NO)
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
  for (int ind = 0; ind < AudioMixer->GetNumberOfDestLines(); ind++)
  {
    if (ind != Active)
      AudioMixer->SetDestLineState(ind, DSP::e::AM_MutedState::MUTED_YES);
    else
      AudioMixer->SetDestLineState(ind, DSP::e::AM_MutedState::MUTED_NO);
  }
  val = AudioMixer->GetDestLineVolume(Active);
  DestLine_slider->SetValue((int)val * MAX_SLIDER_VALUE);
  if (val < 0)
    DestLine_slider->Enable(false);
  val = AudioMixer->GetDestLineVolume(DSP::AM_MasterControl);
  MasterLine_slider->SetValue((int)val * MAX_SLIDER_VALUE);
  if (val < 0)
    MasterLine_slider->Enable(false);
  AudioMixer->SetDestLineState(DSP::AM_MasterControl, DSP::e::AM_MutedState::MUTED_NO);
  // Replace placeholder with morsekey
  KeyingCtrl = new MyMorseKey(this, page3, wxID_ANY, _T("Key OFF"));
  sizer_18->Replace(TextCtrlPlaceholder, KeyingCtrl);
  delete TextCtrlPlaceholder;
  sizer_18->Layout();

#ifndef __DEBUG__
  // remove log page in release version
  notebookWindow->RemovePage(3);
#endif
  // TODO: validate ip (regex?) after 'processing start' button is pressed.
  // allow only numbers and dots
  wxString allowedChars = "0123456789.";
  wxTextValidator IPValidator(wxFILTER_INCLUDE_CHAR_LIST);
  IPValidator.SetCharIncludes(allowedChars);
  ServerAddressEdit->SetValidator(IPValidator);
}

MyGLCanvas *MainFrame::GetGLcanvas(unsigned int CanvasInd)
{
  return GLcanvas;
}

T_TaskElement *MainFrame::GetParentTask(void)
{
  return parent_task;
}

void MainFrame::UpdateGUI(void)
{
  unsigned int ind;
  wxNotebookPage *tmp_page;
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
    // FileParams_sizer->Show(FileParams_sizer, true, true);
    // FileParams_sizer->Layout();

    // update toolbar
    if (interface_state.task_is_running == true)
    {
      // TODO: move to updategui (?)
      RunProcessingButton->Enable(false);
      PauseProcessingButton->Enable(true);
      PauseProcessingButton->SetToolTip(_T("Wstrzymaj przetwarzanie"));
      PauseProcessingButton->SetBitmap(wxBitmap(pause_xpm));
      if (interface_state.task_is_paused == true)
      {
        PauseProcessingButton->SetToolTip(_T("Wznów przetwarzanie"));
        PauseProcessingButton->SetBitmap(wxBitmap(resume_xpm));
        StopProcessingButton->Enable(false);
      }
      else
        StopProcessingButton->Enable(true);

      SamplingRateBox->Enable(false);
      PSD_slots_text->Enable(false);
      PSD_slots_slider->Enable(false);
    }
    else
    {
      RunProcessingButton->Enable(true);
      PauseProcessingButton->Enable(false);
      StopProcessingButton->Enable(false);

      SamplingRateBox->Enable(true);
      PSD_slots_text->Enable(true);
      PSD_slots_slider->Enable(true);
    }

    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
    // Enable / disable controls based on interface_state
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++  //
    //  General settings page
    if (interface_state.run_as_server == true)
    {
      WorksAsServer->SetValue(true);
      WorksAsClient->SetValue(false);
      // ServerAddressEdit->ChangeValue(interface_state.address);
      ServerAddressEdit->ChangeValue(MainApp::HostAddress);
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
}
void MainFrame::FillSettingsInterface(T_TaskElement *selected_task)
{
  if (selected_task == NULL)
  { // clear settings
    WorksAsServer->SetValue(true);
    WorksAsClient->SetValue(false);
    ServerAddressEdit->ChangeValue("127.0.0.1");
    // ConnectionInfo->SetLabel("");
  }
  else
  { // fill settings
    if (interface_state.run_as_server == true)
    {
      WorksAsServer->SetValue(true);
      WorksAsClient->SetValue(false);
      ServerAddressEdit->ChangeValue(MainApp::HostAddress);
      ServerAddressEdit->Enable(false);
    }
    else
    {
      WorksAsServer->SetValue(false);
      WorksAsClient->SetValue(true);
      ServerAddressEdit->ChangeValue(interface_state.address);
      ServerAddressEdit->Enable(true);
    }
    // ConnectionInfo->SetLabel("");

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

  // SamplingRateEdit->SetValue(wxString::Format("%lld", (long long)(selected_task->GetSamplingRate())));
  SamplingRateBox->SetValue(wxString::Format("%ld", interface_state.sampling_rate));

  wxScrollEvent event;
  event.SetId(ID_SELECT_SAMPLING_RATE);
  OnChannelFilterChange(event);
  // LPF_slider->SetValue((int)(interface_state.channel_Fg/100));
  // HPF_slider->SetValue((int)(interface_state.channel_Fd/100));
  wxScrollEvent temp_event;
  temp_event.SetId(ID_SELECT_SAMPLING_RATE);
  OnChannelFilterChange(temp_event);
}

void MainFrame::OnClose(wxCloseEvent &event)
{
  DSP::log << "MainFrame::OnClose" << DSP::e::LogMode::second << "start" << std::endl;
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

  DSP::log << "MainFrame::OnClose" << DSP::e::LogMode::second << "Calling FreeThreads" << std::endl;

  MyProcessingThread::FreeThreads();

  DSP::log << "MainFrame::OnClose" << DSP::e::LogMode::second << "Finished FreeThreads" << std::endl;

  event.Skip();
}

void MainFrame::OnQuit(wxCommandEvent &WXUNUSED(event))
{
  Close();
}

void MainFrame::OnAbout(wxCommandEvent &WXUNUSED(event))
{
  std::string text, text2;

  text = DSP::lib_version_string();
  text2 = "TeleSound 2020\n\n";
  text2 += "Author: Marek Blok (c) 2020\n\n";
  text2 += text;

  wxMessageBox(text2,
               _T("About Zoom Analyzer"));
}

void MainFrame::OnRunTask(wxCommandEvent &event)
{
  if (parent_task == NULL)
  {
    if (task_is_stopping_now == true)
      return;

    if (interface_state.task_is_running == true)
    {
      DSP::log << DSP::e::LogMode::Error << "MainFrame::OnRunTask" << DSP::e::LogMode::second << "Task is still running" << std::endl;
      return;
    }
    parent_task = new T_TaskElement(this);
    interface_state.TransferDataToTask(NULL, parent_task, false); // do not ask user
    interface_state.task_is_running = true;
    interface_state.task_is_paused = false;
    parent_task->RunTaskProcessing();

    // Disable switching between client/server modes when processing starts.
    WorksAsServer->Disable();
    WorksAsClient->Disable();
    ServerAddressEdit->Disable();
    ResetSettingsButton->Disable();
  }
}

void MainFrame::OnPauseTask(wxCommandEvent &event)
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
      // parent_task->ProcessingBranch->ComputeHighResolutionSpectorgram();
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

void MainFrame::OnStopTask(wxCommandEvent &event)
{
  if (parent_task != NULL)
  {
    if (task_is_stopping_now == true)
      return;
    task_is_stopping_now = true;
    // Enable(false);

    parent_task->StopTaskProcessing();
    //! \bug should wait until task finishes
    parent_task = NULL;

    interface_state.task_is_running = false;
    interface_state.task_is_paused = false;

    task_is_stopping_now = false;
    // Enable(true);
    UpdateGUI();

    GetGLcanvas(0)->Refresh(); // invalidate window
    GetGLcanvas(0)->Update();  // refresh canvas immediately
  }

  // Enable switching between client/server modes when processing stops.
  WorksAsServer->Enable();
  WorksAsClient->Enable();
  if (WorksAsClient->GetValue())
  {
    ServerAddressEdit->Enable();
  }
  ResetSettingsButton->Enable();
}

void MainFrame::OnPageChanging(wxNotebookEvent &event)
{
  int page_no;
  wxNotebookPage *current_page;

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
// TODO: check
void MainFrame::SetStatusBoxMessage(std::string MessageText, bool isError)
{
#ifdef __DEBUG__
  if (StatusBox != NULL)
  {
    if (isError)
      StatusBox->SetDefaultStyle(wxTextAttr(*wxRED));
    else
      StatusBox->SetDefaultStyle(wxTextAttr(*wxBLACK));

    StatusBox->AppendText(MessageText);
  }
#endif
}

void MainFrame::OnProcessEnd(wxCommandEvent &event)
{
  DSP::log << DSP::e::LogMode::Error << "MainFrame::OnProcessEnd" << std::endl;
}

void MainFrame::OnSettingsInterfaceChange(wxCommandEvent &event)
{
  wxString t_address;
  long temp_long;
  bool refresh_GLcanvas;

  refresh_GLcanvas = false;
  switch (event.GetId())
  {
  case ID_work_as_server:
    interface_state.run_as_server = true;
    // get address from edit window
    t_address = ServerAddressEdit->GetValue();
    if (t_address.compare(MainApp::HostAddress) != 0)
      interface_state.address = t_address;
    break;
  case ID_work_as_client:
    interface_state.run_as_server = false;
    // get address from edit window
    t_address = ServerAddressEdit->GetValue();
    if (t_address.compare(MainApp::HostAddress) != 0)
      interface_state.address = t_address;
    break;
  case ID_server_address:
    if (ServerAddressEdit == NULL)
      return; // this is only text control initialization
    if (interface_state.run_as_server == false)
    {
      // get address from edit window
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
      SourceLine_slider->SetValue((int)(val * MAX_SLIDER_VALUE));
    }
  }
  break;
  case ID_SELECT_MIXER_DEST_LINE:
  {
    int Active;
    float val;

    Active = DestLine_ComboBox->GetSelection();
    for (int ind = 0; ind < AudioMixer->GetNumberOfDestLines(); ind++)
    {
      if (ind != Active)
        AudioMixer->SetDestLineState(ind, DSP::e::AM_MutedState::MUTED_YES);
      else
        AudioMixer->SetDestLineState(ind, DSP::e::AM_MutedState::MUTED_NO);
    }
    val = AudioMixer->GetDestLineVolume(Active);
    if (val < 0)
      DestLine_slider->Enable(false);
    else
    {
      DestLine_slider->Enable(true);
      DestLine_slider->SetValue((int)(val * MAX_SLIDER_VALUE));
    }
  }
  break;

  case ID_SELECT_DRAW_MODE:
    /*! \todo_later Add all sizers to panel before application close
     *   to make sure that all sizer will be cleared properly
     */
    {
      int val;
      // wxSizer *sizerPanel;
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
      MicStateButton->SetBitmap(wxBitmap(mike_on_xpm));
      MicStateButton->SetToolTip(_T("Status mikrofonu (włączony)"));
    }
    else
    {
      interface_state.mike_is_off = true;
      MicStateButton->SetBitmap(wxBitmap(mike_off_xpm));
      MicStateButton->SetToolTip(_T("Status mikrofonu (wyłączony)"));
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
        DSP::log << "ID_MIKE_ON_OFF" << DSP::e::LogMode::second << "PostCommandToBranch" << std::endl;
#endif
        parent_task->ProcessingBranch->PostCommandToBranch(temp);
      }
    }
    break;
  case ID_RESET_SETTINGS:
  interface_state.Reset();
  FillSettingsInterface(NULL);
  UpdateGUI();
  //InitGUI();
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
          AsciiTextReceiver->SetBackgroundColour(wxColour(255, 255, 255));

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
      // TODO: Move this to UpdateGUI?
      LocalSignalStateButton->SetBitmap(wxBitmap(local_on_xpm));
      LocalSignalStateButton->SetToolTip(_T("Sygnał lokalny (włączony)"));
    }
    else
    {
      interface_state.local_signal_gain = 0.0;
      LocalSignalStateButton->SetBitmap(wxBitmap(local_off_xpm));
      LocalSignalStateButton->SetToolTip(_T("Sygnał lokalny (wyłączony)"));
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
        DSP::log << "ID_LOCAL_SIGNAL_ON_OFF" << DSP::e::LogMode::second << "PostCommandToBranch" << std::endl;
#endif
        parent_task->ProcessingBranch->PostCommandToBranch(temp);
      }
    }
    break;
  case ID_modulator_state:
    interface_state.modulator_state = ModulatorState->GetValue();

    if (parent_task != NULL)
    {
      if (parent_task->ProcessingBranch != NULL)
      {
        T_BranchCommand *temp;
        TCommandData *command_data;

        interface_state.userdata_state = E_US_modulator_state;
        command_data = new TCommandData;
        command_data->UserData = (void *)(&interface_state);
        temp = new T_BranchCommand(E_BC_userdata, command_data);
#ifdef __DEBUG__
        DSP::log << "ID_MODULATOR_ON_OFF" << DSP::e::LogMode::second << "PostCommandToBranch" << std::endl;
#endif
        parent_task->ProcessingBranch->PostCommandToBranch(temp);
      }
    }
    break;
  case ID_SELECT_MODULATOR_TYPE:
      interface_state.modulator_type = (E_ModulatorTypes)(ModulationTypeBox->GetSelection());
      {
      wxCommandEvent evt,evt2;
      //TODO: Add a slight delay between stop and start, so that when the client changes modulator type, the server has enough time to stop processing?  
      UpdateModulatorParametersText();
      evt.SetId(ID_STOP_TASK);
      OnStopTask(evt);
      evt2.SetId(ID_RUN_TASK);
      OnRunTask(evt);
      }
      break;
  case ID_use_logatoms:
    SentenceTranscription->ChangeValue("");
    VoiceFileIndex->ChangeValue("");
    if (fileManager != NULL)
    {
      wxString currentSelection = VoiceTypeBox->GetStringSelection();
      std::vector<wxString> voiceTypes = fileManager->listVoiceTypes(VoiceFileTypes::Logatoms);
      voiceTypes.push_back(_T("losowy"));
      VoiceTypeBox->Set(voiceTypes);
      int index = VoiceTypeBox->FindString(currentSelection);
      if (index != wxNOT_FOUND)
        VoiceTypeBox->Select(index);
      else
        VoiceTypeBox->Select(voiceTypes.size() - 1);
    }
    break;
  case ID_use_sentences:
  {
    SentenceTranscription->ChangeValue("");
    VoiceFileIndex->ChangeValue("");
    if (fileManager != NULL)
    {
      wxString currentSelection = VoiceTypeBox->GetStringSelection();
      std::vector<wxString> voiceTypes = fileManager->listVoiceTypes(VoiceFileTypes::Sentences);
      voiceTypes.push_back(_T("losowy"));
      VoiceTypeBox->Set(voiceTypes);
      int index = VoiceTypeBox->FindString(currentSelection);
      if (index != wxNOT_FOUND)
        VoiceTypeBox->Select(index);
      else
        VoiceTypeBox->Select(voiceTypes.size() - 1);
    }
    break;
  }
  case ID_show_text_checkbox:
    SentenceTranscription->Show(showSentenceText->GetValue());
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

void MyGLCanvas::OnDrawNow(wxCommandEvent &event)
{
  OnDrawNow_();
}

bool MyGLCanvas::BlockDrawing = true;
bool MyGLCanvas::temporary_BlockDrawing = true;

void MyGLCanvas::OnDrawNow_(void)
{
  DSP::e::SocketStatus status;
  E_DrawModes current_mode;

  if (temporary_BlockDrawing == true)
  {
#ifdef __DEBUG__
    DSP::log << DSP::e::LogMode::Error << "Drawing block (temporary)- skipping" << std::endl;
#endif
    return;
  }
  if (BlockDrawing == true)
  {
#ifdef __DEBUG__
    DSP::log << DSP::e::LogMode::Error << "Drawing block - clearing" << std::endl;
#endif
    current_mode = E_DM_none;
    // return;
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

  // if (current_mode != E_DM_none) {
  if (BlockDrawing == false)
  {
    if (T_DSPlib_processing::CurrentObject != NULL)
    {
      if (T_DSPlib_processing::CurrentObject->GraphInitialized == false)
      {
        long double Fp;
        unsigned int samples_per_PSD;

        Fp = T_DSPlib_processing::CurrentObject->Fp;
        samples_per_PSD = T_DSPlib_processing::CurrentObject->BufferStep;
        T_DSPlib_processing::CurrentObject->PSDs->InitialiseSpectrogram(
            0.0, Fp / 2, Fp / 2, samples_per_PSD, Fp, true);

        T_DSPlib_processing::CurrentObject->GraphInitialized = true;
      }
      current_mode = DrawMode;

      SocketsAreConnected = true;
      status = T_DSPlib_processing::CurrentObject->out_socket->GetSocketStatus();
      if (((int)status & (int)DSP::e::SocketStatus::connected) == 0)
        SocketsAreConnected = false;
      status = T_DSPlib_processing::CurrentObject->in_socket->GetSocketStatus();
      if (((int)status & (int)DSP::e::SocketStatus::connected) == 0)
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
    // glScissor (0,0, w, h);
    glDisable(GL_SCISSOR_TEST);

    // glClearColor(1.0,1.0,1.0,0);
    if (SocketsAreConnected == true)
    {
      glClearColor(1.0, 1.0, 1.0, 0);
      // DSP::log << "Connected"<< std::endl;
    }
    else
    {
      glClearColor(1.0, 1.0, 0.0, 0);
      // DSP::log << "Not connected"<< std::endl;
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

  xf = x;
  yf = y;
  //! \bug also get units strings
  in_subplot = GetCords(T_DSPlib_processing::CurrentObject, xf, yf);
  if (in_subplot == true)
  {
    sprintf(tekst, "1. x = %i, y = %i, xf = %.2f , yf = %.2f", x, y, xf, yf);
    GetGrandParent()->SetLabel(tekst);
  }
  else
  {
    sprintf(tekst, "2. x = %i, y = %i, xf = %.2f , yf = %.2f", x, y, xf, yf);
    GetGrandParent()->SetLabel(tekst);
  }
  return in_subplot;
}

bool MyGLCanvas::OnMouseUp(int x, int y)
{
  // char tekst[1024];
  float xf, yf;
  bool in_subplot;

  if (T_DSPlib_processing::CurrentObject == NULL)
    return false;

  xf = x;
  yf = y;
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

  xf = x;
  yf = y;
  //! \bug also get units strings
  in_subplot = GetCords(T_DSPlib_processing::CurrentObject, xf, yf);

  if (x_label != NULL)
  {
    if (in_subplot == true)
    {
      // sprintf(tekst, "1. x = %i, y = %i, xf = %.2f , yf = %.2f", x, y, xf, yf);
      sprintf(tekst, "%s = %.2f%s, %s = %.2f%s",
              x_label, xf, x_units,
              y_label, yf, y_units);
      GetGrandParent()->SetLabel(tekst);
    }
    else
    {
      // sprintf(tekst, "2. x = %i, y = %i, xf = %.2f , yf = %.2f", x, y, xf, yf);
      sprintf(tekst, "%s = %.2f%s, %s = %.2f%s",
              x_label, xf, x_units,
              y_label, yf, y_units);
      GetGrandParent()->SetLabel(tekst);
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
    x_label = const_cast<char *>(psd_x_label);
    x_units = const_cast<char *>(psd_x_units);
    y_label = const_cast<char *>(psd_y_label);
    y_units = const_cast<char *>(psd_y_units);

    Object->PSDs->GetSubPlotCords(1, 1, 1, x_in, y_in);
    in_subplot = ((fabs(x_in) > 1.0) && (fabs(y_in) > 1.0));

    // x_in = (x_in+1)/2 * Object->Fp / 2;
    x_in = (x_in + 1) * Object->Fp / 4;
    // PSD_max_dB, PSD_range_dB;
    y_in = PSD_max_dB + (y_in - 1) / 2 * PSD_range_dB;
    break;

  case E_DM_spectrogram:
    x_label = const_cast<char *>(spec_x_label);
    x_units = const_cast<char *>(spec_x_units);
    y_label = const_cast<char *>(spec_y_label);
    y_units = const_cast<char *>(spec_y_units);

    Object->PSDs->GetSubPlotCords(1, 1, 1, x_in, y_in);
    in_subplot = ((fabs(x_in) > 1.0) && (fabs(y_in) > 1.0));

    // y_in = (y_in+1)/2 * Object->Fp / 2;
    y_in = (y_in + 1) * Object->Fp / 4;
    x_in = (x_in - 1) / 2 * Object->specgram_time_span;
    break;

  case E_DM_histogram:
    x_label = const_cast<char *>(hist_x_label);
    x_units = const_cast<char *>(hist_x_units);
    y_label = const_cast<char *>(hist_y_label);
    y_units = const_cast<char *>(hist_y_units);

    Object->Histograms->GetSubPlotCords(1, 1, 1, x_in, y_in);
    in_subplot = ((fabs(x_in) > 1.0) && (fabs(y_in) > 1.0));

    y_in = (y_in + 1) / 2; // * (Object->NoOfHistBins * Object->BufferStep);
    // x_in = (x_in-1)/2 * Object->specgram_time_span;
    break;

  case E_DM_signal:
    x_label = const_cast<char *>(sig_x_label);
    x_units = const_cast<char *>(sig_x_units);
    y_label = const_cast<char *>(sig_y_label);
    y_units = const_cast<char *>(sig_y_units);

    Object->SignalSegments->GetSubPlotCords(1, 1, 1, x_in, y_in);
    in_subplot = ((fabs(x_in) > 1.0) && (fabs(y_in) > 1.0));

    // y_in = (y_in+1) * Object->Fp / 4;
    x_in = (x_in - 1) / 2 * Object->specgram_time_span;
    break;

  case E_DM_none:
  default:
    x_label = NULL;
    x_units = NULL;
    y_label = NULL;
    y_units = NULL;
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
          0.0, T_DSPlib_processing::CurrentObject->Fp / 2,
          T_DSPlib_processing::CurrentObject->CurrentObject->Fp / 2,
          T_DSPlib_processing::CurrentObject->NoOfSamplesPerAPSD,
          T_DSPlib_processing::CurrentObject->Fp, true);
    }
    else
    {
      temp_plot_stack = T_DSPlib_processing::CurrentObject->PSDs;
      segment_size = T_DSPlib_processing::CurrentObject->PSD_size;
    }

    // clear whole axis field
    // T_DSPlib_processing::CurrentObject->PSDs->SubPlot(1,1,-1, 0, 0, true);
    if (SocketsAreConnected == true)
      temp_plot_stack->SetBackgroundColor(0.4, CLR_gray);
    else
      temp_plot_stack->SetBackgroundColor(1.0, 1.0, 0.0);
    temp_plot_stack->SubPlot(1, 1, -1, width, height, true);
    //  void SubPlot(int Rows, int Cols, int index, int WindowW, int WindowH, bool Clear);
    // ++++++++++++++++++++++++++++++++++++++++ //

    // select axes plot field for drawing
    temp_plot_stack->SubPlot(1, 1, 1, width, height, false);
    temp_plot_stack->DrawSpecgram2_dB(PSD_max_dB, PSD_range_dB, CLR_jet);

    // ++++++++++++++++++++++++++++++++++++++++ //
    // draw axis and labels
    int slot_no;
    // double to, dt;
    slot_no = 0;
    // to = SpectrogramStack->Get_SlotTime(slot_no);
    // dt = SpectrogramStack->Get_TimeWidth();
    // SpectrogramStack->PlotAxis(to, 0.0, dt, 0.0);

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
      T_DSPlib_processing::CurrentObject->SignalSegments->SetBackgroundColor(1.0, 1.0, 0.0);
    T_DSPlib_processing::CurrentObject->SignalSegments->SubPlot(1, 1, -1, width, height, true);
    //  void SubPlot(int Rows, int Cols, int index, int WindowW, int WindowH, bool Clear);
    // ++++++++++++++++++++++++++++++++++++++++ //

    // select axes plot field for drawing
    T_DSPlib_processing::CurrentObject->SignalSegments->SetBackgroundColor(1.0, CLR_gray);
    T_DSPlib_processing::CurrentObject->SignalSegments->SubPlot(1, 1, 1, width, height, true);
    // T_DSPlib_processing::CurrentObject->SignalSegments->SubPlot(1,1,1, width, height, false);
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
      T_DSPlib_processing::CurrentObject->Histograms->SetBackgroundColor(1.0, 1.0, 0.0);
    T_DSPlib_processing::CurrentObject->Histograms->SubPlot(1, 1, -1, width, height, true);
    //  void SubPlot(int Rows, int Cols, int index, int WindowW, int WindowH, bool Clear);
    // ++++++++++++++++++++++++++++++++++++++++ //

    // select axes plot field for drawing
    T_DSPlib_processing::CurrentObject->Histograms->SetBackgroundColor(1.0, CLR_gray);
    T_DSPlib_processing::CurrentObject->Histograms->SubPlot(1, 1, 1, width, height, true);
    // T_DSPlib_processing::CurrentObject->SignalSegments->SubPlot(1,1,1, width, height, false);
    SetColor(0.0, CLR_gray);
    factor = 1.0 / (T_DSPlib_processing::CurrentObject->NoOfHistBins * T_DSPlib_processing::CurrentObject->BufferStep);
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
      temp_plot_stack->SetBackgroundColor(1.0, 1.0, 0.0);
    temp_plot_stack->SubPlot(1, 1, -1, width, height, true);
    //  void SubPlot(int Rows, int Cols, int index, int WindowW, int WindowH, bool Clear);
    // ++++++++++++++++++++++++++++++++++++++++ //

    // select axes plot field for drawing
    temp_plot_stack->SetBackgroundColor(1.0, CLR_gray);
    temp_plot_stack->SubPlot(1, 1, 1, width, height, true);
    // T_DSPlib_processing::CurrentObject->SignalSegments->SubPlot(1,1,1, width, height, false);
    SetColor(0.0, CLR_gray);
    factor = 1.0; ///(T_DSPlib_processing::CurrentObject->PSD_size);
    //! \bug A_PSD_dB ==> high_res_A_PSD ?????
    temp_plot_stack->DrawSignal_dB(
        segment_size,
        T_DSPlib_processing::CurrentObject->A_PSD_dB,
        PSD_max_dB, PSD_range_dB, 1.0);

    // ++++++++++++++++++++++++++++++++++++++++ //
    // ++++++++++++++++++++++++++++++++++++++++ //
    temp_plot_stack->InitAxis(T_DSPlib_processing::CurrentObject->Fp / 2, PSD_range_dB,
                              T_DSPlib_processing::CurrentObject->Fp / 2, 100.0, E_TM_black);
    temp_plot_stack->PlotAxis(0, PSD_max_dB - PSD_range_dB);
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
                       wxWindow *parent_in, wxWindowID id,
                       int *attribList, const wxPoint &pos, const wxSize &size,
                       long style, const wxString &name)
    : wxGLCanvas(parent_in, id, attribList, pos, size, style, name)
{
  ParentFrame = (MainFrame *)(parent_in->GetParent());
  BlockDrawing = true;
  temporary_BlockDrawing = false;

  SocketsAreConnected = false;
  DrawMode = E_DM_none; // E_DM_signal; //

  PSD_max_dB = -3.0;
  PSD_range_dB = 83.0;

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

  x_label = NULL;
  x_units = NULL;
  y_label = NULL;
  y_units = NULL;

  mouse_captured = false;
  // SpecDraw = NULL;
}

MyGLCanvas::~MyGLCanvas(void)
{
#ifdef _USE_GLC_
  glcDeleteFont(glc_font_id);
  glcDeleteContext(glc_context_id);
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
  if (permament == true)
  {
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

void MyGLCanvas::OnPaint(wxPaintEvent &event)
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
  // m_olf = new COutlineFont(this, "Courier"); //"Arial");
  m_bmf = new CBitmapFont(this, _T("Courier"));

#ifdef _USE_GLC_
  glc_context_id = glcGenContext();
  glcContext(glc_context_id);

  glc_font_id = glcGenFontID();
  // glcNewFontFromFamily ( glc_font_id, "Times New Roman" );
  // glcNewFontFromFamily ( glc_font_id, "Arial" );
  glcNewFontFromFamily(glc_font_id, "Courier New");

  glcFont(glc_font_id);
  glcFontFace(glc_font_id, "Regular"); // "Italic", "Regular", "Bold"
  // glcFontFace ( glc_font_id, "Bold");

  // glcStringType(GLC_UTF8_QSO);
  // glcRenderStyle(GLC_BITMAP);
  // glcRenderStyle(GLC_LINE);

  // glcRenderStyle(GLC_TRIANGLE); // <= requires fontconfig.dll and xmlparse.dll
  //; // <== GL scaling and rotation instead of GLC mones must be used
  //  glcRenderStyle(GLC_TEXTURE); // needs textures

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

void MyGLCanvas::OnSize(wxSizeEvent &event)
{
  //// this is also necessary to update the context on some platforms
  // wxGLCanvas::OnSize(event);

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

void MyGLCanvas::OnEraseBackground(wxEraseEvent &WXUNUSED(event))
{
  if (ParentFrame == NULL)
    return;

  if (ParentFrame->GetParentTask() == NULL)
  {
    int w, h;
    GetClientSize(&w, &h);

    InitGL();

    glViewport(0, 0, w, h);
    // glScissor (0,0, w, h);
    glDisable(GL_SCISSOR_TEST);

    // if (SocketsAreConnected == true)
    //   glClearColor(1.0,1.0,1.0,0);
    // else
    glClearColor(1.0, 1.0, 0.0, 0);
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
void MyGLCanvas::OnEnterWindow(wxMouseEvent &WXUNUSED(event))
{
  SetFocus();
}

void MyGLCanvas::OnMouseLeftDown(wxMouseEvent &event)
{
  if (ParentFrame != NULL)
  {
    if (OnMouseDown(event.GetX(), event.GetY()))
      ;
    {
      mouse_captured = true;
      CaptureMouse();
    }
  }
  event.Skip();
}
void MyGLCanvas::OnMouseLeftUp(wxMouseEvent &event)
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

void MyGLCanvas::OnMouseMotion(wxMouseEvent &event)
{
  // char tekst[1024];

  // sprintf(tekst, "x = %i, y = %i", event.GetX(), event.GetY());
  // GetParent()->SetLabel(tekst);
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

MyProcessingThread::MyProcessingThread(MainFrame *Parent_in)
    : wxThread(wxTHREAD_DETACHED) // wxTHREAD_JOINABLE
{
  Parent = Parent_in;
  NoOfBranches = 0;
  Branches = NULL;

  new_branch = NULL;

  UpdateBranches_semaphore = new wxSemaphore(0, 1);
}

void MyProcessingThread::CreateAndRunThreads(MainFrame *Parent_in, int NoOfThreads)
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
    ThreadFinished_semaphore[ind] = new wxSemaphore(0, 1);
    ProcessingThreads[ind] = new MyProcessingThread(Parent_in);
    ProcessingThreads[ind]->ThreadIndex = ind;
    ProcessingThreads[ind]->Create(100000000);

    // ProcessingThreads[0]->SetPriority(20);
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
  DSP::log << tekst << std::endl;

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
          DSP::log << DSP::e::LogMode::Error << "MyProcessingThread::~MyProcessingThread" << DSP::e::LogMode::second << "GetParentTask() == NULL" << std::endl;
          delete Branches[ind];
        }
        // delete Branches[ind];
        Branches[ind] = NULL;
      }
    }
    delete[] Branches;
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

      for (ind2 = ind + 1; ind2 < NoOfProcessingThreads; ind2++)
      {
        ProcessingThreads[ind2 - 1] = ProcessingThreads[ind2];
        ThreadFinished_semaphore[ind2 - 1] = ThreadFinished_semaphore[ind2];
      }

      NoOfProcessingThreads--;
      if (NoOfProcessingThreads == 0)
      {
        delete[] ProcessingThreads;
        ProcessingThreads = NULL;
        delete[] ThreadFinished_semaphore;
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
  // delete [] ProcessingThreads; // this also is delated in FreeThread
  // ProcessingThreads = NULL;
  NoOfCPUs = 0; // Forces initialization next time new thread is created
}

// unsigned int tread_wait_counter = 0;
wxThread::ExitCode MyProcessingThread::Entry(void)
{
  // unsigned int ind;
  bool DoFinish;
  unsigned int ind;
  int NoOfActiveBranches = 0;

  DoFinish = false;

  CS_OnDraw.Enter();
  Parent->GetGLcanvas(0)->EnableDrawing();
  CS_OnDraw.Leave();

  // for (ind=0 ; ind < 1000; ind++)
  while (1)
  {
    //! \bug Use number of active branches (NoOfBranches - NoOfPaused)
    if (NoOfActiveBranches == 0)
    {
      UpdateBranches_semaphore->WaitTimeout(10); // suspend process until new branches will be available
    }

    // DSP::log << "Yield"<< std::endl;
    //! \ bug check if it should be done here (maybe timer in main loop)
    //::wxGetApp().Yield(true);
    // DSP::log << "Yield2"<< std::endl;

    // DSP::log << "UpdateBranches()"<< std::endl;
    if (UpdateBranches() == false)
      DoFinish = true;

    // DSP::log << "ProcessBranches()"<< std::endl;
    //  command can be post before thread start
    NoOfActiveBranches = 0;
    for (ind = 0; ind < NoOfBranches; ind++)
    {
      if (Branches[ind]->ProcessBranch() == true)
        NoOfActiveBranches++;
    }

    // Use number of active branches (NoOfBranches - NoOfPaused)
    if ((Parent != NULL) && (NoOfActiveBranches != 0))
    {
      // AddPendingEvent
      // DSP::log << "GLcanvas->Refresh()"<< std::endl;
      Parent->GetGLcanvas(0)->Refresh(); // invalidate window
      //! \bug na słabszym sprzęcie bez Update poniżej się potrafi wywalić
      // Parent->GetGLcanvas(0)->Update();  // refresh canvas immediately
    }

    /*
        //DSP::log << "DrawWait()"<< std::endl;
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
    // if ((TestDestroy() == true) || (DoFinish == true))
    if (TestDestroy() == true)
      break;

    // DSP::log << "DoFinish()"<< std::endl;
    if (DoFinish == true)
    {
      // DSP::log << "DoFinish() == true"<< std::endl;
      wxCommandEvent event(wxEVT_PROCESS_END, ID_ProcessEnd);
      event.SetClientData(this);
      // Parent->GetEventHandler()->AddPendingEvent( event );
      wxPostEvent(Parent, event);
      break;
    }
  }

  CS_OnDraw.Enter();
  Parent->GetGLcanvas(0)->DisableDrawing();
  CS_OnDraw.Leave();

  DSP::log << DSP::e::LogMode::Error << "Thread loop ended" << std::endl;

  ////  not needed for wxTHREAD_JOINABLE
  DSP::log << DSP::e::LogMode::Error << "ThreadFinished is about to be posted" << std::endl;
  ThreadFinished_semaphore[ThreadIndex]->Post(); // signal finishing
  DSP::log << DSP::e::LogMode::Error << "ThreadFinished has been posted" << std::endl;

  return 0;
}

MyProcessingBranch *MyProcessingThread::AddProcessingBranch(MainFrame *Parent_in,
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
    DSP::log << "MyProcessingThread::UpdateBranches" << DSP::e::LogMode::second << "new_branch" << std::endl;

    MyProcessingBranch **new_Branches;
    new_Branches = new MyProcessingBranch *[NoOfBranches + 1];
    if (Branches != NULL)
    {
      memcpy(new_Branches, Branches, sizeof(MyProcessingBranch *) * NoOfBranches);
      delete[] Branches;
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
      DSP::log << "MyProcessingThread::UpdateBranches" << DSP::e::LogMode::second << "branch delete" << std::endl;

      CS_OnDraw.Enter();
      Parent->GetGLcanvas(0)->DisableDrawing();
      CS_OnDraw.Leave();

      if (Branches[ind]->Parent->GetParentTask() != NULL)
        Branches[ind]->Parent->GetParentTask()->DeleteBranch(Branches[ind]);
      else
      {
        DSP::log << DSP::e::LogMode::Error << "MyProcessingThread::UpdateBranches" << DSP::e::LogMode::second << "branch parent task is NULL" << std::endl;
        delete Branches[ind];
      }
      Branches[ind] = NULL;
    }
    else
    {
      if (ind > new_NoOfBranches)
      {
        DSP::log << "MyProcessingThread::UpdateBranches" << DSP::e::LogMode::second << "branch relocated" << std::endl;

        Branches[new_NoOfBranches] = Branches[ind];
      }
      new_NoOfBranches++;
    }
  }
  NoOfBranches = new_NoOfBranches;
  if ((NoOfBranches == 0) && (Branches != NULL))
  {
    delete[] Branches;
    Branches = NULL;
  }

  // DSP::log << "MyProcessingThread::UpdateBranches"<< DSP::e::LogMode::second <<"update finished"<< std::endl;
  MyProcessingBranch::CS_CommandList.Leave();
  return true; // keep thread working
}
void MainFrame::InitGUI(){
//TODO: Fill settings





}

void MainFrame::OnChannelFilterChange(wxScrollEvent &event)
{
  float Fd, Fg, Carrier_freq;
  bool Fd_fix = false;
  bool Fg_fix = false;
  bool keep_Fg = false;

  switch (event.GetId())
  {
  case ID_LPF_SLIDER:
    Fg = LPF_slider->GetValue();
    Fg *= 100;
    Fd = interface_state.channel_Fd;
    Carrier_freq = interface_state.carrier_freq;
    keep_Fg = true;
    break;
  case ID_HPF_SLIDER:
    Fd = HPF_slider->GetValue();
    Fd *= 100;
    Fg = interface_state.channel_Fg;
    Carrier_freq = interface_state.carrier_freq;
    keep_Fg = false;
    break;

  case ID_SELECT_SAMPLING_RATE:
    // update sliders settings based on current sampling rate
    // interface_state.sampling_rate
    HPF_slider->SetRange(0, (int)(interface_state.sampling_rate / 200));
    LPF_slider->SetRange(0, (int)(interface_state.sampling_rate / 200));
    HPF_slider->SetValue(0);
    LPF_slider->SetValue((int)(interface_state.sampling_rate / 200));
    
    CarrierFreqSlider-> SetRange(0,(int)(interface_state.sampling_rate / 200));
    CarrierFreqSlider-> SetValue((int)(interface_state.sampling_rate / 400));

    interface_state.channel_Fd = 0;
    interface_state.channel_Fg = interface_state.sampling_rate / 2;
    
    interface_state.carrier_freq =  interface_state.sampling_rate / 4;


    Fd = interface_state.channel_Fd;
    Fg = interface_state.channel_Fg; 
    Carrier_freq = interface_state.carrier_freq;
    UpdateModulatorParametersText();
    break;
  }

  if (keep_Fg == true)
  {
    // if (Fd > Fg-100)
    if (Fd > Fg)
    {
      // Fd = Fg - 100;
      Fd = Fg;
      Fd_fix = true;
    }
  }
  else
  {
    // if (Fd > Fg-100)
    if (Fd > Fg)
    {
      // Fg = Fd + 100;
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
  if (Fd >= interface_state.sampling_rate / 2 - 100)
  {
    Fd = interface_state.sampling_rate / 2 - 100;
    Fd_fix = true;
  }
  if (Fg >= interface_state.sampling_rate / 2)
  {
    Fg = interface_state.sampling_rate / 2;
    Fg_fix = true;
  }

  if (event.GetEventType() != wxEVT_SCROLL_CHANGED)
  { // only show current position but do not update
    HPF_text->ChangeValue(wxString::Format("%.0f Hz", Fd));
    LPF_text->ChangeValue(wxString::Format("%.0f Hz", Fg));
    CarrierFreqTextCtrl->ChangeValue(wxString::Format("%.2f [1/Sa] / %.0f [Hz]", (Carrier_freq/interface_state.sampling_rate),Carrier_freq));
    return;
  }

  interface_state.userdata_state = E_US_none;
  if (interface_state.channel_Fd != Fd)
  {
    interface_state.channel_Fd = Fd;
    interface_state.userdata_state = E_US_channel_HPF_coefs;
    HPF_text->ChangeValue(wxString::Format("%.0f Hz", Fd));
  }
  if (interface_state.channel_Fg != Fg)
  {
    interface_state.channel_Fg = Fg;
    interface_state.userdata_state |= E_US_channel_LPF_coefs;
    LPF_text->ChangeValue(wxString::Format("%.0f Hz", Fg));
  }
  if (interface_state.carrier_freq != Carrier_freq)
  {
    interface_state.carrier_freq = Carrier_freq;
    interface_state.userdata_state |= E_US_carrier_freq;
    CarrierFreqTextCtrl->ChangeValue(wxString::Format("%.2f [1/Sa] / %.0f [Hz]", (Carrier_freq / interface_state.sampling_rate), Carrier_freq));
  }
  if (Fg_fix == true)
  {
    LPF_slider->SetValue((int)(Fg / 100));
  }
  if (Fd_fix == true)
  {
    HPF_slider->SetValue((int)(Fd / 100));
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
      DSP::log << "Channel filter settings change" << DSP::e::LogMode::second << "PostCommandToBranch" << std::endl;
#endif
      parent_task->ProcessingBranch->PostCommandToBranch(temp);
    }
  }
  // UpdateGUI();
}

void MainFrame::OnCarrierFreqChange(wxScrollEvent &event)
{
  float Carrier_freq;
  switch (event.GetId())
  {
  case ID_carrier_freq_SLIDER:
    Carrier_freq = CarrierFreqSlider->GetValue();
    Carrier_freq *= 100;
    CarrierFreqTextCtrl->ChangeValue(wxString::Format("%.2f [1/Sa] / %.0f [Hz]", (Carrier_freq / interface_state.sampling_rate), Carrier_freq));
    if (event.GetEventType() != wxEVT_SCROLL_CHANGED)
    { // only show current position but do not update
      return;
    }
    interface_state.carrier_freq = Carrier_freq;
    interface_state.userdata_state |= E_US_carrier_freq;
    break;
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
      DSP::log << "MainFrame::OnCarrierFreqChange" << DSP::e::LogMode::second << "PostCommandToBranch" << std::endl;
#endif
      parent_task->ProcessingBranch->PostCommandToBranch(temp);
    }
}
}
void MainFrame::UpdateModulatorParametersText(){
  double N_symb, f_symb, bit_per_sample, Tsymb, F_symb, bps;
  int M;
  switch (interface_state.modulator_type){//parameters based on modulation type.
  // TODO: parameters based on type+ variant.
  
  case E_MT_ASK:
  F_symb = interface_state.sampling_rate/40.0;
  M = 8;
  break;
  case E_MT_PSK:
  M = 8;
  F_symb = interface_state.sampling_rate/40.0;
  break;
  case E_MT_QAM:
  case E_MT_FSK:
  default:
  F_symb = -1;
  break;
  }
 
  f_symb = F_symb/interface_state.sampling_rate;
  N_symb = interface_state.sampling_rate/F_symb;
  Tsymb = 1.0/F_symb;
  bit_per_sample = M*f_symb;
  bps = M*F_symb;

  NsymbText->SetValue(wxString::FromDouble(N_symb) +" [Sa]");
  f_symb1Text->SetValue(wxString::FromDouble(f_symb) + " [Sa/symb]");
  BitPerSampleText->SetValue(wxString::FromDouble(bit_per_sample)+" [bit/Sa]");
  TsymbText->SetValue(wxString::FromDouble(Tsymb)+" [s]");
  F_symb2Text->SetValue(wxString::FromDouble(F_symb)+" [bod]");
  bpsText->SetValue(wxString::FromDouble(bps)+" [bit/s]");
}

void MainFrame::OnChannelSNRChange(wxScrollEvent &event)
{
  float SNR_dB;

  switch (event.GetId())
  {
  case ID_SNR_SLIDER:
    // 0..MAX_SLIDER_VALUE ==> -20 .. 80
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
      DSP::log << "MainFrame::OnChannelSNRChange" << DSP::e::LogMode::second << "PostCommandToBranch" << std::endl;
#endif
      parent_task->ProcessingBranch->PostCommandToBranch(temp);
    }
  }
  // UpdateGUI();
}

void MainFrame::OnPSDparamsChange(wxScrollEvent &event)
{
  int no_of_slots;

  switch (event.GetId())
  {
  case ID_PSD_SLOTS_SLIDER:
    // 0..MAX_SLIDER_VALUE ==> -20 .. 80
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
  // UpdateGUI();
}

void MainFrame::OnMixerVolumeChange(wxScrollEvent &event)
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
    AudioMixer->SetDestLineVolume(DSP::AM_MasterControl, val);
    break;

  default:
    return;
  }
}

void MainFrame::OnWPMchange(wxScrollEvent &event)
{
  wxString text;
  int WPM;

  switch (event.GetId())
  {
  case ID_WPM_SLIDER:
    WPM = WPM_slider->GetValue();
    WPM = 5 + WPM * 5;

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
      DSP::log << "MainFrame::OnSendAsciText" << DSP::e::LogMode::second << "PostCommandToBranch" << std::endl;
#endif
      parent_task->ProcessingBranch->PostCommandToBranch(temp);
    }
  }
  // UpdateGUI();
}

void MainFrame::OnButtonPress(wxCommandEvent &event)
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
      switch (text[ind].GetValue())
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
    VoiceFileTypes fileType;
    std::string voiceType;
    voiceType = VoiceTypeBox->GetStringSelection().ToStdString();
    if (voiceType == "losowy")
    {
      int randomInd = (rand() % (VoiceTypeBox->GetCount() - 2));
      voiceType = VoiceTypeBox->GetString(randomInd).ToStdString();
    }
    fileType = (UseLogatoms->GetValue() == 1) ? VoiceFileTypes::Logatoms : VoiceFileTypes::Sentences;
    fileManager->getRandomFileInfo(fileType, voiceType, interface_state.selected_wav_info);
    VoiceFileIndex->SetValue(interface_state.selected_wav_info.shortName);
    SentenceTranscription->ChangeValue(wxString::FromUTF8(interface_state.selected_wav_info.transcription));
  }
    return;
    break;

  case ID_stop_wav_file:
    interface_state.wav_filename = "";

    interface_state.userdata_state = E_US_wav_file_open;
    break;

  case ID_open_wav_file:
    interface_state.wav_filename = interface_state.selected_wav_info.path.string();

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
      DSP::log << "MainFrame::OnSendAsciText" << DSP::e::LogMode::second << "PostCommandToBranch" << std::endl;
#endif
      parent_task->ProcessingBranch->PostCommandToBranch(temp);
    }
  }
  // UpdateGUI();
}

void MainFrame::OnDrawModeChange(wxCommandEvent &event)
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

void MainFrame::OnSelectVoiceType(wxCommandEvent &event)
{
  VoiceFileTypes fileType;
  std::string voiceType;
  voiceType = VoiceTypeBox->GetStringSelection().ToStdString();
  if (voiceType == "losowy")
  {
    int randomInd = (rand() % (VoiceTypeBox->GetCount() - 2));
    voiceType = VoiceTypeBox->GetString(randomInd).ToStdString();
  }
  fileType = (UseLogatoms->GetValue() == 1) ? VoiceFileTypes::Logatoms : VoiceFileTypes::Sentences;
  if (VoiceFileIndex->GetValue() == _T(""))
    fileManager->getRandomFileInfo(fileType, voiceType, interface_state.selected_wav_info);
  else
    fileManager->getSelectedFileInfo(fileType, voiceType, interface_state.selected_wav_info);

  SentenceTranscription->ChangeValue(wxString::FromUTF8(interface_state.selected_wav_info.transcription));
  VoiceFileIndex->SetValue(interface_state.selected_wav_info.shortName);
}

// ----------------------------------------------------------------
// Morse Validator
// ----------------------------------------------------------------

BEGIN_EVENT_TABLE(wxMorseValidator, wxValidator)
EVT_CHAR(wxMorseValidator::OnChar)
EVT_TEXT(wxID_ANY, wxMorseValidator::OnTextChange)
END_EVENT_TABLE()

wxMorseValidator::wxMorseValidator(int prec_in, E_NV_mode mode_in, long double *m_long_double_Value_in)
{
  current_morse_block = NULL;
}

wxMorseValidator::wxMorseValidator(const wxMorseValidator &val)
    : wxValidator()
{
  Copy(val);
}

bool wxMorseValidator::Copy(const wxMorseValidator &val)
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

  if (!CheckValidator())
    return false;

  wxTextCtrl *control = (wxTextCtrl *)m_validatorWindow;

  // If window is disabled, simply return
  if (!control->IsEnabled())
    return true;

  wxString val(control->GetValue());

  //! \todo ??? is there really something to do here
  ok = true;

  return ok;
}

// Called to transfer data to the window
bool wxMorseValidator::TransferToWindow(void)
{
  if (!CheckValidator())
    return false;

  if (current_morse_block != NULL)
  {
    wxString stringValue;

    wxTextCtrl *control = (wxTextCtrl *)m_validatorWindow;

    //! \bug get internal ascii buffer of current_morse_block

    control->SetValue(stringValue);
  }

  return true;
}

// Called to transfer data to the window
bool wxMorseValidator::TransferFromWindow(void)
{
  if (!CheckValidator())
    return false;

  if (current_morse_block != NULL)
  {
    wxString stringValue;

    wxTextCtrl *control = (wxTextCtrl *)m_validatorWindow;
    stringValue = control->GetValue();

    //! \bug set internal ascii buffer of current_morse_block
  }
  return true;
}

//! \todo implement EVT_TEXT to control - live text changes control
void wxMorseValidator::OnTextChange(wxCommandEvent &event)
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

void wxMorseValidator::OnChar(wxKeyEvent &event)
{
  if (m_validatorWindow)
  {
    bool discard_key;
    int keyCode = event.GetKeyCode();

    //! \bug implement sensible processing of characters
    discard_key = false;
    // we don't filter special keys and Delete
    if (
        !(keyCode < WXK_SPACE || keyCode == WXK_DELETE || keyCode > WXK_START) && !wxIsdigit(keyCode))
    //&& keyCode != wxT('.') && keyCode != wxT(',') && keyCode != wxT('-') )
    //&& keyCode != wxT('+') && keyCode != wxT('e') && keyCode != wxT('E'))
    {
      discard_key = true;
    }

    if (discard_key == true)
    {
      if (!wxValidator::IsSilent())
        wxBell();

      // eat message
      return;
    }

    //! \bug depending on mode send character to current_morse_block
  }

  event.Skip();
}

// ----------------------------------------------------------------
// Voice File Manager
// ----------------------------------------------------------------

VoiceFileManager::VoiceFileManager(std::string parentPath, std::string logatomsDirName, std::string sentencesDirName, std::string fileExtension)
{
  fs::path tmp_path({parentPath});

  if (fs::exists(tmp_path) && fs::is_directory(tmp_path))
  {
    this->parentPath = tmp_path;

    if (fs::exists(tmp_path / logatomsDirName) && fs::is_directory(tmp_path / logatomsDirName))
      this->logatomsDirName = logatomsDirName;
    else
      throw std::invalid_argument((tmp_path / logatomsDirName).string() + " directory does not exist");

    if (fs::exists(tmp_path / sentencesDirName) && fs::is_directory(tmp_path / sentencesDirName))
      this->sentencesDirName = sentencesDirName;
    else
      throw std::invalid_argument((tmp_path / sentencesDirName).string() + " directory does not exist");
    this->selectedFileExtension = fileExtension;
  }
  else
    throw std::invalid_argument("Parent path: " + tmp_path.string() + " is not a directory or does not exist.");
}

std::vector<wxString> VoiceFileManager::listVoiceTypes(VoiceFileTypes FileType) const
{
  std::vector<wxString> types;
  fs::path directoryPath;

  if (FileType == VoiceFileTypes::Logatoms)
    directoryPath = parentPath / logatomsDirName;
  else
    directoryPath = parentPath / sentencesDirName;

  for (const auto &entry : std::filesystem::directory_iterator(directoryPath))
  {
    if (entry.is_directory())
    {
      wxString subfolder_name(entry.path().filename().string());
      types.push_back(subfolder_name);
    }
  }
  return types;
}

std::vector<std::string> VoiceFileManager::listFiles(VoiceFileTypes fileType, std::string voiceType) const
{
  fs::path subDirectory = (fileType == VoiceFileTypes::Logatoms) ? logatomsDirName : sentencesDirName;
  subDirectory = subDirectory / voiceType;
  std::vector<std::string> fileList;
  for (const auto &entry : std::filesystem::directory_iterator(parentPath / subDirectory))
  {
    if (entry.is_regular_file() && entry.path().extension() == selectedFileExtension)
    {
      std::string fileName = entry.path().filename().string();
      fileList.push_back(fileName);
    }
  }

  return fileList;
}

void VoiceFileManager::getRandomFileInfo(VoiceFileTypes fileType, std::string voiceType, VoiceFileInfo &fileInfo) const
{
  VoiceFileInfo result;
  std::vector<std::string> fileList = listFiles(fileType, voiceType);
  std::string line;
  fs::path randomFile;
  fs::path tmpPath = makePath(fileType, voiceType, false);
  int randomIndex;
  randomIndex = (rand() % fileList.size());
  randomFile = tmpPath / fileList[randomIndex];

  fileInfo.shortName = randomFile.stem().string();
  fileInfo.fullName = randomFile.filename().string();
  fileInfo.path = randomFile.relative_path().string();

  if (fs::exists(parentPath / randomFile.relative_path().parent_path() / "list.txt"))
  {
    std::ifstream p(parentPath / randomFile.relative_path().parent_path() / "list.txt");
    if (!p)
    {
#ifdef __DEBUG__
      DSP::log << "VoiceFileManager" << DSP::e::LogMode::second << "list.txt exists but is not readable.";
#endif
    }
    else
    {

      while (getline(p, line))
      {
        if (line.find(fileInfo.shortName + "-") != std::string::npos)
        {
          fileInfo.transcription = line.substr(fileInfo.shortName.length() + 2, line.length() - fileInfo.shortName.length() - 2 - selectedFileExtension.length()); // ignore the first segment of the transcription e.g. "500- "
          break;
        }
      }
      p.close();
    }
  }
  else
  {
#ifdef __DEBUG__
    DSP::log << "VoiceFileManager" << DSP::e::LogMode::second << "list.txt not found in the current directory. Skipping";
#endif
  }
}

void VoiceFileManager::getSelectedFileInfo(VoiceFileTypes fileType, std::string voiceType, VoiceFileInfo &fileInfo) const
{
  fs::path selectedFile;

  selectedFile = makePath(fileType, voiceType, fileInfo.fullName, false);
  if (fs::exists(parentPath / selectedFile))
  {
    fileInfo.shortName = selectedFile.stem().string();
    fileInfo.fullName = selectedFile.filename().string();
    fileInfo.path = selectedFile.relative_path().string();
  }
  else
  {
    getRandomFileInfo(fileType, voiceType, fileInfo);
  }
}
