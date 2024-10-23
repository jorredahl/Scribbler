#ifndef SCRIBBLER_H
#define SCRIBBLER_H

#include <QGraphicsView>

class MouseEvent {
public:
    enum {
        Press,
        Move,
        Release
    };

    int action;
    QPointF pos;
    quint64 time;
    QGraphicsEllipseItem* dot;
    QGraphicsLineItem* line;

    MouseEvent();
    MouseEvent(int _action, QPointF _pos, quint64 _time);

    void addDot(QGraphicsEllipseItem* newDot);
    void addLine(QGraphicsLineItem* newLine);

    friend QDataStream &operator<<(QDataStream &out, const MouseEvent &evt);
    friend QDataStream &operator>>(QDataStream &in, MouseEvent &evt);
};

class Scribbler : public QGraphicsView
{  
    QGraphicsScene scene;
    double lineWidth;
    QPointF lastPoint;

    bool capture;
    QList<MouseEvent> events;
    int tabSelected;

    QList<QGraphicsLineItem *> lines;
    QList<QGraphicsEllipseItem *> dots;
    QList<QGraphicsItem *> curItems;
    QList<QGraphicsItem *> uncItems;

    QList<QList<QGraphicsItem *>> totalItems;

    Q_OBJECT

public:
    Scribbler();

    void linesVisible(bool visible);
    void flipCapture(bool cap);
    void resetScribbles();
    QList<MouseEvent> returnEvents();
    int returnTabSelected();
    double returnLineWidth();
    QGraphicsEllipseItem* addDot(QPointF p);
    QGraphicsLineItem* addLine(QPointF p0, QPointF p1);

public slots:
    void resetEvents();
    void tabSelectedSlot(int index);

protected:
    void mouseMoveEvent(QMouseEvent *evt) override;
    void mousePressEvent(QMouseEvent *evt) override;
    void mouseReleaseEvent(QMouseEvent *evt) override;
};

#endif // SCRIBBLER_H
