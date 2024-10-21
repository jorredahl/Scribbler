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

    MouseEvent();
    MouseEvent(int _action, QPointF _pos, quint64 _time);

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

    QList<QGraphicsLineItem *> lines;
    QList<QGraphicsEllipseItem *> dots;

    Q_OBJECT
public:
    Scribbler();

    void linesVisible(bool visible);
    void flipCapture(bool cap);
    void resetScribbles();
    QList<MouseEvent> returnEvents();
    void addDot(QPointF p);
    void addLine(QPointF p0, QPointF p1);

public slots:
    void resetEvents();

protected:
    void mouseMoveEvent(QMouseEvent *evt) override;
    void mousePressEvent(QMouseEvent *evt) override;
    void mouseReleaseEvent(QMouseEvent *evt) override;
};

#endif // SCRIBBLER_H
