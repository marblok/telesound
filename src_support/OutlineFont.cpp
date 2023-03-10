#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <windows.h>

#include "OutlineFont.h"

/*! \todo_later wglUseFontOutlines fails with error code 8, see to it later
 */
COutlineFont::COutlineFont(
    wxGLCanvas* win, //CDC* dc,
    wchar_t* fontname)
{
    // Class constructor.
    // Stores each character in its own display list
    // for later drawing via the wglUseFontOutlines() call.

    //if (win && fontname && strlen(fontname) > 0) {
    if (win && fontname && wcslen(fontname) > 0) {

        m_listbase = glGenLists(256);

        // Setup the Font characteristics
        LOGFONT logfont;
        GLYPHMETRICSFLOAT gmf[256];

        logfont.lfHeight        = -12;
        logfont.lfWidth         = 0;
        logfont.lfEscapement    = 0;
        logfont.lfOrientation   = logfont.lfEscapement;
        logfont.lfWeight        = FW_NORMAL;
        logfont.lfItalic        = FALSE;
        logfont.lfUnderline     = FALSE;
        logfont.lfStrikeOut     = FALSE;
        logfont.lfCharSet       = ANSI_CHARSET;
        logfont.lfOutPrecision  = OUT_DEFAULT_PRECIS;
        logfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
        logfont.lfQuality       = DEFAULT_QUALITY;
        logfont.lfPitchAndFamily = FF_DONTCARE|DEFAULT_PITCH;
        //strcpy(logfont.lfFaceName, fontname);
        // https://docs.microsoft.com/pl-pl/cpp/c-runtime-library/reference/strcpy-wcscpy-mbscpy?view=vs-2019
        wcscpy(logfont.lfFaceName, fontname);

        HFONT font;
        HFONT oldfont;
        HDC m_hDC;


        m_hDC = (HDC)(win->GetHDC());
        //        m_hDC = ::GetDC((HWND)(win->GetHandle()));
        //        m_hDC = ::GetDC((HWND)(NULL)); // (HDC)(win->GetHDC());

        font = CreateFontIndirect(&logfont);
        oldfont = (HFONT)SelectObject(m_hDC, &font);
        if (FALSE == wglUseFontOutlines(
//        if (FALSE == wglUseFontOutlinesA(
                m_hDC,
                0,
                256,
                m_listbase,
                0.0,
                0.0,
                WGL_FONT_POLYGONS,
                gmf)) {
          /*
          DWORD err_no;
          err_no = GetLastError();
          LPVOID lpMsgBuf;
          FormatMessage(
                  FORMAT_MESSAGE_ALLOCATE_BUFFER |
                  FORMAT_MESSAGE_FROM_SYSTEM |
                  FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL,
                  err_no,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR) &lpMsgBuf,
                  0, NULL );
           */
            glDeleteLists(m_listbase, 256);
            m_listbase = 0;
        }
        else {
            SelectObject(m_hDC, oldfont);
        }
    }
}

COutlineFont::~COutlineFont()
{
    // Class destructor.

    glDeleteLists(m_listbase, 256);
    m_listbase = 0;
}


void
COutlineFont::DrawString(char* s)
{
    // Draws the given text string.

    GLsizei len = GLsizei(strlen(s));
    if (s && len > 0) {
		// Must save/restore the list base.
		glPushAttrib(GL_LIST_BIT);{
			glListBase(m_listbase);
			glCallLists(len, GL_UNSIGNED_BYTE, (const GLvoid*)s);
		} glPopAttrib();
    }
}


