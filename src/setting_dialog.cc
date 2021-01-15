#include "setting_dialog.h"

#include <QtWidgets>
#include <cmath>

const char* MODES[] = {"combo",   "triangle",       "rect",
                       "ellipse", "circle",         "rotatedrect",
                       "beziers", "rotatedellipse", "polygon"};
static int THUMBNAIL_SIZE = 96;

SettingDialog::SettingDialog(PurrmitiveParam* param, StopCond* stop_cond,
                             const QImage& img)
    : _thumbnail_img(img),
      _thumbnail(new QLabel),
      _param(param),
      _stop_cond(stop_cond) {
  QGroupBox* upGroupBox = createUpGroupBox();
  QGroupBox* downGroupBox = createDownGroupBox();

  QHBoxLayout* buttons_layout = createButtons();

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(upGroupBox);
  mainLayout->addWidget(downGroupBox);
  mainLayout->addLayout(buttons_layout);
  setLayout(mainLayout);
  setWindowTitle(tr("Purrmitive Setup"));

  setDefaultParams();
}

QHBoxLayout* SettingDialog::createButtons() {
  QHBoxLayout* _buttons_layout = new QHBoxLayout;
  QPushButton* _clear_button = new QPushButton("Clear Drawing");
  _buttons_layout->addWidget(_clear_button, 0, Qt::AlignLeft);
  QDialogButtonBox* buttons =
      new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

  connect(_clear_button, &QPushButton::clicked, [=]() { emit clearDrawing(); });
  connect(buttons, &QDialogButtonBox::accepted, [=]() {
    qDebug() << "alpha: " << _param->alpha << ", ";
    qDebug() << "bg: " << _param->bg << ", ";
    qDebug() << "input: " << _param->input << ", ";
    qDebug() << "mode: " << _param->mode << ", ";
    qDebug() << "resize: " << _param->resize << ", ";
    qDebug() << "size: " << _param->size << ", ";
    // should also emit start signal
    this->accept();
  });
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
  _buttons_layout->addWidget(buttons, 0, Qt::AlignRight);
  return _buttons_layout;
}

void SettingDialog::setDefaultParams() {
  // init default
  int default_alpha = 128, default_count = 100, default_resize = 256,
      default_size = 512;
  _alpha_spin->setValue(default_alpha);
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

QGroupBox* SettingDialog::createUpGroupBox() {
  QGroupBox* _upGroupBox = new QGroupBox();
  QHBoxLayout* hlayout = new QHBoxLayout;

  QGroupBox* _upLeftGroupBox = new QGroupBox("Select Image");
  QVBoxLayout* ul_layout = new QVBoxLayout;
  QPushButton* _thumbnail_selector = new QPushButton(tr("Open Image"));
  connect(_thumbnail_selector, &QPushButton::clicked,
          [=]() { emit selectImage(); });

  updateImage(_thumbnail_img);

  ul_layout->addWidget(_thumbnail);
  ul_layout->addWidget(_thumbnail_selector);

  QGroupBox* _upRightGroupBox = new QGroupBox(tr("Select Shape Type"));
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
  return _upGroupBox;
}

QGroupBox* SettingDialog::createDownGroupBox() {
  QGroupBox* _downGroupBox = new QGroupBox();
  QGridLayout* layout = new QGridLayout;

  // 1st col
  QVBoxLayout* col1 = new QVBoxLayout;

  QHBoxLayout* hlayout1 = new QHBoxLayout;
  QCheckBox* _alpha_box = new QCheckBox();
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
  hlayout1->addWidget(_alpha_spin);
  hlayout1->addWidget(_alpha_slider);
  hlayout1->addSpacing(40);

  col1->addLayout(hlayout1);
  layout->addLayout(col1, 0, 0, Qt::AlignTop);

  // 2nd col
  QVBoxLayout* col2 = new QVBoxLayout;

  QHBoxLayout* r1 = new QHBoxLayout;
  QRadioButton* r1_btn = new QRadioButton("Run until stopped");
  r1_btn->setChecked(true);
  connect(r1_btn, &QRadioButton::toggled, [=]() {
    _stop_cond->noStop = true;
    _stop_cond->stopScore = 1.0;
    _stop_cond->stopShapes = INT_MAX;
  });
  r1->addWidget(r1_btn);
  col2->addLayout(r1);

  QSpinBox* _count_spin = new QSpinBox();
  _count_spin->setMaximum(INT_MAX);
  _count_spin->setEnabled(false);
  // default shape to 100
  _count_spin->setValue(100);

  QHBoxLayout* r2 = new QHBoxLayout;
  QRadioButton* r2_btn = new QRadioButton("Run until shapes:");
  connect(r2_btn, &QRadioButton::toggled, [=]() {
    _stop_cond->noStop = false;
    _stop_cond->stopScore = 1.0;
    _count_spin->setEnabled(true);
    _stop_cond->stopShapes = _count_spin->value();
  });
  connect(_count_spin, QOverload<int>::of(&QSpinBox::valueChanged),
          [=](int val) {
            _stop_cond->noStop = false;
            _stop_cond->stopScore = 1.0;
            _stop_cond->stopShapes = val;
          });
  r2->addWidget(r2_btn);
  r2->addWidget(_count_spin);
  col2->addLayout(r2);

  QHBoxLayout* r3 = new QHBoxLayout;
  QRadioButton* r3_btn = new QRadioButton("Run until score:");
  QLineEdit* r3_line = new QLineEdit("95");

  connect(r3_btn, &QRadioButton::toggled, [=]() {
    _stop_cond->noStop = false;
    _stop_cond->stopShapes = INT_MAX;
    r3_line->setEnabled(true);
    _stop_cond->stopScore = r3_line->text().toDouble() / 100.0;
  });
  r3_line->setValidator(new QIntValidator(85, 100, this));
  r3_line->setEnabled(false);
  r3_line->setMaximumWidth(30);

  connect(r3_line, &QLineEdit::textChanged, [=](const QString& val) {
    _stop_cond->noStop = false;
    _stop_cond->stopScore = val.toDouble() / 100.0;
    _stop_cond->stopShapes = INT_MAX;
  });

  r3->addWidget(r3_btn);
  r3->addWidget(r3_line);
  r3->addWidget(new QLabel("%"));
  col2->addLayout(r3);

  col2->setMargin(8);
  layout->addLayout(col2, 0, 1, Qt::AlignTop);

  _downGroupBox->setLayout(layout);

  return _downGroupBox;
}

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
