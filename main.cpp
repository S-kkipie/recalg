#include <iostream>
#include "BPlusTree.h"
#include "valoracion.h"
#include <fstream>

int main(){
    BPlusTree<Valoracion> tree(3); // Create a B+ tree with degree 3
    //Load data from data.csv
    std::ifstream file("data.csv");
    if (!file.is_open()) {
        std::cerr << "Error opening file." << std::endl;
        return 1;
    }
    std::string line;
    std::getline(file, line);

    while (std::getline(file, line)) {
        std::string usuario, cancion;
        int valor;
        size_t pos1 = line.find(',');
        size_t pos2 = line.find(',', pos1 + 1);
        
        if (pos1 != std::string::npos && pos2 != std::string::npos) {
            usuario = line.substr(0, pos1);
            cancion = line.substr(pos1 + 1, pos2 - pos1 - 1);
            valor = std::stoi(line.substr(pos2 + 1));
            Valoracion v(usuario, cancion, valor);
            tree.insert(v); // Insert the Valoracion into the B+ tree
        }
    }
    file.close();
    //Search for the 10 highest songs ratings
    
    Valoracion start("", "", 2);
    Valoracion end("", "", 5);
    
    Valoracion* results = new Valoracion[100];

    int count = tree.range_search(start, end, results, 100);
    for (int i = 0; i < count; i++) {
        std::cout << "Found: " << results[i] << std::endl;
    }
    delete[] results;

    


}