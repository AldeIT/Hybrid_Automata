#include "../include/arrowitem.h"
#include <QStyleOptionGraphicsItem> // Include the header for QStyleOptionGraphicsItem
#include <QPainter>
#include <QStyle>
#include <QPainterPath>
#include <QGraphicsSceneMouseEvent>

/// @brief Constructs an ArrowItem connecting two QGraphicsItem
/// @param startItem The QGraphicsItem this arrow starts from.
/// @param endItem The QGraphicsItem this arrow ends at.
/// @param parent The parent QGraphicsItem of this ArrowItem.
ArrowItem::ArrowItem(QGraphicsItem *startItem, QGraphicsItem *endItem, QGraphicsItem *parent)
    : QGraphicsItem(parent), startItem(startItem), endItem(endItem)
{
    setFlag(ItemSendsGeometryChanges);
    textItem = new QGraphicsTextItem("Arrow", this);
    textItem->setDefaultTextColor(Qt::black);
}

/// @brief Calculates the bounding rectangle for the arrow.
/// @note If either startItem or endItem is nullptr, the method returns an empty QRectF.
QRectF ArrowItem::calculateArrowRect() const
{
    if (!startItem || !endItem)
    {
        // delete this;
        return QRectF();
    }
    qreal penWidth = 1.0; // Adjust pen width as needed
    qreal extra = 10;     // Additional space for the arrow heads
    return QRectF(startItem->sceneBoundingRect().center(),
                  QSizeF(endItem->sceneBoundingRect().center().x() - startItem->sceneBoundingRect().center().x(),
                         endItem->sceneBoundingRect().center().y() - startItem->sceneBoundingRect().center().y()))
        .normalized()
        .adjusted(-extra, -extra, extra, extra);
}

/// @brief Updates the position of the text associated with the arrow.
/// @note The text position is determined based on the center points of the start and end items, along with the angle between them.
void ArrowItem::updateTextPosition()
{
    // Adjust the offset as needed
    QPointF startHomemade(startItem->sceneBoundingRect().center().x(), startItem->sceneBoundingRect().center().y());
    QPointF endHomemade(endItem->sceneBoundingRect().center().x(), endItem->sceneBoundingRect().center().y());
    qreal angle = atan2(startHomemade.y() - endHomemade.y(), startHomemade.x() - endHomemade.x()) + 0.5;
    qreal angle_start = atan2(endHomemade.y() - startHomemade.y(), endHomemade.x() - startHomemade.x()) - 0.5;
    qreal endItemRadius = qMin(endItem->boundingRect().width(), endItem->boundingRect().height()) / 2.0;
    QPointF stopLine = endHomemade + QPointF(endItemRadius * cos(angle), endItemRadius * sin(angle));
    QPointF startLine = startHomemade + QPointF(endItemRadius * cos(angle_start), endItemRadius * sin(angle_start));
    QPointF arrowCenter = (startLine + stopLine) / 2;
    QPointF textOffset(0.5, 0.5); // Adjust the offset as needed
    textItem->setPos(arrowCenter + textOffset);
}

/// @brief Handles changes to the arrow item.
/// @param change The type of change that has occurred.
/// @param value The new value associated with the change.
/// @return The updated value after processing the change.
QVariant ArrowItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if (!textItem || !change)
    {
        QVariant base;
        return base;
    }
    if (change == ItemPositionHasChanged)
    {
        // Update text position whenever the arrow item moves
        updateTextPosition();
    }
    return QGraphicsItem::itemChange(change, value);
}

/// @brief Returns the bounding rectangle of the arrow item.
/// @return The bounding rectangle of the arrow item.
QRectF ArrowItem::boundingRect() const
{
    if (!startItem || !endItem)
    {
        // delete this;
        return QRectF();
    }
    // Calculate bounding rectangle including both arrow and text
    QRectF arrowRect = calculateArrowRect();
    QRectF textRect = textItem->boundingRect();
    return arrowRect.united(textRect);
}

/// @brief Returns the shape of the arrow item.
/// @return The shape of the arrow item as a QPainterPath.
QPainterPath ArrowItem::shape() const
{

    QRectF boundingRect = textItem->mapToParent(textItem->boundingRect()).boundingRect();
    qreal padding = 5.0; // Adjust padding as needed
    boundingRect.adjust(-padding, -padding, padding, padding);
    QPainterPath path;
    path.addRect(boundingRect);
    return path;
}
/// @brief Paints the arrow item. Method Called by QT.
/// @param painter The QPainter object used for painting.
/// @param option unused parameter
/// @param widget unused parameter
void ArrowItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (!startItem || !endItem)
    {
        return;
    }
    bool adjustText = false;
    bool isSelected = option->state & QStyle::State_Selected;

    QPen pen(isSelected ? Qt::red : Qt::black);
    pen.setWidth(2);
    QPointF startHomemade(startItem->sceneBoundingRect().center().x(), startItem->sceneBoundingRect().center().y());
    QPointF endHomemade(endItem->sceneBoundingRect().center().x(), endItem->sceneBoundingRect().center().y());

    painter->setPen(pen);

    qreal arrowSize = 20;
    qreal angle = atan2(startHomemade.y() - endHomemade.y(), startHomemade.x() - endHomemade.x()) + 0.5;
    qreal angle_start = atan2(endHomemade.y() - startHomemade.y(), endHomemade.x() - startHomemade.x()) - 0.5;

    qreal endItemRadius = qMin(endItem->boundingRect().width(), endItem->boundingRect().height()) / 2.0;

    QPointF stopLine = endHomemade + QPointF(endItemRadius * cos(angle), endItemRadius * sin(angle));
    QPointF startLine = startHomemade + QPointF(endItemRadius * cos(angle_start), endItemRadius * sin(angle_start));

    QPointF arrowP1 = QPointF(stopLine.x() + 10 * cos(angle), stopLine.y() + 10 * sin(angle));
    QPointF arrowP2 = QPointF(stopLine.x() + 10 * cos(angle - 1), stopLine.y() + 10 * sin(angle - 1));

    controlPoint = QPointF();

    if (abs(startLine.x() - stopLine.x()) < 80)
    {
        if (startLine.y() - stopLine.y() < 0)
        {
            controlPoint = QPointF(((startLine.x() + stopLine.x()) / 2) + 80, ((startLine.y() + stopLine.y()) / 2));
        }
        else
        {
            controlPoint = QPointF(((startLine.x() + stopLine.x()) / 2) - 80, ((startLine.y() + stopLine.y()) / 2));
        }
    }
    else if (abs(startLine.y() - stopLine.y()) < 80)
    {
        if (startLine.x() - stopLine.x() < 0)
        {

            controlPoint = QPointF((startLine.x() + stopLine.x()) / 2, ((startLine.y() + stopLine.y()) / 2) - 80);
        }
        else
        {
            controlPoint = QPointF((startLine.x() + stopLine.x()) / 2, ((startLine.y() + stopLine.y()) / 2) + 80);
        }
    }
    else
    {
        if (startLine.y() - stopLine.y() < 0)
        {
            if (startLine.x() - stopLine.x() < 0)
            {
                controlPoint = QPointF(((startLine.x() + stopLine.x()) / 2) + 80, ((startLine.y() + stopLine.y()) / 2) - 40);
                adjustText = true;
            }
            else
            {
                controlPoint = QPointF(((startLine.x() + stopLine.x()) / 2) + 40, ((startLine.y() + stopLine.y()) / 2) + 40);
            }
        }
        else
        {
            if (startLine.x() - stopLine.x() < 0)
            {
                controlPoint = QPointF(((startLine.x() + stopLine.x()) / 2) - 40, ((startLine.y() + stopLine.y()) / 2) - 40);
            }
            else
            {
                controlPoint = QPointF(((startLine.x() + stopLine.x()) / 2) - 80, ((startLine.y() + stopLine.y()) / 2) + 40);
                adjustText = true;
            }
        }
    }

    // calculating mid points
    points.clear();
    int numPoints = 1; // number of mid points
    for (int i = 0; i <= numPoints; ++i)
    {
        qreal t = qreal(i) / qreal(numPoints);
        qreal mt = 1.0 - t;
        qreal mt2 = mt * mt;
        qreal t2 = t * t;
        QPointF point = mt2 * startLine + 2.0 * mt * t * controlPoint + t2 * stopLine;
        points.append(point);
    }

    // building the path using mid points
    QPainterPath path;
    path.moveTo(startLine);
    for (int i = 1; i < points.size(); ++i)
    {
        path.lineTo(points[i]);
    }

    // creating pen for the curve
    QPen curvePen(Qt::black);
    pen.setWidth(2);

    // drawing the curve using drawpath
    painter->strokePath(path, pen);

    // drawing curve
    QPolygonF arrowHead;
    arrowHead << stopLine << arrowP1 << arrowP2;

    QBrush fillBrush(Qt::red); // set filler color
    painter->setBrush(fillBrush);

    painter->drawPolygon(arrowHead);
    path.addPolygon(arrowHead);

    controlPoint = points[points.size() / 2];
    if (adjustText)
    {
        QPointF textOffset(10, 10);
        controlPoint += textOffset;
    }
    else
    {
        QPointF textOffset(0.4, 0.4);
        controlPoint += textOffset;
    }

    updateTextPosition();
}

/// @brief Equality comparison operator for ArrowItem objects.
/// @param other The ArrowItem object to compare against.
/// @return true if the start and end positions of both ArrowItem objects are equal, false otherwise.
bool ArrowItem::operator==(const ArrowItem &other) const
{
    return startItem->sceneBoundingRect().center() == other.startItem->sceneBoundingRect().center() && endItem->sceneBoundingRect().center() == other.endItem->sceneBoundingRect().center();
}

/// @brief Checks if a QGraphicsEllipseItem is the start or end item of the arrow.
/// @param check The QGraphicsEllipseItem to check against.
/// @return true if the provided item is the start or end item of the arrow, false otherwise.
bool ArrowItem::isMyStartOrEnd(QGraphicsEllipseItem *check)
{
    if (!startItem || !endItem)
    {
        return true;
    }
    if (check->sceneBoundingRect().center() == startItem->sceneBoundingRect().center() || check->sceneBoundingRect().center() == endItem->sceneBoundingRect().center())
        return true;
    return false;
}
