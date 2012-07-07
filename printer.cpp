#include "printer.h"

Printer::Printer(QObject *parent)
{
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName ("UTF-8"));
    PortSettings settings = {BAUD9600, DATA_8, PAR_NONE, STOP_1, FLOW_OFF, 10};
    this->portObj = new QextSerialPort("",settings,QextSerialPort::EventDriven);
    this->isPrinting=false;
    this->temperatureTimer = new QTimer(this);
    connect(temperatureTimer, SIGNAL(timeout()), this, SLOT(getTemperature()));
    curr_pos.setX(0);
    curr_pos.setY(0);
    curr_pos.setZ(0);
}

bool Printer::isConnected(){
    return this->portObj->isReadable();
}

bool Printer::connectPort(QString port, int baud){
    emit write_to_console(tr("Connecting..."));
    if(port!=""){
        PortSettings settings = {BAUD9600, DATA_8, PAR_NONE, STOP_1, FLOW_OFF, 10};
        this->portObj = new QextSerialPort(port,settings,QextSerialPort::EventDriven);

        this->portObj->setBaudRate((BaudRateType)baud);
        if(this->portObj->open(QIODevice::ReadWrite)){
            emit write_to_console(tr("Printer connected"));
            connect(this->portObj, SIGNAL(readyRead()), this, SLOT(readFromPort()));
            return true;
        }
        else{
           emit write_to_console(tr("Unable to connect"));
           return false;
        }
    }
    else{
        emit write_to_console(tr("Unknown port name"));
        return false;
    }
}

//writing to port
int Printer::writeToPort(QString command){
    //analizing command for position tracking
    QStringList args=command.split(" ");
    for(int i=0 ;i <args.size(); i++){
        if(args.at(i).contains("X"))
            curr_pos.setX(args.at(i).mid(1).toDouble());
        if(args.at(i).contains("Y"))
            curr_pos.setY(args.at(i).mid(1).toDouble());
        if(args.at(i).contains("Z"))
            curr_pos.setZ(args.at(i).mid(1).toDouble());
    }

    emit currentPosition(curr_pos);

    if(this->isConnected()){
        return portObj->write(command.toUpper().toLatin1()+"\n\r");
    }
    else{
        write_to_console(tr("Printer offline"));
        return -1;
    }
}

//reading from port
void Printer::readFromPort(){
    QString buffer;
    buffer=portObj->readAll();
    buffer.replace("\n","");
    buffer.replace("\r","");

    //if we are printing then continue
    if(buffer.contains("ok")){
        if(buffer.contains("T:") && buffer.contains("B:")){
            emit currentTemp(buffer.mid(buffer.indexOf("T:")+2,buffer.indexOf(".",buffer.indexOf("T:"))-buffer.indexOf("T:")+2).toDouble(),0.0,buffer.mid(buffer.indexOf("B:")+2,buffer.indexOf(".",buffer.indexOf("B:"))-buffer.indexOf("B:")+2).toDouble());
        }
        if(this->gCodeBuffer.size()>0 && this->isPrinting){
            writeToPort(this->gCodeBuffer.takeFirst());
            emit progress(this->gCodeBuffer.size());
        }
    }
    else{
        write_to_console("Error: "+buffer);
    }
}

//slot starting print
void Printer::startPrint(){
    this->isPrinting=true;
    writeToPort(this->gCodeBuffer.takeFirst());
}

//slot stoping print
void Printer::stopPrint(){
    this->isPrinting=false;
}

//homing axis
void Printer::homeX(){
    writeToPort("G28 X0");
    curr_pos.setX(0);
}

void Printer::homeY(){
    writeToPort("G28 Y0");
    curr_pos.setY(0);
}
void Printer::homeZ(){
    writeToPort("G28 Z0");
    curr_pos.setZ(0);
}
void Printer::homeAll(){
    writeToPort("G28 X0 Y0 Z0");
    curr_pos.setX(0);
    curr_pos.setY(0);
    curr_pos.setZ(0);
}

//moving head
void Printer::moveHead(QPoint point, int speed){
    writeToPort("G1 F"+QString::number(speed));
    writeToPort("G1 X"+QString::number(point.x())+" Y"+QString::number(200-point.y()));
}
void Printer::moveHeadZ(double z, int speed){
    writeToPort("G1 F"+QString::number(speed));
    writeToPort("G1 Z"+QString::number(z));
}

//setting fan speed
void Printer::setFan(int percent){
    float value=255*((float)percent/(float)100);
    writeToPort("M106 S"+QString::number((int)value));
}
//disable stepper
void Printer::disableSteppers(){
    writeToPort("M18");
}

//adding data to printer buffer
void Printer::loadToBuffer(QStringList buffer, bool clear){
    if(clear){
        this->gCodeBuffer.clear();
    }
    this->gCodeBuffer.append(buffer);
}

void Printer::setMonitorTemperature(bool monitor){
    this->monitorTemperature=monitor;
    if(monitor){
        this->temperatureTimer->start(1000);
    }
    else{
        this->temperatureTimer->stop();
    }
}

void Printer::getTemperature(){
    writeToPort("M105");
}

void Printer::setTemp1(int temp){
    writeToPort("M104 S"+QString::number(temp));
}

void Printer::setTemp3(int temp){
    writeToPort("M140 S"+QString::number(temp));
}
