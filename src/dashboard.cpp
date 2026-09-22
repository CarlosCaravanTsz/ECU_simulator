#include "/home/oscarg/ECU_Simulator_7-8/include/dashboard.h"

#include <iostream>
#include <iomanip>
#include <cstdlib>

const std::string Dashboard::estadoAtexto(EstadoECU estado) const
{
    switch(estado){

        case EstadoECU::INIT:
            return "INIT";

        case EstadoECU::OPERATIONAL:
            return "OPERATIONAL";

        case EstadoECU::DEGRADED:
            return "DEGRADED";

        case EstadoECU::SAFE_STATE:
            return "SAFE_STATE";

    }

    return "DESCONOCIDO";
}

void Dashboard::mostrar(const ECU& ecu) const
{
    system("clear");

    std::cout << "=============================================\n";
    std::cout << "             ECU VEHICLE DASHBOARD           \n";
    std::cout << "=============================================\n\n";

    std::cout << std::fixed << std::setprecision(2);

    std::cout << "Velocidad       : " << ecu.obtenerSensorVelocidad().obtenerValor() << ecu.obtenerSensorVelocidad().obtenerUnidad() <<"\n";
    std::cout << "RPM             : " << ecu.obtenerSensorRPM().obtenerValor() << ecu.obtenerSensorRPM().obtenerUnidad() <<"\n";
    std::cout << "Temperatura     : " << ecu.obtenerSensorTemperatura().obtenerValor() << ecu.obtenerSensorTemperatura().obtenerUnidad() <<"\n";
    std::cout << "Bateria         : " << ecu.obtenerSensorBateria().obtenerValor() << ecu.obtenerSensorBateria().obtenerUnidad() <<"\n";
    std::cout << "Presion aceite  : " << ecu.obtenerSensorPresionAceite().obtenerValor() << ecu.obtenerSensorPresionAceite().obtenerUnidad() <<"\n";

    std::cout << "---------------------------------------------\n";

    std::cout << "Estado ECU      : " << estadoAtexto(ecu.obtenerEstado()) << "\n";

    std::cout << "=============================================\n\n";
}
