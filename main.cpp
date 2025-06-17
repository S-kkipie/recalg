#include <iostream>
#include "BPlusTree.h"
#include "valoracion.h"
#include "valoracionPorUsuario.h"
#include "valoracionPorCancion.h"
#include <fstream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cmath>
#include <map>

using namespace std;

void topNSongs(int n, BPlusTree<Valoracion> &tree, Valoracion *results, float minValue = 0.0f, float maxValue = 5.0f);
void topPUsersNearKUser(string kUser, int p, BPlusTree<ValoracionPtrPorUsuario> &treePorUsuario, BPlusTree<ValoracionPtrPorCancion> &treePorCancion, string *resultUsers = nullptr);
void topNSongsWithoutCustomVal(int n, BPlusTree<Valoracion> &tree, Valoracion *resultSongs, float minValue, float maxValue);
void recommendNSongsToKUser(int n, string kUser, BPlusTree<ValoracionPtrPorUsuario> &treePorUsuario, BPlusTree<ValoracionPtrPorCancion> &treePorCancion);

int mainMenu()
{
    int opcion;
    cout << "\n=== MENÚ PRINCIPAL ===" << endl;
    cout << "1. Mostrar Top N canciones globales" << endl;
    cout << "2. Mostrar Top N canciones de un usuario" << endl;
    cout << "3. Encontrar usuarios similares a un usuario (Top P vecinos)" << endl;
    cout << "4. Recomendar N canciones a un usuario" << endl;
    cout << "5. Salir" << endl;
    cout << "Seleccione una opción: ";
    cin >> opcion;
    return opcion;
}

int main()
{
    BPlusTree<Valoracion> tree(50);
    BPlusTree<ValoracionPtrPorUsuario> treePorUsuario(50);
    BPlusTree<ValoracionPtrPorCancion> treePorCancion(50);

    string n;
    cout << "Ingrese el nombre del archivo: ";
    cin >> n;
    ifstream file(n);
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
            tree.insert(v);
        }
    }
    file.close();

    tree.for_each([&treePorUsuario, &treePorCancion](Valoracion &v)
                  {
        ValoracionPtrPorUsuario valoracionPorUsuario(v.codigoUsuario, &v);
        ValoracionPtrPorCancion valoracionPorCancion(v.codigoCancion, &v);
        treePorUsuario.insert(valoracionPorUsuario);
        treePorCancion.insert(valoracionPorCancion); });
    int opcion;
    do
    {
        opcion = mainMenu();
        switch (opcion)
        {
        case 1:
        {
            int n;
            cout << "Ingrese el número de canciones a mostrar (Top N): ";
            cin >> n;
            Valoracion *resultSongs = new Valoracion[n];
            topNSongs(n, tree, resultSongs, 4.5f, 5.0f);
            cout << "Top " << n << " canciones globales:" << endl;
            for (int i = 0; i < n; ++i)
            {
                if (!resultSongs[i].codigoCancion.empty())
                    cout << "Canción: " << resultSongs[i].codigoCancion << ", Valor: " << resultSongs[i].valor << endl;
            }
            delete[] resultSongs;
            break;
        }
        case 2:
        {
            string usuario;
            int n;
            cout << "Ingrese el código del usuario: ";
            cin >> usuario;
            cout << "Ingrese el número de canciones a mostrar (Top N): ";
            cin >> n;

            ValoracionPtrPorUsuario start(usuario, nullptr);
            ValoracionPtrPorUsuario end(usuario, nullptr);
            ValoracionPtrPorUsuario *results = new ValoracionPtrPorUsuario[100000];
            int count = treePorUsuario.range_search(start, end, results, 100000);

            BPlusTree<Valoracion> songsUsuario(100);
            for (int i = 0; i < count; i++)
            {
                songsUsuario.insert(*results[i].ptr);
            }
            delete[] results;

            Valoracion *resultSongs = new Valoracion[n];
            topNSongsWithoutCustomVal(n, songsUsuario, resultSongs, 0.0f, 5.0f);
            cout << "Top " << n << " canciones del usuario " << usuario << ":" << endl;
            for (int i = 0; i < n; ++i)
            {
                if (!resultSongs[i].codigoCancion.empty())
                    cout << "Canción: " << resultSongs[i].codigoCancion << ", Valor: " << resultSongs[i].valor << endl;
            }
            delete[] resultSongs;
            break;
        }
        case 3:
        {
            string kUser;
            int p;
            cout << "Ingrese el código del usuario de referencia (kUser): ";
            cin >> kUser;
            cout << "Ingrese el número de usuarios similares a mostrar (Top P): ";
            cin >> p;

            cout << "Las " << p << " valoraciones mas cercanas al usuario " << kUser << ":" << endl;
            string *nearestUsers = new string[p];
            topPUsersNearKUser(kUser, p, treePorUsuario, treePorCancion, nearestUsers);
            for (int i = 0; i < p; ++i)
            {
                cout << nearestUsers[i] << endl;
            }
            break;
        }
        case 4:
        {
            string usuario;
            int n;
            cout << "Ingrese el código del usuario: ";
            cin >> usuario;
            cout << "¿Cuántas canciones recomendar? ";
            cin >> n;
            recommendNSongsToKUser(n, usuario, treePorUsuario, treePorCancion);
            break;
        }
        case 5:
            cout << "Saliendo del programa." << endl;
            break;
        default:
            cout << "Opción inválida." << endl;
            break;
        }
    } while (opcion != 5);
}

void topPUsersNearKUser(string kUser, int p, BPlusTree<ValoracionPtrPorUsuario> &treePorUsuario, BPlusTree<ValoracionPtrPorCancion> &treePorCancion, string *resultUsers)
{
    ValoracionPtrPorUsuario start(kUser, nullptr);
    ValoracionPtrPorUsuario end(kUser, nullptr);

    ValoracionPtrPorUsuario *results = new ValoracionPtrPorUsuario[100000];
    int count = treePorUsuario.range_search(start, end, results, 100000);

    BPlusTree<Valoracion> songs(3);

    for (int i = 0; i < count; i++)
    {
        ValoracionPtrPorUsuario current = results[i];
        Valoracion *v = current.ptr;
        songs.insert(*v);
    }

    delete[] results;

    int numSongs = 2;

    Valoracion *resultsSongs = new Valoracion[numSongs];
    topNSongsWithoutCustomVal(numSongs, songs, resultsSongs, 3.0f, 5.0f);

    // cout << "Top " << numSongs << " canciones con sus valores totales:" << endl;

    // for (int i = 0; i < numSongs; ++i)
    // {
    //     if (resultsSongs[i].codigoCancion != "")
    //     {
    //         if (resultsSongs[i].codigoCancion != "")
    //         {
    //             cout << "Canción: " << resultsSongs[i].codigoCancion << ", Valoración: " << resultsSongs[i].valor << endl;
    //         }
    //     }
    // }

    unordered_map<string, pair<int, int>> valoracionesPorCancion;

    for (int i = 0; i < numSongs; i++)
    {
        ValoracionPtrPorCancion startCancion(resultsSongs[i].codigoCancion, nullptr);
        ValoracionPtrPorCancion endCancion(resultsSongs[i].codigoCancion, nullptr);
        ValoracionPtrPorCancion *resultsCancion = new ValoracionPtrPorCancion[100000];
        int countCancion = treePorCancion.range_search(startCancion, endCancion, resultsCancion, 100000);

        for (int j = 0; j < countCancion; j++)
        {
            Valoracion *current = resultsCancion[j].ptr;
            if (current != nullptr)
            {
                if (i == 1)
                {
                    valoracionesPorCancion[current->codigoUsuario].first = static_cast<int>(current->valor);
                }
                else
                {
                    valoracionesPorCancion[current->codigoUsuario].second = static_cast<int>(current->valor);
                }
            }
        }
    }

    vector<pair<string, double>> nearestUsers;

    for (const auto &par : valoracionesPorCancion)
    {
        const string &usuario = par.first;
        int valor1OtherUser = par.second.first;
        int valor2OtherUser = par.second.second;

        int valor1KUser = static_cast<int>(resultsSongs[0].valor);
        int valor2KUser = static_cast<int>(resultsSongs[1].valor);

        int valor1Distancia = abs(valor1OtherUser - valor1KUser);
        int valor2Distancia = abs(valor2OtherUser - valor2KUser);

        double distanciaEuclidiana = sqrt(
            static_cast<double>(valor1Distancia * valor1Distancia +
                                valor2Distancia * valor2Distancia));

        // cout << "Usuario: " << usuario << endl;
        // cout << "  Valor 1 " << kUser << ":" << valor1KUser << ", " << usuario << ": " << valor1OtherUser
        //      << " -> Distancia: " << valor1Distancia << endl;
        // cout << "  Valor 2 " << kUser << ":" << valor2KUser << ", " << usuario << ": " << valor2OtherUser
        //      << " -> Distancia: " << valor2Distancia << endl;
        // cout << "  Distancia Euclidiana: " << distanciaEuclidiana << endl;

        nearestUsers.emplace_back(usuario, distanciaEuclidiana);
    }

    sort(nearestUsers.begin(), nearestUsers.end(), [](const auto &a, const auto &b)
         { return a.second < b.second; });

    for (int i = 0; i < p && i < static_cast<int>(nearestUsers.size()); ++i)
    {
        resultUsers[i] = nearestUsers[i].first;
    }
}

void topNSongs(int n, BPlusTree<Valoracion> &tree, Valoracion *resultSongs, float minValue, float maxValue)
{

    Valoracion start("", "", minValue);
    Valoracion end("", "", maxValue);

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

        valoraciones[current.codigoCancion] += value;
    }
    vector<pair<string, float>> ordenadas(valoraciones.begin(), valoraciones.end());

    sort(ordenadas.begin(), ordenadas.end(),
         [](const pair<string, float> &a, const pair<string, float> &b)
         {
             return a.second > b.second;
         });

    int print_count = min(n, static_cast<int>(ordenadas.size()));
    for (int i = 0; i < print_count; i++)
    {
        resultSongs[i] = Valoracion("", ordenadas[i].first, ordenadas[i].second);
    }
    delete[] results;
}

void topNSongsWithoutCustomVal(int n, BPlusTree<Valoracion> &tree, Valoracion *resultSongs, float minValue, float maxValue)
{

    Valoracion start("", "", minValue);
    Valoracion end("", "", maxValue);

    Valoracion *results = new Valoracion[100000];

    int count = tree.range_search(start, end, results, 100000);

    unordered_map<string, float> valoraciones;

    for (int i = 0; i < count; i++)
    {
        Valoracion current = results[i];
        valoraciones[current.codigoCancion] += current.valor;
    }
    vector<pair<string, float>> ordenadas(valoraciones.begin(), valoraciones.end());

    sort(ordenadas.begin(), ordenadas.end(),
         [](const pair<string, float> &a, const pair<string, float> &b)
         {
             return a.second > b.second;
         });

    int print_count = min(n, static_cast<int>(ordenadas.size()));
    for (int i = 0; i < print_count; i++)
    {
        resultSongs[i] = Valoracion("", ordenadas[i].first, ordenadas[i].second);
    }
    delete[] results;
}

void recommendNSongsToKUser(int n, string kUser, BPlusTree<ValoracionPtrPorUsuario> &treePorUsuario, BPlusTree<ValoracionPtrPorCancion> &treePorCancion)
{
    string *nearestUsers = new string[50];
    topPUsersNearKUser(kUser, 50, treePorUsuario, treePorCancion, nearestUsers);

    int totalCount = 0;
    Valoracion *resultSongs = new Valoracion[n];

    for (int i = 0; i < 50 && totalCount < n; i++)
    {
        if (nearestUsers[i].empty())
            continue;

        ValoracionPtrPorUsuario start(nearestUsers[i], nullptr);
        ValoracionPtrPorUsuario end(nearestUsers[i], nullptr);
        ValoracionPtrPorUsuario *results = new ValoracionPtrPorUsuario[100000];
        int userCount = treePorUsuario.range_search(start, end, results, 100000);

        BPlusTree<Valoracion> songsUsuario(100);
        for (int j = 0; j < userCount; j++)
        {
            songsUsuario.insert(*results[j].ptr);
        }
        delete[] results;

        Valoracion *resultTopSongs = new Valoracion[n];
        topNSongsWithoutCustomVal(n, songsUsuario, resultTopSongs, 5.0f, 5.0f);

        for (int j = 0; j < n && totalCount < n; ++j)
        {
            if (!resultTopSongs[j].codigoCancion.empty())
            {
                resultSongs[totalCount++] = resultTopSongs[j];
            }
        }

        delete[] resultTopSongs;
    }

    cout << n << " canciones recomendadas para el usuario " << kUser << ":" << endl;
    for (int i = 0; i < totalCount; i++)
    {
        cout << "Canción: " << resultSongs[i].codigoCancion << ", Valor: " << resultSongs[i].valor << endl;
    }

    delete[] nearestUsers;
    delete[] resultSongs;
}
