#include <QObject>

#include "STFT.h"
#include "WAV.h"
#include "RtInput.h"

// prototype demo
#include "CDR4proto.h"
#include "Label_tracking4proto.h"
#include "VADStateMachine4proto.h"
#include "MLDR4proto.h"

// PC algorithm demo
#include "CDR.h"
#include "Label_tracking.h"
#include "VADStateMachine.h"
#include "MLDR.h"
#include "Postfilter.h"
#include "OverIVA_Clique_RLS.h"

#include <vector>
#include <atomic>
#include <thread>

#include <time.h>

using std::vector;

class processor : public QObject{
	Q_OBJECT
	private:
		char file_name[2048];
		bool isRunning = false;


	//	app* parent;
	
		/* base modules */
		RtInput* rt_input=nullptr;
		STFT* stft_in=nullptr;
		STFT* stft_out=nullptr;
		WAV* input = nullptr;
		WAV* output=nullptr;
		vector<WAV*> vec_output;

		/* vars */
		double** raw = nullptr;
		double** data = nullptr;
		double** data2 = nullptr;
		double** p_out=nullptr;

		short* buf_in = nullptr;
		short* buf_out = nullptr;
		double*** buf_data = nullptr;
		double*** buf_mask = nullptr;
    short *buf_temp;


		/* CDR_MLDR */
		int len_buf = 200;
		int pad_vad = 30;

		CDR4proto* cdr4proto = nullptr;
		Label_tracking4proto* label_tracker4proto = nullptr;
		VADStateMachine4proto* VAD_machine4proto = nullptr;
		MLDR4proto* mldr4proto = nullptr;

		/* CDR_IVA_MLDR*/

		CDR* cdr = nullptr;
		int VAD_initial_noise = 10;
		double p_init = 0.0;
		double p_thr = 0.7;
		Label_tracking* label_tracker = nullptr;
		VADStateMachine* VAD_machine = nullptr;
		Postfilter* postfilter = nullptr;
		OverIVA_Clique* overiva = nullptr;
		MLDR* mldr = nullptr;


		std::thread* thread_process=nullptr;
		std::atomic<bool> atomic_thread; // is thread running ?

		void Algorithm();
		void CreateOutputs();


		void SetDateTime() {
			time_t rawtime;
			struct tm* timeinfo;
			time(&rawtime);
			timeinfo = localtime(&rawtime);
	   	strftime(file_name, sizeof(file_name), "%Y-%m-%d_%H-%M-%S.wav", timeinfo);
		}
		void SetDateTime(int idx) {
			time_t rawtime;
			struct tm* timeinfo;
			time(&rawtime);
			timeinfo = localtime(&rawtime);
	   	strftime(file_name, sizeof(file_name), "%Y-%m-%d_%H-%M-%S", timeinfo);
			sprintf(file_name, "%s_%d.wav", file_name, idx);
		}
		std::string cur_time_str(int idx) {
			char tmp_chars[1024];
		  time_t rawtime;
			struct tm* timeinfo;
			time(&rawtime);
			timeinfo = localtime(&rawtime);
	   	strftime(tmp_chars, sizeof(tmp_chars), "%Y-%m-%d_%H-%M-%S", timeinfo);
			sprintf(tmp_chars, "%s_%d.wav", tmp_chars, idx);
			return std::string(tmp_chars);
		}

	public :
		processor();
		~processor();


		std::atomic<bool> bool_init;
		/* param */
		int cur_algorithm=1;

		int ch_in=7;
		int ch_out=3;
		int num_out = 1;
		int samplerate=16000;
		int frame=1024;
		int shift=256;
		int ss = 343.3; // speed of sound

		int device=0;

		int cnt = 0;

		double scale_factor = 5; //FPGA board
		//double scale_factor = 10; //UMA-8


		/* CDR_IVA_MLDR */
		double CDR_smooth=0.4;
		int CDR_nsource=12;
    double CDR_mu=1.3;
    double CDR_Gmin=0.1;
    double CDR_alpha=1.0;
    double CDR_beta=0.5;
    double CDR_epsi=1e-10;
    double CDR_dist=0.043; // for UMA8 : 0.043, MEMS : 0.04

		bool do_postfilter = false;

		void init();
		void deinit();

		void Process();
		void Process(std::string);

		void Run();
		void Run(std::string);

		void CDR_MLDR(double ** data);
		void CDR_IVA_MLDR(double ** data);

		constexpr static int idx_CDR_MLDR = 1;
		constexpr static int idx_CDR_IVA_MLDR = 2;
		constexpr static int idx_CDR_IVA_MLDR_4ch = 3;

	void slot_toggle();

signals : 
	void signal_process_done(const char*);
	void signal_request_asr(const char*,int);
	
	
};
