# ==============================================================================
# CMakeLists.txt - Configuración de compilación para Bin Packing Solver
# ==============================================================================
# Este archivo configura la compilación del proyecto usando CMake.
# Genera dos ejecutables:
#   1. bin_packing: Programa principal (usa Gurobi)
#   2. instance_generator: Generador de instancias de prueba
#
# Requisitos:
#   - CMake 3.15 o superior
#   - C++17
#   - Gurobi 9.5.0 (licencia académica o comercial)
#   - Visual Studio 2019 o superior (Windows)
# ==============================================================================

cmake_minimum_required(VERSION 3.15)
project(BinPacking VERSION 1.0 LANGUAGES CXX)

# ==============================================================================
# CONFIGURACIÓN DEL LENGUAJE
# ==============================================================================
# C++17 es necesario para std::filesystem (usado en instance_generator)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==============================================================================
# CONFIGURACIÓN DE GUROBI
# ==============================================================================
# IMPORTANTE: Ajustar esta ruta según tu instalación de Gurobi
# Ruta por defecto: C:/gurobi950/win64
# Para otras versiones: C:/gurobi110/win64, etc.
set(GUROBI_DIR "C:/gurobi950/win64")

# Incluir headers de Gurobi (gurobi_c++.h, gurobi_c.h)
include_directories(${GUROBI_DIR}/include)

# Agregar directorio de librerías de Gurobi
link_directories(${GUROBI_DIR}/lib)

# ==============================================================================
# CONFIGURACIÓN DEL PROYECTO
# ==============================================================================
# Incluir headers propios del proyecto (item.h, bin.h, solver.h)
include_directories(include)

# ==============================================================================
# EJECUTABLE PRINCIPAL: bin_packing
# ==============================================================================
# Programa que ejecuta heurísticas y solver exacto con Gurobi
add_executable(bin_packing
    src/main.cpp          # Programa principal
    src/solver.cpp        # Solver exacto (Gurobi MIP)
    src/heuristic.cpp     # Heurísticas FF, BF, WF, FFD
    src/utils.cpp         # Utilidades (print, save, stats)
    src/bin.cpp           # Clase Bin
    src/item.cpp          # Clase Item
)

# ==============================================================================
# LINKEAR GUROBI AL EJECUTABLE PRINCIPAL
# ==============================================================================
# Gurobi requiere dos librerías:
#   - gurobi_c++md2019: API de C++ para Visual Studio 2019
#   - gurobi95: Librería principal de Gurobi 9.5

# Configuración Release (optimizada, sin símbolos de depuración)
target_link_libraries(bin_packing
    optimized gurobi_c++md2019  # C++ API (Release)
    optimized gurobi95           # Core library
)

# Configuración Debug (con símbolos de depuración)
target_link_libraries(bin_packing
    debug gurobi_c++mdd2019     # C++ API (Debug)
    debug gurobi95               # Core library
)

# NOTA: Si usas Visual Studio 2017, cambiar a gurobi_c++md2017
# NOTA: Si usas otra versión de Gurobi, cambiar gurobi95 → gurobi110, etc.

# ==============================================================================
# OPCIONES DE COMPILACIÓN
# ==============================================================================
if(MSVC)
    # Flags para Visual Studio (Windows)
    # /W4: Nivel de advertencias 4 (alto)
    # /O2: Optimización máxima de velocidad
    target_compile_options(bin_packing PRIVATE /W4 /O2)
else()
    # Flags para g++/clang (Linux/macOS)
    # -Wall -Wextra: Advertencias completas
    # -O2: Optimización nivel 2
    target_compile_options(bin_packing PRIVATE -Wall -Wextra -O2)
endif()

# ==============================================================================
# EJECUTABLE AUXILIAR: instance_generator
# ==============================================================================
# Generador de instancias de prueba (NO requiere Gurobi)
# Este ejecutable es independiente y solo genera archivos .txt
add_executable(instance_generator
    src/instance_generator.cpp
)

# Opciones de compilación para el generador
if(MSVC)
    target_compile_options(instance_generator PRIVATE /W4 /O2)
else()
    target_compile_options(instance_generator PRIVATE -Wall -Wextra -O2)
endif()

# ==============================================================================
# INSTRUCCIONES DE USO
# ==============================================================================
# Para compilar el proyecto:
#
# 1. Crear directorio de compilación:
#    mkdir build
#    cd build
#
# 2. Configurar CMake:
#    cmake ..
#
# 3. Compilar:
#    cmake --build . --config Release
#
# 4. Ejecutables generados en:
#    build/Release/bin_packing.exe
#    build/Release/instance_generator.exe
#
# Para cambiar la instalación de Gurobi:
#    Editar la variable GUROBI_DIR en la línea 31
# ==============================================================================