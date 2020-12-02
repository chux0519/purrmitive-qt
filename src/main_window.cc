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
      _setting_dialog(new SettingDialog(&_param, _image)) {
  // set central
  _image_label->setBackgroundRole(QPalette::Base);
  _image_label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
  _image_label->setScaledContents(true);

  _scroll_area->setBackgroundRole(QPalette::Dark);
  _scroll_area->setWidget(_image_label);
  _scroll_area->setVisible(false);

  setCentralWidget(_scroll_area);

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
  }

  resizeImageWindow(img);
  setImage(img);
  setWindowFilePath(file);
  const QString message = tr("Opened \"%1\", %2x%3, Depth: %4")
                              .arg(QDir::toNativeSeparators(file))
                              .arg(img.width())
                              .arg(img.height())
                              .arg(img.depth());
  statusBar()->showMessage(message);

  return true;
}

void MainWindow::createActions() {
  QMenu *file_menu = menuBar()->addMenu(tr("&File"));
  QAction *open_action =
      file_menu->addAction(tr("&Open"), this, &MainWindow::open);
  open_action->setShortcut(QKeySequence::Open);
  connect(_setting_dialog->_thumbnail_selector, &QPushButton::released, this,
          &MainWindow::open);

  QMenu *edit_menu = menuBar()->addMenu(tr("&Setting"));
  edit_menu->addAction(tr("&Purrmitive Parameters"), this,
                       &MainWindow::openSetting);
}

void MainWindow::open() {
  QFileDialog dialog(this, tr("Open File"));
  initImageFileDialog(dialog, QFileDialog::AcceptOpen);

  while (dialog.exec() == QDialog::Accepted &&
         !loadImage(dialog.selectedFiles().first())) {
  }

  _setting_dialog->updateImage(_image);
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
