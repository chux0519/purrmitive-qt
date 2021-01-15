#pragma once

#include <QImage>
#include <QLabel>
#include <vector>

#include "rust_bridge.h"

class Preview : public QLabel {
  Q_OBJECT
 public:
  explicit Preview(QWidget *parent = nullptr);
  void renderCurrentState();
  void renderImg();
  void setSize(QSize &);
  QString getCurrentSvg();

 public slots:
  void setBg(const PurrmitiveColor &color, const PurrmitiveContextInfo &info);
  void step(const QString &svg, const PurrmitiveContextInfo &info);
  void clearDrawing();

 private:
  inline void _initImage() {
    if (_info.w > _info.h) {
      _img = QImage(1024, round(1024.0 / (double)_info.w * (double)_info.h),
                    QImage::Format_ARGB32);
    } else {
      _img = QImage(round(1024.0 / (double)_info.h * (double)_info.w), 1024,
                    QImage::Format_ARGB32);
    };
  }
  QImage _img;
  QSize _size = QSize(0, 0);
  std::vector<QString> _shapes;
  PurrmitiveContextInfo _info;
  PurrmitiveColor _bg;
};
