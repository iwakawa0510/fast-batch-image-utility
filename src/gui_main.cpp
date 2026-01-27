#include "main_window.h"
#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDir>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    // Set application info
    QCoreApplication::setOrganizationName("FBIU");
    QCoreApplication::setApplicationName("Fast Batch Image Utility");
    QCoreApplication::setApplicationVersion("1.0.0");
    
    fbiu::MainWindow window;
    window.show();
    
    return app.exec();
}
