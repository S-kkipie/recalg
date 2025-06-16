using namespace std;
#include "valoracion.h"
#include <string>

struct ValoracionPtrPorCancion {
    string codigoCancion;
    Valoracion* ptr;

    ValoracionPtrPorCancion(string cancion, Valoracion* p)
        : codigoCancion(cancion), ptr(p) {}

    ValoracionPtrPorCancion() : codigoCancion(""), ptr(nullptr) {}

    // Ordenamos por código de canción
    bool operator<(const ValoracionPtrPorCancion& other) const {
        return codigoCancion < other.codigoCancion;
    }
    bool operator>(const ValoracionPtrPorCancion& other) const {
        return codigoCancion > other.codigoCancion;
    }
    bool operator==(const ValoracionPtrPorCancion& other) const {
        return codigoCancion == other.codigoCancion;
    }
    bool operator<=(const ValoracionPtrPorCancion& other) const {
        return codigoCancion <= other.codigoCancion;
    }
    bool operator>=(const ValoracionPtrPorCancion& other) const {
        return codigoCancion >= other.codigoCancion;
    }

    friend ostream& operator<<(ostream& os, const ValoracionPtrPorCancion& v) {
        os << v.codigoCancion << "," << (v.ptr ? *v.ptr : Valoracion())<< endl;
        return os;
    }

};
