#include "setting_dialog.h"

#include <QtWidgets>

SettingDialog::SettingDialog(PurrmitiveParam* param) : _param(param) {
  _buttonBox =
      new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  connect(_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

  QVBoxLayout* mainLayout = new QVBoxLayout;

  mainLayout->addWidget(_buttonBox);
  setLayout(mainLayout);
  setWindowTitle(tr("Setting"));
}
