#pragma once

#include <QGraphicsView>

#include "rust_bridge.h"

QT_BEGIN_NAMESPACE
class QGraphicsSvgItem;
class QGraphicsRectItem;
class QGraphicsScene;
class QSvgRenderer;
QT_END_NAMESPACE

class Preview : public QGraphicsView {
  Q_OBJECT
 public:
  explicit Preview(QWidget *parent = nullptr);
  void drawBackground(QPainter *p, const QRectF &rect) override;
  QSvgRenderer *renderer() const;
  void renderCurrentState();
  QString getCurrentSvg();

 public slots:
  void setBg(const PurrmitiveColor &color, const PurrmitiveContextInfo &info);
  void step(const QString &svg, const PurrmitiveContextInfo &info);

 private:
  QGraphicsSvgItem *_svg;
  QGraphicsRectItem *_background;
  std::vector<QString> _shapes;
  PurrmitiveContextInfo _info;
  PurrmitiveColor _bg;
};
