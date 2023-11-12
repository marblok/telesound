#ifndef WX_PRECOMP
  #include <wx/wx.h>
#endif

#include <wx/glcanvas.h>
#include <climits>

#include <DSP_lib.h>
//#include <DSP_AudioMixer.h>

#include "BitmapFont.h"
#include "wxAddons.h"
// *************************************************************** //
void wxString2string(char *out, wxString in, int max_len)
{
  int ind;
  int len = in.Length();
  if (len >= max_len)
    len = max_len-1;

  for (ind=0; ind<len; ind++)
  {
    out[ind] = in[ind];
  }
  out[ind] = 0;
}

int ExtractParam(wxString &param, char *data, int size)
{
  int ind;
  char end_char=' ';

  ind = 0;
  param = "";

  if (data[0] == '"')
  {
    ind++;
    end_char = '"';
  }
  while (ind < size)
  {
    if (data[ind] == end_char)
      break;
    param << data[ind];

    ind++;
  }
  //find next parameters
  while (ind < size)
  {
    if (data[ind] != ' ')
      break;
    ind++;
  }

  return ind;
}

// *************************************************************** //
void GetColor(GLfloat *color, float val, const CLR_map_type map_type)
{
  if (val < 0)
    val = 0;
  if (val > 1)
    val = 1;

  switch (map_type)
  {
    case CLR_jet:
      // red
      if (val < 0.375)
        *color = 0;
      else
        if (val < 0.625)
          *color = 4*val-1.5;
        else
          if (val < 0.875)
            *color = 1.0;
          else
            *color = -4*val+4.5;

      // green
      color++;
      if (val < 0.125)
        *color = 0;
      else
        if (val < 0.375)
          *color = 4*val-0.5;
        else
          if (val < 0.625)
            *color = 1;
          else
            if (val < 0.875)
              *color = -4*val+3.5;
            else
              *color = 0;

      // blue
      color++;
      if (val < 0.125)
        *color = 4*val+0.5;
      else
        if (val < 0.375)
          *color = 1.0;
        else
          if (val < 0.625)
            *color = -4*val+2.5;
          else
            *color = 0;
      break;

    case CLR_hot:
      // red
      if (val < 0.375)
        *color = val/0.375;
      else
        *color = 1.0;

      // green
      color++;
      if (val < 0.375)
        *color = 0.0;
      else
        if ((val >= 0.375) && (val < 0.75)) // 2 * 0.375)
          *color = (val - 0.375) / 0.375;
        else
          *color = 1.0;

      // blue
      color++;
      if (val < 0.75)
        *color = 0.0;
      else
        *color = 4 * (val - 0.75);
      break;

    case CLR_gray:
    default:
      *color = val;
      color++;
      *color = val;
      color++;
      *color = val;
  }
}

void SetColor(const float val, const CLR_map_type map_type)
{
  GLfloat RGB[3];

  GetColor(RGB, val, map_type);

  //glColor3f(R,G,B);
  glColor3fv(RGB);
}


// *************************************************************** //
void T_PlotsStack::SetBackgroundColor(float val, const CLR_map_type map_type)
{
  GetColor(BackgroundRGB, val, map_type);
}
void T_PlotsStack::SetBackgroundColor(float R, float G, float B)
{
  BackgroundRGB[0] = R;
  BackgroundRGB[1] = G;
  BackgroundRGB[2] = B;
}

void T_PlotsStack::DrawSpecgram(float C_skala, const CLR_map_type map_type)
{
  DSP::Float *y;
  //float dx, dy;
  int Ny_tmp, ind_x, ind_y;

  //Nx = NoOfSlots;
  //Ny = K;
  dx = 2.0/NoOfSlots; dy = 2.0/K;

  //! \todo make list allocation and deletion global
  if (GL_list_index == 0)
    GL_list_index = glGenLists(1);

  //! \todo_later check if GL_COMPILE with glCallList(index) won't be faster
  glNewList(GL_list_index, GL_COMPILE_AND_EXECUTE);

  //sprintf(tekst, "%i, %i", Nx, Ny);
  //DSP::log << "DrawSpecgram"<<DSP::e::LogMode::second<< tekst<< std::endl;
  for (ind_x = 0; ind_x < NoOfSlots; ind_x++)
  {
    y = GetSlot(ind_x);
    if (y != NULL)
    {
      Ny_tmp = Get_SlotDataSize(ind_x);
      glBegin(GL_QUAD_STRIP);
      for (ind_y = 0; ind_y < Ny_tmp; ind_y++)
      {
        SetColor(C_skala*y[ind_y], map_type);
//        R = C_skala*y[ind_y]; G = R; B = R;
//        glColor3f(R,G,B);
        glVertex2f(ind_x*dx - 1,     ind_y*dy - 1);
        glVertex2f((ind_x+1)*dx - 1, ind_y*dy - 1);
      }
      glColor3f(0,0,0);
      glVertex2f(ind_x*dx - 1,     1.0);
      glVertex2f((ind_x+1)*dx - 1, 1.0);

      glEnd();
    }
  }

  glEndList();

  // glCallList(index); // execute list

  //glDeleteLists(index, 1);
}

/*! \warning F_min_in and F_max_in must be in the range of [-Fs/2, Fs/2]
 */
void T_PlotsStack::InitialiseSpectrogram(long double F_min_in, long double F_max_in,
    long double CenterFrequency_in,
    unsigned int NoOfSamplesPerAPSD_in, long double sampling_rate_in,
    bool real_FFT)
{
  int ind_x, ind_y;
  GLfloat *temp_vertex;
  float min_slot_y_float;

  if (VertexTable != NULL)
  {
    delete [] VertexTable;
    VertexTable = NULL;
  }
  if (ColorTable != NULL)
  {
    delete [] ColorTable;
    ColorTable = NULL;
  }

  if (real_FFT == false)
  {
    if ((K % 2) != 0)
    {
      DSP::log << DSP::e::LogMode::Error <<"T_PlotsStack::InitialiseSpectrogram"<< DSP::e::LogMode::second << "WARNING: odd length FFT not supported"<< std::endl;
      return;
    }

    NoOfSamplesPerAPSD = NoOfSamplesPerAPSD_in;
    sampling_rate = sampling_rate_in;
    F_min = F_min_in; F_max = F_max_in;
    CenterFrequency = CenterFrequency_in;
    // Y = 2/K * ind_y - 1;
    // ind_y = (int) floor(((Y + 1)*K)/2);
    //
    // Y = -1.0 ... +1.0 which corresponds to -Fs/2 ... +Fs/2
    // with zoom Y = 2 * F_min / Fs ... 2 * F_max / Fs
    //
    // min_ind_y = floor((F_min/Fs + 0.5)*K);
    // max_ind_y = ceil((F_min/Fs + 0.5)*K);
    min_slot_y_float = ((F_min-CenterFrequency)/sampling_rate + 0.5)*K;
    min_slot_y = (int)floor(min_slot_y_float);
    if (min_slot_y < 0)
      min_slot_y = 0;
    max_slot_y = (int)ceil(((F_max-CenterFrequency)/sampling_rate + 0.5)*K);
    if (max_slot_y >= K)
      max_slot_y = K-1;

    //! \todo dy must be calculated differently
    // dy = 2.0/K;
    dx = 2.0/NoOfSlots;
    dy = 2.0/K  * (sampling_rate / (F_max - F_min));
    dy_offset = -min_slot_y_float * dy; // minus so we can just add later
    dy_offset = dy_offset - 1; // GL origin is -1.0
  }
  else
  {
    int K2 = 2*(K-1);

    NoOfSamplesPerAPSD = NoOfSamplesPerAPSD_in;
    sampling_rate = sampling_rate_in;
    F_min = F_min_in; F_max = F_max_in;
    CenterFrequency = CenterFrequency_in;
    // Y = 2/K * ind_y - 1;
    // ind_y = (int) floor(((Y + 1)*K)/2);
    //
    // Y = -1.0 ... +1.0 which corresponds to -Fs/2 ... +Fs/2
    // with zoom Y = 2 * F_min / Fs ... 2 * F_max / Fs
    //
    // min_ind_y = floor((F_min/Fs + 0.5)*K);
    // max_ind_y = ceil((F_min/Fs + 0.5)*K);
    min_slot_y_float = ((F_min-CenterFrequency)/sampling_rate + 0.5)*K2;
    min_slot_y = (int)floor(min_slot_y_float);
    if (min_slot_y < 0)
      min_slot_y = 0;
    max_slot_y = (int)ceil(((F_max-CenterFrequency)/sampling_rate + 0.5)*K2);
    if (max_slot_y >= K)
      max_slot_y = K-1;

    //! \todo dy must be calculated differently
    // dy = 2.0/K;
    dx = 2.0/NoOfSlots;
    dy = 2.0/K2  * (sampling_rate / (F_max - F_min));
    dy_offset = -min_slot_y_float * dy; // minus so we can just add later
    dy_offset = dy_offset - 1; // GL origin is -1.0
  }


  VertexTable = new GLfloat[2 * 2 * NoOfSlots*(K+1)];
  for (ind_x = 0; ind_x < NoOfSlots; ind_x++)
  {
    temp_vertex = VertexTable + (2 * 2 * ind_x * (K+1));

    // K even => K+1 points
    //for (ind_y = 0; ind_y < K; ind_y++)
    for (ind_y = min_slot_y; ind_y <= max_slot_y; ind_y++)
    {
      //glVertex2f(ind_x*dx - 1,     ind_y*dy - 1, 0.0);
      *temp_vertex = ind_x*dx - 1; temp_vertex++;
      //*temp_vertex = ind_y*dy - 1; temp_vertex++;
      *temp_vertex = dy_offset + ind_y*dy;
      temp_vertex++;
      //*temp_vertex = 0.0;          temp_vertex++;

      //glVertex2f((ind_x+1)*dx - 1, ind_y*dy - 1, 0.0);
      *temp_vertex = (ind_x+1)*dx - 1; temp_vertex++;
      //*temp_vertex = ind_y*dy - 1; temp_vertex++;
      *temp_vertex = dy_offset + ind_y*dy;
      temp_vertex++;
      //*temp_vertex = 0.0; temp_vertex++;
    }
    // This represents Fs/2
    //glVertex2f(ind_x*dx - 1,     1.0, 0.0);
    *temp_vertex = ind_x*dx - 1; temp_vertex++;
    *temp_vertex = 1.0; temp_vertex++;
    //*temp_vertex = 0.0; temp_vertex++;

    //glVertex2f((ind_x+1)*dx - 1, 1.0, 0.0);
    *temp_vertex = (ind_x+1)*dx - 1; temp_vertex++;
    *temp_vertex = 1.0; temp_vertex++;
    //*temp_vertex = 0.0; temp_vertex++;
  }

  ColorTable  = new GLfloat[2 * 3 * NoOfSlots*(K+1)];
}


void T_PlotsStack::DrawSpecgram2_dB(float dB_max, float dB_range, const CLR_map_type map_type)
{
  DSP::Float *y;
  int Ny_tmp, ind_x, ind_y;
  int counter;
  float C_scale, C_offset;

  // scale to range [0, 1]
  C_scale = 1/dB_range;
  C_offset = dB_max-dB_range;

  //Nx = NoOfSlots;
  //Ny = K;
  //dx = 2.0/NoOfSlots; dy = 2.0/K;

  // K even H(Fs/2) == H(-Fs/2)

  GLfloat *temp_vertex;
  GLfloat *temp_color;

  if ((VertexTable == NULL) || (ColorTable == NULL))
  {
    DSP::log << DSP::e::LogMode::Error <<"T_PlotsStack::DrawSpecgram2_lin"<< DSP::e::LogMode::second << "Spectrogram not initialised"<< std::endl;
    return;
  }


  //glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);

  //counter = 0;
  for (ind_x = 0; ind_x < NoOfSlots; ind_x++)
  {
    temp_color  = ColorTable + (2 * 3 * ind_x * (K+1));

    y = GetSlot(ind_x);
    if (y != NULL)
    {
      Ny_tmp = Get_SlotDataSize(ind_x);
      if (Ny_tmp > max_slot_y+1)
        Ny_tmp = max_slot_y+1;
      for (ind_y = min_slot_y; ind_y < Ny_tmp; ind_y++)
      {
        //SetColor(C_skala*y[ind_y], map_type);
        GetColor(temp_color, C_scale*(y[ind_y]-C_offset), map_type);
        temp_color+= 3;

        GetColor(temp_color, C_scale*(y[ind_y]-C_offset), map_type);
        temp_color+= 3;

        //counter += 2;
      }
      // K even => K+1 points
      if (Ny_tmp == K)
      { // repeat first point
        GetColor(temp_color, C_scale*(y[0]-C_offset), map_type);
        temp_color+= 3;

        GetColor(temp_color, C_scale*(y[0]-C_offset), map_type);
        temp_color+= 3;

        //counter += 2;
      }
    }
  }

  for (ind_x = 0; ind_x < NoOfSlots; ind_x++)
  {
    temp_vertex = VertexTable + (2 * 2 * ind_x * (K+1));
    temp_color  = ColorTable + (2 * 3 * ind_x * (K+1));
    Ny_tmp = Get_SlotDataSize(ind_x);
    if (Ny_tmp > max_slot_y+1)
      Ny_tmp = max_slot_y+1;
    //! \todo check if +1 is needed if Ny_tmp < K
    // we start from min_slot_y and end on Ny_tmp (included)
    counter = (Ny_tmp+1 - min_slot_y)*2;

    if (counter >= 4)
    {
      //counter = 4;
      glColorPointer(3, GL_FLOAT, 0, temp_color); // 3 color components
      glVertexPointer(2, GL_FLOAT, 0, temp_vertex); // 2 coordinates per vertex
      glDrawArrays(GL_QUAD_STRIP, 0, counter);
      //glDrawArrays(GL_QUADS, 0, counter);
    }
  }

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);

  //glColor3f(0,0,0); // ???
}

void T_PlotsStack::DrawSpecgram2_lin(float C_scale, const CLR_map_type map_type)
{
  DSP::Float *y;
  int Ny_tmp, ind_x, ind_y;
  int counter;

  //Nx = NoOfSlots;
  //Ny = K;
  //dx = 2.0/NoOfSlots; dy = 2.0/K;

  // K even H(Fs/2) == H(-Fs/2)

  GLfloat *temp_vertex;
  GLfloat *temp_color;

  if ((VertexTable == NULL) || (ColorTable == NULL))
  {
    DSP::log << DSP::e::LogMode::Error <<"T_PlotsStack::DrawSpecgram2_lin"<< DSP::e::LogMode::second << "Spectrogram not initialised"<< std::endl;
    return;
  }


  //glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);

  //counter = 0;
  for (ind_x = 0; ind_x < NoOfSlots; ind_x++)
  {
    temp_color  = ColorTable + (2 * 3 * ind_x * (K+1));

    y = GetSlot(ind_x);
    if (y != NULL)
    {
      Ny_tmp = Get_SlotDataSize(ind_x);
      if (Ny_tmp > max_slot_y+1)
        Ny_tmp = max_slot_y+1;
      for (ind_y = min_slot_y; ind_y < Ny_tmp; ind_y++)
      {
        //SetColor(C_skala*y[ind_y], map_type);
        GetColor(temp_color, C_scale*y[ind_y], map_type);
        temp_color+= 3;

        GetColor(temp_color, C_scale*y[ind_y], map_type);
        temp_color+= 3;

        //counter += 2;
      }
      // K even => K+1 points
      if (Ny_tmp == K)
      { // repeat first point
        GetColor(temp_color, C_scale*y[0], map_type);
        temp_color+= 3;

        GetColor(temp_color, C_scale*y[0], map_type);
        temp_color+= 3;

        //counter += 2;
      }
    }
  }

  for (ind_x = 0; ind_x < NoOfSlots; ind_x++)
  {
    temp_vertex = VertexTable + (2 * 2 * ind_x * (K+1));
    temp_color  = ColorTable + (2 * 3 * ind_x * (K+1));
    Ny_tmp = Get_SlotDataSize(ind_x);
    if (Ny_tmp > max_slot_y+1)
      Ny_tmp = max_slot_y+1;
    //! \todo check if +1 is needed if Ny_tmp < K
    // we start from min_slot_y and end on Ny_tmp (included)
    counter = (Ny_tmp+1 - min_slot_y)*2;

    if (counter >= 4)
    {
      //counter = 4;
      glColorPointer(3, GL_FLOAT, 0, temp_color); // 3 color components
      glVertexPointer(2, GL_FLOAT, 0, temp_vertex); // 2 coordinates per vertex
      glDrawArrays(GL_QUAD_STRIP, 0, counter);
      //glDrawArrays(GL_QUADS, 0, counter);
    }
  }

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);

  //glColor3f(0,0,0); // ???
}
// *************************************************************** //
/*! 1. Most OpenGL implementations avoid using glBegin ()/ glEnd () to specify geometry due to its inherent performance issues
 *  2. glDrawArrays : Drawing Primitives Using Vertex Arrays
 *  void glGenBuffers( GLsizei n, GLuint* buffers );
 *  GLboolean glIsBuffer( GLuint buffer );
 *
 * \Fixed Use separate VertexTable and ColorTable per GLCavas
 * \Fixed Reuse VertexTable and refill ColorTable
 * \Fixed use 2 coordinates per vertex instead of 3
 * \Fixed For K even at Fs/2 draw sample from -Fs/2
 *
 * \todo_later ColorTable might be pushed instead of full refill
 *    but sometimes it should be reset (e.g. for Clim change)
 *
 * \todo Use glDrawElements() to reuse quad_strips vertexes
 *       (this also will force use of smooth colors for time axis)
 * \todo Use glMultiDrawElements()
 *
 * \todo_later Consider using color indexes and creating color map
 *    instead of using colors itself. Constant color map
 *    should be used instead of different color map each
 *    drawing.
 *
 * \todo Provide zoom parameters (integration with glDrawElements
 *   might be possible).
 *
 * \todo_later  For K odd over Fs/2 draw next sample to -Fs/2 and
 *   below -Fs/2 draw next sample to Fs/2.
 *   Correct also T_PlotsStack::DrawSpecgram and
 *   T_PlotsStack::SpectrogramMouseCordsToRealCords.
 *
 * \note VertexTable and ColorTable and setting them to NULL
 *   will force reinitialization
 *
 * \todo use color map instead of calculation color for each point
 */
void T_PlotsStack::DrawSpecgram2_base(const CLR_map_type map_type)
{
  DSP::Float *y;
  int Ny_tmp, ind_x, ind_y;
  int counter;

  //Nx = NoOfSlots;
  //Ny = K;
  //dx = 2.0/NoOfSlots; dy = 2.0/K;

  // K even H(Fs/2) == H(-Fs/2)

  GLfloat *temp_vertex;
  GLfloat *temp_color;

  if ((VertexTable == NULL) || (ColorTable == NULL))
  {
    DSP::log << DSP::e::LogMode::Error <<"T_PlotsStack::DrawSpecgram2_base"<< DSP::e::LogMode::second <<"Spectrogram not initialised"<< std::endl;
    return;
  }


  //glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);

  //counter = 0;
  for (ind_x = 0; ind_x < NoOfSlots; ind_x++)
  {
    temp_color  = ColorTable + (2 * 3 * ind_x * (K+1));

    y = GetSlot(ind_x);
    if (y != NULL)
    {
      Ny_tmp = Get_SlotDataSize(ind_x);
      if (Ny_tmp > max_slot_y+1)
        Ny_tmp = max_slot_y+1;
      for (ind_y = min_slot_y; ind_y < Ny_tmp; ind_y++)
      {
        //SetColor(C_skala*y[ind_y], map_type);
        GetColor(temp_color, y[ind_y], map_type);
        temp_color+= 3;

        GetColor(temp_color, y[ind_y], map_type);
        temp_color+= 3;

        //counter += 2;
      }
      // K even => K+1 points
      if (Ny_tmp == K)
      { // repeat first point
        GetColor(temp_color, y[0], map_type);
        temp_color+= 3;

        GetColor(temp_color, y[0], map_type);
        temp_color+= 3;

        //counter += 2;
      }
    }
  }

  for (ind_x = 0; ind_x < NoOfSlots; ind_x++)
  {
    temp_vertex = VertexTable + (2 * 2 * ind_x * (K+1));
    temp_color  = ColorTable + (2 * 3 * ind_x * (K+1));
    Ny_tmp = Get_SlotDataSize(ind_x);
    if (Ny_tmp > max_slot_y+1)
      Ny_tmp = max_slot_y+1;
    //! \todo check if +1 is needed if Ny_tmp < K
    // we start from min_slot_y and end on Ny_tmp (included)
    counter = (Ny_tmp+1 - min_slot_y)*2;

    if (counter >= 4)
    {
      //counter = 4;
      glColorPointer(3, GL_FLOAT, 0, temp_color); // 3 color components
      glVertexPointer(2, GL_FLOAT, 0, temp_vertex); // 2 coordinates per vertex
      glDrawArrays(GL_QUAD_STRIP, 0, counter);
      //glDrawArrays(GL_QUADS, 0, counter);
    }
  }

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);

  //glColor3f(0,0,0); // ???
}

/*! \bug -# Dopasowa� odpowiednie rozmiary textury
 *   -# obr�ci� textur� i rozci�gn�� na pe�ny axis
 *   -# podzieli� textur� na segmenty i wyrysowywa�
 *   -# quad_strip
 *
 * @param dB_max
 * @param dB_range
 * @param map_type
 */
void T_PlotsStack::DrawSpecgram3_dB(float dB_max, float dB_range, const CLR_map_type map_type)
{
  DSP::Float *y;
  int Ny_tmp, ind_x, ind_y;
  //int counter;
  float C_scale, C_offset;

  // scale to range [0, 1]
  C_scale = 1/dB_range;
  C_offset = dB_max-dB_range;

  //Nx = NoOfSlots;
  //Ny = K;
  //dx = 2.0/NoOfSlots; dy = 2.0/K;

  // K even H(Fs/2) == H(-Fs/2)

  //GLfloat *temp_vertex;
  GLfloat *temp_color;

  if ((VertexTable == NULL) || (ColorTable == NULL))
  {
    DSP::log << DSP::e::LogMode::Error <<"T_PlotsStack::DrawSpecgram2_lin"<< DSP::e::LogMode::second << "Spectrogram not initialised"<< std::endl;
    return;
  }


  //glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);

  //counter = 0;
  for (ind_x = 0; ind_x < NoOfSlots; ind_x++)
  {
    temp_color  = ColorTable + (2 * 3 * ind_x * (K+1));

    y = GetSlot(ind_x);
    if (y != NULL)
    {
      Ny_tmp = Get_SlotDataSize(ind_x);
      if (Ny_tmp > max_slot_y+1)
        Ny_tmp = max_slot_y+1;
      for (ind_y = min_slot_y; ind_y < Ny_tmp; ind_y++)
      {
        //SetColor(C_skala*y[ind_y], map_type);
        GetColor(temp_color, C_scale*(y[ind_y]-C_offset), map_type);
        temp_color+= 3;

        GetColor(temp_color, C_scale*(y[ind_y]-C_offset), map_type);
        temp_color+= 3;

        //counter += 2;
      }
      // K even => K+1 points
      if (Ny_tmp == K)
      { // repeat first point
        GetColor(temp_color, C_scale*(y[0]-C_offset), map_type);
        temp_color+= 3;

        GetColor(temp_color, C_scale*(y[0]-C_offset), map_type);
        temp_color+= 3;

        //counter += 2;
      }
    }
  }

  GLuint texture;
  glGenTextures(1, &texture);

  glBindTexture(GL_TEXTURE_2D, texture);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL); //GL_MODULATE););

  glTexParameterf ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameterf ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexParameterf ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameterf ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

  GLsizei border = 1;
  GLsizei width = 256; // 2^n + 2*border_witdh
  GLsizei height = 256; // 2^n + 2*border_witdh
  glTexImage2D(
      GL_TEXTURE_2D, 0, 3, // 3 color components
      width, height, border,
      GL_RGB, GL_FLOAT, ColorTable );

  glEnable( GL_TEXTURE_2D );

  //glBindTexture ( GL_TEXTURE_2D, texture );

  glBegin( GL_QUADS );
  glTexCoord2d(0.0, 0.0); glVertex2d( 0.0, 0.0);
  glTexCoord2d(1.0, 0.0); glVertex2d( 1.0, 0.0);
  glTexCoord2d(1.0, 1.0); glVertex2d( 1.0, 1.0);
  glTexCoord2d(0.0, 1.0); glVertex2d( 0.0, 1.0);
  glEnd();

  glDisable( GL_TEXTURE_2D );

  //  glDisableClientState(GL_VERTEX_ARRAY);
  //  glDisableClientState(GL_COLOR_ARRAY);
  glDeleteTextures( 1, &texture );
}
void T_PlotsStack::DrawScatterPlot(int SegmentSize, DSP::Float *XYdata, DSP::Complex_vector constellation, float skala, float size, bool demodulator_state)
{
  // if (!demodulator_state) // draw demodulator status
  //   return;
  
  float x, y;
  int ind;
  glColor3f(1.0, 0.0, 0.0);
  glPointSize(size+3);
  glBegin(GL_POINTS);
  for (auto &element : constellation) 
  {  
    glVertex2f(element.re/(skala*3.15),element.im/(skala*3.15));
  }
  glEnd();
  // Line antialiasing is controlled by calling glEnable and glDisable with argument GL_LINE_SMOOTH
  // axis
  glLineWidth(1.0);
  glColor3f(0.5, 0.5, 0.5);
  glLineStipple(1, 0x0FF0);
  glEnable(GL_LINE_STIPPLE);
  glBegin(GL_LINES);

  glVertex2f(-1.0, 0.0);
  glVertex2f(+1.0, 0.0);
  glVertex2f(0.0, -1.0);
  glVertex2f(0.0, +1.0);
  
  glColor3f(0.8, 0.8, 0.8);
  glVertex2f(-1.0,+1.0);
  glVertex2f(+1.0,-1.0);
  glVertex2f(-1.0,-1.0);
  glVertex2f(+1.0,+1.0);
  glEnd();
  glDisable(GL_LINE_STIPPLE);

  glColor3f(1.0, 1.0, 1.0);

  glBegin(GL_POLYGON);
  glColor3f(0.0, 0.0, 0.5);
  glVertex2f(0.0, 0.90);
  glVertex2f(0.0, 0.97);
  glEnd();

  glBegin(GL_POLYGON);
  glColor3f(0.0, 0.3, 0.0);

  glVertex2f(0.0, 0.91);
  glVertex2f(0.0, 0.98);
  glEnd();

  glColor3f(0.0, 0.0, 1.0);
  glPointSize(size);
  glBegin(GL_POINTS);
  for (ind=0; ind<SegmentSize; ind+=2)
  {
    x=XYdata[ind]*skala;
    y=XYdata[ind+1]*skala;
    glVertex2f(x, y);
  }
  glEnd();
}
void T_PlotsStack::DrawEyeDiagram(int SamplingRate, DSP::Float_vector samples, int samplesPerSymbol, int symbolsPerTrace, float skala_x, float skala_y, bool input_complex, bool demodulator_state)
{
  if (!demodulator_state) // draw demodulator status
    return; 

  
  glPointSize(2);
  int numSamples;
  float reBaseLine, imBaseLine;
  std::vector<float> x_values;

  if (input_complex)
  {
    numSamples = samples.size() / 2;
    reBaseLine = 0.5;
    imBaseLine = -0.5;
  }
  else
  {
    numSamples = samples.size();
  }

  int samplesPerTrace = samplesPerSymbol * symbolsPerTrace;
  int numTraces = numSamples / samplesPerTrace;
  float color_tick = 1.0f / numTraces;
  float x_tick = (2 * skala_x) / samplesPerTrace;


  for (float i = -skala_x; i < skala_x; i += x_tick)
  { // prepare x values;
    x_values.push_back(i);
  }
  x_values.push_back(skala_x);
  unsigned int x_size = x_values.size();
  int offset = 0;
  float color = 0;

  glLineWidth(1.0f);
 // glEnable(GL_LINE_SMOOTH);
  if (input_complex)
  {
    for (int i = 0; i < numTraces; i++)
    {
      



  glBegin(GL_LINES);
      for (int j = 0; j < x_size - 2; j+=2)
      {
        glColor3f(color + color_tick, color + color_tick, 0.0);
      
        glVertex2d(x_values[j], reBaseLine+samples[j + offset] * skala_y);//re
        glVertex2d(x_values[j + 2], reBaseLine+samples[j + 2 + offset] * skala_y);//re
        
        glColor3f(0.0, color + color_tick, color + color_tick);
        glVertex2d(x_values[j], imBaseLine+samples[j + 1 + offset] * skala_y);//im
        glVertex2d(x_values[j + 2], imBaseLine+samples[j + 3 + offset] * skala_y);//im
     
      }
      glEnd();
      offset += samplesPerTrace;
      color += color_tick;
    }

  }
  else
  {
    for (int i = 0; i < numTraces; i++)
    {
      glBegin(GL_LINES);
      // glBegin(GL_POINTS);
      for (int j = 1; j < x_size - 1; j++)
      {
        glColor3f(color + color_tick, color + color_tick, 0.0);
        glVertex2d(x_values[j], samples[j + offset] * skala_y);
        glVertex2d(x_values[j + 1], samples[j + 1 + offset] * skala_y);
      }
      glEnd();
      offset += samplesPerTrace;
      color += color_tick;
    }
  }
   // glDisable(GL_LINE_SMOOTH);
}

void T_PlotsStack::DrawSignal(float skala, DS_type type, float width)
{
  float x_offset;
  float tmp;
  float ind_factor;
  float *Ydata;
  int ind, ind0, y_size;

  glLineWidth(width);
  glBegin(GL_LINE_STRIP);

  switch (type)
  {
    case DS_unsigned:
      skala = skala*2;
      break;
    default:
      break;
  }

  ind_factor = 2.0/K / NoOfSlots;
//  x0=-1.0; // -1 : +1 <- segment offset
  x_offset = -1.0;
  for (ind0=0; ind0<NoOfSlots; ind0++)
  {
    Ydata = GetSlot(ind0);
    y_size = Get_SlotDataSize(ind0);

    for (ind=0; ind<y_size; ind++)
    {
      switch (type)
      {
        case DS_unsigned:
          tmp=Ydata[ind]*skala - 1.0;
          break;
        case DS_signed:
        default:
          tmp=Ydata[ind]*skala;
          break;
      }
      //x_offset = ind_factor*ind;
      //x=x0+x_offset;
      glVertex2f(x_offset, tmp);//((float)ind)/8);
      x_offset += ind_factor;
    }
  }
  glEnd();
}


// ************************************************ //
// ************************************************ //
//T_PlotsStack SpectrumPlotsStack;
//T_PlotsStack SignalPlotsStack;
unsigned char T_PlotsStack::x_subsubticks_pixels_W[] = {
    0, 0, 255, 96,   255, 255, 0, 255,   0, 0, 255, 96,
    0, 0, 255, 96,   255, 255, 0, 255,   0, 0, 255, 96,
    0, 0, 255, 96,   255, 255, 0, 255,   0, 0, 255, 96,
    0, 0, 255, 96,   255, 255, 0, 255,   0, 0, 255, 96,
    0, 0, 255, 96,   0, 0, 255, 96,   0, 0, 255, 96};
unsigned char T_PlotsStack::x_subticks_pixels_W[] = {
    0, 0, 0, 96,  255, 255, 255, 255,  0, 0, 0, 96,
    0, 0, 0, 96,  255, 255, 255, 255,  0, 0, 0, 96,
    0, 0, 0, 96,  255, 255, 255, 255,  0, 0, 0, 96,
    0, 0, 0, 96,  255, 255, 255, 255,  0, 0, 0, 96,
    0, 0, 0, 96,  255, 255, 255, 255,  0, 0, 0, 96,
    0, 0, 0, 96,  255, 255, 255, 255,  0, 0, 0, 96,
    0, 0, 0, 96,    0, 0, 0, 96,    0, 0, 0, 96};
unsigned char T_PlotsStack::x_ticks_pixels_W[] = {
    0, 0, 0, 96,  255, 255, 255, 255,  255, 255, 255, 255,  255, 255, 255, 255,  0, 0, 0, 96,
    0, 0, 0, 96,  255, 255, 255, 255,  255, 255, 255, 255,  255, 255, 255, 255,  0, 0, 0, 96,
    0, 0, 0, 96,  255, 255, 255, 255,  255, 255, 255, 255,  255, 255, 255, 255,  0, 0, 0, 96,
    0, 0, 0, 96,  255, 255, 255, 255,  255, 255, 255, 255,  255, 255, 255, 255,  0, 0, 0, 96,
    0, 0, 0, 96,  255, 255, 255, 255,  255, 255, 255, 255,  255, 255, 255, 255,  0, 0, 0, 96,
    0, 0, 0, 96,  255, 255, 255, 255,  255, 255, 255, 255,  255, 255, 255, 255,  0, 0, 0, 96,
    0, 0, 0, 96,  255, 255, 255, 255,  255, 255, 255, 255,  255, 255, 255, 255,  0, 0, 0, 96,
    0, 0, 0, 96,  255, 255, 255, 255,  255, 255, 255, 255,  255, 255, 255, 255,  0, 0, 0, 96,
    0, 0, 0, 96,  0, 0, 0, 96,             0, 0, 0, 96,   0, 0, 0, 96,        0, 0, 0, 96
};

unsigned char T_PlotsStack::y_subsubticks_pixels_W[4*5*3] = {
    0, 0, 255, 96,     0, 0, 255, 96,     0, 0, 255, 96,     0, 0, 255, 96,     0, 0, 255, 96,
    255, 255, 0, 255,  255, 255, 0, 255,  255, 255, 0, 255,  255, 255, 0, 255,  0, 0, 255, 96,
    0, 0, 255, 96,     0, 0, 255, 96,     0, 0, 255, 96,     0, 0, 255, 96,     0, 0, 255, 96
};
unsigned char T_PlotsStack::y_subticks_pixels_W[4*7*3] = {
    0, 0, 0, 96,         0, 0, 0, 96,         0, 0, 0, 96,         0, 0, 0, 96,         0, 0, 0, 96,         0, 0, 0, 96,         0, 0, 0, 96,
    255, 255, 255, 255,  255, 255, 255, 255,  255, 255, 255, 255,  255, 255, 255, 255,  255, 255, 255, 255,  255, 255, 255, 255,  0, 0, 0, 96,
    0, 0, 0, 96,         0, 0, 0, 96,         0, 0, 0, 96,         0, 0, 0, 96,         0, 0, 0, 96,         0, 0, 0, 96,         0, 0, 0, 96
};
unsigned char T_PlotsStack::y_ticks_pixels_W[4*9*5] = {
    0, 0, 0, 96,     0, 0, 0, 96,     0, 0, 0, 96,     0, 0, 0, 96,     0, 0, 0, 96,     0, 0, 0, 96,    0, 0, 0, 96,   0, 0, 0, 96,    0, 0, 0, 96,
    255, 255, 255, 255,  255, 255, 255, 255,  255, 255, 255, 255,  255, 255, 255, 255,  255, 255, 255, 255,  255, 255, 255, 255,  255, 255, 255, 255,  255, 255, 255, 255,  0, 0, 0, 96,
    255, 255, 255, 255,  255, 255, 255, 255,  255, 255, 255, 255,  255, 255, 255, 255,  255, 255, 255, 255,  255, 255, 255, 255,  255, 255, 255, 255,  255, 255, 255, 255,  0, 0, 0, 96,
    255, 255, 255, 255,  255, 255, 255, 255,  255, 255, 255, 255,  255, 255, 255, 255,  255, 255, 255, 255,  255, 255, 255, 255,  255, 255, 255, 255,  255, 255, 255, 255,  0, 0, 0, 96,
    0, 0, 0, 96,     0, 0, 0, 96,     0, 0, 0, 96,     0, 0, 0, 96,     0, 0, 0, 96,     0, 0, 0, 96,    0, 0, 0, 96,   0, 0, 0, 96,    0, 0, 0, 96
};


unsigned char T_PlotsStack::x_subsubticks_pixels_B[] = {
    255, 255, 0, 96,  0, 0, 255, 255,  255, 255, 0, 96,
    255, 255, 0, 96,  0, 0, 255, 255,  255, 255, 0, 96,
    255, 255, 0, 96,  0, 0, 255, 255,  255, 255, 0, 96,
    255, 255, 0, 96,  0, 0, 255, 255,  255, 255, 0, 96,
    255, 255, 0, 96,  255, 255, 0, 96,  255, 255, 0, 96};
unsigned char T_PlotsStack::x_subticks_pixels_B[] = {
    255, 255, 255, 96,  0, 0, 0, 255,  255, 255, 255, 96,
    255, 255, 255, 96,  0, 0, 0, 255,  255, 255, 255, 96,
    255, 255, 255, 96,  0, 0, 0, 255,  255, 255, 255, 96,
    255, 255, 255, 96,  0, 0, 0, 255,  255, 255, 255, 96,
    255, 255, 255, 96,  0, 0, 0, 255,  255, 255, 255, 96,
    255, 255, 255, 96,  0, 0, 0, 255,  255, 255, 255, 96,
    255, 255, 255, 96,  255, 255, 255, 96,  255, 255, 255, 96};
unsigned char T_PlotsStack::x_ticks_pixels_B[] = {
    255, 255, 255, 96,  0, 0, 0, 255,        0, 0, 0, 255,        0, 0, 0, 255,  255, 255, 255, 96,
    255, 255, 255, 96,  0, 0, 0, 255,        0, 0, 0, 255,        0, 0, 0, 255,  255, 255, 255, 96,
    255, 255, 255, 96,  0, 0, 0, 255,        0, 0, 0, 255,        0, 0, 0, 255,  255, 255, 255, 96,
    255, 255, 255, 96,  0, 0, 0, 255,        0, 0, 0, 255,        0, 0, 0, 255,  255, 255, 255, 96,
    255, 255, 255, 96,  0, 0, 0, 255,        0, 0, 0, 255,        0, 0, 0, 255,  255, 255, 255, 96,
    255, 255, 255, 96,  0, 0, 0, 255,        0, 0, 0, 255,        0, 0, 0, 255,  255, 255, 255, 96,
    255, 255, 255, 96,  0, 0, 0, 255,        0, 0, 0, 255,        0, 0, 0, 255,  255, 255, 255, 96,
    255, 255, 255, 96,  0, 0, 0, 255,        0, 0, 0, 255,        0, 0, 0, 255,  255, 255, 255, 96,
    255, 255, 255, 96,  255, 255, 255, 96,    255, 255, 255, 96,    255, 255, 255, 96,    255, 255, 255, 96
    };

unsigned char T_PlotsStack::y_subsubticks_pixels_B[] = {
    255, 255, 0, 96,     255, 255, 0, 96,     255, 255, 0, 96,     255, 255, 0, 96,     255, 255, 0, 96,
    0, 0, 0, 255,        0, 0, 0, 255,        0, 0, 0, 255,        0, 0, 0, 255,        255, 255, 0, 96,
    255, 255, 0, 96,     255, 255, 0, 96,     255, 255, 0, 96,     255, 255, 0, 96,     255, 255, 0, 96
};
unsigned char T_PlotsStack::y_subticks_pixels_B[] = {
    255, 255, 255, 96,  255, 255, 255, 96,   255, 255, 255, 96,  255, 255, 255, 96,  255, 255, 255, 96,  255, 255, 255, 96,  255, 255, 255, 96,
    0, 0, 0, 255,       0, 0, 0, 255,        0, 0, 0, 255,       0, 0, 0, 255,       0, 0, 0, 255,       0, 0, 0, 255,       255, 255, 255, 96,
    255, 255, 255, 96,  255, 255, 255, 96,   255, 255, 255, 96,  255, 255, 255, 96,  255, 255, 255, 96,  255, 255, 255, 96,  255, 255, 255, 96
};
unsigned char T_PlotsStack::y_ticks_pixels_B[] = {
    255, 255, 255, 96,   255, 255, 255, 96,   255, 255, 255, 96,   255, 255, 255, 96,   255, 255, 255, 96,   255, 255, 255, 96,   255, 255, 255, 96,   255, 255, 255, 96,   255, 255, 255, 96,
    0, 0, 0, 255,        0, 0, 0, 255,        0, 0, 0, 255,        0, 0, 0, 255,        0, 0, 0, 255,        0, 0, 0, 255,        0, 0, 0, 255,        0, 0, 0, 255,        255, 255, 255, 96,
    0, 0, 0, 255,        0, 0, 0, 255,        0, 0, 0, 255,        0, 0, 0, 255,        0, 0, 0, 255,        0, 0, 0, 255,        0, 0, 0, 255,        0, 0, 0, 255,        255, 255, 255, 96,
    0, 0, 0, 255,        0, 0, 0, 255,        0, 0, 0, 255,        0, 0, 0, 255,        0, 0, 0, 255,        0, 0, 0, 255,        0, 0, 0, 255,        0, 0, 0, 255,        255, 255, 255, 96,
    255, 255, 255, 96,   255, 255, 255, 96,   255, 255, 255, 96,   255, 255, 255, 96,   255, 255, 255, 96,   255, 255, 255, 96,   255, 255, 255, 96,   255, 255, 255, 96,   255, 255, 255, 96
};

unsigned char T_PlotsStack::x_subsubticks_pixels[TICKS_TEXTURE_MAXSIZE*TICKS_TEXTURE_MAXSIZE*4];
unsigned char T_PlotsStack::x_subticks_pixels[TICKS_TEXTURE_MAXSIZE*TICKS_TEXTURE_MAXSIZE*4];
unsigned char T_PlotsStack::x_ticks_pixels[TICKS_TEXTURE_MAXSIZE*TICKS_TEXTURE_MAXSIZE*4];
unsigned char T_PlotsStack::y_subsubticks_pixels[TICKS_TEXTURE_MAXSIZE*TICKS_TEXTURE_MAXSIZE*4];
unsigned char T_PlotsStack::y_subticks_pixels[TICKS_TEXTURE_MAXSIZE*TICKS_TEXTURE_MAXSIZE*4];
unsigned char T_PlotsStack::y_ticks_pixels[TICKS_TEXTURE_MAXSIZE*TICKS_TEXTURE_MAXSIZE*4];

T_PlotsStack::T_PlotsStack(int Initial_NoOfSlots, int Initial_K)
{
  ticks_mode = E_TM_black;

  BackgroundRGB[0] = 0;
  BackgroundRGB[1] = 0;
  BackgroundRGB[2] = 0;
  InitColors(6);

  NoOfSlots=0;

  K = 0;
  Slots = NULL;
  SlotDataSize = NULL;
  ColorIndex = NULL;
  SlotTime = NULL;

  GL_list_index = 0;
  VertexTable = NULL;
  ColorTable = NULL;

  m_bmf = NULL;

  sampling_rate = 0.0;

  Reset(Initial_K, Initial_NoOfSlots);

  InitAxis(1, 1);

  currentSlot=0;
}
T_PlotsStack::~T_PlotsStack(void)
{
  FreeSlots();
  InitColors(0);
  if (GL_list_index != 0)
    glDeleteLists(GL_list_index, 1);
}


void T_PlotsStack::InitColors(int No)
{
  int ind;

  NoOfColors=No;
  if (NoOfColors == 0)
    delete [] ColorsTable;
  else
  {
    ColorsTable = new float[NoOfColors*3];
    for (ind = 0; ind < NoOfColors; ind++)
    {
      switch (ind)
      {
        case 0:
          ColorsTable[3*ind]  =1.0;
          ColorsTable[3*ind+1]=1.0;
          ColorsTable[3*ind+2]=1.0;
          break;
        case 1:
          ColorsTable[3*ind]  =1.0;
          ColorsTable[3*ind+1]=1.0;
          ColorsTable[3*ind+2]=0.0;
          break;
        case 2:
          ColorsTable[3*ind]  =0.0;
          ColorsTable[3*ind+1]=1.0;
          ColorsTable[3*ind+2]=1.0;
          break;
        case 3:
          ColorsTable[3*ind]  =1.0;
          ColorsTable[3*ind+1]=0.0;
          ColorsTable[3*ind+2]=1.0;
          break;
        case 4:
          ColorsTable[3*ind]  =1.0;
          ColorsTable[3*ind+1]=0.0;
          ColorsTable[3*ind+2]=0.0;
          break;
        case 5:
          ColorsTable[3*ind]  =0.0;
          ColorsTable[3*ind+1]=1.0;
          ColorsTable[3*ind+2]=0.0;
          break;
        case 6:
          ColorsTable[3*ind]  =0.0;
          ColorsTable[3*ind+1]=0.0;
          ColorsTable[3*ind+2]=1.0;
          break;
        default:
          ColorsTable[3*ind]  =1.0;
          ColorsTable[3*ind+1]=1.0;
          ColorsTable[3*ind+2]=1.0;
      }
    }
  }
}

void T_PlotsStack::FreeSlots(void)
{
  int ind;
  //char tekst[1024];

  currentSlot=-1;
  if (SlotDataSize != NULL)
    delete [] SlotDataSize;
  if (ColorIndex != NULL)
    delete [] ColorIndex;
  if (SlotTime != NULL)
    delete [] SlotTime;

  if (Slots != NULL)
  {
    for (ind=0; ind < NoOfSlots; ind++)
    {
      //sprintf(tekst, "%i/%i %p", ind+1, NoOfSlots, Slots[ind]);
      //DSP::log << "T_PlotsStack::FreeSlots", tekst);
      //sprintf(tekst, "test%i.log", ind+1);
      //DSP::f::SetLogFileName (tekst);
      if (Slots[ind] != NULL)
        delete [] Slots[ind];
    }
    delete [] Slots;
  }

  if (VertexTable != NULL)
  {
    delete [] VertexTable;
    VertexTable = NULL;
  }
  if (ColorTable != NULL)
  {
    delete [] ColorTable;
    ColorTable = NULL;
  }
}

void T_PlotsStack::Reset(int new_K, int New_NoOfSlots)
{
  int ind;

  if (New_NoOfSlots == -1)
    New_NoOfSlots = NoOfSlots;
  if (new_K == -1)
    new_K = K;

  if ((K != new_K) || (NoOfSlots != New_NoOfSlots))
  { // allocate new slots data space
    FreeSlots();
    NoOfSlots = New_NoOfSlots;

    K=new_K;
    Slots        = new DSP::Float_ptr[NoOfSlots];
    SlotDataSize = new int[NoOfSlots];
    ColorIndex   = new int[NoOfSlots];
    SlotTime     = new long double[NoOfSlots];

    for (ind=0; ind < NoOfSlots; ind++)
    {
      if (K > 0)
        Slots[ind] = new DSP::Float[K];
      else
        Slots[ind] = NULL;

      //char tekst[1024];
      //sprintf(tekst, "%i/%i %p", ind+1, NoOfSlots, Slots[ind]);
      //DSP::log << "T_PlotsStack::Reset", tekst);

      SlotDataSize[ind] = 0;
      SlotTime[ind] = 0.0;
      ColorIndex[ind]   = (ind % NoOfColors);
    }
  }
  else
  { // just reset stack state
    for (ind=0; ind < NoOfSlots; ind++)
    {
      SlotDataSize[ind] = 0;
      SlotTime[ind] = 0.0;
      ColorIndex[ind]   = (ind % NoOfColors);
    }
  }

  currentSlot=0;
}

void T_PlotsStack::NextSlot(bool IsCyclic)
{
  if (currentSlot == -1)
    return;
  if (currentSlot == NoOfSlots-1)
  {
    if (IsCyclic == true)
    {
      currentSlot = 0;
      SlotDataSize[0] = 0;
    }
    else
      currentSlot = NoOfSlots;
    return;
  }

  currentSlot++;
  SlotDataSize[currentSlot] = 0;
  SlotTime[currentSlot] = 0.0;
}

int T_PlotsStack::Get_SlotIndex(void)
{
  return currentSlot;
}

int T_PlotsStack::Get_MaxSlotDataSize(void)
{
  return K;
}

int T_PlotsStack::Get_NoOfSlots(void)
{
  return NoOfSlots;
}

DSP::Float_ptr T_PlotsStack::GetSlot(bool PushIfLast)
{
  if (currentSlot >= NoOfSlots)
  {
    if (PushIfLast == true)
    {
      if (Slots != NULL)
      {
        DSP::Float_ptr tempSlot;
        //int tempSize;
        int tempColor;

        tempSlot = Slots[0];
        //tempSize = SlotDataSize[0];
        tempColor = ColorIndex[0];
        for (int ind=1; ind < NoOfSlots; ind++)
        {
          Slots[ind-1] = Slots[ind];
          SlotDataSize[ind-1] = SlotDataSize[ind];
          SlotTime[ind-1] = SlotTime[ind];
          ColorIndex[ind-1] = ColorIndex[ind];
        }
        SlotDataSize[NoOfSlots-1] = 0;
        SlotTime[NoOfSlots-1] = 0.0;
        ColorIndex[NoOfSlots-1] = tempColor;
        Slots[NoOfSlots-1] = tempSlot;
      }
      currentSlot = NoOfSlots-1;
    }
    else
      return NULL;
  }
  return Slots[currentSlot];
}

DSP::Float_ptr T_PlotsStack::GetSlot(int No)
{
  if (No == -1)
    No = currentSlot;
  if (No >= NoOfSlots)
    No = -1;

  if (No == -1)
    return NULL;
  return Slots[No];
}

// if No == -1 for current slot
void T_PlotsStack::Set_SlotDataSize(int new_DataSize, int No)
{
  if (No == -1)
    No = currentSlot;
  if (No >= NoOfSlots)
    No = -1;

  if (No == -1)
    return;

  if (new_DataSize > K)
  {
    new_DataSize = K;
    DSP::log << DSP::e::LogMode::Error <<"T_PlotsStack::Set_SlotDataSize"<< DSP::e::LogMode::second << "New data size exceeds allocated memory"<< std::endl;
  }
  SlotDataSize[No]=new_DataSize;
}
// if No == -1 for current slot
int T_PlotsStack::Get_SlotDataSize(int No)
{
  if (No == -1)
    No = currentSlot;
  if (No >= NoOfSlots)
    No = -1;

  if (No == -1)
    return 0;
  return SlotDataSize[No];
}
void T_PlotsStack::Set_ColorIndex(int index, int No)
{
  if (No == -1)
    No = currentSlot;
  if (No >= NoOfSlots)
    No = -1;

  if (No == -1)
    return;
  ColorIndex[No]=index;
}
void T_PlotsStack::Set_SlotTime(long double new_time, int No)
{
  if (No == -1)
    No = currentSlot;
  if (No >= NoOfSlots)
    No = -1;

  if (No == -1)
    return;
  SlotTime[No]=new_time;
}
long double T_PlotsStack::Get_TimeWidth(void)
{
  return (NoOfSlots * NoOfSamplesPerAPSD) / sampling_rate;
}
long double T_PlotsStack::Get_FrequencyWidth(void)
{
  return (F_max - F_min);
}
long double T_PlotsStack::Get_Fo(void)
{
  return F_min;
}
long double T_PlotsStack::Get_SlotTime(int &No)
{
  if (No == -1)
    No = currentSlot;
  if (No >= NoOfSlots)
    No = -1;

  if (No == -1)
    return 0;
  if (SlotDataSize[No] == 0)
  {
    No = 0;
  }
  return SlotTime[No];
}

void T_PlotsStack::Get_Color(float &R, float &G, float &B, int No)
{
  float *tmp;

  if (No == -1)
    No = currentSlot;
  if (No >= NoOfSlots)
    No = -1;

  if (No == -1)
  {
    R=1.0; G=1.0; B=1.0;
  }
  else
  {
    tmp=ColorsTable + 3*ColorIndex[No];
    R=*tmp; tmp++;
    G=*tmp; tmp++;
    B=*tmp;
  }
}

void GetMixerError(int rs, wxString &tempString)
{
  switch(rs)
  {
    case MIXERR_INVALCONTROL:
      tempString="MIXERR_INVALCONTROL\n";
      break;
    case MMSYSERR_BADDEVICEID:
      tempString="MMSYSERR_BADDEVICEID\n";
      break;
    case MMSYSERR_INVALFLAG:
      tempString="MMSYSERR_INVALFLAG\n";
      break;
    case MMSYSERR_INVALHANDLE:
      tempString="MMSYSERR_INVALHANDLE\n";
      break;
    case MMSYSERR_INVALPARAM:
      tempString="MMSYSERR_INVALPARAM\n";
      break;
    case MMSYSERR_NODRIVER:
      tempString="MMSYSERR_NODRIVER\n";
      break;
    case MMSYSERR_NOERROR:
      tempString="MMSYSERR_NOERROR\n";
      break;
    default:
      tempString="";  tempString << (int)rs << "\n";
      break;
  }
}

void TestAudioMixer(wxTextCtrl *Lines, TAudioMixer *Mixer)
{
  char tekst[1024];
//  UINT WaveInNo; //Number of wave in audio devices
  DWORD WaveIn_Destination, Destination;
  DWORD WaveInLineID, LineID;
  DWORD WaveIn_cControls, cControls;
  DWORD WaveIn_cConnections, cConnections;
  DWORD WaveIn_cChannels, cChannels;
  MIXERCONTROL *MixerControls;
  MIXERCONTROLDETAILS MixerControlDetails;
//  MIXERCONTROLDETAILS_LISTTEXT *MixerControlDetails_LISTTEXT;

  MMRESULT rs;
  MIXERCAPS MixerCaps;
  MIXERLINE MixerLine;
  MIXERLINECONTROLS MixerLineControls;

//  UINT temp;
  MIXERCONTROLDETAILS_BOOLEAN pom_BOOLEAN[100];
  MIXERCONTROLDETAILS_LISTTEXT pom_LISTTEXT[100];
  MIXERCONTROLDETAILS_SIGNED pom_SIGNED[100];
  MIXERCONTROLDETAILS_UNSIGNED pom_UNSIGNED[100];
//    MIXERCONTROLDETAILS_BOOLEAN pom_BOOLEAN[100];
//    //MIXERCONTROLDETAILS_LISTTEXT pom_LISTTEXT[100];
//    MIXERCONTROLDETAILS_SIGNED pom_SIGNED[100];
//    MIXERCONTROLDETAILS_UNSIGNED pom_UNSIGNED[100];



  DWORD ind, ind_1, ind_2, ind_3;
  wxString tempString;
  bool temp_PCMwaveFileActive;

  temp_PCMwaveFileActive=Mixer->PCMwaveFileActive;
  Mixer->PCMwaveFileActive=false;

  Lines->Clear();

  tempString="mixerGetNumDevs:";
  tempString << (int)(Mixer->MixersNumber) << "\n";
////  Lines->Add(tempString);
//  Lines->Strings[0]=tempString;
  Lines->AppendText(tempString);

  tempString="waveInGetNumDevs:";
  tempString << (int)(Mixer->WaveInNumber) << "\n";
//  Lines->Add(tempString);
  Lines->AppendText(tempString);
//  Lines->Add("==================");
  tempString = "==================\n";
  Lines->AppendText(tempString);

  if (Mixer->WaveInNumber>0)
  { //Okreslamy mozliwosci mixer'a
    rs=mixerGetDevCaps(UINT(Mixer->hMixer_in), &MixerCaps, sizeof(MIXERCAPS));

//    Lines->Add("mixerGetDevCaps");
    tempString = "mixerGetDevCaps\n";
    Lines->AppendText(tempString);
    GetMixerError(rs, tempString);
    Lines->AppendText(tempString);

    tempString="   Product Name: ";
    tempString << MixerCaps.szPname << "\n";
    Lines->AppendText(tempString);

    tempString="   cDestinations: ";
    tempString << (int)MixerCaps.cDestinations << "\n";
    Lines->AppendText(tempString);
    tempString = "==================\n";
    Lines->AppendText(tempString);

    //Okreslamy mozliwosci linii WAVEIN
    MixerLine.cbStruct=sizeof(MIXERLINE);
    rs=mixerGetLineInfo((HMIXEROBJ)0, //the identifier of a waveform-audio input device in the range of zero to one less than the number of devices returned by the waveInGetNumDevs function
                        &MixerLine, MIXER_OBJECTF_WAVEIN);
    WaveInLineID=MixerLine.dwLineID;
    WaveIn_cControls=MixerLine.cControls;
    WaveIn_cChannels=MixerLine.cChannels;
    WaveIn_cConnections=MixerLine.cConnections;
    WaveIn_Destination=MixerLine.dwDestination;

//    Lines->Add("mixerGetLineInfo / MIXER_OBJECTF_WAVEIN");
    tempString = "mixerGetLineInfo / MIXER_OBJECTF_WAVEIN\n";
    Lines->AppendText(tempString);
    GetMixerError(rs, tempString);
    Lines->AppendText(tempString);
    sprintf(tekst,"   dwDestination: 0x%.8x\n", (unsigned int)MixerLine.dwDestination);
    tempString = tekst;
    Lines->AppendText(tempString);
/*
    sprintf(tekst,"   dwSource: 0x%.8x\n", (unsigned int)MixerLine.dwSource);
    tempString = tekst;
    Lines->AppendText(tempString);
    sprintf(tekst,"   dwLineID: 0x%.8x\n", (unsigned int)MixerLine.dwLineID);
    tempString = tekst;
    Lines->AppendText(tempString);
    sprintf(tekst,"   fdwLine:  0x%.8x\n", (unsigned int)MixerLine.fdwLine);
    tempString = tekst;
    Lines->AppendText(tempString);
    sprintf(tekst,"   dwComponentType: %s\n", Mixer->GetMixerComponentType(MixerLine.dwComponentType));
    tempString = tekst;
    Lines->AppendText(tempString);
    tempString="   cChannels: ";
    tempString << (int)(MixerLine.cChannels) << "\n";
    Lines->AppendText(tempString);
    tempString="   cConnections: ";
    tempString << (int)(MixerLine.cConnections) << "\n";
    Lines->AppendText(tempString);
    tempString="   cControls: ";
    tempString << (int)(MixerLine.cControls) << "\n";
    Lines->AppendText(tempString);
    tempString="   szShortName: ";
    tempString << MixerLine.szShortName << "\n";
    Lines->AppendText(tempString);
    tempString="   szName: ";
    tempString << MixerLine.szName << "\n";
    Lines->AppendText(tempString);
*/
    tempString = "==================\n";
    Lines->AppendText(tempString);

    //Okreslamy wlasciwosci kolejnych linii miksera
    for (ind=0; ind<MixerCaps.cDestinations; ind++)
    {
      MixerLine.cbStruct=sizeof(MIXERLINE);
      MixerLine.dwDestination=ind;
      rs=mixerGetLineInfo((HMIXEROBJ)0, //the identifier of a waveform-audio input device in the range of zero to one less than the number of devices returned by the waveInGetNumDevs function
                          &MixerLine, MIXER_GETLINEINFOF_DESTINATION);

      tempString = "mixerGetLineInfo / MIXER_GETLINEINFOF_DESTINATION\n";
      Lines->AppendText(tempString);
      GetMixerError(rs, tempString);
      Lines->AppendText(tempString);
      if (WaveIn_Destination == MixerLine.dwDestination)
        sprintf(tekst,"   dwDestination: 0x%.8x (WAVE_IN)\n", (unsigned int)MixerLine.dwDestination);
      else
        sprintf(tekst,"   dwDestination: 0x%.8x\n", (unsigned int)MixerLine.dwDestination);
      tempString = tekst;
      Lines->AppendText(tempString);
      sprintf(tekst,"   dwSource: 0x%.8x\n", (unsigned int)MixerLine.dwSource);
      tempString = tekst;
      Lines->AppendText(tempString);
      sprintf(tekst,"   dwLineID: 0x%.8x\n", (unsigned int)MixerLine.dwLineID);
      tempString = tekst;
      Lines->AppendText(tempString);
      sprintf(tekst,"   fdwLine:  0x%.8x\n", (unsigned int)MixerLine.fdwLine);
      tempString = tekst;
      Lines->AppendText(tempString);
      sprintf(tekst,"   dwComponentType:  %s\n", Mixer->GetMixerComponentType(MixerLine.dwComponentType));
      tempString = tekst;
      Lines->AppendText(tempString);
      tempString="   cChannels: ";
      tempString << (int)(MixerLine.cChannels) << "\n";
      Lines->AppendText(tempString);
      tempString="   cConnections: ";
      tempString << (int)(MixerLine.cConnections) << "\n";
      Lines->AppendText(tempString);
      tempString="   cControls: ";
      tempString << (int)(MixerLine.cControls) << "\n";
      Lines->AppendText(tempString);
      tempString="   szShortName: ";
      tempString << MixerLine.szShortName << "\n";
      Lines->AppendText(tempString);
      tempString="   szName: ";
      tempString << MixerLine.szName << "\n";
      Lines->AppendText(tempString);
      tempString = "==================\n";
      Lines->AppendText(tempString);



      // ********************************************** //
      // ********************************************** //
      // ********************************************** //
      tempString = "==================\n\n";
      Lines->AppendText(tempString);

      LineID=MixerLine.dwLineID;
      cControls=MixerLine.cControls;
      cChannels=MixerLine.cChannels;
      cConnections=MixerLine.cConnections;
      Destination=MixerLine.dwDestination;

      //Okreslamy wlasciwosci kontrolek dla bie��cej linii
      MixerControls=new MIXERCONTROL [cControls];
      MixerLineControls.cbStruct=sizeof(MIXERLINECONTROLS);
      MixerLineControls.dwLineID=LineID;
      MixerLineControls.cControls=cControls;
      MixerLineControls.pamxctrl=MixerControls;
      MixerLineControls.cbmxctrl=sizeof(MIXERCONTROL);

      rs=mixerGetLineControls(HMIXEROBJ(Mixer->hMixer_in), //HMIXEROBJ hmxobj,
        &MixerLineControls, MIXER_GETLINECONTROLSF_ALL);

      if (WaveIn_Destination == Destination)
        tempString="mixerGetLineControls [WAVE_IN] / MIXER_GETLINECONTROLSF_ALL\n";
      else
        tempString="mixerGetLineControls / MIXER_GETLINECONTROLSF_ALL\n";
      Lines->AppendText(tempString);
      GetMixerError(rs, tempString);
      Lines->AppendText(tempString);
      for (ind_1=0; ind_1<cControls; ind_1++)
      {
        sprintf(tekst,"   dwControlID: 0x%.8x\n", (unsigned int)MixerControls[ind_1].dwControlID);
        tempString = tekst;
        Lines->AppendText(tempString);
        sprintf(tekst,"   dwControlType: %s\n", Mixer->GetMixerControlType(MixerControls[ind_1].dwControlType));
        tempString = tekst;
        Lines->AppendText(tempString);
        tempString="   fdwControl: ";
        switch (MixerControls[ind_1].fdwControl)
        {
          case MIXERCONTROL_CONTROLF_DISABLED:
            tempString << "MIXERCONTROL_CONTROLF_DISABLED" << "\n";
            break;
          case MIXERCONTROL_CONTROLF_MULTIPLE:
            tempString << "MIXERCONTROL_CONTROLF_MULTIPLE" << "\n";
            break;
          case MIXERCONTROL_CONTROLF_UNIFORM:
            tempString << "MIXERCONTROL_CONTROLF_UNIFORM" << "\n";
            break;
          default:
            tempString << (int)(MixerControls[ind_1].fdwControl) << "\n";
            break;
        }
        Lines->AppendText(tempString);
        tempString="   cMultipleItems: ";
        tempString << (int)(MixerControls[ind_1].cMultipleItems) << "\n";
        Lines->AppendText(tempString);
        tempString="   Metrics.cSteps: ";
        tempString << (int)(MixerControls[ind_1].Metrics.cSteps) << "\n";
        Lines->AppendText(tempString);
        tempString="   szShortName: ";
        tempString << MixerControls[ind_1].szShortName << "\n";
        Lines->AppendText(tempString);
        tempString="   szName: ";
        tempString << MixerControls[ind_1].szName << "\n";
        Lines->AppendText(tempString);

         //Otczytanie stanu kontrolki
        MixerControlDetails.cbStruct=sizeof(MIXERCONTROLDETAILS);
        MixerControlDetails.dwControlID=MixerControls[ind_1].dwControlID;
        MixerControlDetails.cChannels=MixerLine.cChannels; //MixerLine.cChannels; //1; //WaveIn_cChannels; one mixer ON/OFF control per channel
        MixerControlDetails.cMultipleItems=MixerControls[ind_1].cMultipleItems;
        switch (MixerControls[ind_1].dwControlType)
        {
          //List controls:
          case MIXERCONTROL_CONTROLTYPE_MIXER:
          case MIXERCONTROL_CONTROLTYPE_MULTIPLESELECT:
          case MIXERCONTROL_CONTROLTYPE_MUX:
          case MIXERCONTROL_CONTROLTYPE_SINGLESELECT:
            MixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_BOOLEAN);
            MixerControlDetails.paDetails=&(pom_BOOLEAN[0]);
            rs=mixerGetControlDetails(HMIXEROBJ(Mixer->hMixer_in), &MixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
            if (rs != 0)
            {
              MixerControlDetails.cChannels--;
              rs=mixerGetControlDetails(HMIXEROBJ(Mixer->hMixer_in), &MixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
            }
            MixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_LISTTEXT);
            MixerControlDetails.paDetails=&(pom_LISTTEXT[0]);
            rs=mixerGetControlDetails(HMIXEROBJ(Mixer->hMixer_in), &MixerControlDetails, MIXER_GETCONTROLDETAILSF_LISTTEXT);
            tempString="   -------------\n";
            Lines->AppendText(tempString);
            tempString="   mixerGetControlDetails / MIXER_GETCONTROLDETAILSF_VALUE/LISTTEXT\n";
            Lines->AppendText(tempString);
            GetMixerError(rs, tempString);
            Lines->AppendText(tempString);
            tempString = "     cChannels: ";
            tempString <<  MixerControlDetails.cChannels << "\n";
            Lines->AppendText(tempString);

            if (MixerControlDetails.cMultipleItems<2)
              MixerControlDetails.cMultipleItems=1;
            for (ind_2=0; ind_2<MixerControlDetails.cMultipleItems*MixerControlDetails.cChannels; ind_2++)
            {
              if (pom_BOOLEAN[ind_2].fValue==0)
                sprintf(tekst,"    (%i) OFF / %s%*s/ dwLineID: 0x%.8x\n",
                        (int) ind_2, pom_LISTTEXT[ind_2].szName,
                        20-wcslen(pom_LISTTEXT[ind_2].szName), " ",
                        (unsigned int) pom_LISTTEXT[ind_2].dwParam1);
              else
                sprintf(tekst,"    (%i) ON  / %s%*s/ dwLineID: 0x%.8x\n",
                        (int) ind_2, pom_LISTTEXT[ind_2].szName,
                        20-wcslen(pom_LISTTEXT[ind_2].szName), " ",
                        (unsigned int) pom_LISTTEXT[ind_2].dwParam1);
              tempString = tekst;
              Lines->AppendText(tempString);
            }
            break;
          //Switch controls:
          case MIXERCONTROL_CONTROLTYPE_BOOLEAN:
          case MIXERCONTROL_CONTROLTYPE_BUTTON:
          case MIXERCONTROL_CONTROLTYPE_LOUDNESS:
          case MIXERCONTROL_CONTROLTYPE_MONO:
          case MIXERCONTROL_CONTROLTYPE_MUTE:
          case MIXERCONTROL_CONTROLTYPE_ONOFF:
          case MIXERCONTROL_CONTROLTYPE_STEREOENH:
          //Meter controls:
          case MIXERCONTROL_CONTROLTYPE_BOOLEANMETER:
            MixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_BOOLEAN);
            MixerControlDetails.paDetails=&(pom_BOOLEAN[0]);
            rs=mixerGetControlDetails(HMIXEROBJ(Mixer->hMixer_in), &MixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
            if (rs != 0)
            {
              MixerControlDetails.cChannels--;
              rs=mixerGetControlDetails(HMIXEROBJ(Mixer->hMixer_in), &MixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
            }
            tempString = "   -------------\n";
            Lines->AppendText(tempString);
            tempString = "   mixerGetControlDetails / MIXER_GETCONTROLDETAILSF_VALUE\n";
            Lines->AppendText(tempString);
            GetMixerError(rs, tempString);
            Lines->AppendText(tempString);
            tempString = "     cChannels: ";
            tempString <<  MixerControlDetails.cChannels << "\n";
            Lines->AppendText(tempString);
            if (MixerControlDetails.cMultipleItems<2)
              MixerControlDetails.cMultipleItems=1;
            for (ind_2=0; ind_2<MixerControlDetails.cMultipleItems*MixerControlDetails.cChannels; ind_2++)
            {
              tempString="    (";
              tempString << (int)ind_2;
              tempString << ") ";
              if (pom_BOOLEAN[ind_2].fValue==0)
                tempString << "OFF" << "\n";
              else
                tempString << "ON" << "\n";
              Lines->AppendText(tempString);
            }
            break;
          //Meter controls:
          case MIXERCONTROL_CONTROLTYPE_PEAKMETER:
          case MIXERCONTROL_CONTROLTYPE_SIGNEDMETER:
          //Member controls:
          case MIXERCONTROL_CONTROLTYPE_SIGNED:
          //Number controls:
          case MIXERCONTROL_CONTROLTYPE_DECIBELS:
          //Slider controls:
          case MIXERCONTROL_CONTROLTYPE_PAN:
          case MIXERCONTROL_CONTROLTYPE_QSOUNDPAN:
          case MIXERCONTROL_CONTROLTYPE_SLIDER:
            MixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_SIGNED);
            MixerControlDetails.paDetails=&(pom_SIGNED[0]);
            rs=mixerGetControlDetails(HMIXEROBJ(Mixer->hMixer_in), &MixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
            if (rs != 0)
            {
              MixerControlDetails.cChannels--;
              rs=mixerGetControlDetails(HMIXEROBJ(Mixer->hMixer_in), &MixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
            }
            tempString = "   -------------\n";
            Lines->AppendText(tempString);
            tempString = "   mixerGetControlDetails / MIXER_GETCONTROLDETAILSF_VALUE\n";
            Lines->AppendText(tempString);
            GetMixerError(rs, tempString);
            Lines->AppendText(tempString);
            tempString = "     cChannels: ";
            tempString <<  MixerControlDetails.cChannels;
            Lines->AppendText(tempString);
            tempString = "     cChannels: ";
            tempString <<  MixerControlDetails.cChannels << "\n";
            Lines->AppendText(tempString);
            if (MixerControlDetails.cMultipleItems<2)
              MixerControlDetails.cMultipleItems=1;
            for (ind_2=0; ind_2<MixerControlDetails.cMultipleItems*MixerControlDetails.cChannels; ind_2++)
            {
              tempString="    (";
              tempString << (int)ind_2;
              tempString << ") ";
              tempString << (int)pom_SIGNED[ind_2].lValue;
              tempString << " [";
              tempString << (int)MixerControls[ind_1].Bounds.lMinimum;
              tempString << ",";
              tempString << (int)MixerControls[ind_1].Bounds.lMaximum;
              tempString << "]" << "\n";
              Lines->AppendText(tempString);
            }
            break;
          //Meter control:
          case MIXERCONTROL_CONTROLTYPE_UNSIGNEDMETER:
          //Number control:
          case MIXERCONTROL_CONTROLTYPE_UNSIGNED:
          //Fader controls:
          case MIXERCONTROL_CONTROLTYPE_BASS:
          case MIXERCONTROL_CONTROLTYPE_EQUALIZER:
          case MIXERCONTROL_CONTROLTYPE_FADER:
          case MIXERCONTROL_CONTROLTYPE_TREBLE:
          case MIXERCONTROL_CONTROLTYPE_VOLUME:
          //Time controls:
          case MIXERCONTROL_CONTROLTYPE_MICROTIME:
          case MIXERCONTROL_CONTROLTYPE_MILLITIME:
          case MIXERCONTROL_CONTROLTYPE_PERCENT:
            MixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_UNSIGNED);
            MixerControlDetails.paDetails=&(pom_UNSIGNED[0]);
            rs=mixerGetControlDetails(HMIXEROBJ(Mixer->hMixer_in), &MixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
            if (rs != 0)
            {
              MixerControlDetails.cChannels--;
              rs=mixerGetControlDetails(HMIXEROBJ(Mixer->hMixer_in), &MixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
            }
            tempString = "   -------------\n";
            Lines->AppendText(tempString);
            tempString = "   mixerGetControlDetails / MIXER_GETCONTROLDETAILSF_VALUE\n";
            Lines->AppendText(tempString);
            GetMixerError(rs, tempString);
            Lines->AppendText(tempString);
            tempString = "     cChannels: ";
            tempString <<  MixerControlDetails.cChannels << "\n";
            Lines->AppendText(tempString);
            if (MixerControlDetails.cMultipleItems<2)
              MixerControlDetails.cMultipleItems=1;
            for (ind_2=0; ind_2<MixerControlDetails.cMultipleItems*MixerControlDetails.cChannels; ind_2++)
            {
              sprintf(tekst,"    (%i) %d [%d,%d]\n",
                      (int)ind_2, (int)pom_UNSIGNED[ind_2].dwValue,
                      (int)MixerControls[ind_1].Bounds.dwMinimum, (int)MixerControls[ind_1].Bounds.dwMaximum);
              tempString = tekst;
              Lines->AppendText(tempString);
            }
            break;
        }
        if (ind_1==cControls-1)
          tempString = "==================\n";
        else
          tempString = "  --------------------------------\n";
        Lines->AppendText(tempString);
      }


    if (MixerControls!=NULL)
    {
      delete [] MixerControls;
      MixerControls=NULL;
    }


      // **************************************** //
      //Okreslamy wlasciwosci przylaczy dla bie��cej linii // WAVEIN
      for (ind_1=0; ind_1<cConnections; ind_1++)
      {
        //Okreslamy wlasciwosci linii dla kolejnych przylaczy linii WAVEIN
        MixerLine.cbStruct=sizeof(MIXERLINE);
        MixerLine.dwDestination=Destination;
        MixerLine.dwSource=ind_1;
        rs=mixerGetLineInfo((HMIXEROBJ)0, //the identifier of a waveform-audio input device in the range of zero to one less than the number of devices returned by the waveInGetNumDevs function
                            &MixerLine, MIXER_GETLINEINFOF_SOURCE);

        tempString = "mixerGetLineInfo / MIXER_GETLINEINFOF_SOURCE\n";
        Lines->AppendText(tempString);
        GetMixerError(rs, tempString);
        Lines->AppendText(tempString);
        sprintf(tekst,"   dwDestination: 0x%.8x\n",(unsigned int)MixerLine.dwDestination);
        tempString = tekst;
        Lines->AppendText(tempString);
        sprintf(tekst,"   dwSource: 0x%.8x\n", (unsigned int)MixerLine.dwSource);
        tempString = tekst;
        Lines->AppendText(tempString);
        sprintf(tekst,"   dwLineID: 0x%.8x\n",(unsigned int)MixerLine.dwLineID);
        tempString = tekst;
        Lines->AppendText(tempString);
        sprintf(tekst,"   fdwLine:  0x%.8x\n",(unsigned int)MixerLine.fdwLine);
        tempString = tekst;
        Lines->AppendText(tempString);
        tempString="   dwComponentType: ";
        tempString << Mixer->GetMixerComponentType(MixerLine.dwComponentType) << "\n";
        Lines->AppendText(tempString);
        tempString="   cChannels: ";
        tempString << (int)(MixerLine.cChannels) << "\n";
        Lines->AppendText(tempString);
        tempString="   cConnections: ";
        tempString << (int)(MixerLine.cConnections) << "\n";
        Lines->AppendText(tempString);
        tempString="   cControls: ";
        tempString << (int)(MixerLine.cControls) << "\n";
        Lines->AppendText(tempString);
        tempString="   szShortName: ";
        tempString << MixerLine.szShortName << "\n";
        Lines->AppendText(tempString);
        tempString="   szName: ";
        tempString << MixerLine.szName << "\n";
        Lines->AppendText(tempString);
        tempString = "   ---------------------------------\n";
        Lines->AppendText(tempString);

        //Controls
        //Okreslamy wlasciwosci kontrolek dla kolejnych przylaczy linii WAVEIN
        MixerControls=new MIXERCONTROL [MixerLine.cControls];
        MixerLineControls.cbStruct=sizeof(MIXERLINECONTROLS);
        MixerLineControls.dwLineID=MixerLine.dwLineID;
        MixerLineControls.cControls=MixerLine.cControls;
        MixerLineControls.pamxctrl=MixerControls;
        MixerLineControls.cbmxctrl=sizeof(MIXERCONTROL);

        rs=mixerGetLineControls(HMIXEROBJ(Mixer->hMixer_in), //HMIXEROBJ hmxobj,
          &MixerLineControls, MIXER_GETLINECONTROLSF_ALL);

        tempString = "   mixerGetLineControls / MIXER_GETLINECONTROLSF_ALL\n";
        Lines->AppendText(tempString);
        GetMixerError(rs, tempString);
        Lines->AppendText(tempString);
        for (ind_2=0; ind_2<MixerLine.cControls; ind_2++)
        {
          tempString="      dwControlID: ";
          tempString << (int)(MixerControls[ind_2].dwControlID) << "\n";
          Lines->AppendText(tempString);
          tempString="      dwControlType: ";
          tempString  << Mixer->GetMixerControlType(MixerControls[ind_2].dwControlType) << "\n";
          Lines->AppendText(tempString);
          tempString="      fdwControl: ";
          switch (MixerControls[ind_2].fdwControl)
          {
            case MIXERCONTROL_CONTROLF_DISABLED:
              tempString << "MIXERCONTROL_CONTROLF_DISABLED" << "\n";
              break;
            case MIXERCONTROL_CONTROLF_MULTIPLE:
              tempString << "MIXERCONTROL_CONTROLF_MULTIPLE" << "\n";
              break;
            case MIXERCONTROL_CONTROLF_UNIFORM:
              tempString << "MIXERCONTROL_CONTROLF_UNIFORM" << "\n";
              break;
            default:
              tempString << (int)(MixerControls[ind_2].fdwControl) << "\n";
              break;
          }
          Lines->AppendText(tempString);
          tempString="      cMultipleItems: ";
          tempString << (int)(MixerControls[ind_2].cMultipleItems) << "\n";
          Lines->AppendText(tempString);
          tempString="      Metrics.cSteps: ";
          tempString << (int)(MixerControls[ind_2].Metrics.cSteps) << "\n";
          Lines->AppendText(tempString);
          tempString="      szShortName: ";
          tempString << MixerControls[ind_2].szShortName << "\n";
          Lines->AppendText(tempString);
          tempString="      szName: ";
          tempString << MixerControls[ind_2].szName << "\n";
          Lines->AppendText(tempString);

          //Otczytanie stanu kontrolki
          MixerControlDetails.cbStruct=sizeof(MIXERCONTROLDETAILS);
          MixerControlDetails.dwControlID=MixerControls[ind_2].dwControlID;
          MixerControlDetails.cChannels=MixerLine.cChannels; //can be also 1 (all channels together)
            //If you stuff here too much rs==11
          MixerControlDetails.cMultipleItems=MixerControls[ind_2].cMultipleItems;
          switch (MixerControls[ind_2].dwControlType)
          {
            //List controls:
            case MIXERCONTROL_CONTROLTYPE_MIXER:
            case MIXERCONTROL_CONTROLTYPE_MULTIPLESELECT:
            case MIXERCONTROL_CONTROLTYPE_MUX:
            case MIXERCONTROL_CONTROLTYPE_SINGLESELECT:
            //Switch controls:
            case MIXERCONTROL_CONTROLTYPE_BOOLEAN:
            case MIXERCONTROL_CONTROLTYPE_BUTTON:
            case MIXERCONTROL_CONTROLTYPE_LOUDNESS:
            case MIXERCONTROL_CONTROLTYPE_MONO:
            case MIXERCONTROL_CONTROLTYPE_MUTE:
            case MIXERCONTROL_CONTROLTYPE_ONOFF:
            case MIXERCONTROL_CONTROLTYPE_STEREOENH:
            //Meter controls:
            case MIXERCONTROL_CONTROLTYPE_BOOLEANMETER:
              MixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_BOOLEAN);
              MixerControlDetails.paDetails=&(pom_BOOLEAN[0]);
              rs=mixerGetControlDetails(HMIXEROBJ(Mixer->hMixer_in), &MixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
              if (rs != 0)
              {
                MixerControlDetails.cChannels--;
                rs=mixerGetControlDetails(HMIXEROBJ(Mixer->hMixer_in), &MixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
              }
              tempString = "      -------------\n";
              Lines->AppendText(tempString);
              tempString = "      mixerGetControlDetails / MIXER_GETCONTROLDETAILSF_VALUE\n";
              Lines->AppendText(tempString);
              GetMixerError(rs, tempString);
              Lines->AppendText(tempString);

              tempString = "        cChannels: ";
              tempString << MixerControlDetails.cChannels << "\n";
              Lines->AppendText(tempString);
              tempString = "        cMultipleItems: ";
              tempString << MixerControlDetails.cMultipleItems << "\n";
              Lines->AppendText(tempString);
              if (MixerControlDetails.cMultipleItems<2)
                MixerControlDetails.cMultipleItems=1;
              for (ind_3=0; ind_3<MixerControlDetails.cMultipleItems*MixerControlDetails.cChannels; ind_3++)
              {
                tempString="        (";
                tempString << (int)ind_3;
                tempString << ") ";
                if (pom_BOOLEAN[ind_3].fValue==0)
                  tempString << "OFF" << "\n";
                else
                  tempString << "ON" << "\n";
                Lines->AppendText(tempString);
              }
              break;
            //Meter controls:
            case MIXERCONTROL_CONTROLTYPE_PEAKMETER:
            case MIXERCONTROL_CONTROLTYPE_SIGNEDMETER:
            //Member controls:
            case MIXERCONTROL_CONTROLTYPE_SIGNED:
            //Number controls:
            case MIXERCONTROL_CONTROLTYPE_DECIBELS:
            //Slider controls:
            case MIXERCONTROL_CONTROLTYPE_PAN:
            case MIXERCONTROL_CONTROLTYPE_QSOUNDPAN:
            case MIXERCONTROL_CONTROLTYPE_SLIDER:
              MixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_SIGNED);
              MixerControlDetails.paDetails=&(pom_SIGNED[0]);
              rs=mixerGetControlDetails(HMIXEROBJ(Mixer->hMixer_in), &MixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
              if (rs != 0)
              {
                MixerControlDetails.cChannels--;
                rs=mixerGetControlDetails(HMIXEROBJ(Mixer->hMixer_in), &MixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
              }
              tempString = "      -------------\n";
              Lines->AppendText(tempString);
              tempString = "      mixerGetControlDetails / MIXER_GETCONTROLDETAILSF_VALUE\n";
              Lines->AppendText(tempString);
              GetMixerError(rs, tempString);
              Lines->AppendText(tempString);

              tempString = "        cChannels: ";
              tempString << MixerControlDetails.cChannels << "\n";
              Lines->AppendText(tempString);
              tempString = "        cMultipleItems: ";
              tempString << MixerControlDetails.cMultipleItems << "\n";
              Lines->AppendText(tempString);
              if (MixerControlDetails.cMultipleItems<2)
                MixerControlDetails.cMultipleItems=1;
              for (ind_3=0; ind_3<MixerControlDetails.cMultipleItems*MixerControlDetails.cChannels; ind_3++)
              {
                tempString="        (";
                tempString << (int)ind_3;
                tempString << ") ";
                tempString << (int)(pom_SIGNED[ind_3].lValue);
                tempString << " [";
                tempString << (int)(MixerControls[ind_2].Bounds.lMinimum);
                tempString << ",";
                tempString << (int)(MixerControls[ind_2].Bounds.lMaximum);
                tempString << "]" << "\n";
                Lines->AppendText(tempString);
              }
              break;
            //Meter control:
            case MIXERCONTROL_CONTROLTYPE_UNSIGNEDMETER:
            //Number control:
            case MIXERCONTROL_CONTROLTYPE_UNSIGNED:
            //Fader controls:
            case MIXERCONTROL_CONTROLTYPE_BASS:
            case MIXERCONTROL_CONTROLTYPE_EQUALIZER:
            case MIXERCONTROL_CONTROLTYPE_FADER:
            case MIXERCONTROL_CONTROLTYPE_TREBLE:
            case MIXERCONTROL_CONTROLTYPE_VOLUME:
            //Time controls:
            case MIXERCONTROL_CONTROLTYPE_MICROTIME:
            case MIXERCONTROL_CONTROLTYPE_MILLITIME:
            case MIXERCONTROL_CONTROLTYPE_PERCENT:
              MixerControlDetails.cbDetails=sizeof(MIXERCONTROLDETAILS_UNSIGNED);
              MixerControlDetails.paDetails=&(pom_UNSIGNED[0]);
              rs=mixerGetControlDetails(HMIXEROBJ(Mixer->hMixer_in), &MixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
              if (rs != 0)
              {
                MixerControlDetails.cChannels--;
                rs=mixerGetControlDetails(HMIXEROBJ(Mixer->hMixer_in), &MixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);
              }
              tempString = "      -------------\n";
              Lines->AppendText(tempString);
              tempString = "      mixerGetControlDetails / MIXER_GETCONTROLDETAILSF_VALUE\n";
              Lines->AppendText(tempString);
              GetMixerError(rs, tempString);
              Lines->AppendText(tempString);

              tempString = "        cChannels: ";
              tempString << MixerControlDetails.cChannels << "\n";
              Lines->AppendText(tempString);
              tempString = "        cMultipleItems: ";
              tempString << MixerControlDetails.cMultipleItems << "\n";
              Lines->AppendText(tempString);
              if (MixerControlDetails.cMultipleItems<2)
                MixerControlDetails.cMultipleItems=1;
              for (ind_3=0; ind_3<MixerControlDetails.cMultipleItems*MixerControlDetails.cChannels; ind_3++)
              {
                sprintf(tekst,"        (%i) %d [%d,%d]\n",
                        (int) ind_3, (int)pom_UNSIGNED[ind_3].dwValue,
                        (int) MixerControls[ind_2].Bounds.dwMinimum,
                        (int) MixerControls[ind_2].Bounds.dwMaximum);
                tempString = tekst;
                Lines->AppendText(tempString);
              }
              break;
          }

          if (ind_2==MixerLine.cControls-1)
            tempString = "==================\n";
          else
            tempString = "      -----------------------------------------\n";
          Lines->AppendText(tempString);
        }
        delete [] MixerControls;
      }
    }
  }
  Mixer->PCMwaveFileActive=temp_PCMwaveFileActive;

}

/*! \Fixed Added function to draw some character primitives
 *
 * \note Primary Color must be changed before setting raster pos
 *  otherwise Drawing character won't work
  //glPopMatrix();
  //glPopClientAttrib();
  glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT); // lighting and color mask
  glDisable(GL_LIGHTING);     // need to disable lighting for proper text color
  glColor3f(1.0, 1.0, 1.0);
  glRasterPos2f(0.0, 0.0);
  //glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_10, 'X');
  glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_10, (const unsigned char *)("Proba mikrofonu"));
  glEnable(GL_LIGHTING);
  glPopAttrib();
 *
 *
 * glPushMatrix glPopMatrix
 * glRotatef
 */
void DrawChar(GLfloat x, GLfloat y, char znak, GLfloat scale_, GLfloat width,
              GLfloat win_x_scale, GLfloat win_y_scale)
{
  //glDrawPixels
  GLfloat x0, x0_1, x1, x1_1;
  GLfloat y0, y0_1, y1, y1_1;
  GLfloat korekta_x,   korekta_y;
  GLfloat scale_x, scale_y;

  scale_y = scale_;
  scale_x = scale_*win_x_scale/win_y_scale;

  korekta_x = floor(width/2)*win_x_scale;
  korekta_y = floor(width/2)*win_y_scale;
  x0 = x-0.1*scale_x; x0_1 = x-0.1*scale_x-korekta_x;
  x1 = x+0.1*scale_x; x1_1 = x+0.1*scale_x+korekta_x;
  y0 = y-0.2*scale_y; y0_1 = y-0.2*scale_y-korekta_y;
  y1 = y+0.2*scale_y; y1_1 = y+0.2*scale_y+korekta_y;

  glLineWidth(width);
//  glBegin(GL_LINE_STRIP);
  glBegin(GL_LINES);
  switch (znak)
  {
    case '0':
      glVertex2f(x0, y0);  glVertex2f(x1, y1);
      glVertex2f(x1, y1);  glVertex2f(x1, y0);
      glVertex2f(x1, y0);  glVertex2f(x0, y0);
      glVertex2f(x0, y0);  glVertex2f(x0, y1);
      glVertex2f(x0, y1);  glVertex2f(x1, y1);
      break;
    case '1':
//      glVertex2f(x-0.05*scale_x, y+0.12*scale_y); glVertex2f(x+0.05*scale_x-korekta_x, y1);
      glVertex2f(x-0.05*scale_x, y+0.12*scale_y); glVertex2f(x+0.05*scale_x, y1_1);
      glVertex2f(x+0.05*scale_x, y0_1);  glVertex2f(x+0.05*scale_x, y1_1);
      break;
    case '2':
      glVertex2f(x0, y1);  glVertex2f(x1, y1);
      glVertex2f(x1, y1);  glVertex2f(x1, y+0.02*scale_y);
      glVertex2f(x1, y+0.02*scale_y); glVertex2f(x0, y+0.02*scale_y);
      glVertex2f(x0, y+0.02*scale_y); glVertex2f(x0, y0+korekta_y);
      break;
    case '3':
      glVertex2f(x0, y1); glVertex2f(x1, y1);
      glVertex2f(x1, y1); glVertex2f(x1, y0);
//      glVertex2f(x1, y1); glVertex2f(x1, y+0.02*scale);
      glVertex2f(x-0.08*scale_x, y+0.02*scale_y); glVertex2f(x1, y+0.02*scale_y);
//      glVertex2f(x1, y+0.02*scale); glVertex2f(x1, y0);
      glVertex2f(x1, y0); glVertex2f(x0, y0);
      break;
    case '4':
      glVertex2f(x0, y1); glVertex2f(x0, y-0.02*scale_y);
      glVertex2f(x0, y-0.02*scale_y); glVertex2f(x1, y-0.02*scale_y);
//      glVertex2f(x1, y-0.02*scale); glVertex2f(x1, y1);
      glVertex2f(x1, y1); glVertex2f(x1, y0);
      break;
    case '5':
      glVertex2f(x1, y1); glVertex2f(x0_1, y1);
      glVertex2f(x0, y1); glVertex2f(x0, y+0.02*scale_y);
      glVertex2f(x0_1, y+0.02*scale_y); glVertex2f(x1, y+0.02*scale_y);
      glVertex2f(x1, y+0.02*scale_y); glVertex2f(x1, y0);
      glVertex2f(x0_1, y0); glVertex2f(x1, y0);
      break;
    case '6':
      glVertex2f(x0, y1); glVertex2f(x0, y0);
      glVertex2f(x0, y0); glVertex2f(x1, y0);
      glVertex2f(x1, y0); glVertex2f(x1, y+0.02*scale_y);
      glVertex2f(x1, y+0.02*scale_y); glVertex2f(x0, y+0.02*scale_y);
      break;
    case '7':
      glVertex2f(x0, y1); glVertex2f(x1-korekta_x, y1);
      glVertex2f(x1, y1+korekta_y); glVertex2f(x+0.02*scale_x, y0);
      break;
    case '8':
      glVertex2f(x0, y0); glVertex2f(x0, y1);
      glVertex2f(x0, y1); glVertex2f(x1, y1);
      glVertex2f(x1, y+0.02*scale_y); glVertex2f(x0, y+0.02*scale_y);
      glVertex2f(x0, y0); glVertex2f(x1, y0);
      glVertex2f(x1, y0); glVertex2f(x1, y1);
      break;
    case '9':
      glVertex2f(x1, y0); glVertex2f(x1, y1);
      glVertex2f(x1, y1); glVertex2f(x0, y1);
      glVertex2f(x0, y1); glVertex2f(x0, y+0.02*scale_y);
      glVertex2f(x0, y+0.02*scale_y); glVertex2f(x1, y+0.02*scale_y);
      break;
    case 'A':
      glVertex2f(x1, y0_1); glVertex2f(x1, y1);
      glVertex2f(x1, y1); glVertex2f(x0, y1);
      glVertex2f(x0, y-0.02*scale_y); glVertex2f(x1, y-0.02*scale_y);
//      glVertex2f(x1, y-0.02*scale); glVertex2f(x0, y-0.02*scale);
      glVertex2f(x0, y1); glVertex2f(x0, y0_1);
      break;
    case 'B':
      glVertex2f(x0, y0_1); glVertex2f(x0, y1_1);
      glVertex2f(x0, y1); glVertex2f(x+0.06*scale_x-korekta_x, y1);
      glVertex2f(x+0.06*scale_x-korekta_x, y1); glVertex2f(x+0.07*scale_x+korekta_x, y+0.18*scale_y-korekta_y);
      glVertex2f(x+0.07*scale_x, y+0.18*scale_y); glVertex2f(x+0.07*scale_x, y+0.04*scale_y);
      glVertex2f(x+0.07*scale_x+korekta_x, y+0.04*scale_y+korekta_y); glVertex2f(x+0.06*scale_x-korekta_x, y+0.02*scale_y);
      glVertex2f(x+0.06*scale_x, y+0.02*scale_y); glVertex2f(x0, y+0.02*scale_y);

      glVertex2f(x+0.06*scale_x-korekta_x, y+0.02*scale_y); glVertex2f(x1_1, y-0.03*scale_y-korekta_y);
      glVertex2f(x1, y-0.03*scale_y); glVertex2f(x1, y0+0.05*scale_y);
      glVertex2f(x+0.05*scale_x, y0); glVertex2f(x1_1, y0+0.05*scale_y+korekta_y);
      glVertex2f(x0, y0); glVertex2f(x+0.05*scale_x, y0);
      break;

    case 'P':
      glVertex2f(x0, y0_1); glVertex2f(x0, y1_1);
      glVertex2f(x0, y1); glVertex2f(x+0.06*scale_x-korekta_x, y1);
      glVertex2f(x+0.06*scale_x-korekta_x, y1); glVertex2f(x+0.07*scale_x+korekta_x, y+0.18*scale_y-korekta_y);
      glVertex2f(x+0.07*scale_x, y+0.18*scale_y); glVertex2f(x+0.07*scale_x, y+0.04*scale_y);
      glVertex2f(x+0.07*scale_x+korekta_x, y+0.04*scale_y+korekta_y); glVertex2f(x+0.06*scale_x-korekta_x, y+0.02*scale_y);
      glVertex2f(x+0.06*scale_x, y+0.02*scale_y); glVertex2f(x0, y+0.02*scale_y);
      break;
  }
  glEnd();
}

void T_PlotsStack::SetWGLFont(CBitmapFont  *m_bmf_in)
{
  m_bmf = m_bmf_in;
}

/*! \todo Add subplot mode - position and size related to subplot
 *    not to client window
 */
void T_PlotsStack::DrawString(GLfloat x, GLfloat y, char *text,
              GLfloat height, GLfloat width,
              E_DS_mode mode)
{
  GLfloat x_pixel, y_pixel;
  GLfloat x_norm,  y_norm;
  GLfloat h_pixel, w_pixel;
  GLfloat h_norm,  w_norm;

  switch (mode & DS_unit_mask)
  {
    case DS_unit_normalized:
      // convert pixels to normalized
      x_norm = x; y_norm = y;
      x_pixel = ((x_norm + 1)*client_W)/2;
      y_pixel = ((y_norm + 1)*client_H)/2;

      h_norm = height; w_norm = width;
      w_pixel = (w_norm*client_W)/2;
      h_pixel = (h_norm*client_H)/2;
      break;

    case DS_unit_pixel:
    default:
      // convert normalized to pixels
      x_pixel = x; y_pixel = y+1;
      x_norm =(2 * x_pixel / client_W) - 1;
      y_norm =(2 * y_pixel / client_H) - 1;

      h_pixel = height; w_pixel = width;
      w_norm =(2 * w_pixel / client_W);
      h_norm =(2 * h_pixel / client_H);
      break;
  }

  //glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT); // lighting and color mask
  //glDisable(GL_LIGHTING);     // need to disable lighting for proper text color
  glPushMatrix();

  //char text[1024];
  //text[0] = 0xc4;  text[1] = 0x85; // � UTF8
  //strcpy(text+2, "Proba mikrofonu");

  switch (mode & DS_method_mask)
  {
    #ifdef GLUT_API_VERSION
      case DS_GLUT_bitmap:
        // ++++++++++++++++++++++++++++++++
        // GLUT bitmap (no font rotation)
        //glColor3f(1.0, 1.0, 1.0);
        glRasterPos2f(0.0, 0.0);
        //glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_10, 'X');
        glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_10, (const unsigned char *)(text));
        break;

      case DS_GLUT_stroke:
        // ++++++++++++++++++++++++++++++++
        // GLUT stroke (with font rotation)
        //glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        //glRotatef(90, 0, 0, 1);
        //glColor3f(1.0, 1.0, 1.0);
        //glLineWidth(3);
        glTranslatef(0.0, 0.0, 0.0);
        glScalef(0.0005, 0.0005, 0.0005);
        ////glutStrokeCharacter(GLUT_STROKE_ROMAN, 'X');
        glutStrokeString(GLUT_STROKE_ROMAN, (const unsigned char *)(text));
        break;
    #endif // GLUT_API_VERSION

    #ifdef _USE_GLC_
    case DS_GLC_line:
      // ++++++++++++++++++++++++++++++++
      // GLC stroke (with font rotation and UTF8)
      glcRenderStyle(GLC_LINE);
      //glcStringType(GLC_UTF8_QSO);
      glLoadIdentity();
      glTranslatef( x_norm, y_norm, 0.0);
      glScalef( w_norm, h_norm, 0.0);
      //glRotatef(90, 0, 0, 1);
      //glColor3f ( 1.f, 1.f, 0.f );
      glcRenderString(text);

      /*
      GLfloat bbox[8];
      glColor3f(0.f, 1.f, 1.f);
      glLoadIdentity();
      glTranslatef( x_norm, y_norm, 0.0);
      glScalef( w_norm, h_norm, 0.0);
      //glScalef(0.2, 0.2, 0.0);
      glcGetCharMetric('H', GLC_BOUNDS, bbox);
      glBegin(GL_LINE_LOOP);
      for (int i = 0; i < 4; i++)
        glVertex2fv(&bbox[2*i]);
      glEnd();
      */
      break;

    case DS_GLC_bitmap:
      // ++++++++++++++++++++++++++++++++
      // GLC bitmap (with font rotation and UTF8)
      glLoadIdentity();
      glcRenderStyle(GLC_BITMAP);
      //glcStringType(GLC_UTF8_QSO);
      glcLoadIdentity();
      glcScale(w_pixel, h_pixel); // in pixels
      //glColor3f ( 1.f, 1.f, 0.f );
      glRasterPos2f ( x_norm, y_norm );
      //glcRotate(90); // rotation
      glcRenderString(text);

      break;
    #endif // _USE_GLC_

    case DS_WGL_bitmap:
      // ++++++++++++++++++++++++++++++++
      // WGL bitmap (nice font without font rotation and scaling)
/*!      NOTES
     To set a valid raster position outside the viewport, first
     set a valid raster position inside the viewport, then call
     glBitmap with NULL as the bitmap parameter and with xmove
     and ymove set to the offsets of the new raster position.
     This technique is useful when panning an image around the
     viewport.
   */
      if (m_bmf != NULL)
        m_bmf->DrawStringAt(x_norm, y_norm, 0.0, text);
      else
        DSP::log << DSP::e::LogMode::Error <<"T_PlotsStack::DrawString" << DSP::e::LogMode::second <<"WGL bitmap font not set"<< std::endl;

      break;

    default:
      DSP::log << DSP::e::LogMode::Error <<"T_PlotsStack::DrawString"<< DSP::e::LogMode::second <<"Unsupported drawing mode"<< std::endl;
      break;
  }

  glPopMatrix();
  //glEnable(GL_LIGHTING);
  //glPopAttrib();

}

/*! \todo make use of arrays like in DrawSpecgram2
 */
void T_PlotsStack::DrawSignal(int SegmentSize, DSP::Float *Ydata, float skala,
                              DS_type type, float width)
{
  float x_offset;
  float tmp;
  float ind_factor;
  int ind;
  //glColor3f(0.0, 1.0, 1.0);
  // Line antialiasing is controlled by calling glEnable and glDisable with argument GL_LINE_SMOOTH

//  float max_data = 0.0;

  glLineWidth(width);
  glBegin(GL_LINE_STRIP);

  switch (type)
  {
    case DS_unsigned:
      skala = skala*2;
      break;
    default:
      break;
  }

  ind_factor = 2.0/SegmentSize;
//  x0=-1.0; // -1 : +1 <- segment offset
  x_offset = -1.0;
  for (ind=0; ind<SegmentSize; ind++)
  {
//    if (Ydata[ind] > max_data)
//      max_data = Ydata[ind];
    switch (type)
    {
      case DS_unsigned:
        tmp=Ydata[ind]*skala - 1.0;
        break;
      case DS_signed:
      default:
        tmp=Ydata[ind]*skala;
        break;
    }
    //x_offset = ind_factor*ind;
    //x=x0+x_offset;
    glVertex2f(x_offset, tmp);
    x_offset += ind_factor;
  }

  glEnd();

/*
  if (type == DS_signed)
  {
    wxString tekst;
    tekst=""; tekst << max_data;
    frame->SetLabel(tekst);
  }
 */
}

void T_PlotsStack::DrawSignal_dB(int SegmentSize, DSP::Float *Ydata,
                                 float dB_max, float dB_range, float width)
{
  float x_offset;
  float tmp;
  float ind_factor;
  int ind;
  float Y_scale, Y_offset;

  // konwertuj na przedzia� [-1, +1]
  Y_scale = 2/dB_range;
  Y_offset = dB_max-dB_range/2; // max ==> dB_range/2

  // Line antialiasing is controlled by calling glEnable and glDisable with argument GL_LINE_SMOOTH
  glLineWidth(width);
  glBegin(GL_LINE_STRIP);

  ind_factor = 2.0/SegmentSize;
  x_offset = -1.0;
  for (ind=0; ind<SegmentSize; ind++)
  {
    tmp = Y_scale*(Ydata[ind]-Y_offset);
    glVertex2f(x_offset, tmp);
    x_offset += ind_factor;
  }

  glEnd();
}

void T_PlotsStack::SubPlot(int Rows, int Cols, int index, int WindowW, int WindowH, bool Clear)
{
  int x, y;
  bool no_border;

  no_border = false;
  if (index < 0)
  {
    index = -index;
    no_border = true;
  }
  x=(index-1)%Cols;
  y=(index-1)/Cols + 1;

  if ((WindowW != 0) && (WindowH != 0))
  {
    client_W = WindowW;
    client_H = WindowH;

    //char text[1024];
    //sprintf(text, "client_W = %i, client_H = %i", client_W, client_H);
    //DSP::log << "T_PlotsStack::SubPlot", text);

    axis_dx = 20; axis_dy = 20;
    axis_dw = 10 + axis_dx; axis_dh = 10 + axis_dy;
  }

  subplot_W = client_W/Cols;
  subplot_H = client_H/Rows;
  subplot_X = x*subplot_W;
  //subplot_Y = client_H-y*subplot_H-2;
  subplot_Y = client_H-y*subplot_H;

  if (no_border == true)
  {
    glViewport(subplot_X, subplot_Y,
        subplot_W, subplot_H);
    glScissor (subplot_X, subplot_Y,
        subplot_W, subplot_H);
    glEnable (GL_SCISSOR_TEST);

    subplot_x_scale = 2.0/(subplot_W);
    subplot_y_scale = 2.0/(subplot_H);
  }
  else
  {
    glViewport(subplot_X + axis_dx, subplot_Y + axis_dy,
        subplot_W - axis_dw, subplot_H - axis_dh);
    glScissor (subplot_X + axis_dx, subplot_Y + axis_dy,
        subplot_W - axis_dw, subplot_H - axis_dh);
    glEnable (GL_SCISSOR_TEST);

    subplot_x_scale = 2.0/(subplot_W - axis_dw);
    subplot_y_scale = 2.0/(subplot_H - axis_dh);
  }
  if (Clear == true)
  {
    glClearColor(BackgroundRGB[0],BackgroundRGB[1],BackgroundRGB[2],0);
    glClear(GL_COLOR_BUFFER_BIT);
    // glDisable (GL_SCISSOR_TEST); glClear (...); glEnable (GL_SCISSOR_TEST);

    /*
    GLfloat ClearColor[4], CurrentColor[4];
    glGetFloatv(GL_COLOR_CLEAR_VALUE, ClearColor);
    glGetFloatv(GL_CURRENT_COLOR, CurrentColor);
    glColor3fv(ClearColor);

    glBegin(GL_POLYGON);
      glVertex2f(-1, -1);
      glVertex2f(-1, 1);
      glVertex2f(1, 1);
      glVertex2f(1, -1);
    glEnd();
    glColor3fv(CurrentColor);
    */
  }
}

// Gets coordinates related to given subplot
/* Input: (0,0) <-- upper left corner
 *  - x: GLcanvas mouse x coordinates
 *  - y: GLcanvas mouse y coordinates
 *  .
 *  Output:
 *  - x: axis x coordinates (-1, +1)
 *  - y: axis y coordinates (-1, +1)
 *  .
 */
void T_PlotsStack::GetSubPlotCords(int Rows, int Cols, int index, float &x_in, float &y_in)
{
  int x, y;
  bool no_border;

  no_border = false;
  if (index < 0)
  {
    index = -index;
    no_border = true;
  }
  x=(index-1)%Cols;
  y=(index-1)/Cols + 1;

  y_in = client_H-1 - y_in;
  subplot_W = client_W/Cols;
  subplot_H = client_H/Rows;
  subplot_X = x*subplot_W;
  subplot_Y = client_H-y*subplot_H-2;

  if (no_border == true)
  {
    subplot_x_scale = 2.0/(subplot_W);
    subplot_y_scale = 2.0/(subplot_H);

    x_in = (x_in - subplot_X)*subplot_x_scale - 1.0;
    y_in = (y_in - subplot_Y)*subplot_y_scale - 1.0;
  }
  else
  {
    subplot_x_scale = 2.0/(subplot_W - axis_dw);
    subplot_y_scale = 2.0/(subplot_H - axis_dh);

    x_in = (x_in - subplot_X-axis_dx)*subplot_x_scale - 1.0;
    y_in = (y_in - subplot_Y-axis_dy)*subplot_y_scale - 1.0;
  }
}

bool T_PlotsStack::ticks_textures_initialized = false;
GLuint T_PlotsStack::ticks_x_textures[3];
GLuint T_PlotsStack::ticks_y_textures[3];

void CopyTexture(unsigned char *dest64, unsigned char *source, int w, int h)
{
  int ind, ind_x, ind_y, ind_c;

  ind = 0;
  for (ind_y =0; ind_y < h; ind_y++)
    for (ind_x =0; ind_x < w; ind_x++)
      for (ind_c =0; ind_c < 4; ind_c++)
      {
        dest64[4*(ind_x+ind_y*TICKS_TEXTURE_MAXSIZE)+ind_c] = 0x64;
        ind++;
      }

  ind = 0;
  for (ind_y =0; ind_y < h; ind_y++)
    for (ind_x =0; ind_x < w; ind_x++)
      for (ind_c =0; ind_c < 4; ind_c++)
      {
        dest64[4*(ind_x+ind_y*TICKS_TEXTURE_MAXSIZE)+ind_c] = source[ind];
        ind++;
      }
}

/*! \bug Todo
 *    - user defined unit size
 *    - user defined maximum ticks density
 *    .
 *
 * @param dt time axis length
 * @param t_unit time main unit size (<= 0.0 auto select)
 * @param dF frequency axis length
 * @param F_unit frequency main unit size (<= 0.0 auto select)
 * @param ticks_mode_in
 */
void T_PlotsStack::InitAxis(
    double dt, double dF,
    double t_unit,double F_unit,
    E_PS_ticks_mode ticks_mode_in)
{
  int axis_W, axis_H;
  axis_W = subplot_W - axis_dw;
  axis_H = subplot_H - axis_dh;

  ticks_mode = ticks_mode_in;

  // ++++++++++++++++++++++++++++++++++++++++++++++++ //
  //  X axis
  // ++++++++++++++++++++++++++++++++++++++++++++++++ //
  if (t_unit <= 0.0)
  {
    // dt = a * 10^b = a * 10^c * metric unit (eg. [ms])
    // 10.0 > a >= 1.0
    // 10^b - unit
    int b = int(floor(log10(dt)));
    //! x axis unit size
    x_unit = pow(10,b);
  }
  else
  {
    x_unit = t_unit;
  }
  //! number of main units per x axis (should be > 0 but might be < 1)
  x_units_per_axis = dt/x_unit;
  x_pixels_per_unit = axis_W / x_units_per_axis;

  // ++++++++++++++++++++++++++++++++++++++++++++++++ //
  //  Y axis
  // ++++++++++++++++++++++++++++++++++++++++++++++++ //
  if (F_unit <= 0.0)
  {
    int dFb = int(floor(log10(dF)));
    //! x axis unit size
    y_unit = pow(10,dFb);
  }
  else
  {
    y_unit = F_unit;
  }
  y_units_per_axis = dF/y_unit;
  y_pixels_per_unit = axis_H / y_units_per_axis;

  switch (ticks_mode)
  {
    case E_TM_white:
      CopyTexture(x_subsubticks_pixels, x_subsubticks_pixels_W, 3, 5);
      CopyTexture(x_subticks_pixels, x_subticks_pixels_W, 3, 7);
      CopyTexture(x_ticks_pixels, x_ticks_pixels_W, 5, 9);
      CopyTexture(y_subsubticks_pixels, y_subsubticks_pixels_W, 5, 3);
      CopyTexture(y_subticks_pixels, y_subticks_pixels_W, 7, 3);
      CopyTexture(y_ticks_pixels, y_ticks_pixels_W, 9, 5);
      break;
    case E_TM_black:
      CopyTexture(x_subsubticks_pixels, x_subsubticks_pixels_B, 3, 5);
      CopyTexture(x_subticks_pixels, x_subticks_pixels_B, 3, 7);
      CopyTexture(x_ticks_pixels, x_ticks_pixels_B, 5, 9);
      CopyTexture(y_subsubticks_pixels, y_subsubticks_pixels_B, 5, 3);
      CopyTexture(y_subticks_pixels, y_subticks_pixels_B, 7, 3);
      CopyTexture(y_ticks_pixels, y_ticks_pixels_B, 9, 5);
      break;
    case E_TM_ignore:
    default:
      // ignore
      break;
  }

  if (ticks_textures_initialized == true)
  {
    glDeleteTextures(3, ticks_x_textures);
    glDeleteTextures(3, ticks_y_textures);
  }
  glGenTextures(3, ticks_x_textures);
  glGenTextures(3, ticks_y_textures);
  ticks_textures_initialized = true;


  glBindTexture(GL_TEXTURE_2D, ticks_x_textures[2]);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // GL_CLAMP);
  glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // GL_CLAMP);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // GL_DECAL);
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, // 3 color components
        TICKS_TEXTURE_MAXSIZE, TICKS_TEXTURE_MAXSIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, x_subsubticks_pixels );

  glBindTexture(GL_TEXTURE_2D, ticks_x_textures[1]);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // GL_CLAMP);
  glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // GL_CLAMP);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // GL_DECAL);
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, // 3 color components
        TICKS_TEXTURE_MAXSIZE, TICKS_TEXTURE_MAXSIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, x_subticks_pixels );

  glBindTexture(GL_TEXTURE_2D, ticks_x_textures[0]);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // GL_CLAMP);
  glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // GL_CLAMP);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // GL_DECAL);
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, // 3 color components
        TICKS_TEXTURE_MAXSIZE, TICKS_TEXTURE_MAXSIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, x_ticks_pixels );
//      5, 9, 0, GL_RGBA, GL_UNSIGNED_BYTE, x_ticks_pixels );


  glBindTexture(GL_TEXTURE_2D, ticks_y_textures[2]);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // GL_CLAMP);
  glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // GL_CLAMP);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // GL_DECAL);
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, // 3 color components
        TICKS_TEXTURE_MAXSIZE, TICKS_TEXTURE_MAXSIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, y_subsubticks_pixels );

  glBindTexture(GL_TEXTURE_2D, ticks_y_textures[1]);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // GL_CLAMP);
  glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // GL_CLAMP);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // GL_DECAL);
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, // 3 color components
        TICKS_TEXTURE_MAXSIZE, TICKS_TEXTURE_MAXSIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, y_subticks_pixels );

  glBindTexture(GL_TEXTURE_2D, ticks_y_textures[0]);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // GL_CLAMP);
  glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // GL_CLAMP);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // GL_DECAL);
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, // 3 color components
        TICKS_TEXTURE_MAXSIZE, TICKS_TEXTURE_MAXSIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, y_ticks_pixels );

}

/*! \bug Todo
 *   - do not draw subticks under main tick
 *   - adjust ticks position
 *   - DONE // check if smaller texture sizes will work
 *   .
 *
 */
void T_PlotsStack::PlotAxis(double to, double Fo, bool use_XOR)
{
  // ++++++++++++++++++++++++++++++++++++++ //
  // ++++++++++++++++++++++++++++++++++++++ //
  int axis_W, axis_H;
  axis_W = subplot_W - axis_dw;
  axis_H = subplot_H - axis_dh;

  //glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); glEnable(GL_LINE_SMOOTH);

  //glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();

  switch (ticks_mode)
  {
    case E_TM_white:
      glColor3f(1.0, 1.0, 1.0);
      break;
    case E_TM_black:
    default:
      glColor3f(0.0, 0.0, 0.0);
      break;
  }
  glLineWidth(1);
  // added 1 pixel frame for box
  //glViewport(subplot_X + axis_dx, subplot_Y + axis_dy,
  //    subplot_W - axis_dw, subplot_H - axis_dh);
  glViewport(subplot_X + axis_dx -1  , subplot_Y + axis_dy -1,
             axis_W + 2 , axis_H + 2);
  glScissor(subplot_X + axis_dx -1  , subplot_Y + axis_dy -1,
             axis_W + 2 , axis_H + 2);
  glOrtho(0, axis_W+2,   0, axis_H+2, -1, 1);
  glEnable (GL_SCISSOR_TEST);

  glBegin(GL_LINE_LOOP);
    glVertex2i(0, 0);
    glVertex2i(0, axis_H +1);
    glVertex2i(axis_W + 1, axis_H + 1);
    glVertex2i(axis_W + 1, 0);
  glEnd();

  /*! \todo
   *   - ticks
   *   - labels
   *   - ticklabels
   */
  /*! \todo ticks
   *   - get unit
   *   - main ticks at k*unit
   *   - ticks at 0.1*k*unit
   *   - if no of ticks <= 2 the draw subticks
   *     - subticks at 0.025*k*unit
   *   - first tick position
   *     - first main tick at ceil(to/unit)
   *     - first tick at ceil(10*to/unit)/10
   *     - first subtick at ceil(40*to/unit)/40
   *
   *  \bug units should be calculated beforehand at PlotStack initialization
   */

  //glEnable(GL_ALPHA_TEST);
  //glDisable(GL_ALPHA_TEST);
  //glDisable(GL_POINT_SMOOTH);
  //glDisable(GL_POLYGON_SMOOTH);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  if (use_XOR == true)
  {
    glEnable(GL_COLOR_LOGIC_OP);
    glLogicOp(GL_XOR);
  }
  // ++++++++++++++++++++++++++++++++++++++++++++++++ //
  //  X axis
  // ++++++++++++++++++++++++++++++++++++++++++++++++ //
  double to_u, To;
  double dTm, dTs, dTss;

  to_u = to/x_unit;
  To = ceil(to_u);
  //! main ticks offset
  dTm = To - to_u; // >= 0 <== offset to first main tick
  //! sub ticks offset
  dTs = dTm - floor(dTm*10)/10; // <== offset to first tick
  //! subsubticks offset
  dTss = dTs - floor(dTs*40)/40; // <== offset to first subtick

  int x, x0, y0;
  // one pixel frame
  x0 = 1; y0 = 1;
  for (int ind0 = 0; ind0 <x_units_per_axis; ind0++)
  {
    for (int ind1 = 0; ind1 < 10; ind1++)
    {
      if (x_units_per_axis <= 0.2)
      { // draw subsubticks
        for (int ind2 = 0; ind2 < 4; ind2++)
        {
          x = int((dTss + ind0 + 0.1*ind1 + 0.025*ind2) * x_pixels_per_unit);

          //glRasterPos2i(x0 + x, y0);
          //glDrawPixels(3, //GLsizei width,
          //             5, //GLsizei height,
          //             GL_RGBA, //GLenum format,
          //             GL_UNSIGNED_BYTE, //GLenum type,
          //             x_subsubticks_pixels); //const GLvoid *pixels);
          glEnable( GL_TEXTURE_2D );
          glBindTexture ( GL_TEXTURE_2D, ticks_x_textures[2] );
          glBegin( GL_QUADS );
          glTexCoord2d(0.0, 0.0);
          glVertex2d( x0 + x -1, y0);
          glTexCoord2d(3*1.0/TICKS_TEXTURE_MAXSIZE, 0.0);
          glVertex2d( x0 + x +1+1, y0);
          glTexCoord2d(3*1.0/TICKS_TEXTURE_MAXSIZE, 5*1.0/TICKS_TEXTURE_MAXSIZE);
          glVertex2d( x0 + x +1+1, y0+5);
          glTexCoord2d(0.0, 5*1.0/TICKS_TEXTURE_MAXSIZE);
          glVertex2d( x0 + x -1, y0+5);
          glEnd();
          glDisable( GL_TEXTURE_2D );
        }
      }

      // draw subticks
      x = int((dTs + ind0 + 0.1*ind1) * x_pixels_per_unit);
      //glRasterPos2i(x0 + x, y0);
      //glDrawPixels(3, //GLsizei width,
      //             7, //GLsizei height,
      //             GL_RGBA, //GLenum format,
      //             GL_UNSIGNED_BYTE, //GLenum type,
      //             x_subticks_pixels); //const GLvoid *pixels);
      glEnable( GL_TEXTURE_2D );
      glBindTexture ( GL_TEXTURE_2D, ticks_x_textures[1] );
      glBegin( GL_QUADS );
      glTexCoord2d(0.0, 0.0);
      glVertex2d( x0 + x -1, y0);
      glTexCoord2d(3*1.0/TICKS_TEXTURE_MAXSIZE, 0.0);
      glVertex2d( x0 + x +1+1, y0);
      glTexCoord2d(3*1.0/TICKS_TEXTURE_MAXSIZE, 7*1.0/TICKS_TEXTURE_MAXSIZE);
      glVertex2d( x0 + x +1+1, y0+7);
      glTexCoord2d(0.0, 7*1.0/TICKS_TEXTURE_MAXSIZE);
      glVertex2d( x0 + x -1, y0+7);
      glEnd();
      glDisable( GL_TEXTURE_2D );
    }

    // draw main ticks
    x = int((dTm + ind0) * x_pixels_per_unit);
    /*
    glRasterPos2i(x0 + x-1, y0);
    glDrawPixels(5, //GLsizei width,
                 9, //GLsizei height,
                 GL_RGBA, //GLenum format,
                 GL_UNSIGNED_BYTE, //GLenum type,
                 x_ticks_pixels); //const GLvoid *pixels);
                 */

    glEnable( GL_TEXTURE_2D );
    glBindTexture ( GL_TEXTURE_2D, ticks_x_textures[0] );
    glBegin( GL_QUADS );
    glTexCoord2d(0.0, 0.0);
    glVertex2d( x0 + x -2, y0);
    glTexCoord2d(5*1.0/TICKS_TEXTURE_MAXSIZE, 0.0);
    glVertex2d( x0 + x +2+1, y0);
    glTexCoord2d(5*1.0/TICKS_TEXTURE_MAXSIZE, 9*1.0/TICKS_TEXTURE_MAXSIZE);
    glVertex2d( x0 + x +2+1, y0+9);
    glTexCoord2d(0.0, 9*1.0/TICKS_TEXTURE_MAXSIZE);
    glVertex2d( x0 + x -2, y0+9);
    glEnd();
    glDisable( GL_TEXTURE_2D );
    //! draw main ticks
  }

  // ++++++++++++++++++++++++++++++++++++++++++++++++ //
  //  Y axis
  // ++++++++++++++++++++++++++++++++++++++++++++++++ //
  double y_Fo_u;
  double y_Fo;
  double dFm, dFs, dFss;

  y_Fo_u = Fo/y_unit;
  y_Fo = ceil(y_Fo_u);
  //! main ticks offset
  dFm = y_Fo - y_Fo_u; // >= 0 <== offset to first main tick
  //! sub ticks offset
  dFs = dFm - floor(dFm*10)/10; // <== offset to first tick
  //! subsubticks offset
  dFss = dFs - floor(dFs*40)/40; // <== offset to first subtick

  int y;
  for (int ind0 = 0; ind0 <y_units_per_axis; ind0++)
  {
    //for (int ind1 = 0; ind1 < 10; ind1++)
    for (int ind1 = 0; ind1 < 10; ind1++)
    {
      if (y_units_per_axis <= 0.2)
      { // draw subsubticks
        for (int ind2 = 0; ind2 < 4; ind2++)
        {
          y = int((dFss + ind0 + 0.1*ind1 + 0.025*ind2) * y_pixels_per_unit);

          //glRasterPos2i(x0, y0+y);
          //glDrawPixels(5, //GLsizei width,
          //             3, //GLsizei height,
          //             GL_RGBA, //GLenum format,
          //             GL_UNSIGNED_BYTE, //GLenum type,
          //             y_subsubticks_pixels); //const GLvoid *pixels);
          glEnable( GL_TEXTURE_2D );
          glBindTexture ( GL_TEXTURE_2D, ticks_y_textures[2] );
          glBegin( GL_QUADS );
          glTexCoord2d(0.0, 0.0);
          glVertex2d( x0, y + y0-1);
          glTexCoord2d(5*1.0/TICKS_TEXTURE_MAXSIZE, 0.0);
          glVertex2d( x0+5, y + y0-1);
          glTexCoord2d(5*1.0/TICKS_TEXTURE_MAXSIZE, 3*1.0/TICKS_TEXTURE_MAXSIZE);
          glVertex2d( x0+5, y + y0+1+1);
          glTexCoord2d(0.0, 3*1.0/TICKS_TEXTURE_MAXSIZE);
          glVertex2d( x0, y + y0+1+1);
          glEnd();
          glDisable( GL_TEXTURE_2D );
        }
      }

      // draw subticks
      //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      y = int((dFs + ind0 + 0.1*ind1) * y_pixels_per_unit);
      /*
      glRasterPos2i(x0, y0+y+0);
      glDrawPixels(7, //GLsizei width,
                   1, //GLsizei height,
                   GL_RGBA, //GLenum format,
                   GL_UNSIGNED_BYTE, //GLenum type,
                   y_subticks_pixels+4*7); //const GLvoid *pixels);
      glRasterPos2i(x0, y0+y-1);
      glDrawPixels(7, //GLsizei width,
                   1, //GLsizei height,
                   GL_RGBA, //GLenum format,
                   GL_UNSIGNED_BYTE, //GLenum type,
                   y_subticks_pixels); //const GLvoid *pixels);
      glRasterPos2i(x0, y0+y+1);
      glDrawPixels(7, //GLsizei width,
                   1, //GLsizei height,
                   GL_RGBA, //GLenum format,
                   GL_UNSIGNED_BYTE, //GLenum type,
                   y_subticks_pixels+2*4*7); //const GLvoid *pixels);
                   */
      /*glDrawPixels(7, //GLsizei width,
                   3, //GLsizei height,
                   GL_RGBA, //GLenum format,
                   GL_UNSIGNED_BYTE, //GLenum type,
                   y_subticks_pixels); //const GLvoid *pixels);
                   */
      glEnable( GL_TEXTURE_2D );
      glBindTexture ( GL_TEXTURE_2D, ticks_y_textures[1] );
      glBegin( GL_QUADS );
      glTexCoord2d(0.0, 0.0);
      glVertex2d( x0, y + y0-1);
      glTexCoord2d(7*1.0/TICKS_TEXTURE_MAXSIZE, 0.0);
      glVertex2d( x0+7, y + y0-1);
      glTexCoord2d(7*1.0/TICKS_TEXTURE_MAXSIZE, 3*1.0/TICKS_TEXTURE_MAXSIZE);
      glVertex2d( x0+7, y + y0+1+1);
      glTexCoord2d(0.0, 3*1.0/TICKS_TEXTURE_MAXSIZE);
      glVertex2d( x0, y + y0+1+1);
      glEnd();
      glDisable( GL_TEXTURE_2D );
    }

    // draw main ticks
    y = int((dFm + ind0) * y_pixels_per_unit);

    /*! \bug w okolicy linii �rodkowej zmienia si� offset pixela o 1
    glRasterPos2i(x0, y0+y-1);
    glDrawPixels(9, //GLsizei width,
                 3, //GLsizei height,
                 GL_RGBA, //GLenum format,
                 GL_UNSIGNED_BYTE, //GLenum type,
                 y_ticks_pixels+4*9); //const GLvoid *pixels);
    glRasterPos2i(x0, y0+y-2);
    glDrawPixels(9, //GLsizei width,
                 1, //GLsizei height,
                 GL_RGBA, //GLenum format,
                 GL_UNSIGNED_BYTE, //GLenum type,
                 y_ticks_pixels); //const GLvoid *pixels);
    glRasterPos2i(x0, y0+y+2);
    glDrawPixels(9, //GLsizei width,
                 1, //GLsizei height,
                 GL_RGBA, //GLenum format,
                 GL_UNSIGNED_BYTE, //GLenum type,
                 y_ticks_pixels+4*4*9); //const GLvoid *pixels);
    */
    /*
    glRasterPos2i(x0, y0+y-1);
    glDrawPixels(9, //GLsizei width,
                 5, //GLsizei height,
                 GL_RGBA, //GLenum format,
                 GL_UNSIGNED_BYTE, //GLenum type,
                 y_ticks_pixels); //const GLvoid *pixels);
                 */
    //! draw main ticks
    glEnable( GL_TEXTURE_2D );
    glBindTexture ( GL_TEXTURE_2D, ticks_y_textures[0] );
    glBegin( GL_QUADS );
    glTexCoord2d(0.0, 0.0);
    glVertex2d( x0, y + y0-2);
    glTexCoord2d(9*1.0/TICKS_TEXTURE_MAXSIZE, 0.0);
    glVertex2d( x0+9, y + y0-2);
    glTexCoord2d(9*1.0/TICKS_TEXTURE_MAXSIZE, 5*1.0/TICKS_TEXTURE_MAXSIZE);
    glVertex2d( x0+9, y + y0+2+1);
    glTexCoord2d(0.0, 5*1.0/TICKS_TEXTURE_MAXSIZE);
    glVertex2d( x0, y + y0+2+1);
    glEnd();
    glDisable( GL_TEXTURE_2D );
  }
  if (use_XOR == true)
  {
    glDisable(GL_COLOR_LOGIC_OP);
  }
  glDisable(GL_BLEND);

  glPopMatrix();

  // ++++++++++++++++++++++++++++++++++++++ //
  // ++++++++++++++++++++++++++++++++++++++ //
  glEnable (GL_SCISSOR_TEST);
  //print labels

}

void T_PlotsStack::SpectrogramMouseCordsToRealCords(int x, int y, long double &t, long double &F)
{
  long double X, Y;
  long double ind_x;
  long double ind_y;
  long double dt;
  int floor_ind_x;

  // ++++++++++++++++++++++++++++++++++++++++++++ //
  //  Mouse cords to GL cords
  // ++++++++++++++++++++++++++++++++++++++++++++ //
  X = (long double)(x - (subplot_X + axis_dx));
  X /= (subplot_W - axis_dw);
  X *= 2; X -= 1;
  // in mouse cords (0,0) <- left upper corner
  // in GL cords (-1.0,-1.0) <- left bottom corner
  Y = (long double)((client_H - y) - (subplot_Y + axis_dy));
  Y /= (subplot_H - axis_dh);
  Y *= 2; Y -= 1;

  // ++++++++++++++++++++++++++++++++++++++++++++ //
  //  GL cords to time and FFT indexes
  // ++++++++++++++++++++++++++++++++++++++++++++ //
  //! \bug this does not allow for zoom (this can be done by X and Y correction above)
  //dx = 2.0/NoOfSlots; dy = 2.0/K; // <== K even
  //
  //X = ind_x*dx - 1 ... (ind_x+1)*dx - 1
  //(X + 1)/dx = ind_x ... (ind_x+1)
  //ind_x = (int) floor(((X + 1)*NoOfSlots)/2);
  ind_x = (((X + 1)*NoOfSlots)/2);
  ////Y = ind_y*dy - 1 ... (ind_y+1)*dy - 1
  ////(Y + 1)/dy = ind_y ... (ind_y+1)
  //ind_y = (int) floor(((Y + 1)*K)/2);
  //Y = dy_offset + ind_y*dy
  //ind_y = (Y-dy_offset) / dy
  //! \warning ind_y probably should not be rounded
  ind_y = (Y-dy_offset)/dy;
  //! \todo_later check if ind_x or ind_y fall out of range and return false in sauch case

  // ++++++++++++++++++++++++++++++++++++++++++++ //
  //  time and FFT indexes to analog time and frequency
  // ++++++++++++++++++++++++++++++++++++++++++++ //
  //t = ((NoOfSlots - ind_x) * NoOfSamplesPerAPSD) / sampling_rate;
  floor_ind_x = (int)floor(ind_x);
  if (floor_ind_x < 0)
    floor_ind_x = 0;
  if (floor_ind_x >= NoOfSlots)
    floor_ind_x = NoOfSlots-1;
  //dt = ((NoOfSlots - (ind_x-floor_ind_x)) * NoOfSamplesPerAPSD) / sampling_rate;
  dt = ((ind_x-floor_ind_x) * NoOfSamplesPerAPSD) / sampling_rate;
  t = Get_SlotTime(floor_ind_x) + dt;
  if (floor_ind_x == 0)
  {
    dt = ((ind_x-floor_ind_x) * NoOfSamplesPerAPSD) / sampling_rate;
    t = Get_SlotTime(floor_ind_x) + dt;
  }
  if ((K % 2) == 0)
  { // even FFT length
    F = (sampling_rate * (ind_y - K/2)) / K;
  }
  else
  { // odd FFT length
    F = (sampling_rate * (ind_y - (K-1)/2)) / K;
  }
  F += CenterFrequency;
}
bool T_PlotsStack::IsMouseInAxis(int x, int y)
{
  if ((x < subplot_X + axis_dx) || ((client_H - y) <=  (subplot_Y + axis_dy)) ||
      (x >= subplot_X + axis_dx + subplot_W - axis_dw) ||
      ((client_H - y) > (subplot_Y + axis_dy + subplot_H - axis_dh)))
    return false;
  return true;
}
bool T_PlotsStack::OnMouseDown(int x, int y)
{
  return IsMouseInAxis(x, y);
}
bool T_PlotsStack::OnMouseUp(int x, int y)
{
  return IsMouseInAxis(x, y);
}
bool T_PlotsStack::OnMouseMove(int x, int y, long double &X, long double &Y)
{
  // X - time, Y - frequency
  SpectrogramMouseCordsToRealCords(x, y, X, Y);

  return IsMouseInAxis(x, y);
}

void MoveImage_Left(float move_factor)
{
  int x, y;
  GLint H, W, MW;
  GLint viewport[4];

  glGetIntegerv(GL_VIEWPORT,viewport);

  x=viewport[0]; y=viewport[1];
  W=viewport[2]; H=viewport[3];

  glRasterPos2f(-1.0,-1.0);
  MW=(int)(floor(W*move_factor));
  glCopyPixels(MW, y, W-MW, H, GL_COLOR);
}




T_time_base::T_time_base(void)
{
  year = 0;
  month = 0;
  day = 0;

  real_hours = 0;
  real_minutes = 0;
  real_seconds = 0;

  // offset related to file beginning
  hours = 0;
  minutes = 0;
  seconds = 0;
  milliseconds = 0;
}

void T_time_base::GetTimeString(char *time_str, int mode)
{
  short s, m, h, d;

  switch (mode)
  {
    case 0:
      sprintf(time_str, "%i:%02i:%02i.%03i",
          (int)hours, (int)minutes, (int)seconds, (int)milliseconds);
      break;

    case 1:
      s = (short)(seconds + real_seconds);
      m = (short)(minutes + real_minutes);
      h = (short)(hours + real_hours);
      d = 0;
      while (s >= 60)
      { s -= 60; m ++; }
      while (m >= 60)
      { m -= 60; h ++; }
      while (h >= 24)
      { h -= 24; d ++; }
      if (d > 0)
      {
        sprintf(time_str, "%02i.%02i.%04i(+%i) %i:%02i:%02i.%03i",
            day, month, year, d,
            h, m, s, (int)milliseconds);
      }
      else
      {
        sprintf(time_str, "%02i.%02i.%04i %i:%02i:%02i.%03i",
            day, month, year,
            h, m, s, (int)milliseconds);
      }
      break;
    default:
      time_str[0] = 0x00;
      break;
  }
}

// compares current object with src
bool T_time_base::Cmp(const T_time_base &src)
{
  if (year != src.year)
    return false;
  if (month != src.month)
    return false;
  if (day != src.day)
    return false;

  if (real_hours != src.real_hours)
    return false;
  if (real_minutes != src.real_minutes)
    return false;
  if (real_seconds != src.real_seconds)
    return false;

  // offset related to file beginning
  if (hours != src.hours)
    return false;
  if (minutes != src.minutes)
    return false;
  if (seconds != src.seconds)
    return false;
  if (milliseconds != src.milliseconds)
    return false;

  return true;
}

// fills current object with data from  src
void T_time_base::Copy(const T_time_base &src)
{
  year = src.year;
  month = src.month;
  day = src.day;

  real_hours = src.real_hours;
  real_minutes = src.real_minutes;
  real_seconds = src.real_seconds;

  // offset related to file beginning
  hours = src.hours;
  minutes = src.minutes;
  seconds = src.seconds;
  milliseconds = src.milliseconds;
}
void T_time_base::CopyReferenceDateTime(const T_time_base &src)
{
  year = src.year;
  month = src.month;
  day = src.day;

  real_hours = src.real_hours;
  real_minutes = src.real_minutes;
  real_seconds = src.real_seconds;
}
// fills current object with date data from  date_tmp, but skips time data
void T_time_base::SetReferenceDate(const wxDateTime &date_tmp)
{
  year = date_tmp.GetYear();
  month = (date_tmp.GetMonth() - wxDateTime::Jan) + 1;
  day = date_tmp.GetDay();
}
// fills current object with time data from  date_tmp, but skips date data
void T_time_base::SetReferenceTime(const wxDateTime &date_tmp)
{
  real_hours = date_tmp.GetHour();
  real_minutes = date_tmp.GetMinute();
  real_seconds = date_tmp.GetSecond();
  //real_milliseconds = date_tmp.GetMillisecond();
}
// fills current object with date and time data from date_tmp
void T_time_base::SetReferenceDateTime(const wxDateTime &date_tmp)
{
  year = date_tmp.GetYear();
  month = (date_tmp.GetMonth() - wxDateTime::Jan) + 1;
  day = date_tmp.GetDay();

  real_hours = date_tmp.GetHour();
  real_minutes = date_tmp.GetMinute();
  real_seconds = date_tmp.GetSecond();
}
void T_time_base::SetTimeOffset(long long overall_seconds)
{
  milliseconds = 0;
  seconds = (overall_seconds % 60);
  overall_seconds -= (long long)seconds;
  overall_seconds /= 60;

  minutes = (overall_seconds % 60);
  overall_seconds -= (long long)minutes;
  overall_seconds /= 60;

  hours = overall_seconds;
}
void T_time_base::SetTimeOffset(long double overall_seconds_d)
{
  long long overall_seconds;

  overall_seconds = (long long)overall_seconds_d;

  milliseconds = 1000 * (overall_seconds_d - overall_seconds);
  seconds = (overall_seconds % 60);
  overall_seconds -= (long long)seconds;
  overall_seconds /= 60;

  minutes = (overall_seconds % 60);
  overall_seconds -= (long long)minutes;
  overall_seconds /= 60;

  hours = overall_seconds;
}
long long T_time_base::GetTimeOffset(void)
{
  long long overall_seconds;

  overall_seconds = (int)seconds;
  overall_seconds += (int)minutes*60;
  overall_seconds += (int)hours*3600;

  return overall_seconds;
}
// change date of reference time with offset (ignore time)
void T_time_base::SetOffsetDate(const wxDateTime &date_tmp)
{
  wxDateTime dt;
  wxTimeSpan ts;

  // Czas referencyjny
  dt.Set(day, wxDateTime::Month(wxDateTime::Jan+(month-1)), year,
         real_hours, real_minutes, real_seconds, 0);
  // uwzgl�dniamy time offset
  dt.Add(wxTimeSpan((long int)hours, (long int)minutes, (long int)seconds, (long int)milliseconds));

  // wymieniamy date
  dt.SetDay(date_tmp.GetDay());
  dt.SetMonth(date_tmp.GetMonth());
  dt.SetYear(date_tmp.GetYear());

  // Odejmujemy czas referencyjny
  ts = dt.Subtract(wxDateTime(day, wxDateTime::Month(wxDateTime::Jan+(month-1)), year,
         real_hours, real_minutes, real_seconds, 0));
  // i mamy nowy offset
  //! \bug offsets longe then 24 hours is not supported
  //hours = ts.GetHours(); // + dt.GetDay()*24;
  //minutes = ts.GetMinutes();
  //seconds = ts.GetSeconds();
  //milliseconds = ts.GetMilliseconds();
  long long temp;
  temp = ts.GetSeconds().GetValue();
  SetTimeOffset(temp);
}

// sets real time of file beginning (does not change date)
void T_time_base::SetOffsetTime(const wxDateTime &date_tmp)
{
  wxDateTime dt;
  wxTimeSpan ts;

  // Czas referencyjny
  dt.Set(day, wxDateTime::Month(wxDateTime::Jan+(month-1)), year,
         real_hours, real_minutes, real_seconds, 0);
  // uwzgl�dniamy time offset
  dt.Add(wxTimeSpan((long int)hours, (long int)minutes, (long int)seconds, (long int)milliseconds));

  // wymieniamy czas
  dt.SetHour(date_tmp.GetHour());
  dt.SetMinute(date_tmp.GetMinute());
  dt.SetSecond(date_tmp.GetSecond());
  dt.SetMillisecond(date_tmp.GetMillisecond());

  // Odejmujemy czas referencyjny
  ts = dt.Subtract(wxDateTime(day, wxDateTime::Month(wxDateTime::Jan+(month-1)), year,
         real_hours, real_minutes, real_seconds, 0));
  // i mamy nowy offset
  //! \bug offsets longe then 24 hours is not supported
  //hours = ts.GetHours(); // + dt.GetDay()*24;
  //minutes = ts.GetMinutes();
  //seconds = ts.GetSeconds();
  //milliseconds = ts.GetMilliseconds();
  long long temp;
  temp = ts.GetSeconds().GetValue();
  SetTimeOffset(temp);
}

void T_time_base::AddOffsetToReferenceTime(void)
{
  wxDateTime dt;

  // Czas referencyjny
  dt.Set(day, wxDateTime::Month(wxDateTime::Jan+(month-1)), year,
         real_hours, real_minutes, real_seconds, 0);
  // uwzgl�dniamy time offset
  dt.Add(wxTimeSpan((long int)hours, (long int)minutes, (long int)seconds, (long int)milliseconds));

  // zapisz nowy reference time
  SetReferenceDate(dt);
  SetReferenceTime(dt);

  // reset offset to zero
  hours = 0; minutes = 0; seconds = 0; milliseconds = 0;
}

BEGIN_EVENT_TABLE(wxNumValidator, wxValidator)
    EVT_CHAR(wxNumValidator::OnChar)
    EVT_TEXT(wxID_ANY, wxNumValidator::OnTextChange)
END_EVENT_TABLE()

wxNumValidator::wxNumValidator(int prec_in, E_NV_mode mode_in, long double *m_long_double_Value_in)
{
  prec = prec_in;
  mode = mode_in;
  m_long_double_Value = m_long_double_Value_in;
  max_val = LLONG_MAX; // LONG_LONG_MAX;
  min_val = -LLONG_MAX;
  if (mode == E_NV_non_negative)
    min_val = 0.0;
  if (mode == E_NV_only_positive)
    min_val = 1.0;


  wxString str;
  double val;

  str = _("1,1");
  if (str.ToDouble(&val) == true)
    dot_char = ',';
  else
    dot_char = '.';
}

wxNumValidator::wxNumValidator(int prec_in,
    long double min_in, long double max_in,
    long double *m_long_double_Value_in)
{
  prec = prec_in;
  min_val = min_in; max_val = max_in;
  mode = E_NV_any;
  if (min_val == 0.0)
    mode = E_NV_non_negative;
  if (min_val > 0.0)
    mode = E_NV_only_positive;
  m_long_double_Value = m_long_double_Value_in;


  wxString str;
  double val;

  str = _("1,1");
  if (str.ToDouble(&val) == true)
    dot_char = ',';
  else
    dot_char = '.';
}

wxNumValidator::wxNumValidator(const wxNumValidator& val)
    : wxValidator()
{
  Copy(val);
}

bool wxNumValidator::SetNumMin(long double new_min)
{
  min_val = new_min;

  if (min_val > max_val)
    return false;

  switch (mode)
  {
    case E_NV_non_negative:
      if (min_val < 0)
        return false;
      break;
    case E_NV_only_positive:
      if (min_val <= 0)
        return false;
      break;
    case E_NV_any:
    default:
      break;
  }
  return true;
}

bool wxNumValidator::SetNumMax(long double new_max)
{
  max_val = new_max;

  if (min_val > max_val)
    return false;

  switch (mode)
  {
    case E_NV_non_negative:
      if (max_val < 0)
        return false;
      break;
    case E_NV_only_positive:
      if (max_val <= 0)
        return false;
      break;
    case E_NV_any:
    default:
      break;
  }
  return true;
}

bool wxNumValidator::Copy(const wxNumValidator& val)
{
  wxValidator::Copy(val);

  prec = val.prec;
  mode = val.mode;
  min_val = val.min_val;
  max_val = val.max_val;

  m_long_double_Value = val.m_long_double_Value;

  dot_char = val.dot_char;

  return true;
}

// Called when the value in the window must be validated.
// This function can pop up an error message.
bool wxNumValidator::Validate(wxWindow *parent)
{
  long double long_double_val;
  double double_val;
  wxWindow *control;
  wxString val;

  if( !CheckValidator() )
    return false;

  control = (wxWindow *) m_validatorWindow;
  // If window is disabled, simply return
  if ( !control->IsEnabled() )
      return true;

  if ( m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)) )
  {
    wxTextCtrl *txt_control = (wxTextCtrl *) m_validatorWindow;

    val = txt_control->GetValue();
  }
  if ( m_validatorWindow->IsKindOf(CLASSINFO(wxComboBox)) )
  {
    wxComboBox *cmb_control = (wxComboBox *) m_validatorWindow;

    val = cmb_control->GetValue();
  }

  // NB: this format string should contain exactly one '%.*Lf'
  wxString errormsg;

  bool ok = true;
  if (prec > 0)
  {
    ok = val.ToDouble(&double_val);
    long_double_val = double_val;
  }
  else
  {
    long long val_longlong;
    unsigned long long val_ulonglong;

    if (mode == E_NV_any)
    {
      //ok = val.ToLongLong(&val_longlong);
      ok = ToLongLong(val, &val_longlong);
      long_double_val = val_longlong;
    }
    else
    {
      //ok = val.ToULongLong(&val_ulonglong);
      ok = ToULongLong(val, &val_ulonglong);
      long_double_val = val_ulonglong;
    }
  }

  if ( ok == false)
  {
    errormsg = _("%.*Lf is not a proper number");
  }
  else
  {
    switch (mode)
    {
      case E_NV_non_negative:
        if (long_double_val < 0)
        {
          //long_double_val = 0.0;
          ok = false;
          errormsg = _("%.*Lf is negative");
        }
        break;
      case E_NV_only_positive:
        if (long_double_val <= 0)
        {
          //long_double_val = 1.0;
          ok = false;
          errormsg = _("%.*Lf is negative or zero");
        }
        break;
      case E_NV_any:
      default:
        break;
    }
  }

  if (long_double_val < min_val)
  {
    //long_double_val = min_val;
    ok = false;
    errormsg = _("%.*Lf is less than allowed minimum");
  }
  else
  {
    if (long_double_val > max_val)
    {
      //long_double_val = max_val;
      ok = false;
      errormsg = _("%.*Lf is more than allowed maximum");
    }
  }

  if ( !ok )
  {
      wxASSERT_MSG( !errormsg.empty(), _T("you forgot to set errormsg") );

      m_validatorWindow->SetFocus();

      wxString buf;
      //buf.Printf(errormsg, val.c_str());
      buf.Printf(errormsg, prec, long_double_val);


      wxMessageBox(buf, _("Validation conflict"),
                   wxOK | wxICON_EXCLAMATION, parent);
  }

  return ok;
}

// Called to transfer data to the window
bool wxNumValidator::TransferToWindow(void)
{
    if( !CheckValidator() )
        return false;

    if ( m_long_double_Value != NULL )
    {
      wxString stringValue;

      if (prec > 0)
        stringValue.Printf("%.*f", prec, (double)(*m_long_double_Value));
      else
      {
        long int val;
        val = (long int)(*m_long_double_Value);
        if (prec < 0)
        {
          stringValue.Printf("%0*ld", -prec, val);
        }
        else
        {
          stringValue.Printf("%ld", val);
        }
      }
      //control->SetValue(stringValue);
      if ( m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)) )
      {
        wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow;
        control->ChangeValue(stringValue);
      }
      if ( m_validatorWindow->IsKindOf(CLASSINFO(wxComboBox)) )
      {
        wxComboBox *control = (wxComboBox *) m_validatorWindow;
        control->SetValue(stringValue);
      }
    }

    return true;
}

// Called to transfer data to the window
bool wxNumValidator::TransferFromWindow(void)
{
    if( !CheckValidator() )
        return false;

    if ( m_long_double_Value != NULL )
    {
      wxString stringValue;

      if ( m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)) )
      {
        wxTextCtrl *control = (wxTextCtrl *) m_validatorWindow;
        stringValue = control->GetValue();
      }
      if ( m_validatorWindow->IsKindOf(CLASSINFO(wxComboBox)) )
      {
        wxComboBox *control = (wxComboBox *) m_validatorWindow;
        stringValue = control->GetValue();
      }

      if (prec > 0)
      {
        double val;

        if (stringValue.ToDouble(&val) == false)
        {
          *m_long_double_Value = val;
        }
      }
      else
      {
        if (mode == E_NV_any)
        {
          long long val;

          ///if (stringValue.ToLongLong(&val) == true)
          if (ToLongLong(stringValue, &val) == true)
          {
            *m_long_double_Value = val;
          }
          else
          {
            if (mode == E_NV_only_positive)
              *m_long_double_Value = 1.0;
            else
              *m_long_double_Value = 0.0;
          }
        }
        else
        {
          unsigned long long val;

          //if (stringValue.ToULongLong(&val) == true)
          if (ToULongLong(stringValue, &val) == true)
          {
            *m_long_double_Value = val;
          }
          else
          {
            if (mode == E_NV_only_positive)
              *m_long_double_Value = 1.0;
            else
              *m_long_double_Value = 0.0;
          }
        }
      }

    }
    return true;
}

//! \todo implement EVT_TEXT to control - live text changes control
void wxNumValidator::OnTextChange( wxCommandEvent &event )
{
  int id;

  id = event.GetId();

  if (m_validatorWindow->GetId() == id)
  {
    // 1. check if string is correct
    // 2. store cursor position
    // 3. update text with ChangeValue
    //((wxTextCtrl *)m_validatorWindow)->ChangeValue(_("11"));
    // 4. restore cursor position
  }
  event.Skip();
}

void wxNumValidator::OnChar(wxKeyEvent& event)
{
  if ( m_validatorWindow )
  {
    bool discard_key;
    int keyCode = event.GetKeyCode();

    discard_key = false;
    // we don't filter special keys and Delete
    if (
        !(keyCode < WXK_SPACE || keyCode == WXK_DELETE || keyCode > WXK_START)
         && !wxIsdigit(keyCode))
         //&& keyCode != wxT('.') && keyCode != wxT(',') && keyCode != wxT('-') )
         //&& keyCode != wxT('+') && keyCode != wxT('e') && keyCode != wxT('E'))
    {
      discard_key = true;
      if (prec <= 0)
      {
        if ((keyCode == wxT('.')) || (keyCode == wxT(',')))
          discard_key = false;
      }
      switch (mode)
      {
        case E_NV_non_negative:
          break;
        case E_NV_only_positive:
          break;

        case E_NV_any:
          if (keyCode == wxT('-'))
            discard_key = false;
          break;

        default:
          break;
      }
    }
    if (discard_key == true)
    {
      if ( !wxValidator::IsSilent() )
         wxBell();

      // eat message
      return;
    }
  }

  event.Skip();
}

bool ToLongLong(wxString &str, long long *val_out)
{
  const char *temp;
  long long val;
  int sign;


  temp = str.c_str();

  sign = 1;
  if (*temp == '-')
  {
    sign = -1;
    temp++;
  }

  val = 0;
  while ((*temp >= '0') && (*temp <= '9'))
  {
    val *= 10;
    val += (*temp - '0');

    temp++;
  }
  if (*temp == 0)
  {
    *val_out = sign * val;
    return true;
  }
  return false;
}

bool ToULongLong(wxString &str, unsigned long long *val_out)
{
  const char *temp;
  unsigned long long val;

  temp = str.c_str();
  val = 0;
  while ((*temp >= '0') && (*temp <= '9'))
  {
    val *= 10;
    val += (*temp - '0');

    temp++;
  }
  if (*temp == 0)
  {
    *val_out = val;
    return true;
  }
  return false;
}

