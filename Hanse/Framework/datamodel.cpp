#include "datamodel.h"
#include <QtCore>

DataModel::DataModel(QObject *parent, ModulesGraph* graph)
    : QAbstractTableModel(parent)
{
    this->graph = graph;

    connect(&timer,SIGNAL(timeout()), this, SLOT(updateModel()));
    timer.start(500);

}

int DataModel::rowCount(const QModelIndex __attribute__ ((unused)) &parent) const
{
    return mergedMap.size();
}


int DataModel::columnCount(const QModelIndex __attribute__ ((unused)) &parent) const
{
    return 2;
}

void DataModel::updateModel()
{
    // TODO: we will not notice when a module *removes* an item from their data store.
    foreach (RobotModule *module, graph->getModules()) {
        QString name = module->getTabName();
        foreach (QString key, module->getDataCopy().keys()) {
            QString mk = name+"/"+key;
            if (mk.contains(filterModule,Qt::CaseInsensitive)) {
//                mergedMap.insert(mk,module->getData().value(key));
                if (!mergedMap.contains(mk)) {
                    // yes, this looks very wrong. but it also works. :)
                    // so until I figure out the Right Way(tm) to do this,
                    // just pray that this won't break.
//                    mergedMap[mk] = module->getData().value(key);
                    mergedMap.insert(mk,module->getDataCopy().value(key));
                    emit beginInsertRows(QModelIndex(),mergedMap.keys().indexOf(mk),mergedMap.keys().indexOf(mk));
                    emit endInsertRows();
                }
                else {
////                    mergedMap[mk] = module->getData().value(key);
                    mergedMap.insert(mk,module->getDataCopy().value(key));
                }
            } else if (mergedMap.contains(mk) ) {
                emit beginRemoveRows(QModelIndex(), mergedMap.keys().indexOf(mk),mergedMap.keys().indexOf(mk));
                mergedMap.remove(mk);
                emit endRemoveRows();
            }

        }
    }

    emit dataChanged(index(0,0),index(mergedMap.size()-1,1));
}

QVariant DataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {

        if (orientation == Qt::Horizontal) {

            switch (section) {
            case 0:
                return "Key";
            case 1:
                return "Value";
            }
        }
    }

    return QVariant::Invalid;
}

QVariant DataModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        if (index.column()==0) {
            return mergedMap.keys().at(index.row());
        } else {
            return mergedMap.values().at(index.row());
        }
    }

    return QVariant::Invalid;
}


void DataModel::setFilter(QString module)
{
   this->filterModule = module;
   updateModel();
}
