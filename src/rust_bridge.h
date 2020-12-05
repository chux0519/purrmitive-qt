#pragma once

#include <QDebug>
#include <QMetaType>
#include <QObject>
#include <QString>
#include <QThread>

#include "purr-binding/target/libpurrmitive.h"

Q_DECLARE_METATYPE(PurrmitiveColor);

static void registerPurrmitiveMetaTypes() {
  qRegisterMetaType<PurrmitiveColor>("PurrmitiveColor");
}

class PurrmitiveWorker : public QObject {
  Q_OBJECT
 public:
  PurrmitiveWorker() {}
  ~PurrmitiveWorker() {}
 public slots:
  void init(const PurrmitiveParam *param) {
    qDebug() << "init: " << param->input;
    purrmitive_init(param);
    PurrmitiveColor bg = purrmitive_get_bg();
    // PurrmitiveColor bg;
    emit initResultReady(bg);
  }

  void step() {
    qDebug() << "step";
    purrmitive_step();
    char *svg_bytes = purrmitive_get_last_shape();
    _svg = QString::fromUtf8(svg_bytes);
    purrmitive_free_str(svg_bytes);
    emit stepResultReady(_svg);
  }
  void stop() {}

 signals:
  void initResultReady(const PurrmitiveColor &bg);
  void stepResultReady(const QString &svg);

 private:
  QString _svg;
};

class PurrmitiveController : public QObject {
  Q_OBJECT
  QThread thread;

 public:
  PurrmitiveController() {
    PurrmitiveWorker *worker = new PurrmitiveWorker;
    worker->moveToThread(&thread);
    connect(&thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(this, &PurrmitiveController::init, worker, &PurrmitiveWorker::init);
    connect(this, &PurrmitiveController::step, worker, &PurrmitiveWorker::step);
    connect(this, &PurrmitiveController::stop, worker, &PurrmitiveWorker::stop);
    connect(worker, &PurrmitiveWorker::initResultReady, this,
            &PurrmitiveController::handleInitResult);
    connect(worker, &PurrmitiveWorker::stepResultReady, this,
            &PurrmitiveController::handleStepResult);
    purrmitive_set_verbose(1);
    thread.start();
  }
  ~PurrmitiveController() {
    stop();
    thread.quit();
    thread.wait();
  }

 public slots:
  void handleInitResult(const PurrmitiveColor &bg) {
    qDebug() << "(" << bg.r << "," << bg.g << "," << bg.b << "," << bg.a << ")";
  }
  void handleStepResult(const QString &svg) { qDebug() << "handleStepResult"; }

 signals:
  void init(const PurrmitiveParam *param);
  void step();
  void stop();
};
