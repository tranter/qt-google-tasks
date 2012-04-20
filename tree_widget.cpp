#include "tree_widget.h"

TreeWidget::TreeWidget(QWidget *parent) :
    QTreeWidget(parent)
{
}

void TreeWidget::drawBranches(QPainter *  /* painter*/,
                              const QRect& /* rect */,
                              const QModelIndex & /* index */) const
{
}
