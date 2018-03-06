#include "mainwindow.h"
#include "ui_mainwindow.h"

QList <ProcInfo> *procList;
QList <int> nextList;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    curve1 = new QwtPlotCurve("Cpu Usage");
    curve2 = new QwtPlotCurve("Memory Usage");
    curve3 = new QwtPlotCurve("Swap Usage");
    timer1 = new QTimer(this);
    timer2 = new QTimer(this);
    timer3 = new QTimer(this);
    grid1 = new QwtPlotGrid;
    grid2 = new QwtPlotGrid;
    connect(timer1,SIGNAL(timeout()),SLOT(ShowSystemRunningTime()));
    connect(timer2,SIGNAL(timeout()),SLOT(ShowProcessInfo()));
    connect(timer3,SIGNAL(timeout()),SLOT(ShowCpuUsage()));
    connect(timer3,SIGNAL(timeout()),SLOT(ShowMemorySwapUsage()));
    connect(ui->SearchButton,SIGNAL(clicked(bool)),SLOT(SearchProcess()));
    connect(ui->KillButton,SIGNAL(clicked(bool)),SLOT(KillProcess()));
    connect(ui->NextButton,SIGNAL(clicked(bool)),SLOT(NextProcess()));
    connect(ui->RefreshButton,SIGNAL(clicked(bool)),SLOT(ShowProcessInfo()));
    connect(ui->RunButton,SIGNAL(clicked(bool)),SLOT(RunNewProcess()));
    connect(ui->ShutDownButton,SIGNAL(clicked(bool)),SLOT(ShutDown()));
    timer1->start(10); // every 10ms refresh stats
    timer2->start(20000); //every 20s refresh the list
    timer3->start(1000);
    ShowHostName();
    ShowSystemStartUpTime();
    ShowSystemVersion();
    ShowCpu();
    ShowProcessInfo();
    ShowCpuUsage();
    ShowMemorySwapUsage();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::ShowHostName(){
    QFile hostName_File("/proc/sys/kernel/hostname");
    QString hostName = "Host Name: ";
    if(!hostName_File.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() <<"hostName_File opened failed";
        return;
    }
    hostName.append(hostName_File.readLine());
    ui->HostName->setText(hostName);
}

void MainWindow::ShowSystemStartUpTime(){
    QDateTime currentTime = QDateTime::currentDateTime();
    QDateTime startUpTime;
    QString runningTime;
    QFile upTimeFile("/proc/uptime");
    if(!upTimeFile.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() << "upTimeFile opened failed";
        return;
    }
    runningTime = QString(upTimeFile.readLine()).section(" ",0,0);
    qDebug() << runningTime;
    runningTime.chop(3);
    startUpTime = currentTime.addSecs(runningTime.toInt() * (-1));
    QString timeString = startUpTime.toString("yyyy.MM.dd   hh:mm:ss ");
    qDebug() << timeString;
    ui->StartTime->setText("Start Time:  " + timeString);

}

void MainWindow::ShowSystemRunningTime(){
    int runningHours;
    int runningMinutes;
    int runningSecond;
    QString runningTime;
    QDateTime currentTime = QDateTime::currentDateTime();
    QFile upTimeFile("/proc/uptime");
    if(!upTimeFile.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() << "upTimeFile opened failed";
        return;
    }
    runningTime = QString(upTimeFile.readLine()).section(" ",0,0);
    runningTime.chop(3);
    runningHours = runningTime.toInt() / 3600;
    runningMinutes = (runningTime.toInt() / 60) % 60;
    runningSecond = runningTime.toInt() % 60;
    runningTime = QString::number(runningHours) + ":" + QString::number(runningMinutes) + ":" + QString::number(runningSecond);
    runningTime = "Running Time:  " + runningTime;
    ui->RunningTime->setText(runningTime);
    ui->State_currentTime->setText(currentTime.toString("yyyy.MM.dd   hh:mm:ss "));
}

void MainWindow::ShowSystemVersion(){
    QFile sysTypeFile("/proc/sys/kernel/ostype");
    QFile sysVersionFile("/proc/sys/kernel/osrelease");
    QString sysVersion;
    QString sysType;
    QString TypeAndVersion;
    if(!sysTypeFile.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() << "sysTypeFile opened failed";
        return;
    }
    if(!sysVersionFile.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() << "sysVersionFile opened failed";
        return;
    }
    sysType = QString(sysTypeFile.readLine());
    sysType.chop(1);
    sysVersion = QString(sysVersionFile.readLine());
    TypeAndVersion = "System Type:  " + sysType + "  " + sysVersion;
    ui->SysVersion->setText(TypeAndVersion);
}

void MainWindow::ShowCpu(){
    QFile cpuFile("/proc/cpuinfo");
    QString cpuType;
    if(!cpuFile.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() << "cpuFile opened failed";
        return;
    }
    while(1){
        cpuType = QString(cpuFile.readLine());
        qDebug() << cpuType;
        if(cpuType.section("\t",0,0) == "model name"){
            cpuType = cpuType.right(41);
            ui->Cpu->setText("CPU:  " + cpuType);
            break;
        }
    }
}

void MainWindow::ShowProcessInfo(){
    QFile procFile;
    QFile statusFile;
    QString pathString; //for traverse all process
    QString memString;
    QString procString;
    QString showString;
    QString emptyString;
    QString memUseString;
    procList = new QList <ProcInfo>;
    ProcInfo procInfo;
    int i,index = 0;
    ui->ProcList->clear();
    for(i = 0; i < 32768; i++){
        pathString = "/proc/" + QString::number(i) + "/stat";
        memString = "/proc/" + QString::number(i) + "/status";
        procFile.setFileName(pathString);
        statusFile.setFileName(memString);
        if(procFile.open(QIODevice::ReadOnly | QIODevice::Text) && statusFile.open(QIODevice::ReadOnly | QIODevice::Text)){
            procString = QString(procFile.readLine());
            memUseString = QString(statusFile.readLine());
            //start dealing with process String
            procInfo.procName = procString.section("(",1,1).section(")",0,0);
            procInfo.procPID = procString.section(" ",0,0);
            procInfo.procPPID = procString.section(" ",3,3);
            procInfo.procPriority = procString.section(" ",18,18);

            procInfo.procMemoryUse = QString(statusFile.readAll()).section("RssAnon:",1,1).left(10);
            if(procInfo.procMemoryUse.toInt() > 1024) procInfo.procMemoryUse = QString::number(procInfo.procMemoryUse.toInt() / 1024) + " MB";
            else procInfo.procMemoryUse = QString::number(procInfo.procMemoryUse.toInt()) + " KB";
            //qDebug() << "mem" + QString::number(i) + "   :"+ procInfo.procMemoryUse;
            procInfo.indexInList = index++;
            procList->append(procInfo); //save process information to list
            //process string to show on the list
            emptyString = "                       ";
            showString.append(emptyString.replace(0,procInfo.procName.length(),procInfo.procName));
            emptyString = "         ";
            showString.append(emptyString.replace(0,procInfo.procPID.length(),procInfo.procPID));
            emptyString = "             ";
            showString.append(emptyString.replace(0,procInfo.procPPID.length(),procInfo.procPPID));
            emptyString = "                    ";
            showString.append(emptyString.replace(0,procInfo.procMemoryUse.length(),procInfo.procMemoryUse));
            emptyString = "    ";
            showString.append(emptyString.replace(0,procInfo.procPriority.length(),procInfo.procPriority));
            //qDebug() << showString;
            ui->ProcList->addItem(showString);
            showString = "";
        }
        procFile.close();
        statusFile.close();
    }
    if(ui->ProcList->currentItem() != 0)
        qDebug() << ui->ProcList->currentItem()->text();
        //qDebug() << index;
    //ui->ProcList->setCurrentRow(index / 2);
}

void MainWindow::SearchProcess(){
    int i,found = 0;
    ShowProcessInfo();
    nextList.clear();
    timer2->start(20000);
    QString inputString = ui->SearchText->text();
    if(QString::number(inputString.toInt()) == inputString){ //input is number, search PID
        for(i = 0; i < procList->size(); i++){
            if(procList->at(i).procPID == inputString){
                if(found == 0){
                    ui->ProcList->setCurrentRow(procList->at(i).indexInList);
                    found = 1;
                }
                else found = 1;
            }
        }
        if(found == 0){
            QMessageBox::information(NULL,"","\nProcess Not Found.\n");
        }
    }
    else{ // search process name
        for(i = 0; i < procList->size(); i++){
            if(procList->at(i).procName == inputString || procList->at(i).procName.contains(inputString)){
                if(found == 0){
                    ui->ProcList->setCurrentRow(procList->at(i).indexInList);
                    found = 1;
                }
                else found = 1;
                nextList.append(procList->at(i).indexInList);
            }
        }
        if(found == 0){
            QMessageBox::information(NULL,"","\nProcess Not Found.\n");
        }
    }
}

void MainWindow::KillProcess(){
    int i;
    for(i = 0; i < procList->size(); i++){
        if(procList->at(i).indexInList == ui->ProcList->currentRow()){
            system("kill " + procList->at(i).procPID.toLatin1());
            break;
        }
    }
    ShowProcessInfo();
}

void MainWindow::NextProcess(){
    static int i;
    if(i == nextList.size() - 1) i = 0;
    if(nextList.size() == 0) return;
    ui->ProcList->setCurrentRow(nextList.at(i++));
}

void MainWindow::ShowCpuUsage(){
    int i;
    static int count;
    static int preTotalTime;
    static int preIdleTime;
    static int currentTotalTime;
    static int currentIdleTime;
    static double time[120];
    static double rate[120];
    for(i = 0;i < 120; i++){
        time[i] = i;
    }
    int usage;
    QFile cpuFile("/proc/stat");
    QString line;
    if(!cpuFile.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() << "cpuFile opened failed";
        return;
    }
    preTotalTime = currentTotalTime;
    preIdleTime = currentIdleTime;
    line = QString(cpuFile.readLine());
    currentTotalTime = line.section(" ",2,2).toInt() //user time
            + line.section(" ",3,3).toInt() // nice time
            + line.section(" ",4,4).toInt() // system time
            + line.section(" ",5,5).toInt() // idle time
            + line.section(" ",7,7).toInt() // irq time
            + line.section(" ",8,8).toInt() // softirq time
            + line.section(" ",9,9).toInt();// guest time
    currentIdleTime = line.section(" ",5,5).toInt();
    usage = 100 - ((currentIdleTime - preIdleTime) * 100) / (currentTotalTime - preTotalTime);
    for(i = 0;i < 119; i ++){
        rate[i] = rate[i + 1];
    }
    rate[119] = usage;
    grid1->setMajorPen( Qt::black, 0, Qt::DotLine );
    grid1->attach(ui->CpuPlot);
    ui->CpuPlot->setAxisScale(QwtPlot::yLeft,0,100);
    curve1->setPen(Qt::green); //set curve's color
    curve1->setRenderHint(QwtPlotItem::RenderAntialiased,true); //smooth curve
    curve1->setSamples(time, rate, 120);
    curve1->attach(ui->CpuPlot);
    ui->CpuPlot->replot();
    ui->State_cpuUsage->setText("Cpu Usage: " + QString::number(usage) + "% ");
    return;
}

void MainWindow::ShowMemorySwapUsage(){
    QFile memFile("/proc/meminfo");
    QString line;
    int i;
    int memTotal;
    int memAvailable;
    int swapTotal;
    int swapFree;
    int memUsage;
    int swapUsage;
    static double time[120];
    static double memRate[120];
    static double swapRate[120];
    for(i = 0;i < 120; i++){
        time[i] = i;
    }
    if(!memFile.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() << "memFile opened failed.";
        return;
    }
    while(true){
        line = QString(memFile.readLine());
        if(line.contains("Dirty")) break;
        else if(line.contains("MemTotal")) memTotal = line.right(14).left(10).toInt();
        else if(line.contains("MemAvailable")) memAvailable = line.right(14).left(10).toInt();
        else if(line.contains("SwapTotal")) swapTotal = line.right(14).left(10).toInt();
        else if(line.contains("SwapFree")) swapFree = line.right(14).left(10).toInt();
    }
    for(i = 0;i < 119; i++){
        memRate[i] = memRate[i+1];
    }
    for(i = 0;i < 119; i++){
        swapRate[i] = swapRate[i+1];
    }
    swapUsage = 100 - (swapFree * 100) / swapTotal;
    memUsage = 100 - (memAvailable * 100) / memTotal;
    memRate[119] = memUsage;
    swapRate[119] = swapUsage;
    ui->MemPlot->setAxisScale(QwtPlot::yLeft,0,100);
    grid2->setMajorPen( Qt::black, 0, Qt::DotLine );
    grid2->attach(ui->MemPlot);
    curve2->setPen(Qt::red);
    curve2->setRenderHint(QwtPlotItem::RenderAntialiased,true);
    curve3->setPen(Qt::yellow);
    curve3->setRenderHint(QwtPlotItem::RenderAntialiased,true);
    curve2->setSamples(time, memRate, 120);
    curve3->setSamples(time, swapRate, 120);
    curve2->attach(ui->MemPlot);
    curve3->attach(ui->MemPlot);
    ui->MemPlot->replot();
    ui->State_memUsage->setText("Memory Usage: " + QString::number(memUsage) + "% ");
    return;
}

void MainWindow::RunNewProcess(){
    QStringList argumentList;
    QList <QString> arglist;
    if(ui->ProcList->currentRow() != -1){
        QString procString = procList->at(ui->ProcList->currentRow()).procName;
        QString desProcString = QFileDialog::getOpenFileName();
        arglist.append(desProcString);
        argumentList.append(arglist);
        QProcess::execute(procString,argumentList);
    }
    return;
}

void MainWindow::ShutDown(){
    system("halt");
}
