#include <algorithm>

#include "vehicleSimulator.h"

VehicleSimulator::VehicleSimulator(
        unsigned int semilla) 
        
        : 

        generador(semilla),
        ruido(0.0, 1.0),
        velocidad(0.0),
        rpm(800.0),
        temperatura(25.0),
        voltajeBateria(12.6),
        presionAceite(1.5),
        acelerador(0.0)
{
}

void VehicleSimulator::actualizar(){

    // Acelerador

    std::uniform_real_distribution<double> cambioAcelerador(-8.0, 8.0);

    acelerador += cambioAcelerador(generador);

    acelerador = std::clamp(acelerador, 0.0, 100.0);

    // Velocidad 

    double velocidadObjetivo = acelerador * 1.8;

    velocidad += (velocidadObjetivo - velocidad) * 0.05;

    velocidad = std::max(0.0, velocidad);

    // RPM

    rpm = 800.0 + velocidad * 22.0 + acelerador * 18.0 + ruido(generador) * 50;

    rpm = std::clamp(rpm, 700.0, 7000.0);


    // Temperatura 

    double temperaturaObjetivo = 85.0 + acelerador * 0.08;

    temperatura += (temperaturaObjetivo - temperatura) * 0.01;

    temperatura += ruido(generador) * 0.05;

    // Voltaje 

    voltajeBateria = 13.8 + ruido(generador)*0.08;

    // Presion de Aceite

    presionAceite = 1.0 + rpm/2000.0 + ruido(generador) * 0.05;

    presionAceite = std::max(0.0, presionAceite);

}


double VehicleSimulator::obtenerVelocidad() const{
    return velocidad;
}

double VehicleSimulator::obtenerRPM() const{
    return rpm;
}

double VehicleSimulator::obtenerTemperatura() const {
    return temperatura;
}

double VehicleSimulator::obtenerVoltaje() const {
    return voltajeBateria;
}

double VehicleSimulator::obtenerPresion() const {
    return presionAceite;
}

double VehicleSimulator::obtenerAcelerador() const {
    return acelerador;
}