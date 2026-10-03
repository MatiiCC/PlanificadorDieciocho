# Tarea 1: Planificador Dieciochero
**Sistemas Operativos (UDP - 2026-02)**  
**Estudiante:** Matías Cáceres

---

## 1. Descripción
Este proyecto implementa un simulador y planificador de actividades modeladas como un Grafo Acíclico Dirigido (DAG). El sistema gestiona la concurrencia de hasta K procesos en paralelo, sincroniza el paso de mensajes de insumos usando pipes, aísla errores abortando ramas dependientes en cascada y responde a la señal `SIGINT` (Ctrl+C) para simular una inspección sanitaria.

---

## 2. Compilación y Ejecución

El proyecto incluye un `Makefile` configurado con los estándares exigidos (`g++ -Wall -Wextra -std=c++17 -lpthread`).

### Compilar:
```bash
make
```

### Ejecutar:
```bash
./planificador <archivo_plan.txt> <K>
```

**Ejemplo:**
```bash
./planificador plan.txt 2
```

### Limpiar ejecutable:
```bash
make clean
```

### Simular fallas (Aislamiento de Errores):
Para simular una falla en una actividad, se puede descomentar el bloque de simulacion de error en las lineas 225-229 de planificador.cpp (por ejemplo, para la tarea prender_carbon) y volver a compilar con make.

---

## 3. Estructura y Funciones Implementadas

* **`struct Planificador`**: Estructura del nodo del DAG. Almacena el ID, nombre, tiempo de ejecución, lista de dependencias, sucesoras, el contador `dep_counter`, el insumo generado y el estado booleano `cancelada`.
* **`leer_archivo(...)`**: Lee el archivo `.txt` línea por línea. Limpia comas y separa campos. Si una actividad no especifica tiempo, le asigna automáticamente un valor aleatorio entre 100 y 5000 ms usando `rand()`. Al final, conecta a cada tarea con sus respectivas sucesoras.
* **`abortar_rama(...)`**: Función recursiva para el aislamiento de fallos. Cuando una tarea falla, recorre todas sus sucesoras directas e indirectas marcándolas como `cancelada = true` y aumentando el contador de tareas canceladas para evitar bloqueos en el programa.
* **`llegada_seremi(...)`**: Manejador de la señal `SIGINT`. Recorre la lista de procesos activos (`pid_activos`), les envía la señal `SIGTERM` con `kill()`, espera a que terminen con `waitpid()` y finaliza el programa de forma limpia sin dejar procesos zombis.
* **`main(...)`**: Inicializa las estructuras, configura `sigaction` para `SIGINT`, carga las tareas sin dependencias en la cola `listos` y orquesta la creación de procesos respetando la concurrencia máxima $K$.

---

## 4. Decisiones de Diseño

1. **Evitar Busy Waiting y Race Conditions:**  
   Para controlar la concurrencia $K$ no se usan bucles vacíos (`while(1)`) que consuman CPU innecesariamente. En su lugar, se utiliza la llamada al sistema bloqueante `wait(&status)`, lo que permite que el kernel duerma al proceso padre hasta que algún hijo libere un cupo.

2. **Pipes dinámicos por proceso (`fd` y `fd_in`):**  
   En vez de abrir miles de pipes al inicio del programa (lo que saturaría el límite de descriptores de archivos `ulimit` en pruebas grandes de 10.000 tareas), se abren dos pipes por proceso al momento de hacer `fork()`:
   * `fd_in`: Para que el padre le pase al nuevo hijo los insumos producidos por sus dependencias previas.
   * `fd`: Para que el hijo reporte su propio insumo al terminar.  
   Ambos extremos no utilizados se cierran inmediatamente con `close()`.

3. **Manejo de `SIGINT` en Procesos Hijos:**  
   Al presionar `Ctrl+C` en la terminal, la señal llega a todo el grupo de procesos en primer plano. Para evitar que tanto el padre como los hijos ejecuten el handler e impriman mensajes repetidos, los hijos ejecutan `signal(SIGINT, SIG_IGN)` apenas nacen. De esta forma, solo el proceso padre gestiona la llegada de la autoridad y clausura ordenadamente a sus hijos.

4. **Aislamiento de Fallos con `WEXITSTATUS`:**  
   Al capturar un proceso con `wait(&status)`, se verifica si terminó con error mediante las macros `WIFEXITED` y `WEXITSTATUS`. Si el código de salida es distinto de 0, la tarea se marca como fallida y se cancelan únicamente sus ramas dependientes con `abortar_rama()`, permitiendo que el resto de las tareas independientes finalicen su trabajo.
