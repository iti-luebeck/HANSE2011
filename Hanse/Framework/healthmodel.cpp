#include "healthmodel.h"
#include <QtCore>
#include <QColor>


HealthModel::HealthModel(ModulesGraph* graph)
{
    this->graph = graph;

    foreach (RobotModule *module, graph->getModules()) {
        //connect(module, SIGNAL(healthStatusChanged(RobotModule*)), this, SLOT(healthStatusChanged(RobotModule*)));
        connect(module,SIGNAL(enabled(bool)), this, SLOT(moduleEnabled(bool)));
    }

    // update table via timer for now
    connect(&timer, SIGNAL(timeout()), this, SLOT(timerEvent()));
    timer.start(500);
}

void HealthModel::moduleEnabled(bool)
{
    // we don't easily know which module changed, so just refresh the whole table
    int size = graph->getModules().size();
    emit dataChanged(index(0,0), index(size,3));
}

void HealthModel::timerEvent()
{
    int s = graph->getModules().size();
    emit dataChanged(index(0,0), index(s-1,3));
}

void HealthModel::healthStatusChanged(RobotModule *module)
{
    int mi = graph->getModules().indexOf(module);
    emit dataChanged(index(mi,0), index(mi,3));
}

int HealthModel::rowCount(const QModelIndex &parent) const
{
    return graph->getModules().size();
}


int HealthModel::columnCount(const QModelIndex &parent) const
{
    return 4;
}

QVariant HealthModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {

        if (orientation == Qt::Vertical) {
            return graph->getModules().at(section)->getTabName();
        } else {
            switch (section) {
            case 0:
                return "Enabled";
            case 1:
                return "Health OK";
            case 2:
                return "Error count";
            case 3:
                return "Last error";
            }
        }
    }

    return QVariant::Invalid;
}

QVariant HealthModel::data(const QModelIndex &index, int role) const
{
    RobotModule *m = graph->getModules().at(index.row());
    if (role == Qt::DisplayRole) {

        switch (index.column())
        {
        case 0:
            return m->isEnabled();
        case 1:
            return m->getHealthStatus().isHealthOk();
        case 2:
            return m->getHealthStatus().getErrorCount();
        case 3:
            return m->getHealthStatus().getLastError();
        }
    } else if (role == Qt::DecorationRole) {
        if (index.column()==1) {
            return m->getHealthStatus().isHealthOk() ? QColor("green") : QColor("red");
        }
    }

    return QVariant::Invalid;
}
