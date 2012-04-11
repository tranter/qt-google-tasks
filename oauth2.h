#ifndef OAUTH2_H
#define OAUTH2_H

#include <QString>
#include <QObject>

class OAuth2 : public QObject
{
    Q_OBJECT

public:
    OAuth2();
    QString accessToken();
    bool isAuthorized();
    void startLogin(QWidget* parent, bool bForce);

    QString loginUrl();

signals:
    void loginDone();

private:
    QString m_strAccessToken;

    QString m_strEndPoint;
    QString m_strScope;
    QString m_strClientID;
    QString m_strRedirectURI;
    QString m_strResponseType;
};

#endif // OAUTH2_H
