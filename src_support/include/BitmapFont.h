#ifndef _BitmapFont_h_
#define _BitmapFont_h_

//#include <GL/gl.h>
//#include <string.h>

// CBitmapFont is a class which encapsulates the details necessary
// to draw bitmapped text in OpenGL on the Windows platform.

/*! \todo Implement font size and name change on user demand
 *   or multiple fonts lists for beter rendering performance
 *
 */
class CBitmapFont {
public:
    CBitmapFont(
        wxGLCanvas* win,//CDC* dc,
        const wchar_t* fontname);
    virtual ~CBitmapFont();

    void
    DrawStringAt(
        GLfloat x,
        GLfloat y,
        GLfloat z,
        char* s);

private:
    GLuint m_listbase;
    wxDC* m_pDC; //CDC* m_pDC;

private:
    // Hide these.
    CBitmapFont() { }
    CBitmapFont(const CBitmapFont& obj) { }
    CBitmapFont& operator=(const CBitmapFont& obj) { return *this; }
};

#endif
