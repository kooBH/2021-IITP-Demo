#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QPushButton>
#include <QComboBox>

#include "KAnalysis.h"
#include "WidgetConfig.h"
#include "WidgetASR.h"

#include "processor.h"

class app : public QWidget {
  Q_OBJECT

private:
  QVBoxLayout layout_main;

  QHBoxLayout layout_top;
  QPushButton btn_play;
  QPushButton btn_load;
  QComboBox CB_algo;

  QTabWidget widget_main;
  WidgetConfig widget_config;
  KAnalysis widget_disp;
  WidgetASR widget_ASR;

  bool is_playing = false;

  //template <typename T>
  //T get(QString, string);
  double get(QString, string);

  processor proc;
  void setProcParam();

public:

  app();
  ~app();


public slots:
  void slot_btn_play();
  void slot_load(QString);
  void slot_request_asr(const char*,int idx);
  void slot_change_algo(int);


signals:
  void signal_process();
  void signal_load(QString fileName);


};
