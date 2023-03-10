#ifndef _MyGLCanvas_
#define _MyGLCanvas_

#include <wx/wx.h>
#include <wx/glcanvas.h>

#include "BitmapFont.h"
#include "OutlineFont.h"

#include "Misc.h"

class T_SpectrogramDraw;
class T_DSPlib_processing;

class MyFrame;

class MyGLCanvas: public wxGLCanvas
{
  private:
    //! canvas index in Parent window
    unsigned int this_canvas_index;
    //! if true drawing should be blocked (processing loop doesn't work)
    static bool BlockDrawing;
    static bool temporary_BlockDrawing;

    GLint glc_context_id;
    GLint glc_font_id;

    bool SocketsAreConnected;
    E_DrawModes DrawMode;
    float PSD_max_dB, PSD_range_dB;

    char *x_label, *x_units, *y_label, *y_units;
    static const char *psd_x_label, *psd_x_units, *psd_y_label, *psd_y_units;
    static const char *spec_x_label, *spec_x_units, *spec_y_label, *spec_y_units;
    static const char *hist_x_label, *hist_x_units, *hist_y_label, *hist_y_units;
    static const char *sig_x_label, *sig_x_units, *sig_y_label, *sig_y_units;
    bool GetCords(T_DSPlib_processing *Object, float &x_in, float &y_in);

    bool OnMouseDown(int x, int y);
    bool OnMouseUp(int x, int y);
    void OnMouseMove(int x, int y, bool captured);

  public:
    CBitmapFont  *m_bmf;
    COutlineFont *m_olf;

    E_DrawModes GetDrawMode(void);
    void SetDrawMode(E_DrawModes draw_mode_in);

  public:
    MyFrame *ParentFrame;
    wxGLContext *GLcontext;

    void LockDrawingData(unsigned int GLcanvas_index);
    void UnlockDrawingData(unsigned int GLcanvas_index);

    //! controls access to draw data
    wxSemaphore *DrawData_semaphore;

    MyGLCanvas(unsigned int CanvasIndex,
        MyFrame *parent_in, wxWindowID id = wxID_ANY,
        int* attribList = 0,
        const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
        long style = 0, const wxString& name = _T("MyGLCanvas") );
    ~MyGLCanvas(void);

    static void EnableDrawing(bool permament = true);
    static void DisableDrawing(bool permament = true);

    T_SpectrogramDraw *SpecDraw;
    void OnDrawNow( wxCommandEvent &event );
    void OnDrawNow_( void );
    void DrawPSD(int width, int height);
    void DrawSpectrogram(int width, int height);
    void DrawSignal(int width, int height);
    void DrawHistogram(int width, int height);

    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnEraseBackground(wxEraseEvent& event);
    void OnEnterWindow(wxMouseEvent& event);

  public: //private:
    bool mouse_captured;
  public:
    void OnMouseLeftDown(wxMouseEvent& event);
    void OnMouseLeftUp(wxMouseEvent& event);
    void OnMouseMotion(wxMouseEvent& event);

    void Render();
    //! returns true if full init was performed and false if init was done before
    bool InitGL(void);
    void Action( long code, unsigned long lasttime,
          unsigned long acceltime );

  private:
    bool   m_init;
//    GLuint m_gllist;
//    long   m_rleft;
//    long   m_rright;

    DECLARE_EVENT_TABLE()
};



#endif
