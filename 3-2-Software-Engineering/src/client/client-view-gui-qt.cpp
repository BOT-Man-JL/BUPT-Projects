
//
// Air Conditioner - Client MVC View (GUI View - Qt Implementation)
// Youjie Zhang, 2017
//

#include "ui_authwindow.h"
#include "ui_controlwindow.h"

#include "client-view-gui-qt.h"

AuthWindow::AuthWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AuthWindow)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setTabOrder(ui->RoomId,ui->GuestId);
}

AuthWindow::~AuthWindow()
{
    delete ui;
}

void AuthWindow::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void AuthWindow::on_LogOnBtn_clicked()
{
    try{
        QString roomId = ui->RoomId->text();
        QString guestId = ui->GuestId->text();
        if(_onAuth){
            _onAuth(Air_Conditioner::GuestInfo{roomId.toStdString(),
                                          guestId.toStdString()});
            this->close();
        }
    }
    catch(std::exception &ex){
        QMessageBox::warning(this,
                         QStringLiteral("登录失败"),
                         QString(ex.what()));
    }
    catch(int){
         QMessageBox::critical(this,QStringLiteral("无法连接"),
                               QStringLiteral("无法连接到服务器。请重启程序！"),QStringLiteral("确定"));
         this->close();
    }

}

void AuthWindow::on_QuitBtn_clicked()
{
    int ret = QMessageBox::information(this,QStringLiteral("退出？"),
                   QStringLiteral("确定退出从控机系统？"),QStringLiteral("确定"),
                                       QStringLiteral("取消"));
    if(ret == 0)
        this->close();
}

ControlWindow::ControlWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ControlWindow)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    ui->WindBtn->setFlat(true);
    ui->WindBtn->setAutoFillBackground(true);
    ui->WindBtn->setStyleSheet("QPushButton{border-image: url(:/button/Mid.png);}");
}

ControlWindow::~ControlWindow()
{
    delete ui;
}

void ControlWindow::Message(QString message)
{
    QMessageBox::information(this,QStringLiteral(""),message,QStringLiteral("确定"));
}

void ControlWindow::ShowState(Air_Conditioner::ServerInfo _serverInfo,
                              Air_Conditioner::ClientInfo _clientInfo,
                              Air_Conditioner::RoomRequest _roomRequest)
{
    //ClientInfo And roomRequest
    ui->Cost->setText(QString::number(_clientInfo.cost,'g',3));
    ui->Energy->setText(QString::number(_clientInfo.energy,'g',3));

    if (!_serverInfo.isOn)
        ui->Wind->setText(QStringLiteral("主控关机"));
    else if (!_clientInfo.hasWind)
        ui->Wind->setText(QStringLiteral("停止送风"));
    else{
        switch(_roomRequest.wind){
            case 0:
                ui->Wind->setText(QStringLiteral("停止送风"));
                break;
            case 1:
                ui->Wind->setText(QStringLiteral("低风速"));
                break;
            case 2:
                ui->Wind->setText(QStringLiteral("中风速"));
                break;
            case 3:
                ui->Wind->setText(QStringLiteral("高风速"));
                break;
            default:
                break;
        }
   }
   ui->CurrTemp->setText(QString::number(_roomRequest.current,'g',3));
   ui->TargetTemp->setText(QString::number(_roomRequest.target,'g',3));

   //ServerInfo
   if(_serverInfo.isOn == true){
       ui->ServerState->setChecked(true);
   }
   else{
       ui->ServerState->setChecked(false);
   }
   if(_serverInfo.mode == 0){
       ui->WorkingMode->setText(QStringLiteral("夏天"));
   }
   else{
       ui->WorkingMode->setText(QStringLiteral("冬天"));
   }

}
void ControlWindow::LoadGuestInfo(Air_Conditioner::GuestInfo guest)
{
     ui->RoomId->setText(QString::fromStdString(guest.room));
}

void ControlWindow::on_UpBtn_clicked()
{
    Air_Conditioner::Temperature temp = ui->TargetTemp->text().toDouble();
    temp += 0.1;
    ui->TargetTemp->setText(QString::number(temp,'g',3));
    _onTempChanged(Air_Conditioner::Temperature{temp});
}

void ControlWindow::on_DownBtn_clicked()
{
     Air_Conditioner::Temperature temp = ui->TargetTemp->text().toDouble();
     temp -= 0.1;
     ui->TargetTemp->setText(QString::number(temp,'g',3));
     _onTempChanged(Air_Conditioner::Temperature{temp});
}

void ControlWindow::on_QuitBtn_clicked()
{
    this->close();
}

void ControlWindow::on_WindBtn_clicked()
{
    static int windLevel = 1;
     windLevel = ((windLevel+1)%4) == 0? 1:((windLevel+1)%4);
    switch(windLevel){
    case 1:
        ui->WindBtn->setStyleSheet("QPushButton{border-image: url(:/button/Low.png);}");
        break;
    case 2:
        ui->WindBtn->setStyleSheet("QPushButton{border-image: url(:/button/Mid.png);}");
        break;
    case 3:
        ui->WindBtn->setStyleSheet("QPushButton{border-image: url(:/button/Strong.png);}");
        break;
    }
    _onWindChanged(Air_Conditioner::Wind{windLevel});
}
