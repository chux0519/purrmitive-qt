#include "preview.h"

#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsSvgItem>
#include <QSvgRenderer>

Preview::Preview(QWidget *parent)
    : QGraphicsView(parent), _svg(nullptr), _background(nullptr) {
  setScene(new QGraphicsScene(this));
}

void Preview::drawBackground(QPainter *p, const QRectF &) {
  p->save();
  p->resetTransform();
  p->drawTiledPixmap(viewport()->rect(), backgroundBrush().texture());
  p->restore();
}

void Preview::setBg(const PurrmitiveColor &color,
                    const PurrmitiveContextInfo &info) {
  _bg = color;
  _info = info;
  QColor c(color.r, color.g, color.b, color.a);
  QPixmap tilePixmap(512, 512);
  tilePixmap.fill(Qt::transparent);
  QPainter tilePainter(&tilePixmap);
  tilePainter.fillRect(0, 0, 512, 512, c);
  tilePainter.end();

  setBackgroundBrush(tilePixmap);
}

QSvgRenderer *Preview::renderer() const {
  if (_svg) return _svg->renderer();
  return nullptr;
}

void Preview::step(const QString &svg, const PurrmitiveContextInfo &info) {
  _info = info;
  _shapes.push_back(svg);

  renderCurrentState();
}

void Preview::renderCurrentState() {
  QGraphicsScene *s = scene();
  s->clear();

  QSvgRenderer *renderer = new QSvgRenderer(getCurrentSvg().toUtf8());
  // qDebug() << renderer->isValid();
  _svg = new QGraphicsSvgItem();
  _svg->setSharedRenderer(renderer);
  QSize preview_size = size();
  double scale_ratio =
      std::min((double)preview_size.width() / (double)_info.w,
               (double)preview_size.height() / (double)_info.h);
  _svg->setScale(scale_ratio);
  // to hide to scrollbar
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  s->addItem(_svg);
  // qDebug() << sceneRect();
  // qDebug() << "rendered" << preview_size << ", " << _info.w * scale_ratio
  //          << ", " << _info.h * scale_ratio;
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
