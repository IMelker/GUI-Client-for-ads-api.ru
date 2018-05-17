#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUrlQuery>
#include <QListWidget>
#include <QString>
#include <QMap>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    enum GetCheckedType{kListId, kToolTip, kName};
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    void SetupConfigs();
    void LoadParametersFromFile(QMap<QString,QString>& map, const QString& path);
    void SetupCheckedInList(const QString& value, QListWidget* list_widget);
    void SetDisabledWhileRun(bool disable);
    void LogMessage(const QString& msg, QString color);

    bool FormQuery(QUrlQuery* query);
    QString GetCheckedAsCSV(QListWidget* list, const QString& del, GetCheckedType get_type);
    void ClearStringFromTrash(QString& data);

    void HandleResponse(const QJsonDocument& response);
    void JSONArrayToText(const QJsonArray& array, QString& value, const QString& del = ",");
    QStringList GetCheckedList(QListWidget* list);
    int GetCheckedCount(QListWidget* list);

    Ui::MainWindow *ui_;
    QMap<QString, QString> configs_;
    QString file_path_;
    QString last_date_;
    int limit_per_query_;
    int last_bulletin_count_;
    int last_startid_;
    int parts_count_;
    int bulletin_sum_;
    bool interrupt_signal_;
    bool serf_by_date_;

private slots:
    void SendQuery();
    void StopRun();
    void SaveCurrentParameters();
    void ChangeDefaultLimit(bool is_test_account);
};

#endif // MAINWINDOW_H
