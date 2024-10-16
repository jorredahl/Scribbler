#include "mainwindow.h"

#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QHBoxLayout *center = new QHBoxLayout();
    scribbler = new Scribbler();
    tabCount = 0;
    tabs = new QTabWidget();
    for (int i = 0; i < tabs->count(); ++i) tabs->setTabVisible(i, false);
    tabs->setTabVisible(0, false);
    center->addWidget(scribbler);
    center->addWidget(tabs);

    QAction *resetScribbleAct = new QAction("&Reset Scribble");
    connect(resetScribbleAct, &QAction::triggered, this, &MainWindow::resetScribbleSlot);
    resetScribbleAct->setShortcut(Qt::CTRL | Qt::Key_R);

    QAction *openFileAct = new QAction("&Open File");
    connect(openFileAct, &QAction::triggered, this, &MainWindow::openFileSlot);
    openFileAct->setShortcut(Qt::CTRL | Qt::Key_O);

    QAction *saveFileAct = new QAction("&Save File");
    connect(saveFileAct, &QAction::triggered, this, &MainWindow::saveFileSlot);
    saveFileAct->setShortcut(Qt::CTRL | Qt::Key_S);

    QMenu *fileMenu = new QMenu("&File");
    fileMenu->addAction(resetScribbleAct);
    fileMenu->addAction(openFileAct);
    fileMenu->addAction(saveFileAct);

    QAction *startCaptureAct = new QAction("&Start Capture");
    connect(startCaptureAct, &QAction::triggered, this, &MainWindow::startCaptureSlot);
    startCaptureAct->setShortcut(Qt::CTRL | Qt::Key_B); //B for Begin :)

    QAction *endCaptureAct = new QAction("&End Capture");
    connect(endCaptureAct, &QAction::triggered, this, &MainWindow::endCaptureSlot);
    endCaptureAct->setShortcut(Qt::CTRL | Qt::Key_E);

    QMenu *captureMenu = new QMenu("&Capture");
    captureMenu->addAction(startCaptureAct);
    captureMenu->addAction(endCaptureAct);

    QAction *lineSegmentsAct = new QAction("&Line Segments");
    connect(lineSegmentsAct, &QAction::triggered, this, &MainWindow::lineSegmentsSlot);
    lineSegmentsAct->setShortcut(Qt::CTRL | Qt::Key_L);

    QAction *dotsOnlyAct = new QAction("&Dots Only");
    connect(dotsOnlyAct, &QAction::triggered, this, &MainWindow::dotsOnlySlot);
    dotsOnlyAct->setShortcut(Qt::CTRL | Qt::Key_D);

    QMenu *viewMenu = new QMenu("&View");
    viewMenu->addAction(lineSegmentsAct);
    viewMenu->addAction(dotsOnlyAct);

    menuBar()->addMenu(fileMenu);
    menuBar()->addMenu(captureMenu);
    menuBar()->addMenu(viewMenu);

    setCentralWidget(scribbler);
}

MainWindow::~MainWindow() {}

void MainWindow::resetScribbleSlot() {
    for (int i = 0; i < tabs->count(); ++i) tabs->removeTab(i);
    tabCount = 0;
    scribbler->resetScribbles();
}

// This method does not work
void MainWindow::openFileSlot() {
    resetScribbleSlot();

    QString inFileName = QFileDialog::getOpenFileName();
    QFile in(inFileName);
    if(!in.open(QIODevice::ReadOnly)) {
        QMessageBox::information(this, "Error", QString("Can't open file \"%1\"").arg(inFileName));
        return;
    }
    QDataStream data(&in);
    qsizetype size = 0;
    data >> size;
    QList<MouseEvent> events = QList<MouseEvent>(size);
    for(int i = 0; i < size; ++i) data >> events[i];

    for(int i = 0; i < size; ++i) {
        if (events[i].action == MouseEvent::Move) {
            scribbler->addDot(events[i].pos);
        } else if (events[i].action == MouseEvent::Press) {
            scribbler->addDot(events[i].pos);
            scribbler->addLine(events[i - 1].pos, events[i].pos);
        }
    }
}

void MainWindow::saveFileSlot() {
    QString fileName = QFileDialog::getSaveFileName();
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::information(this, "Error", QString("Can't write to file \"%1\"").arg(fileName));
        return;
    }
    QDataStream out(&file);
    QList<MouseEvent> events = scribbler->returnEvents();
    out << events.length();
    for(int i = 0; i < events.length(); ++i) out << events.at(i);
}

void MainWindow::startCaptureSlot() {

}

void MainWindow::endCaptureSlot() {
    QTableWidget *table = new QTableWidget();
    tabs->addTab(table, "Table");
    ++tabCount;
    for (int i = 0; i < tabs->count(); ++i) tabs->setTabVisible(i, true);
}

void MainWindow::lineSegmentsSlot() {
    scribbler->linesVisible(true);
}

void MainWindow::dotsOnlySlot() {
    scribbler->linesVisible(false);
}
