//#define _CRTSECURE_NO_WARNINGS
//#define _CRT_SECURE_NO_DEPRCATE
#define _CRTDBG_MAP_ALLOC

#include <crtdbg.h>
#include <stdlib.h> //for chrtdbg.h https://docs.microsoft.com/ko-kr/visualstudio/debugger/finding-memory-leaks-using-the-crt-library?view=vs-2019
#include "STFT.h"
#include "WAV.h"
#include <string>
#include <filesystem>
/* Inlcude Algorithm source here */
#include "CDR.h"
#include "Label_tracking.h"
#include "VADStateMachine.h"
#include "MLDR.h"
#include "OverIVA_Clique_RLS.h"

/* Set Parameter of Input */
constexpr int ch = 7;
constexpr int out_ch = 3;
constexpr int rate = 16000;
constexpr int frame = 1024;
constexpr int shift = 256;
constexpr double ss = 343.3;


int main() 
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
   //_crtBreakAlloc = 39702;

  /* Define Algortihm Class here */
  int VAD_initial_noise = 10;
  double p_init = 0.01;

  CDR CDR(frame, shift, ch, rate, ss);
  Label_tracking Label_tracking(CDR.nsource, out_ch, frame/2+1);
  VADStateMachine VADStateMachine(out_ch);
  OverIVA_Clique overiva(frame, ch, out_ch, rate);
  MLDR MLDR(frame, shift, ch, out_ch);

  int length;
  WAV input;
  WAV output(out_ch, rate);
  STFT stft_in(ch, frame, shift);
  STFT stft_out(out_ch, frame, shift);

  //short buf_in[ch * shift];
  short* buf_in;
  double** data;
  short* buf_out;

  data = new double* [ch];
  for (int i = 0; i < ch; i++) {
    data[i] = new double[frame + 2];
    memset(data[i], 0, sizeof(double) * (frame + 2));
  }
  buf_in = new short[ch * shift];
  memset(buf_in, 0, sizeof(short) * ch * shift);
  buf_out = new short[out_ch * shift];

  //input.OpenFile("../../wav/s01_max.wav");
  input.OpenFile("N:/user/albert/2021_IITP_share/2021-08-27_È¸ÀÇ_real_data_set_±¸Ãà/Seoul_mal/trial01/circular_7ch/data/s12.wav");
  output.NewFile("../../wav/output_Cxx.wav");

  // Buffer
  double*** VAD_buffer;
  double*** Mask_buffer;  
  int bufferlen = Label_tracking.UPframe + VADStateMachine.UPframe + VAD_initial_noise;

  VAD_buffer = new double** [bufferlen];
  for (int i = 0; i < bufferlen; i++)
  {
      VAD_buffer[i] = new double* [ch];
      for (int j = 0; j < ch; j++)
      {
          VAD_buffer[i][j] = new double [frame + 2];
          memset(VAD_buffer[i][j], 0.0, sizeof(double) * (frame + 2));
      }      
  }
  Mask_buffer = new double** [bufferlen];
  for (int i = 0; i < bufferlen; i++)
  {
      Mask_buffer[i] = new double* [frame / 2 + 1];
      for (int j = 0; j < frame / 2 + 1; j++)
      {
          Mask_buffer[i][j] = new double[CDR.nsource];
          for (int k = 0; k < CDR.nsource; k++)
              Mask_buffer[i][j][k] = p_init;
      }
  }

  // For multiple input files?
  /*
  for (auto path : std::filesystem::directory_iterator{"../input"}) {
    /*std::string i(path.path().string());
    printf("processing : %s\n",i.c_str());
    input.OpenFile(i.c_str());
    i = "../output/"+ i.substr(9, i.length() - 9);
    output.NewFile((i).c_str());
    }*/

  int cnt = 0;
  while (!input.IsEOF()) {
      cnt++;      
      //printf("================ frame %d =================== \n", cnt);
      length = input.ReadUnit(buf_in, shift * ch);
      stft_in.stft(buf_in, length, data);

      // VAD buffer
      for (int k = 1; k < bufferlen; k++)
      {
          for (int i = 0; i < ch; i++)
          {
              for (int j = 0; j < frame + 2; j++)
              {
                  VAD_buffer[k - 1][i][j] = VAD_buffer[k][i][j];
              }
          }
      }
      for (int i = 0; i < ch; i++)
      {
          for (int j = 0; j < frame + 2; j++)
          {
              VAD_buffer[bufferlen - 1][i][j] = data[i][j];
          }
      }
      
      if (cnt < bufferlen)
      {
          stft_out.istft(VAD_buffer[0], buf_out);
          output.Append(buf_out, shift * out_ch);
          continue;
      }
          
      
      CDR.Process(data);

      for (int k = 1; k < bufferlen; k++)
      {
          for (int i = 0; i < frame / 2 + 1; i++)
          {
              for (int j = 0; j < CDR.nsource; j++)
              {
                  Mask_buffer[k - 1][i][j] = Mask_buffer[k][i][j];
              }
          }
      }
      for (int i = 0; i < frame / 2 + 1; i++)
      {
          for (int j = 0; j < CDR.nsource; j++)
          {
              if (CDR.mask[i][j] > p_init)
              {
                  Mask_buffer[bufferlen - 1][i][j] = CDR.mask[i][j];
              }
              else
              {
                  Mask_buffer[bufferlen - 1][i][j] = p_init;
              }
                
          }
      }
      
      Label_tracking.Process(CDR.mask);      
      VADStateMachine.Process(Label_tracking.L, Label_tracking.ind2label, Label_tracking.upcount, Label_tracking.downcount, MLDR.alpha_null_pre, MLDR.alpha_null, cnt);
      overiva.Process(VAD_buffer[0], MLDR.W, Mask_buffer[0], VADStateMachine.ind2vad);
      MLDR.Process(VAD_buffer[0], Mask_buffer[0], VADStateMachine.ind2vad, overiva.A, overiva.W);

      
      stft_out.istft(VAD_buffer[0], buf_out);
      output.Append(buf_out, shift * out_ch);
  }
  output.Finish();
  input.Finish();

  for (int i = 0; i < ch; i++)
      delete[] data[i];
  delete[] data;
  delete[] buf_in;
  delete[] buf_out;

  for (int i = 0; i < bufferlen; i++)
  {
      for (int j = 0; j < ch; j++)
          delete[] VAD_buffer[i][j];
      delete[] VAD_buffer[i];
  }
  delete[] VAD_buffer;

  for (int i = 0; i < bufferlen; i++)
  {      
      for (int j = 0; j < frame / 2 + 1; j++)
          delete[] Mask_buffer[i][j];
      delete[] Mask_buffer[i];
  }
  delete[] Mask_buffer;

  return 0;
}
