#include "tree_widget.h"
#include <QDebug>

TreeWidget::TreeWidget(QWidget *parent) :
    QTreeWidget(parent)
{
}

void TreeWidget::drawBranches(QPainter *  /* painter*/,
                              const QRect& /* rect */,
                              const QModelIndex & /* index */) const
{
}
