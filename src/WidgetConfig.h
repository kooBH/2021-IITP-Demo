#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTextBrowser>

#include "KJsonConfig.h"

#include "RtBase.h"

using std::to_string;

class WidgetConfig : public QWidget{
  Q_OBJECT

private:
  QHBoxLayout layout_main;
  KJsonConfig widget_config;
  QVBoxLayout layout_audio;
  QPushButton btn_audioprobe;
  QTextBrowser TB_audio;

  map<string, unsigned int> map_device;
  QString text_device;

public:
  WidgetConfig();
  ~WidgetConfig();

  json operator[](string name) {
    return widget_config[name];
  }

  void Add(QString name, string path) {
    widget_config.Add(name, path);
  }

public slots:
  void slot_audioprobe();


};


