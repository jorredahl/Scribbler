#include "scribbler.h"

#include <QtWidgets>

MouseEvent::MouseEvent() {}

MouseEvent::MouseEvent(const MouseEvent &me) {

}

MouseEvent::MouseEvent(int _action, QPointF _pos, quint64 _time)
    :action(_action),pos(_pos),time(_time) { }

QDataStream &operator<<(QDataStream &out, const MouseEvent &evt) {
    return out << evt.action << evt.pos << evt.time;
}

QDataStream &operator>>(QDataStream &in, MouseEvent &evt) {
    return in >> evt.action >> evt.pos >> evt.time;
}

Scribbler::Scribbler()
    :lineWidth(4.0){
    setScene(&scene);
    setSceneRect(QRectF(0.0, 0.0, 800.0, 600.0));
    setMinimumSize(QSize(800, 600));
    setRenderHint(QPainter::Antialiasing, true);

    scene.addRect(sceneRect());
}

void Scribbler::linesVisible(bool visible) {
    for (int i = 0; i < lines.length(); ++i) {
        lines.at(i)->setVisible(visible);
    }
}

void Scribbler::resetScribbles() {
    events.clear();
    for (int i = 0; i < lines.length(); ++i) scene.removeItem(lines.at(i));
    for (int i = 0; i < dots.length(); ++i) scene.removeItem(dots.at(i));
    lines.clear();
    dots.clear();
}

QList<MouseEvent> Scribbler::returnEvents() {
    return events;
}

void Scribbler::addDot(QPointF p) {
    QGraphicsEllipseItem *tempDot;
    tempDot = scene.addEllipse(QRectF(p - QPointF(lineWidth * 0.5, lineWidth * 0.5), QSizeF(lineWidth, lineWidth)), Qt::NoPen, Qt::black);

    dots.append(tempDot);

    events << MouseEvent(MouseEvent::Press, p, 0);
}

void Scribbler::addLine(QPointF p0 , QPointF p1) {
    QGraphicsLineItem *tempLine;
    tempLine = scene.addLine(QLineF(p0, p1), QPen(Qt::black, lineWidth, Qt::SolidLine, Qt::FlatCap));

    lines.append(tempLine);


    events << MouseEvent(MouseEvent::Move, p1, 0);
}

void Scribbler::mousePressEvent(QMouseEvent *evt) {
    QGraphicsView::mousePressEvent(evt);

    QPointF p = mapToScene(evt->pos());
    lastPoint = p;

    QGraphicsEllipseItem *tempDot;
    tempDot = scene.addEllipse(QRectF(p - QPointF(lineWidth * 0.5, lineWidth * 0.5), QSizeF(lineWidth, lineWidth)), Qt::NoPen, Qt::black);

    dots.append(tempDot);

    events << MouseEvent(MouseEvent::Press, p, evt->timestamp());
}

void Scribbler::mouseMoveEvent(QMouseEvent *evt) {
    QGraphicsView::mouseMoveEvent(evt);

    QPointF p = mapToScene(evt->pos());

    QGraphicsLineItem *tempLine;
    QGraphicsEllipseItem *tempDot;
    tempLine = scene.addLine(QLineF(lastPoint, p), QPen(Qt::black, lineWidth, Qt::SolidLine, Qt::FlatCap));
    tempDot = scene.addEllipse(QRectF(p - QPointF(lineWidth * 0.5, lineWidth * 0.5), QSizeF(lineWidth, lineWidth)), Qt::NoPen, Qt::black);

    lines.append(tempLine);
    dots.append(tempDot);

    lastPoint = p;

    events << MouseEvent(MouseEvent::Move, p, evt->timestamp());
}

void Scribbler::mouseReleaseEvent(QMouseEvent *evt) {
    QGraphicsView::mouseReleaseEvent(evt);

    QPointF p = mapToScene(evt->pos());

    events << MouseEvent(MouseEvent::Release, p, evt->timestamp());

    // qDebug() << events;
}
