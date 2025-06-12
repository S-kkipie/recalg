#include <iostream>
#include <fstream>
#include <sstream>
#include "btree.h"

int main() {
    std::ifstream archivo("data.csv");
    BTree arbol;
    std::string linea;

    std::getline(archivo, linea);

    while (std::getline(archivo, linea)) {
        std::stringstream ss(linea);
        std::string usuario, cancion, valoracionStr;
        std::getline(ss, usuario, ',');
        std::getline(ss, cancion, ',');
        std::getline(ss, valoracionStr, ',');
        arbol.insert(std::stoi(valoracionStr));
    }

    std::cout << "Valoraciones en el árbol B:";
    arbol.traverse();
    std::cout << std::endl;

    return 0;
}