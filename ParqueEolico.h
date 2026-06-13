#ifndef PARQUE_EOLICO_H
#define PARQUE_EOLICO_H

#include <QObject>
#include <QVector>
#include "TurbinaEolica.h"

class ParqueEolico : public QObject
{
    Q_OBJECT

public:
    explicit ParqueEolico(QObject *parent = nullptr);
    ~ParqueEolico();

    void agregarTurbina(int id, double velInicial = 5.0);
    bool eliminarTurbina(int id);
    TurbinaEolica* buscarTurbina(int id) const;
    double calcularProduccionTotal();
    QString mostrarResumen() const;

    const QVector<TurbinaEolica*>& getTurbinas() const;
    int getCantidadTurbinas() const;

signals:
    void datosActualizados();   // Señal para notificar cambios a la UI

private:
    QVector<TurbinaEolica*> turbinas;
    double produccionTotal;
};

#endif // PARQUE_EOLICO_H