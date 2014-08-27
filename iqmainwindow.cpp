#include "iqmainwindow.h"
#include "ui_iqmainwindow.h"
#include <QDebug>
#include <QSqlError>
#include <QStandardPaths>
#include <QSqlQuery>
#include <QSqlDriver>
#include <QSqlRecord>
#include <QSettings>
#include <QMessageBox>
#include <QFile>
#include <QTextCodec>

IqMainWindow* IqMainWindow::_instanse = NULL;

IqMainWindow::IqMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::IqMainWindow),
    _db(QSqlDatabase::addDatabase("QODBC")),
    _backupTimer(new QTimer(this))
{
    ui->setupUi(this);
    IqMainWindow::_instanse = this;

    connect(_backupTimer, SIGNAL(timeout()), this, SLOT(checkTime()));
    connect(ui->startStopPushButton, SIGNAL(clicked()), this, SLOT(startStop()));
    connect(ui->aboutPushButton, SIGNAL(clicked()), this, SLOT(showAbout()));
    connect(ui->aboutQtPushButton, SIGNAL(clicked()), this, SLOT(showAboutQt()));

    loadSettings();

    connect(ui->innCounterpartyLineEdit, SIGNAL(textChanged(QString)), this, SLOT(saveSettings()));
    connect(ui->backupTimeTimeEdit, SIGNAL(timeChanged(QTime)), this, SLOT(saveSettings()));
    connect(ui->odbConnectionLineEdit, SIGNAL(textChanged(QString)), this, SLOT(saveSettings()));
    connect(ui->sqlUserLineEdit, SIGNAL(textChanged(QString)), this, SLOT(saveSettings()));
    connect(ui->sqlPasswordLineEdit, SIGNAL(textChanged(QString)), this, SLOT(saveSettings()));
    connect(ui->ftpServerLineEdit, SIGNAL(textChanged(QString)), this, SLOT(saveSettings()));
    connect(ui->ftpUserLineEdit, SIGNAL(textChanged(QString)), this, SLOT(saveSettings()));
    connect(ui->ftpPasswordLineEdit, SIGNAL(textChanged(QString)), this, SLOT(saveSettings()));
    connect(ui->fileNameLineEdit, SIGNAL(textChanged(QString)), this, SLOT(saveSettings()));
    connect(ui->pathLineEdit, SIGNAL(textChanged(QString)), this, SLOT(saveSettings()));
}

IqMainWindow::~IqMainWindow()
{
    delete ui;
}

void IqMainWindow::startStop()
{
    if (_backupTimer->isActive())
    {
        ui->startStopPushButton->setText(tr("Start"));
        _backupTimer->stop();
    }
    else
    {
        ui->startStopPushButton->setText(tr("Stop"));
        _lastBackupDate = QDate();
        _backupTimer->start(10000);
        checkTime();
    }
}

void IqMainWindow::writeToLog(const QtMsgType &debugType, const QString &msg)
{
    if (_instanse)
    {
        switch (debugType)
        {
        case QtDebugMsg:
            _instanse->ui->logsPlainTextEdit->appendPlainText(tr("DEBUG: %0").arg(msg.trimmed()));
            break;
        case QtWarningMsg:
            _instanse->ui->logsPlainTextEdit->appendPlainText(tr("WARNING: %0").arg(msg.trimmed()));
            break;
        case QtCriticalMsg:
            _instanse->ui->logsPlainTextEdit->appendPlainText(tr("CRITICAL: %0").arg(msg.trimmed()));
            break;
        case QtFatalMsg:
            _instanse->ui->logsPlainTextEdit->appendPlainText(tr("FATAL: %0").arg(msg.trimmed()));
            break;
        }
    }
}

void IqMainWindow::checkTime()
{
    if (!_lastBackupDate.isValid())
    {
        backup();
    }
    else
    {
        if (QTime::currentTime() >= ui->backupTimeTimeEdit->time()
                && _lastBackupDate != QDate::currentDate())
        {
            backup();
        }
    }
}

void IqMainWindow::backup()
{
    qDebug() << tr("STARTING BACKUP...");
    qDebug() << tr("Connecting to DB...");
    _lastBackupDate = QDate::currentDate();
    //Подключаемся к БД
    _db.setDatabaseName(ui->odbConnectionLineEdit->text());
    _db.setUserName(ui->sqlUserLineEdit->text());
    _db.setPassword(ui->sqlPasswordLineEdit->text());
    if (!_db.open())
    {
        qWarning() << tr("Can not connect to DB. Error: %0")
                      .arg(_db.lastError().text());
        return;
    }
    qDebug() << tr("Connection to DB complected.");

    //Выгружаем данные
    qDebug() << tr("Creating backup data...");
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);

    QString data;
    data += ui->innCounterpartyLineEdit->text() + "\n";
    data += dumpSqlTable("MV_COUNTRY",  QStringList(), _db);
    qDebug() << tr("Creating backup data completed.");

    qDebug() << tr("Writing data to temp file \"%0\"...")
                .arg(tempDir + "/" + ui->fileNameLineEdit->text());
    if (ui->fileNameLineEdit->text().isEmpty())
    {
        qWarning() << tr("File name must be set.");
        return;
    }
    QFile tmpFile (tempDir + "/" + ui->fileNameLineEdit->text());
    if (!tmpFile.open(QFile::WriteOnly | QIODevice::Text))
    {
        qWarning() << tr("Can not open file \"%0\".")
                      .arg(tempDir + "/" + ui->fileNameLineEdit->text());
        return;
    }

    QTextStream ts (&tmpFile);
    ts.setCodec(QTextCodec::codecForName("CP1251"));
    ts << data;
    tmpFile.close();
    qDebug() << tr("Writing data to temp file completed.");
    qDebug() << tr("BACKUP FINISHED.");
}

void IqMainWindow::loadSettings()
{
    QSettings settings;
    ui->innCounterpartyLineEdit->setText(settings.value("inn").toString());
    ui->backupTimeTimeEdit->setTime(settings.value("startTime", QTime(4, 0)).toTime());
    settings.beginGroup("sqlSettings");
    ui->odbConnectionLineEdit->setText(settings.value("odbcName").toString());
    ui->sqlUserLineEdit->setText(settings.value("user").toString());
    ui->sqlPasswordLineEdit->setText(settings.value("password").toString());
    settings.endGroup();
    settings.beginGroup("ftpSettings");
    ui->ftpServerLineEdit->setText(settings.value("server").toString());
    ui->ftpUserLineEdit->setText(settings.value("user").toString());
    ui->ftpPasswordLineEdit->setText(settings.value("password").toString());
    ui->fileNameLineEdit->setText(settings.value("fileName", "farma_dump.txt").toString());
    ui->pathLineEdit->setText(settings.value("path").toString());
    settings.endGroup();
}

void IqMainWindow::saveSettings() const
{
    QSettings settings;
    settings.setValue("inn", ui->innCounterpartyLineEdit->text());
    settings.setValue("startTime", ui->backupTimeTimeEdit->time());
    settings.beginGroup("sqlSettings");
    settings.setValue("odbcName", ui->odbConnectionLineEdit->text());
    settings.setValue("user", ui->sqlUserLineEdit->text());
    settings.setValue("password", ui->sqlPasswordLineEdit->text());
    settings.endGroup();
    settings.beginGroup("ftpSettings");
    settings.setValue("server", ui->ftpServerLineEdit->text());
    settings.setValue("user", ui->ftpUserLineEdit->text());
    settings.setValue("password", ui->ftpPasswordLineEdit->text());
    settings.setValue("fileName", ui->fileNameLineEdit->text());
    settings.setValue("path", ui->pathLineEdit->text());
    settings.endGroup();
}

QString IqMainWindow::dumpSqlTable(const QString &tableName, const QStringList &columns, QSqlDatabase db)
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

    QStringList escapedColumns;
    foreach (QString column, columns)
    {
        escapedColumns << sqlDriver->escapeIdentifier(column, QSqlDriver::FieldName);
    }
    QString queryStr = QString("SELECT %0 FROM %1")
            .arg(escapedColumns.count() > 0?escapedColumns.join(","):"*")
            .arg(sqlDriver->escapeIdentifier(tableName, QSqlDriver::TableName));

    qDebug() << tr("Exec \"%0\"")
                .arg(queryStr);

    if (!query.exec(queryStr))
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
            result += values.join(";") + "\n";
        }
        while (query.next());
    }

    return result.trimmed();
}

void IqMainWindow::showAbout()
{

}

void IqMainWindow::showAboutQt()
{
    QMessageBox::aboutQt(this);
}
