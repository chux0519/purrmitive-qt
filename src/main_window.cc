#include "main_window.h"

#include <QDebug>
#include <QtWidgets>

#include "setting_dialog.h"

namespace {

const int TOOLBAR_HEIGHT = 30;
const int WINDOW_SIZE = 500;

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
  // FIXME: bug in macOS big sur
  dialog.selectMimeTypeFilter("image/jpeg");
  if (mode == QFileDialog::AcceptSave) dialog.setDefaultSuffix("png");
}

QSize getInitialWindowSize() {
  return QGuiApplication::primaryScreen()->availableSize() * 3 / 5;
}

}  // namespace

MainWindow::MainWindow()
    : _image_label(new QLabel),
      _setting_dialog(new SettingDialog(&_param, &_stop_cond, _image)),
      _preview(new Preview()),
      _zstack(new QStackedWidget()) {
  _image_label->setText("Open an image\n or Drag and Drop in the window");
  _image_label->setAlignment(Qt::AlignCenter);
  QFont f = _image_label->font();
  f.setPixelSize(24);
  f.setPointSize(24);
  f.setBold(true);
  _image_label->setFont(f);
  QPalette palette = _image_label->palette();
  palette.setColor(_image_label->backgroundRole(), Qt::gray);
  palette.setColor(_image_label->foregroundRole(), Qt::gray);
  _image_label->setPalette(palette);

  _zstack->addWidget(_image_label);
  _zstack->addWidget(_preview);

  setCentralWidget(_zstack);

  // init actions
  createActions();

  resize(WINDOW_SIZE, WINDOW_SIZE);
  setAcceptDrops(true);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
  event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event) {
  event->acceptProposedAction();

  if (event->mimeData()->hasUrls()) {
    QList<QUrl> urls = event->mimeData()->urls();
    if (loadImage(urls.first().path())) reset();
  }
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
  if (_controller.getStep() <= 0) return;
  if (event->key() == Qt::Key_Space) showOriginImage();
}

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
  if (_controller.getStep() <= 0) return;
  if (event->key() == Qt::Key_Space) {
    if (isParamValid()) showPreviewImage();
  }
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

    _setting_dialog->updateImage(_image);
    resizeImageWindow();

    emit imageSelected();
    return true;
  }
  return false;
}

void MainWindow::createActions() {
  QMenu *file_menu = menuBar()->addMenu(tr("&File"));
  QAction *open_action =
      file_menu->addAction(tr("&Open"), this, &MainWindow::open);
  open_action->setShortcut(QKeySequence::Open);
  QToolBar *file_tool_bar = addToolBar(tr("File"));
  file_tool_bar->addAction(open_action);
  file_tool_bar->setMovable(false);
  file_tool_bar->setFixedHeight(TOOLBAR_HEIGHT);

  QAction *save_action =
      file_menu->addAction(tr("&Save"), this, &MainWindow::save);
  save_action->setShortcut(QKeySequence::Save);
  QToolBar *save_tool_bar = addToolBar(tr("Save"));
  save_tool_bar->addAction(save_action);
  save_tool_bar->setMovable(false);
  save_tool_bar->setFixedHeight(TOOLBAR_HEIGHT);
  save_tool_bar->setContentsMargins(0, 0, 40, 0);
  save_tool_bar->setEnabled(false);

  connect(_setting_dialog, &SettingDialog::selectImage, this,
          &MainWindow::open);
  connect(_setting_dialog, &SettingDialog::clearDrawing, this,
          &MainWindow::reset);
  connect(_setting_dialog, &SettingDialog::startDrawing, this,
          &MainWindow::start);

  QMenu *edit_menu = menuBar()->addMenu(tr("&Setup"));
  QAction *param_action =
      edit_menu->addAction(tr("&Setup"), this, &MainWindow::openSetting);
  QToolBar *edit_tool_bar = addToolBar(tr("Setup"));
  edit_tool_bar->addAction(param_action);
  edit_tool_bar->setMovable(false);

  QMenu *run_menu = menuBar()->addMenu(tr("&Run"));
  QAction *start_action =
      run_menu->addAction(tr("&Start"), this, &MainWindow::start);
  QToolBar *start_tool_bar = addToolBar(tr("Start"));
  start_tool_bar->addAction(start_action);
  start_tool_bar->setMovable(false);
  start_tool_bar->setEnabled(false);
  connect(this, &MainWindow::imageSelected, start_tool_bar,
          [=]() { start_tool_bar->setEnabled(true); });

  QAction *step_action =
      run_menu->addAction(tr("Step"), this, &MainWindow::step);
  QToolBar *step_tool_bar = addToolBar(tr("Step"));
  step_tool_bar->addAction(step_action);
  step_tool_bar->setMovable(false);
  step_tool_bar->setEnabled(false);

  QAction *stop_action =
      run_menu->addAction(tr("Stop"), this, &MainWindow::pause);
  QToolBar *stop_tool_bar = addToolBar(tr("Stop"));
  stop_tool_bar->addAction(stop_action);
  stop_tool_bar->setMovable(false);
  stop_tool_bar->setEnabled(false);

  connect(this, &MainWindow::workerStarted, stop_tool_bar, [=]() {
    stop_tool_bar->setEnabled(true);
    step_tool_bar->setEnabled(false);
    start_tool_bar->setEnabled(false);
  });
  connect(this, &MainWindow::workerPaused, step_tool_bar, [=]() {
    stop_tool_bar->setEnabled(false);
    step_tool_bar->setEnabled(true);
    start_tool_bar->setEnabled(true);
    save_tool_bar->setEnabled(true);
  });

  QAction *clear_action =
      run_menu->addAction(tr("Clear"), this, &MainWindow::reset);

  connect(&_controller, &PurrmitiveController::onBgReceived, _preview,
          &Preview::setBg);
  connect(&_controller, &PurrmitiveController::onStepResultReceived, _preview,
          &Preview::step);

  connect(&_controller, &PurrmitiveController::onBgReceived, this,
          &MainWindow::onBgReceived);
  connect(&_controller, &PurrmitiveController::onStepResultReceived, this,
          &MainWindow::onStepResultReceived);
}

void MainWindow::showStatus() {
  uint32_t step = _controller.getStep() < 0 ? 0 : _controller.getStep();
  statusBar()->showMessage(
      QString("Score: %1, Step: %2").arg(_controller.getScore()).arg(step));
}
void MainWindow::onBgReceived(const PurrmitiveColor &color,
                              const PurrmitiveContextInfo &info) {
  showStatus();
  if (shouldRun()) step();
}

void MainWindow::onStepResultReceived(const QString &svg,
                                      const PurrmitiveContextInfo &info) {
  showStatus();
  // use timer to prevent main thread blocking
  // rate at around 60/s
  QTimer::singleShot(17, [=]() {
    if (shouldRun())
      step();
    else
      emit workerPaused();
  });
}

bool MainWindow::isParamValid() {
  if (_param.input != nullptr && !_image.isNull()) return true;
  return false;
}

void MainWindow::open() {
  QFileDialog dialog(this, tr("Open File"));
  initImageFileDialog(dialog, QFileDialog::AcceptOpen);

  while (dialog.exec() == QDialog::Accepted &&
         !loadImage(dialog.selectedFiles().first())) {
  }
  reset();
}

void MainWindow::save() {
  QString file_name = QFileDialog::getSaveFileName(this, tr("Save File"), "",
                                                   tr("Images (*.png *.jpg)"));
  if (saveImage(file_name)) {
    QMessageBox msg_box;
    msg_box.setText(QString("Result has been saved: %1").arg(file_name));
    msg_box.exec();
  }
}

bool MainWindow::saveImage(const QString &output) {
  return _preview->saveImg(output);
}

void MainWindow::openSetting() { _setting_dialog->exec(); }

void MainWindow::start() {
  if (!isParamValid()) return;
  showPreviewImage();
  _stop_cond.noStop = true;
  step();
  emit workerStarted();
}

void MainWindow::step() {
  if (!isParamValid()) return;
  if (!shouldRun()) showPreviewImage();
  _controller.doStartOrStep(&_param);
}

void MainWindow::_stop() {
  _stop_cond.noStop = false;
  _controller.doStop();
}

void MainWindow::pause() {
  _stop_cond.noStop = false;
  emit workerPaused();
}

void MainWindow::setImage(const QImage &image) {
  _image = image;
  QSize init_size = getInitialWindowSize();
  _image =
      _image.scaled(init_size.width(), init_size.height(), Qt::KeepAspectRatio);
  _image_label->setPixmap(QPixmap::fromImage(_image));
  _image_label->setAlignment(Qt::AlignCenter);
}

void MainWindow::resizeImageWindow() {
  if (_image.isNull()) return;
  int title_bar_height =
      QApplication::style()->pixelMetric(QStyle::PM_TitleBarHeight);
  QSize img_size = _image.size();
  qDebug() << img_size;
  _image_label->resize(img_size);
  _preview->setSize(img_size);

  QSize new_window_size(img_size.width(),
                        img_size.height() + title_bar_height + TOOLBAR_HEIGHT);
  resize(new_window_size);
  setMinimumSize(new_window_size);
  setMaximumSize(new_window_size);
}

void MainWindow::reset() {
  _stop();
  _preview->clearDrawing();
  showOriginImage();
  showStatus();
}

void MainWindow::showOriginImage() {
  if (_zstack->currentIndex() != 0) _zstack->setCurrentIndex(0);
}

void MainWindow::showPreviewImage() {
  if (_zstack->currentIndex() != 1) _zstack->setCurrentIndex(1);
}
