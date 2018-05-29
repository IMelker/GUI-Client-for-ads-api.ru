#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtNetwork>
#include <QMessageBox>

static const int kDelayMsecs = 5200;
static const int kTestLimitPerQuerty = 50;
static const int kFullLimitPerQuerty = 1000;
static const QString kCSVSeparator = ";";
static const QString kLogColorRed = "#ff0000";
static const QString kLogColorGreen = "#005500";
static const QString kLogColorGray = "#8a8a8a";

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui_(new Ui::MainWindow),
    last_date_(""),
    limit_per_query_(kTestLimitPerQuerty),
    last_bulletin_count_(0),
    last_startid_(0),
    parts_count_(0),
    bulletin_sum_(0),
    interrupt_signal_(false),
    serf_by_date_(true) {

    ui_->setupUi(this);
    file_path_= QCoreApplication::applicationDirPath() + "/data/";
    QDir dir;
    if (!dir.exists(file_path_)) {
        dir.mkpath(file_path_);
    }
    SetupConfigs();

    connect(ui_->pb_send, SIGNAL(clicked()), this, SLOT(SendQuery()));
    connect(ui_->pb_stop, SIGNAL(clicked()), this, SLOT(StopRun()));
    connect(ui_->cb_test_account, SIGNAL(toggled(bool)), this, SLOT(ChangeDefaultLimit(bool)));
    connect(ui_->pb_send, SIGNAL(clicked()), this, SLOT(SaveCurrentParameters()));
}

MainWindow::~MainWindow() {
    disconnect(ui_->pb_send, SIGNAL(clicked()), this, SLOT(SendQuery()));
    disconnect(ui_->pb_stop, SIGNAL(clicked()), this, SLOT(StopRun()));
    disconnect(ui_->cb_test_account, SIGNAL(toggled(bool)), this, SLOT(ChangeDefaultLimit(bool)));
    disconnect(ui_->pb_send, SIGNAL(clicked()), this, SLOT(SaveCurrentParameters()));
    delete ui_;
}

void MainWindow::SetupConfigs() {
    LoadParametersFromFile(configs_, QCoreApplication::applicationDirPath() + "/config.ini");

    ui_->le_user->setText(configs_["user"]);
    ui_->le_token->setText(configs_["token"]);
    if(configs_.contains("test_account")) {
       ui_->cb_test_account->setChecked(true);
       ChangeDefaultLimit(true);
    }
    if(configs_.contains("categories")) {
        ui_->cb_categories->setChecked(true);
        SetupCheckedInList(configs_["categories"], ui_->list_categories);
    }
    if(configs_.contains("source")) {
        ui_->cb_source->setChecked(true);
        const auto& value = configs_["source"];
        if(!value.isEmpty()) {
            ui_->cb_source_2->setCurrentIndex(value.toInt());
        }
    }
    if(configs_.contains("city")) {
        ui_->cb_city->setChecked(true);
        SetupCheckedInList(configs_["city"], ui_->list_city);
        ui_->le_city->setText(configs_["le_city"]);
    }
    if(configs_.contains("metro")) {
        ui_->cb_metro->setChecked(true);
        ui_->le_metro->setText(configs_["metro"]);
    }
    if(configs_.contains("person_type")) {
        ui_->cb_person_type->setChecked(true);
        SetupCheckedInList(configs_["person_type"], ui_->list_person_type);
    }
    if(configs_.contains("nedvigimost_type")) {
        ui_->cb_nedvigimost_type->setChecked(true);
        SetupCheckedInList(configs_["nedvigimost_type"], ui_->list_nedvigimost_type);
    }
    if(configs_.contains("q")) {
        ui_->cb_q->setChecked(true);
        ui_->le_q->setText(configs_["q"]);
    }
    if(configs_.contains("price")) {
        ui_->cb_price->setChecked(true);
        QStringList list = configs_["price"].split("-");
        ui_->sb_price1->setValue(list.at(0).toInt());
        ui_->sb_price2->setValue(list.at(1).toInt());
    }
    if(configs_.contains("startid")) {
        ui_->cb_startid->setChecked(true);
        ui_->sb_startid->setValue(configs_["startid"].toInt());
    }
    if(configs_.contains("date1")) {
        ui_->cb_date1->setChecked(true);
        QDateTime* date_time = new QDateTime();
        date_time->setMSecsSinceEpoch(configs_["date1"].toLongLong());
        ui_->date_date1->setDateTime(*date_time);
    }
    if(configs_.contains("date2")) {
        ui_->cb_date2->setChecked(true);
        QDateTime* date_time = new QDateTime();
        date_time->setMSecsSinceEpoch(configs_["date2"].toLongLong());
        ui_->date_date2->setDateTime(*date_time);
    }
    if(configs_.contains("phone")) {
        ui_->cb_phone->setChecked(true);
        ui_->le_phone->setText(configs_["phone"]);
    }
    if(configs_.contains("limit")) {
        ui_->cb_limit->setChecked(true);
        ui_->sb_limit->setValue(configs_["limit"].toInt());
    }
    SetupCheckedInList(configs_["out"], ui_->list_out);
}

void MainWindow::SetupCheckedInList(const QString& value, QListWidget* list_widget) {
    if(!value.isEmpty()) {
        QStringList list = value.split(",");
        for(QString& str : list) {
            list_widget->item(str.toInt())->setCheckState(Qt::Checked);
        }
    }
}

void MainWindow::LoadParametersFromFile(QMap<QString,QString>& map, const QString& path) {
    map.clear();
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly)) {
        file.close();
        SaveCurrentParameters();
        return;
    }

    QTextStream in(&file);

    while(!in.atEnd()) {
        QString line = in.readLine();
        int ind = line.indexOf("=");
        map[line.left(ind)] = line.mid(ind+1,line.length());
    }
    file.close();
}

void MainWindow::SaveCurrentParameters() {
    configs_.clear();
    configs_["user"] = ui_->le_user->text();
    configs_["token"] = ui_->le_token->text();
    if(ui_->cb_test_account->isChecked()) {
        configs_["test_account"] = "test_account";
    }
    if(ui_->cb_categories->isChecked()) {
        configs_["categories"] = GetCheckedAsCSV(ui_->list_categories, ",", GetCheckedType::kListId);
    }
    if(ui_->cb_source->isChecked()) {
        configs_["source"] = QString::number(ui_->cb_source_2->currentIndex());
    }
    if(ui_->cb_city->isChecked()) {
        configs_["city"] = GetCheckedAsCSV(ui_->list_city, ",", GetCheckedType::kListId);
        configs_["le_city"] = ui_->le_city->text();
    }
    if(ui_->cb_metro->isChecked()) {
        configs_["metro"] = ui_->le_metro->text();
    }
    if(ui_->cb_person_type->isChecked()) {
        configs_["person_type"] = GetCheckedAsCSV(ui_->list_person_type, ",", GetCheckedType::kListId);
    }
    if(ui_->cb_nedvigimost_type->isChecked()) {
        configs_["nedvigimost_type"] = GetCheckedAsCSV(ui_->list_nedvigimost_type, ",", GetCheckedType::kListId);
    }
    if(ui_->cb_q->isChecked()) {
        configs_["q"] = ui_->le_q->text();
    }
    if(ui_->cb_price->isChecked()) {
        configs_["price"] = ui_->sb_price1->text() + "-" +  ui_->sb_price2->text();
    }
    if(ui_->cb_startid->isChecked()) {
        configs_["startid"] = QString::number(ui_->sb_startid->value());
    }
    if(ui_->cb_date1->isChecked()) {
        configs_["date1"] = QString::number(ui_->date_date1->dateTime().toMSecsSinceEpoch());
    }
    if(ui_->cb_date2->isChecked()) {
        configs_["date2"] = QString::number(ui_->date_date2->dateTime().toMSecsSinceEpoch());
    }
    if(ui_->cb_phone->isChecked()) {
        configs_["phone"] = ui_->le_phone->text();
    }
    if(ui_->cb_limit->isChecked()) {
        configs_["limit"] = QString::number(ui_->sb_limit->value());
    }
    configs_["out"] = GetCheckedAsCSV(ui_->list_out, ",", GetCheckedType::kListId);

    QFile file_out(QCoreApplication::applicationDirPath() + "/config.ini");
    QString config_content = "";
    for(auto val : configs_.toStdMap())
    {
        config_content.append(val.first + "=" + val.second + "\n");
    }

    if(!file_out.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
    {
        QMessageBox::critical(this, QString("Ошибка"), "Не удается открыть файл для записи config.ini.");
        return;
    }
    QTextStream write_stream(&file_out);
    write_stream << config_content;
    file_out.close();
}

void debugRequest(QNetworkRequest request)
{
  qDebug() << request.url().toString();
  const QList<QByteArray>& rawHeaderList(request.rawHeaderList());
  foreach (QByteArray rawHeader, rawHeaderList) {
    qDebug() << request.rawHeader(rawHeader);
  }
}

void MainWindow::SendQuery() {
    SetDisabledWhileRun(true);
    interrupt_signal_ = false;
    LogMessage("Начинаем работу.", kLogColorGreen);

    QUrl url("http://ads-api.ru/main/api");
    QUrlQuery *query = new QUrlQuery();
    if(!FormQuery(query)) {
        SetDisabledWhileRun(false);
        return;
    }

    int bulletin_max_sum = 0;
    if(ui_->cb_limit->isChecked()) {
        bulletin_max_sum = ui_->sb_limit->value();
    }
    QNetworkAccessManager manager;
    do {
        int sum_delta = bulletin_max_sum - bulletin_sum_;
        if(bulletin_max_sum && sum_delta <= 0) {
            break;
        } else if (bulletin_max_sum && limit_per_query_ > sum_delta) {
            query->removeQueryItem("limit");
            query->addQueryItem("limit", QString::number(sum_delta));
        }

        if(last_bulletin_count_ == limit_per_query_) {
            if(serf_by_date_) {
                query->removeQueryItem("date2");
                query->addQueryItem("date2", last_date_);
            } else {
                query->removeQueryItem("startid");
                query->addQueryItem("startid", QString::number(last_startid_ + 1));
            }
        }

        url.setQuery(*query);
        QNetworkRequest request(url);
        debugRequest(request);
        QNetworkReply *reply = manager.get(request);
        LogMessage("Получаем ответ от сервера.", kLogColorGray);
        while(!reply->isFinished()) {
            qApp->processEvents();
        }

        QTime dieTime = QTime::currentTime().addMSecs(kDelayMsecs); // for timer
        QByteArray response_data = reply->readAll();
        QJsonDocument json = QJsonDocument::fromJson(response_data);
        HandleResponse(json);

        while (QTime::currentTime() < dieTime) {
            qApp->processEvents();
        }

        ++parts_count_;
        reply->deleteLater();
    } while(last_bulletin_count_ >= limit_per_query_ && !interrupt_signal_);

    if(parts_count_>0) {
        LogMessage("Результаты сохранены в файл: " + file_path_, kLogColorGreen);
        LogMessage("Всего строк: " + QString::number(bulletin_sum_) + ", в " + QString::number(parts_count_) + " частях.", kLogColorGray);
    }

    last_bulletin_count_ = 0;
    last_startid_ = 0;
    parts_count_ = 0;
    bulletin_sum_ = 0;
    file_path_= QCoreApplication::applicationDirPath() + "/data/";

    SetDisabledWhileRun(false);
}

void MainWindow::StopRun() {
    interrupt_signal_ = true;
    ui_->pb_stop->setEnabled(false);
    LogMessage("Обрабатываем последний пакет и останавливаемся.", kLogColorRed);
}

bool MainWindow::FormQuery(QUrlQuery* query) {
    LogMessage("Подготовка запроса объявлений по заданным параметрам.", kLogColorGreen);
    const QString& user_name = ui_->le_user->text();
    if(user_name.isEmpty() ||  ui_->le_token->text().isEmpty()) {
        LogMessage("Не заполнены параметры пользователя.", kLogColorRed);
        QMessageBox::critical(this,"Ошибка","Заполните параметры пользователя.");
        return false;
    }
    if(GetCheckedCount(ui_->list_out)==0) {
        LogMessage("Не выбрано ни одного поля для вывода.", kLogColorRed);
        QMessageBox::critical(this,"Ошибка","Выберите хотя бы одно поле для вывода.");
        return false;
    }

    query->addQueryItem("user", user_name);
    query->addQueryItem("token", ui_->le_token->text());

    file_path_= QCoreApplication::applicationDirPath() + "/data/" + user_name + "/";
    QDir dir;
    if (!dir.exists(file_path_)) {
        dir.mkpath(file_path_);
    }

    file_path_+= "o" + QString::number(GetCheckedCount(ui_->list_out));

    if(ui_->cb_categories->isChecked()) {
        QString cats = GetCheckedAsCSV(ui_->list_categories, ",", GetCheckedType::kToolTip);
        file_path_+="_cat" + cats;
        query->addQueryItem("category_id", cats);
    }
    if(ui_->cb_q->isChecked()) {
        file_path_+="_q" + ui_->le_q->text();
        query->addQueryItem("q", ui_->le_q->text());
    }
    if(ui_->cb_price->isChecked()) {
        QString price1 = QString::number(ui_->sb_price1->value());
        QString price2 = QString::number(ui_->sb_price2->value());
        file_path_+="_p" + price1 + "-" + price2;
        query->addQueryItem("price1", price1);
        query->addQueryItem("price2", price2);
    }

    if(ui_->cb_date1->isChecked()) {
        file_path_+="_d1" + ui_->date_date1->dateTime().toString("yyyy-MM-dd+hh.mm.ss");
        query->addQueryItem("date1", ui_->date_date1->dateTime().toString("yyyy-MM-dd hh:mm:ss"));
        serf_by_date_ = true;
    }
    if(ui_->cb_date2->isChecked()) {
        file_path_+="_d2" + ui_->date_date2->dateTime().toString("yyyy-MM-dd+hh.mm.ss");
        query->addQueryItem("date2", ui_->date_date2->dateTime().toString("yyyy-MM-dd hh:mm:ss"));
        serf_by_date_ = true;
    }
    if(ui_->cb_startid->isChecked()) {
        query->addQueryItem("startid", QString::number(ui_->sb_startid->value()));
        serf_by_date_ = false;
    }
    if(ui_->cb_person_type->isChecked()) {
        QString pt = GetCheckedAsCSV(ui_->list_person_type, ",", GetCheckedType::kToolTip);
        file_path_+="_pt" + pt;
        query->addQueryItem("person_type", pt);
    }
    if(ui_->cb_city->isChecked()) {
        QString regions = GetCheckedAsCSV(ui_->list_city, ",", GetCheckedType::kListId);
        file_path_ += "_c" + regions + (ui_->le_city->text().isEmpty() ? "" : ("," + ui_->le_city->text()));
        regions = GetCheckedAsCSV(ui_->list_city, "|", GetCheckedType::kName)  + (ui_->le_city->text().isEmpty() ? "" : ("|" + ui_->le_city->text()));
        query->addQueryItem("city", regions);

    }
    if(ui_->cb_metro->isChecked()) {
        file_path_+="_m" + ui_->le_metro->text();
        query->addQueryItem("metro", ui_->le_metro->text());
    }
    if(ui_->cb_nedvigimost_type->isChecked()) {
        QString nt = GetCheckedAsCSV(ui_->list_nedvigimost_type, ",", GetCheckedType::kToolTip);
        file_path_ += "_nt" + nt;
        query->addQueryItem("nedvigimost_type", nt);
    }
    if(ui_->cb_phone->isChecked()) {
        query->addQueryItem("phone", ui_->le_phone->text());
    }
    if(ui_->cb_source->isChecked()) {
        QString source = QString::number(ui_->cb_source_2->currentIndex()+1);
        file_path_ += "_s" + source;
        query->addQueryItem("source", source);
    }
    query->addQueryItem("limit", QString::number(limit_per_query_));

    file_path_+= ".csv";
    return true;
}

void MainWindow::SetDisabledWhileRun(bool disable) {
    ui_->pb_send->setDisabled(disable);
    ui_->list_out->setDisabled(disable);
    ui_->pb_stop->setEnabled(disable);
}

QString MainWindow::GetCheckedAsCSV(QListWidget* list, const QString& del, GetCheckedType get_type) {
    QString value = "";

    switch (get_type) {
    case GetCheckedType::kListId:
    {
        for(int row = 0; row < list->count(); row++)
        {
            if(list->item(row)->checkState() == Qt::Checked) {
                if(!value.isEmpty()) {
                    value += del;
                }
                value += QString::number(row);
            }
        }
        break;
    }
    case GetCheckedType::kToolTip:
    {
        for(int row = 0; row < list->count(); row++)
        {
            const auto& item = list->item(row);
            if(item->checkState() == Qt::Checked) {
                if(!value.isEmpty()) {
                    value += del;
                }
                value += item->toolTip();
            }
        }
        break;
    }
    case GetCheckedType::kName:
    {
        for(int row = 0; row < list->count(); row++)
        {
            const auto& item = list->item(row);
            if(item->checkState() == Qt::Checked) {
                if(!value.isEmpty()) {
                    value += del;
                }
                value += item->text();
            }
        }
        break;
    }
    default:
        break;
    }
    return value;
}

QStringList MainWindow::GetCheckedList(QListWidget* list) {
    QStringList values;
    for(int row = 0; row < list->count(); row++)
    {
        if(list->item(row)->checkState() == Qt::Checked) {
            values.append(list->item(row)->text());
        }
    }
    return values;
}

int MainWindow::GetCheckedCount(QListWidget* list) {
    int count = 0;
    for(int row = 0; row < list->count(); row++)
    {
        if(list->item(row)->checkState() == Qt::Checked) {
            count++;
        }
    }
    return count;
}

void MainWindow::HandleResponse(const QJsonDocument& response){
    if (response.isEmpty()) {
       LogMessage("В ответ получили пустое сообщение.", "#ff0000");
       QMessageBox::critical(this, QString("Ошибка"), "В ответ получили пустое сообщение.");
       return;
    }

    QJsonObject jsonObject = response.object();

    if (jsonObject.contains("code") && jsonObject["code"].isDouble()) {
        int code = jsonObject["code"].toInt();
        if(code != 200) {
            QString error_text = "";
            if(jsonObject.contains("error") && jsonObject["error"].isString()) {
                error_text +=  jsonObject["error"].toString();
            } else {
                error_text = "Ошибка при формировании запроса.";
            }

            switch (code) {
            case 429:
            {
                LogMessage(error_text, "#ff0000");
                return;
            }
            case 400:
            case 401:
            case 402:
            case 403:
            case 404:
            case 452:
            case 453:
            case 454:
                {
                    LogMessage(error_text, kLogColorRed);
                    QMessageBox::critical(this, QString("Ошибка"), error_text);
                    StopRun();
                    return;
                }
            default:
                {
                    LogMessage(error_text, kLogColorRed);
                    QMessageBox::critical(this, QString("Ошибка"), "Неизвестная ошибка в ответе от сервера.");
                    StopRun();
                    return;
                }
            }
        }
    }

    if (jsonObject.contains("data") && jsonObject["data"].isArray()) {
        QJsonArray bulletin_array = jsonObject["data"].toArray();
        last_bulletin_count_ = bulletin_array.size();
        bulletin_sum_+=last_bulletin_count_;
        if(last_bulletin_count_ == 0) {
            LogMessage("По данному запросу данные отсутствуют.", kLogColorRed);
            return;
        }

        LogMessage("Обработка и запись " + QString::number(parts_count_+1) + " части(всего " + QString::number(bulletin_sum_) + " строк) в *.csv файл.", kLogColorGray);
        qDebug() << QString(response.toJson(QJsonDocument::Compact));
        qDebug() << file_path_;
        QFile file_out(file_path_);
        if(!file_out.open(QIODevice::Append | QIODevice::WriteOnly))
        {
            LogMessage("Не удается открыть файл для записи ответа. Остановка выполнения запроса.", kLogColorRed);
            QMessageBox::critical(this, QString("Ошибка"), "Не удается открыть файл для записи ответа.");
            interrupt_signal_ = true;
            return;
        }
        QTextStream write_stream(&file_out);
        QStringList params = GetCheckedList(ui_->list_out);
        if (file_out.pos() == 0) {
            QString headers = "";
            for(int i = 0; i < params.size(); ++i) {
                if(i!=0) {
                    headers += kCSVSeparator;
                }
                headers+=params.at(i);
            }
           write_stream << headers + "\n";
        }

        for (int i = 0; i < bulletin_array.size(); ++i) {
            QString value = "";
            QJsonObject json_object = bulletin_array[i].toObject();
            for(int j = 0; j < params.size(); ++j) {
                if(j != 0) {
                    value+=kCSVSeparator;
                }
                const QString& param = params.at(j) ;
                if(json_object.contains(param)) {
                    const auto& param_obj = json_object[param];
                    QString data = "";
                    if(param_obj.isString()) {
                        data = param_obj.toString();
                    } else if (param_obj.isDouble()) {
                        data = QString::number(param_obj.toInt());
                    } else if (param_obj.isArray()) {
                        JSONArrayToText(param_obj.toArray(), data);
                    }  else if (param_obj.isBool()) {
                        data = param_obj.toBool() ? "1" : "0";
                    }
                    ClearStringFromTrash(data);
                    value += data;
                }
            }

            if(i == bulletin_array.size()-1){
                last_date_ = json_object["time"].toString();
                last_startid_ = json_object["id"].toInt();
            }
            write_stream << value + "\n";
        }
        file_out.close();
    }

}

void MainWindow::ClearStringFromTrash(QString& data){
    data.replace(kCSVSeparator, ".");
    data.replace("\n", " ");
}

void MainWindow::JSONArrayToText(const QJsonArray& array, QString& value, const QString& del) {
    for(int i = 0; array.size() < i; ++i) {
        if(i!=0) {
            value+=del;
        }
        const auto& param_obj = array.at(i);
        if(param_obj.isString()) {
            value += param_obj.toString();
        } else if (param_obj.isDouble()) {
            value += QString::number(param_obj.toInt());
        } else if (param_obj.isArray()) {
            JSONArrayToText(param_obj.toArray(), value);
        }  else if (param_obj.isBool()) {
            value += param_obj.toBool() ? "1" : "0";
        }
    }
}

void  MainWindow::LogMessage(const QString& msg, QString color){
    ui_->text_logs->append("<span style=\" color:" + color + ";\">" + msg +  "</span>");
}

void  MainWindow::ChangeDefaultLimit(bool is_test_account) {
     limit_per_query_ = is_test_account ? kTestLimitPerQuerty : kFullLimitPerQuerty;
}
