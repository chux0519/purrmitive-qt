#include "preview.h"

#include <QGraphicsRectItem>
#include <QGraphicsSvgItem>

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

void Preview::setBg(const PurrmitiveColor &color) {
  QColor c(color.r, color.g, color.b, color.a);
  QPixmap tilePixmap(512, 512);
  tilePixmap.fill(Qt::transparent);
  QPainter tilePainter(&tilePixmap);
  tilePainter.fillRect(0, 0, 512, 512, c);
  tilePainter.end();

  setBackgroundBrush(tilePixmap);
}
