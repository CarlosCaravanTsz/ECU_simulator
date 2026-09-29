#include "sensor.h"


Sensor::Sensor(
       const std::string& nombre, 
       const std::string& unidad, 
       double minimo, 
       double maximo) 
       
       : 
       
       nombre(nombre), 
       unidad(unidad), 
       valor(0.0), 
       minimo(minimo), 
       maximo(maximo)
{           
}


void Sensor::actualizar(double nuevoValor){

    valor = nuevoValor;

}

double Sensor::obtenerValor() const {

    return valor;
    
}

double Sensor::obtenerMinimo() const{
    return minimo;
}

double Sensor::obtenerMaximo() const{
    return maximo;
}

std::string Sensor::obtenerNombre() const {

    return nombre;

}

std::string Sensor::obtenerUnidad() const {

    return unidad;

}

bool Sensor::esValido() const{

    return valor >= minimo && valor <= maximo;

}