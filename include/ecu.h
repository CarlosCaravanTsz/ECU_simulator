#pragma once

#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "sensor.h"

enum class EstadoECU
{
    INIT,
    OPERATIONAL,
    DEGRADED,
    SAFE_STATE
};


enum class EstadoDtc
{
    INACTIVO,       // nunca ha fallado (o se borro con CLEAR CODES)
    PENDIENTE,      // fallando, pero aun no llega al limite de debounce
    CONFIRMADO,     // llego al umbral: la falla es real
    HISTORICO       // estuvo confirmado y ya se recupero (stale)
};

enum class LimiteDtc
{
    BAJO,           
    ALTO           
};

struct LecturaSensor
{
    std::string sensor;
    double valor;
};

struct Dtc
{
    std::string codigo;             // "P0217"
    std::string descripcion;
    std::string sensor;             // nombre del sensor que vigila
    LimiteDtc limite;
    bool critico;                   // true -> SAFE_STATE

    // Estado que la ECU va actualizando
    int contador = 0;               // debounce
    EstadoDtc estado = EstadoDtc::INACTIVO;
    int ocurrencias = 0;            // veces que se ha confirmado

    // Freeze frame de la primera confirmacion
    std::string timestamp = "";
    long ciclo = 0;
    std::vector<LecturaSensor> freezeFrame = {};
};


class ECU{

    private:

        std::vector<Sensor> sensores;

        EstadoECU estado;

        // Memoria de diagnostico
        std::vector<Dtc> dtcs;
        long ciclo;

        // Protege dtcs y ciclo: los usan el hilo principal y el monitor
        mutable std::mutex mutexDtc;

        // Hilo que atiende los comandos del escaner
        std::atomic<bool> monitorActivo;
        std::thread hiloMonitor;        // siempre al final: arranca cuando todo lo demas existe

        bool actualizarEstadoDtc(Dtc& dtc);
        void guardarJson() const;
        void borrarDtcs();
        std::string generarReporte(bool incluirHistoricos) const;
        void monitorComandos();
        void ejecutarComando(const std::string& comando);

    public:

        ECU();

        ~ECU();

        void actualizar(double velocidad,
                        double rpm,
                        double temperatura,
                        double bateria,
                        double presionAceite);

        void diagnosticar();

        EstadoECU obtenerEstado() const;

        const std::vector<Sensor>& obtenerSensores() const;

        const Sensor* buscarSensor(
            const std::string& nombre
        ) const;

        int contarSensoresFallas() const;

        std::vector<Dtc> obtenerDtcs() const;

};
