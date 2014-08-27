#ifndef IQMAINWINDOW_H
#define IQMAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QTimer>
#include <QDate>

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
    void startStop();
    void saveSettings() const;
    void showAbout();
    void showAboutQt();

private:
    Ui::IqMainWindow *ui;
    QSqlDatabase _db;
    QTimer *_backupTimer;
    QDate _lastBackupDate;

    static IqMainWindow *_instanse;

    void loadSettings();
    QString dumpSqlTable(const QString &tableName, const QStringList &columns = QStringList(), QSqlDatabase db = QSqlDatabase());
};

#endif // IQMAINWINDOW_H
