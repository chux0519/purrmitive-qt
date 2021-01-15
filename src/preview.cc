#include "preview.h"

#include <QPainter>
#include <QSvgRenderer>

Preview::Preview(QWidget *parent) {}

void Preview::setBg(const PurrmitiveColor &color,
                    const PurrmitiveContextInfo &info) {
  _bg = color;
  _info = info;
  _initImage();
  QColor c = QColor(color.r, color.g, color.b, color.a);
  _img.fill(c);
  renderCurrentState();
}

void Preview::setSize(QSize &size) { _size = size; }

void Preview::step(const QString &svg, const PurrmitiveContextInfo &info) {
  _info = info;
  _shapes.push_back(svg);

  renderImg();
  renderCurrentState();
}

void Preview::renderCurrentState() {
  if (_size.width() != 0 && _size.height() != 0) {
    setPixmap(QPixmap::fromImage(_img).scaled(_size));
  } else {
    setPixmap(QPixmap::fromImage(_img));
  }
}

void Preview::renderImg() {
  _initImage();
  _img.fill(Qt::transparent);
  QPainter painter(&_img);
  QSvgRenderer *renderer = new QSvgRenderer(getCurrentSvg().toUtf8());
  renderer->setAspectRatioMode(Qt::KeepAspectRatio);
  renderer->render(&painter);
}

QString Preview::getCurrentSvg() {
  QString ret;
  ret += QString(
             "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" "
             "width=\"%1\" height=\"%2\">")
             .arg(_info.w)
             .arg(_info.h);

  ret +=
      QString(
          "<rect x=\"0\" y=\"0\" width=\"%1\" height=\"%2\" fill=\"#%3%4%5\"/>")
          .arg(_info.w)
          .arg(_info.h)
          .arg(_bg.r, 1, 16)
          .arg(_bg.g, 1, 16)
          .arg(_bg.b, 1, 16);

  ret += "<g transform=\"scale(1) translate(0.5 0.5)\">";
  for (auto shape : _shapes) {
    ret += shape;
  }
  ret += "</g>";
  ret += "</svg>";
  return ret;
}

void Preview::clearDrawing() {
  _shapes.clear();
  renderCurrentState();
}
