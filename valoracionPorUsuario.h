using namespace std;
#include "valoracion.h"
#include <string>

struct ValoracionPtrPorUsuario {
    string codigoUsuario;
    Valoracion* ptr;

    ValoracionPtrPorUsuario(string usuario, Valoracion* p)
        : codigoUsuario(usuario), ptr(p) {}

    ValoracionPtrPorUsuario() : codigoUsuario(""), ptr(nullptr) {}

    
    // Ordenamos por código de usuario
    bool operator<(const ValoracionPtrPorUsuario& other) const {
        return codigoUsuario < other.codigoUsuario;
    }
    bool operator>(const ValoracionPtrPorUsuario& other) const {
        return codigoUsuario > other.codigoUsuario;
    }
    bool operator==(const ValoracionPtrPorUsuario& other) const {
        return codigoUsuario == other.codigoUsuario;
    }
    bool operator<=(const ValoracionPtrPorUsuario& other) const {
        return codigoUsuario <= other.codigoUsuario;
    }
    bool operator>=(const ValoracionPtrPorUsuario& other) const {
        return codigoUsuario >= other.codigoUsuario;
    }

    friend ostream& operator<<(ostream& os, const ValoracionPtrPorUsuario& v) {
        os << v.codigoUsuario << "," << (v.ptr ? *v.ptr : Valoracion()) << endl;
        return os;
    }

};
