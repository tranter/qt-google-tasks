#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "form.h"
#include "settingsdialog.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->actionLogin, SIGNAL(triggered()), this, SLOT(startLogin()));
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));

    connect(ui->actionSettings, SIGNAL(triggered()), this, SLOT(settings()));


    m_pForm = new Form(this);
    setCentralWidget(m_pForm);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::settings()
{
    SettingsDialog dlg(this);
    dlg.exec();
}

void MainWindow::startLogin()
{
    m_pForm->startLogin(true);
}

void MainWindow::startLoginDefault()
{
    m_pForm->startLogin(false);
}
