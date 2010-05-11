#ifndef DATAMODEL_H
#define DATAMODEL_H

#include <QtCore>
#include <QAbstractTableModel>
#include "modulesgraph.h"

class DataModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    DataModel(ModulesGraph* graph);

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;


private:
    ModulesGraph* graph;
    QTimer timer;
    QMap<QString,QVariant> mergedMap;

private slots:
    void updateModel();
};

#endif // DATAMODEL_H
