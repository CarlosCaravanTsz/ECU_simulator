#pragma once

#include <string>
#include <vector>
#include "sensor.h"

enum class EstadoDtc { INACTIVO, PENDIENTE, CONFIRMADO };

struct Dtc {
    std::string codigo; // "P0217"
    std::string sensor; // "Temperatura"
    bool critico;
    int contador = 0; // debounce
    EstadoDtc estado = EstadoDtc::INACTIVO;
    int cicloFoto = 0; // freeze frame
    std::vector<double> foto // valores de todos los sensores
}

class Gestor Dtc {
    private:
        std::vector<Dtc> dtcs;
        int ciclo = 0;
    public:
    static constexpr int UMBRAL = 3;
    
    GestorDtc();
    void evaluar(const std::vectoer<Sensor>& sensores);
    bool hayConfirmado(bool soloCriticos) const;
    const std::vector<Dtc>& obtenerDtcs() const;
    void borrar(); // Clear dtc del escaner
};

