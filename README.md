# Bin Packing 1D Solver

Implementación de algoritmos heurísticos y solver exacto para el problema de bin packing unidimensional.

## Descripción

Este proyecto compara el rendimiento de cuatro algoritmos heurísticos (First Fit, Best Fit, Worst Fit, First Fit Decreasing) y un modelo exacto basado en programación lineal entera mixta (MILP) usando Gurobi Optimizer.

Este proyecto compara el rendimiento de cuatro algoritmos heurísticos (First Fit, Best Fit, Worst Fit, First Fit Decreasing) y un solver exacto basado en programación lineal entera mixta (MILP) usando Gurobi Optimizer.
(Add PowerShell automation script with proper paths)

## Características

- **Heurísticas clásicas**: FF, BF, WF, FFD
- **Solver exacto**: Formulación MILP con Gurobi
- **Warm start**: Inicialización del MILP con solución heurística
- **Generador de instancias**: Crea instancias de prueba con diferentes características
- **Análisis automático**: Scripts para ejecutar experimentos y generar reportes

## Requisitos

- CMake 3.15+
- C++17
- Gurobi Optimizer 9.5+ (licencia académica o comercial)
- Visual Studio 2019+ (Windows)

## Compilación
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## Uso

### Programa principal
```bash
# Ejecutar casos predefinidos
cd build/Release
.\bin_packing.exe

# Ejecutar con instancia específica
.\bin_packing.exe ..\..\instances\instance_uniform_n50.txt
```

### Generar instancias
```bash
.\instance_generator.exe
```

### Ejecutar experimentos completos
```powershell
.\run_experiments.ps1
```

## Estructura del proyecto
```
bin-packing-1d/
├── include/          # Headers
│   ├── item.h
│   ├── bin.h
│   └── solver.h
├── src/              # Implementación
│   ├── main.cpp
│   ├── solver.cpp
│   ├── heuristic.cpp
│   └── utils.cpp
├── instances/        # Instancias de prueba
├── output/           # Resultados (JSON, logs, CSV)
└── CMakeLists.txt
```

## Resultados

Los resultados se guardan en:
- `output/*.json`: Soluciones en formato JSON
- `output/logs/*.txt`: Logs detallados de cada ejecución
- `output/results_*.csv`: Tabla resumen con métricas
