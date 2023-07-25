#ifndef   IDs
#define   IDs
#include <wx/wx.h>

const int ID_DrawNow    = wxID_HIGHEST+1;
const int ID_ProcessEnd = wxID_HIGHEST+2;
const int ID_BranchEnd  = wxID_HIGHEST+3;

enum E_PageIDs
{
  E_PID_tasks = 0,
  E_PID_CHANNEL = 1,
  E_PID_MORSE = 2,
  E_PID_status = 3,
  E_PID_MODULATOR = 4,
  E_PID_default = 1000
};

#define MAX_SLIDER_VALUE 100
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
    ID_show_text_checkbox = 138,
    
    
    
    ID_modulator_state = 139,



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
    
    ID_STATUSBOX = 407,
    
    MDI_REFRESH,
    MDI_CHANGE_TITLE,
    MDI_CHANGE_POSITION,
    MDI_CHANGE_SIZE,
    MDI_CHILD_QUIT,

    MDI_ABOUT = wxID_ABOUT
};
#endif