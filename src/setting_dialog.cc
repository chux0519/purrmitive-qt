#include "setting_dialog.h"

#include <QtWidgets>
#include <cmath>

const char* MODES[] = {"combo",   "triangle",       "rect",
                       "ellipse", "circle",         "rotatedrect",
                       "beziers", "rotatedellipse", "polygon"};
static int THUMBNAIL_SIZE = 96;

SettingDialog::SettingDialog(PurrmitiveParam* param, const QImage& img)
    : _thumbnail_img(img),
      _thumbnail(new QLabel),
      _thumbnail_selector(new QPushButton(tr("Open Image"))),
      _param(param) {
  createUpGroupBox();
  createDownGroupBox();

  _buttonBox =
      new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  connect(_buttonBox, &QDialogButtonBox::accepted, this,
          &SettingDialog::confirm);
  connect(_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

  QVBoxLayout* mainLayout = new QVBoxLayout;

  mainLayout->addWidget(_upGroupBox);
  mainLayout->addWidget(_downGroupBox);
  mainLayout->addWidget(_buttonBox);
  setLayout(mainLayout);
  setWindowTitle(tr("Purrmitive Settings"));
  setDefaultParams();
}

void SettingDialog::confirm() {
  qDebug() << "alpha: " << _param->alpha << ", ";
  qDebug() << "count: " << _param->count << ", ";
  qDebug() << "input: " << _param->input << ", ";
  qDebug() << "mode: " << _param->mode << ", ";
  qDebug() << "resize: " << _param->resize << ", ";
  qDebug() << "size: " << _param->size << ", ";
  this->accept();
}

void SettingDialog::setDefaultParams() {
  // init default
  int default_alpha = 128, default_count = 100, default_resize = 256,
      default_size = 512;
  _alpha_spin->setValue(default_alpha);
  _count_spin->setValue(default_count);
  _param->resize = default_resize;
  _param->size = default_size;
}

void SettingDialog::updateImage(const QImage& img) {
  _thumbnail_img = img;
  _thumbnail_img = _thumbnail_img.scaled(THUMBNAIL_SIZE, THUMBNAIL_SIZE,
                                         Qt::AspectRatioMode::KeepAspectRatio);
  _thumbnail->setPixmap(QPixmap::fromImage(_thumbnail_img));
  _thumbnail->setAlignment(Qt::AlignCenter);
}

void SettingDialog::createUpGroupBox() {
  _upGroupBox = new QGroupBox();
  QHBoxLayout* hlayout = new QHBoxLayout;

  _upLeftGroupBox = new QGroupBox("Select Image");
  QVBoxLayout* ul_layout = new QVBoxLayout;
  updateImage(_thumbnail_img);
  ul_layout->addWidget(_thumbnail);
  ul_layout->addWidget(_thumbnail_selector);

  _upRightGroupBox = new QGroupBox(tr("Select Shape Type"));
  QGridLayout* ur_layout = new QGridLayout;
  int rows = 3, cols = 3;
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < cols; ++j) {
      int idx = i * 3 + j;
      _modesButtons[idx] = new QPushButton(tr("%1").arg(MODES[idx]));
      connect(_modesButtons[idx], &QPushButton::released, this,
              [idx, this]() { setMode(idx); });
      ur_layout->addWidget(_modesButtons[idx], i, j);
    }
  }

  int init_mode = 1;
  setMode(init_mode);

  _upLeftGroupBox->setLayout(ul_layout);
  _upRightGroupBox->setLayout(ur_layout);
  hlayout->addWidget(_upLeftGroupBox);
  hlayout->addWidget(_upRightGroupBox);
  _upGroupBox->setLayout(hlayout);
}

void SettingDialog::createDownGroupBox() {
  _downGroupBox = new QGroupBox();
  QGridLayout* layout = new QGridLayout;

  // 1st row
  QHBoxLayout* hlayout1 = new QHBoxLayout;
  _alpha_box = new QCheckBox();
  _alpha_box->setChecked(false);
  _alpha_spin = new QSpinBox();
  _alpha_spin->setEnabled(false);
  _alpha_spin->setMaximum(255);
  _alpha_slider = new QSlider(Qt::Horizontal);
  _alpha_slider->setFocusPolicy(Qt::StrongFocus);
  _alpha_slider->setTickPosition(QSlider::TicksBothSides);
  _alpha_slider->setTickInterval(20);
  _alpha_slider->setSingleStep(20);
  _alpha_slider->setEnabled(false);
  connect(_alpha_box, &QCheckBox::toggled, _alpha_spin, &QSpinBox::setEnabled);
  connect(_alpha_box, &QCheckBox::toggled, _alpha_slider, &QSlider::setEnabled);
  connect(_alpha_slider, &QSlider::valueChanged, this,
          &SettingDialog::setAlphaBySlider);
  connect(_alpha_spin, QOverload<int>::of(&QSpinBox::valueChanged), this,
          &SettingDialog::setAlphaBySpinBox);
  hlayout1->addWidget(_alpha_box);
  hlayout1->addWidget(new QLabel(tr("Shape alpha: ")));
  layout->addLayout(hlayout1, 0, 0, Qt::AlignRight);
  layout->addWidget(_alpha_spin, 0, 1);
  layout->addWidget(_alpha_slider, 0, 2, 1, 2);

  // 2nd row
  _count_spin = new QSpinBox();
  _count_spin->setMaximum(65535);
  layout->addWidget(new QLabel(tr("Shape number: ")), 1, 0, Qt::AlignRight);
  layout->addWidget(_count_spin, 1, 1);
  connect(_count_spin, QOverload<int>::of(&QSpinBox::valueChanged), this,
          &SettingDialog::setCount);

  _downGroupBox->setLayout(layout);
}

void SettingDialog::setCount(int val) { _param->count = val; }

void SettingDialog::setAlphaBySpinBox(int val) {
  int percent = round(double(val * 100) / (double)256);
  _param->alpha = val;

  disconnect(_alpha_slider, &QSlider::valueChanged, this,
             &SettingDialog::setAlphaBySlider);
  _alpha_slider->setValue(percent);
  connect(_alpha_slider, &QSlider::valueChanged, this,
          &SettingDialog::setAlphaBySlider);
}

void SettingDialog::setAlphaBySlider(int val) {
  if (val == 99) val += 1;
  int alpha = round(double(val * 256) / (double)100);
  _param->alpha = alpha;

  disconnect(_alpha_spin, QOverload<int>::of(&QSpinBox::valueChanged), this,
             &SettingDialog::setAlphaBySpinBox);
  _alpha_spin->setValue(alpha);
  connect(_alpha_spin, QOverload<int>::of(&QSpinBox::valueChanged), this,
          &SettingDialog::setAlphaBySpinBox);
}

void SettingDialog::setMode(int mode) {
  if (mode >= 9) return;
  for (int i = 0; i < 9; ++i) {
    _modesButtons[i]->setDown(i == mode);
  }
  _param->mode = mode;
}
