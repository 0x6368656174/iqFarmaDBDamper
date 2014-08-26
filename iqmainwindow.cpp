#include "iqmainwindow.h"
#include "ui_iqmainwindow.h"
#include <QDebug>
#include <QSqlError>
#include <QStandardPaths>
#include <QSqlQuery>
#include <QSqlDriver>
#include <QSqlRecord>

IqMainWindow* IqMainWindow::_instanse = NULL;

IqMainWindow::IqMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::IqMainWindow),
    _db(QSqlDatabase::addDatabase("QODBC"))
{
    ui->setupUi(this);
    IqMainWindow::_instanse = this;

    connect(ui->startStopPushButton, SIGNAL(clicked()), this, SLOT(backup()));
}

IqMainWindow::~IqMainWindow()
{
    delete ui;
}

void IqMainWindow::writeToLog(const QtMsgType &debugType, const QString &msg)
{
    if (_instanse)
    {
        switch (debugType)
        {
        case QtDebugMsg:
            _instanse->ui->logsPlainTextEdit->appendPlainText(tr("DEBUG: %0\n").arg(msg.trimmed()));
            break;
        case QtWarningMsg:
            _instanse->ui->logsPlainTextEdit->appendPlainText(tr("WARNING: %0\n").arg(msg.trimmed()));
            break;
        case QtCriticalMsg:
            _instanse->ui->logsPlainTextEdit->appendPlainText(tr("CRITICAL: %0\n").arg(msg.trimmed()));
            break;
        case QtFatalMsg:
            _instanse->ui->logsPlainTextEdit->appendPlainText(tr("FATAL: %0\n").arg(msg.trimmed()));
            break;
        }
    }
}

void IqMainWindow::checkTime()
{

}

void IqMainWindow::backup()
{
    //Подключаемся к БД
    _db.setDatabaseName(ui->odbConnectionLineEdit->text());
    _db.setUserName(ui->userLineEdit->text());
    _db.setPassword(ui->passwordLineEdit->text());
    if (!_db.open())
    {
        qWarning() << tr("Can not connect to DB. Error: %0")
                      .arg(_db.lastError().text());
        return;
    }

    //Выгружаем данные
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);

    qDebug() << dumpSqlTable("MV_COUNTRY", _db);
}

QString IqMainWindow::dumpSqlTable(const QString &tableName, QSqlDatabase db)
{
    if (!_db.isOpen())
    {
        qWarning() << tr("DB not open. Can not dump table \"%0\".")
                      .arg(tableName);
        return "";
    }

    QSqlDriver *sqlDriver = db.driver();

    QSqlQuery query (db);
    //Оптимизация
    query.setForwardOnly(true);

    if (!query.exec(QString("SELECT * FROM %0")
                    .arg(sqlDriver->escapeIdentifier(tableName, QSqlDriver::TableName))))
    {
        qWarning() << tr("Failure dump DB table \"%0\". Error: %0")
                      .arg(query.lastError().text());
        return "";
    }

    QString result;
    int columnCount = 0;

    if (query.next())
    {
        columnCount = query.record().count();

        do
        {
            QStringList values;
            for (int i = 0; i < columnCount; i++)
            {
                values << query.value(i).toString();
            }
            result += values.join(", ") + "\n";
        }
        while (query.next());
    }

    return result.trimmed();
}
