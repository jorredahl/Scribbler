#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "scribbler.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

    QString lastDir;

    Scribbler *scribbler;
    QTabWidget *tabs;
    int tabCount;
    QList<QList<MouseEvent>> dataHistory;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void resetScribbleSlot();
    void openFileSlot();
    void saveFileSlot();
    void startCaptureSlot();
    void endCaptureSlot();
    void lineSegmentsSlot();
    void dotsOnlySlot();

signals:
    void clearEvents();

};
#endif // MAINWINDOW_H
