#include <QMessageBox>
#include <QInputDialog>
#include <QSettings>

#include "form.h"
#include "ui_form.h"

Form::Form(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form)
{
    ui->setupUi(this);

    m_strCompanyName = "YOU_COMPANY_NAME_HERE";
    m_strAppName = "QtTasks";

    // Load settings
    m_pSettings = new QSettings(m_strCompanyName, m_strAppName);
    m_oauth2.setAccessToken(m_pSettings->value("access_token").toString());
    m_oauth2.setRefreshToken(m_pSettings->value("refresh_token").toString());
    m_oauth2.setSettings(m_pSettings);

    connect(ui->listWidget, SIGNAL(currentRowChanged(int)), this, SLOT(loadTasks()));
    //connect(ui->tasksListWidget, SIGNAL(currentRowChanged(int)), this, SLOT(showTaskInfo()));

    connect(ui->deleteTaskButton, SIGNAL(clicked()), this, SLOT(deleteTask()));
    connect(ui->newTaskButton, SIGNAL(clicked()), this, SLOT(newTask()));
    connect(ui->updateTaskButton, SIGNAL(clicked()), this, SLOT(updateTask()));

    connect(ui->leftButton, SIGNAL(clicked()), this, SLOT(leftClicked()));
    connect(ui->rightButton, SIGNAL(clicked()), this, SLOT(rightClicked()));
    connect(ui->upButton, SIGNAL(clicked()), this, SLOT(upClicked()));
    connect(ui->downButton, SIGNAL(clicked()), this, SLOT(downClicked()));

    connect(ui->deleteListButton, SIGNAL(clicked()), this, SLOT(deleteList()));
    connect(ui->newListButton, SIGNAL(clicked()), this, SLOT(newList()));


    connect(&m_oauth2, SIGNAL(sigErrorOccured(QString)),this,SLOT(errorOccured(QString)));
    connect(&m_oauth2, SIGNAL(loginDone()), this, SLOT(loadLists()));

    connect(&m_tasksDataManager, SIGNAL(taskListsReady()), this, SLOT(getTaskListsFromManager()));
    connect(&m_tasksDataManager, SIGNAL(tasksReady()),  this, SLOT(getTasksFromManager()));
    connect(&m_tasksDataManager, SIGNAL(errorOccured(QString)), this, SLOT(errorOccured(QString)));
    connect(&m_tasksDataManager, SIGNAL(taskChanged()), this, SLOT(loadTasks()));
    connect(&m_tasksDataManager, SIGNAL(listsChanged()), this, SLOT(loadLists()));
}

Form::~Form()
{
    saveSettings();
    delete m_pSettings;
    delete ui;
}

void Form::startLogin(bool bForce)
{
    //Now we allow to start logging in when m_oauth2.isAuthorized(). User can log in using another Google account.
    //if(!m_oauth2.isAuthorized())
    {
        m_oauth2.startLogin(this, bForce); //this is a parent widget for a login dialog.
    }
}


void Form::loadLists()
{
    disconnect(ui->listWidget, SIGNAL(currentRowChanged(int)), this, SLOT(loadTasks()));
    disconnect(ui->treeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem* )), this, SLOT(showTaskInfo()));
    disconnect(ui->treeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(taskItemChanged(QTreeWidgetItem*, int)));


    //ui->tasksListWidget->clear();
    ui->listWidget->clear();
    ui->treeWidget->clear();

    m_tasksDataManager.getMyTaskLists(m_oauth2.accessToken());
}

void Form::getTaskListsFromManager()
{
    m_taskLists = m_tasksDataManager.getTaskLists();
    QStringList lst;
    for(int i = 0; i < m_taskLists.count(); ++i)
    {
        lst << m_taskLists[i].toMap()["title"].toString();
    }
    ui->listWidget->clear();
    ui->listWidget->addItems(lst);

    connect(ui->listWidget, SIGNAL(currentRowChanged(int)), this, SLOT(loadTasks()));
    connect(ui->treeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem* )), this, SLOT(showTaskInfo()));
}

void Form::getTasksFromManager()
{
    disconnect(ui->treeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(taskItemChanged(QTreeWidgetItem*, int)));

    int sel_row = getCurrentTaskIndex();
    QString selID = "";
    if(sel_row != -1)
    {
        selID = m_tasks[sel_row].toMap()["id"].toString();
    }

    m_tasks = m_tasksDataManager.getTasks();
    QStringList lst;
    for(int i = 0; i < m_tasks.count(); ++i)
    {
        lst << m_tasks[i].toMap()["title"].toString();
    }
    ui->treeWidget->clear();

    QList<QTreeWidgetItem *> items;
    for(int i = 0; i < m_tasks.count(); ++i)
    {
        Qt::CheckState state = Qt::Unchecked;
        if(m_tasks[i].toMap()["status"].toString() == "completed")
        {
            state = Qt::Checked;
        }
        QString name = m_tasks[i].toMap()["title"].toString();
        QString parentID = m_tasks[i].toMap()["parent"].toString();
        QTreeWidgetItem* parentItem = 0;
        if(!parentID.isEmpty())
        {
            for(int j = 0; j < i; ++j)
            {
                if(m_tasks[j].toMap()["id"].toString() == parentID)
                {
                    parentItem = items[j];
                    break;
                }
            }
        }
        QTreeWidgetItem* item = new QTreeWidgetItem(parentItem, QStringList(name));
        item->setFlags(Qt::ItemIsEditable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        item->setData(0, Qt::UserRole, i);
        item->setCheckState(0, state);

        QFont font = item->font(0);
        font.setStrikeOut(state == Qt::Checked);
        item->setFont(0, font);

        items.append(item);

    }

    ui->treeWidget->insertTopLevelItems(0, items);
    ui->treeWidget->expandAll();

    for(int i = 0; i < m_tasks.count(); ++i)
    {
        if(m_tasks[i].toMap()["id"].toString() == selID)
        {
            ui->treeWidget->setCurrentItem(items[i]);
            break;
        }
    }

    connect(ui->treeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(taskItemChanged(QTreeWidgetItem*, int)));
}

void Form::errorOccured(const QString& error)
{
    if(error == "Invalid Credentials")
    {
        startLogin(true);
    }
    else
    {
        QMessageBox::warning(this, tr("Error occured"), error);
    }
}

void Form::loadTasks()
{
    int index = ui->listWidget->currentRow();
    QString  taskListID = m_taskLists[index].toMap()["id"].toString();
    m_tasksDataManager.getMyTasks(m_oauth2.accessToken(), taskListID);
}

void Form::showTaskInfo()
{
    int index = getCurrentTaskIndex();
    if(index == -1)
    {
        ui->taskCheckBox->setText("");
        ui->taskCheckBox->setCheckState(Qt::Unchecked);
        ui->notesTextEdit->setPlainText("");
        ui->dueDateEdit->setDate(QDate::currentDate());
        ui->dueDateCheckBox->setChecked(false);
        return;
    }
    ui->taskCheckBox->setText(m_tasks[index].toMap()["title"].toString());
    Qt::CheckState state = Qt::Unchecked;
    if(m_tasks[index].toMap()["status"].toString() == "completed")
    {
        state = Qt::Checked;
    }
    ui->taskCheckBox->setCheckState(state);
    ui->notesTextEdit->setPlainText(m_tasks[index].toMap()["notes"].toString());

    QString strDate = m_tasks[index].toMap()["due"].toString();
    if(strDate.isEmpty())
    {
        ui->dueDateEdit->setDate(QDate::currentDate());
        ui->dueDateCheckBox->setChecked(false);
    }
    else
    {
        QStringList lst = strDate.split("T");
        QStringList items = lst[0].split("-");
        QDate date(items[0].toInt(), items[1].toInt(), items[2].toInt());
        ui->dueDateEdit->setDate(date);
        ui->dueDateCheckBox->setChecked(true);
    }
}

void Form::deleteTask()
{
    int index = getCurrentTaskIndex();
    if(index == -1)
    {
        QMessageBox::warning(this, tr("Warning"), tr("No selected task."));
        return;
    }
    QString taskID = m_tasks[index].toMap()["id"].toString();

    index = ui->listWidget->currentRow();
    if(index == -1)
    {
        QMessageBox::warning(this, tr("Warning"), tr("No selected tasks list."));
        return;
    }
    QString taskListID = m_taskLists[index].toMap()["id"].toString();
    m_tasksDataManager.deleteTask(m_oauth2.accessToken(), taskListID, taskID);
}

void Form::newTask()
{
    QString strTitle = QInputDialog::getText(this, "Create new task", "Task name");
    if(strTitle.isEmpty())
    {
        return;
    }
    int index = ui->listWidget->currentRow();
    if(index == -1)
    {
        QMessageBox::warning(this, tr("Warning"), tr("No selected tasks list."));
        return;
    }
    QString taskListID = m_taskLists[index].toMap()["id"].toString();

    QString prevTaskID = "";
    QString parentID = "";

    index = getCurrentTaskIndex();
    if(index != -1)
    {
        prevTaskID = m_tasks[index].toMap()["id"].toString();
        parentID = m_tasks[index].toMap()["parent"].toString();
        //Let's us see, if prevTaskID is a parent for some other items.
        bool bHasChildren = false;
        for(int i = index +1; i < m_tasks.count(); ++i)
        {
            if(m_tasks[i].toMap()["parent"].toString() == prevTaskID)
            {
                bHasChildren = true;
                break;
            }
        }
        if(bHasChildren)
        {
            parentID = prevTaskID;
            prevTaskID = "";
        }
    }

    m_tasksDataManager.createTask(m_oauth2.accessToken(), taskListID, strTitle, prevTaskID, parentID);
}

void Form::updateTask()
{
    int index = getCurrentTaskIndex();
    if(index == -1)
    {
        QMessageBox::warning(this, tr("Warning"), tr("No selected task."));
        return;
    }
    QString taskID = m_tasks[index].toMap()["id"].toString();
    QVariantMap json_object = m_tasks[index].toMap();

    index = ui->listWidget->currentRow();
    if(index == -1)
    {
        QMessageBox::warning(this, tr("Warning"), tr("No selected tasks list."));
        return;
    }
    QString taskListID = m_taskLists[index].toMap()["id"].toString();

    if(ui->taskCheckBox->isChecked())
    {
        json_object["status"] = "completed";
    }
    else
    {
        json_object.remove("completed");
        json_object["status"] = "needsAction";
    }
    json_object["notes"] = ui->notesTextEdit->toPlainText();
    if(ui->dueDateCheckBox->isChecked())
    {
        QString str = ui->dueDateEdit->date().toString(Qt::ISODate) + "T00:00:00.000Z";
        json_object["due"] = str;
    }
    else
    {
        json_object.remove("due");
    }
    m_tasksDataManager.updateTask(m_oauth2.accessToken(), taskListID, taskID, json_object);
}

void Form::taskItemChanged(QTreeWidgetItem * item, int column)
{
    if( column != 0 ) return;

    int index = item->data(0, Qt::UserRole).toInt();

    QString taskID = m_tasks[index].toMap()["id"].toString();
    QVariantMap json_object = m_tasks[index].toMap();

    index = ui->listWidget->currentRow();
    if(index == -1)
    {
        QMessageBox::warning(this, tr("Warning"), tr("No selected tasks list."));
        return;
    }
    QString taskListID = m_taskLists[index].toMap()["id"].toString();

    if(item->checkState(0) == Qt::Checked)
    {
        json_object["status"] = "completed";
    }
    else
    {
        json_object.remove("completed");
        json_object["status"] = "needsAction";
    }
    json_object["title"] = item->text(0);
    m_tasksDataManager.updateTask(m_oauth2.accessToken(), taskListID, taskID, json_object);
}

void Form::newList()
{
    QString strTitle = QInputDialog::getText(this, "Create new list", "List name");
    if(strTitle.isEmpty())
    {
        return;
    }
    m_tasksDataManager.createList(m_oauth2.accessToken(), strTitle);
}

void Form::deleteList()
{
    int index = ui->listWidget->currentRow();
    if(index == -1)
    {
        QMessageBox::warning(this, tr("Warning"), tr("No selected tasks list."));
        return;
    }
    QString taskListID = m_taskLists[index].toMap()["id"].toString();
    m_tasksDataManager.deleteList(m_oauth2.accessToken(), taskListID);
    ui->treeWidget->clear();
}

int Form::getCurrentTaskIndex()
{
    QTreeWidgetItem* item = ui->treeWidget->currentItem();
    int index = -1;
    if(item)
    {
        index = item->data(0, Qt::UserRole).toInt();
    }
    return index;
}

void Form::leftClicked()
{
    int index = getCurrentTaskIndex();
    if(index == -1)
    {
        QMessageBox::warning(this, tr("Warning"), tr("No selected task."));
        return;
    }

    int index1 = ui->listWidget->currentRow();
    if(index1 == -1)
    {
        QMessageBox::warning(this, tr("Warning"), tr("No selected tasks list."));
        return;
    }
    QString taskListID = m_taskLists[index1].toMap()["id"].toString();

    QString taskID   = m_tasks[index].toMap()["id"].toString();
    QString parentID = m_tasks[index].toMap()["parent"].toString();
    if(parentID.isEmpty())
    {
        //Cannot move left!
        return;
    }
    QString newParentID = "";
    QString newParentTitle = "";
    for(int i = 0; i < index; ++i)
    {
        if(m_tasks[i].toMap()["id"].toString() == parentID)
        {
            newParentID = m_tasks[i].toMap()["parent"].toString();
            break;
        }
    }
    QString prevID = "";
    for(int i = 0; i < index; ++i)
    {
        if(m_tasks[i].toMap()["parent"].toString() == newParentID)
        {
            prevID = m_tasks[i].toMap()["id"].toString();
        }
    }

    m_tasksDataManager.startMoving();
    m_tasksDataManager.moveTask(m_oauth2.accessToken(), taskListID, taskID, prevID, newParentID);

////    IMPORTANT!!!
////    Find all the items, that had the same parent as our current, and set to them new parent - our current!!!
////    We do not need to do this work while moving right. Without those calls the "current" item will change
////    the order.
    QString prevID1 = "";
    for(int i = index+1; i < m_tasks.count(); ++i)
    {
        if(m_tasks[i].toMap()["parent"].toString() == parentID)
        {
            QString id = m_tasks[i].toMap()["id"].toString();

            m_tasksDataManager.moveTask(m_oauth2.accessToken(), taskListID, id, prevID1, taskID);
            prevID1 = id;
        }
    }
    m_tasksDataManager.endMoving();
}

void Form::rightClicked()
{
    int index = getCurrentTaskIndex();
    if(index == -1)
    {
        QMessageBox::warning(this, tr("Warning"), tr("No selected task."));
        return;
    }
    int index1 = ui->listWidget->currentRow();
    if(index1 == -1)
    {
        QMessageBox::warning(this, tr("Warning"), tr("No selected tasks list."));
        return;
    }
    QString taskListID = m_taskLists[index1].toMap()["id"].toString();

    QString taskID   = m_tasks[index].toMap()["id"].toString();
    QString parentID = m_tasks[index].toMap()["parent"].toString();

    //This is the nearest item, that has the same parent, as the current one!
    //This item is going to be the next parent (if any).
    QString newParentID = "";
    for(int i = 0; i < index; ++i)
    {
        if(m_tasks[i].toMap()["parent"].toString() == parentID)
        {
            newParentID = m_tasks[i].toMap()["id"].toString();
        }
    }

    if(newParentID.isEmpty())
    {
        return;
    }
    QString prevID = "";
    for(int i = 0; i < index; ++i)
    {
        if(m_tasks[i].toMap()["parent"].toString() == newParentID)
        {
            prevID = m_tasks[i].toMap()["id"].toString();
        }
    }

    m_tasksDataManager.startMoving();
    m_tasksDataManager.moveTask(m_oauth2.accessToken(), taskListID, taskID, prevID, newParentID);
    m_tasksDataManager.endMoving();
}

void Form::upClicked()
{
    int index = getCurrentTaskIndex();
    if(index == -1)
    {
        QMessageBox::warning(this, tr("Warning"), tr("No selected task."));
        return;
    }
    int index1 = ui->listWidget->currentRow();
    if(index1 == -1)
    {
        QMessageBox::warning(this, tr("Warning"), tr("No selected tasks list."));
        return;
    }
    QString taskListID = m_taskLists[index1].toMap()["id"].toString();

    QString taskID   = m_tasks[index].toMap()["id"].toString();
    QString parentID = m_tasks[index].toMap()["parent"].toString();

    int count = 0;
    QString prevID = "";
    for(int i = index-1; i >=0; --i)
    {
        if(m_tasks[i].toMap()["parent"].toString() == parentID)
        {
            if(count == 0)
            {
                ++count;
            }
            else
            {
                prevID = m_tasks[i].toMap()["id"].toString();
                break;
            }
        }
    }
    if(count)
    {
        m_tasksDataManager.startMoving();
        m_tasksDataManager.moveTask(m_oauth2.accessToken(), taskListID, taskID, prevID, parentID);
        m_tasksDataManager.endMoving();
    }
    else if(!parentID.isEmpty())
    {
        QString newParentID = "";
        for(int i = 0; i < index; ++i)
        {
            if(m_tasks[i].toMap()["parent"].toString() == parentID)
            {
                newParentID = m_tasks[i].toMap()["parent"].toString();
            }
        }
        for(int i = index-1; i >=0; --i)
        {
            if(m_tasks[i].toMap()["parent"].toString() == newParentID)
            {
                if(count == 0)
                {
                    ++count;
                }
                else
                {
                    prevID = m_tasks[i].toMap()["id"].toString();
                    break;
                }
            }
        }
        if(count)
        {
            m_tasksDataManager.startMoving();
            m_tasksDataManager.moveTask(m_oauth2.accessToken(), taskListID, taskID, prevID, newParentID);
            m_tasksDataManager.endMoving();
        }
    }
}

void Form::downClicked()
{
    int index = getCurrentTaskIndex();
    if(index == -1)
    {
        QMessageBox::warning(this, tr("Warning"), tr("No selected task."));
        return;
    }
    int index1 = ui->listWidget->currentRow();
    if(index1 == -1)
    {
        QMessageBox::warning(this, tr("Warning"), tr("No selected tasks list."));
        return;
    }
    QString taskListID = m_taskLists[index1].toMap()["id"].toString();

    QString taskID   = m_tasks[index].toMap()["id"].toString();
    QString parentID = m_tasks[index].toMap()["parent"].toString();

    QString nextID = "";
    for(int i = index+1; i < m_tasks.count(); ++i)
    {
        if(m_tasks[i].toMap()["parent"].toString() == parentID)
        {
            nextID = m_tasks[i].toMap()["id"].toString();
            break;
        }
    }
    if(!nextID.isEmpty())
    {
        m_tasksDataManager.startMoving();
        m_tasksDataManager.moveTask(m_oauth2.accessToken(), taskListID, taskID, nextID, parentID);
        m_tasksDataManager.endMoving();
    }
    else if(!parentID.isEmpty())
    {
        QString newParentID = "";
        for(int i = 0; i < index; ++i)
        {
            if(m_tasks[i].toMap()["parent"].toString() == parentID)
            {
                newParentID = m_tasks[i].toMap()["parent"].toString();
            }
        }
        for(int i = index+1; i < m_tasks.count(); ++i)
        {
            if(m_tasks[i].toMap()["parent"].toString() == newParentID)
            {
                nextID = m_tasks[i].toMap()["id"].toString();
                break;
            }
        }
        if(!nextID.isEmpty())
        {
            m_tasksDataManager.startMoving();
            m_tasksDataManager.moveTask(m_oauth2.accessToken(), taskListID, taskID, nextID, newParentID);
            m_tasksDataManager.endMoving();
        }
    }

}

void Form::saveSettings()
{
    m_pSettings->setValue("access_token",m_oauth2.accessToken());
    m_pSettings->setValue("refresh_token",m_oauth2.refreshToken());
}
