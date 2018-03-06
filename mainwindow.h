#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QIODevice>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QTime>
#include <QMessageBox>
#include <QFileDialog>
#include <QProcess>
#include <qwt_plot_curve.h>
#include <qwt_abstract_scale.h>
#include <qwt_abstract_scale_draw.h>
#include <qwt_plot_grid.h>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QTimer *timer1;
    QTimer *timer2;
    QTimer *timer3;
    QwtPlotCurve *curve1;
    QwtPlotCurve *curve2;
    QwtPlotCurve *curve3;
    QwtPlotGrid *grid1;
    QwtPlotGrid *grid2;
public slots:
    void ShowProcessInfo(void);
    void ShowSystemRunningTime(void);
    void SearchProcess(void);
    void KillProcess(void);
    void NextProcess(void);
    void ShowCpuUsage(void);
    void ShowMemorySwapUsage(void);
    void RunNewProcess(void);
    void ShutDown(void);

private:
    Ui::MainWindow *ui;
    void ShowHostName(void);
    void ShowSystemStartUpTime(void);
    void ShowSystemVersion(void);
    void ShowCpu(void);
    void PastePresentTime(void);
    void PastePresentCpuUsage(void);
    void PasteMemoryUsage(void);
};

class ProcInfo{
public:
    QString procName;
    QString procPID;
    QString procPPID;
    QString procMemoryUse;
    QString procPriority;
    int indexInList;
};

#endif // MAINWINDOW_H
