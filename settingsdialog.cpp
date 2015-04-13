#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "registration.h"

#include <QSettings>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    QSettings settings(COMPANY_NAME, APP_NAME);

    QString strClientID = settings.value(CLIENT_ID_KEY,  "").toString();
    QString strClientSecret = settings.value(CLIENT_SECRET_KEY, "").toString();
    QString strRedirectURI = settings.value(REDIRECT_URI_KEY, "").toString();

    ui->clientIdLineEdit->setText(strClientID);
    ui->clientSecretLineEdit->setText(strClientSecret);
    ui->redirectUriLineEdit->setText(strRedirectURI);

}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::accept()
{
    QSettings settings(COMPANY_NAME, APP_NAME);
    settings.setValue(CLIENT_ID_KEY,  ui->clientIdLineEdit->text());
    settings.setValue(CLIENT_SECRET_KEY,  ui->clientSecretLineEdit->text());
    settings.setValue(REDIRECT_URI_KEY,  ui->redirectUriLineEdit->text());
    QDialog::accept();
}
