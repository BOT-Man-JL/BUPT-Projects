
//
// Air Conditioner - Server MVC View (GUI View - Qt Interface)
// Youjie Zhang, 2017
//

#ifndef AC_SERVER_VIEW_GUI_QT_H
#define AC_SERVER_VIEW_GUI_QT_H

#include <functional>

#include <QWidget>
#include <QCloseEvent>
#include <QTimer>
#include <QMessageBox>
#include <QStandardItemModel>

#include "server-view.h"
#include "time-helper.h"

namespace Ui {
class WelcomeWindow;
class StatisticWindow;
class ClientWindow;
class GuestWindow;
class ConfigWindow;
}

using namespace Air_Conditioner;

class WelcomeWindow : public QWidget
{
    Q_OBJECT
    using OnNav = std::function<void()>;
public:
    explicit WelcomeWindow(QWidget *parent = 0);
    ~WelcomeWindow();

    void setOnQuit(OnNav && onQuit){
        _onQuit = onQuit;
    }
    void setOnConfig(OnNav && onConfig){
        _onConfig = onConfig;
    }
    void setOnClient(OnNav && onClient){
        _onClient = onClient;
    }
    void setOnGuest(OnNav && onGuest){
        _onGuest = onGuest;
    }
    void setOnLog(OnNav && onLog){
        _onLog = onLog;
    }
private slots:
    void on_QuitBtn_clicked();

    void on_ConfigBtn_clicked();

    void on_GuestBtn_clicked();

    void on_StatisticBtn_clicked();

    void on_ClientBtn_clicked();

private:
    OnNav _onQuit;
    OnNav _onConfig;
    OnNav _onClient;
    OnNav _onGuest;
    OnNav _onLog;
    Ui::WelcomeWindow *ui;
};

class StatisticWindow : public QWidget
{
    Q_OBJECT
    using OnBack = std::function<void ()>;
    using OnTimeBegin = std::function<std::pair<TimePoint,TimePoint> (const int mode, const std::string &time)>;
    using OnExport = std::function<void (TimePoint,TimePoint)>;
public:
    explicit StatisticWindow(QWidget *parent = 0);
    ~StatisticWindow();
    void SetOnBack(OnBack && onBack){
        _onBack = onBack;
    }
    void SetOnTimeBegin(OnTimeBegin && onTimeBegin){
        _onTimeBegin = onTimeBegin;
    }
    void SetOnExport(OnExport &&onExport){
        _onExport = onExport;
    }
    void SetTimeRange(std::string _timeBeg,std::string _timeEnd);

protected:
    void closeEvent(QCloseEvent *event);
private slots:
    void on_QueryBtn_clicked();

    void on_StartTime_dateChanged(const QDate &date);

    void on_QuitBtn_clicked();

private:
    Ui::StatisticWindow *ui;
    OnBack _onBack;
    OnTimeBegin _onTimeBegin;
    OnExport  _onExport;
    TimePoint _timeBegin;
    TimePoint _timeEnd;
};

class ClientWindow : public QWidget
{
    Q_OBJECT
    using OnBack = std::function<void ()>;
    using OnUpdate = std::function<ClientList ()>;

public:
    explicit ClientWindow(QWidget *parent = 0);
    ~ClientWindow();

    void SetOnBack(OnBack && onBack){
        _onBack = onBack;
    }

    void SetOnUpdate(OnUpdate && onUpdate){
        _onUpdate = onUpdate;
        _timer->start();
    }


protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void on_BackBtn_clicked();
    void UpdateClient();

    void on_PulseInterval_valueChanged(int arg1);

private:
    Ui::ClientWindow *ui;
    OnUpdate _onUpdate;
    OnBack _onBack;
    ClientList _clients;
    QStandardItemModel * _itemModel;
    QTimer * _timer;
    int _pulseInterval;
};

class GuestWindow : public QWidget
{
    Q_OBJECT
    using OnBack = std::function<void()>;
    using OnAdd = std::function<void (const GuestInfo &)>;
    using OnDel = std::function<void (const RoomId &)>;

    OnAdd _onAdd;
    OnDel _onDel;
public:
    explicit GuestWindow(QWidget *parent = 0);
    ~GuestWindow();

    void SetOnBack(OnBack && onBack){
        _onBack = onBack;
    }

    void SetOnAdd(OnAdd && onAdd){
        _onAdd = onAdd;
    }

    void SetOnDel(OnDel && onDel){
        _onDel = onDel;
    }

    void LoadGuest(const std::list<GuestInfo> &list);

    void RefreshGuest();

protected:
    void closeEvent(QCloseEvent *event);
private slots:
    void on_AddBtn_clicked();

    void on_CancelBtn_clicked();

    void on_ConfirmBtn_clicked();

    void on_DelBtn_clicked();

    void on_QuitBtn_clicked();

    
private:
    std::list<GuestInfo> _list;
    Ui::GuestWindow *ui;
    OnBack _onBack;
    QStandardItemModel * _itemModel;
};


class ConfigWindow : public QWidget
{
    Q_OBJECT
    using OnSet = std::function<void (const ServerInfo &)>;
    using OnBack = std::function<void ()>;
public:
    explicit ConfigWindow(QWidget *parent = 0);
    ~ConfigWindow();
    void LoadConfig(ServerInfo config){
        _config = config;
        ShowConfig();
    }
    void ShowConfig();
    void SetConfig(){
        _onSet(_config);
    }
    void SetOnBack(OnBack && onBack){
        _onBack = onBack;
    }
    void SetOnSet(OnSet && onSet){
        _onSet = onSet;
    }

protected:
    void closeEvent(QCloseEvent *event);
private slots:
    void on_ConfirmBtn_clicked();

    void on_CancelBtn_clicked();

private:
    Ui::ConfigWindow *ui;
    OnBack _onBack;
    OnSet _onSet;
    ServerInfo _config;
};

#endif // !AC_SERVER_VIEW_GUI_QT_H