#ifndef _OutlineFont_h_
#define _OutlineFont_h_

//#include <GL/gl.h>
//#include <string.h>

// COutlineFont is a class which encapsulates the details necessary
// to draw outline font text in OpenGL on the Windows platform.

class COutlineFont {
public:
    COutlineFont(
        wxGLCanvas* win, //CDC* dc,
        wchar_t* fontname);
    virtual ~COutlineFont();

    void
    DrawString(char* s);

private:
    GLuint m_listbase;
    wxDC* m_pDC; // CDC* m_pDC;

private:
    // Hide these.
    COutlineFont() { }
    COutlineFont(const COutlineFont& obj) { }
    COutlineFont& operator=(const COutlineFont& obj) { return *this; }
};

#endif
