#include "solver.h"
#include <iostream>
#include <fstream>
#include <iomanip>

/**
 * @brief Namespace que agrupa funciones de utilidad para bin packing
 * 
 * Este namespace proporciona herramientas independientes que pueden
 * usarse con cualquier solución de bin packing, sin importar el
 * algoritmo utilizado para generarla.
 */
namespace BinPackingUtils {


/**
 * @brief Implementación de printSolution
 * 
 * Imprime una solución de bin packing en formato legible para consola.
 * 
 * Formato de salida:
 * ```
 * === Nombre de la Solución ===
 * Bins usados: N
 * Bin 1: Bin: [id:size, id:size] = used/capacity
 * Bin 2: Bin: [id:size, id:size] = used/capacity
 * ...
 * ```
 * 
 * @param name Nombre descriptivo de la solución (ej: "First Fit", "Optimal")
 * @param bins Vector de bins que contiene la solución
 * 
 * @implementation
 * - Usa '\n' al final de cada línea para mejor rendimiento que std::endl
 * - Delega el formateo de cada bin a Bin::print()
 * - Numera bins desde 1 (no desde 0) para legibilidad humana
 * 
 * @example
 * std::vector<Bin> solution = heuristic.solveFirstFit();
 * BinPackingUtils::printSolution("First Fit", solution);
 * 
 * // Salida:
 * // === First Fit ===
 * // Bins usados: 3
 * // Bin 1: Bin: [1:50, 2:30] = 80/100
 * // Bin 2: Bin: [3:70, 4:20] = 90/100
 * // Bin 3: Bin: [5:40] = 40/100
 */
void printSolution(const std::string& name, const std::vector<Bin>& bins) {
    std::cout << "\n=== " << name << " ===\n";
    std::cout << "Bins usados: " << bins.size() << "\n";

    // Imprimir cada bin con numeración empezando en 1
    for (size_t i = 0; i < bins.size(); ++i) {
        std::cout << "Bin " << (i+1) << ": ";
        bins[i].print();
    }
}

/**
 * @brief Guarda una solución de bin packing en formato JSON
 * 
 * Genera un archivo JSON con la estructura:
 * - num_bins: total de bins
 * - bins: array con bin_id, capacity, used, items
 * 
 * @param filename Ruta del archivo (se sobrescribe si existe)
 * @param bins Vector de bins a guardar
 * @return true si se guardó exitosamente, false si no se pudo abrir el archivo
 *  * 
 * @warning Sobrescribe el archivo sin advertencia
 * @note Solo valida apertura, no cada escritura
 * 
 * @example
 * if (BinPackingUtils::saveSolution("output.json", solution)) {
 *     std::cout << "Guardado exitoso" << std::endl;
 * }
 */
bool saveSolution(const std::string& filename, const std::vector<Bin>& bins) {
    std::ofstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Error: No se pudo abrir " << filename << std::endl;
        return false;
    }

    // Cabecera del JSON
    file << "{\n";
    file << "  \"num_bins\": " << bins.size() << ",\n";
    file << "  \"bins\": [\n";
    
    // Iterar sobre cada bin
    for (size_t i = 0; i < bins.size(); ++i) {
        file << "    {\n";
        file << "      \"bin_id\": " << (i+1) << ",\n";
        file << "      \"capacity\": " << bins[i].getCapacity() << ",\n";
        file << "      \"used\": " << bins[i].getUsed() << ",\n";
        file << "      \"items\": [";
        
        // Lista de items (solo tamaños)
        const auto& items = bins[i].getItems();
        for (size_t j = 0; j < items.size(); ++j) {
            file << items[j].getSize();
            if (j < items.size() - 1) file << ", ";
        }
        
        file << "]\n";
        file << "    }";

        // Añadir coma si no es el último bin
        if (i < bins.size() - 1) file << ",";
        file << "\n";
    }
    
    // Cierre del JSON
    file << "  ]\n";
    file << "}\n";
    
    file.close();
    return true;
}


/**
 * @brief Implementación de calculateUtilization
 * 
 * Calcula el porcentaje de utilización total de todos los bins.
 * 
 * Fórmula:
 * ```
 * Utilización = (Suma de espacios usados) / (Capacidad total) * 100
 *             = (Σ used_i) / (n * capacity) * 100
 * ```
 * 
 * donde:
 * - used_i es el espacio usado en el bin i
 * - n es el número de bins
 * - capacity es la capacidad de cada bin
 * 
 * Interpretación:
 * - 100%: Todos los bins están completamente llenos (óptimo teórico)
 * - 75%: En promedio, cada bin está 3/4 lleno
 * - 50%: Mucho espacio desperdiciado (pobre utilización)
 * 
 * @param bins Vector de bins que contiene la solución
 * @param capacity Capacidad de cada bin
 * @return Porcentaje de utilización (0.0 a 100.0)
 * 
 * @implementation
 * - Caso especial: retorna 0.0 si bins.empty() para evitar división por cero
 * - Suma todos los espacios usados en un bucle
 * - Calcula capacidad total como: bins.size() * capacity
 * - Retorna porcentaje como double
 * 
 * @warning Asume que todos los bins tienen la misma capacidad
 * 
 * @example
 * std::vector<Bin> solution = heuristic.solveFirstFit();
 * double util = BinPackingUtils::calculateUtilization(solution, 100);
 * std::cout << "Utilización: " << util << "%" << std::endl;
 * 
 * // Ejemplo con capacidad=100:
 * // Bin 1: 80/100, Bin 2: 90/100, Bin 3: 40/100
 * // Total usado: 210, Total capacidad: 300
 * // Utilización: 70.0%
 */
double calculateUtilization(const std::vector<Bin>& bins, int capacity) {
    // Caso especial: sin bins, utilización es 0
    if (bins.empty()) return 0.0;
    
    // Sumar todo el espacio usado
    int total_used = 0;
    for (const auto& bin : bins) {
        total_used += bin.getUsed();
    }
    
    // Calcular capacidad total disponible
    int total_capacity = bins.size() * capacity;

    // Retornar porcentaje
    return 100.0 * total_used / total_capacity;
}



/**
 * @brief Implementación de calculateStats
 * 
 * Calcula un conjunto completo de estadísticas sobre una solución:
 * - Número de bins
 * - Utilización total
 * - Bin menos lleno (mínimo usado)
 * - Bin más lleno (máximo usado)
 * - Promedio de espacio usado
 * 
 * @param bins Vector de bins
 * @param capacity Capacidad de cada bin
 * @return SolutionStats con todas las métricas
 * 
 * @note Si bins está vacío, retorna estadísticas con valores 0
 */
SolutionStats calculateStats(const std::vector<Bin>& bins, int capacity) {
    SolutionStats stats;
    stats.num_bins = bins.size();
    stats.utilization = calculateUtilization(bins, capacity);
    
    // Caso especial: sin bins
    if (bins.empty()) {
        stats.min_used = 0;
        stats.max_used = 0;
        stats.avg_used = 0.0;
        return stats;
    }
    
    // Inicializar con el primer bin
    stats.min_used = bins[0].getUsed();
    stats.max_used = bins[0].getUsed();
    int total = 0;
    
    // Recorrer todos los bins para calcular min, max y total
    for (const auto& bin : bins) {
        int used = bin.getUsed();
        stats.min_used = std::min(stats.min_used, used);
        stats.max_used = std::max(stats.max_used, used);
        total += used;
    }
    
    // Calcular promedio
    stats.avg_used = static_cast<double>(total) / bins.size();
    return stats;
}



} 