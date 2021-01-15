#pragma once

#include <QDialog>

#include "rust_bridge.h"

QT_BEGIN_NAMESPACE
class QAction;
class QDialogButtonBox;
class QGroupBox;
class QPushButton;
class QLabel;
class QCheckBox;
class QSpinBox;
class QSlider;
class QHBoxLayout;
QT_END_NAMESPACE

class MainWindow;

class SettingDialog : public QDialog {
  Q_OBJECT;

 public:
  SettingDialog(PurrmitiveParam*, StopCond*, const QImage&);
  void updateImage(const QImage& img);

 signals:
  void clearDrawing();
  void selectImage();

 private:
  QGroupBox* createUpGroupBox();
  QGroupBox* createDownGroupBox();
  QHBoxLayout* createButtons();
  void setMode(int mode);
  void setAlphaBySpinBox(int val);
  void setAlphaBySlider(int val);
  void setDefaultParams();

  // upleft
  QImage _thumbnail_img;
  QLabel* _thumbnail;

  // upright
  QPushButton* _modesButtons[9];

  // bottomleft
  QSpinBox* _alpha_spin;
  QSlider* _alpha_slider;

  PurrmitiveParam* _param;
  StopCond* _stop_cond;

  friend class MainWindow;
};
