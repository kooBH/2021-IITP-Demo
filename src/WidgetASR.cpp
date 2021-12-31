#include "WidgetASR.h"


WidgetASR::WidgetASR() {

    this->setLayout(&layout_ASR);

    layout_ASR.addWidget(&widget_ASR_1);
    layout_ASR.addWidget(&widget_ASR_2);
    layout_ASR.addWidget(&widget_ASR_3);
    layout_ASR.addWidget(&widget_ASR_4);

    widget_ASR_1.setAttribute(Qt::WA_NoSystemBackground);
    widget_ASR_2.setAttribute(Qt::WA_NoSystemBackground);
    widget_ASR_3.setAttribute(Qt::WA_NoSystemBackground);
    widget_ASR_4.setAttribute(Qt::WA_NoSystemBackground);

    widget_ASR_1.setAttribute(Qt::WA_TranslucentBackground);
    widget_ASR_2.setAttribute(Qt::WA_TranslucentBackground);
    widget_ASR_3.setAttribute(Qt::WA_TranslucentBackground);
    widget_ASR_4.setAttribute(Qt::WA_TranslucentBackground);

    // ASR widget init
    std::ifstream ifs("../private/config.json");
    json j = json::parse(ifs);
    widget_ASR_1.Init(j["key1"].get<string>(),"english");
    widget_ASR_2.Init(j["key2"].get<string>(),"english");
    widget_ASR_3.Init(j["key3"].get<string>(),"english");
    widget_ASR_4.Init(j["key4"].get<string>(),"english");

    pixmap_bkgnd.load("../res/background.png");
}

void WidgetASR::paintEvent(QPaintEvent* p) {
  QPixmap tmp = pixmap_bkgnd.scaled(this->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

  QPainter painter(this);
  painter.drawPixmap(0, 0, tmp);

  QWidget::paintEvent(p);


}
