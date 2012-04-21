#include "tasks_data_manager.h"
#include <QRegExp>
#include <QJson/Parser>
#include <QJson/Serializer>


TasksDataManager::TasksDataManager(QObject *parent) :
    QObject(parent)
{
    m_pNetworkAccessManager = new QNetworkAccessManager(this);
    connect(m_pNetworkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));

    m_bInMoving = false;
}

QVariantList TasksDataManager::getTaskLists()
{
    return m_taskLists;
}

QVariantList TasksDataManager::getTasks()
{
    return m_tasks;
}

void TasksDataManager::getMyTaskLists(const QString& access_token)
{
    QString s = QString("https://www.googleapis.com/tasks/v1/users/@me/lists?&access_token=%1").arg(access_token);
    m_pNetworkAccessManager->get(QNetworkRequest(QUrl(s)));
}

void TasksDataManager::getMyTasks(const QString& access_token, const QString& taskListID)
{
    QString s = QString("https://www.googleapis.com/tasks/v1/lists/%1/tasks?&access_token=%2").arg(taskListID).arg(access_token);
    m_pNetworkAccessManager->get(QNetworkRequest(QUrl(s)));
}

void TasksDataManager::createList(const QString& access_token, const QString& title)
{
    QString s = QString("https://www.googleapis.com/tasks/v1/users/@me/lists?access_token=%1").arg(access_token);
    QByteArray params;
    params.append(QString("{ title: \"%1\" }").arg(title).toUtf8());
    QNetworkRequest request;
    request.setUrl(QUrl(s));
    request.setRawHeader("Content-Type", "application/json");
    m_pNetworkAccessManager->post(request, params);
}

void TasksDataManager::deleteList(const QString& access_token, const QString& taskListID)
{
    QString s = QString("https://www.googleapis.com/tasks/v1/users/@me/lists/%1?access_token=%2").arg(taskListID).arg(access_token);
    m_pNetworkAccessManager->deleteResource(QNetworkRequest(QUrl(s)));
}

void TasksDataManager::deleteTask(const QString& access_token, const QString& taskListID, const QString& taskID)
{
    QString s = QString("https://www.googleapis.com/tasks/v1/lists/%1/tasks/%2?access_token=%3").arg(taskListID).arg(taskID).arg(access_token);
    m_pNetworkAccessManager->deleteResource(QNetworkRequest(QUrl(s)));
}

void TasksDataManager::createTask(const QString& access_token, const QString& taskListID, const QString& title,
                                  const QString& prevTaskID, const QString& parentID)
{
    QString s;
    s = QString("https://www.googleapis.com/tasks/v1/lists/%1/tasks?access_token=%2").arg(taskListID).arg(access_token);

    if(!prevTaskID.isEmpty())
    {
        s += QString("&previous=%1").arg(prevTaskID);
    }
    if(!parentID.isEmpty())
    {
        s += QString("&parent=%1").arg(parentID);
    }

    QByteArray params;
    params.append(QString("{ title: \"%1\" }").arg(title).toUtf8());
    QNetworkRequest request;
    request.setUrl(QUrl(s));
    request.setRawHeader("Content-Type", "application/json");
    m_pNetworkAccessManager->post(request, params);
}

void TasksDataManager::updateTask(const QString& access_token, const QString& taskListID, const QString& taskID, const QVariant& json_object)
{
    QString s = QString("https://www.googleapis.com/tasks/v1/lists/%1/tasks/%2?access_token=%3").arg(taskListID).arg(taskID).arg(access_token);

    QJson::Serializer serializer;
    QByteArray params = serializer.serialize( json_object );

    QNetworkRequest request;
    request.setUrl(QUrl(s));
    request.setRawHeader("Content-Type", "application/json");
    m_pNetworkAccessManager->put(request, params);
}

void TasksDataManager::moveTask(const QString& access_token, const QString& taskListID, const QString& taskID,
                                const QString& prevTaskID,   const QString& parentID)
{
    QString s;
    s = QString("https://www.googleapis.com/tasks/v1/lists/%1/tasks/%2/move?access_token=%3").
            arg(taskListID).arg(taskID).arg(access_token);

    if(!prevTaskID.isEmpty())
    {
        s += QString("&previous=%1").arg(prevTaskID);
    }
    if(!parentID.isEmpty())
    {
        s += QString("&parent=%1").arg(parentID);
    }

    if(m_bInMoving)
    {
        m_moveRequests.append(s);
        return;
    }

    QByteArray params;
    QNetworkRequest request;
    request.setUrl(QUrl(s));
    request.setRawHeader("Content-Type", "application/json");
    m_pNetworkAccessManager->post(request, params);
}

void TasksDataManager::getUserEmail(const QString& access_token)
{
    QString query = QString("https://www.googleapis.com/oauth2/v1/userinfo"
                            "?access_token=%1")
            .arg(access_token);
    m_pNetworkAccessManager->get(QNetworkRequest(QUrl(query)));
}

void TasksDataManager::replyFinished(QNetworkReply *reply)
{
    QString json = reply->readAll();
    QString strUrl = reply->url().toString();

    //Empty answer usually after creting task or list!
    if(json.length() == 0)
    {
        //Deletion of lists or tasks
        QRegExp reg("lists/*/tasks", Qt::CaseSensitive, QRegExp::Wildcard);
        if(strUrl.indexOf(reg) != -1)
        {
            emit taskChanged();
        }
        else
        {
            emit listsChanged();
        }
        return;
    }

    QJson::Parser parser;
    bool ok;

    // json is a QString containing the data to convert
    QVariant result = parser.parse (json.toLatin1(), &ok);
    if(!ok)
    {
        emit errorOccured(QString("Cannot convert to QJson object: %1").arg(json));
        return;
    }

    if(result.toMap().contains("error"))
    {
        emit errorOccured(result.toMap()["error"].toMap()["message"].toString());
        return;
    }

    if(strUrl.indexOf("/move") != -1)
    {
        if(strUrl == m_strLastMoveRequest)
        {
            m_moveRequests.clear();
            emit taskChanged();
            return;
        }
        else
        {
            int ind = m_moveRequests.indexOf(strUrl);
            if(ind != -1 && ind < m_moveRequests.count())
            {
                QByteArray params;
                QNetworkRequest request;
                request.setUrl(QUrl(m_moveRequests[ind+1]));
                request.setRawHeader("Content-Type", "application/json");
                m_pNetworkAccessManager->post(request, params);
            }
            return;
        }
    }

    if(result.toMap()["kind"] == "tasks#taskLists")
    {
        m_taskLists = result.toMap()["items"].toList();
        emit taskListsReady();
    }
    else if(result.toMap()["kind"] == "tasks#tasks") //Not search results
    {
        m_tasks = result.toMap()["items"].toList();
        emit tasksReady();
    }
    else if(result.toMap()["kind"] == "tasks#task") //Not search results
    {
        emit taskChanged();
    }
    else if(result.toMap()["kind"] == "tasks#taskList") //New created list
    {
        emit listsChanged();
    } else if (strUrl.contains("userinfo")) {
        m_strUserEmail = result.toMap()["email"].toString();
        emit sigUserEmailReady();
        return;
    }
}

void TasksDataManager::startMoving()
{
    m_bInMoving = true;
}

void TasksDataManager::endMoving()
{
    m_bInMoving = false;
    for(int i = 0; i < m_moveRequests.count(); ++i)
    {
        QByteArray params;
        QNetworkRequest request;
        request.setUrl(QUrl(m_moveRequests[i]));
        request.setRawHeader("Content-Type", "application/json");
        m_pNetworkAccessManager->post(request, params);
        if(i == 0)
            break;
    }
    if(!m_moveRequests.isEmpty())
    {
        m_strLastMoveRequest = m_moveRequests[m_moveRequests.count()-1];
    }
    else
    {
        m_strLastMoveRequest = "";
    }
}


