// -*- C++ -*-
//
// generated by wxGlade 1.0.4 on Fri Nov 17 18:47:39 2023
//
// Example for compiling a single file project under Linux using g++:
//  g++ MyApp.cpp $(wx-config --libs) $(wx-config --cxxflags) -o MyApp
//
// Example for compiling a multi file project under Linux using g++:
//  g++ main.cpp $(wx-config --libs) $(wx-config --cxxflags) -o MyApp Dialog1.cpp Frame1.cpp
//

#ifndef TELESOUND_WXGLADE_H
#define TELESOUND_WXGLADE_H

#include <wx/wx.h>
#include <wx/image.h>
#include <wx/intl.h>

#ifndef APP_CATALOG
#define APP_CATALOG "app"  // replace with the appropriate catalog name
#endif


#include <wx/notebook.h>
#include <wx/splitter.h>
#include <wx/statline.h>

#include "IDs.h"
#include "Icons.h"
#include "MyGLcanvas.h"


class GUIFrame: public wxFrame {
public:
    GUIFrame(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos=wxDefaultPosition, const wxSize& size=wxDefaultSize, long style=wxDEFAULT_FRAME_STYLE);

private:

protected:
    // begin wxGlade: GUIFrame::attributes
    wxMenuBar* frame_menubar;
    wxStatusBar* statusbar;
    wxSplitterWindow* sashWindow;
    MyGLCanvas* GLcanvas;
    wxNotebook* notebookWindow;
    wxPanel* page1;
    wxBitmapButton* RunProcessingButton;
    wxBitmapButton* PauseProcessingButton;
    wxBitmapButton* StopProcessingButton;
    wxBitmapButton* MicStateButton;
    wxBitmapButton* LocalSignalStateButton;
    wxBitmapButton* ResetSettingsButton;
    wxRadioButton* WorksAsServer;
    wxRadioButton* WorksAsClient;
    wxTextCtrl* ServerAddressEdit;
    wxComboBox* SamplingRateBox;
    wxComboBox* DrawModeBox;
    wxTextCtrl* PSD_slots_text;
    wxBitmapButton* ResizeToSquare;
    wxSlider* PSD_slots_slider;
    wxComboBox* SourceLine_ComboBox;
    wxSlider* SourceLine_slider;
    wxComboBox* DestLine_ComboBox;
    wxSlider* DestLine_slider;
    wxSlider* MasterLine_slider;
    wxPanel* page2;
    wxTextCtrl* SNR_text;
    wxSlider* SNR_slider;
    wxSlider* HPF_slider;
    wxTextCtrl* HPF_text;
    wxTextCtrl* LPF_text;
    wxSlider* LPF_slider;
    wxStaticBoxSizer* sizer_5;
    wxRadioButton* UseLogatoms;
    wxRadioButton* UseSentences;
    wxComboBox* VoiceTypeBox;
    wxButton* SelectVoiceFile;
    wxTextCtrl* VoiceFileIndex;
    wxBitmapButton* OpenWAVEfile;
    wxBitmapButton* StopWAVEfile;
    wxCheckBox* showSentenceText;
    wxTextCtrl* SentenceTranscription;
    wxPanel* page3;
    wxTextCtrl* WPM_text;
    wxSlider* WPM_slider;
    wxTextCtrl* AsciiTextEntry;
    wxBoxSizer* sizer_18;
    wxTextCtrl* TextCtrlPlaceholder;
    wxButton* SendAsciiText;
    wxCheckBox* MorseReceiverState;
    wxTextCtrl* AsciiTextReceiver;
    wxPanel* page5;
    wxTextCtrl* StatusBox;
    wxScrolledWindow* page6;
    wxComboBox* ModulationTypeBox;
    wxChoice* ModulatorVariantSelect;
    wxCheckBox* ModulatorState;
    wxTextCtrl* NsymbText;
    wxTextCtrl* f_symb1Text;
    wxTextCtrl* BitPerSampleText;
    wxTextCtrl* TsymbText;
    wxTextCtrl* F_symb2Text;
    wxTextCtrl* bpsText;
    wxTextCtrl* CarrierFreqTextCtrl;
    wxSlider* CarrierFreqSlider;
    wxCheckBox* demodState;
    wxTextCtrl* DemodCarrierFreqTextCtrl;
    wxSlider* DemodCarrierFreq;
    wxSlider* DemodDelay;
    // end wxGlade

    DECLARE_EVENT_TABLE();

public:
    virtual void OnLanguageChange(wxCommandEvent &event); // wxGlade: <event_handler>
}; // wxGlade: end class


#endif // TELESOUND_WXGLADE_H
