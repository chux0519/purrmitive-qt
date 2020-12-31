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
  qDebug() << renderer->isValid();
  _svg = new QGraphicsSvgItem();
  _svg->setSharedRenderer(renderer);

  s->addItem(_svg);
  qDebug() << "rendered";
}

QString Preview::getCurrentSvg() {
  QString ret;
  ret += QString().sprintf(
      "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"%d\" "
      "height=\"%d\">",
      _info.w, _info.h);
  ret += QString().sprintf(
      "<rect x=\"0\" y=\"0\" width=\"%d\" height=\"%d\" "
      "fill=\"#%02X%02X%02X\"/>",
      _info.w, _info.h, _bg.r, _bg.g, _bg.b);
  ret += "<g transform=\"scale(1) translate(0.5 0.5)\">";
  for (auto shape : _shapes) {
    ret += shape;
  }
  ret += "</g>";
  ret += "</svg>";
  return ret;
}
