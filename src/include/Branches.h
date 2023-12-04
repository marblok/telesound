/*
 * Branches.h
 *
 *  Created on: 2008-10-23
 *      Author: Marek
 */

#ifndef BRANCHES_H_
#define BRANCHES_H_

#include "project_definitions.h"
#include "main.h"
class MyProcessingBranch;

//! base class for command data
class TCommandData
{
  public:
    MyProcessingBranch *BranchToClose;
    void *UserData;

    TCommandData(void);
    virtual ~TCommandData(void);
};
enum E_BranchCommand
{
  E_BC_none = 0,
  //! process must finish and inform parent window
  E_BC_end = 1,
  //! process must finished because parent window is closing (OnClose)
  E_BC_closing = 2,
  //! branch must pause processing
  E_BC_pause = 3,
  //! branch must continue paused processing
  E_BC_continue = 4,
  //! send user data to branch
  E_BC_userdata = 5
};

class T_BranchCommand
{
  friend class MyProcessingBranch;

  private:
    E_BranchCommand command;
    TCommandData *command_data;

    T_BranchCommand *Next;

  public:
    //! Creates command object
    /*! Note command data must be allocated with "new" operator,
     *  so the command destructor can delete it.
     *
     *  The same about command_data_.
     */
    T_BranchCommand(E_BranchCommand command_in, TCommandData *command_data_in = NULL);
    //! deletes command_data object
    ~T_BranchCommand(void);
};

class MyProcessingBranch
{
  friend class MyProcessingThread;

  private:
    //MyGLCanvas  *GLcanvas;
    MyBranchParent  *Parent;
    MyProcessingThread *ParentThread;

    T_BranchCommand *CommandList;
    void DeleteCommandList(void);
    //! Removes command from the CommandList
    void RemoveCommandFromList(T_BranchCommand *command);

    //! variable storing processing stack
    /*! First element must be T_InputElement
     *  T_OutputStacElement will not be enough
     */
    T_InputElement *ProcessingStack;

    enum E_branch_state
    {
      E_BS_none = 0,
      E_BS_step_forward = 1,
      E_BS_pause = 2,
      E_BS_end = -1,    // stop processing => tell parent window to close
      E_BS_closing = -2 // stop processing => parent window is closing
    };
    E_branch_state branch_state;

    //! true if drawing is blocked
    /*! \warning should only be changed in MyProcessingBranch::ProcessBranchCommandList
     */
    bool DrawIsBlocked;
    bool DoFinish;

  public:
    //! Semaphore for acknowledging message reception by main thread
    /*! \warning Separate for each branch
     */
    wxSemaphore *MessageACK;

    //! common for all branches
    static wxCriticalSection CS_CommandList;

  public:
    //! Processes commands from the list
    /*! Analyses commands registered on the list
     *  and updates process state.
     *  Compatible commands should be processed in one go
     *  so they will modify processing as soon as possible
     *
     *  \note Not all the command might be processed in one go.
     *
     *  \warning Accessing the list must be done in critical section
     */
    void ProcessBranchCommandList(void);

    //! adds info onto thread commnad's list
    /*! Suspends thread and adds command onto the list.
     *  Some commands can modify existing tread's command list
     *
     *  \note new_command_in must be allocated with "new" operator
     *        so the thread can delete it later.
     */
    void PostCommandToBranch(T_BranchCommand *new_command_in);

    //! Branch processing (single cycle)
    /*! Returns true if process is active.
     *  Not paused or about to be closed.
     */
    bool ProcessBranch(void);

  private:
    //! \warning Must only be called from  MyProcessingThread::AddProcessingBranch
    MyProcessingBranch(MyProcessingThread *ParentThread_in,
        MyBranchParent *Parent_in, T_InputElement *ProcessingStack_in);
  public:
    ~MyProcessingBranch(void);
};


#endif /* BRANCHES_H_ */
