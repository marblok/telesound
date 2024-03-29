/*! Digital Signal Processing Engine sockets usage example.
 *  Simple client application (2).
 * \author Marek Blok
 * \date 2010.02.09
 * \date updated 2021.01.18
 */
#include <DSP_sockets.h>
#include <DSP_lib.h>

int main(void)
{
  DSP::Clock_ptr MasterClock;
  int temp;
  unsigned long Fp1, Fp2, M, L;
  DSP::LoadCoef coef_info;
  int N_SRC;  DSP::Float_vector h_SRC;

  DSP::log.SetLogState(DSP::e::LogState::console | DSP::e::LogState::file);
  DSP::log.SetLogFileName("log_file_client.log");

  DSP::log << DSP::lib_version_string() << std::endl << std::endl;

  coef_info.Open("LPF_8000_11025.coef", "matlab");
  N_SRC = coef_info.GetSize(0);
  if (N_SRC < 1)
  {
    DSP::log << "Filter coefficients files should be generated using >>socket_filters.m<<" << std::endl;
    return 1;
  }
  else
  {
    coef_info.Load(h_SRC);
  }

  MasterClock=DSP::Clock::CreateMasterClock();

  // use client socket
//  DSP::u::SocketInput in_socket(MasterClock, "127.0.0.1", true, 0x00000003);
  std::string server_address = "127.0.0.1:10000";
  DSP::u::SocketInput in_socket(MasterClock, server_address, true, 0x00000003);
  in_socket.SetName(server_address);
  Fp1 = 8000;  Fp2 = 11025;
  M = 320; L = 441;
  DSP::u::SamplingRateConversion SRC(false,MasterClock, L, M, h_SRC);
  DSP::u::AudioOutput AudioOut(Fp2);

  in_socket.Output("out") >> SRC.Input("in");
  SRC.Output("out") >> AudioOut.Input("in");

  DSP::u::FileOutput WAVEfile("morse.wav", DSP::e::SampleType::ST_short, 1, DSP::e::FileType::FT_wav, Fp2);
  SRC.Output("out") >> WAVEfile.Input("in");

  DSP::Component::CheckInputsOfAllComponents();
  DSP::Clock::SchemeToDOTfile(MasterClock, "socket_client_2.dot");

  // in_socket.WaitForConnection(); <== client not server
  temp=1;
  do
  {
    DSP::Clock::Execute(MasterClock, Fp1/4);

    DSP::log << "MAIN" << DSP::e::LogMode::second << temp 
             << " (" << in_socket.GetBytesRead() << ")" << std::endl;
    temp++;
  }
  while (in_socket.GetBytesRead() != 0);

  DSP::Clock::FreeClocks();
  return 0;
}
