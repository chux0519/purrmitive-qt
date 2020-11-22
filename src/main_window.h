#pragma once

#include <QImage>
#include <QMainWindow>

#include "purrmitive.h"

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QLabel;
class QScrollArea;
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT
 public:
  MainWindow();
  bool loadImage(const QString &);

 private slots:
  void open();
  void openSetting();

 private:
  void createActions();
  void setImage(const QImage &image);
  void resizeImageWindow(const QImage &image);

  QImage _image;
  QLabel *_image_label;
  QScrollArea *_scroll_area;
  PurrmitiveParam _param;
};
