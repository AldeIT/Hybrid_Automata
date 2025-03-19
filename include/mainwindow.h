#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#define MAXVALUE 1000000

#include <QMainWindow>
#include <QPainter>
#include <QFrame>
#include <QDialog>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTimer>
#include <QGraphicsItem>
#include <QLineEdit>
#include <QMap>
#include <QMouseEvent> // Include the necessary header for QMouseEvent
#include "../include/circleitem.h"
#include "../include/arrowitem.h"
#include "qlistwidget.h"
#include <QVBoxLayout>
#include <semaphore.h>
#include <string>
#include <unordered_map>

namespace Ui
{
    class MainWindow;
}

class Automata;

class Switch; // forward declaration

/// @brief class for the MainWindow.
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QWidget *leftWidget;  // The left QFrame
    QWidget *rightWidget; // The right QFrame
    QGraphicsScene *scene;
    QGraphicsEllipseItem *ellipse;
    QGraphicsEllipseItem *selectedCircle1 = nullptr;
    QGraphicsEllipseItem *selectedCircle2 = nullptr;
    QList<CircleItem *> circles;
    QTimer *timer;
    QMap<QGraphicsEllipseItem *, QList<QGraphicsEllipseItem *>> arrows;
    QMap<QString, QString> variablesValues;
    QMap<QGraphicsEllipseItem *, CircleItem *> ellipseMap;
    QMap<QString, QColor> automataColors;
    QList<QString> actualVariables;
    QList<ArrowItem *> drawnArrows;
    QLineEdit *valueLabel;
    ArrowItem *selectedArrow;
    Switch *switchDebug;
    QList<QString> automatas;
    int finalTime;
    double delta;
    QList<int> posUpdateButton;
    bool isCircleSelected = false;
    bool ascendingSelection = true;
    bool dragMode = false;
    bool runningStatus = false;
    void deleteSelectedItems();
    void handleSelectionChanged();
    void runIt(int mode, std::string path);
    void handleRefresh();
    bool checkSelected();
    void hideDesignerInput();
    void setEditStatus(bool mode);
    void showDesignerInput(int mode);
    void clearAll(int mode);
    void runDebuggingSteps(int steps);
    QVBoxLayout *debugSpaceLayout;
    bool *stop;
    bool *pause;
    QString inputFile = "void";
    sem_t semaforo;
    std::vector<Automata> v;
    int _count;
    double currentTime;
    QMap<std::string, QGraphicsEllipseItem *> tempMap;
    QList<double> trasparenze;
    int ct;
    int goalStep;
    int istanti;
    std::unordered_map<std::string, double> updateVariables;
    QVector<int> counter_colors;
    int colors_left = 10;

protected:
    bool eventFilter(QObject *watched, QEvent *event); // Declare eventFilter function

private slots:
    void on_updateButton_clicked();
    void on_tabWidget_currentChanged(int index);
    void on_updtateVariable_clicked();
    void on_jsonButton_clicked();
    void on_addAutoma_clicked();
    void on_loadData_clicked();
    void on_listVariables_itemDoubleClicked(QListWidgetItem *item);
    void on_selectVariable_currentIndexChanged(int index);
    void on_saveData_clicked();
    void on_pauseButton_clicked();
    void on_stopButton_clicked();
    void on_runForButton_clicked();
    void on_stepButton_clicked();
    void on_inputFileButton_clicked();
    void on_discardInput_clicked();
};

#endif // MAINWINDOW_H
