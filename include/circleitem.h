#include <QGraphicsEllipseItem>
#include <QBrush>
#include <QPen>
#include <QGraphicsItemGroup>
#include <QPointF>

#define CIRCLEITEM_TYPE (QGraphicsItem::UserType + 1)

#ifndef CIRCLEITEM_H
#define CIRCLEITEM_H

class CircleItem : public QGraphicsItemGroup{
public:
    CircleItem(QGraphicsEllipseItem* ellipse, QGraphicsTextItem *textItem) {
        // Retrieve properties from the ellipse

        addToGroup(ellipse);
        addToGroup(textItem);
        this->ellipse = ellipse;
        this->textItem = textItem;
        position = ellipse->sceneBoundingRect().center();
        size = ellipse->rect().size();
        pen = ellipse->pen();
        brush = ellipse->brush();
    }

    // Override type() function to return a unique value for CircleItem
    int type() const override {
        return CIRCLEITEM_TYPE;
    }


    // Comparison operator to check equality
    bool operator==(const CircleItem& other) const {
        return ellipse->sceneBoundingRect().center() == other.ellipse->sceneBoundingRect().center() &&
               ellipse->rect().size() == other.ellipse->rect().size() &&
               ellipse->pen() == other.ellipse->pen() &&
               ellipse->brush() == other.ellipse->brush();
    }

    // Overload the << operator for qDebug()
    friend QDebug operator<<(QDebug dbg, const CircleItem& circleItem) {
        dbg.nospace() << "CircleItem: "
                      << "Position=" << circleItem.ellipse->sceneBoundingRect().center() << "\n";
        return dbg.space();
    }
    QPointF position;
    QSizeF size;
    QPen pen;
    QBrush brush;
    QGraphicsEllipseItem* ellipse;
    QGraphicsTextItem* textItem;
};


#endif // CIRCLEITEM_H
