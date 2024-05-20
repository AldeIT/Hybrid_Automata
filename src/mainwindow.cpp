#include "../include/mainwindow.h"
#include "../include/arrowitem.h"
#include "../include/json.hpp"
#include "./ui_mainwindow.h"
#include <QMouseEvent> // Include the necessary header for QMouseEvent
#include <QtWidgets>
#include <iostream>
#include <fstream>
#include <QApplication>
#include <QGuiApplication>
#include <QScreen>
#include <QMessageBox>
#include <QRandomGenerator>
#include "../include/UtilsJson.h"
#include "../include/tools.h"
#include "../include/csvfile.h"
#include <QKeyEvent>
#include <thread>
#include "switch.h"
#include <algorithm>

using json = nlohmann::json;

/// @brief Constructs the MainWindow object.
/// This constructor initializes the MainWindow object, setting up the user interface,
/// initializing member variables, creating a graphics scene, and configuring various UI elements.
/// @param parent The parent widget of the MainWindow.
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
                                          ui(new Ui::MainWindow)

{
    sem_init(&semaforo, 0, 0);
    _count = 0;
    ui->setupUi(this);
    ui->tabWidget->setTabText(0, "Main");
    ui->tabWidget->setTabText(1, "Designer");
    posUpdateButton.append(ui->updateButton->pos().x());
    posUpdateButton.append(ui->updateButton->pos().y());
    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    QColor colorBackgroundGrid(200, 200, 200);
    QBrush brush(colorBackgroundGrid, Qt::CrossPattern);
    ui->graphicsView->scene()->setBackgroundBrush(brush);
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    ui->frameDebug->hide();
    ui->discardInput->hide();
    ui->stopButton->hide();
    ui->pauseButton->hide();
    ui->showVariables->hide();
    ui->frameDebug->setStyleSheet("border: none;");
    ui->frameDebug->hide();
    ui->horizontalSpacer_2->changeSize(10, 20);

    counter_colors.resize(colors_left);
    counter_colors.fill(1);

    for (double i = 0.1; i < 1.2; i += 0.05)
        trasparenze.append(i);
    for (double i = 1.2; i > 0; i -= 0.05)
        trasparenze.append(i);
    ct = 0;

    timer = new QTimer(this);

    // movable text
    ui->graphicsView->installEventFilter(this);
    connect(scene, &QGraphicsScene::selectionChanged, this, &MainWindow::handleSelectionChanged);
    connect(timer, &QTimer::timeout, this, &MainWindow::handleRefresh);
    timer->start(33);

    // dimensions of the primary screen
    QRect screenGeometry = QGuiApplication::primaryScreen()->geometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();

    // 80% of dimensions
    int newWidth = screenWidth * 0.8;
    int newHeight = screenHeight * 0.8;

    this->setFixedSize(newWidth, newHeight);
    ui->deltaSpinBox->setMinimum(0.0001);

    ui->graphicsView->setFixedSize(newWidth * 0.817, newHeight * 0.935);
    hideDesignerInput();

    switchDebug = new Switch("DEBUG MODE");
    ui->debugSpace->addWidget(switchDebug);

    // boolean value for the stop button
    stop = new bool(false);
    pause = new bool(false);
}

/// @brief Destroys the MainWindow object.
/// This destructor cleans up resources associated with the MainWindow object, including
/// stopping a background process, releasing a semaphore, and deleting the user interface.
MainWindow::~MainWindow()
{
    *stop = true;
    sem_post(&semaforo);
    clearAll(1);
    delete ui;
}

/// @brief Blends two colors with a specified overlay opacity.
/// @param baseColor The base color.
/// @param overlayColor The overlay color.
/// @param overlayOpacity The opacity of the overlay color, ranging from 0.0 (fully transparent) to 1.0 (fully opaque).
/// @return The blended color.
QColor blendColors(const QColor &baseColor, const QColor &overlayColor, qreal overlayOpacity)
{
    // Calculate combined opacity
    qreal alpha = overlayColor.alphaF() * overlayOpacity;

    // Calculate combined RGBA components
    int red = baseColor.red() * (1 - alpha) + overlayColor.red() * alpha;
    int green = baseColor.green() * (1 - alpha) + overlayColor.green() * alpha;
    int blue = baseColor.blue() * (1 - alpha) + overlayColor.blue() * alpha;

    // Return the blended color
    return QColor(red, green, blue);
}

/// @brief Handles the refresh of the main window.
/// This method updates the graphics view, adjusts the UI elements based on the
/// current running status, updates the step counter, and updates the colors of
/// circles and other graphical elements based on the current state of the simulation.
void MainWindow::handleRefresh()
{
    ui->graphicsView->update();
    QColor shadowColor(0, 0, 0, 128);
    QColor combinedColor;

    if (goalStep == istanti)
    {
        ui->runForButton->setEnabled(true);
        ui->stepButton->setEnabled(true);
        ui->runForButton->setEnabled(true);
        ui->moreSteps->setEnabled(true);
    }

    if (runningStatus)
    {
        if (switchDebug->isChecked())
            ui->currentStep->setText(QString::fromStdString(std::to_string(istanti) + "/" + std::to_string(static_cast<int>(finalTime / delta))));
        else
            ui->currentStep->setText(QString::fromStdString("STEP: " + std::to_string(istanti) + "/" + std::to_string(static_cast<int>(finalTime / delta))));

        for (int z = 0; z < circles.size(); z++)
        {
            QPen outlinePen(Qt::black);
            outlinePen.setWidth(2);
            circles[z]->ellipse->setBrush(QBrush(automataColors[circles[z]->automata]));
            circles[z]->ellipse->setPen(outlinePen);
        }
        for (int i = 0; i < v.size(); i++)
        {
            if (switchDebug->isChecked() && false) // serve per dopo, mancano altri controlli
            {
                tempMap[v[i].getCurrentNodeName() + "~" + v[i].getName()]->setBrush(QBrush(QColor(Qt::red)));
            }
            else
            {
                combinedColor = blendColors(automataColors[QString::fromStdString(v[i].getName())], shadowColor, trasparenze[ct % trasparenze.size()]);
                tempMap[v[i].getCurrentNodeName() + "~" + v[i].getName()]->setBrush(QBrush(combinedColor));
            }
        }
        string temp;
        ui->showVariables->clear();
        for (const auto &pair : updateVariables)
        {
            temp = pair.first + "=" + std::to_string(pair.second);
            ui->showVariables->addItem(QString::fromStdString(temp));
        }
        ct++;
    }
}

/// @brief Checks if the selected items in the scene are valid.
/// @return true if the selected items are valid circle items and are either the first or second selected circles, false otherwise.
bool MainWindow::checkSelected()
{
    QList<QGraphicsItem *> selectedItems = scene->selectedItems();
    for (int i = 0; i < selectedItems.size(); i++)
    {
        if (selectedItems[i]->type() != CIRCLEITEM_TYPE)
            return false;

        CircleItem *selectedCircle = dynamic_cast<CircleItem *>(selectedItems[i]);
        if (selectedCircle->ellipse->sceneBoundingRect().center() != selectedCircle1->sceneBoundingRect().center() && selectedCircle->ellipse->sceneBoundingRect().center() != selectedCircle2->sceneBoundingRect().center())
            return false;
    }
    return true;
}

/// @brief Filters events for the graphics view.
/// This method filters events for the graphics view, handling mouse button presses,
/// mouse wheel events, and key presses. It adds new circle items to the scene,
/// handles zooming with the mouse wheel while holding the Ctrl key, deletes selected items
/// when the Delete or Backspace key is pressed, creates arrows between selected circles
/// when the 'K' key is pressed, and clears all elements from the scene when the 'C' key is pressed.
/// @param watched The object that the event filter is installed on.
/// @param event The event to be processed.
/// @return true if the event was handled and should not be propagated further, false otherwise.
bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->graphicsView)
    {
        if (event->type() == QEvent::MouseButtonPress && !runningStatus)
        {
            if (automatas.size() == 0 || ui->automatasList->currentText().isEmpty())
                return true;
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() != Qt::RightButton)
                return true;
            QPointF scenePos = ui->graphicsView->mapToScene(mouseEvent->pos());
            QBrush brush(automataColors[ui->automatasList->currentText()]);

            QPen outlinePen(Qt::black);
            outlinePen.setWidth(2);
            QGraphicsEllipseItem *newEllipse = new QGraphicsEllipseItem(scenePos.x(), scenePos.y(), 80, 80);
            newEllipse->setPen(outlinePen);
            newEllipse->setBrush(brush);
            QGraphicsTextItem *textLabel = new QGraphicsTextItem("default Name");
            textLabel->setDefaultTextColor(Qt::white);
            textLabel->setFont(QFont("Arial", 10));
            textLabel->setPos(scenePos.x() + 10, scenePos.y() + 10);
            CircleItem *circleItem = new CircleItem(newEllipse, textLabel, ui->automatasList->currentText());
            ellipseMap[newEllipse] = circleItem;
            circleItem->setFlag(QGraphicsItem::ItemIsMovable);
            circleItem->setFlag(QGraphicsItem::ItemIsSelectable);
            bool found = false;
            for (int i = 0; i < circles.size(); i++)
            {
                if (circles[i]->automata == ui->automatasList->currentText())
                    found = true;
            }
            if (!found)
                circleItem->startNode = true;
            circles.append(circleItem);
            scene->addItem(circleItem);
            hideDesignerInput();
            return true; // Consume the event
        }
        else if (event->type() == QEvent::Wheel)
        {
            QWheelEvent *wheelEvent = static_cast<QWheelEvent *>(event);
            if (wheelEvent->modifiers() & Qt::ControlModifier)
            {
                if (wheelEvent->angleDelta().y() > 0)
                {

                    ui->graphicsView->scale(1.1, 1.1);
                }
                else
                {
                    ui->graphicsView->scale(0.9, 0.9);
                }
                return true;
            }
        }
        else if (event->type() == QEvent::KeyPress && !runningStatus)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent->key() == Qt::Key_Delete || keyEvent->key() == Qt::Key_Backspace)
            {
                deleteSelectedItems();
                hideDesignerInput();
                return true; // Consume the event
            }
            else if (keyEvent->key() == Qt::Key_K)
            {
                if (selectedCircle1 && selectedCircle2 && scene->selectedItems().size() == 2 && checkSelected())
                {
                    if (ellipseMap[selectedCircle1]->automata != ellipseMap[selectedCircle2]->automata)
                        return true;
                    ArrowItem *arrow;

                    if (arrows.contains(selectedCircle1))
                    {
                        QList<QGraphicsEllipseItem *> dest = arrows[selectedCircle1];
                        if (!dest.contains(selectedCircle2))
                        {
                            arrow = new ArrowItem(selectedCircle1, selectedCircle2);
                            arrow->setFlag(QGraphicsItem::ItemIsSelectable);
                            scene->addItem(arrow);
                            drawnArrows.append(arrow);
                            arrows[selectedCircle1].append(selectedCircle2);
                        }
                    }
                    else
                    {
                        arrow = new ArrowItem(selectedCircle1, selectedCircle2);
                        arrow->setFlag(QGraphicsItem::ItemIsSelectable);
                        scene->addItem(arrow);
                        drawnArrows.append(arrow);
                        arrows.insert(selectedCircle1, QList<QGraphicsEllipseItem *>());
                        arrows[selectedCircle1].append(selectedCircle2);
                    }
                    ascendingSelection = true;
                    //  Clear previously selected circles
                    scene->selectedItems().clear();
                }
                return true; // Consume the event
            }
            else if (keyEvent->key() == Qt::Key_Escape)
            {
                scene->clearSelection();
                hideDesignerInput();
            }
            else if (keyEvent->key() == Qt::Key_C)
            { // clear all
                clearAll(0);
            }
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

/// @brief Clears all elements from the scene.
///
/// This method clears all elements from the scene, including arrows, circle items,
/// automata colors, automata names, and related data structures. It also resets
/// variables related to colors and hides designer input elements if needed.
///
/// @param mode The mode indicating whether to prompt the user for confirmation before clearing (0) or to clear without confirmation (1).
void MainWindow::clearAll(int mode)
{

    scene->clearSelection();
    QMessageBox msgBox;
    int reply = -1;

    if (mode == 0)
    {
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle("Attention!");
        msgBox.setText("Do you want to delete all?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        reply = msgBox.exec();
    }

    if (reply == QMessageBox::Yes || mode == 1)
    {
        arrows.clear();
        ellipseMap.clear();
        automataColors.clear();
        automatas.clear();
        ui->automatasList->clear();

        counter_colors.fill(1);
        colors_left = counter_colors.size();

        for (int i = 0; i < drawnArrows.size(); i++)
        {
            // delete drawnArrows[i];

            drawnArrows[i]->textItem->setVisible(false);
            drawnArrows[i]->setVisible(false);
            scene->removeItem(drawnArrows[i]);
            ui->graphicsView->invalidateScene();
            ui->graphicsView->update();
        }
        drawnArrows.clear();
        for (int i = 0; i < circles.size(); i++)
        {

            scene->removeItem(circles[i]->ellipse);
            delete circles[i];
        }
        circles.clear();
        variablesValues.clear();
        actualVariables.clear();
        hideDesignerInput();
    }
}

/// @brief Handles the change in selection of items in the scene.
///
/// This method is called whenever the selection of items in the scene changes.
/// It updates the appearance of selected circle items by blending their colors
/// with a shadow color to indicate selection. It also updates the displayed
/// information about the selected items in the UI. If exactly two circle items
/// are selected, it determines which ones are selected first and second, and
/// updates related variables accordingly. It manages the visibility of designer
/// input elements based on the selection.
void MainWindow::handleSelectionChanged()
{
    // Get selected items
    QList<QGraphicsItem *> selectedItems = scene->selectedItems();

    for (int i = 0; i < circles.size(); i++)
    {
        QBrush brush(automataColors[circles[i]->automata]);
        circles[i]->ellipse->setBrush(brush);
    }
    for (int i = 0; i < selectedItems.size(); i++)
    {
        if (selectedItems[i]->type() == CIRCLEITEM_TYPE)
        {
            CircleItem *selectedCircle = dynamic_cast<CircleItem *>(selectedItems[i]);
            QColor shadowColor(0, 0, 0, 128);                                                               // Black color with 50% alpha
            QColor combinedColor = blendColors(automataColors[selectedCircle->automata], shadowColor, 0.6); // 50% opacity
            selectedCircle->ellipse->setBrush(QBrush(combinedColor));
        }
    }
    if (selectedItems.size() == 0)
    {
        ui->automatasList->setCurrentIndex(-1);
        selectedCircle1 = nullptr;
        selectedCircle2 = nullptr;
        ascendingSelection = true;
        isCircleSelected = false;
        ui->valueLabel->setText("");
        ui->nameLabel->setText("");
        ui->descriptionLabel->setText("");
        ui->startCheckBox->setChecked(false);
        hideDesignerInput();
    }

    if (selectedItems.size() == 1)
    {
        if (selectedItems[0]->type() == CIRCLEITEM_TYPE)
        {
            isCircleSelected = true;
            CircleItem *selectedCircle = dynamic_cast<CircleItem *>(selectedItems[0]);
            ui->automatasList->setCurrentIndex(automatas.indexOf(selectedCircle->automata));
            ui->valueLabel->setText(selectedCircle->instructions);
            ui->nameLabel->setText(selectedCircle->name);
            ui->descriptionLabel->setText(selectedCircle->description);
            if (selectedCircle->startNode)
            {
                ui->startCheckBox->setChecked(true);
            }
            else
                ui->startCheckBox->setChecked(false);
            QGraphicsEllipseItem *temp = selectedCircle->ellipse;

            selectedCircle1 = qgraphicsitem_cast<QGraphicsEllipseItem *>(selectedCircle->ellipse);

            showDesignerInput(0);
        }
        else
        {
            ui->startCheckBox->setChecked(false);
            ui->nameLabel->setText("");
            ui->descriptionLabel->setText("");
            isCircleSelected = false;
            selectedArrow = dynamic_cast<ArrowItem *>(selectedItems[0]);
            ui->valueLabel->setText(selectedArrow->textItem->toPlainText());

            showDesignerInput(1);
        }
    }

    // Check if exactly two circles are selected
    if (selectedItems.size() == 2 && ascendingSelection)
    {
        if (selectedItems[0]->type() == CIRCLEITEM_TYPE &&
            selectedItems[1]->type() == CIRCLEITEM_TYPE)
        {
            CircleItem *selectedCircle = dynamic_cast<CircleItem *>(selectedItems[0]);
            CircleItem *selectedCircleSecond = dynamic_cast<CircleItem *>(selectedItems[1]);
            if (selectedCircle1->sceneBoundingRect().center() == qgraphicsitem_cast<QGraphicsEllipseItem *>(selectedCircle->ellipse)->sceneBoundingRect().center())
                selectedCircle2 = qgraphicsitem_cast<QGraphicsEllipseItem *>(selectedCircleSecond->ellipse);
            else
                selectedCircle2 = qgraphicsitem_cast<QGraphicsEllipseItem *>(selectedCircle->ellipse);
        }
        ascendingSelection = true;
        ascendingSelection = false;
        hideDesignerInput();
    }

    if (selectedItems.size() >= 3)
    {
        ascendingSelection = false;
        hideDesignerInput();
    }
}

/// @brief Deletes the selected items from the scene.
/// This method removes selected items from the scene, including both circle items and arrow items.
/// It updates the internal data structures and UI elements accordingly.
/// For each selected circle item, it removes associated arrows and updates automata - related data.
/// For each selected arrow item, it removes the arrow and updates related data.*Finally, it clears the selection in the scene.
void MainWindow::deleteSelectedItems()
{
    // Get a list of all selected items
    QList<QGraphicsItem *> selectedItems = scene->selectedItems();
    int index = -1;
    int count = 0;
    QGraphicsItem *tempItem;
    for (int i = 0; i < selectedItems.size(); i++)
    {
        tempItem = selectedItems[i];
        if (dynamic_cast<ArrowItem *>(tempItem) != nullptr)
        {
            ArrowItem *temp = static_cast<ArrowItem *>(tempItem);

            QGraphicsItem *inizio = temp->startItem;
            QGraphicsEllipseItem *cerchioInizio = qgraphicsitem_cast<QGraphicsEllipseItem *>(inizio);

            QList<QGraphicsEllipseItem *> toRemove = arrows[cerchioInizio];
            index = -1;
            for (int i = 0; i < toRemove.size(); i++)
            {
                if (toRemove[i] == temp->endItem)
                {
                    index = i;
                }
            }
            if (index != -1)
            {
                arrows[qgraphicsitem_cast<QGraphicsEllipseItem *>(temp->startItem)].removeAt(index);
                if (arrows[qgraphicsitem_cast<QGraphicsEllipseItem *>(temp->startItem)].isEmpty())
                {
                    arrows.remove(qgraphicsitem_cast<QGraphicsEllipseItem *>(temp->startItem));
                }
            }
            index = -1;
            for (int i = 0; i < drawnArrows.size(); i++)
            {
                if (drawnArrows[i] == temp)
                {
                    index = i;
                }
            }
            if (index != -1)
            {
                drawnArrows[index]->textItem->setVisible(false);
                drawnArrows[index]->setVisible(false);
                scene->removeItem(drawnArrows[index]);
                ui->graphicsView->invalidateScene();
                ui->graphicsView->update();
                drawnArrows.removeAt(index);
            }

            selectedItems[i] = nullptr;
        }
    }

    // Delete each selected item
    for (QGraphicsItem *item : selectedItems)
    {

        if (item == nullptr)
        {
            continue;
        }

        if (dynamic_cast<CircleItem *>(item) == nullptr)
        {
            ArrowItem *temp = static_cast<ArrowItem *>(item);

            QGraphicsItem *inizio = temp->startItem;
            QGraphicsEllipseItem *cerchioInizio = qgraphicsitem_cast<QGraphicsEllipseItem *>(inizio);

            QList<QGraphicsEllipseItem *> toRemove = arrows[cerchioInizio];
            index = -1;
            for (int i = 0; i < toRemove.size(); i++)
            {
                if (toRemove[i] == temp->endItem)
                {
                    index = i;
                }
            }
            if (index != -1)
            {
                arrows[qgraphicsitem_cast<QGraphicsEllipseItem *>(temp->startItem)].removeAt(index);
                if (arrows[qgraphicsitem_cast<QGraphicsEllipseItem *>(temp->startItem)].isEmpty())
                {
                    arrows.remove(qgraphicsitem_cast<QGraphicsEllipseItem *>(temp->startItem));
                }
            }
            index = -1;
            for (int i = 0; i < drawnArrows.size(); i++)
            {
                if (drawnArrows[i] == temp)
                {
                    index = i;
                }
            }
            if (index != -1)
            {

                drawnArrows[index]->textItem->setVisible(false);
                drawnArrows[index]->setVisible(false);

                scene->removeItem(drawnArrows[index]);
                ui->graphicsView->invalidateScene();
                ui->graphicsView->update();
                drawnArrows.removeAt(index);
            }

            continue;
        }
        CircleItem *selectedCircle = dynamic_cast<CircleItem *>(item);
        QGraphicsEllipseItem *temp = qgraphicsitem_cast<QGraphicsEllipseItem *>(selectedCircle->ellipse);

        if (arrows.contains(temp))
        {
            arrows.remove(temp);
        }

        QMap<QGraphicsEllipseItem *, QList<QGraphicsEllipseItem *>>::const_iterator it;
        QList<QGraphicsEllipseItem *> toRemove;
        for (it = arrows.constBegin(); it != arrows.constEnd(); ++it)
        {
            QGraphicsEllipseItem *start = it.key();
            QList<QGraphicsEllipseItem *> end = it.value();
            index = -1;
            for (int i = 0; i < end.size(); i++)
            {
                if (end[i] == temp)
                {
                    index = i;
                }
            }
            if (index != -1)
            {
                arrows[start].removeAt(index);
                if (arrows[start].isEmpty())
                {
                    toRemove.append(start);
                }
            }
        }
        for (int i = 0; i < toRemove.size(); i++)
        {
            arrows.remove(toRemove[i]);
        }
        QList<int> arrowsToRemove;

        // Iterate over the drawnArrows list in reverse order
        for (int i = 0; i < drawnArrows.size(); i++)
        {
            if (drawnArrows[i]->isMyStartOrEnd(temp))
            {
                drawnArrows[i]->textItem->setVisible(false);
                drawnArrows[i]->setVisible(false);
                scene->removeItem(drawnArrows[i]);
                ui->graphicsView->invalidateScene();
                ui->graphicsView->update();
                arrowsToRemove.append(i);
            }
        }

        // Remove the items from the drawnArrows list in reverse order
        for (int i = arrowsToRemove.size() - 1; i >= 0; i--)
        {
            drawnArrows.removeAt(arrowsToRemove[i]);
        }
        ellipseMap.remove(temp);
        int countCirclesAutomata = 0;
        for (int i = 0; i < circles.size(); i++)
        {
            if (circles[i]->automata == selectedCircle->automata)
                countCirclesAutomata++;
        }
        if (countCirclesAutomata < 2)
        {
            automataColors.remove(selectedCircle->automata);
            int pos;
            for (int x = 0; x < automatas.size(); x++)
            {
                if (automatas[x] == selectedCircle->automata)
                {
                    pos = x;
                }
            }
            automatas.remove(pos);
            ui->automatasList->clear();
            ui->automatasList->addItems(automatas);
        }

        index = -1;
        for (int i = 0; i < circles.size(); i++)
        {
            if (*circles[i] == *selectedCircle)
            {
                index = i;
            }
        }
        if (index != -1)
            circles.removeAt(index);

        scene->removeItem(selectedCircle);
        // delete item;
    }
    scene->selectedItems().clear();
    scene->clearSelection();
}

/// @brief Handles the click event of the update button.
///
/// This method is triggered when the user clicks the update button in the UI.
/// It updates the properties of the selected circle or arrow item based on the input values from the UI.
/// For a selected circle item, it verifies the correctness of the input instructions and node name,
/// updates the circle's properties, and ensures that only one start node is selected per automata.
/// For a selected arrow item, it verifies the correctness of the input conditions and updates the arrow's conditions.
/// If any error is encountered during the update process, warning messages are displayed to the user.
/// Finally, it clears the input fields and deselects all items in the scene.
void MainWindow::on_updateButton_clicked()
{
    QList<QGraphicsItem *> selectedItems = scene->selectedItems();
    bool error = false;
    int pos = 0;
    if (selectedItems.size() == 1)
    {
        if (isCircleSelected)
        {
            CircleItem *selectedCircle = dynamic_cast<CircleItem *>(selectedItems[0]);
            QGraphicsEllipseItem *ellipse = selectedCircle->ellipse;
            if (ellipse->sceneBoundingRect().center() == selectedCircle1->sceneBoundingRect().center())
            {
                for (int i = 0; i < circles.size(); i++)
                {
                    if (circles[i] == selectedCircle)
                    {
                        if (ui->valueLabel->text().count(";") != ui->valueLabel->text().count("=") || ui->valueLabel->text().count(";") == 0)
                        {
                            QMessageBox::information(nullptr, "Warning", "Typo in current intructions");
                            error = true;
                            break;
                        }
                        for (int j = 0; j < circles.size(); j++)
                        {
                            if (circles[j]->automata == circles[i]->automata && i != j)
                            {
                                if (circles[j]->name == ui->nameLabel->text())
                                {
                                    QMessageBox::information(nullptr, "Warning", "A node with this name already exists");
                                    return;
                                }
                            }
                        }
                        circles[i]->name = ui->nameLabel->text();
                        circles[i]->description = ui->descriptionLabel->text();
                        circles[i]->instructions = ui->valueLabel->text();
                        circles[i]->startNode = ui->startCheckBox->isChecked();
                        circles[i]->textItem->setPlainText(circles[i]->name);
                        pos = i;
                    }
                }
                for (int j = 0; j < circles.size(); j++)
                {
                    if (ui->startCheckBox->isChecked() && circles[j]->automata == selectedCircle->automata && j != pos)
                        circles[j]->startNode = false;
                }
                selectedCircle->setSelected(false);
            }
        }
        else
        {
            for (int i = 0; i < drawnArrows.size(); i++)
            {
                if (drawnArrows[i] == selectedArrow)
                {
                    if (!(ui->valueLabel->text().startsWith("(") && ui->valueLabel->text().endsWith(")") &&
                          (ui->valueLabel->text().count("(") == ui->valueLabel->text().count(")"))))
                    {
                        QMessageBox::information(nullptr, "Warning", "Typo in current conditions");
                        error = true;
                        break;
                    }
                    drawnArrows[i]->textItem->setPlainText(ui->valueLabel->text());
                }
            }
        }
    }
    if (!error)
    {
        ui->valueLabel->setText("");
        ui->nameLabel->setText("");
        ui->descriptionLabel->setText("");
        ui->startCheckBox->setChecked(false);
        scene->clearSelection();
    }
}

/// @brief Checks if a QString represents a numeric value.
/// @param str The QString to be checked for numeric value.
/// @return True if the QString represents a numeric value, otherwise false
bool isNumeric(const QString &str)
{
    QRegularExpression regex("[+-]?[0-9]+(\\.[0-9]+)?");
    return str.contains(regex);
}

/// @brief Handles the change of the current tab in the MainWindow.
///
/// This slot function is called when the current tab in the MainWindow's tab widget changes.
/// It updates the list of actual variables and their values based on the instructions
/// stored in each CircleItem. It also updates the list of variables in the UI.
///
/// @param index The index of the newly selected tab.
void MainWindow::on_tabWidget_currentChanged(int index)
{
    if (index == 1)
        return;
    QList<QString> separator;
    separator << "+"
              << "-"
              << "*"
              << "/"
              << "'"
              << ";"
              << "("
              << ")"
              << "["
              << "]"
              << "=";
    QList<QString> functions;
    functions << "log"
              << "exp"
              << "sqrt"
              << "cos"
              << "sin"
              << "ln"
              << "tan"
              << "arctan"
              << "cosin"
              << "cotan";
    actualVariables.clear();
    QString temp;
    QString tempVar;
    for (int i = 0; i < circles.size(); i++)
    {
        temp = circles[i]->instructions;
        tempVar.clear();
        temp.erase(std::remove(temp.begin(), temp.end(), ' '), temp.end());
        for (int j = 0; j < temp.length(); j++)
        {
            if (!separator.contains(temp[j]))
            {
                tempVar += temp[j];
            }
            else
            {
                if (functions.contains(tempVar))
                { // math function
                    tempVar.clear();
                }
                else
                {
                    if (!isNumeric(tempVar) && !tempVar.isEmpty() && !actualVariables.contains(tempVar))
                    {
                        actualVariables.append(tempVar);
                        if (!variablesValues.contains(tempVar))
                        {
                            variablesValues[tempVar] = "NaN";
                        }
                    }
                    tempVar.clear();
                }
            }
        }
    }

    QList<QString> variablesToRemove;
    QMap<QString, QString>::const_iterator it;
    for (it = variablesValues.constBegin(); it != variablesValues.constEnd(); ++it)
    {
        QString start = it.key();
        if (!actualVariables.contains(start))
        {
            variablesToRemove.append(start);
        }
    }

    for (QString toRemove : variablesToRemove)
    {
        variablesValues.remove(toRemove);
    }

    ui->selectVariable->clear();
    ui->selectVariable->addItems(actualVariables);

    temp.clear();
    ui->listVariables->clear();
    for (QString toAdd : actualVariables)
    {
        temp = toAdd + "=" + variablesValues[toAdd];
        ui->listVariables->addItem(temp);
    }
}

/// @brief Handles the update of a variable's value in the MainWindow.
///
/// This slot function is called when the "Update Variable" button is clicked.
/// It updates the value of the selected variable in the variablesValues map
/// based on the value entered in the variableValue spin box. It then updates
/// the list of variables and their values in the UI accordingly.
void MainWindow::on_updtateVariable_clicked()
{
    QString temp;
    variablesValues[ui->selectVariable->currentText()] = QString::number(ui->variableValue->value());
    ui->listVariables->clear();
    for (QString toAdd : actualVariables)
    {
        temp = toAdd + "=" + variablesValues[toAdd];
        ui->listVariables->addItem(temp);
    }
}

/// @brief Replaces all commas with periods in the input string.
/// @param input The input string in which commas will be replaced.
/// @return A new string with commas replaced by periods.
std::string replaceCommasWithPeriods(const std::string &input)
{
    std::string result = input;
    for (char &c : result)
    {
        if (c == ',')
        {
            c = '.';
        }
    }
    return result;
}

/// @brief Hides the UI elements related to designer input.
void MainWindow::hideDesignerInput()
{
    ui->valueLabel->setVisible(false);
    ui->istruction->setVisible(false);
    ui->name->setVisible(false);
    ui->nameLabel->setVisible(false);
    ui->description->setVisible(false);
    ui->descriptionLabel->setVisible(false);
    ui->startCheckBox->setVisible(false);
    ui->updateButton->setVisible(false);
}

/// @brief Sets the edit status of UI elements.
/// @param mode A boolean indicating whether to enable (true) or disable (false) the UI elements.
void MainWindow::setEditStatus(bool mode)
{
    QString stringa = "IMPORT INPUT FILE";
    ui->loadData->setVisible(mode);
    ui->saveData->setVisible(mode);
    if (mode == true && ui->inputFileButton->text() != stringa || mode == false)
        ui->discardInput->setVisible(mode);
    ui->inputFileButton->setVisible(mode);
    ui->automatasList->setEnabled(mode);
    ui->valueLabel->setEnabled(mode);
    ui->nameLabel->setEnabled(mode);
    ui->descriptionLabel->setEnabled(mode);
    ui->startCheckBox->setEnabled(mode);
    ui->updateButton->setEnabled(mode);
    ui->automataName->setEnabled(mode);
    ui->addAutoma->setEnabled(mode);
    ui->selectVariable->setEnabled(mode);
    ui->variableValue->setEnabled(mode);
    ui->updtateVariable->setEnabled(mode);
    ui->finalTimeSpinBox->setEnabled(mode);
    ui->deltaSpinBox->setEnabled(mode);
    ui->jsonButton->setEnabled(mode);
    switchDebug->setEnabled(mode);
    ui->loadData->setEnabled(mode);
    ui->saveData->setEnabled(mode);
}

/// @brief Shows designer input elements based on the mode.
/// @param mode An integer indicating the mode: 0 for selected circle mode, 1 for selected arrow mode.
void MainWindow::showDesignerInput(int mode)
{
    if (mode == 0)
    { // modalità cerchio selezionato
        ui->valueLabel->setVisible(true);
        ui->istruction->setVisible(true);
        ui->istruction->setText("Instructions");
        ui->name->setVisible(true);
        ui->nameLabel->setVisible(true);
        ui->description->setVisible(true);
        ui->descriptionLabel->setVisible(true);
        ui->startCheckBox->setVisible(true);
        ui->updateButton->setVisible(true);
        ui->updateButton->move(posUpdateButton[0], posUpdateButton[1]);
    }
    else
    {
        ui->valueLabel->setVisible(true);
        ui->istruction->setVisible(true);
        ui->istruction->setText("Conditions");
        ui->name->setVisible(false);
        ui->nameLabel->setVisible(false);
        ui->description->setVisible(false);
        ui->descriptionLabel->setVisible(false);
        ui->startCheckBox->setVisible(false);
        ui->updateButton->setVisible(true);
        ui->updateButton->move(60, ui->nameLabel->pos().y());
    }
}

/// @brief Executes the simulation with the specified mode and exports data to a JSON file.
///
/// This function runs the simulation based on the specified mode and exports the system data to a JSON file.
/// If the mode is non-zero, it only exports the data without executing the simulation.
///
/// @param mode An integer indicating the mode: 0 for simulation mode, 1 for data export mode.
/// @param path The path to the JSON file where the system data will be exported.
void MainWindow::runIt(int mode, string path)
{
    json systemData;
    json globalData;
    json automataArray;
    json automatonData;
    json nodeArray;
    json transitionsArray;
    json transitionData;
    json variablesArray;
    json variableData;
    *stop = false;
    *pause = false;

    setlocale(LC_ALL, "C");

    QMap<std::string, QGraphicsEllipseItem *> mappetta;

    int debugMode = switchDebug->isChecked() ? 1 : 0;
    if (debugMode)
    {
        ui->tabWidget->setCurrentIndex(1);
        *pause = true;
    }
    ui->showVariables->setVisible(true);
    string tempAux = ui->deltaSpinBox->text().toStdString();
    globalData["delta"] = replaceCommasWithPeriods(tempAux);
    globalData["finaltime"] = ui->finalTimeSpinBox->text().toStdString();

    transitionsArray.clear();
    for (int x = 0; x < automatas.size(); x++)
    {
        for (int i = 0; i < circles.size(); i++)
        {
            if (circles[i]->automata == automatas[x])
            {
                json nodeData;
                nodeData["name"] = circles[i]->name.toStdString();
                nodeData["description"] = circles[i]->description.toStdString();
                nodeData["instructions"] = circles[i]->instructions.toStdString();
                nodeData["x"] = circles[i]->ellipse->sceneBoundingRect().center().x();
                nodeData["y"] = circles[i]->ellipse->sceneBoundingRect().center().y();
                if (circles[i]->startNode)
                    nodeData["flag"] = "start";
                else
                    nodeData["flag"] = "none";
                QList<QGraphicsEllipseItem *> destinations = arrows[circles[i]->ellipse];
                for (int j = 0; j < destinations.size(); j++)
                {
                    for (int x = 0; x < circles.size(); x++)
                    {
                        if (circles[x]->ellipse->sceneBoundingRect().center() == destinations[j]->sceneBoundingRect().center())
                        {
                            transitionData["to"] = circles[x]->name.toStdString();
                            break;
                        }
                    }

                    for (int x = 0; x < drawnArrows.size(); x++)
                    {
                        if (drawnArrows[x]->startItem->sceneBoundingRect().center() == circles[i]->ellipse->sceneBoundingRect().center() &&
                            drawnArrows[x]->endItem->sceneBoundingRect().center() == destinations[j]->sceneBoundingRect().center())
                        {
                            transitionData["condition"] = drawnArrows[x]->textItem->toPlainText().toStdString();
                            break;
                        }
                    }
                    transitionsArray.push_back(transitionData);
                }
                nodeData["transitions"] = transitionsArray;
                nodeArray.push_back(nodeData);
                transitionsArray.clear();
            }
        }
        variablesArray.clear();
        if (x == 0)
        {
            QMap<QString, QString>::const_iterator it;
            for (it = variablesValues.constBegin(); it != variablesValues.constEnd(); ++it)
            {
                json variableData;
                variableData["name"] = it.key().toStdString();
                variableData["value"] = it.value().toStdString();
                variablesArray.push_back(variableData);
            }
        }
        automatonData["name"] = automatas[x].toStdString();
        automatonData["node"] = nodeArray;
        automatonData["variables"] = variablesArray;
        automataArray.push_back(automatonData);
        nodeArray.clear();
    }

    json test;
    systemData["global"] = globalData;
    systemData["automata"] = automataArray;
    test["system"] = systemData;
    std::string jsonData = test.dump(4);

    std::ofstream outFile(path);

    if (outFile.is_open())
    {
        outFile << jsonData;
        outFile.close();
    }
    else
    {
        qDebug() << "Error: Unable to open output file\n";
    }

    if (mode)
    {
        ui->showVariables->setVisible(false);
        return;
    }

    std::vector<string> auxVar;
    std::ifstream fileTemp(inputFile.toStdString());
    std::ifstream file(inputFile.toStdString());
    std::ifstream fileLookahead(inputFile.toStdString());
    std::string tempStringa;
    std::getline(file, tempStringa);
    std::getline(fileLookahead, tempStringa);
    std::getline(fileLookahead, tempStringa);
    double startInputTime;
    double deltaSim;
    char separator = ',';
    if (inputFile != "void")
    {

        if (!fileTemp.is_open())
        {
            return;
        }

        std::string line;
        if (std::getline(fileTemp, line))
        {
            std::istringstream iss(line);
            std::vector<std::string> cells;
            std::string cell;
            int count = 0;
            if (line.find(';') != string::npos)
                separator = ';';
            while (std::getline(iss, cell, separator))
            {
                auxVar.push_back(cell);
            }
        }

        vector<string> tempValue;
        if (std::getline(fileTemp, line))
        { // first row
            tempValue = split_string(line, separator);
            setlocale(LC_ALL, "C");
            startInputTime = stod(tempValue[0]);
        }
        if (std::getline(fileTemp, line))
        { // second row
            tempValue = split_string(line, separator);
            setlocale(LC_ALL, "C");
            deltaSim = stod(tempValue[0]) - startInputTime;
        }

        fileTemp.close();
    }

    long start = time(NULL);
    UtilsJson j;

    System s = j.ScrapingJson(path);
    finalTime = s.numSeconds;
    delta = s.delta;
    v = s.getAutomata();
    runningStatus = true;

    for (int i = 0; i < v.size(); i++)
    {
        vector<Node> vettoreNodi = v[i].getNodes();
        for (int j = 0; j < vettoreNodi.size(); j++)
        {
            for (int z = 0; z < circles.size(); z++)
            {
                if (circles[z]->automata.toStdString() == v[i].getName() && vettoreNodi[j].getName() == circles[z]->name.toStdString())
                {
                    mappetta[vettoreNodi[j].getName() + "~" + v[i].getName()] = circles[z]->ellipse;
                }
            }
        }
    }

    istanti = 0;

    try
    {
#ifdef WINDOWS
        csvfile csv("../export.csv", true);
#else
        csvfile csv("../export.csv", true);
#endif
        // throws exceptions!
        csv << "TIMES";
        for (auto const &key : s.getAutomataDependence())
        {
            csv << key.first;
        }
        csv << endrow;
    }
    catch (const exception &ex)
    {
        qDebug() << "Exception was thrown: " << ex.what();
    }
    updateVariables.clear();
    for (const auto &pair : s.getVariables())
    {
        updateVariables.insert(pair);
    }

    bool leggi = false;
    int nextTime = 0;
    unordered_map<string, double> mapVar;
    for (int i = 1; i < auxVar.size(); i++)
    {
        mapVar[auxVar[i]] = 0;
    }
    for (currentTime = 1; currentTime < s.numSeconds + 1 + 0.000001 - s.delta; currentTime = currentTime + s.delta)
    {

        if (*stop)
            break;
        if (*pause)
            sem_wait(&semaforo);

        // executing all automatas instructions and checking for possible transitions
        bool back = false;
        if (inputFile != "void")
        {

            if (currentTime + 0.000001 >= startInputTime)
            {
                std::getline(fileLookahead, tempStringa);
                std::istringstream iss(tempStringa);
                std::vector<std::string> cells;
                std::string cell;

                if (std::getline(iss, cell, separator))
                {
                    if (cell != "")
                    {
                        startInputTime = stod(cell);
                        leggi = true;
                    }
                }
                else
                {
                    leggi = true;
                }
            }

            std::string line;
            if (leggi)
            {
                leggi = false;
                if (std::getline(file, line))
                {
                    std::istringstream iss(line);
                    std::vector<std::string> cells;
                    std::string cell;
                    int count = 0;
                    while (std::getline(iss, cell, separator))
                    {
                        if (count == 0)
                        {
                            count++;
                            continue;
                        }
                        if (cell != "")
                            mapVar[auxVar[count]] = stod(cell);
                        count++;
                    }
                }
                else
                {
                    // std::cerr << "File is empty: " << inputFile << std::endl;
                }
            }
            for (int j = 0; j < v.size(); j++)
            {
                v[j].currentNode.setFileValues(mapVar);
            }
        }

        for (int j = 0; j < v.size(); j++)
        {
            v[j].checkForChanges();
        }

        // refreshing AutomataVariables
        s.refreshVariables();
        // updateVariables.clear();
        for (const auto &pair : s.getVariables())
        {
            updateVariables[pair.first] = pair.second;
        }

        QList<string> attuali;
        try
        {
#ifdef WINDOWS
            csvfile csv("../export.csv", false);
#else
            csvfile csv("../export.csv", false);
#endif
            csv << currentTime; // timestamp
            for (auto const &key : s.getAutomataDependence())
            {

                // if the variable has not already been assigned then i print 0
                if ((*(v[0].getAutomataVariables())).find(key.first) == (*(v[0].getAutomataVariables())).end())
                {
                    csv << 0;
                }
                else
                {
                    csv << *(*(v[0].getAutomataVariables()))[key.first];
                }
            }

            csv << endrow;
        }
        catch (const exception &ex)
        {
        }

        istanti++;
    }

    for (int z = 0; z < circles.size(); z++)
    {
        QPen outlinePen(Qt::black);
        outlinePen.setWidth(2);
        circles[z]->ellipse->setBrush(QBrush(automataColors[circles[z]->automata]));
        circles[z]->ellipse->setPen(outlinePen);
    }

    ui->stopButton->hide();
    ui->pauseButton->hide();

    setEditStatus(true);
    runningStatus = false;
    ui->frameDebug->hide();
    ui->commands->show();
    ui->showVariables->hide();
    ui->runForButton->setEnabled(true);
    ui->stepButton->setEnabled(true);
    ui->runForButton->setEnabled(true);
    ui->moreSteps->setEnabled(true);
}

/// @brief Slot function called when the "JSON" button is clicked.
/// Saves the system configuration to a JSON file and starts the simulation.
/// Checks for errors such as missing instructions, missing variables, and multiple start nodes.
/// If errors are found, displays appropriate messages and returns without proceeding.
/// Otherwise, saves the system configuration to a JSON file, initializes the simulation,
/// and starts a new thread to run the simulation in the background.
void MainWindow::on_jsonButton_clicked()
{

    string path;
#ifdef WINDOWS
    path = "..//output.json";
#else
    path = "../output.json";
#endif
    for (int i = 0; i < circles.size(); i++)
    {
        if (circles[i]->instructions == "default")
        {
            QMessageBox::information(nullptr, "Error", "You need to set the instructions of all the nodes in the system");
            return;
        }
    }
    if (actualVariables.isEmpty())
    {
        QMessageBox::information(nullptr, "Error", "No variables in the system");
        return;
    }

    bool foundError = false;
    if (circles.empty())
    {
        foundError = true;
    }
    int count = 0;
    for (int i = 0; i < automatas.size(); i++)
    {
        for (int j = 0; j < circles.size(); j++)
        {
            if (circles[j]->automata == automatas[i])
            {
                if (circles[j]->startNode)
                {
                    count++;
                }
            }
        }
        if (count == 0)
        {
            QMessageBox::information(nullptr, "Error", "No Start found in " + automatas[i]);
            return;
        }
        else if (count > 1)
        {
            QMessageBox::information(nullptr, "Error", "Multiple start nodes found in " + automatas[i]);
            return;
        }

        count = 0;
    }
    QMap<QString, QString>::const_iterator checkVariablesValues;
    for (checkVariablesValues = variablesValues.constBegin(); checkVariablesValues != variablesValues.constEnd(); ++checkVariablesValues)
    {
        if (checkVariablesValues.value().toStdString() == "NaN")
        {
            std::string error = "You can't run the system because the variable '" + checkVariablesValues.key().toStdString() + "' hasn't a initial value!";
            QString qError = QString::fromStdString(error);
            QMessageBox::information(nullptr, "Warning", qError);
            foundError = true;
        }
    }
    if (foundError)
    {
        QMessageBox::information(nullptr, "Warning", "Check your system, something went wrong");
        return;
    }

    setEditStatus(false);
    tempMap.clear();

    for (int k = 0; k < circles.size(); k++)
    {
        QString t = circles[k]->name + "~" + (circles[k]->automata);
        tempMap[t.toStdString()] = circles[k]->ellipse;
    }

    ui->frameDebug->show();
    ui->commands->hide();
    ui->pauseButton->setText("PAUSE");
    ui->moreSteps->setValue(0);
    sem_destroy(&semaforo);
    sem_init(&semaforo, 0, 0);
    // Allows the thread to run in the background.
    std::thread thread_obj(&MainWindow::runIt, this, 0, path);
    thread_obj.detach();
    if (!switchDebug->isChecked())
    {
        ui->stopButton->show();
        ui->pauseButton->show();
        ui->stepButton->hide();
        ui->runForButton->hide();
        ui->moreSteps->hide();
    }
    else
    {
        ui->stopButton->show();
        ui->stepButton->show();
        ui->runForButton->show();
        ui->moreSteps->show();
    }
}

/// @brief Slot function called when the "Add Automaton" button is clicked.
/// Adds a new automaton with the specified name to the system.
/// If no name is provided or if an automaton with the same name already exists, shows an appropriate message.
/// Generates a random color for the new automaton and adds it to the list of automata.
void MainWindow::on_addAutoma_clicked()
{

    if (ui->automataName->text().isEmpty())
    {
        QMessageBox::information(nullptr, "Information", "Please insert a name for the automa");
        return;
    }
    if (automatas.contains(ui->automataName->text()))
    {
        QMessageBox::information(nullptr, "Information", "An automata with this name already exist");
        return;
    }
    QList<QColor> colors;
    colors << QColor(Qt::cyan)
           << QColor(Qt::magenta)
           << QColor(Qt::red)
           << QColor(Qt::darkRed)
           << QColor(Qt::darkCyan)
           << QColor(Qt::darkMagenta)
           << QColor(255, 117, 20)
           << QColor(0, 102, 204)
           << QColor(Qt::gray)
           << QColor(0, 255, 0);

    QColor randomColor;

    if (colors_left == 0)
    {

        counter_colors.fill(1);
        colors_left = counter_colors.size();
    }

    while (true)
    {
        // Generate a random index within the range of the list
        int randomIndex = QRandomGenerator::global()->bounded(colors.size());

        // Retrieve the color at the random index
        randomColor = colors[randomIndex];

        if (counter_colors[randomIndex] == 1)
        {
            counter_colors[randomIndex] -= 1;
            break;
        }
    }
    colors_left -= 1;

    QString newAutomata = ui->automataName->text();
    automataColors[newAutomata] = randomColor;
    automatas.append(newAutomata);
    ui->automatasList->clear();
    ui->automatasList->addItems(automatas);
    ui->automataName->clear();
}

/// @brief Slot function called when the "Load Data" button is clicked.
/// Opens a file dialog to select a JSON file containing system data.
/// If no file path is selected, returns without loading.
/// Otherwise, clears existing data and loads the data from the selected JSON file.
void MainWindow::on_loadData_clicked()
{

    QString filePath = QFileDialog::getOpenFileName(this, tr("Seleziona un file JSON"), QDir::currentPath(), tr("File JSON (*.json)"));

    if (filePath.isEmpty())
    {
        return;
    }

    clearAll(1);
    std::ifstream f(filePath.toStdString());
    json data = json::parse(f);
    setlocale(LC_ALL, "C");

    string h_string = data["system"]["global"]["delta"];
    double system_delta = stod(h_string);
    ui->deltaSpinBox->setValue(system_delta);

    h_string = data["system"]["global"]["finaltime"];
    double system_finaltime = stod(h_string);
    ui->finalTimeSpinBox->setValue(system_finaltime);

    QList<QColor> colors;
    colors << QColor(Qt::cyan)
           << QColor(Qt::magenta)
           << QColor(Qt::red)
           << QColor(Qt::darkRed)
           << QColor(Qt::darkCyan)
           << QColor(Qt::darkMagenta)
           << QColor(255, 117, 20)
           << QColor(0, 102, 204)
           << QColor(Qt::gray)
           << QColor(0, 255, 0);

    // find all the automata in settings.json
    for (json automata : data["system"]["automata"])
    {
        automatas.append(QString::fromStdString(automata["name"]));
        bool found = true;
        QColor randomColor;
        while (true)
        {

            if (colors_left == 0)
            {
                counter_colors.fill(1);
                colors_left = counter_colors.size();
            }

            // Generate a random index within the range of the list
            int randomIndex = QRandomGenerator::global()->bounded(colors.size());

            // Retrieve the color at the random index
            randomColor = colors[randomIndex];
            if (counter_colors[randomIndex] == 1)
            {
                counter_colors[randomIndex] -= 1;
                break;
            }
        }
        colors_left -= 1;
        automataColors[QString::fromStdString(automata["name"])] = randomColor;
    }
    ui->automatasList->addItems(automatas);
    QMap<QString, QGraphicsEllipseItem *> mappetta;
    for (json automata : data["system"]["automata"])
    {
        for (json node : automata["node"])
        {
            QPointF point(node["x"], node["y"]);
            QBrush brush(automataColors[QString::fromStdString(automata["name"])]);
            QPen outlinePen(QColor(0, 0, 0));
            outlinePen.setWidth(2);
            QGraphicsEllipseItem *newEllipse = new QGraphicsEllipseItem(point.x(), point.y(), 80, 80);
            newEllipse->setPen(outlinePen);
            newEllipse->setBrush(brush);
            string temp = node["name"];
            QGraphicsTextItem *textLabel = new QGraphicsTextItem(temp.c_str());
            textLabel->setDefaultTextColor(Qt::white);
            textLabel->setFont(QFont("Arial", 10));
            textLabel->setPos(point.x() + 10, point.y() + 10);
            CircleItem *circleItem = new CircleItem(newEllipse, textLabel, QString::fromStdString(automata["name"]));
            ellipseMap[newEllipse] = circleItem;
            circleItem->setFlag(QGraphicsItem::ItemIsMovable);
            circleItem->setFlag(QGraphicsItem::ItemIsSelectable);
            circleItem->description = QString::fromStdString(node["description"]);
            circleItem->name = QString::fromStdString(node["name"]);
            circleItem->instructions = QString::fromStdString(node["instructions"]);
            mappetta[circleItem->name + "~" + (circleItem->automata)] = newEllipse;
            temp = node["flag"];

            if (temp == "start")
            {
                circleItem->startNode = true;
            }
            else
            {
                circleItem->startNode = false;
            }

            circles.append(circleItem);
            scene->addItem(circleItem);
            hideDesignerInput();
        }
    }

    for (json automata : data["system"]["automata"])
    {
        for (json node : automata["node"])
        {
            for (json transition : node["transitions"])
            {
                ArrowItem *arrow;
                if (arrows.contains(mappetta[QString::fromStdString(node["name"]) + "~" + (QString::fromStdString(automata["name"]))]))
                {
                    QList<QGraphicsEllipseItem *> dest = arrows[mappetta[QString::fromStdString(node["name"]) + "~" + (QString::fromStdString(automata["name"]))]];
                    if (!dest.contains(mappetta[QString::fromStdString(transition["to"]) + "~" + (QString::fromStdString(automata["name"]))]))
                    {
                        arrow = new ArrowItem(mappetta[QString::fromStdString(node["name"]) + "~" + (QString::fromStdString(automata["name"]))], mappetta[QString::fromStdString(transition["to"]) + "~" + (QString::fromStdString(automata["name"]))]);
                        arrow->setFlag(QGraphicsItem::ItemIsSelectable);
                        arrow->textItem->setPlainText(QString::fromStdString(transition["condition"]));
                        scene->addItem(arrow);
                        drawnArrows.append(arrow);
                        arrows[mappetta[QString::fromStdString(node["name"]) + "~" + (QString::fromStdString(automata["name"]))]].append(mappetta[QString::fromStdString(transition["to"]) + "~" + (QString::fromStdString(automata["name"]))]);
                    }
                }
                else
                {
                    arrow = new ArrowItem(mappetta[QString::fromStdString(node["name"]) + "~" + (QString::fromStdString(automata["name"]))], mappetta[QString::fromStdString(transition["to"]) + "~" + (QString::fromStdString(automata["name"]))]);
                    arrow->setFlag(QGraphicsItem::ItemIsSelectable);
                    arrow->textItem->setPlainText(QString::fromStdString(transition["condition"]));
                    scene->addItem(arrow);
                    drawnArrows.append(arrow);
                    arrows.insert(mappetta[QString::fromStdString(node["name"]) + "~" + (QString::fromStdString(automata["name"]))], QList<QGraphicsEllipseItem *>());
                    arrows[mappetta[QString::fromStdString(node["name"]) + "~" + (QString::fromStdString(automata["name"]))]].append(mappetta[QString::fromStdString(transition["to"]) + "~" + (QString::fromStdString(automata["name"]))]);
                }
            }
        }
        for (json variables : automata["variables"])
        {
            variablesValues[QString::fromStdString(variables["name"])] = QString::fromStdString(variables["value"]);
            actualVariables.append(QString::fromStdString(variables["name"]));
        }
    }

    ui->tabWidget->setCurrentIndex(1);
}

/// @brief Slot function called when an item in the "List Variables" QListWidget is double-clicked.
/// Extracts the variable name from the double-clicked item's text and sets it as the current index
/// in the "Select Variable" combo box.
void MainWindow::on_listVariables_itemDoubleClicked(QListWidgetItem *item)
{
    QString testo = item->text().split('=')[0];
    int index = actualVariables.indexOf(item->text());
    ui->selectVariable->setCurrentIndex(actualVariables.indexOf(item->text().split('=')[0]));
    ui->variableValue->setValue(ui->listVariables->currentItem()->text().split('=')[1].toDouble());
}

/// @brief Slot function called when the current index of the "Select Variable" combo box changes.
/// Sets the current row of the "List Variables" QListWidget to match the index of the combo box.
void MainWindow::on_selectVariable_currentIndexChanged(int index)
{
    ui->listVariables->setCurrentRow(index);
}

/// @brief Slot function called when the "Save Data" button is clicked.
/// Opens a file dialog to select a location to save JSON data.
void MainWindow::on_saveData_clicked()
{
    QString filePath = QFileDialog::getSaveFileName(this, tr("Salva il file JSON"), QDir::currentPath(), tr("File JSON (*.json)"));

    if (filePath.isEmpty())
    {
        return;
    }
    runIt(1, filePath.toStdString());
}

/// @brief Slot function called when the "Pause" button is clicked.
/// Toggles between pausing and resuming the execution of debugging steps.
void MainWindow::on_pauseButton_clicked()
{
    if (ui->pauseButton->text() == "PAUSE")
    {
        *pause = true;
        ui->pauseButton->setText("RESUME");
    }
    else
    {
        sem_post(&semaforo);
        *pause = false;
        ui->pauseButton->setText("PAUSE");
        ui->stopButton->setEnabled(true);
    }
}

/// @brief Slot function called when the "Stop" button is clicked.
/// Sets the stop flag to true, posts to a semaphore to signal a step execution,
/// and hides the "Show Variables" widget.
void MainWindow::on_stopButton_clicked()
{
    *stop = true;
    sem_post(&semaforo);
    ui->showVariables->hide();
}

/// @brief Runs debugging steps for the specified number of iterations.
/// Posts to a semaphore for each step to signal their execution.
/// @param steps The number of debugging steps to execute.
void MainWindow::runDebuggingSteps(int steps)
{

    for (int i = 0; i < steps; i++)
    {
        sem_post(&semaforo);
    }
}

/// @brief Slot function called when the "Run For" button is clicked.
/// Initiates debugging steps for a specified number of iterations.
/// @note Initializes a thread to run the debugging steps concurrently.
void MainWindow::on_runForButton_clicked()
{
    QString number = ui->moreSteps->text();
    bool ciao;
    int steps = number.toInt(&ciao);
    int left = (((finalTime + 1) - currentTime) / delta) + 1;
    ui->moreSteps->setValue(std::min(steps, left - 1));
    goalStep = istanti + steps;
    ui->runForButton->setEnabled(false);
    ui->stepButton->setEnabled(false);
    ui->runForButton->setEnabled(false);
    ui->moreSteps->setEnabled(false);
    // Allows the thread to run in the background.
    std::thread thread_obj(&MainWindow::runDebuggingSteps, this, std::min(steps, left));
    thread_obj.detach();
}

/// @brief Slot function called when the "Step" button is clicked.
/// Posts to a semaphore to signal that a step should be executed.
void MainWindow::on_stepButton_clicked()
{
    sem_post(&semaforo);
}

/// @brief Slot function called when the "Input File" button is clicked.
/// Opens a file dialog to select a CSV file, sets the selected file path to the 'inputFile' variable,
/// and updates the text of the "Input File" button to display the selected file name.
void MainWindow::on_inputFileButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Seleziona un file CSV"), QDir::currentPath(), tr("File CSV (*.csv)"));
    inputFile = filePath;
    if (filePath.isEmpty())
    {
        inputFile = "void";
        return;
    }
    std::ifstream f(filePath.toStdString());
    ui->inputFileButton->setText(filePath.split('/').last());
    ui->discardInput->show();
}

/// @brief Slot function called when the "Discard Input" button is clicked.
void MainWindow::on_discardInput_clicked()
{
    ui->discardInput->hide();
    ui->inputFileButton->setText("IMPORT INPUT FILE");
    inputFile = "void";
}
