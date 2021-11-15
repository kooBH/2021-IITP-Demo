#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QPushButton>

#include "KAnalysis.h"
#include "KJsonConfig.h"
#include "KWidgetASR.h"

#include "processor.h"

class app : public QWidget {
  Q_OBJECT

private:
  QVBoxLayout layout_main;

  QHBoxLayout layout_top;
  QLabel label_top;
  QPushButton btn_play;
  QPushButton btn_load;

  QTabWidget widget_main;
  KJsonConfig widget_config;
  KAnalysis widget_disp;
  QWidget widget_ASR;
  QVBoxLayout layout_ASR;
  KWidgetASR widget_ASR_1;
  KWidgetASR widget_ASR_2;
  KWidgetASR widget_ASR_3;

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


signals:
  void signal_process();
  void signal_load(QString fileName);


};