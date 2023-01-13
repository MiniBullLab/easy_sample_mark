#ifndef CAMERAINTRINSICSTHREAD_H
#define CAMERAINTRINSICSTHREAD_H

#include <QThread>
#include <QString>

class CameraIntrinsicsThread: public QThread
{
    Q_OBJECT
public:
    CameraIntrinsicsThread();
    ~CameraIntrinsicsThread();

signals:
    void signalFinish(QString name);

public slots:

protected:
    void run();

private:
    bool isStart;//是否允许线程

};

#endif // CAMERAINTRINSICSTHREAD_H
