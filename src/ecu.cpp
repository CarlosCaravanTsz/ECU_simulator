#include "/home/oscarg/ECU_Simulator_7-8/include/ecu.h"
#include <algorithm>

ECU::ECU() 

:

//velocidad("Velocidad", " km/h", 0.0, 220.0),
//rpm("RPM", " rpm", 600.0, 7000.0),
//temperatura("Temperatura", " C", -20.0, 110.0),
//bateria("Bateria", " V", 11.5, 15.0),
//presionAceite("Presion aceite", " psi", 0.8, 6.0),

estado(EstadoECU::INIT)
{
    sensores.emplace_back("Velocidad", " km/h", 0.0, 100.0);

    sensores.emplace_back("RPM", " rpm", 600.0, 4000.0);

    sensores.emplace_back("Temperatura", " C", -20.0, 90.0);

    sensores.emplace_back("Bateria", " V", 11.5, 15.0);

    sensores.emplace_back("Presion aceite", " psi", 0.8, 4.0);
}


void ECU::actualizar(
    double nuevaVelocidad, 
    double nuevaRpm, 
    double nuevaTemperatura, 
    double nuevaBateria, 
    double nuevaPresionAceite)
{
    
    Sensor* sensorVelocidad = nullptr;
    Sensor* sensorRPM = nullptr;
    Sensor* sensorTemperatura = nullptr;
    Sensor* sensorBateria = nullptr;
    Sensor* sensorPresion = nullptr;

    for (Sensor& sensor : sensores)
    {
        if(sensor.obtenerNombre() == "Velocidad")
        {
            sensorVelocidad = &sensor;
        }

        else if(sensor.obtenerNombre() == "RPM")
        {
            sensorRPM = &sensor;
        }
        
        else if(sensor.obtenerNombre() == "Temperatura")
        {
            sensorTemperatura = &sensor;
        }

        else if(sensor.obtenerNombre() == "Bateria")
        {
            sensorBateria = &sensor;
        }

        else if(sensor.obtenerNombre() == "Presion aceite")
        {
            sensorPresion = &sensor;
        }
    }

    if(sensorVelocidad != nullptr)
    {
        sensorVelocidad -> actualizar(nuevaVelocidad);
    }
    
    if(sensorRPM != nullptr)
    {
        sensorRPM -> actualizar(nuevaRpm);
    }

    if(sensorTemperatura != nullptr)
    {
        sensorTemperatura -> actualizar(nuevaTemperatura);
    }

    if(sensorBateria != nullptr)
    {
        sensorBateria -> actualizar(nuevaBateria);
    }

    if(sensorPresion != nullptr)
    {
        sensorPresion -> actualizar(nuevaPresionAceite);
    }
      
    diagnosticar();
}

void ECU::diagnosticar()
{

    bool fallaCritica = false;
    bool fallaNoCritica = false;

    std::for_each(sensores.begin(), sensores.end(),
    [&](const Sensor& sensor)
    {
        if(!sensor.esValido())
        {
            if (sensor.obtenerNombre() == "Temperatura" || sensor.obtenerNombre() == "Presion aceite")
            {
                fallaCritica = true;
            }
            else 
            {
                fallaNoCritica = true;
            }
        }
    }
    );

    if(fallaCritica)
    {
        estado = EstadoECU::SAFE_STATE;
    }

    else if (fallaNoCritica)
    {
        estado = EstadoECU::DEGRADED;
    }

    else
    {
        estado = EstadoECU::OPERATIONAL;
    }

    /*
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
    */
}

EstadoECU ECU::obtenerEstado() const
{
    return estado;
}

const std::vector<Sensor>& ECU::obtenerSensores() const
{
    return sensores;
}


const Sensor* ECU::buscarSensor(const std::string& nombre) const 
{   
    auto encontrado = std::find_if(
        sensores.begin(), 
        sensores.end(), 
        [&](const Sensor& sensor)
        {
            return sensor.obtenerNombre() == nombre;
        }
    );

    if(encontrado != sensores.end())
    {
        return &(*encontrado);          
    }

    return nullptr;
}

int ECU::contarSensoresFallas() const
{
    return static_cast<int>(
        std::count_if(
            sensores.begin(),
            sensores.end(),
            [](const Sensor& sensor){
                return !sensor.esValido();
            }
        )
    );
}




/*
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

*/