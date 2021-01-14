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
  inline bool shouldRun() {
    if (_controller.getScore() >= _stop_cond.stopScore) {
      return false;
    }
    if (_controller.getStep() >= _stop_cond.stopShapes) {
      return false;
    }
    return _stop_cond.noStop;
  };

  QImage _image;
  QLabel *_image_label;
  SettingDialog *_setting_dialog;
  Preview *_preview;
  QStackedWidget *_zstack;
  std::string _input;
  PurrmitiveParam _param;
  PurrmitiveController _controller;
  StopCond _stop_cond;
};
