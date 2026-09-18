#pragma once

#include "/home/oscarg/ECU_Simulator_7-8/include/ecu.h"

class Dashboard {

    private:

        const std::string estadoAtexto(EstadoECU estado) const;

    public:

        void mostrar(const ECU& ecu) const;

};