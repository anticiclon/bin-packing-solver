# Scripts de Experimentación

## run_experiments.ps1

Script de PowerShell para ejecutar experimentos completos con todas las instancias.

### Requisitos

- PowerShell 5.1 o superior
- Proyecto compilado en `build/Release/bin_packing.exe`
- Instancias generadas en `instances/`

### Uso
```powershell
# Desde la raíz del proyecto
cd scripts
.\run_experiments.ps1
```

### Salida

El script genera:

- **`output/results_YYYYMMDD_HHMMSS.csv`**: Tabla resumen con métricas
- **`output/logs/*.txt`**: Logs detallados por instancia
- **`output/*.json`**: Soluciones en formato JSON

### Tabla CSV

Columnas generadas:
- `Instancia`: Nombre de la instancia
- `Items`: Número de items
- `Capacidad`: Capacidad de cada bin
- `FF`, `BF`, `WF`, `FFD`: Bins usados por cada heurística
- `Gurobi`: Bins en solución exacta
- `Optimo`: SI/NO si es óptimo probado
- `Gap_FFD`: Gap de FFD respecto al óptimo
- `Tiempo_seg`: Tiempo de resolución
- `Utilizacion`: Porcentaje de utilización

### Troubleshooting

**Error: "No se encuentra bin_packing.exe"**
```powershell
cd ..\build
cmake --build . --config Release
```

**No se encuentran instancias**
```powershell
cd ..\build\Release
.\instance_generator.exe
```