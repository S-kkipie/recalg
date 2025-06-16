#ifndef VALORACION_H
#define VALORACION_H
#include <string>
#include <iostream>

using namespace std;

class Valoracion {
public:
    string codigoUsuario;
    string codigoCancion;
    float valor;

    Valoracion();
    Valoracion(const string& usuario, const string& cancion, float v);

    bool operator<(const Valoracion& other) const;
    bool operator<=(const Valoracion& other) const;
    bool operator>=(const Valoracion& other) const;
    bool operator>(const Valoracion& other) const;
    bool operator==(const Valoracion& other) const;

    friend ostream& operator<<(ostream& os, const Valoracion& v);
};

#endif // VALORACION_H;