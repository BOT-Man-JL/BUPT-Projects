
//
// Air Conditioner - Server MVC View (GUI View - Qt Implementation)
// Youjie Zhang, 2017
//

#include "ui_welcomewindow.h"
#include "ui_statisticwindow.h"
#include "ui_clientwindow.h"
#include "ui_configwindow.h"
#include "ui_guestwindow.h"

#include "server-view-gui-qt.h"

#include <QHeaderView>
#include <QMessageBox>
#include <QStandardItemModel>

#include <iostream>

WelcomeWindow::WelcomeWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WelcomeWindow)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);
}

WelcomeWindow::~WelcomeWindow()
{
    delete ui;
}

void WelcomeWindow::on_QuitBtn_clicked()
{
    if(_onQuit) {
        _onQuit();
    }
}

void WelcomeWindow::on_ConfigBtn_clicked()
{
    if(_onConfig){
        _onConfig();
    }
}

void WelcomeWindow::on_GuestBtn_clicked()
{
    if(_onGuest){
        _onGuest();
    }
}

void WelcomeWindow::on_StatisticBtn_clicked()
{
    if(_onLog) {
        _onLog();
    }
}

void WelcomeWindow::on_ClientBtn_clicked()
{
    if(_onClient){
        _onClient();
    }
}


GuestWindow::GuestWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GuestWindow)
{
    ui->setupUi(this);

    //Create data model
    _itemModel = new QStandardItemModel();
    _itemModel->setColumnCount(2);
    _itemModel->setHeaderData(0, Qt::Horizontal, QString(QStringLiteral("房间 ID")));
    _itemModel->setHeaderData(1, Qt::Horizontal, QString(QStringLiteral("房客 ID")));

    //Bind data model to the table.
    ui->GuestTable->setModel(_itemModel);
    ui->GuestTable->showGrid();
    ui->GuestTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->GuestTable->setSortingEnabled(true);
    ui->GuestTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->GuestTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    //Hide other buttons.
    ui->ConfirmBtn->hide();
    ui->CancelBtn->hide();
    ui->GuestID->hide();
    ui->GuestIDLabel->hide();
    ui->RoomID->hide();
    ui->RoomIDLabel->hide();
}

GuestWindow::~GuestWindow()
{
    delete _itemModel;
    delete ui;
}

void GuestWindow::closeEvent(QCloseEvent * event){
    if(_onBack)
        _onBack();
    else{
        event->accept();
    }
}

void GuestWindow::on_AddBtn_clicked()
{
    ui->ConfirmBtn->show();
    ui->CancelBtn->show();
    ui->GuestID->show();
    ui->GuestIDLabel->show();
    ui->RoomID->show();
    ui->RoomIDLabel->show();

    ui->GuestTable->hide();
    ui->AddBtn->hide();
    ui->DelBtn->hide();
    ui->QuitBtn->hide();
}

void GuestWindow::on_CancelBtn_clicked()
{
     ui->ConfirmBtn->hide();
     ui->CancelBtn->hide();
     ui->GuestID->hide();
     ui->GuestIDLabel->hide();
     ui->RoomID->hide();
     ui->RoomIDLabel->hide();

     ui->GuestTable->show();
     ui->AddBtn->show();
     ui->DelBtn->show();
     ui->QuitBtn->show();
}

void GuestWindow::on_ConfirmBtn_clicked()
{
    GuestId guestID = ui->GuestID->text().toStdString();
    RoomId roomID = ui->RoomID->text().toStdString();
    try
    {
        GuestInfo guest = GuestInfo{roomID,guestID};
        if (_onAdd) _onAdd(guest);
        _list.emplace_back (std::move (guest));
        QMessageBox::information(this,
                         QString(""),
                         QString(QStringLiteral("添加成功")));
        RefreshGuest();
        on_CancelBtn_clicked();//Return to the previous gui.
    }
    catch (const std::exception &ex)
    {
        std::cerr << ex.what() << std::endl;
        QMessageBox::information(this,
                         QString(QStringLiteral("添加失败")),
                         QString(ex.what()));
        ui->GuestID->clear();
        ui->RoomID->clear();
    }
}


void GuestWindow::on_QuitBtn_clicked()
{
    if(_onBack){
        this->close();
        _onBack();
    }
}

void GuestWindow::LoadGuest(const std::list<GuestInfo> &list)
{
    _list = list;
    RefreshGuest();
}

void GuestWindow::RefreshGuest()
{
    int row = 0;
    _itemModel->removeRows(0,_itemModel->rowCount());
    for(auto i = _list.begin();i != _list.end();++i,++row){
        _itemModel->setItem(row,0,new QStandardItem(QString::fromStdString(i->room)));
        _itemModel->setItem(row,1,new QStandardItem(QString::fromStdString(i->guest)));
    }
}

void GuestWindow::on_DelBtn_clicked()
{
    try{
        QModelIndex ind = ui->GuestTable->currentIndex();
        int row = ind.row();
        RoomId roomId = ind.sibling(row,0).data().toString().toStdString();
        int ret = QMessageBox::information(this,QStringLiteral("提示"),
                       QStringLiteral("确定删除该房客信息？"),QStringLiteral("确定"),
                                           QStringLiteral("取消"));
        if(ret == 1)
            return;
        else{
            _onDel(roomId);
            for(auto i = _list.begin();i!=_list.end();++i){
                if(i->room == roomId){
                    _list.erase(i);
                    break;
            }
        }
        RefreshGuest();
        QMessageBox::information(this,
                         QString(""),
                         QStringLiteral("删除成功"));
        }
    }
    catch(const std::exception &ex){
        std::cerr << ex.what() << std::endl;
        QMessageBox::information(this,
                         QStringLiteral("删除失败"),
                         QString(ex.what()));
    }

}

ConfigWindow::ConfigWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConfigWindow)
{
    ui->setupUi(this);
}

ConfigWindow::~ConfigWindow()
{
    delete ui;
}

void ConfigWindow::closeEvent(QCloseEvent *event){
    if(_onBack)
        _onBack();
    else{
        event->accept();
    }
}

ClientWindow::ClientWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ClientWindow)
{
    ui->setupUi(this);

    //Create data model
    _itemModel = new QStandardItemModel();
    _itemModel->setColumnCount(8);

    _itemModel->setHeaderData(0, Qt::Horizontal, QString(QStringLiteral("房间 ID ")));
    _itemModel->setHeaderData(1, Qt::Horizontal, QString(QStringLiteral("房客 ID ")));
    _itemModel->setHeaderData(2, Qt::Horizontal, QString(QStringLiteral("当前温度")));
    _itemModel->setHeaderData(3, Qt::Horizontal, QString(QStringLiteral("目标温度")));
    _itemModel->setHeaderData(4, Qt::Horizontal, QString(QStringLiteral("风速")));
    _itemModel->setHeaderData(5, Qt::Horizontal, QString(QStringLiteral("能耗")));
    _itemModel->setHeaderData(6, Qt::Horizontal, QString(QStringLiteral("费用")));
    _itemModel->setHeaderData(7, Qt::Horizontal, QString(QStringLiteral("最近一次心跳")));

    //Bind data model to the table.
    ui->ClientTable->setModel(_itemModel);
    ui->ClientTable->showGrid();
    ui->ClientTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->ClientTable->setSortingEnabled(true);
    ui->ClientTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->ClientTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    //Update clients information periodically
    //Bind signal and slot
    _timer = new QTimer();
    connect(_timer,SIGNAL(timeout()),this,SLOT(UpdateClient()));

    //Pulse defualt value
    _pulseInterval = 1000;
    ui->PulseInterval->setValue(_pulseInterval/1000);

}

ClientWindow::~ClientWindow()
{
    delete _itemModel;
    delete _timer;
    delete ui;
}

void ClientWindow::closeEvent(QCloseEvent *event){
    if(_onBack){
        _onBack();
    }
    else{
        event->accept();
    }
}

void ClientWindow::on_BackBtn_clicked()
{
    if(_onBack){
        this->close();
        _onBack();
    }
}

void ClientWindow::UpdateClient(){
    _clients = _onUpdate();
    using minutes_type = std::chrono::duration<int,std::ratio<60*60>> ;
    int row = 0;
    _itemModel->removeRows(0,_itemModel->rowCount());

    for(auto i = _clients.begin();i != _clients.end();++i,++row){
        RoomId room= i->first;
        ClientState state = i->second;
        _itemModel->setItem(row,0,new QStandardItem(QString::fromStdString(room)));
        _itemModel->setItem(row,1,new QStandardItem(QString::fromStdString(state.guest)));

        //TODO: Fix the precision of display temperature.
        _itemModel->setItem(row,2,new QStandardItem(QString::number(state.current,'g',3)));
        _itemModel->setItem(row,3,new QStandardItem(QString::number(state.target,'g',3)));
        auto wind = state.hasWind ? state.wind : 0;
        switch(wind){
        case 0:
            _itemModel->setItem(row,4,new QStandardItem(QString(QStringLiteral("停止送风"))));
            break;
        case 1:
            _itemModel->setItem(row,4,new QStandardItem(QString(QStringLiteral("低速风"))));
            break;
        case 2:
            _itemModel->setItem(row,4,new QStandardItem(QString(QStringLiteral("中速风"))));
            break;
        case 3:
            _itemModel->setItem(row,4,new QStandardItem(QString(QStringLiteral("高速风"))));
            break;
         default:
            break;
        }
        _itemModel->setItem(row,5,new QStandardItem(QString::number(state.energy,'g',3)));
        _itemModel->setItem(row,6,new QStandardItem(QString::number(state.cost,'g',3)));
         time_t t = std::chrono::system_clock :: to_time_t(state.pulse);
        _itemModel->setItem(row,7,new QStandardItem(QString(ctime(&t))));
    }
}

void ClientWindow::on_PulseInterval_valueChanged(int arg1)
{
    if(arg1 < 0){
        QMessageBox::warning(this,
                             QStringLiteral("范围错误"),
                             QString(QStringLiteral("刷新频率必须为正数，请重新输入")));
        ui->PulseInterval->setValue(1000);
        return ;
    }
    else{
        _pulseInterval = arg1 * 1000;
        _timer->stop();
        _timer->setInterval(_pulseInterval);
        _timer->start();
    }
}

StatisticWindow::StatisticWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StatisticWindow)
{
    ui->setupUi(this);
    ui->StartTime->setCalendarPopup(true);
    ui->EndTime->setCalendarPopup(true);
    ui->StartTime->setDisplayFormat("yyyy-MM-dd");
    ui->EndTime->setDisplayFormat("yyyy-MM-dd");
    ui->StartTime->setDateTime(QDateTime::currentDateTime());
    ui->EndTime->setDateTime(ui->StartTime->dateTime());
    ui->DayBtn->setChecked(true);
    ui->EndTime->setHidden(true);
    ui->label_2->setHidden(true);
  //  ui->EndTime->setReadOnly(true);
}

StatisticWindow::~StatisticWindow()
{
    delete ui;
}

void StatisticWindow::closeEvent(QCloseEvent * event){
    if(_onBack)
        _onBack();
    else{
        event->accept();
    }
}

void ConfigWindow::ShowConfig(){

   if(_config.isOn)
       ui->OnBtn->setChecked(true);
   else
       ui->OnBtn->setChecked(false);

   if(_config.mode == 0)
       ui->SummerBtn->setChecked(true);
   else
       ui->WinterBtn->setChecked(true);
}

void ConfigWindow::on_ConfirmBtn_clicked()
{
    _config.isOn = ui->OnBtn->isChecked();
    _config.mode = ui->SummerBtn->isChecked() == true ? 0 : 1;
    SetConfig();
    this->close();
    _onBack();

}

void ConfigWindow::on_CancelBtn_clicked()
{
    this->close();
    _onBack();
}

void StatisticWindow::on_QueryBtn_clicked()
{
    if(_onExport){
        try{
        _onExport(_timeBegin,_timeEnd);
        QMessageBox::information(this,QStringLiteral("成功"),
                                 QStringLiteral("报表导出成功"),QStringLiteral("确定"));
        }
        catch(std::exception &ex){
            QMessageBox::warning(this,QStringLiteral("开始时间取值错误，请重新选择"),QString::fromStdString(ex.what()),
                        QStringLiteral("确定"));
        }
    }
}

void StatisticWindow::on_StartTime_dateChanged(const QDate &date)
{
   QString begin = date.toString("yyyy-MM-dd");
   try{
      if(_onTimeBegin){
        int mode = 0;
        if(ui->DayBtn->isChecked())
            mode = 0;
        else if(ui->WeekBtn->isChecked())
            mode = 1;
        else
            mode = 2;
        auto duration = _onTimeBegin(mode,begin.toStdString());
        _timeBegin = duration.first;
        _timeEnd = duration.second;
        ui->EndTime->setTime(QTime::fromString
                       (QString::fromStdString(TimeHelper::TimeToString(_timeEnd)),
                        "yyyy-mm-dd"));
     }
   }
   catch(std::exception &ex){
       QMessageBox::warning(this,QStringLiteral("时间取值错误"),QString::fromStdString(ex.what()),
                   QStringLiteral("确定"));
   }
}

void StatisticWindow::on_QuitBtn_clicked()
{
    this->close();
    _onBack();
}

void StatisticWindow::SetTimeRange(std::string _timeBeg,std::string _timeEnd){
    ui->StartTime->setMinimumDate(QDate::fromString(
                                      QString::fromStdString(_timeBeg),"yyyy-mm-dd"));

    ui->StartTime->setMaximumDate(QDate::fromString(
                                      QString::fromStdString(_timeEnd),"yyyy-mm-dd"));
}
