#include "switch.h"

/// @brief Constructor for Animator.
///
/// @param target The target object to animate.
/// @param parent The parent object.
Animator::Animator(QObject *target, QObject *parent) : QVariantAnimation(parent)
{
    setTargetObject(target);
}

/// @brief Destructor for Animator.
Animator::~Animator()
{
    stop();
}

/// @brief Sets the target object for the animation.
///
/// @param _target Pointer to the target object.
///
/// @return None.
QObject *Animator::targetObject() const
{
    return target.data();
}

/// @brief Sets the target object to animate.
/// @param _target The target object.
void Animator::setTargetObject(QObject *_target)
{
    if (target.data() == _target)
        return;

    if (isRunning())
    {
        return;
    }

    target = _target;
}

/// @brief Updates the current value of the animation.
/// @param value The new value (unused).
void Animator::updateCurrentValue(const QVariant &value)
{
    Q_UNUSED(value);

    if (!target.isNull())
    {
        auto update = QEvent(QEvent::StyleAnimationUpdate);
        update.setAccepted(false);
        QCoreApplication::sendEvent(target.data(), &update);
        if (!update.isAccepted())
            stop();
    }
}

/// @brief Updates the state of the animation.
/// @param newState The new state of the animation.
/// @param oldState The previous state of the animation.
void Animator::updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState)
{
    if (target.isNull() && oldState == Stopped)
    {
        return;
    }

    QVariantAnimation::updateState(newState, oldState);
}

/// @brief Sets up the animator with the given duration and easing curve.
/// @param duration The duration of the animation.
/// @param easing The easing curve to be used.
void Animator::setup(int duration, QEasingCurve easing)
{
    setDuration(duration);
    setEasingCurve(easing);
}

/// @brief Interpolates between the start and end values and starts the animation.
/// @param _start The start value of the animation.
/// @param end The end value of the animation.
void Animator::interpolate(const QVariant &_start, const QVariant &end)
{
    setStartValue(_start);
    setEndValue(end);
    start();
}

/// @brief Sets the current value of the animation.
/// @param value The current value to set.
void Animator::setCurrentValue(const QVariant &value)
{
    setStartValue(value);
    setEndValue(value);
    updateCurrentValue(currentValue());
}

/// @brief Constructor for SelectionControl.
/// @param parent The parent widget.
SelectionControl::SelectionControl(QWidget *parent) : QAbstractButton(parent)
{
    setObjectName("SelectionControl");
    setCheckable(true);
}

SelectionControl::~SelectionControl()
{
}

/// @brief Handles the event when the mouse enters the widget.
/// @param e The event object containing event information.
void SelectionControl::enterEvent(QEnterEvent *e)
{
    setCursor(Qt::PointingHandCursor);
    QAbstractButton::enterEvent(e);
}

/// @brief Returns the check state of the selection control.
/// @return The check state of the selection control.
Qt::CheckState SelectionControl::checkState() const
{
    return isChecked() ? Qt::Checked : Qt::Unchecked;
}

/// @brief Sets the check state of the selection control and emits the stateChanged signal.
void SelectionControl::checkStateSet()
{
    const auto state = checkState();
    emit stateChanged(state);
    toggle(state);
}

/// @brief Calls the nextCheckState of the parent class and then checkStateSet of SelectionControl.
void SelectionControl::nextCheckState()
{
    QAbstractButton::nextCheckState();
    SelectionControl::checkStateSet();
}

/// @brief Initializes the Switch widget.
void Switch::init()
{
    setFont(style.font);
    setObjectName("Switch");
    /* setup animations */
    thumbBrushAnimation = new Animator{this, this};
    trackBrushAnimation = new Animator{this, this};
    thumbPosAniamtion = new Animator{this, this};
    thumbPosAniamtion->setup(style.thumbPosAniamtion.duration, style.thumbPosAniamtion.easing);
    trackBrushAnimation->setup(style.trackBrushAnimation.duration, style.trackBrushAnimation.easing);
    thumbBrushAnimation->setup(style.thumbBrushAnimation.duration, style.thumbBrushAnimation.easing);
    /* set init values */
    trackBrushAnimation->setStartValue(colorFromOpacity(style.trackOffBrush, style.trackOffOpacity));
    trackBrushAnimation->setEndValue(colorFromOpacity(style.trackOffBrush, style.trackOffOpacity));
    thumbBrushAnimation->setStartValue(colorFromOpacity(style.thumbOffBrush, style.thumbOffOpacity));
    thumbBrushAnimation->setEndValue(colorFromOpacity(style.thumbOffBrush, style.thumbOffOpacity));
    /* set standard palettes */
    auto p = palette();
    p.setColor(QPalette::Active, QPalette::ButtonText, style.textColor);
    p.setColor(QPalette::Disabled, QPalette::ButtonText, style.textColor);
    setPalette(p);
    setSizePolicy(QSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Fixed));
}

/// @brief Returns the rectangle for the switch indicator.
/// @return The rectangle representing the switch indicator.
QRect Switch::indicatorRect()
{
    const auto w = style.indicatorMargin.left() + style.height + style.indicatorMargin.right();
    return ltr(this) ? QRect(0, 0, w, style.height) : QRect(width() - w, 0, w, style.height);
}

/// @brief Returns the rectangle for the switch text.
/// @return The rectangle representing the switch text.
QRect Switch::textRect()
{
    const auto w = style.indicatorMargin.left() + style.height + style.indicatorMargin.right();
    return ltr(this) ? rect().marginsRemoved(QMargins(w, 0, 0, 0)) : rect().marginsRemoved(QMargins(0, 0, w, 0));
}

/// @brief Constructor for Switch.
/// @param parent The parent widget.
Switch::Switch(QWidget *parent) : SelectionControl(parent)
{
    init();
}

/// @brief Constructor for Switch.
/// @param text The text to display.
/// @param parent The parent widget.
Switch::Switch(const QString &text, QWidget *parent) : Switch(parent)
{
    setText(text);
}

/// @brief Constructor for Switch with custom text and brush.
/// @param text The text to display.
/// @param brush The brush color.
/// @param parent The parent widget.
Switch::Switch(const QString &text, const QBrush &brush, QWidget *parent) : Switch(text, parent)
{
    style.thumbOnBrush = brush.color();
    style.trackOnBrush = brush.color();
}

Switch::~Switch()
{
}

/// @brief Returns the preferred size hint for the Switch widget.
/// @return The preferred size hint.
QSize Switch::sizeHint() const
{
    auto h = style.height;
    auto w = style.indicatorMargin.left() + style.height + style.indicatorMargin.right() + QFontMetrics(font()).horizontalAdvance(text());

    return QSize(w, h);
}

/// @brief Handles the paint event for the Switch widget.
/// @param event The paint event.
void Switch::paintEvent(QPaintEvent *)
{
    /* for desktop usage we do not need Radial reaction */

    QPainter p(this);

    const auto _indicatorRect = indicatorRect();
    const auto _textRect = textRect();
    auto trackMargin = style.indicatorMargin;
    trackMargin.setTop(trackMargin.top() + 2);
    trackMargin.setBottom(trackMargin.bottom() + 2);
    QRectF trackRect = _indicatorRect.marginsRemoved(trackMargin);

    if (isEnabled())
    {
        p.setOpacity(1.0);
        p.setPen(Qt::NoPen);
        /* draw track */
        p.setBrush(trackBrushAnimation->currentValue().value<QColor>());
        p.setRenderHint(QPainter::Antialiasing, true);
        p.drawRoundedRect(trackRect, CORNER_RADIUS, CORNER_RADIUS);
        p.setRenderHint(QPainter::Antialiasing, false);
        /* draw thumb */
        trackRect.setX(trackRect.x() - trackMargin.left() - trackMargin.right() - 2 + thumbPosAniamtion->currentValue().toInt());
        auto thumbRect = trackRect;

        if (!shadowPixmap.isNull())
            p.drawPixmap(thumbRect.center() - QPointF(THUMB_RADIUS, THUMB_RADIUS - 1.0), shadowPixmap);

        p.setBrush(thumbBrushAnimation->currentValue().value<QColor>());
        p.setRenderHint(QPainter::Antialiasing, true);
        p.drawEllipse(thumbRect.center(), THUMB_RADIUS - SHADOW_ELEVATION - 1.0, THUMB_RADIUS - SHADOW_ELEVATION - 1.0);
        p.setRenderHint(QPainter::Antialiasing, false);

        /* draw text */
        if (text().isEmpty())
            return;

        p.setOpacity(1.0);
        p.setPen(palette().color(QPalette::Active, QPalette::ButtonText));
        p.setFont(font());
        p.drawText(_textRect, Qt::AlignLeft | Qt::AlignVCenter, text());
    }
    else
    {
        p.setOpacity(style.trackDisabledOpacity);
        p.setPen(Qt::NoPen);
        // draw track
        p.setBrush(style.trackDisabled);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.drawRoundedRect(trackRect, CORNER_RADIUS, CORNER_RADIUS);
        p.setRenderHint(QPainter::Antialiasing, false);
        // draw thumb
        p.setOpacity(1.0);
        if (!isChecked())
            trackRect.setX(trackRect.x() - trackMargin.left() - trackMargin.right() - 2);
        else
            trackRect.setX(trackRect.x() + trackMargin.left() + trackMargin.right() + 2);
        auto thumbRect = trackRect;

        if (!shadowPixmap.isNull())
            p.drawPixmap(thumbRect.center() - QPointF(THUMB_RADIUS, THUMB_RADIUS - 1.0), shadowPixmap);

        p.setOpacity(1.0);
        p.setBrush(style.thumbDisabled);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.drawEllipse(thumbRect.center(), THUMB_RADIUS - SHADOW_ELEVATION - 1.0, THUMB_RADIUS - SHADOW_ELEVATION - 1.0);

        /* draw text */
        if (text().isEmpty())
            return;

        p.setOpacity(style.disabledTextOpacity);
        p.setPen(palette().color(QPalette::Disabled, QPalette::ButtonText));
        p.setFont(font());
        p.drawText(_textRect, Qt::AlignLeft | Qt::AlignVCenter, text());
    }
}

/// @brief Handles the resize event for the Switch widget.
/// @param e The resize event.
void Switch::resizeEvent(QResizeEvent *e)
{
    shadowPixmap = Style::drawShadowEllipse(THUMB_RADIUS, SHADOW_ELEVATION, QColor(0, 0, 0, 70));
    SelectionControl::resizeEvent(e);
}

/// @brief Toggles the switch state based on the provided check state.
/// @param state The check state.
void Switch::toggle(Qt::CheckState state)
{
    if (state == Qt::Checked)
    {
        const QVariant posEnd = (style.indicatorMargin.left() + style.indicatorMargin.right() + 2) * 2;
        const QVariant thumbEnd = colorFromOpacity(style.thumbOnBrush, style.thumbOnOpacity);
        const QVariant trackEnd = colorFromOpacity(style.trackOnBrush, style.trackOnOpacity);

        if (!isVisible())
        {
            thumbPosAniamtion->setCurrentValue(posEnd);
            thumbBrushAnimation->setCurrentValue(thumbEnd);
            trackBrushAnimation->setCurrentValue(trackEnd);
        }
        else
        {
            thumbPosAniamtion->interpolate(0, posEnd);
            thumbBrushAnimation->interpolate(colorFromOpacity(style.thumbOffBrush, style.thumbOffOpacity), thumbEnd);
            trackBrushAnimation->interpolate(colorFromOpacity(style.trackOffBrush, style.trackOffOpacity), trackEnd);
        }
    }
    else
    { // Qt::Unchecked
        const QVariant posEnd = 0;
        const QVariant thumbEnd = colorFromOpacity(style.thumbOffBrush, style.thumbOffOpacity);
        const QVariant trackEnd = colorFromOpacity(style.trackOffBrush, style.trackOffOpacity);

        if (!isVisible())
        {
            thumbPosAniamtion->setCurrentValue(posEnd);
            thumbBrushAnimation->setCurrentValue(thumbEnd);
            trackBrushAnimation->setCurrentValue(trackEnd);
        }
        else
        {
            thumbPosAniamtion->interpolate(thumbPosAniamtion->currentValue().toInt(), posEnd);
            thumbBrushAnimation->interpolate(colorFromOpacity(style.thumbOnBrush, style.thumbOnOpacity), thumbEnd);
            trackBrushAnimation->interpolate(colorFromOpacity(style.trackOnBrush, style.trackOnOpacity), trackEnd);
        }
    }
}
