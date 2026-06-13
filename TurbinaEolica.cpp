#include "TurbinaEolica.h"
#include <cmath>

TurbinaEolica::TurbinaEolica(int id, double velVientoInicial)
    : id(id), velocidadViento(velVientoInicial), potencia(0.0), estado("Apagado")
{}

int TurbinaEolica::getId() const { return id; }

double TurbinaEolica::getVelocidadViento() const { return velocidadViento; }

void TurbinaEolica::setVelocidadViento(double nuevaVel) { velocidadViento = nuevaVel; }

double TurbinaEolica::getPotencia() const { return potencia; }

QString TurbinaEolica::getEstado() const { return estado; }

void TurbinaEolica::setEstado(const QString &nuevoEstado) { estado = nuevoEstado; }

double TurbinaEolica::calcularPotencia()
{
    if (estado == "Encendido") {
        potencia = 0.5 * velocidadViento * velocidadViento * velocidadViento;
        if (potencia < 0) potencia = 0;
    } else {
        potencia = 0;
    }
    return potencia;
}