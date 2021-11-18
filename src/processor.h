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
		int len_buf = 30;
		CDR4proto* cdr4proto = nullptr;
		Label_tracking4proto* label_tracker4proto = nullptr;
		VADStateMachine4proto* VAD_machine4proto = nullptr;
		MLDR4proto* mldr4proto = nullptr;

		/* CDR_IVA_MLDR*/

		CDR* cdr = nullptr;
		int VAD_initial_noise = 10;
		double p_init = 0.01;
		Label_tracking* label_tracker = nullptr;
		VADStateMachine* VAD_machine = nullptr;
		OverIVA_Clique* overiva = nullptr;
		MLDR* mldr = nullptr;


		std::thread* thread_process=nullptr;
		std::atomic<bool> atomic_thread; // is thread running ?

		void Algorithm();


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

	public :
		processor();
		~processor();


		std::atomic<bool> bool_init;
		/* param */
		int cur_algorithm=1;

		int ch_in=7;
		int ch_out=3;
		int samplerate=16000;
		int frame=1024;
		int shift=256;
		int ss = 343.3; // speed of sound

		int device=0;

		int cnt = 0;


		void init();
		void deinit();

		void Run();
		void Run(const char*);

		void Process();
		void Process(const char*);

		void CDR_MLDR(double ** data);
		void CDR_IVA_MLDR(double ** data);

		constexpr static int idx_CDR_MLDR = 1;
		constexpr static int idx_CDR_IVA_MLDR = 2;

	void slot_toggle();

signals : 
	void signal_process_done(const char*);
	void signal_request_asr(const char*,int);
	
	
};