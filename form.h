#ifndef FORM_H
#define FORM_H

#include <QWidget>
#include "oauth2.h"
#include <QVariantList>
#include "tasks_data_manager.h"

class QListWidgetItem;

namespace Ui {
    class Form;
}

class Form : public QWidget
{
    Q_OBJECT

public:
    explicit Form(QWidget *parent = 0);
    ~Form();

    void startLogin(bool bForce);

private slots:
    void getTaskListsFromManager();
    void getTasksFromManager();
    void errorOccured(const QString& error);

    void loadLists();
    void loadTasks();
    void showTaskInfo();

    void deleteTask();
    void newTask();
    void updateTask();

    void newList();
    void deleteList();

    void leftClicked();
    void rightClicked();
    void upClicked();
    void downClicked();
    void onUserEmailReady();

    void taskItemChanged(class QTreeWidgetItem * item, int column);

private:
    int getCurrentTaskIndex();
    void saveSettings();


private:
    Ui::Form *ui;
    OAuth2 m_oauth2;
    QVariantList m_taskLists;
    QVariantList m_tasks;
    TasksDataManager m_tasksDataManager;

    QSettings* m_pSettings;
};

#endif // FORM_H
