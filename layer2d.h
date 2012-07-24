#ifndef LAYER2D_H
#define LAYER2D_H

#include <QGraphicsObject>
#include <QPainter>

class Layer2D : public QGraphicsObject
{
    Q_OBJECT
private:
    QList<QPointF*> printedPoints;
public:
    explicit Layer2D();
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    bool showLayer;
signals:

public slots:
    void addPrintedPoint(QPointF);
    void resetLayer();
    void setShow(bool);
};

#endif // LAYER2D_H
