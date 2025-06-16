#include "valoracion.h"

Valoracion::Valoracion() : codigoUsuario(""), codigoCancion(""), valor(0) {}

Valoracion::Valoracion(const std::string& usuario, const std::string& cancion, float v)
    : codigoUsuario(usuario), codigoCancion(cancion), valor(v) {}

bool Valoracion::operator<(const Valoracion& other) const {
    if (valor != other.valor)
        return valor < other.valor;
    if (codigoUsuario != other.codigoUsuario)
        return codigoUsuario < other.codigoUsuario;
    return codigoCancion < other.codigoCancion;
}

bool Valoracion::operator>(const Valoracion& other) const {
    if (valor != other.valor)
        return valor > other.valor;
    if (codigoUsuario != other.codigoUsuario)
        return codigoUsuario > other.codigoUsuario;
    return codigoCancion > other.codigoCancion;
}
bool Valoracion::operator<=(const Valoracion& other) const {
    return *this < other || valor == other.valor;
}
bool Valoracion::operator>=(const Valoracion& other) const {
    return *this > other || valor == other.valor;
}

bool Valoracion::operator==(const Valoracion& other) const {
    return valor == other.valor &&
           codigoUsuario == other.codigoUsuario &&
           codigoCancion == other.codigoCancion;
}

std::ostream& operator<<(std::ostream& os, const Valoracion& v) {
    os << v.codigoUsuario << "," << v.codigoCancion << "," << v.valor;
    return os;
}