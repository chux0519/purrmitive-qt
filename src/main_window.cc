#include "main_window.h"

#include <QDebug>
#include <QtWidgets>

#include "setting_dialog.h"

namespace {

const int TOOLBAR_HEIGHT = 30;

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
      _setting_dialog(new SettingDialog(&_param, _image)),
      _preview(new Preview()),
      _zstack(new QStackedWidget()) {
  _image_label->setText("Open an image\n or Drag and Drop in the window");
  _image_label->setAlignment(Qt::AlignCenter);
  QPalette palette = _image_label->palette();
  palette.setColor(_image_label->backgroundRole(), Qt::gray);
  palette.setColor(_image_label->foregroundRole(), Qt::gray);
  _image_label->setPalette(palette);

  _zstack->addWidget(_image_label);
  _zstack->addWidget(_preview);

  setCentralWidget(_zstack);

  // init actions
  createActions();

  resize(256, 256);
  setAcceptDrops(true);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
  event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event) {
  event->acceptProposedAction();

  if (event->mimeData()->hasUrls()) {
    QList<QUrl> urls = event->mimeData()->urls();
    if (loadImage(urls.first().path())) {
      if (!_image.isNull()) {
        _setting_dialog->updateImage(_image);
        resizeImageWindow();
      }
    }
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
  file_tool_bar->setFixedHeight(TOOLBAR_HEIGHT);

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
      run_menu->addAction(tr("&Start"), this, &MainWindow::start);
  QToolBar *start_tool_bar = addToolBar(tr("Start"));
  start_tool_bar->addAction(start_action);
  start_tool_bar->setMovable(false);

  QAction *pause_resume_action =
      run_menu->addAction(tr("Pause/Resume"), this, &MainWindow::pauseResume);
  QToolBar *pause_resume_tool_bar = addToolBar(tr("Pause/Resume"));
  pause_resume_tool_bar->addAction(pause_resume_action);
  pause_resume_tool_bar->setMovable(false);

  QAction *step_action =
      run_menu->addAction(tr("Step"), this, &MainWindow::step);
  QToolBar *step_tool_bar = addToolBar(tr("Step"));
  step_tool_bar->addAction(step_action);
  step_tool_bar->setMovable(false);

  QAction *stop_action =
      run_menu->addAction(tr("Stop"), this, &MainWindow::stop);
  QToolBar *stop_tool_bar = addToolBar(tr("Stop"));
  stop_tool_bar->addAction(stop_action);
  stop_tool_bar->setMovable(false);

  connect(&_controller, &PurrmitiveController::onBgReceived, _preview,
          &Preview::setBg);
  connect(&_controller, &PurrmitiveController::onStepResultReceived, _preview,
          &Preview::step);

  connect(&_controller, &PurrmitiveController::onBgReceived, this,
          &MainWindow::displayBgInfo);
  connect(&_controller, &PurrmitiveController::onStepResultReceived, this,
          &MainWindow::displayStepInfo);
}

void MainWindow::displayBgInfo(const PurrmitiveColor &color,
                               const PurrmitiveContextInfo &info) {
  statusBar()->showMessage(
      QString("Score: %1, Step: %2").arg(1.0 - info.score).arg(_step));
}

void MainWindow::displayStepInfo(const QString &svg,
                                 const PurrmitiveContextInfo &info) {
  statusBar()->showMessage(
      QString("Score: %1, Step: %2").arg(1.0 - info.score).arg(_step));
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
}

void MainWindow::openSetting() { _setting_dialog->exec(); }

void MainWindow::start() {}

void MainWindow::step() {
  int idx = _zstack->currentIndex();
  if (idx == 0) {
    // start and set bg
    if (!_image.isNull()) {
      _setting_dialog->updateImage(_image);
      resizeImageWindow();
    }

    if (isParamValid()) {
      _step = 0;
      _controller.init(&_param);
      _zstack->setCurrentIndex(1);
    }
  } else if (idx == 1) {
    _step += 1;
    _controller.step();
  }
}

void MainWindow::stop() { _controller.stop(); }
void MainWindow::pauseResume() {}

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
  QSize new_window_size(img_size.width(),
                        img_size.height() + title_bar_height + TOOLBAR_HEIGHT);
  resize(new_window_size);
  setMinimumSize(new_window_size);
  setMaximumSize(new_window_size);
}
