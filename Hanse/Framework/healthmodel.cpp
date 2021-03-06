#include "healthmodel.h"
#include <QtCore>
#include <QColor>
#include <QFont>


HealthModel::HealthModel(QObject *parent, ModulesGraph* graph)
    : QAbstractTableModel(parent)
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
    emit dataChanged(index(0,0), index(size,4));
}

void HealthModel::timerEvent()
{
    int s = graph->getModules().size();
    emit dataChanged(index(0,0), index(s-1,4));
}

void HealthModel::healthStatusChanged(RobotModule *module)
{
    int mi = graph->getModules().indexOf(module);
    emit dataChanged(index(mi,0), index(mi,4));
}

int HealthModel::rowCount(const QModelIndex __attribute__ ((unused)) &parent) const
{
    return graph->getModules().size();
}


int HealthModel::columnCount(const QModelIndex __attribute__ ((unused)) &parent) const
{
    return 5;
}

QVariant HealthModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {

        if (orientation == Qt::Vertical) {
        } else {
            switch (section) {
            case 0:
                return "Module";
            case 1:
                return "Enabled";
            case 2:
                return "Health OK";
            case 3:
                return "Error count";
            case 4:
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
            return m->getTabName();
        case 1:
            return m->isEnabled();
        case 2:
            return m->getHealthStatus().isHealthOk();
        case 3:
            return m->getHealthStatus().getErrorCount();
        case 4:
            return m->getHealthStatus().getLastError();
        }
    } else if (role == Qt::DecorationRole) {
        if (index.column()==2) {
            return m->getHealthStatus().isHealthOk() ? QColor("green") : QColor("red");
        }
    } else if (role == Qt::FontRole && !m->isEnabled() && index.column()>1){
        QFont f;
        f.setStrikeOut(true);
        return f;
    }

    return QVariant::Invalid;
}
