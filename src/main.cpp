#include "../include/UtilsJson.h"
#include "../include/tinyexpr.h"
#include "../include/tools.h"

#include <algorithm>
#include <time.h>
#include <random>
#include <thread>
#include <chrono>
#include <ctime>
#include "../include/csvfile.h"

#include "../include/mainwindow.h"
#include <QApplication>
#include <QStyleFactory>

#ifdef DEBUG_MODE
#define DEBUG_COMMENT(comment) std::cout << "[DEBUG] " << comment << std::endl;
#else
#define DEBUG_COMMENT(comment)
#endif

using namespace std;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setStyle(QStyleFactory::create("Fusion"));
    app.setStyleSheet("QApplication { "
                      "   palette: QPalette(Dark); "
                      "   background-color: #333333; "
                      "   color: #F0F0F0; "
                      "   font-family: 'Arial', sans-serif;"
                      "   font-size: 10pt;"
                      "}");

    MainWindow w;
    w.setWindowTitle("CHAOS");
    w.show();
    w.setWindowIcon(QIcon());

    return app.exec();
}
