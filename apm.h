#ifndef APM_H
#define APM_H

#include <QMainWindow>
#include <QFile>
#include <QTextStream>
#include <QIODevice>
#include <QWebDatabase>
#include <QWebSecurityOrigin>
#include <QWebFrame>
#include <QWebView>
#include <QString>
#include <iostream>
#include <vector>
#include <QButtonGroup>
#include <QListWidgetItem>
#include <QTimer>

using namespace std;
namespace Ui {
class APM;
}

struct GPS{
    float lon;
    float lat;
};

class APM : public QMainWindow
{
    Q_OBJECT

public:
    explicit APM(QWidget *parent = 0);
    ~APM();
    void route(float,float);
    int decode();
    void setupgraphy();
    void createStatusBars();
    void updateStatusBars(QString);


public slots:
    void main_map_init();
    void mark_map_init();
    void addToJavascript();
    void addToJavascript2 ();
    void addFromHtml(QString,QString);
    void delFromHtml(int);
    void setAnimation(int);
    void getTarget();
    void realtimeDataSlot();
    void loadMarkMap(int);
    void deleteSeedSlot();

private slots:
    void on_listWidget_customContextMenuRequested(const QPoint &pos);

private:
    Ui::APM *ui;
    QString runPath;
    GPS initPos;
    vector<GPS> targetList;
    QTimer dataTimer;
    QWebView *view;
    QWebView *mark_view;
};

#endif // APM_H
