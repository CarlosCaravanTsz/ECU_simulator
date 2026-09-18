#include "/home/oscarg/ECU_Simulator_7-8/include/ecu.h"

ECU::ECU() 

:

velocidad("Velocidad", " km/h", 0.0, 220.0),
rpm("RPM", " rpm", 600.0, 7000.0),
temperatura("Temperatura", " C", -20.0, 110.0),
bateria("Bateria", " V", 11.5, 15.0),
presionAceite("Presion aceite", " psi", 0.8, 6.0),

estado(EstadoECU::INIT)
{
}


void ECU::actualizar(
    double nuevaVelocidad, 
    double nuevaRpm, 
    double nuevaTemperatura, 
    double nuevaBateria, 
    double nuevaPresionAceite)
{
    
    velocidad.actualizar(nuevaVelocidad);
    rpm.actualizar(nuevaRpm);
    temperatura.actualizar(nuevaTemperatura);
    bateria.actualizar(nuevaBateria);
    presionAceite.actualizar(nuevaPresionAceite);
    
    diagnosticar();
}

void ECU::diagnosticar()
{
    if (!temperatura.esValido() || !presionAceite.esValido())
    {
        estado = EstadoECU::SAFE_STATE;
    }

    else if (!bateria.esValido() || !rpm.esValido())
    {
        estado = EstadoECU::DEGRADED;
    }

    else 
    {
        estado = EstadoECU::OPERATIONAL;
    }
}

EstadoECU ECU::obtenerEstado() const
{
    return estado;
}

const Sensor& ECU::obtenerSensorVelocidad() const 
{
    return velocidad;
}

const Sensor& ECU::obtenerSensorRPM() const
{
    return rpm;
}

const Sensor& ECU::obtenerSensorTemperatura() const
{
    return temperatura;
}

const Sensor& ECU::obtenerSensorBateria() const
{
    return bateria;
}

const Sensor& ECU::obtenerSensorPresionAceite() const
{
    return presionAceite;
}