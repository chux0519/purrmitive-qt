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

 private:
  void createHorizontalGroupBox();
  void createUpGroupBox();
  void createDownGroupBox();
  void createButtons();
  void setMode(int mode);
  void setAlphaBySpinBox(int val);
  void setAlphaBySlider(int val);
  void setDefaultParams();
  void confirm();
  void clear();

  QGroupBox* _upGroupBox;
  QGroupBox* _upLeftGroupBox;
  QGroupBox* _upRightGroupBox;
  QGroupBox* _downGroupBox;
  QHBoxLayout* _buttons_layout;

  // upleft
  QImage _thumbnail_img;
  QLabel* _thumbnail;
  QPushButton* _thumbnail_selector;
  // upright
  QPushButton* _modesButtons[9];
  // bottomleft
  QCheckBox* _alpha_box;
  QSpinBox* _alpha_spin;
  QSlider* _alpha_slider;

  QSpinBox* _count_spin;

  // clear button
  QPushButton* _clear_button;

  PurrmitiveParam* _param;
  StopCond* _stop_cond;

  friend class MainWindow;
};
