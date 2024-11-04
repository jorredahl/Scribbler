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

    connect(tabs, &QTabWidget::currentChanged, scribbler, &Scribbler::tabSelectedSlot);

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

    endCaptureAct = new QAction("&End Capture");
    connect(endCaptureAct, &QAction::triggered, this, &MainWindow::endCaptureSlot);
    endCaptureAct->setEnabled(false);
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

    QSettings settings("FJS Systems", "Scribbler");
    lastDir = settings.value("lastDir", "").toString();
}

MainWindow::~MainWindow() {
    QSettings settings("FJS Systems", "Scribbler");
    settings.setValue("lastDir", lastDir);
}

void MainWindow::resetScribbleSlot() {
    tabCount = 0;
    scribbler->resetScribbles();
    tables.clear();
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
        emit clearEvents();
        scribbler->flipCapture(true);
        for(int j = 0; j < dataHistory[i].length(); ++j) {
            if (dataHistory[i][j].action == MouseEvent::Press) {
                dataHistory[i][j].addDot(scribbler->addDot(dataHistory[i][j].pos));
                dataHistory[i][j].addLine(nullptr);
            } else if (dataHistory[i][j].action == MouseEvent::Move) {
                dataHistory[i][j].addDot(scribbler->addDot(dataHistory[i][j].pos));
                dataHistory[i][j].addLine(scribbler->addLine(dataHistory[i][j - 1].pos, dataHistory[i][j].pos));
            } else {
                dataHistory[i][j].addDot(nullptr);
                dataHistory[i][j].addLine(nullptr);
            }
        }
        emit clearEvents();
        scribbler->flipCapture(false);


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
        tables.append(table);

        connect(table, &QTableWidget::itemSelectionChanged, this, &MainWindow::highlightItem);

        ++tabCount;
        tabs->addTab(table, "Table " + QString::number(tabCount));
        tabs->setVisible(true);
        emit tabs->currentChanged(0);
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
    emit clearEvents();
    scribbler->flipCapture(true);
    endCaptureAct->setEnabled(true);
}

void MainWindow::endCaptureSlot() {
    QList<MouseEvent> events = scribbler->returnEvents();
    dataHistory << events;
    emit clearEvents();
    scribbler->flipCapture(false);

    QTableWidget *table = new QTableWidget();
    table->setRowCount(events.length());
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels(QStringList() << "Mouse Event" << "Position" << "Timestamp" << "Distance" << "Speed");
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
        QTableWidgetItem *item4;
        QTableWidgetItem *item5;
        if (events[iRow].action == 0 || events[iRow].action == 2) {
            item4 = new QTableWidgetItem("N/A");
            item5 = new QTableWidgetItem("N/A");
        } else {
            item4 = new QTableWidgetItem(QString::number(sqrt(pow(events[iRow - 1].pos.x() - events[iRow].pos.x(), 2) + pow(events[iRow - 1].pos.y() - events[iRow].pos.y(), 2))));
            item5 = new QTableWidgetItem(QString::number(sqrt(pow(events[iRow - 1].pos.x() - events[iRow].pos.x(), 2) + pow(events[iRow - 1].pos.y() - events[iRow].pos.y(), 2)) / (events[iRow].time - events[iRow - 1].time)));
        }
        table->setItem(iRow, 3, item4);
        table->setItem(iRow, 4, item5);
    }
    tables.append(table);

    connect(table, &QTableWidget::itemSelectionChanged, this, &MainWindow::highlightItem);

    ++tabCount;
    tabs->addTab(table, "Table " + QString::number(tabCount));
    tabs->setVisible(true);
    endCaptureAct->setEnabled(false);
}

void MainWindow::lineSegmentsSlot() {
    scribbler->linesVisible(true);
}

void MainWindow::dotsOnlySlot() {
    scribbler->linesVisible(false);
}

void MainWindow::highlightItem() {
    QList<int> rowNums;
    for (int i = 0; i < tables[scribbler->returnTabSelected()]->selectedItems().length(); ++i) {
        rowNums.append(tables[scribbler->returnTabSelected()]->selectedItems()[i]->row());
    }
    for (int i = 0; i < dataHistory.length(); ++i) {
        for (int j = 0; j < dataHistory[i].length(); ++j) {
            if (tabs->currentWidget() == tables[i] && rowNums.contains(j)) {
                if (dataHistory[i][j].dot != nullptr) {
                    dataHistory[i][j].dot->setBrush(QColor(Qt::red));
                    dataHistory[i][j].dot->setZValue(INFINITY);
                }
                if (dataHistory[i][j].line != nullptr) {
                    dataHistory[i][j].line->setPen(QPen(Qt::red, scribbler->returnLineWidth(), Qt::SolidLine, Qt::FlatCap));
                    dataHistory[i][j].line->setZValue(INFINITY);
                }
            } else {
                if (dataHistory[i][j].dot != nullptr) {
                    dataHistory[i][j].dot->setBrush(QColor(Qt::black));
                    dataHistory[i][j].dot->setZValue(0);
                }
                if (dataHistory[i][j].line != nullptr) {
                    dataHistory[i][j].line->setPen(QPen(Qt::black, scribbler->returnLineWidth(), Qt::SolidLine, Qt::FlatCap));
                    dataHistory[i][j].line->setZValue(0);
                }
            }
        }
    }
}
