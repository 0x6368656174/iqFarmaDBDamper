#ifndef IQMAINWINDOW_H
#define IQMAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>

namespace Ui {
class IqMainWindow;
}

class IqMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit IqMainWindow(QWidget *parent = 0);
    ~IqMainWindow();

    static void writeToLog(const QtMsgType &debugType, const QString &msg);

public slots:
    void checkTime();
    void backup();

private:
    Ui::IqMainWindow *ui;
    QSqlDatabase _db;

    static IqMainWindow *_instanse;

    QString dumpSqlTable(const QString &tableName, QSqlDatabase db = QSqlDatabase());
};

#endif // IQMAINWINDOW_H
