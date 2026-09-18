#pragma once

#include "/home/oscarg/ECU_Simulator_7-8/include/sensor.h"

enum class EstadoECU
{
    INIT,
    OPERATIONAL,
    DEGRADED,
    SAFE_STATE
};


class ECU{
    
    private:

        Sensor velocidad;
        Sensor rpm;
        Sensor temperatura;
        Sensor bateria;
        Sensor presionAceite;

        EstadoECU estado;

    public:

        ECU();

        void actualizar(double velocidad, 
                        double rpm, 
                        double temperatura, 
                        double bateria, 
                        double presionAceite);

        void diagnosticar();

        EstadoECU obtenerEstado() const;

        const Sensor& obtenerSensorVelocidad() const;
        const Sensor& obtenerSensorRPM() const;
        const Sensor& obtenerSensorTemperatura() const;
        const Sensor& obtenerSensorBateria() const;
        const Sensor& obtenerSensorPresionAceite() const;

};