#include <iostream>
#include <chrono>
#include <thread>

#include "ecu.h"
#include "vehicleSimulator.h"
#include "dashboard.h"


int main(){

    VehicleSimulator simulador(12345);

    ECU ecu;

    Dashboard Dashboard;

    while(true)
    {
        simulador.actualizar();

        ecu.actualizar(
            simulador.obtenerVelocidad(),
            simulador.obtenerRPM(),
            simulador.obtenerTemperatura(),
            simulador.obtenerVoltaje(),
            simulador.obtenerPresion()
        );

        Dashboard.mostrar(ecu);

        std::cout << "---------------------------------------------\n";
        std::cout << "Acelerador: " << simulador.obtenerAcelerador() << " %\n";

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    
    return 0;
}