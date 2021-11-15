#include "processor.h"

#define _DEALLOC(x)\
if(x){              \
delete x;           \
x = nullptr;        \
}                   \

processor::processor() {

}

processor::~processor() {

}

void processor::init() {


  if (cur_algorithm == idx_CDR_MLDR) {
    ch_out = 3;
  }

  printf("\n -- processor::init -- \n");
  printf(" ch_in : %d\n", ch_in);
  printf(" ch_out : %d\n", ch_out);
  printf(" frame : %d\n", frame);
  printf(" shift : %d\n", shift);
  printf(" cur_algorithm : %d\n", cur_algorithm);

  rt_input = new RtInput(device,ch_in,samplerate,shift,frame);
  stft_in = new STFT(ch_in, frame, shift);
  stft_out = new STFT(ch_out, frame, shift);

  raw = new double*[ch_in];
  for (int i = 0; i < ch_in; i++) {
    raw[i] = new double[frame+2];
    memset(raw[i], 0, sizeof(double) * (frame+2));

  }
  data = new double*[ch_in];
  for (int i = 0; i < ch_in; i++) {
    data[i] = new double[frame+2];
    memset(data[i], 0, sizeof(double)*(frame+2));
  }
  buf_in = new short[ch_in * shift];
  buf_out = new short[ch_out * shift];
  cnt = 0;

  /* init for algorithm */
  if (cur_algorithm == idx_CDR_MLDR) {
    cdr4proto = new CDR4proto(
      frame,
      shift
      ,ch_in,
      samplerate,
      ss,
      0.4
    );
    label_tracker4proto = new Label_tracking4proto(
      cdr4proto->nsource,
      ch_out,
      frame / 2 + 1
    );
    VAD_machine4proto = new VADStateMachine4proto(
      ch_out,
      2, 
      20,
      30, 
      0.5,
      0.2, 
      0.5);
    mldr4proto = new MLDR4proto(
      frame,
      shift,
      ch_in,
      ch_out);
      
    len_buf = label_tracker4proto->UPframe + VAD_machine4proto->UPframe;
    buf_data = new double** [len_buf];
    for (int i = 0; i < len_buf; i++){
      buf_data[i] = new double* [ch_in];
      for (int j = 0; j < ch_in; j++){
        buf_data[i][j] = new double[frame + 2];
        memset(buf_data[i][j], 0.0, sizeof(double) * (frame + 2));
      }

    }
    buf_mask= new double** [len_buf];
    for (int i = 0; i < len_buf; i++)
    {
      buf_mask[i] = new double* [frame / 2 + 1];
      for (int j = 0; j < frame / 2 + 1; j++)
      {
        buf_mask[i][j] = new double[cdr4proto->nsource];
        memset(buf_mask[i][j], 0.01, sizeof(double) * (cdr4proto->nsource));
      }
    }

    printf(" -- CDR_MLDR --\n");
    printf(" len_buf : %d\n", len_buf);
  }
  else if (cur_algorithm == idx_CDR_IVA_MLDR) {
    
  }
  else{
    
  }

}

void processor::deinit() {
  _DEALLOC(cdr4proto);
  _DEALLOC(label_tracker4proto);
  _DEALLOC(VAD_machine4proto);
  _DEALLOC(rt_input);
  _DEALLOC(output);

}

void processor::Process() {
  rt_input->Start();

  SetDateTime();
  output->NewFile(file_name);

  while (rt_input->IsRunning()) {
    if (rt_input->data.stock.load() > shift) {
      rt_input->Convert2ShiftedArray(raw);
      stft_in->stft(raw, data);

      if (cur_algorithm == idx_CDR_MLDR) {
        CDR_MLDR(data, p_out);
        stft_out->istft(buf_data[0], buf_out);
        output->Append(buf_out, shift);
      }
    }
    else {
      SLEEP(10);
    }
  }

  output->Finish();
  emit(signal_process_done(file_name));
  deinit();
}

void processor::Process(const char* path_input) {

  input = new WAV();
  input->OpenFile(path_input);

  printf("\n -- procssor::Process(%s) --\n",path_input);
  input->Print();

  WAV** outputs = new WAV*[3];
  outputs[0] = new WAV(1, samplerate);
  outputs[1] = new WAV(1, samplerate);
  outputs[2] = new WAV(1, samplerate);

  SetDateTime(1);
  outputs[0]->NewFile(file_name);
  SetDateTime(2);
  outputs[1]->NewFile(file_name);
  SetDateTime(3);
  outputs[2]->NewFile(file_name);

  short *buf_temp;
  buf_temp = new short[shift];

  if (buf_out);

  int cnt = 0;
  while (!input->IsEOF()) {
     // input->Convert2ShiftedArray(raw);
      int length = input->ReadUnit(buf_in, shift * ch_in);
      stft_in->stft(buf_in, length, data);

      if (cur_algorithm == idx_CDR_MLDR) {
        CDR_MLDR(data, p_out);
        stft_out->istft(buf_data[0], buf_out);
          for (int idx_ch = 0; idx_ch < ch_out; idx_ch++) {
            // Append if speech is active
            if (VAD_machine4proto->write_on[idx_ch]) {
              memset(buf_temp, 0, sizeof(short) * shift);
              for (int j = 0; j < shift; j++)
                buf_temp[j] = buf_out[j * ch_out + idx_ch];
              outputs[idx_ch]->Append(buf_temp, shift);
            }
          }
       // printf("cnt : %d\n", cnt++);
      }
  }
  input->Finish();
  deinit();

  for (int i = 0; i < ch_out; i++) {
    emit(signal_request_asr(outputs[i]->GetFileName(),i));
    emit(signal_process_done(outputs[i]->GetFileName()));
  }
  delete input;
  delete buf_temp;
  delete outputs[0];
  delete outputs[1];
  delete outputs[2];
}


void processor::CDR_MLDR(double** data,double**out) {
  cnt++;

  /* shift  of VAD buffer*/
  for (int k = 1; k < len_buf; k++){
    for (int i = 0; i < ch_in; i++){
      for (int j = 0; j < frame + 2; j++){
        buf_data[k - 1][i][j] = buf_data[k][i][j];
      }
    }
  }
  for (int i = 0; i < ch_in; i++){
    for (int j = 0; j < frame + 2; j++){
      buf_data[len_buf- 1][i][j] = data[i][j];
    }
  }

  //#pragma omp parallel for schedule(static,32)
  cdr4proto->Process(data);

  for (int k = 1; k < len_buf; k++)
  {
    for (int i = 0; i < frame / 2 + 1; i++)
    {
      for (int j = 0; j < cdr4proto->nsource; j++)
      {
        buf_mask[k - 1][i][j] = buf_mask[k][i][j];
      }
    }
  }
  for (int i = 0; i < frame / 2 + 1; i++)
  {
    for (int j = 0; j < cdr4proto->nsource; j++)
    {
      buf_mask[len_buf - 1][i][j] = cdr4proto->mask[i][j];
    }
  }

  label_tracker4proto->Process(cdr4proto->mask);
  VAD_machine4proto->Process(label_tracker4proto->L, label_tracker4proto->ind2label, label_tracker4proto->upcount, label_tracker4proto->downcount, mldr4proto->alpha_null_pre, mldr4proto->alpha_null);
  mldr4proto->Process(buf_data[0], buf_mask[0], label_tracker4proto->ind2label);
}

void processor::slot_toggle() { 

  if (isRunning) {
    rt_input->Stop();
    bool_init.store(false);
  }
  else {
    while (!bool_init.load())SLEEP(5);
    init();
    Process();
  }
}
