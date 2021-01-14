#pragma once

#include <QImage>
#include <QMainWindow>

#include "preview.h"
#include "rust_bridge.h"
#include "setting_dialog.h"

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QLabel;
class QScrollArea;
class QStackedWidget;
class QDragEnterEvent;
class QDropEvent;
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT
 public:
  MainWindow();
  bool loadImage(const QString &);

 protected:
  void dragEnterEvent(QDragEnterEvent *event) override;
  void dropEvent(QDropEvent *event) override;

 private slots:
  void open();
  void openSetting();
  void start();
  void step();
  void stop();
  void pauseResume();
  void reset();

 private:
  void createActions();
  void setImage(const QImage &image);
  void resizeImageWindow();
  bool isParamValid();
  void showStatus();
  void onBgReceived(const PurrmitiveColor &color,
                    const PurrmitiveContextInfo &info);
  void onStepResultReceived(const QString &svg,
                            const PurrmitiveContextInfo &info);

  QImage _image;
  QLabel *_image_label;
  SettingDialog *_setting_dialog;
  Preview *_preview;
  QStackedWidget *_zstack;
  std::string _input;
  PurrmitiveParam _param;
  PurrmitiveController _controller;
  bool _cont_run = false;
};
