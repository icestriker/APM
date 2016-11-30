#include "apm.h"
#include "ui_apm.h"

APM::APM(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::APM)
{
    ui->setupUi(this);

    //this->setWindowIcon(QIcon(":/logo"));
    //get the init  POS
    initPos.lon=120.898334;
    initPos.lat=31.405428;

    view = new QWebView(ui->mapWidget);
    mark_view=new QWebView(ui->mark_view_widget);

    //load map
    runPath = QCoreApplication::applicationDirPath();
    view->load(QUrl::fromLocalFile (runPath+"/html/main_map.html"));
    mark_view->load (QUrl::fromLocalFile (runPath+"/html/mark_map.html"));


    connect(mark_view,SIGNAL(loadFinished(bool)),this,SLOT(mark_map_init()));
    connect(view, SIGNAL(loadFinished(bool)), this, SLOT(main_map_init()));
    connect(ui->listWidget,SIGNAL(currentRowChanged(int)),this,SLOT(setAnimation()));
    connect(mark_view->page()->mainFrame (),SIGNAL(javaScriptWindowObjectCleared()),this,SLOT(addToJavascript2()));
    connect(view->page ()->mainFrame (),SIGNAL(javaScriptWindowObjectCleared()),this,SLOT(addToJavascript()));
    connect(ui->locatepushButton,SIGNAL(clicked()),this,SLOT(mark_map_init()));

    view->settings()->setAttribute(QWebSettings::PluginsEnabled, true);
    view->settings()->setAttribute(QWebSettings::JavascriptEnabled, true);
    view->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
    view->settings()->setAttribute(QWebSettings::JavascriptCanOpenWindows, true);
    view->settings()->setAttribute(QWebSettings::JavaEnabled, true);
    mark_view->settings()->setAttribute(QWebSettings::PluginsEnabled, true);
    mark_view->settings()->setAttribute(QWebSettings::JavascriptEnabled, true);
    mark_view->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
    mark_view->settings()->setAttribute(QWebSettings::JavascriptCanOpenWindows, true);
    mark_view->settings()->setAttribute(QWebSettings::JavaEnabled, true);

    view->show();
    mark_view->show ();
    createStatusBars ();
    setupgraphy ();
}

APM::~APM()
{
    delete ui;
}
int APM::decode (){

    return 0;
}

void APM::setupgraphy()
{
    ui->customPlot->addGraph(); // blue line
    ui->customPlot->graph(0)->setPen(QPen(Qt::blue));
    ui->customPlot->graph(0)->setBrush(QBrush(QColor(240, 255, 200)));
    ui->customPlot->graph(0)->setAntialiasedFill(false);
    ui->customPlot->addGraph(); // red line
    ui->customPlot->graph(1)->setPen(QPen(Qt::red));
    ui->customPlot->graph(0)->setChannelFillGraph(ui->customPlot->graph(1));

    ui->customPlot->addGraph(); // blue dot
    ui->customPlot->graph(2)->setPen(QPen(Qt::blue));
    ui->customPlot->graph(2)->setLineStyle(QCPGraph::lsNone);
    ui->customPlot->graph(2)->setScatterStyle(QCPScatterStyle::ssDisc);
    ui->customPlot->addGraph(); // red dot
    ui->customPlot->graph(3)->setPen(QPen(Qt::red));
    ui->customPlot->graph(3)->setLineStyle(QCPGraph::lsNone);
    ui->customPlot->graph(3)->setScatterStyle(QCPScatterStyle::ssDisc);

    ui->customPlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    ui->customPlot->xAxis->setDateTimeFormat("hh:mm:ss");
    ui->customPlot->xAxis->setAutoTickStep(false);
    ui->customPlot->xAxis->setTickStep(2);
    ui->customPlot->axisRect()->setupFullAxesBox();

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->yAxis2, SLOT(setRange(QCPRange)));

    // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
    connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
    dataTimer.start(0); // Interval 0 means to refresh as fast as possible
    ui->customPlot->replot();
}

void APM::createStatusBars()
{
    ui->statusBar->addWidget (ui->statusLabel);
    updateStatusBars ("Ready");
}

void APM::updateStatusBars(QString info)
{
    ui->statusLabel->setText (info);
}



void APM::realtimeDataSlot(){
    // calculate two new data points:
  #if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
    double key = 0;
  #else
    double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
  #endif
    static double lastPointKey = 0;
    if (key-lastPointKey > 0.01) // at most add point every 10 ms
    {
      double value0 = qSin(key); //qSin(key*1.6+qCos(key*1.7)*2)*10 + qSin(key*1.2+0.56)*20 + 26;
      double value1 = qCos(key); //qSin(key*1.3+qCos(key*1.2)*1.2)*7 + qSin(key*0.9+0.26)*24 + 26;
      // add data to lines:
      ui->customPlot->graph(0)->addData(key, value0);
      ui->customPlot->graph(1)->addData(key, value1);
      // set data of dots:
      ui->customPlot->graph(2)->clearData();
      ui->customPlot->graph(2)->addData(key, value0);
      ui->customPlot->graph(3)->clearData();
      ui->customPlot->graph(3)->addData(key, value1);
      // remove data of lines that's outside visible range:
      ui->customPlot->graph(0)->removeDataBefore(key-8);
      ui->customPlot->graph(1)->removeDataBefore(key-8);
      // rescale value (vertical) axis to fit the current data:
      ui->customPlot->graph(0)->rescaleValueAxis();
      ui->customPlot->graph(1)->rescaleValueAxis(true);
      lastPointKey = key;
    }
    // make key axis range scroll with the data (at a constant range size of 8):
    ui->customPlot->xAxis->setRange(key+0.25, 8, Qt::AlignRight);
    ui->customPlot->replot();

    // calculate frames per second:
    static double lastFpsKey;
    static int frameCount;
    ++frameCount;
    if (key-lastFpsKey > 2) // average fps over 2 seconds
    {

      lastFpsKey = key;
      frameCount = 0;
    }

}

void APM::mark_map_init()
{
    QWebFrame *mark_frame = mark_view->page()->mainFrame();
    QString initCmd=QString("init(%1,%2);").arg (initPos.lon).arg (initPos.lat);
    QString markCmd = QString("markGPS(%1, %2);").arg(initPos.lon).arg(initPos.lat);
    mark_frame->evaluateJavaScript(markCmd);
    mark_frame->evaluateJavaScript(initCmd);
}

void APM::main_map_init (){
    QWebFrame *main_frame = view->page()->mainFrame();
    QString initCmd=QString("init(%1,%2);").arg (initPos.lon).arg (initPos.lat);
    QString markCmd = QString("markGPS(%1, %2);").arg(initPos.lon).arg(initPos.lat);
    //QString markCmd2 = QString("markGPS(%1, %2);").arg(120.895772).arg(31.413162);
    main_frame->evaluateJavaScript(initCmd);
    main_frame->evaluateJavaScript(markCmd);
   // main_frame->evaluateJavaScript (markCmd2);
}

void APM::addToJavascript (){
        view->page ()->mainFrame ()->addToJavaScriptWindowObject ("QT",this);
}
void APM::addToJavascript2 (){
        mark_view->page()->mainFrame()->addToJavaScriptWindowObject("QT",this);
}


void APM::addFromHtml (QString a,QString b){
    GPS newPoint={a.toFloat (),b.toFloat ()};
    APM::targetList.push_back(newPoint);
    //APM::tar.push_back (new QListWidgetItem(QIcon(runPath+"/icons/target.png"),a+','+b,ui->listWidget));
    int index=targetList.size ();
    ui->listWidget->insertItem (index,new QListWidgetItem(QIcon(runPath+"/icons/target.png"),a+','+b,ui->listWidget));
    ui->listWidget->show ();
}

void APM::delFromHtml(int i)
{
    GPS zero={0.0,0.0};
    targetList[i]=zero;

    QListWidgetItem *item=ui->listWidget->takeItem (i-1);
    ui->listWidget->removeItemWidget (item);
    delete item;
}

void APM::setAnimation()
{
    int row=ui->listWidget->currentRow ();
    QWebFrame *mark_frame = mark_view->page()->mainFrame();
    QString cmd = QString("setAnimation(%1);").arg(row);
    mark_frame->evaluateJavaScript(cmd);
    //cout<<ui->listWidget->currentRow ();
}

void APM::getTarget()
{
    QWebFrame *main_frame = view->page()->mainFrame();
    if(targetList.size ()==0)
    {
        QString cmd=QString("alert('no target setted')");
        main_frame->evaluateJavaScript (cmd);
    }
    else
    for(unsigned int i=0;i<targetList.size ();i++){
        if(targetList[i].lon!=0.0 &&targetList[i].lat!=0.0){
            QString markCmd = QString("markGPS(%1, %2);").arg(targetList[i].lon).arg(targetList[i].lat);
            main_frame->evaluateJavaScript(markCmd);
        }
    }
}

void APM::updateLocation()
{
    QWebFrame *main_frame = view->page()->mainFrame();
    QString cmd =QString("markGPS(%1,%2);").arg(initPos.lon).arg(initPos.lat);
    main_frame->evaluateJavaScript(cmd);
}


void APM::route (float x, float y){
    QWebFrame *frame = view->page()->mainFrame();
    QString cmd = QString("addToLine(%1, %2);").arg(x).arg(y);
    frame->evaluateJavaScript(cmd);
}

