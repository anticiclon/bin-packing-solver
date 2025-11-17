// solver_utils.h - Funciones LIBRES (no en clase)
#pragma once
#include "bin.h"
#include <vector>
#include <string>
#include <map> 


/**
 * @brief Estructura para almacenar la solución exacta del bin packing
 * 
 * Contiene el resultado de resolver el problema de bin packing con
 * el método exacto (Branch-and-Bound con Gurobi).
 */
struct ExactSolution {
    int num_bins;                           // Número de bins usados
    std::map<int, std::vector<int>> bins;   // bin_id -> lista de tamaños de items
    double solve_time;                       // Tiempo de resolución en segundos
    bool is_optimal;                         // ¿Se encontró solución óptima?
    
    /**
     * @brief Constructor por defecto
     * 
     * Inicializa todos los campos a valores por defecto:
     * - num_bins = 0
     * - solve_time = 0.0
     * - is_optimal = false
     */
    ExactSolution() 
        : num_bins(0), solve_time(0.0), is_optimal(false) {}
};




/**
* @brief Calcula estadísticas completas de una solución
* 
* @param bins Vector de bins
* @param capacity Capacidad de cada bin
* @return Struct con estadísticas detalladas
*/
struct SolutionStats {
    int num_bins;
    double utilization;
    int min_used;
    int max_used;
    double avg_used;
};








/**
 * @brief Funciones de utilidad para bin packing
 * 
 * Contiene funciones auxiliares para imprimir, guardar y
 * analizar soluciones de bin packing.
 */
namespace BinPackingUtils {
    /**
     * @brief Imprime una solución en formato legible
     * 
     * @param name Nombre de la solución (ej: "First Fit")
     * @param bins Vector de bins que contiene la solución
     */
    void printSolution(const std::string& name, const std::vector<Bin>& bins);

    /**
     * @brief Guarda una solución en formato JSON
     * 
     * @param filename Ruta del archivo donde guardar (ej: "output/solution.json")
     * @param bins Vector de bins a guardar
     */
    bool saveSolution(const std::string& filename, const std::vector<Bin>& bins);

    /**
     * @brief Calcula la utilización promedio de los bins
     * 
     * @param bins Vector de bins
     * @param capacity Capacidad de cada bin
     * @return Porcentaje de utilización (0.0 a 100.0)
     */
    double calculateUtilization(const std::vector<Bin>& bins, int capacity);

    /**
     * @brief Calcula estadísticas completas de una solución
     * 
     * @param bins Vector de bins
     * @param capacity Capacidad de cada bin
     * @return Struct con estadísticas detalladas
     */
    SolutionStats calculateStats(const std::vector<Bin>& bins, int capacity);
}


/**
 * @brief Clase para resolver bin packing con heurísticas
 * 
 * Implementa cuatro algoritmos heurísticos clásicos:
 * - First Fit (FF)
 * - Best Fit (BF)
 * - Worst Fit (WF)
 * - First Fit Decreasing (FFD)
 */
class BinPackingHeuristic {
private:
    int bin_capacity_;           ///< Capacidad de cada bin
    std::vector<Item> items_;    ///< Lista de items a empaquetar
    
public:
    /**
     * @brief Constructor de la clase
     * 
     * @param capacity Capacidad máxima de cada bin
     * @param items Vector de items a empaquetar
     */
    BinPackingHeuristic(int capacity, const std::vector<Item>& items);
    
    /**
     * @brief Resuelve usando First Fit
     * 
     * Coloca cada item en el primer bin donde quepa.
     * 
     * @return Vector de bins con la solución
     * @complexity O(n²) en el peor caso
     */
    std::vector<Bin> solveFirstFit(const std::vector<Item>& items_to_pack); 
    std::vector<Bin> solveFirstFit();

    /**
     * @brief Resuelve usando Best Fit
     * 
     * Coloca cada item en el bin más lleno donde aún quepa.
     * 
     * @return Vector de bins con la solución
     * @complexity O(n²) en el peor caso
     */
    std::vector<Bin> solveBestFit();

    /**
     * @brief Resuelve usando Worst Fit
     * 
     * Coloca cada item en el bin menos lleno.
     * 
     * @return Vector de bins con la solución
     * @complexity O(n²) en el peor caso
     */
    std::vector<Bin> solveWorstFit();

    /**
     * @brief Resuelve usando First Fit Decreasing
     * 
     * Ordena items en orden decreciente y aplica First Fit.
     * Generalmente produce mejores resultados que FF.
     * 
     * @return Vector de bins con la solución
     * @complexity O(n log n + n²) = O(n²)
     * 
     * @note Esta heurística suele encontrar soluciones cercanas al óptimo
     */
    std::vector<Bin> solveFirstFitDecreasing();
    
};



/**
 * @brief Clase para resolver bin packing con método exacto (Gurobi)
 * 
 * Utiliza programación lineal entera mixta (MILP) para encontrar
 * soluciones óptimas o de alta calidad con límite de tiempo.
 */
class BinPackingSolver {
private:
    int bin_capacity_;           ///< Capacidad de cada bin
    std::vector<Item> items_;    ///< Lista de items a empaquetar
    
public:
    /**
     * @brief Constructor de la clase
     * 
     * @param capacity Capacidad máxima de cada bin
     * @param items Vector de items a empaquetar
     */
    BinPackingSolver(int capacity, const std::vector<Item>& items);
    
    /**
     * @brief Resuelve el problema de forma exacta usando Gurobi
     * 
     * Construye y resuelve un modelo MILP con Branch-and-Bound.
     * Tiene un límite de tiempo de 5 minutos.
     * 
     * @param warm_start Solución inicial opcional (puede acelerar la resolución)
     * @return Estructura ExactSolution con el resultado
     * 
     * @note Si se alcanza el límite de tiempo, devuelve la mejor solución
     *       encontrada (que puede no ser óptima)
     * 
     * @warning Requiere licencia de Gurobi instalada
     */
    ExactSolution solveExact(const std::vector<Bin>* warm_start = nullptr);

    /**
     * @brief Convierte ExactSolution a vector de Bin
     * 
     * @param solution Solución en formato ExactSolution
     * @return Vector de bins reconstruido
     * 
     * @note Útil para usar BinPackingUtils con soluciones exactas
     */
    std::vector<Bin> solutionToBins(const ExactSolution& solution) const;
};








































