
#include "dtc.h"

GestorDtc::GestorDtc()
{
    dtcs.push_back({"P0500", "Velocidad",      false});
    dtcs.push_back({"P0219", "RPM",            false});
    dtcs.push_back({"P0217", "Temperatura",    true });
    dtcs.push_back({"P0562", "Bateria",        false});
    dtcs.push_back({"P0520", "Presion aceite", true });
}
