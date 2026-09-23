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
    string insumo;
    bool cancelada = false;
};

void abortar_rama(Planificador* tarea, unordered_map<string, Planificador*>& mapa_tareas, int& tareas_canceladas){
    for(auto& sucesora : tarea->sucesoras){
        Planificador* suc = mapa_tareas[sucesora];
        if(!suc->cancelada){
            suc->cancelada = true;
            tareas_canceladas++;
            cout<< "[Alerta] Tarea Cancelada: " << suc->nombre << " (ID: " << suc->id_tarea << ")" << endl;
            abortar_rama(suc, mapa_tareas, tareas_canceladas);
        }
    }
}
    

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
        cerr << "Uso: " << argv[0] << " <plan_10000.txt> <K limite>" << endl;
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

    
    int procesos_activos = 0;
    int tareas_completadas = 0;
    int total_tareas = docho.size();

    unordered_map<pid_t, string> pid_to_tarea;
    unordered_map<pid_t,int> pid_to_pipe;
    unordered_map<string,Planificador*> mapa_tareas;
    
    for (auto& tarea : docho){
        mapa_tareas[tarea.id_tarea] = &tarea;
    }
    
    int tareas_canceladas = 0;

    while (tareas_completadas + tareas_canceladas < total_tareas){
        while (!listos.empty() && procesos_activos < K){
            Planificador* actual = listos.front();
            listos.pop();
            if(actual->cancelada){
                continue;
            }

            int fd[2];
            if(pipe(fd) == -1){
                perror("Error al crear pipe");
                exit(1);
            }

            int fd_in[2];
            if(pipe(fd_in) == -1){
                perror("Error al crear pipe");
                exit(1);
            }

            string insumos_previos = "";
            for (auto& dep : actual->dependencias) {
                insumos_previos += mapa_tareas[dep]->insumo + " ";
            }

            pid_t pid = fork();
            if (pid == 0) {
                close(fd[0]);
                close(fd_in[1]);

                if (!actual->dependencias.empty()) {
                    char buf_in[200];
                    ssize_t n = read(fd_in[0], buf_in, sizeof(buf_in) - 1);
                    if (n > 0) {
                        buf_in[n] = '\0';
                        cout << "[Hijo: " << getpid() << "] " << actual->nombre 
                             << " recibió insumos: " << buf_in << endl;
                    }
                }
                close(fd_in[0]);
                

                /* //simular error en prender_carbon
                if(actual->nombre == "prender_carbon"){
                    cout<< "[Hijo : " << getpid() << "]" << " hubo un error al prender el carbon " << endl;
                    exit(1);
                }*/


                cout << "[Hijo: " << getpid() << "] Ejecutando " << actual->nombre << " (ID: " << actual->id_tarea << ", " << actual->tiempo << " ms)" << endl; 
                usleep(actual->tiempo * 1000);
                string insumo_completado= "listo_" + actual->nombre;
                write(fd[1], insumo_completado.c_str(), insumo_completado.size());
                close(fd[1]);
                cout << "[Hijo: " << getpid() << "] Finalizado " << actual->nombre << " (ID: " << actual->id_tarea << ")" << endl; 
                exit(0);
            }
            else{
                close(fd[1]);
                close(fd_in[0]);

                if (!actual->dependencias.empty()) {
                    write(fd_in[1], insumos_previos.c_str(), insumos_previos.size());
                }
                close(fd_in[1]);

                pid_to_tarea[pid] = actual->id_tarea;
                pid_to_pipe[pid] = fd[0];
                procesos_activos++;
            }
        }

        int status;
        pid_t pid_muerto = wait(&status);
        if (pid_muerto > 0) {
            procesos_activos--;
            tareas_completadas++;

            string tarea_muerta = pid_to_tarea[pid_muerto];
            Planificador* tarea_terminada = mapa_tareas[tarea_muerta];

            int fd_lectura = pid_to_pipe[pid_muerto];
            
            if(WIFEXITED(status) && WEXITSTATUS(status) == 0){
                char buffer[200];

            ssize_t bytes_leidos = read(fd_lectura, buffer, sizeof(buffer) - 1);
            if(bytes_leidos > 0){
                buffer[bytes_leidos] = '\0';
                tarea_terminada->insumo = buffer;

            }

            cout << "[Padre] Terminó tarea: " << tarea_terminada->nombre << " (PID: " << pid_muerto << ") -> Insumo recibido: " << tarea_terminada->insumo << endl;

            for (auto& sucesora : tarea_terminada->sucesoras) {
                mapa_tareas[sucesora]->dep_counter--;
                if (mapa_tareas[sucesora]->dep_counter == 0) {
                    listos.push(mapa_tareas[sucesora]);
                }
            }
            }
            else{
                cout<< "[Padre] La tarea " << tarea_terminada->nombre << " falló ( " << WEXITSTATUS(status) << " ) " << endl;
                abortar_rama(tarea_terminada, mapa_tareas, tareas_canceladas);
            }
            close(fd_lectura);
            pid_to_tarea.erase(pid_muerto);
            pid_to_pipe.erase(pid_muerto);
        }
    }
    

    return 0;
}
