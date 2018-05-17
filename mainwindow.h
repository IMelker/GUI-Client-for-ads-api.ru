#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>
#include <QListWidget>
#include <QString>
#include <QMap>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    void SetupConfigs();
    void LoadParametersFromFile(QMap<QString,QString>& map, const QString& path);
    void SetupCheckedInList(const QString& value, QListWidget* list_widget);
    void SetDisabledWhileRun(bool disable);

    bool FormQuery(QUrlQuery* query);
    void HandleResponse(const QJsonDocument& response);
    QString GetCheckedAsCSV(QListWidget* list, const QString& del, bool use_tooltip = true);
    QStringList GetCheckedList(QListWidget* list);
    int GetCheckedCount(QListWidget* list);
    void JSONArrayToText(const QJsonArray& array, QString& value, const QString& del = ",");
    void ClearStringFromTrash(QString& data);
    void LogMessage(const QString& msg, QString color = "");


    Ui::MainWindow *ui;
    QString file_path_;
    QMap<QString, QString> configs_;
    QString last_date_;
    int kLimitPerQuery;
    int last_bulletin_count_;
    int last_startid_;
    int parts_count_;
    int bulletin_sum_;
    bool interrupt_signal;
    bool serf_by_date_;

private slots:
    void SendQuery();
    void StopRun();
    void SaveCurrentParameters();
    void ChangeDefaultLimit(bool is_test_account);
};

#endif // MAINWINDOW_H
