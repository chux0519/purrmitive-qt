#include <QApplication>

#include "main_window.h"

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  QCoreApplication::setOrganizationName("Hexyoungs");
  QCoreApplication::setApplicationName("Purrmitive");
  QCoreApplication::setApplicationVersion("1.0.0");
  registerPurrmitiveMetaTypes();

  MainWindow mainWin;

  mainWin.show();
  return app.exec();
}
