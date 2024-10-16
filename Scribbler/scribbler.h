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
    MouseEvent(const MouseEvent&me);
    MouseEvent(int _action, QPointF _pos, quint64 _time);

    friend QDataStream &operator<<(QDataStream &out, const MouseEvent &evt);
    friend QDataStream &operator>>(QDataStream &in, MouseEvent &evt);
};

class Scribbler : public QGraphicsView
{  
    QGraphicsScene scene;
    double lineWidth;
    QPointF lastPoint;

    QList<MouseEvent> events;

    QList<QGraphicsLineItem *> lines;
    QList<QGraphicsEllipseItem *> dots;

    Q_OBJECT
public:
    Scribbler();

    void linesVisible(bool visible);
    void resetScribbles();
    QList<MouseEvent> returnEvents();
    void addDot(QPointF p);
    void addLine(QPointF p0, QPointF p1);

protected:
    void mouseMoveEvent(QMouseEvent *evt) override;
    void mousePressEvent(QMouseEvent *evt) override;
    void mouseReleaseEvent(QMouseEvent *evt) override;
};

#endif // SCRIBBLER_H
