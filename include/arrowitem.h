#ifndef ARROWITEM_H
#define ARROWITEM_H

#include <QGraphicsItem>

#include <QGraphicsTextItem>

/// @brief Class that rapresents an Arrow in the GUI
class ArrowItem : public QGraphicsItem
{
public:
    ArrowItem(QGraphicsItem *startItem, QGraphicsItem *endItem, QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    QPainterPath shape() const override;
    bool isMyStartOrEnd(QGraphicsEllipseItem *check);
    bool operator==(const ArrowItem &other) const;
    void handleClicked();
    QGraphicsItem *startItem;
    QGraphicsItem *endItem;
    QPointF controlPoint;
    QList<QPointF> points;
    QGraphicsTextItem *textItem;

protected:
    void updateTextPosition();
    QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) override;

private:
    QRectF calculateArrowRect() const;
};

#endif // ARROWITEM_H
