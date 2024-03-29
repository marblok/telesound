// -*- C++ -*-
//
// generated by wxGlade 1.0.4 on Wed Nov 29 09:55:47 2023
//
// Example for compiling a single file project under Linux using g++:
//  g++ MyApp.cpp $(wx-config --libs) $(wx-config --cxxflags) -o MyApp
//
// Example for compiling a multi file project under Linux using g++:
//  g++ main.cpp $(wx-config --libs) $(wx-config --cxxflags) -o MyApp Dialog1.cpp Frame1.cpp
//

#include "TeleSound_wxGlade.h"




GUIFrame::GUIFrame(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style):
    wxFrame(parent, id, title, pos, size, wxDEFAULT_FRAME_STYLE)
{
    SetSize(wxSize(1158, 727));
    SetTitle(_("TeleSound ver. 2.1 (Marek.Blok@eti.pg.edu.pl)"));
    frame_menubar = new wxMenuBar();
    wxMenu *wxglade_tmp_menu;
    wxglade_tmp_menu = new wxMenu();
    wxglade_tmp_menu->Append(ID_Lang_PL, _("PL"), _("jezyk polski"), wxITEM_RADIO);
    Bind(wxEVT_MENU, &GUIFrame::OnLanguageChange, this, ID_Lang_PL);
    wxglade_tmp_menu->Append(ID_Lang_EN, _("ENG"), _("jezyk angielski"), wxITEM_RADIO);
    Bind(wxEVT_MENU, &GUIFrame::OnLanguageChange, this, ID_Lang_EN);
    frame_menubar->Append(wxglade_tmp_menu, _("Language"));
    wxglade_tmp_menu = new wxMenu();
    wxglade_tmp_menu->Append(MDI_ABOUT, _("O programie..."), _("Wyswietl informacje o programie"));
    frame_menubar->Append(wxglade_tmp_menu, _("Pomoc"));
    SetMenuBar(frame_menubar);
    statusbar = CreateStatusBar(1, wxSTB_DEFAULT_STYLE);
    int statusbar_widths[] = { 1 };
    statusbar->SetStatusWidths(1, statusbar_widths);
    
    sashWindow = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_THIN_SASH);
    sashWindow->SetMinimumPaneSize(20);
    GLcanvas = new MyGLCanvas(0, sashWindow, wxID_ANY);
    notebookWindow = new wxNotebook(sashWindow, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_FIXEDWIDTH|wxNB_TOP);
    notebookWindow->SetMinSize(wxSize(300, -1));
    page1 = new wxPanel(notebookWindow, E_PID_tasks);
    notebookWindow->AddPage(page1, _("Konfiguracja"));
    wxBoxSizer* sizerPanel = new wxBoxSizer(wxVERTICAL);
    wxStaticBoxSizer* TaskActions_sizer = new wxStaticBoxSizer(new wxStaticBox(page1, wxID_ANY, _("Kontrola przetwarzania")), wxHORIZONTAL);
    sizerPanel->Add(TaskActions_sizer, 0, wxEXPAND, 0);
    wxStaticBitmap* ConnectionState = new wxStaticBitmap(page1, ID_connection_state, wxBitmap( new_xpm ), wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    ConnectionState->SetToolTip(_("Status polaczenia"));
    ConnectionState->Enable(0);
    ConnectionState->Hide();
    TaskActions_sizer->Add(ConnectionState, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3);
    wxStaticLine* static_line_4 = new wxStaticLine(page1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL);
    TaskActions_sizer->Add(static_line_4, 0, wxALL|wxEXPAND, 5);
    RunProcessingButton = new wxBitmapButton(page1, ID_RUN_TASK, wxBitmap( play_xpm ));
    RunProcessingButton->SetToolTip(_("Rozpocznij przetwarzanie"));
    RunProcessingButton->SetSize(RunProcessingButton->GetBestSize());
    TaskActions_sizer->Add(RunProcessingButton, 0, 0, 0);
    PauseProcessingButton = new wxBitmapButton(page1, ID_PAUSE_TASK, wxBitmap( pause_xpm ));
    PauseProcessingButton->SetToolTip(_("Wstrzymaj przetwarzanie"));
    PauseProcessingButton->SetSize(PauseProcessingButton->GetBestSize());
    TaskActions_sizer->Add(PauseProcessingButton, 0, 0, 0);
    StopProcessingButton = new wxBitmapButton(page1, ID_STOP_TASK, wxBitmap( stop_xpm ));
    StopProcessingButton->SetToolTip(_("Zakoncz przetwarzanie"));
    StopProcessingButton->SetSize(StopProcessingButton->GetBestSize());
    TaskActions_sizer->Add(StopProcessingButton, 0, 0, 0);
    wxStaticLine* static_line_3 = new wxStaticLine(page1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL);
    TaskActions_sizer->Add(static_line_3, 0, wxALL|wxEXPAND, 5);
    MicStateButton = new wxBitmapButton(page1, ID_MIKE_ON_OFF, wxBitmap( mike_off_xpm ));
    MicStateButton->SetToolTip(_("Status mikrofonu (wylaczony)"));
    MicStateButton->SetSize(MicStateButton->GetBestSize());
    TaskActions_sizer->Add(MicStateButton, 0, 0, 0);
    LocalSignalStateButton = new wxBitmapButton(page1, ID_LOCAL_SIGNAL_ON_OFF, wxBitmap( local_off_xpm ));
    LocalSignalStateButton->SetToolTip(_("Sygnal lokalny (wylaczony)"));
    LocalSignalStateButton->SetSize(LocalSignalStateButton->GetBestSize());
    TaskActions_sizer->Add(LocalSignalStateButton, 0, 0, 0);
    wxStaticLine* static_line_8 = new wxStaticLine(page1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL);
    TaskActions_sizer->Add(static_line_8, 0, wxALL|wxEXPAND, 5);
    ResetSettingsButton = new wxBitmapButton(page1, ID_RESET_SETTINGS, wxBitmap( reset_xpm ));
    ResetSettingsButton->SetToolTip(_("Przywroc ustawienia domyslne"));
    ResetSettingsButton->SetSize(ResetSettingsButton->GetBestSize());
    TaskActions_sizer->Add(ResetSettingsButton, 0, 0, 0);
    wxStaticBoxSizer* NetParams_sizer = new wxStaticBoxSizer(new wxStaticBox(page1, wxID_ANY, _("Konfiguracja polaczenia")), wxVERTICAL);
    sizerPanel->Add(NetParams_sizer, 0, wxEXPAND, 0);
    wxBoxSizer* subsizer = new wxBoxSizer(wxHORIZONTAL);
    NetParams_sizer->Add(subsizer, 0, wxALL|wxEXPAND, 5);
    WorksAsServer = new wxRadioButton(page1, ID_work_as_server, _("serwer"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    subsizer->Add(WorksAsServer, 1, wxBOTTOM, 5);
    WorksAsClient = new wxRadioButton(page1, ID_work_as_client, _("klient"));
    subsizer->Add(WorksAsClient, 1, wxBOTTOM, 5);
    ServerAddressEdit = new wxTextCtrl(page1, ID_server_address, _("xxx"), wxDefaultPosition, wxDefaultSize, wxTE_LEFT);
    NetParams_sizer->Add(ServerAddressEdit, 0, wxBOTTOM|wxEXPAND, 5);
    wxBoxSizer* subsizer_1 = new wxBoxSizer(wxHORIZONTAL);
    NetParams_sizer->Add(subsizer_1, 0, wxBOTTOM|wxEXPAND, 5);
    wxStaticText* SamplingRateST = new wxStaticText(page1, wxID_ANY, _("Szybkosc probkowania:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
    subsizer_1->Add(SamplingRateST, 0, wxALIGN_CENTER_VERTICAL|wxALL|wxFIXED_MINSIZE, 5);
    const wxString SamplingRateBox_choices[] = {
        _("8000"),
        _("11025"),
        _("16000"),
        _("22050"),
        _("32000"),
        _("44100"),
        _("48000"),
    };
    SamplingRateBox = new wxComboBox(page1, ID_SELECT_SAMPLING_RATE, wxT(""), wxDefaultPosition, wxDefaultSize, 7, SamplingRateBox_choices, wxCB_DROPDOWN|wxCB_READONLY);
    SamplingRateBox->SetSelection(2);
    subsizer_1->Add(SamplingRateBox, 1, wxBOTTOM|wxEXPAND|wxLEFT|wxTOP, 5);
    wxStaticBoxSizer* GraphConfig_sizer = new wxStaticBoxSizer(new wxStaticBox(page1, wxID_ANY, _("Konfiguracja wykresow")), wxVERTICAL);
    sizerPanel->Add(GraphConfig_sizer, 0, wxEXPAND, 0);
    const wxString DrawModeBox_choices[] = {
        _("Bez wykresów"),
        _("Przebieg czasowy"),
        _("Histogram"),
        _("Periodogram"),
        _("Spektrogram"),
        _("Konstelacja"),
        _("Wykres oczkowy"),
    };
    DrawModeBox = new wxComboBox(page1, ID_SELECT_DRAW_MODE, wxT(""), wxDefaultPosition, wxDefaultSize, 7, DrawModeBox_choices, wxCB_DROPDOWN|wxCB_READONLY);
    DrawModeBox->SetSelection(0);
    GraphConfig_sizer->Add(DrawModeBox, 1, wxBOTTOM|wxEXPAND|wxTOP, 5);
    wxBoxSizer* sizer_25 = new wxBoxSizer(wxHORIZONTAL);
    GraphConfig_sizer->Add(sizer_25, 1, wxEXPAND, 0);
    PSD_slots_text = new wxTextCtrl(page1, wxID_ANY, _("200"), wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    sizer_25->Add(PSD_slots_text, 3, wxLEFT|wxRIGHT|wxTOP, 10);
    ResizeToSquare = new wxBitmapButton(page1, ID_RESIZE_TO_SQUARE, wxBitmap( resize_xpm ));
    ResizeToSquare->SetToolTip(_("Proporcja 1:1"));
    ResizeToSquare->SetSize(ResizeToSquare->GetBestSize());
    sizer_25->Add(ResizeToSquare, 1, wxLEFT|wxRIGHT|wxTOP, 10);
    PSD_slots_slider = new wxSlider(page1, ID_PSD_SLOTS_SLIDER, 2, 0, 6);
    GraphConfig_sizer->Add(PSD_slots_slider, 0, wxBOTTOM|wxEXPAND|wxTOP, 5);
    wxStaticBoxSizer* AudioMixer_sizer = new wxStaticBoxSizer(new wxStaticBox(page1, wxID_ANY, _("Ustawienia karty dzwiekowej")), wxVERTICAL);
    sizerPanel->Add(AudioMixer_sizer, 0, wxEXPAND, 0);
    wxBoxSizer* sizer_7 = new wxBoxSizer(wxHORIZONTAL);
    AudioMixer_sizer->Add(sizer_7, 1, wxEXPAND|wxLEFT|wxRIGHT, 5);
    wxStaticText* SourceLine_ST = new wxStaticText(page1, wxID_ANY, _("Nagrywanie:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
    sizer_7->Add(SourceLine_ST, 1, wxALIGN_CENTER_VERTICAL|wxALL|wxFIXED_MINSIZE, 5);
    const wxString SourceLine_ComboBox_choices[] = {};
    SourceLine_ComboBox = new wxComboBox(page1, ID_SELECT_MIXER_SOURCE_LINE, wxT(""), wxDefaultPosition, wxDefaultSize, 0, SourceLine_ComboBox_choices, wxCB_DROPDOWN|wxCB_READONLY);
    sizer_7->Add(SourceLine_ComboBox, 8, wxBOTTOM|wxEXPAND|wxLEFT|wxTOP, 5);
    SourceLine_slider = new wxSlider(page1, ID_SourceLine_SLIDER, 0, 0, 10);
    AudioMixer_sizer->Add(SourceLine_slider, 0, wxEXPAND, 0);
    wxBoxSizer* sizer_8 = new wxBoxSizer(wxHORIZONTAL);
    AudioMixer_sizer->Add(sizer_8, 1, wxEXPAND|wxLEFT|wxRIGHT, 5);
    wxStaticText* DestLine_ST = new wxStaticText(page1, wxID_ANY, _("Odtwarzanie:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
    sizer_8->Add(DestLine_ST, 1, wxALIGN_CENTER_VERTICAL|wxALL|wxFIXED_MINSIZE, 5);
    const wxString DestLine_ComboBox_choices[] = {};
    DestLine_ComboBox = new wxComboBox(page1, ID_SELECT_MIXER_DEST_LINE, wxT(""), wxDefaultPosition, wxDefaultSize, 0, DestLine_ComboBox_choices, wxCB_DROPDOWN|wxCB_READONLY);
    sizer_8->Add(DestLine_ComboBox, 8, wxBOTTOM|wxLEFT|wxTOP, 5);
    DestLine_slider = new wxSlider(page1, ID_DestLine_SLIDER, 0, 0, 10);
    AudioMixer_sizer->Add(DestLine_slider, 0, wxEXPAND, 0);
    wxStaticLine* static_line_1 = new wxStaticLine(page1, wxID_ANY);
    AudioMixer_sizer->Add(static_line_1, 0, wxBOTTOM|wxEXPAND|wxTOP, 10);
    wxBoxSizer* sizer_9 = new wxBoxSizer(wxHORIZONTAL);
    AudioMixer_sizer->Add(sizer_9, 1, wxEXPAND, 0);
    wxStaticText* MasterLine_ST = new wxStaticText(page1, wxID_ANY, _("Głosnosc glowna:"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
    sizer_9->Add(MasterLine_ST, 0, wxALL, 5);
    MasterLine_slider = new wxSlider(page1, ID_MasterLine_SLIDER, 0, 0, 10);
    sizer_9->Add(MasterLine_slider, 1, wxEXPAND, 0);
    page2 = new wxPanel(notebookWindow, E_PID_CHANNEL);
    notebookWindow->AddPage(page2, _("Model kanalu"));
    wxBoxSizer* sizer_3 = new wxBoxSizer(wxVERTICAL);
    wxStaticBoxSizer* sizer_2 = new wxStaticBoxSizer(new wxStaticBox(page2, wxID_ANY, _("Model kanału")), wxVERTICAL);
    sizer_3->Add(sizer_2, 0, wxEXPAND, 0);
    SNR_text = new wxTextCtrl(page2, wxID_ANY, _("80 dB"), wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    sizer_2->Add(SNR_text, 0, wxALL|wxEXPAND, 10);
    SNR_slider = new wxSlider(page2, ID_SNR_SLIDER, 100, 0, 100);
    sizer_2->Add(SNR_slider, 0, wxALL|wxEXPAND, 5);
    wxStaticLine* static_line_2 = new wxStaticLine(page2, wxID_ANY);
    sizer_2->Add(static_line_2, 0, wxBOTTOM|wxEXPAND|wxTOP, 10);
    HPF_slider = new wxSlider(page2, ID_HPF_SLIDER, 0, 0, 100);
    sizer_2->Add(HPF_slider, 0, wxEXPAND|wxLEFT|wxRIGHT|wxTOP, 5);
    wxGridSizer* sizer_4 = new wxGridSizer(1, 2, 0, 5);
    sizer_2->Add(sizer_4, 1, wxEXPAND|wxLEFT|wxRIGHT, 10);
    HPF_text = new wxTextCtrl(page2, wxID_ANY, _("0 Hz"), wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    sizer_4->Add(HPF_text, 0, wxALIGN_CENTER_VERTICAL, 0);
    LPF_text = new wxTextCtrl(page2, wxID_ANY, _("8000 Hz"), wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    sizer_4->Add(LPF_text, 0, wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT, 0);
    LPF_slider = new wxSlider(page2, ID_LPF_SLIDER, 100, 0, 100);
    sizer_2->Add(LPF_slider, 0, wxBOTTOM|wxEXPAND|wxLEFT|wxRIGHT, 5);
    sizer_3->Add(20, 20, 0, 0, 0);
    sizer_5 = new wxStaticBoxSizer(new wxStaticBox(page2, wxID_ANY, _("Sygnal mowy")), wxVERTICAL);
    sizer_3->Add(sizer_5, 0, wxEXPAND, 0);
    wxBoxSizer* sizer_6 = new wxBoxSizer(wxHORIZONTAL);
    sizer_5->Add(sizer_6, 0, wxALL|wxEXPAND, 5);
    UseLogatoms = new wxRadioButton(page2, ID_use_logatoms, _("Logatomy"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    sizer_6->Add(UseLogatoms, 0, 0, 0);
    UseSentences = new wxRadioButton(page2, ID_use_sentences, _("Zdania"));
    sizer_6->Add(UseSentences, 0, 0, 0);
    wxBoxSizer* sizer_10 = new wxBoxSizer(wxHORIZONTAL);
    sizer_5->Add(sizer_10, 0, wxALL|wxEXPAND, 5);
    wxStaticText* VoiceTypeST = new wxStaticText(page2, wxID_ANY, _("Glos:"));
    sizer_10->Add(VoiceTypeST, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    const wxString VoiceTypeBox_choices[] = {
        _("losowo"),
        _("męski 1"),
        _("męski 2 "),
        _("żeński 1 "),
        _("żeński 2"),
    };
    VoiceTypeBox = new wxComboBox(page2, ID_voice_type, wxT(""), wxDefaultPosition, wxDefaultSize, 5, VoiceTypeBox_choices, wxCB_DROPDOWN|wxCB_READONLY);
    VoiceTypeBox->SetSelection(0);
    sizer_10->Add(VoiceTypeBox, 1, wxALIGN_CENTER_VERTICAL, 0);
    wxBoxSizer* sizer_11 = new wxBoxSizer(wxHORIZONTAL);
    sizer_5->Add(sizer_11, 0, wxALL|wxEXPAND, 5);
    SelectVoiceFile = new wxButton(page2, ID_select_voice_file, _("Losuj"));
    sizer_11->Add(SelectVoiceFile, 0, wxBOTTOM|wxRIGHT|wxTOP, 5);
    VoiceFileIndex = new wxTextCtrl(page2, ID_select_voice_file, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    sizer_11->Add(VoiceFileIndex, 1, wxBOTTOM|wxRIGHT|wxTOP, 5);
    wxBoxSizer* sizer_12 = new wxBoxSizer(wxHORIZONTAL);
    sizer_11->Add(sizer_12, 0, wxEXPAND, 0);
    OpenWAVEfile = new wxBitmapButton(page2, ID_open_wav_file, wxBitmap( play_xpm ));
    OpenWAVEfile->SetSize(OpenWAVEfile->GetBestSize());
    sizer_12->Add(OpenWAVEfile, 1, wxALIGN_CENTER_VERTICAL, 0);
    StopWAVEfile = new wxBitmapButton(page2, ID_stop_wav_file, wxBitmap( stop_xpm ));
    StopWAVEfile->SetSize(StopWAVEfile->GetBestSize());
    sizer_12->Add(StopWAVEfile, 1, wxALIGN_CENTER_VERTICAL, 0);
    wxStaticLine* static_line_5 = new wxStaticLine(page2, wxID_ANY);
    sizer_5->Add(static_line_5, 0, wxALL|wxEXPAND, 5);
    showSentenceText = new wxCheckBox(page2, ID_show_text_checkbox, _("Pokaz tekst"));
    sizer_5->Add(showSentenceText, 0, wxALL, 5);
    SentenceTranscription = new wxTextCtrl(page2, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    SentenceTranscription->Enable(0);
    SentenceTranscription->Hide();
    sizer_5->Add(SentenceTranscription, 0, wxALL|wxEXPAND|wxRESERVE_SPACE_EVEN_IF_HIDDEN, 5);
    page3 = new wxPanel(notebookWindow, E_PID_MORSE);
    notebookWindow->AddPage(page3, _("Telegraf"));
    wxBoxSizer* sizer_13 = new wxBoxSizer(wxVERTICAL);
    wxStaticBoxSizer* sizer_14 = new wxStaticBoxSizer(new wxStaticBox(page3, wxID_ANY, _("Szybkosc kluczowania")), wxVERTICAL);
    sizer_13->Add(sizer_14, 0, wxEXPAND, 0);
    WPM_text = new wxTextCtrl(page3, wxID_ANY, _("20 WPM"), wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    sizer_14->Add(WPM_text, 0, wxEXPAND|wxLEFT|wxRIGHT|wxTOP, 5);
    WPM_slider = new wxSlider(page3, ID_WPM_SLIDER, 3, 0, 5);
    sizer_14->Add(WPM_slider, 0, wxBOTTOM|wxEXPAND|wxLEFT|wxRIGHT, 5);
    wxStaticBoxSizer* sizer_15 = new wxStaticBoxSizer(new wxStaticBox(page3, wxID_ANY, _("Nadajnik")), wxVERTICAL);
    sizer_13->Add(sizer_15, 1, wxEXPAND, 0);
    wxStaticText* AsciiTextST = new wxStaticText(page3, wxID_ANY, _("Nadawany tekst:"));
    sizer_15->Add(AsciiTextST, 0, 0, 0);
    AsciiTextEntry = new wxTextCtrl(page3, ID_ascii_text, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_LEFT|wxTE_MULTILINE|wxTE_NO_VSCROLL);
    sizer_15->Add(AsciiTextEntry, 1, wxALL|wxEXPAND, 5);
    sizer_18 = new wxBoxSizer(wxHORIZONTAL);
    sizer_15->Add(sizer_18, 0, wxEXPAND, 0);
    wxStaticText* label_2 = new wxStaticText(page3, wxID_ANY, _("Reczne nadawanie:"));
    sizer_18->Add(label_2, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    TextCtrlPlaceholder = new wxTextCtrl(page3, wxID_ANY, wxEmptyString);
    sizer_18->Add(TextCtrlPlaceholder, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    SendAsciiText = new wxButton(page3, ID_send_ascii_text, _("Wyslij"));
    sizer_18->Add(SendAsciiText, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    wxStaticBoxSizer* sizer_16 = new wxStaticBoxSizer(new wxStaticBox(page3, wxID_ANY, _("Odbiornik")), wxVERTICAL);
    sizer_13->Add(sizer_16, 1, wxEXPAND, 0);
    MorseReceiverState = new wxCheckBox(page3, ID_morse_receiver_state, _("wlaczony"));
    sizer_16->Add(MorseReceiverState, 0, wxALL, 5);
    AsciiTextReceiver = new wxTextCtrl(page3, ID_received_ascii_text, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_LEFT|wxTE_MULTILINE|wxTE_NO_VSCROLL|wxTE_READONLY);
    sizer_16->Add(AsciiTextReceiver, 1, wxEXPAND, 0);
    page5 = new wxPanel(notebookWindow, E_PID_status);
    notebookWindow->AddPage(page5, _("Status programu"));
    wxBoxSizer* sizer_17 = new wxBoxSizer(wxHORIZONTAL);
    StatusBox = new wxTextCtrl(page5, ID_STATUSBOX, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_RICH|wxTE_WORDWRAP);
    sizer_17->Add(StatusBox, 1, wxEXPAND, 0);
    page6 = new wxScrolledWindow(notebookWindow, E_PID_MODULATOR, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    page6->SetScrollRate(0, 10);
    notebookWindow->AddPage(page6, _("Modulacja"));
    wxBoxSizer* sizer_1 = new wxBoxSizer(wxVERTICAL);
    wxStaticBoxSizer* sizer_21 = new wxStaticBoxSizer(new wxStaticBox(page6, wxID_ANY, _("Wybor modulacji")), wxHORIZONTAL);
    sizer_1->Add(sizer_21, 0, wxEXPAND, 0);
    const wxString ModulationTypeBox_choices[] = {
        _("ASK"),
        _("PSK"),
        _("QAM"),
    };
    ModulationTypeBox = new wxComboBox(page6, ID_SELECT_MODULATOR_TYPE, wxT(""), wxDefaultPosition, wxDefaultSize, 3, ModulationTypeBox_choices, wxCB_DROPDOWN|wxCB_READONLY);
    ModulationTypeBox->SetSelection(1);
    sizer_21->Add(ModulationTypeBox, 1, wxBOTTOM|wxLEFT|wxTOP, 3);
    const wxString ModulatorVariantSelect_choices[] = {
        _("1"),
        _("2"),
        _("3"),
        _("4"),
    };
    ModulatorVariantSelect = new wxChoice(page6, ID_SELECT_MODULATOR_VARIANT, wxDefaultPosition, wxDefaultSize, 4, ModulatorVariantSelect_choices);
    ModulatorVariantSelect->SetSelection(0);
    sizer_21->Add(ModulatorVariantSelect, 1, wxBOTTOM|wxRIGHT|wxTOP, 3);
    wxStaticBoxSizer* sizer_19 = new wxStaticBoxSizer(new wxStaticBox(page6, wxID_ANY, _("Modulator")), wxVERTICAL);
    sizer_1->Add(sizer_19, 0, wxEXPAND, 0);
    ModulatorState = new wxCheckBox(page6, ID_modulator_state, _("Wlacz modulator"));
    sizer_19->Add(ModulatorState, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 11);
    wxStaticBoxSizer* sizer_23 = new wxStaticBoxSizer(new wxStaticBox(page6, wxID_ANY, _("Parametry modulatora")), wxVERTICAL);
    sizer_19->Add(sizer_23, 1, wxEXPAND, 0);
    wxGridSizer* grid_sizer_1 = new wxGridSizer(3, 2, 2, 0);
    sizer_23->Add(grid_sizer_1, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxSHAPED, 0);
    wxStaticText* label_3 = new wxStaticText(page6, wxID_ANY, _("N_symb:"));
    grid_sizer_1->Add(label_3, 0, 0, 0);
    NsymbText = new wxTextCtrl(page6, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    grid_sizer_1->Add(NsymbText, 0, 0, 0);
    wxStaticText* label_7 = new wxStaticText(page6, wxID_ANY, _("f_symb"));
    grid_sizer_1->Add(label_7, 0, 0, 0);
    f_symb1Text = new wxTextCtrl(page6, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    grid_sizer_1->Add(f_symb1Text, 0, 0, 0);
    wxStaticText* label_8 = new wxStaticText(page6, wxID_ANY, _("bit_per_sample"));
    grid_sizer_1->Add(label_8, 0, 0, 0);
    BitPerSampleText = new wxTextCtrl(page6, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    grid_sizer_1->Add(BitPerSampleText, 0, 0, 0);
    wxStaticLine* static_line_6 = new wxStaticLine(page6, wxID_ANY);
    sizer_23->Add(static_line_6, 0, wxBOTTOM|wxEXPAND|wxTOP, 8);
    wxGridSizer* grid_sizer_2 = new wxGridSizer(3, 2, 2, 0);
    sizer_23->Add(grid_sizer_2, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxSHAPED, 0);
    wxStaticText* label_4 = new wxStaticText(page6, wxID_ANY, _("T_symb"));
    grid_sizer_2->Add(label_4, 0, 0, 0);
    TsymbText = new wxTextCtrl(page6, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    grid_sizer_2->Add(TsymbText, 0, 0, 0);
    wxStaticText* label_5 = new wxStaticText(page6, wxID_ANY, _("F_symb"));
    grid_sizer_2->Add(label_5, 0, 0, 0);
    F_symb2Text = new wxTextCtrl(page6, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    grid_sizer_2->Add(F_symb2Text, 0, 0, 0);
    wxStaticText* label_6 = new wxStaticText(page6, wxID_ANY, _("bps"));
    grid_sizer_2->Add(label_6, 0, 0, 0);
    bpsText = new wxTextCtrl(page6, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    grid_sizer_2->Add(bpsText, 0, 0, 0);
    wxStaticLine* static_line_7 = new wxStaticLine(page6, wxID_ANY);
    sizer_23->Add(static_line_7, 0, wxBOTTOM|wxEXPAND|wxTOP, 8);
    wxBoxSizer* sizer_22 = new wxBoxSizer(wxHORIZONTAL);
    sizer_23->Add(sizer_22, 0, wxALL|wxEXPAND, 1);
    wxStaticText* label_1 = new wxStaticText(page6, wxID_ANY, _("Czestotliwosc nosna (f_o / F_o):"));
    sizer_22->Add(label_1, 1, 0, 0);
    CarrierFreqTextCtrl = new wxTextCtrl(page6, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_LEFT|wxTE_READONLY);
    sizer_22->Add(CarrierFreqTextCtrl, 1, wxEXPAND, 0);
    CarrierFreqSlider = new wxSlider(page6, ID_carrier_freq_SLIDER, 50, 0, 100);
    CarrierFreqSlider->SetToolTip(_("Wybór częstotliwości nośnej"));
    CarrierFreqSlider->Enable(0);
    sizer_23->Add(CarrierFreqSlider, 0, wxEXPAND, 0);
    wxStaticBoxSizer* sizer_20 = new wxStaticBoxSizer(new wxStaticBox(page6, wxID_ANY, _("Demodulator")), wxVERTICAL);
    sizer_1->Add(sizer_20, 0, wxEXPAND|wxTOP, 23);
    demodState = new wxCheckBox(page6, ID_demod_state, _("Wlacz demodulator"));
    sizer_20->Add(demodState, 0, wxALIGN_CENTER_HORIZONTAL|wxBOTTOM|wxTOP, 10);
    wxBoxSizer* sizer_24 = new wxBoxSizer(wxHORIZONTAL);
    sizer_20->Add(sizer_24, 0, wxALL|wxEXPAND, 0);
    wxStaticText* label_9 = new wxStaticText(page6, wxID_ANY, _("Czestotliwosc nosna:"));
    sizer_24->Add(label_9, 1, wxALIGN_CENTER_VERTICAL|wxALL, 0);
    DemodCarrierFreqTextCtrl = new wxTextCtrl(page6, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
    sizer_24->Add(DemodCarrierFreqTextCtrl, 1, wxLEFT|wxRIGHT, 7);
    DemodCarrierFreq = new wxSlider(page6, ID_demod_carrier_freq_SLIDER, 50, 0, 100);
    DemodCarrierFreq->SetToolTip(_("Wybor czestotliwosci nosnej"));
    DemodCarrierFreq->Enable(0);
    sizer_20->Add(DemodCarrierFreq, 0, wxEXPAND|wxLEFT|wxRIGHT, 3);
    wxStaticText* label_11 = new wxStaticText(page6, wxID_ANY, _("Opoznienie (Sa):"));
    sizer_20->Add(label_11, 1, wxTOP, 10);
    DemodDelay = new wxSlider(page6, ID_demod_delay_SLIDER, 0, 0, 50, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL|wxSL_LABELS);
    DemodDelay->SetToolTip(_("Wybor opoznienia demodulatora"));
    DemodDelay->Enable(0);
    sizer_20->Add(DemodDelay, 0, wxALL|wxEXPAND, 3);
    wxStaticText* label_12 = new wxStaticText(page6, wxID_ANY, _("Korekta fazy:"));
    sizer_20->Add(label_12, 1, wxTOP, 3);
    wxBoxSizer* sizer_26 = new wxBoxSizer(wxHORIZONTAL);
    sizer_20->Add(sizer_26, 0, wxEXPAND, 0);
    wxStaticText* label_13 = new wxStaticText(page6, wxID_ANY, _("-pi"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
    sizer_26->Add(label_13, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 3);
    DemodCarrierOffset = new wxSlider(page6, ID_demod_carrieroffset_SLIDER, 0, -180, 180);
    DemodCarrierOffset->SetToolTip(_("Korekta fazy"));
    DemodCarrierOffset->Enable(0);
    sizer_26->Add(DemodCarrierOffset, 1, wxEXPAND|wxLEFT|wxRIGHT, 3);
    wxStaticText* label_14 = new wxStaticText(page6, wxID_ANY, _("pi"), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
    sizer_26->Add(label_14, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 3);
    wxStaticText* label_15 = new wxStaticText(page6, wxID_ANY, _("Wzmocnienie:"));
    sizer_20->Add(label_15, 1, wxTOP, 3);
    wxBoxSizer* sizer_27 = new wxBoxSizer(wxHORIZONTAL);
    sizer_20->Add(sizer_27, 1, wxEXPAND, 0);
    wxStaticText* label_16 = new wxStaticText(page6, wxID_ANY, _("0"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
    sizer_27->Add(label_16, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 3);
    DemodGain = new wxSlider(page6, ID_demod_gain_SLIDER, 100, 0, 400);
    DemodGain->SetToolTip(_("Wybor wzmocnienia"));
    DemodGain->Enable(0);
    sizer_27->Add(DemodGain, 1, wxEXPAND|wxLEFT|wxRIGHT, 3);
    wxStaticText* label_17 = new wxStaticText(page6, wxID_ANY, _("4"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
    sizer_27->Add(label_17, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 3);
    DemodulatorGainText = new wxStaticText(page6, wxID_ANY, _("1,00"));
    DemodulatorGainText->SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, 0, wxT("Segoe UI")));
    DemodulatorGainText->Enable(0);
    sizer_20->Add(DemodulatorGainText, 0, wxALIGN_CENTER_HORIZONTAL|wxBOTTOM, 3);
    wxBoxSizer* sizer_28 = new wxBoxSizer(wxHORIZONTAL);
    sizer_20->Add(sizer_28, 1, wxEXPAND, 0);
    wxStaticText* label_10 = new wxStaticText(page6, wxID_ANY, _("Zrodlo bufora"));
    sizer_28->Add(label_10, 2, 0, 0);
    const wxString EyeBufferSource_choices[] = {
        _("Wyjscie kanalu"),
        _("Po filtracji"),
    };
    EyeBufferSource = new wxChoice(page6, ID_SELECT_EYEBUFFER_SOURCE, wxDefaultPosition, wxDefaultSize, 2, EyeBufferSource_choices);
    EyeBufferSource->SetSelection(0);
    sizer_28->Add(EyeBufferSource, 1, 0, 0);
    
    page6->SetSizer(sizer_1);
    page5->SetSizer(sizer_17);
    page3->SetSizer(sizer_13);
    page2->SetSizer(sizer_3);
    page1->SetSizer(sizerPanel);
    sashWindow->SplitVertically(GLcanvas, notebookWindow, 634);
    Layout();
}


BEGIN_EVENT_TABLE(GUIFrame, wxFrame)
    // begin wxGlade: GUIFrame::event_table
    // end wxGlade
END_EVENT_TABLE();


void GUIFrame::OnLanguageChange(wxCommandEvent &event)  // wxGlade: GUIFrame.<event_handler>
{
    event.Skip();
    // notify the user that he hasn't implemented the event handler yet
    wxLogDebug(wxT("Event handler (GUIFrame::OnLanguageChange) not implemented yet"));
}


// wxGlade: add GUIFrame event handlers

