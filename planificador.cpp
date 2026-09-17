#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <queue>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <sstream>
#include <sys/wait.h>
#include <signal.h>
using namespace std;

// Tarea 1: Planificador Dieciochero

struct Planificador {
    string id_tarea;
    string nombre;
    int tiempo;
    vector<string> dependencias;
    vector<string> sucesoras;
    int dep_counter;
};

void leer_archivo(string archivo, vector<Planificador>& docho){
    ifstream plan(archivo);
    string linea;
    while(getline(plan, linea)){
        if(linea.empty()) continue;

        Planificador p;
        stringstream ss(linea);
        char puntos;
        
        ss >> p.id_tarea >> puntos >> p.nombre >> puntos >> p.tiempo >> puntos;

        string dependencia;
        while (ss >> dependencia) {
            if (dependencia.back() == ',') {
                dependencia.pop_back();
            }
            
            p.dependencias.push_back(dependencia);
        }
        
        p.dep_counter = p.dependencias.size();  

        cout << "-----------------------------" << endl;
        cout << "ID: " << p.id_tarea << endl;
        cout << "Nombre: " << p.nombre << endl;
        cout << "Tiempo: " << p.tiempo << endl;
        cout << "Dependencias: ";
        for(auto &dep : p.dependencias){
            cout << dep << " ";
        }
        cout << endl;
        cout << "Dependencias Counter: " << p.dep_counter << endl;
        cout << endl;

        docho.push_back(p);
    }

    for (auto& tarea : docho) {
            for (const string& id_dep : tarea.dependencias) {
                for (auto& padre : docho) {
                    if (padre.id_tarea == id_dep) {
                        padre.sucesoras.push_back(tarea.id_tarea);
                    }
                }
            }
        }
    
        for(auto& tarea : docho){
            cout << "Sucesoras de " << tarea.id_tarea << ": ";
            for(auto& sucesora : tarea.sucesoras){
                cout << sucesora << " ";
            }
            cout << endl;
        }

}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Uso: " << argv[0] << " <plan.txt> <K limite>" << endl;
        return 1;
    }

    string archivo_plan = argv[1];
    int K = stoi(argv[2]);

    cout << "Iniciando Planificador con " << archivo_plan 
        << " y concurrencia máxima K = " << K << endl;

    vector<Planificador> docho;
    leer_archivo(archivo_plan, docho);

    queue<Planificador*> listos;
    for(auto& tarea : docho){
        if(tarea.dep_counter == 0){
            listos.push(&tarea);
        }
    }

    //Variables de control (No superar k procesos, tareas completadas (saber cuando terminar bucle),
    //saber tareas a desbloquear y quitar dependencias
    
    int procesos_activos = 0;
    int tareas_completadas = 0;
    int total_tareas = docho.size();

    unordered_map<pid_t, string> pid_to_tarea;

    unordered_map<string,Planificador*> mapa_tareas;
    
    for (auto& tarea : docho){
        mapa_tareas[tarea.id_tarea] = &tarea;
    }

    while (tareas_completadas < total_tareas){
        
    }
    

    return 0;
}
