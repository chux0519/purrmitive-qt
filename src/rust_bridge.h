#pragma once

#include <QDebug>
#include <QMetaType>
#include <QObject>
#include <QString>
#include <QThread>
#include <atomic>

#include "purr/purrmitive-ffi/target/libpurrmitive_ffi.h"

struct StopCond {
  double stopScore = 1.0;
  int stopShapes = 65535;
  bool noStop = true;
};

Q_DECLARE_METATYPE(PurrmitiveColor);

static void registerPurrmitiveMetaTypes() {
  qRegisterMetaType<PurrmitiveColor>("PurrmitiveColor");
  qRegisterMetaType<PurrmitiveContextInfo>("PurrmitiveContextInfo");
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
    PurrmitiveContextInfo ctx_info = purrmitive_get_ctx_info();

    emit initResultReady(bg, ctx_info);
  }

  void step() {
    qDebug() << "step";
    purrmitive_step();
    char *svg_bytes = purrmitive_get_last_shape();
    _svg = QString::fromUtf8(svg_bytes);
    purrmitive_free_str(svg_bytes);
    PurrmitiveContextInfo ctx_info = purrmitive_get_ctx_info();

    emit stepResultReady(_svg, ctx_info);
  }

  void stop() { purrmitive_stop(); }

 signals:
  void initResultReady(const PurrmitiveColor &bg,
                       const PurrmitiveContextInfo &info);
  void stepResultReady(const QString &svg, const PurrmitiveContextInfo &info);

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

  double getScore() { return _score; }
  int getStep() { return _step; }

  void doStartOrStep(const PurrmitiveParam *param) {
    if (_running) {
      emit step();
      _step++;
    } else {
      emit init(param);
      _running = true;
      _step = 0;
    }
  }

  void doStop() {
    emit stop();
    _running = false;
    _step = -1;
    _score = 0.0;
  }

 public slots:  // worker -> controller
  void handleInitResult(const PurrmitiveColor &bg,
                        const PurrmitiveContextInfo &info) {
    qDebug() << "(" << bg.r << "," << bg.g << "," << bg.b << "," << bg.a << ")";
    _score = 1.0 - info.score;
    emit onBgReceived(bg, info);
  }
  void handleStepResult(const QString &svg, const PurrmitiveContextInfo &info) {
    qDebug() << "handleStepResult";
    _score = 1.0 - info.score;
    // for UI, display score, step and preview
    emit onStepResultReceived(svg, info);
  }

 signals:  // UI -> controller -> worker
  void init(const PurrmitiveParam *param);
  void step();
  void stop();

  // controller -> UI
  void onBgReceived(const PurrmitiveColor &bg,
                    const PurrmitiveContextInfo &info);
  void onStepResultReceived(const QString &svg,
                            const PurrmitiveContextInfo &info);

 private:
  bool _running = false;
  int _step = -1;
  double _score = 0.0;
};
