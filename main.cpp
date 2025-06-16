#include <iostream>
#include "BPlusTree.h"
#include "valoracion.h"
#include <fstream>
#include <unordered_map>
#include <vector>
#include <algorithm>

using namespace std;

int main()
{
    BPlusTree<Valoracion> tree(3); // Create a B+ tree with degree 3
    // Load data from data.csv
    ifstream file("data2.csv");
    if (!file.is_open())
    {
        cerr << "Error opening file." << endl;
        return 1;
    }
    string line;
    getline(file, line);

    while (getline(file, line))
    {
        string usuario, cancion;
        float valor;
        size_t pos1 = line.find(',');
        size_t pos2 = line.find(',', pos1 + 1);

        if (pos1 != string::npos && pos2 != string::npos)
        {
            usuario = line.substr(0, pos1);
            cancion = line.substr(pos1 + 1, pos2 - pos1 - 1);
            valor = stof(line.substr(pos2 + 1));
            Valoracion v(usuario, cancion, valor);
            tree.insert(v); // Insert the Valoracion into the B+ tree
        }
    }
    file.close();
    // Search for the 10 highest songs ratings

    Valoracion start("", "", 5.0f);
    Valoracion end("", "", 5.0f);

    Valoracion *results = new Valoracion[100000];

    int count = tree.range_search(start, end, results, 100000);

    unordered_map<string, float> valoraciones;

    for (int i = 0; i < count; i++)
    {
        Valoracion current = results[i];
        float value;

        if (current.valor == 5.0f)
        {
            value = 30.0f;
        }
        else if (current.valor > 2.5f && current.valor < 5.0f)
        {
            value = 10.0f * (current.valor - 2.5f);
        }
        else if (current.valor > 0.0f && current.valor <= 2.5f)
        {
            value = -10.0f * (2.5f - current.valor);
        }
        else
        {
            value = -30.0f;
        }

        // If the song does not exist, add it to the hashmap
        valoraciones[current.codigoCancion] += value;

    }
    vector<pair<string, float>> ordenadas(valoraciones.begin(), valoraciones.end());

    sort(ordenadas.begin(), ordenadas.end(),
        [](const pair<string, float>& a, const pair<string, float>& b) {
            return a.second > b.second; // Orden descendente por valor
        });

    // Print the top 10 songs with their total values
    cout << "Top 10 canciones con sus valores totales:" << endl;
    int print_count = min(10, static_cast<int>(ordenadas.size()));
    for (int i = 0; i < print_count; i++)
    {
        cout << ordenadas[i].first << ": " << ordenadas[i].second << endl;
    }    
    delete[] results; // Clean up dynamically allocated memory
}