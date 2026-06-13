#include "ParqueEolico.h"
#include <QDebug>

ParqueEolico::ParqueEolico(QObject *parent)
    : QObject(parent), produccionTotal(0.0)
{
}

ParqueEolico::~ParqueEolico()
{
    for (TurbinaEolica *t : turbinas)
        delete t;
    turbinas.clear();
}

void ParqueEolico::agregarTurbina(int id, double velInicial)
{
    if (buscarTurbina(id) != nullptr) {
        qDebug() << "Error: Ya existe turbina con ID" << id;
        return;
    }
    TurbinaEolica *nueva = new TurbinaEolica(id, velInicial);
    nueva->setEstado("Encendido");
    turbinas.append(nueva);
    qDebug() << "Turbina" << id << "agregada. Total:" << turbinas.size();
    emit datosActualizados();
}

bool ParqueEolico::eliminarTurbina(int id)
{
    for (int i = 0; i < turbinas.size(); ++i) {
        if (turbinas[i]->getId() == id) {
            delete turbinas[i];
            turbinas.remove(i);
            qDebug() << "Turbina" << id << "eliminada.";
            emit datosActualizados();
            return true;
        }
    }
    qDebug() << "Error: No se encontró turbina con ID" << id;
    return false;
}

TurbinaEolica* ParqueEolico::buscarTurbina(int id) const
{
    for (TurbinaEolica *t : turbinas) {
        if (t->getId() == id)
            return t;
    }
    return nullptr;
}

double ParqueEolico::calcularProduccionTotal()
{
    double total = 0.0;
    for (TurbinaEolica *t : turbinas) {
        total += t->calcularPotencia();
    }
    produccionTotal = total;
    return total;
}

QString ParqueEolico::mostrarResumen() const
{
    QString resumen = "=== RESUMEN DEL PARQUE EÓLICO ===\n";
    resumen += QString("Número de turbinas: %1\n").arg(turbinas.size());
    resumen += "Turbinas:\n";
    for (TurbinaEolica *t : turbinas) {
        resumen += QString(" ID %1 | Vel=%2 m/s | Pot=%3 kW | Estado=%4\n")
        .arg(t->getId())
            .arg(t->getVelocidadViento())
            .arg(t->calcularPotencia())
            .arg(t->getEstado());
    }
    resumen += QString("Producción total actual: %1 kW\n").arg(produccionTotal);
    return resumen;
}

const QVector<TurbinaEolica*>& ParqueEolico::getTurbinas() const
{
    return turbinas;
}

int ParqueEolico::getCantidadTurbinas() const
{
    return turbinas.size();
}