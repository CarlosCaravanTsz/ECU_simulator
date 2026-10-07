#include "dashboard.h"

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <algorithm>

Dashboard::Dashboard()
{
    nombresEstados[EstadoECU::INIT] = "INIT";
    nombresEstados[EstadoECU::OPERATIONAL] = "OPERATIONAL";
    nombresEstados[EstadoECU::DEGRADED] = "DEGRADED";
    nombresEstados[EstadoECU::SAFE_STATE] = "SAFE_STATE";
}


/*
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

*/

void Dashboard::mostrarBarra(double valor, double minimo, double maximo, int ancho) const
{
    double proporcion = (valor - minimo)/(maximo - minimo);
    proporcion = std::clamp(proporcion, 0.0, 1.0);

    int cantidad = static_cast<int>(proporcion*ancho);

    std::cout << "[";

    for (int i = 0; i < ancho; i++)
    {
        if(i < cantidad)
        {
            std::cout << "#";
        }
        else
        {
            std::cout << "-";
        }
    }

    std::cout << "]";
}


void Dashboard::mostrar(const ECU& ecu) const
{
    system("clear");

    const std::vector<Sensor>& sensores = ecu.obtenerSensores();



    std::cout << "=============================================\n";
    std::cout << "             ECU VEHICLE DASHBOARD           \n";
    std::cout << "=============================================\n\n";

    std::cout << std::fixed << std::setprecision(2);

    std::cout << "---------------------------------------------\n";

    std::for_each(
        sensores.begin(),
        sensores.end(),
        [&](const Sensor& sensor)
        {
            std::cout << std::left << std::setw(18) << sensor.obtenerNombre();
            std::cout << std::right << std::setw(8) << sensor.obtenerValor() << " " << std::setw(6) << sensor.obtenerUnidad() << "  ";
            mostrarBarra(sensor.obtenerValor(), sensor.obtenerMinimo(), sensor.obtenerMaximo(), 15);
            if(!sensor.esValido())
            {
                std::cout << "   [FALLA]";
            }
            std::cout << "\n";
        }
    );

    // TODO: Usar la funcion de buscar sensores a partir de cada nombre, luego extraer las propiedades del objeto encontrado.

    /*
    std::cout << "Velocidad       : " << ecu.obtenerSensorVelocidad().obtenerValor() << ecu.obtenerSensorVelocidad().obtenerUnidad() <<"\n";
    std::cout << "RPM             : " << ecu.obtenerSensorRPM().obtenerValor() << ecu.obtenerSensorRPM().obtenerUnidad() <<"\n";
    std::cout << "Temperatura     : " << ecu.obtenerSensorTemperatura().obtenerValor() << ecu.obtenerSensorTemperatura().obtenerUnidad() <<"\n";
    std::cout << "Bateria         : " << ecu.obtenerSensorBateria().obtenerValor() << ecu.obtenerSensorBateria().obtenerUnidad() <<"\n";
    std::cout << "Presion aceite  : " << ecu.obtenerSensorPresionAceite().obtenerValor() << ecu.obtenerSensorPresionAceite().obtenerUnidad() <<"\n";

    */

    std::cout << "---------------------------------------------\n";

    std::cout << "Estado ECU      : " << nombresEstados.at(ecu.obtenerEstado()) << "\n";

    std::cout << "Sensores con Falla: " << ecu.contarSensoresFallas() << "\n";

    std::cout << "=============================================\n\n";
}
