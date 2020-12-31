#include "main_window.h"

#include <QDebug>
#include <QtWidgets>

#include "setting_dialog.h"

namespace {

void initImageFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode mode) {
  static bool first_dialog = true;
  if (first_dialog) {
    first_dialog = false;
    const QStringList picturesLocations =
        QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
    dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath()
                                                    : picturesLocations.last());
  }
  QStringList mime_types;
  const QByteArrayList supported_mime_types =
      mode == QFileDialog::AcceptOpen ? QImageReader::supportedMimeTypes()
                                      : QImageWriter::supportedMimeTypes();
  for (const QByteArray &mime : supported_mime_types) mime_types.append(mime);
  mime_types.sort();
  dialog.setMimeTypeFilters(mime_types);
  dialog.selectMimeTypeFilter("image/jpeg");
  if (mode == QFileDialog::AcceptSave) dialog.setDefaultSuffix("png");
}

QSize getInitialWindowSize() {
  return QGuiApplication::primaryScreen()->availableSize() * 3 / 5;
}

}  // namespace

MainWindow::MainWindow()
    : _image_label(new QLabel),
      _scroll_area(new QScrollArea),
      _setting_dialog(new SettingDialog(&_param, _image)),
      _preview(new Preview()),
      _zstack(new QStackedWidget()) {
  // set central
  _image_label->setBackgroundRole(QPalette::Base);
  _image_label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
  _image_label->setScaledContents(true);

  _scroll_area->setBackgroundRole(QPalette::Dark);
  _scroll_area->setWidget(_image_label);
  _scroll_area->setVisible(false);

  _zstack->addWidget(_scroll_area);
  _zstack->addWidget(_preview);

  setCentralWidget(_zstack);

  // init actions
  createActions();

  resize(getInitialWindowSize());
}

bool MainWindow::loadImage(const QString &file) {
  QImageReader reader(file);
  reader.setAutoTransform(true);
  const QImage img = reader.read();
  if (img.isNull()) {
    QMessageBox::information(
        this, QGuiApplication::applicationDisplayName(),
        tr("Cannot load %1: %2")
            .arg(QDir::toNativeSeparators(file), reader.errorString()));
  } else {
    resizeImageWindow(img);
    setImage(img);
    setWindowFilePath(file);
    const QString message = tr("Opened \"%1\", %2x%3, Depth: %4")
                                .arg(QDir::toNativeSeparators(file))
                                .arg(img.width())
                                .arg(img.height())
                                .arg(img.depth());
    statusBar()->showMessage(message);

    _input = file.toStdString();
    _param.input = _input.c_str();

    return true;
  }
}

void MainWindow::createActions() {
  QMenu *file_menu = menuBar()->addMenu(tr("&File"));
  QAction *open_action =
      file_menu->addAction(tr("&Open"), this, &MainWindow::open);
  open_action->setShortcut(QKeySequence::Open);
  QToolBar *file_tool_bar = addToolBar(tr("File"));
  file_tool_bar->addAction(open_action);
  file_tool_bar->setMovable(false);

  connect(_setting_dialog->_thumbnail_selector, &QPushButton::released, this,
          &MainWindow::open);

  QMenu *edit_menu = menuBar()->addMenu(tr("&Setting"));
  QAction *param_action =
      edit_menu->addAction(tr("&Params"), this, &MainWindow::openSetting);
  QToolBar *edit_tool_bar = addToolBar(tr("Params"));
  edit_tool_bar->addAction(param_action);
  edit_tool_bar->setMovable(false);

  QMenu *run_menu = menuBar()->addMenu(tr("&Run"));
  QAction *start_action =
      run_menu->addAction(tr("&Start"), this, &MainWindow::openSetting);
  QToolBar *start_tool_bar = addToolBar(tr("Start"));
  start_tool_bar->addAction(start_action);
  start_tool_bar->setMovable(false);

  QAction *pause_resume_action =
      run_menu->addAction(tr("Pause/Resume"), this, &MainWindow::openSetting);
  QToolBar *pause_resume_tool_bar = addToolBar(tr("Pause/Resume"));
  pause_resume_tool_bar->addAction(pause_resume_action);
  pause_resume_tool_bar->setMovable(false);

  QAction *step_action =
      run_menu->addAction(tr("Step"), this, &MainWindow::openSetting);
  QToolBar *step_tool_bar = addToolBar(tr("Step"));
  step_tool_bar->addAction(step_action);
  step_tool_bar->setMovable(false);

  QAction *stop_action =
      run_menu->addAction(tr("Stop"), this, &MainWindow::openSetting);
  QToolBar *stop_tool_bar = addToolBar(tr("Stop"));
  stop_tool_bar->addAction(stop_action);
  stop_tool_bar->setMovable(false);

  connect(&_controller, &PurrmitiveController::onBgReceived, _preview,
          &Preview::setBg);
}

bool MainWindow::isParamValid() {
  if (_param.input != nullptr) return true;
  return false;
}

void MainWindow::open() {
  QFileDialog dialog(this, tr("Open File"));
  initImageFileDialog(dialog, QFileDialog::AcceptOpen);

  while (dialog.exec() == QDialog::Accepted &&
         !loadImage(dialog.selectedFiles().first())) {
  }

  if (!_image.isNull()) {
    _setting_dialog->updateImage(_image);
  }

  if (isParamValid()) {
    _controller.init(&_param);
    _zstack->setCurrentIndex(1);
  }
}

void MainWindow::openSetting() { _setting_dialog->exec(); }

void MainWindow::resizeImageWindow(const QImage &image) {
  // resize the window size
  int w = image.width();
  int h = image.height();
  QSize screen_size = getInitialWindowSize();
  int screen_w = screen_size.width();
  int screen_h = screen_size.height();

  int h1 = int(double(h * screen_w) / double(w));
  if (h1 <= screen_h) {
    resize(screen_w, h1);
  } else {
    int w1 = int(double(w * screen_h) / double(h));
    resize(w1, screen_h);
  }
}

void MainWindow::setImage(const QImage &image) {
  _image = image;
  _image_label->setPixmap(QPixmap::fromImage(_image));
  _scroll_area->setVisible(true);
  _scroll_area->setWidgetResizable(true);
  _image_label->adjustSize();
}
