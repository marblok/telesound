#include "GUIcalls.h"
//#define wxAddons_wxTimePickerCtrl
#include "DSP.h"
#include "main.h"

void CallPauseTaskProcessing(void *task)
{
  DSPf_InfoMessage("CallPauseTaskProcessing", "start");
  ((T_TaskElement *)task)->PauseTaskProcessing();
  DSPf_InfoMessage("CallPauseTaskProcessing", "end");
}
