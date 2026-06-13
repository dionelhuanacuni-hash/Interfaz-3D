#ifndef TURBINA_EOLICA_H
#define TURBINA_EOLICA_H

#include <QString>

class TurbinaEolica
{
public:
    TurbinaEolica(int id, double velVientoInicial = 5.0);

    int getId() const;
    double getVelocidadViento() const;
    void setVelocidadViento(double nuevaVel);
    double getPotencia() const;
    QString getEstado() const;
    void setEstado(const QString &nuevoEstado);

    double calcularPotencia();

private:
    int id;
    double velocidadViento;
    double potencia;
    QString estado;
};

#endif // TURBINA_EOLICA_H