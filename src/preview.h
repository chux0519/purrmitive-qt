#pragma once

#include <QGraphicsView>

#include "rust_bridge.h"

QT_BEGIN_NAMESPACE
class QGraphicsSvgItem;
class QGraphicsRectItem;
QT_END_NAMESPACE

class Preview : public QGraphicsView {
  Q_OBJECT
 public:
  explicit Preview(QWidget *parent = nullptr);
  void drawBackground(QPainter *p, const QRectF &rect) override;

 public slots:
  void setBg(const PurrmitiveColor &color);

 private:
  QGraphicsSvgItem *_svg;
  QGraphicsRectItem *_background;
};
