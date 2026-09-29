#pragma once

#include <map>
#include <string>

#include "ecu.h"

class Dashboard {

    private:

        std::map<EstadoECU, std::string> nombresEstados;

        void mostrarBarra(double valor, double minimo, double maximo, int ancho) const;

    public:

        Dashboard();

        void mostrar(const ECU& ecu) const;

};