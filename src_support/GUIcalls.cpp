#include "GUIcalls.h"
//#define wxAddons_wxTimePickerCtrl
#include "DSP.h"
#include "main.h"

void CallPauseTaskProcessing(void *task)
{
  DSP::log << "CallPauseTaskProcessing"<< DSP::e::LogMode::second << "start"<<endl;
  ((T_TaskElement *)task)->PauseTaskProcessing();
  DSP::log << "CallPauseTaskProcessing"<< DSP::e::LogMode::second << "end"<<endl;
}
