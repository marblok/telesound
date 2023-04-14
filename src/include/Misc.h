/*
 * Misc.h
 *
 *  Created on: 2008-10-20
 *      Author: Marek
 */

#ifndef MISC_H_
#define MISC_H_

#include <DSP_lib.h>
#include <DSP_modules_misc.h>

#include <wxAddons.h>

//! \todo implement other modes
enum E_DrawModes
{
  E_DM_none = 0,
  E_DM_signal = 1,
  E_DM_histogram = 2,
  E_DM_psd = 4,
  E_DM_spectrogram = 8
};

enum E_UpdateState
{
  E_US_none = 0,
  E_US_audio_in_gain = 1,
  E_US_noise_level = 2,
  E_US_channel_LPF_coefs = 4,
  E_US_channel_HPF_coefs = 8,
  E_US_ascii_text = 16,
  E_US_WPM_change = 32,
  E_US_wav_file_open = 64,
  E_US_local_signal = 128,
  E_US_morse_receiver_state = 256,
  E_US_high_res_psd = 512
};
E_UpdateState& operator|= (E_UpdateState& left, const E_UpdateState& right);
E_UpdateState& operator&= (E_UpdateState& left, const E_UpdateState& right);
E_UpdateState& operator^= (E_UpdateState& left, const E_UpdateState& right);
E_UpdateState operator! (const E_UpdateState& right);

//! Processing specification list element
/*!
 * what about time zones ???
 * wxDateTime
 * wxTimeSpan
 * wxDaySpan
 */
class T_ProcessingSpec
{
  friend class T_InterfaceState;

  private:
    // draw mode
    //E_DrawModes draw_mode;

    //! pointer to the next processing specification segment
    T_ProcessingSpec *Next;

  public:
    bool run_as_server;
//    char IP_address[1024];
    std::string IP_address;
    long SamplingRate;
    float time_span;

    std::string wav_filename;

    int no_of_psd_slots;

    int WPM;
    float SNR_dB;

    bool MikeIsOff;
    float local_signal_gain;

    bool ChannelFilterON;
    float ChannelFd;
    float ChannelFg;

    bool morse_receiver_state;

    const E_DrawModes Get_draw_mode(void);
    void Set_draw_mode(E_DrawModes new_draw_mode);

    T_ProcessingSpec *GetNext(void);


    /*! \note the class does not take ownership of the *file_time_data_in object
     *
     *  \note all calculations rely on the sampling rate
     *    specified in file_time_data_in object
     */
    T_ProcessingSpec(void);
    ~T_ProcessingSpec(void);

    //! Delete all the T_ProcessingSpec objects on the list
    /*! \warning current task (this) is also deleted
     */
    void DeleteList(void);
};

class wxMorseValidator: public wxValidator
{
  public:
    //! prec_in - number of digits after decimal point (== 0 - simple integer number)
    /*! \todo implement allowed value range setting (eg. 0-59 for seconds)
     *    - test during edition
     *    - test only when validating
     *    .
     *
     *  \note if prec_in < 0 then add preceding zeroes to the integer number
     *    so the string is -prec_in characters long
     *
     *  \todo Implements support for forcing limited precision (prec_in > 0)
     */
    wxMorseValidator(int prec_in, E_NV_mode mode_in, // = E_NV_any,
                   long double *m_long_double_Value_in = NULL);
    wxMorseValidator(const wxMorseValidator& val);

    virtual ~wxMorseValidator(){}

    // Make a clone of this validator (or return NULL) - currently necessary
    // if you're passing a reference to a validator.
    // Another possibility is to always pass a pointer to a new validator
    // (so the calling code can use a copy constructor of the relevant class).
    virtual wxObject *Clone() const { return new wxMorseValidator(*this); }
    bool Copy(const wxMorseValidator& val);

    // Called when the value in the window must be validated.
    // This function can pop up an error message.
    virtual bool Validate(wxWindow *parent);

    /*! Called to transfer data to the window.
     *  Here we should get internal ascii buffer from  current_morse_block.
     */
    virtual bool TransferToWindow();

    /*! Called to transfer data from the window.
     *  Here we should set internal ascii buffer of current_morse_block
     *  with content of the text control.
     */
    virtual bool TransferFromWindow();

    //! Filter keystrokes
    void OnChar(wxKeyEvent& event);
    //! Monitors text changes
    void OnTextChange( wxCommandEvent &event );


    DECLARE_EVENT_TABLE();

    /*! if morse_block == NULL remove connection to current morse key block
     */
    void SetMorseKey(DSP::u::MORSEkey *morse_block = NULL);

  protected:
    DSP::u::MORSEkey *current_morse_block;

    bool CheckValidator() const
    {
        wxCHECK_MSG( m_validatorWindow, false,
                     _T("No window associated with validator") );
        wxCHECK_MSG( m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)), false,
                     _T("wxMorseValidator is only for wxTextCtrl's") );

        return true;
    }

  private:

    // Cannot use
    //  DECLARE_NO_COPY_CLASS(wxTextValidator)
    // because copy constructor is explicitly declared above;
    // but no copy assignment operator is defined, so declare
    // it private to prevent the compiler from defining it:
    wxMorseValidator& operator=(const wxNumValidator&);
};

#endif /* MISC_H_ */
