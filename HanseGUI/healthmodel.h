#ifndef HEALTHMODEL_H
#define HEALTHMODEL_H

#include <QtCore>
#include <QAbstractTableModel>
#include "modulesgraph.h"

class HealthModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    HealthModel(ModulesGraph* graph);

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;


private:
    ModulesGraph* graph;

private slots:
    void healthStatusChanged(RobotModule *module);
};

#endif // HEALTHMODEL_H
