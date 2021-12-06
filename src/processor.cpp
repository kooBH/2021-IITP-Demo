#include "processor.h"

#define _DEALLOC0(x)\
if(x){              \
delete x;           \
x = nullptr;        \
}                   \

#define _DEALLOC1(x)\
if(x){              \
delete[] x;           \
x = nullptr;        \
}

#define _DEALLOC2(x,a) if(x){for(int _i=0;_i<(a);_i++){delete[] x[_i];}; delete[] x; x = nullptr; }
#define _DEALLOC3(x,a,b) if(x){for(int _i=0;_i<(a);_i++){for (int _j = 0; _j < (b); _j++) {delete[] x[_i][_j];}delete[] x[_i];}; delete[] x;x = nullptr;}



processor::processor() {
  atomic_thread.store(false);
 // parent = app_;

}

processor::~processor() {

}

void processor::init() {


  if (cur_algorithm == idx_CDR_MLDR) {
    ch_out = 3;
  }
  else if (cur_algorithm == idx_CDR_IVA_MLDR) {
    ch_out = 3;
  }
  else if (cur_algorithm == idx_CDR_IVA_MLDR_4ch) {
    ch_out = 4;
  }
  else {
    ch_out = 1;
  }

  printf("\n -- processor::init -- \n");
  printf(" ch_in : %d\n", ch_in);
  printf(" ch_out : %d\n", ch_out);
  printf(" frame : %d\n", frame);
  printf(" shift : %d\n", shift);
  printf(" cur_algorithm : %d\n", cur_algorithm);

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
  buf_temp = new short[shift];
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
      
    len_buf = label_tracker4proto->UPframe + VAD_machine4proto->UPframe+pad_vad;
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
        for (int k = 0; k < cdr4proto->nsource; k++)
            buf_mask[i][j][k] = p_init;
      }
    }
  }
  /*   PC Algorithm : CDR IVA MLDR  */
  else if (cur_algorithm == idx_CDR_IVA_MLDR || cur_algorithm == idx_CDR_IVA_MLDR_4ch) {
    cdr = new CDR(
      frame,
      shift,
      ch_in,
      samplerate,
      ss,
      CDR_smooth,
      CDR_nsource,
      CDR_mu,
      CDR_Gmin,
      CDR_alpha,
      CDR_beta,
      CDR_epsi,
      CDR_dist
    );
    label_tracker = new Label_tracking(cdr->nsource,
      ch_out,
      frame / 2 + 1
    );
    VAD_machine = new VADStateMachine(ch_out, 
      frame,
      ch_in
    );
    overiva = new OverIVA_Clique(frame, 
      ch_in, 
      ch_out,
      samplerate
    );
    postfilter = new Postfilter(frame, ch_out);
    mldr = new MLDR(frame,
      shift,
      ch_in,
      ch_out
    );

    len_buf = label_tracker->UPframe + VAD_machine->UPframe+pad_vad;
    buf_data = new double** [len_buf];
    for (int i = 0; i < len_buf; i++) {
      buf_data[i] = new double* [ch_in];
      for (int j = 0; j < ch_in; j++) {
        buf_data[i][j] = new double[frame + 2];
        memset(buf_data[i][j], 0.0, sizeof(double) * (frame + 2));
      }

    }
    buf_mask = new double** [len_buf];
    for (int i = 0; i < len_buf; i++)
    {
      buf_mask[i] = new double* [frame / 2 + 1];
      for (int j = 0; j < frame / 2 + 1; j++)
      {
        buf_mask[i][j] = new double[cdr->nsource];        
        for (int k = 0; k < cdr->nsource; k++)
            buf_mask[i][j][k] = p_init;
      }
    }
  }
  else{
    
  }

}

void processor::deinit() {

  _DEALLOC0(cdr4proto);
  _DEALLOC0(label_tracker4proto);
  _DEALLOC0(VAD_machine4proto);
  _DEALLOC0(mldr4proto);

  _DEALLOC0(cdr);
  _DEALLOC0(label_tracker);
  _DEALLOC0(VAD_machine);
  _DEALLOC0(overiva);
  _DEALLOC0(postfilter);
  _DEALLOC0(mldr);

  _DEALLOC1(buf_in);
  _DEALLOC1(buf_out);

  _DEALLOC2(raw,ch_in);
  _DEALLOC2(data,ch_in);

  _DEALLOC3(buf_mask,len_buf,frame/2+1);
  _DEALLOC3(buf_data,len_buf,ch_in);


  _DEALLOC0(rt_input);
  _DEALLOC0(stft_in);
  _DEALLOC0(stft_out);
  bool_init.store(false);
}

void processor::slot_toggle() { 

  if (atomic_thread.load()) {
    printf("STOP\n");
    rt_input->Stop();
  }
  else {
    while (!bool_init.load())SLEEP(5);
    printf("START\n");
    init();
    Run();
  }
}


void processor::CreateOutputs() {
  if (cur_algorithm == idx_CDR_MLDR) {
    num_out = cdr4proto->nsource;
  }
  else if (cur_algorithm == idx_CDR_IVA_MLDR || cur_algorithm == idx_CDR_IVA_MLDR_4ch) {
    num_out = cdr->nsource;
  }
  else {
    num_out = ch_out;
  }
  
  for (int i = 0; i < num_out; i++) {
    vec_output.push_back(new WAV(1, samplerate));

    if (cur_algorithm == idx_CDR_MLDR) {
      std::string tmp_str = cur_time_str(i + 1);
      vec_output[i]->NewFile(tmp_str.c_str());
    
    }
    //printf("NewFile : %s\n",tmp_str.c_str());
  }

}

void processor::Process() {
  rt_input = new RtInput(device,ch_in,samplerate,shift,frame);
  atomic_thread.store(true);

  CreateOutputs();


  output = new WAV(ch_in, samplerate);
 output->NewFile("input.wav");
  rt_input->Start();
  
  while (rt_input->IsRunning()) {
    if (rt_input->data.stock.load() > shift) {
     // printf("cnt : %d\n",cnt++);
      rt_input->GetBuffer(buf_in);

      /* Scaling : input is two small */
      for (int i = 0; i < shift * ch_in; i++) {
        if (buf_in[i] > 0) {
          buf_in[i] = std::min(int(buf_in[i] * scale_factor), 32767);
          if (buf_in[i] == 32767)
            printf("WARNNING::clipping 32767\n");
        }
        else {
          buf_in[i] = std::max(int(buf_in[i] * scale_factor), -32767);
          if (buf_in[i] == -32767)
            printf("WARNNING::clipping -32767\n");
        }
      }
      output->Append(buf_in, shift*ch_in);
      stft_in->stft(buf_in, shift * ch_in,data);
      Algorithm();

    }
    else {
      SLEEP(10);
    }
  }
 
  if (cur_algorithm == idx_CDR_MLDR) {
    int asr_cnt = 0;
    for (int i = 0; i < num_out; i++)
      vec_output[i]->Finish();
    for (int i = 0; i < num_out && asr_cnt < ch_out; i++) {
      if (vec_output[i]->GetSize() < 1024)
        continue;
      vec_output[i]->Normalize();
      vec_output[i]->Finish();
      emit(signal_request_asr(vec_output[i]->GetFileName(), asr_cnt++));
      //parent->slot_request_asr(vec_output[i]->GetFileName(),asr_cnt++);
      emit(signal_process_done(vec_output[i]->GetFileName()));
    }
  }

  output->Finish();
  delete output;
  delete[] buf_temp;
  vec_output.clear();
  deinit();
  atomic_thread.store(false);
}

void processor::Process(std::string path_input) {
  atomic_thread.store(true);

  input = new WAV();
  input->OpenFile(path_input.c_str());

  printf("\n -- procssor::Process(%s) --\n",path_input.c_str());
  input->Print();

  CreateOutputs();

  int cnt = 0;
  while (!input->IsEOF()) {
     // input->Convert2ShiftedArray(raw);
      int length = input->ReadUnit(buf_in, shift * ch_in);
      stft_in->stft(buf_in, length, data);
      Algorithm();
  }
  input->Finish();
  deinit();

  if (cur_algorithm == idx_CDR_MLDR) {
    int asr_cnt = 0;
    for (int i = 0; i < num_out; i++)
      vec_output[i]->Finish();
    for (int i = 0; i < num_out && asr_cnt < ch_out; i++) {
      if (vec_output[i]->GetSize() < 1024)
        continue;
      vec_output[i]->Normalize();
      vec_output[i]->Finish();
      emit(signal_request_asr(vec_output[i]->GetFileName(), asr_cnt++));
      //parent->slot_request_asr(vec_output[i]->GetFileName(),asr_cnt++);
      emit(signal_process_done(vec_output[i]->GetFileName()));
    }
  }
  delete input;
  delete[] buf_temp;
  vec_output.clear();
  atomic_thread.store(false);
}

void processor::Algorithm(){
 switch(cur_algorithm){
        case idx_CDR_MLDR:
          CDR_MLDR(data);
          stft_out->istft(buf_data[0], buf_out);
          for (int idx_ch = 0; idx_ch < ch_out; idx_ch++) {
            // Append if speech is active
            if (VAD_machine4proto->vad_on[idx_ch]) {
              memset(buf_temp, 0, sizeof(short) * shift);
              for (int j = 0; j < shift; j++)
                buf_temp[j] = buf_out[j * ch_out + idx_ch];
              vec_output[label_tracker4proto->ind2label[idx_ch]]->Append(buf_temp, shift);
            }
          }
          break;
        case idx_CDR_IVA_MLDR :
          CDR_IVA_MLDR(data);
          stft_out->istft(buf_data[0], buf_out);
          for (int idx_ch = 0; idx_ch < ch_out; idx_ch++) {
            int cur_azimuth = VAD_machine->ind2vad_1[idx_ch];
            //Detect VAD on 
            if (VAD_machine->vad_indicator[idx_ch] == 1) {
              if (vec_output[cur_azimuth]->GetIsOpen())
                vec_output[cur_azimuth]->Finish();
              std::string tmp_str = cur_time_str(cur_azimuth + 1);
              vec_output[cur_azimuth]->NewFile("outputs/"+tmp_str);

              VAD_machine->vad_indicator[idx_ch] = 0;
            }
            // Append if speech is active
            if (VAD_machine->vad_on[idx_ch] && vec_output[cur_azimuth]->GetIsOpen()) {
              memset(buf_temp, 0, sizeof(short) * shift);
              for (int j = 0; j < shift; j++)
                buf_temp[j] = buf_out[j * ch_out + idx_ch];
              vec_output[cur_azimuth]->Append(buf_temp, shift);
            }
            if (VAD_machine->vad_indicator[idx_ch] < 0 ) {
              cur_azimuth = -VAD_machine->vad_indicator[idx_ch]-1;
              vec_output[cur_azimuth]->Normalize();
              vec_output[cur_azimuth]->Finish();
              emit(signal_request_asr(vec_output[cur_azimuth]->GetFileName(), (int)(cur_azimuth/3)));
              //emit(signal_request_asr(vec_output[cur_azimuth]->GetFileName(),cur_azimuth%4 ));
              emit(signal_process_done(vec_output[cur_azimuth]->GetFileName()));
              VAD_machine->vad_indicator[idx_ch] = 0;
            }
          }
          break;
        case idx_CDR_IVA_MLDR_4ch :
          CDR_IVA_MLDR(data);
          stft_out->istft(buf_data[0], buf_out);
          for (int idx_ch = 0; idx_ch < ch_out; idx_ch++) {
            int cur_azimuth = VAD_machine->ind2vad_1[idx_ch];
            //Detect VAD on 
            if (VAD_machine->vad_indicator[idx_ch] == 1) {
              if (vec_output[cur_azimuth]->GetIsOpen())
                vec_output[cur_azimuth]->Finish();
              std::string tmp_str = cur_time_str(cur_azimuth + 1);
              vec_output[cur_azimuth]->NewFile("outputs/"+tmp_str);

              VAD_machine->vad_indicator[idx_ch] = 0;
            }
            // Append if speech is active
            else if (VAD_machine->vad_on[idx_ch] && vec_output[cur_azimuth]->GetIsOpen()) {
              memset(buf_temp, 0, sizeof(short) * shift);
              for (int j = 0; j < shift; j++)
                buf_temp[j] = buf_out[j * ch_out + idx_ch];
              vec_output[cur_azimuth]->Append(buf_temp, shift);
            }
            else if (VAD_machine->vad_indicator[idx_ch] < 0 ) {
              cur_azimuth = -VAD_machine->vad_indicator[idx_ch]-1;
              vec_output[cur_azimuth]->Normalize();
              vec_output[cur_azimuth]->Finish();
              emit(signal_request_asr(vec_output[cur_azimuth]->GetFileName(), (int)(cur_azimuth/3)));
              //emit(signal_request_asr(vec_output[cur_azimuth]->GetFileName(),cur_azimuth%4 ));
              emit(signal_process_done(vec_output[cur_azimuth]->GetFileName()));
              VAD_machine->vad_indicator[idx_ch] = 0;
            }


          }
          break;

        default:
          ;
      }
}


void processor::CDR_MLDR(double** data) {

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

  for (int k = 1; k < len_buf; k++){
    for (int i = 0; i < frame / 2 + 1; i++){
      for (int j = 0; j < cdr4proto->nsource; j++){
        buf_mask[k - 1][i][j] = buf_mask[k][i][j];
      }
    }
  }
  for (int i = 0; i < frame / 2 + 1; i++){
    for (int j = 0; j < cdr4proto->nsource; j++){
      buf_mask[len_buf - 1][i][j] = cdr4proto->mask[i][j];
    }
  }
  if (cnt++ < len_buf)
    return;

  label_tracker4proto->Process(cdr4proto->mask);
  VAD_machine4proto->Process(label_tracker4proto->L, label_tracker4proto->ind2label, label_tracker4proto->upcount, label_tracker4proto->downcount, mldr4proto->alpha_null_pre, mldr4proto->alpha_null);
  mldr4proto->Process(buf_data[0], buf_mask[0], label_tracker4proto->ind2label);
}


void processor::CDR_IVA_MLDR(double** data) {
  // VAD buffer
  for (int k = 1; k < len_buf; k++){
    for (int i = 0; i < ch_in; i++){
      for (int j = 0; j < frame + 2; j++){
        buf_data[k - 1][i][j] = buf_data[k][i][j];
      }
    }
  }
  for (int i = 0; i < ch_in; i++){
    for (int j = 0; j < frame + 2; j++){
      buf_data[len_buf - 1][i][j] = data[i][j];
    }
  }

  cdr->Process(data);

  for (int k = 1; k < len_buf; k++){
    for (int i = 0; i < frame / 2 + 1; i++){
      for (int j = 0; j < cdr->nsource; j++){
        buf_mask[k - 1][i][j] = buf_mask[k][i][j];
      }
    }
  }
  for (int i = 0; i < frame / 2 + 1; i++){
    for (int j = 0; j < cdr->nsource; j++){
      if (cdr->mask[i][j] > p_thr){
        buf_mask[len_buf - 1][i][j] = cdr->mask[i][j];
      }
      else{
        buf_mask[len_buf - 1][i][j] = p_init;
      }
    }
  }

  if (cnt++ < len_buf)
    return;

  label_tracker->Process(cdr->mask);
  VAD_machine->Process(cdr->st_angle, label_tracker->LRT_val, label_tracker->ind2label, label_tracker->upcount, label_tracker->downcount, mldr->alpha_null_pre, mldr->alpha_null, cnt);
  overiva->Process(buf_data[0], mldr->W, buf_mask[0], VAD_machine->ind2vad_1);
  mldr->Process(buf_data[0], buf_mask[0], VAD_machine->ind2vad_1, VAD_machine->active_st, VAD_machine->State, overiva->A, overiva->W);
  if(do_postfilter)
    postfilter->Process(buf_data[0], buf_mask[0], VAD_machine->ind2vad_1);
}



void processor::Run(){
  
  if (atomic_thread.load()) {
    printf("Warnning::Process thread is still running");
    return;
  }
  else {
    if (thread_process) {
      delete thread_process;
      thread_process = nullptr;
    }
  }

  thread_process = new std::thread([=] {this->Process(); });
  thread_process->detach();

}

void processor::Run(std::string path) {
  if (atomic_thread.load()) {
    printf("Warnning::Process thread is still running");
    return;
  }
  else {
    if (thread_process) {
      delete thread_process;
      thread_process = nullptr;
    }
  }
  thread_process = new std::thread([=] {this->Process(path); });
  thread_process->detach();


}


