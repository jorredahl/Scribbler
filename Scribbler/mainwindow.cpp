#include "mainwindow.h"

#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QWidget *center = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout();
    scribbler = new Scribbler();
    tabCount = 0;
    tabs = new QTabWidget();
    tabs->setVisible(false);
    center->setLayout(layout);
    layout->addWidget(scribbler, 1);
    layout->addWidget(tabs, 1);

    connect(this, &MainWindow::clearEvents, scribbler, &Scribbler::resetEvents);

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

    setCentralWidget(center);

    QSettings settings("FJS Systems", "Graphics1");
    lastDir = settings.value("lastDir", "").toString();
}

MainWindow::~MainWindow() {
    QSettings settings("FJS Systems", "Graphics1");
    settings.setValue("lastDir", lastDir);
}

void MainWindow::resetScribbleSlot() {
    tabCount = 0;
    scribbler->resetScribbles();
    dataHistory.clear();
    tabs->clear();
    tabs->setVisible(false);
}

void MainWindow::openFileSlot() {
    resetScribbleSlot();

    QString inFileName = QFileDialog::getOpenFileName(this, "Select Scribble File",  lastDir);
    QFile in(inFileName);
    if(!in.open(QIODevice::ReadOnly)) {
        QMessageBox::information(this, "Error", QString("Can't open file \"%1\"").arg(inFileName));
        return;
    }
    QDataStream data(&in);
    data >> dataHistory;

    for(int i = 0; i < dataHistory.length(); ++i) {
        for(int j = 0; j < dataHistory[i].length(); ++j) {
            if (dataHistory[i][j].action == MouseEvent::Press) {
                scribbler->addDot(dataHistory[i][j].pos);
            } else if (dataHistory[i][j].action == MouseEvent::Move) {
                scribbler->addDot(dataHistory[i][j].pos);
                scribbler->addLine(dataHistory[i][j - 1].pos, dataHistory[i][j].pos);
            }
        }

        QTableWidget *table = new QTableWidget();
        table->setRowCount(dataHistory[i].length());
        table->setColumnCount(3);
        table->setHorizontalHeaderLabels(QStringList() << "Mouse Event" << "Position" << "Timestamp");
        for (int row = 0; row < dataHistory[i].length(); ++row) {
            QTableWidgetItem *item1;
            if (dataHistory[i][row].action == 0) {
                item1 = new QTableWidgetItem("Press");
            } else if (dataHistory[i][row].action == 1) {
                item1 = new QTableWidgetItem("Move");
            } else {
                item1 = new QTableWidgetItem("Release");
            }
            table->setItem(row, 0, item1);
            QTableWidgetItem *item2 = new QTableWidgetItem(QString("(%1, %2)").arg(dataHistory[i][row].pos.x()).arg(dataHistory[i][row].pos.y()));
            table->setItem(row, 1, item2);
            QTableWidgetItem *item3 = new QTableWidgetItem(QString::number(dataHistory[i][row].time));
            table->setItem(row, 2, item3);
        }

        ++tabCount;
        tabs->addTab(table, "Table " + QString::number(tabCount));
        tabs->setVisible(true);
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
    out << dataHistory;
}

void MainWindow::startCaptureSlot() {
    scribbler->flipCapture(true);
    emit clearEvents();
}

void MainWindow::endCaptureSlot() {
    scribbler->flipCapture(false);
    QList<MouseEvent> events = scribbler->returnEvents();
    dataHistory << events;
    emit clearEvents();

    QTableWidget *table = new QTableWidget();
    table->setRowCount(events.length());
    table->setColumnCount(3);
    table->setHorizontalHeaderLabels(QStringList() << "Mouse Event" << "Position" << "Timestamp");
    for (int iRow = 0; iRow < events.length(); ++iRow) {
        QTableWidgetItem *item1;
        if (events[iRow].action == 0) {
            item1 = new QTableWidgetItem("Press");
        } else if (events[iRow].action == 1) {
            item1 = new QTableWidgetItem("Move");
        } else {
            item1 = new QTableWidgetItem("Release");
        }
        table->setItem(iRow, 0, item1);
        QTableWidgetItem *item2 = new QTableWidgetItem(QString("(%1, %2)").arg(events[iRow].pos.x()).arg(events[iRow].pos.y()));
        table->setItem(iRow, 1, item2);
        QTableWidgetItem *item3 = new QTableWidgetItem(QString::number(events[iRow].time));
        table->setItem(iRow, 2, item3);
    }

    ++tabCount;
    tabs->addTab(table, "Table " + QString::number(tabCount));
    tabs->setVisible(true);

}

void MainWindow::lineSegmentsSlot() {
    scribbler->linesVisible(true);
}

void MainWindow::dotsOnlySlot() {
    scribbler->linesVisible(false);
}
