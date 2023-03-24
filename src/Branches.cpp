/*
 * Branches.cpp
 *
 *  Created on: 2008-10-23
 *      Author: Marek
 */

#include "DSP.h"
#include "main.h"
#include "Branches.h"

// +++++++++++++++++++++++++++++++++++++++ //

TCommandData::TCommandData(void)
{
  BranchToClose = NULL;
  UserData = NULL;
}
TCommandData::~TCommandData(void)
{
}

// +++++++++++++++++++++++++++++++++++++++ //

T_BranchCommand::T_BranchCommand(E_BranchCommand command_in, TCommandData *command_data_in)
{
  command = command_in;
  command_data = command_data_in;
  Next = NULL;
}
T_BranchCommand::~T_BranchCommand(void)
{
  // delete data
  if (command_data != NULL)
  {
    delete command_data;
    command_data = NULL;
  }
}

// +++++++++++++++++++++++++++++++++++++++ //
wxCriticalSection MyProcessingBranch::CS_CommandList;

// +++++++++++++++++++++++++++++++++++++++ //
MyProcessingBranch::MyProcessingBranch(MyProcessingThread *ParentThread_in,
    MyBranchParent *Parent_in, T_InputElement *ProcessingStack_in)
{ // wxThread::Create() <= constructor creates thread object but not the thread itself
  // wxThread::Run() <= from another thread after wxThread::Create()
  // wxThread::Sleep()
  // wxThread::TestDestroy() <= must be call periodically
  // wxThread::Yield() <= allows other threads to run
  ParentThread = ParentThread_in;
  Parent = Parent_in;


  DoFinish = false;
  DrawIsBlocked = false;

  branch_state = E_BS_step_forward;
  CommandList = NULL;
  MessageACK = new wxSemaphore(0, 1);

  ProcessingStack = ProcessingStack_in;
}

MyProcessingBranch::~MyProcessingBranch(void)
{
  DeleteCommandList();

  if (ProcessingStack != NULL)
  {
    ProcessingStack->DeleteStack();
    // delete ProcessingStack;
    ProcessingStack = NULL;
  }

  if (MessageACK != NULL)
  {
    delete MessageACK;
    MessageACK = NULL;
  }
}

bool MyProcessingBranch::ProcessBranch(void)
{
  bool is_running;
  //unsigned int ind;

  //for (ind=0 ; ind < 1000; ind++)
  // while (1)
  /// JUST SINGLE GO
  // command can be post before thread start
  ProcessBranchCommandList();

  /*! \todo when processing dir changes or start/end times change
   * initialize start/end time tables
   * branch->UpdateTimeTables(E_PD_forward, -1);
   */

  is_running = false;
  switch (branch_state)
  {
    case E_BS_pause:
      is_running = false;
      // do nothing
      break;

    case E_BS_step_forward:
      if (ProcessingStack != NULL)
      {
        is_running = ((T_FILEinput *)ProcessingStack)->Process();
        if (is_running == false)
        { // ask user to close this branch
          wxCommandEvent event( wxEVT_BRANCH_END, ID_BranchEnd );
          event.SetClientData( this );
          //DSP::log << "ProcessBranch"<< DSP::e::LogMode::second << "sending wxEVT_BRANCH_END"<<endl;
          //Parent->AddPendingEvent( event );
          Parent->GetEventHandler()->AddPendingEvent(event);
          //Parent->ProcessEvent( event ); // would block ???
          //DSP::log << "ProcessBranch"<< DSP::e::LogMode::second << "sent wxEVT_BRANCH_END"<<endl;

          // main thread will do this
          //DoFinish = true;
          //DrawIsBlocked = true;
        }
      }
      break;

    case E_BS_closing:
      DSP::log << "ProcessBranch"<< DSP::e::LogMode::second << "E_BS_closing"<<endl;
      is_running = false;

      DoFinish = true;
      DrawIsBlocked = true;
      break;
  }
  return is_running;
}

void MyProcessingBranch::DeleteCommandList(void)
{
  T_BranchCommand *temp_command;

  CS_CommandList.Enter();

  temp_command = CommandList;
  if (temp_command != NULL)
  {
    temp_command = CommandList->Next;
    delete CommandList;
    CommandList = temp_command;
  }

  CS_CommandList.Leave();
}

// remove command from the list
void MyProcessingBranch::RemoveCommandFromList(T_BranchCommand *command)
{
  T_BranchCommand *current;

  if (command == NULL)
    return;

  if (command == CommandList)
    CommandList = command->Next;
  else
  {
    current = CommandList;
    while (current != NULL)
    {
      if (current->Next == command)
      {
        current->Next = command->Next;
        break;
      }
      current = current->Next;
    }
  }
}


void MyProcessingBranch::PostCommandToBranch(T_BranchCommand *new_command_in)
{
  T_BranchCommand *temp_command;
  T_BranchCommand *last_command;

  //ParentThread->Pause(); // pause thread operations
  CS_CommandList.Enter();

  temp_command = CommandList;
  last_command = NULL;

  // modify command's list
  while (temp_command != NULL)
  {
    //! \todo Check new_command_in compatibility with commands on the list

    last_command = temp_command;
    temp_command = temp_command->Next;
  }
  if (last_command != NULL)
    last_command->Next = new_command_in;
  else
    CommandList = new_command_in;

  CS_CommandList.Leave();
  //ParentThread->Resume(); // resume thread operations
#ifdef __DEBUG__
  DSP::log << "Command Posted"<<endl;
#endif
}

void MyProcessingBranch::ProcessBranchCommandList(void)
{
  T_BranchCommand *temp_command, *current;

  CS_CommandList.Enter()  ;
  // process and modify command's list
  current = CommandList;

  while (current != NULL)
  {
    if (current->command == E_BC_closing)
    {
      branch_state = E_BS_closing;

      DSP::log << "ProcessBranchCommandList"<< DSP::e::LogMode::second << "Closing"<<endl;
      /*! \bug commands queue should be purged
       * so the commands which owned data
       * will be forced to delete then
       * instead of the branch
       */
      CS_CommandList.Leave();
      return;
    }

    switch (current->command)
    {
      case E_BC_pause:
        branch_state = E_BS_pause;
        break;
      case E_BC_userdata:
        ProcessingStack->ProcessUserData(current->command_data->UserData);
#ifdef __DEBUG__
  DSP::log << "User command Processed"<<endl;
#endif
        break;
      case E_BC_continue:
        // start processing
        branch_state = E_BS_step_forward;
        break;
      default:
        #ifdef __DEBUG__
          DSP::log << DSP::e::LogMode::Error <<"MyProcessingBranch::ProcessBranchCommandList"<< DSP::e::LogMode::second <<"Unsupported branch command"<<endl;
        #endif
        break;
    }

    RemoveCommandFromList(current);
    temp_command = current;
    current = current->Next;
    delete temp_command;
  }

  CS_CommandList.Leave();

}

