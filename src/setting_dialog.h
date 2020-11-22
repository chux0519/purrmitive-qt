#pragma once

#include <QDialog>

#include "purrmitive.h"

QT_BEGIN_NAMESPACE
class QAction;
class QDialogButtonBox;
class QGroupBox;
QT_END_NAMESPACE

class SettingDialog : public QDialog {
  Q_OBJECT;

 public:
  SettingDialog(PurrmitiveParam* param);

 private:
  void createHorizontalGroupBox();
  void createUpGroupBox();
  void createDownGroupBox();

  QGroupBox* _horizontalBox;
  QGroupBox* _upGroupBox;
  QGroupBox* _downGroupBox;
  QDialogButtonBox* _buttonBox;

  PurrmitiveParam* _param;
};
