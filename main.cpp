#include "iqmainwindow.h"
#include <QApplication>
#include "iqmainwindow.h"

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context);
    IqMainWindow::writeToLog(type, msg);
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(myMessageOutput);

    QApplication::setOrganizationName("itQuasar");
    QApplication::setApplicationName("iqFarmaDBDamper");
    QApplication a(argc, argv);
    IqMainWindow w;
    w.show();

    return a.exec();
}
