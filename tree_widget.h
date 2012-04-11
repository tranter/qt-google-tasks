#ifndef TREE_WIDGET_H
#define TREE_WIDGET_H

#include <QTreeWidget>

class TreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit TreeWidget(QWidget *parent = 0);

    void drawBranches(QPainter * painter, const QRect & rect, const QModelIndex & index) const;

signals:

public slots:

};

#endif // TREE_WIDGET_H
