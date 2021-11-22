#include "app.h"

app::app(){
  setStyleSheet("\
			QWidget{background:rgb(130, 227, 179);}\
      \
      ");

  /* top widget */{
    layout_top.addWidget(&label_top);
    layout_top.addWidget(&btn_play);
    layout_top.addWidget(&btn_load);
    layout_top.addWidget(&CB_algo);
    label_top.setText("Top");
    btn_play.setText("Play");
    btn_load.setText("Load");

    QObject::connect(&btn_load, &QToolButton::clicked, [&]() {
      QString fileName;
      QFileDialog dialog;

      fileName = dialog.getOpenFileName(this,
        tr("Open Wav File"), ".", tr("something (*.wav)"));
      // Exception
      if (fileName.isEmpty())
        return;
      emit(signal_load(fileName));
      });
    layout_main.addLayout(&layout_top);
    layout_top.setAlignment(Qt::AlignLeft);

    QObject::connect(&btn_play, &QPushButton::pressed, this, &app::slot_btn_play);

    // ComboBox Algorithm
    CB_algo.addItem("None");
    CB_algo.addItem("CDR_MLDR");
    CB_algo.addItem("CDR_IVA_MLDR");
    CB_algo.addItem("CDR_IVA_MLDR_4ch");
    QObject::connect(&CB_algo, &QComboBox::currentIndexChanged, this, &app::slot_change_algo);
  }

  /* main widget */{
    widget_main.setStyleSheet("\
			QWidget{background:rgb(95, 94, 116);}\
      \
      ");

    widget_main.addTab(&widget_disp, "display");


    widget_main.addTab(&widget_config, "parameters");
    widget_config.setStyleSheet("\
			QWidget{background:rgb(149, 234, 150);}\
			QLineEdit{background:rgb(255, 255, 255);}\
			QComboBox{background:rgb(255, 255, 255);}\
      QCheckBox:indicator{background:rgb(210, 53, 50);border: 1px solid;}\
      QCheckBox:indicator:checked{background:rgb(79,214,130); border: 1px solid;}\
      \
      ");

    widget_config.Add("Input/Output", "../config/io.json");
    widget_config.Add("p1", "../config/param1.json");

    widget_ASR.setLayout(&layout_ASR);
    widget_ASR.setStyleSheet("\
			QWidget{background:rgb(231, 234, 139);}\
			QLabel{background:white;}\
      \
      ");

    layout_ASR.addWidget(&widget_ASR_1);
    layout_ASR.addWidget(&widget_ASR_2);
    layout_ASR.addWidget(&widget_ASR_3);
    layout_ASR.addWidget(&widget_ASR_4);

    widget_main.addTab(&widget_ASR, "ASR");

  layout_main.addWidget(&widget_main);
  }

  setLayout(&layout_main);

  //std::cout << "p1 param_1 : " <<get("p1","param_1") << std::endl;
  //std::cout << "p1 param_2 : " <<get("p1","param_2") << std::endl;
  //std::cout << "p1 param_3 : " <<get("p1","param_3") << std::endl;
  //std::cout << "p1 param_4 : " <<get("p1","param_4") << std::endl;

  /* Processor */
  QObject::connect(&btn_play, &QPushButton::pressed, &proc, &processor::slot_toggle);
  QObject::connect(this,&app::signal_load, this,&app::slot_load);
  QObject::connect(&proc,&processor::signal_request_asr, this,&app::slot_request_asr);

  // spectrogram
  QObject::connect(&proc, &processor::signal_process_done, &widget_disp, &KAnalysis::LoadFile);

  // ASR widget init
  std::ifstream ifs("../private/config.json");
  json j = json::parse(ifs);
  std::cout << j.dump();
  widget_ASR_1.Init(j["key1"].get<string>(),"korean");
  widget_ASR_2.Init(j["key2"].get<string>(),"korean");
  widget_ASR_3.Init(j["key3"].get<string>(),"korean");
  widget_ASR_4.Init(j["key4"].get<string>(),"korean");
};

app::~app() {

};

void app::slot_btn_play() {
  // Stop
  if (is_playing) {
    btn_play.setText("Play");
  }
  // Start
  else {
    setProcParam();
    btn_play.setText("Stop");
  
  }
  is_playing = !is_playing;
  std::cout << is_playing << std::endl;
}


 double app::get(QString a, string b) {
  double val;
   json j = widget_config[a.toStdString()][b]["value"];
   switch (j.type()) {
    //case json::value_t::object :
      //break;
    case json::value_t::boolean :
      return static_cast<double>(j.get<bool>());
    case json::value_t::number_float :
      return j.get<double>();
    case json::value_t::number_unsigned :
      return static_cast<double>(j.get<int>());
    case json::value_t::number_integer :
      return static_cast<double>(j.get<int>());
    default:
      printf("ERROR::unsupported json type:%d\n",j.type());
    }
 }

 void app::setProcParam() {
   proc.ch_in = static_cast<int>(get("Input/Output", "input_channels"));
   proc.device = static_cast<int>(get("Input/Output", "input_device"));

   proc.bool_init.store(true);
 }

 void app::slot_load(QString fileName){
   proc.init();
   proc.Run(fileName.toStdString());
 }

 void app::slot_request_asr(const char*path,int idx) {
   widget_main.setCurrentIndex(2);
   switch (idx) {
   case 0 :
     widget_ASR_1.Load(string(path));
     break;
   case 1:
     widget_ASR_2.Load(string(path));
     break;
   case 2:
     widget_ASR_3.Load(string(path));
     break;
   case 3:
     widget_ASR_4.Load(string(path));
     break;
   }
   update();
  
 }


 void app::slot_change_algo(int idx) {
   proc.cur_algorithm = idx;
 }
