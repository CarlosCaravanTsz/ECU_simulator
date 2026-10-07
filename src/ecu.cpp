#include "ecu.h"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace
{
    const int UMBRAL_DEBOUNCE = 3;

    // Archivos relativos a la carpeta desde donde se ejecuta el programa
    const char* ARCHIVO_JSON = "dtc_freeze_frames.json";
    const char* ARCHIVO_COMANDO = "dtc_comando.txt";
    const char* ARCHIVO_REPORTE = "dtc_reporte.txt";

    const std::chrono::milliseconds PERIODO_MONITOR(100);


    // Fecha y hora local en formato ISO 8601: 2026-10-06T16:40:12
    std::string ahoraIso()
    {
        // std::localtime devuelve un puntero a un buffer compartido:
        // dos hilos llamandola a la vez se pisarian el resultado.
        static std::mutex mutexHora;
        std::lock_guard<std::mutex> candado(mutexHora);

        std::time_t ahora = std::chrono::system_clock::to_time_t(
            std::chrono::system_clock::now()
        );

        std::ostringstream texto;
        texto << std::put_time(std::localtime(&ahora), "%Y-%m-%dT%H:%M:%S");
        return texto.str();
    }

    std::string estadoDtcATexto(EstadoDtc estado)
    {
        switch(estado)
        {
            case EstadoDtc::INACTIVO:
                return "INACTIVO";

            case EstadoDtc::PENDIENTE:
                return "PENDIENTE";

            case EstadoDtc::CONFIRMADO:
                return "CONFIRMADO";

            case EstadoDtc::HISTORICO:
                return "HISTORICO";
        }

        return "DESCONOCIDO";
    }

    // Quita espacios y '\r' de los extremos y pasa a mayusculas
    std::string normalizarComando(std::string texto)
    {
        auto esEspacio = [](unsigned char c){ return std::isspace(c) != 0; };

        texto.erase(texto.begin(), std::find_if_not(texto.begin(), texto.end(), esEspacio));
        texto.erase(std::find_if_not(texto.rbegin(), texto.rend(), esEspacio).base(), texto.end());

        std::transform(texto.begin(), texto.end(), texto.begin(),
            [](unsigned char c){ return static_cast<char>(std::toupper(c)); });

        return texto;
    }
}


ECU::ECU()

:

//velocidad("Velocidad", " km/h", 0.0, 220.0),
//rpm("RPM", " rpm", 600.0, 7000.0),
//temperatura("Temperatura", " C", -20.0, 110.0),
//bateria("Bateria", " V", 11.5, 15.0),
//presionAceite("Presion aceite", " psi", 0.8, 6.0),

estado(EstadoECU::INIT),
ciclo(0),
monitorActivo(true)
{
    sensores.emplace_back("Velocidad", " km/h", 0.0, 100.0);

    sensores.emplace_back("RPM", " rpm", 600.0, 4000.0);

    sensores.emplace_back("Temperatura", " C", -20.0, 90.0);

    sensores.emplace_back("Bateria", " V", 11.5, 15.0);

    sensores.emplace_back("Presion aceite", " psi", 0.8, 4.0);

    // Codigos OBD-II: un codigo por cada lado del rango de cada sensor
    dtcs.push_back({"P0501", "Velocidad fuera de rango (alta)",       "Velocidad",      LimiteDtc::ALTO, false});
    dtcs.push_back({"P0506", "RPM de ralenti mas bajas de lo esperado", "RPM",          LimiteDtc::BAJO, false});
    dtcs.push_back({"P0219", "Sobrevelocidad del motor (RPM altas)",  "RPM",            LimiteDtc::ALTO, false});
    dtcs.push_back({"P0117", "Sensor de temperatura: senal baja",     "Temperatura",    LimiteDtc::BAJO, true });
    dtcs.push_back({"P0217", "Sobretemperatura del motor",            "Temperatura",    LimiteDtc::ALTO, true });
    dtcs.push_back({"P0562", "Voltaje del sistema bajo",              "Bateria",        LimiteDtc::BAJO, false});
    dtcs.push_back({"P0563", "Voltaje del sistema alto",              "Bateria",        LimiteDtc::ALTO, false});
    dtcs.push_back({"P0524", "Presion de aceite demasiado baja",      "Presion aceite", LimiteDtc::BAJO, true });
    dtcs.push_back({"P0523", "Sensor de presion de aceite: senal alta", "Presion aceite", LimiteDtc::ALTO, true });

    // El hilo arranca al final, cuando la ECU ya esta completa
    hiloMonitor = std::thread(&ECU::monitorComandos, this);
}


ECU::~ECU()
{
    monitorActivo = false;

    if(hiloMonitor.joinable())
    {
        hiloMonitor.join();
    }
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
    std::lock_guard<std::mutex> candado(mutexDtc);

    ciclo++;

    bool hayCambios = false;

    for(Dtc& dtc : dtcs)
    {
        const Sensor* sensor = buscarSensor(dtc.sensor);

        if(sensor == nullptr)
        {
            continue;
        }

        bool fueraDeRango = (dtc.limite == LimiteDtc::BAJO)
            ? sensor->obtenerValor() < sensor->obtenerMinimo()
            : sensor->obtenerValor() > sensor->obtenerMaximo();

        // Debounce: una lectura mala suma, una buena resta
        if(fueraDeRango)
        {
            dtc.contador = std::min(dtc.contador + 1, UMBRAL_DEBOUNCE);
        }
        else
        {
            dtc.contador = std::max(dtc.contador - 1, 0);
        }

        // Solo se reescribe el JSON si cambia un codigo que aparece en el
        // (los que nunca se han confirmado no estan en el archivo)
        if(actualizarEstadoDtc(dtc) && dtc.ocurrencias > 0)
        {
            hayCambios = true;
        }
    }

    if(hayCambios)
    {
        guardarJson();
    }

    // El estado de la ECU sale de los codigos CONFIRMADOS, no de una sola lectura
    bool fallaCritica = std::any_of(dtcs.begin(), dtcs.end(),
        [](const Dtc& dtc){ return dtc.estado == EstadoDtc::CONFIRMADO && dtc.critico; });

    bool fallaNoCritica = std::any_of(dtcs.begin(), dtcs.end(),
        [](const Dtc& dtc){ return dtc.estado == EstadoDtc::CONFIRMADO && !dtc.critico; });

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

}

// Aplica la maquina de estados de un DTC segun su contador.
// Devuelve true si el estado cambio. Requiere mutexDtc tomado.
bool ECU::actualizarEstadoDtc(Dtc& dtc)
{
    EstadoDtc anterior = dtc.estado;

    if(dtc.contador >= UMBRAL_DEBOUNCE)
    {
        if(dtc.estado != EstadoDtc::CONFIRMADO)
        {
            dtc.estado = EstadoDtc::CONFIRMADO;
            dtc.ocurrencias++;

            // Freeze frame solo en la primera confirmacion (como OBD-II)
            if(dtc.ocurrencias == 1)
            {
                dtc.timestamp = ahoraIso();
                dtc.ciclo = ciclo;
                dtc.freezeFrame.clear();

                for(const Sensor& sensor : sensores)
                {
                    dtc.freezeFrame.push_back({sensor.obtenerNombre(), sensor.obtenerValor()});
                }
            }
        }
    }

    else if(dtc.estado == EstadoDtc::CONFIRMADO)
    {
        // Para "sanar" tambien se exige debounce: el contador debe bajar a 0
        if(dtc.contador == 0)
        {
            dtc.estado = EstadoDtc::HISTORICO;
        }
    }

    else if(dtc.contador > 0)
    {
        dtc.estado = EstadoDtc::PENDIENTE;
    }

    else
    {
        dtc.estado = (dtc.ocurrencias > 0) ? EstadoDtc::HISTORICO : EstadoDtc::INACTIVO;
    }

    return dtc.estado != anterior;
}

// Escribe los DTC que alguna vez se confirmaron. Requiere mutexDtc tomado.
void ECU::guardarJson() const
{
    // Se escribe a un archivo temporal y luego se renombra:
    // si el programa muere a mitad (Ctrl+C), el JSON anterior queda intacto.
    const std::string temporal = std::string(ARCHIVO_JSON) + ".tmp";

    {
        std::ofstream archivo(temporal);

        if(!archivo)
        {
            return;
        }

        archivo << std::fixed << std::setprecision(2);
        archivo << "{\n  \"dtcs\": [";

        bool primero = true;

        for(const Dtc& dtc : dtcs)
        {
            if(dtc.ocurrencias == 0)
            {
                continue;
            }

            archivo << (primero ? "\n" : ",\n");
            primero = false;

            archivo << "    {\n";
            archivo << "      \"codigo\": \"" << dtc.codigo << "\",\n";
            archivo << "      \"descripcion\": \"" << dtc.descripcion << "\",\n";
            archivo << "      \"estado\": \"" << estadoDtcATexto(dtc.estado) << "\",\n";
            archivo << "      \"critico\": " << (dtc.critico ? "true" : "false") << ",\n";
            archivo << "      \"ocurrencias\": " << dtc.ocurrencias << ",\n";
            archivo << "      \"freeze_frame\": {\n";
            archivo << "        \"timestamp\": \"" << dtc.timestamp << "\",\n";
            archivo << "        \"ciclo\": " << dtc.ciclo << ",\n";
            archivo << "        \"sensores\": {";

            for(std::size_t i = 0; i < dtc.freezeFrame.size(); i++)
            {
                archivo << (i == 0 ? " " : ", ")
                        << "\"" << dtc.freezeFrame[i].sensor << "\": " << dtc.freezeFrame[i].valor;
            }

            archivo << " }\n";
            archivo << "      }\n";
            archivo << "    }";
        }

        archivo << "\n  ]\n}\n";
    }   // aqui se destruye el ofstream y se cierra el archivo

    std::error_code error;
    std::filesystem::rename(temporal, ARCHIVO_JSON, error);
}

void ECU::borrarDtcs()
{
    std::lock_guard<std::mutex> candado(mutexDtc);

    for(Dtc& dtc : dtcs)
    {
        dtc.contador = 0;
        dtc.estado = EstadoDtc::INACTIVO;
        dtc.ocurrencias = 0;
        dtc.timestamp.clear();
        dtc.ciclo = 0;
        dtc.freezeFrame.clear();
    }

    std::error_code error;
    std::filesystem::remove(ARCHIVO_JSON, error);
}

std::string ECU::generarReporte(bool incluirHistoricos) const
{
    std::lock_guard<std::mutex> candado(mutexDtc);

    std::ostringstream reporte;
    reporte << std::fixed << std::setprecision(2);

    reporte << (incluirHistoricos ? "READ ALL CODES" : "READ CURRENT CODES")
            << "  |  " << ahoraIso() << "  |  ciclo " << ciclo << "\n";
    reporte << "---------------------------------------------\n";

    int mostrados = 0;

    for(const Dtc& dtc : dtcs)
    {
        bool mostrar = incluirHistoricos
            ? dtc.estado != EstadoDtc::INACTIVO
            : dtc.estado == EstadoDtc::CONFIRMADO;

        if(!mostrar)
        {
            continue;
        }

        mostrados++;

        reporte << dtc.codigo << "  " << std::left << std::setw(11) << estadoDtcATexto(dtc.estado)
                << (dtc.critico ? "CRITICO  " : "         ") << dtc.descripcion << "\n";

        if(dtc.ocurrencias > 0)
        {
            reporte << "       ocurrencias: " << dtc.ocurrencias
                    << "  |  freeze frame: " << dtc.timestamp << " (ciclo " << dtc.ciclo << ")\n";

            for(const LecturaSensor& lectura : dtc.freezeFrame)
            {
                reporte << "         " << std::left << std::setw(16) << lectura.sensor
                        << std::right << std::setw(8) << lectura.valor << "\n";
            }
        }
    }

    if(mostrados == 0)
    {
        reporte << "Sin codigos.\n";
    }

    return reporte.str();
}

// Hilo paralelo: revisa el archivo de comandos cada PERIODO_MONITOR
void ECU::monitorComandos()
{
    while(monitorActivo)
    {
        std::string linea;

        {
            std::ifstream entrada(ARCHIVO_COMANDO);
            std::getline(entrada, linea);
        }

        std::string comando = normalizarComando(linea);

        if(!comando.empty())
        {
            // Vaciar el archivo para no ejecutar el mismo comando dos veces
            std::ofstream(ARCHIVO_COMANDO, std::ios::trunc);

            ejecutarComando(comando);
        }

        std::this_thread::sleep_for(PERIODO_MONITOR);
    }
}

void ECU::ejecutarComando(const std::string& comando)
{
    std::string respuesta;

    if(comando == "READ CURRENT CODES")
    {
        respuesta = generarReporte(false);
    }

    else if(comando == "READ ALL CODES")
    {
        respuesta = generarReporte(true);
    }

    else if(comando == "CLEAR CODES")
    {
        borrarDtcs();
        respuesta = "CLEAR CODES  |  " + ahoraIso() + "\nCodigos y freeze frames borrados.\n";
    }

    else
    {
        respuesta = "Comando desconocido: " + comando +
                    "\nUsa: READ CURRENT CODES | READ ALL CODES | CLEAR CODES\n";
    }

    std::ofstream salida(ARCHIVO_REPORTE, std::ios::trunc);
    salida << respuesta;
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

std::vector<Dtc> ECU::obtenerDtcs() const
{
    std::lock_guard<std::mutex> candado(mutexDtc);

    return dtcs;    // copia: quien la reciba no comparte nada con el hilo monitor
}

