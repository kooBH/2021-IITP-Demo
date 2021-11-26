#include <QWidget>
#include <QPainter>
#include "KWidgetASR.h"

#include <fstream>

#include "json.hpp"

using nlohmann::json;

class WidgetASR : public QWidget{
	Q_OBJECT

public :
  QVBoxLayout layout_ASR;
  KWidgetASR widget_ASR_1;
  KWidgetASR widget_ASR_2;
  KWidgetASR widget_ASR_3;
  KWidgetASR widget_ASR_4;

  WidgetASR();

private :
  QPixmap pixmap_bkgnd;

protected:
  void paintEvent(QPaintEvent* p);

	
};