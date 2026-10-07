#pragma once

#include <string>

class Sensor {

    private:

        std::string nombre;
        std::string unidad;
        double valor;
        double minimo;
        double maximo;

    public:

        Sensor(const std::string& nombre, 
               const std::string& unidad, 
               double minimo, 
               double maximo);

        void actualizar(double nuevoValor);

        double obtenerValor() const;

        double obtenerMinimo() const;
         
        double obtenerMaximo() const;

        std::string obtenerNombre() const;

        std::string obtenerUnidad() const;

        bool esValido() const;

};