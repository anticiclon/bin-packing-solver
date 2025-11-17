#include "solver.h"
#include "item.h"
#include "bin.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <filesystem>  


/**
 * @brief Estructura para almacenar una instancia de bin packing
 * 
 * Representa una instancia completa del problema: items a empaquetar
 * y la capacidad de cada bin.
 */
struct Instance {
    std::vector<Item> items;  ///< Items a empaquetar
    int capacity;             ///< Capacidad de cada bin
};


/**
 * @brief Carga una instancia desde un archivo de texto
 * 
 * Formato esperado del archivo:
 * - Línea 1: n_items capacity
 * - Línea 2: size_1 size_2 ... size_n (separados por espacios)
 * 
 * @param filename Ruta del archivo a cargar
 * @return Instance con los datos cargados
 * 
 * @throw exit(1) si no se puede abrir el archivo
 * 
 * @note Los items se numeran automáticamente desde 1
 * 
 * @example
 * // Archivo instance.txt:
 * // 5 100
 * // 40 30 50 20 60
 * 
 * Instance inst = loadInstanceFromFile("instance.txt");
 * // inst.items.size() == 5
 * // inst.capacity == 100
 */
Instance loadInstanceFromFile(const std::string& filename) {
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Error: No se pudo abrir " << filename << std::endl;
        exit(1);
    }
    
    Instance inst;
    int n_items;
    
    // Primera línea: n_items capacity
    file >> n_items >> inst.capacity;
    
    // Segunda línea: tamaños de items
    for (int i = 0; i < n_items; i++) {
        int size;
        file >> size;
        inst.items.push_back(Item(i + 1, size));
    }
    
    file.close();
    
    std::cout << "Instancia cargada: " << n_items << " items, capacidad " << inst.capacity << "\n";
    
    return inst;
}



/**
 * @brief Crea un directorio si no existe
 * 
 * @param path Ruta del directorio a crear
 * 
 * @note Usa std::filesystem para crear directorios recursivamente
 * @note No genera error si el directorio ya existe
 * 
 */
void createOutputDirectory(const std::string& path) {
    std::filesystem::create_directories(path);
}




/**
 * @brief Ejecuta comparación completa entre heurísticas y solver exacto
 * 
 * Pipeline de ejecución:
 * 1. Ejecuta 4 heurísticas (FF, BF, WF, FFD)
 * 2. Resuelve con Gurobi usando warm start de FFD
 * 3. Compara resultados y calcula gaps
 * 4. Muestra estadísticas detalladas
 * 5. Guarda todas las soluciones en JSON
 * 
 * @param items Vector de items a empaquetar
 * @param capacity Capacidad de cada bin
 * @param case_name Nombre del caso (usado para archivos de salida)
 * 
 * @implementation
 * Estructura en 4 partes:
 * - PARTE 1: Heurísticas (FF, BF, WF, FFD)
 * - PARTE 2: Solver exacto con warm start
 * - PARTE 3: Comparación y estadísticas
 * - PARTE 4: Guardado de resultados en JSON
 *  * 
 * @note Crea directorio ../output/ automáticamente
 * @note Genera 5 archivos JSON por caso (4 heurísticas + 1 exacto)
 * 
 * @postcondition Archivos guardados en ../output/:
 *                - heuristic_FF_<case_name>.json
 *                - heuristic_BF_<case_name>.json
 *                - heuristic_WF_<case_name>.json
 *                - heuristic_FFD_<case_name>.json
 *                - exact_solution_<case_name>.json
 * 
 * @example
 * std::vector<Item> items = { Item(1, 50), Item(2, 30) };
 * runComparison(items, 100, "test");
 * // Genera: heuristic_FF_test.json, exact_solution_test.json, etc.
 */
void runComparison(const std::vector<Item>& items, int capacity, const std::string& case_name) {
    std::cout << "\n";
    std::cout << "╔═══════════════════════════════════════════════════════════╗\n";
    std::cout << "║  " << std::left << std::setw(55) << case_name << "  ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════════╝\n\n";
    
    std::cout << "Items: " << items.size() << ", Capacidad: " << capacity << "\n";
    
    // ==========================================
    // PARTE 1: RESOLVER CON HEURÍSTICAS
    // ==========================================
    BinPackingHeuristic heuristic(capacity, items);
    
    auto ff = heuristic.solveFirstFit();
    auto bf = heuristic.solveBestFit();
    auto wf = heuristic.solveWorstFit();
    auto ffd = heuristic.solveFirstFitDecreasing();
    
    std::cout << "\n--- Resultados Heurísticas ---\n";
    std::cout << "First Fit:           " << ff.size() << " bins\n";
    std::cout << "Best Fit:            " << bf.size() << " bins\n";
    std::cout << "Worst Fit:           " << wf.size() << " bins\n";
    std::cout << "First Fit Decreasing: " << ffd.size() << " bins\n";
    
    // ==========================================
    // PARTE 2: RESOLVER DE FORMA EXACTA
    // ==========================================
    // PARTE 2: RESOLVER CON WARM START
    BinPackingSolver solver(capacity, items);  // ← AÑADIR ESTA LÍNEA
    std::cout << "\n--- Resolviendo CON warm start (FFD) ---\n";
    ExactSolution optimal = solver.solveExact(&ffd);
    
    std::vector<Bin> optimal_bins = solver.solutionToBins(optimal);
    BinPackingUtils::printSolution("Optimal (Gurobi)", optimal_bins);

    // Comparar tiempos
    std::cout << "\n--- Comparación Warm Start ---\n";
    std::cout << "Con warm start: " << optimal.solve_time << " segundos\n";

    // ==========================================
    // PARTE 3: COMPARACIÓN
    // ==========================================
    if (optimal.num_bins > 0) {
        if (optimal.is_optimal) {
            // Solución óptima garantizada
            std::cout << "\n--- Comparación con Óptimo ---\n";
            std::cout << "Óptimo (probado):    " << optimal.num_bins << " bins\n";
        } else {
            // Solución no óptima (time limit)
            std::cout << "\n--- Comparación con Mejor Solución Encontrada ---\n";
            std::cout << "Gurobi (no óptimo):  " << optimal.num_bins << " bins\n";
        }

        std::cout << "First Fit:           " << ff.size() << " bins  (+" 
                  << (ff.size() - optimal.num_bins) << ")\n";
        std::cout << "Best Fit:            " << bf.size() << " bins  (+" 
                  << (bf.size() - optimal.num_bins) << ")\n";
        std::cout << "Worst Fit:           " << wf.size() << " bins  (+" 
                  << (wf.size() - optimal.num_bins) << ")\n";
        std::cout << "First Fit Decreasing: " << ffd.size() << " bins  (+" 
                  << (ffd.size() - optimal.num_bins) << ")\n";
        
    
        // Encontrar la mejor heurística
        int best_heuristic = std::min({ff.size(), bf.size(), wf.size(), ffd.size()});
        double gap;
        
        if (optimal.is_optimal) {
            // Gap respecto al óptimo probado
            gap = 100.0 * (best_heuristic - optimal.num_bins) / (double)optimal.num_bins;
            std::cout << "\nMejor heurística: " << best_heuristic << " bins\n";
            std::cout << "Gap con óptimo: " << std::fixed << std::setprecision(2) << gap << "%\n";
        } else {
            // Gap respecto a mejor solución (no necesariamente óptimo)
            gap = 100.0 * (best_heuristic - optimal.num_bins) / (double)optimal.num_bins;
            std::cout << "\nMejor heurística: " << best_heuristic << " bins\n";
            std::cout << "Gap con mejor solución Gurobi: " << std::fixed << std::setprecision(2) << gap << "%\n";
            
            // Información adicional sobre el rango del óptimo real
            if (best_heuristic == optimal.num_bins) {
                std::cout << "⚠ La mejor heurística empata con Gurobi, pero el óptimo real podría ser menor\n";
            }
        }

        // Estadísticas detalladas (solo para soluciones con bins)
        if (optimal.num_bins > 0) {
            std::cout << "\n--- Estadísticas de la Solución ---\n";
            auto stats = BinPackingUtils::calculateStats(optimal_bins, capacity);
            
            std::cout << std::fixed << std::setprecision(2);
            std::cout << "Utilización total: " << stats.utilization << "%\n";
            std::cout << "Bin más lleno:     " << stats.max_used << "/" << capacity 
                    << " (" << (100.0 * stats.max_used / capacity) << "%)\n";
            std::cout << "Bin menos lleno:   " << stats.min_used << "/" << capacity 
                    << " (" << (100.0 * stats.min_used / capacity) << "%)\n";
            std::cout << "Promedio por bin:  " << stats.avg_used << "/" << capacity << "\n";

        }
    } 
    
    // ==========================================
    // PARTE 4: GUARDAR RESULTADOS
    // ==========================================
    // Definir carpeta de salida
    std::string output_dir = "../output/";
    createOutputDirectory(output_dir);

    // Guardar heurísticas con BinPackingUtils
    BinPackingUtils::saveSolution(output_dir + "heuristic_FF_" + case_name + ".json", ff);
    BinPackingUtils::saveSolution(output_dir + "heuristic_BF_" + case_name + ".json", bf);
    BinPackingUtils::saveSolution(output_dir + "heuristic_WF_" + case_name + ".json", wf);
    BinPackingUtils::saveSolution(output_dir + "heuristic_FFD_" + case_name + ".json", ffd);

    // Guardar solución exacta (también con BinPackingUtils)
    BinPackingUtils::saveSolution(output_dir + "exact_solution_" + case_name + ".json", optimal_bins);
    
    std::cout << "\n";
}














/**
 * @brief Programa principal
 * 
 * Ejecuta el análisis comparativo de algoritmos de bin packing.
 * 
 * @param argc Número de argumentos
 * @param argv Vector de argumentos (argv[1] = archivo de instancia)
 * @return 0 si la ejecución fue exitosa
 * 
 * @note Todos los resultados se guardan en ../output/
 * @note Casos predefinidos siempre se ejecutan (incluso con archivo)
 * 
 * @example
 * // Modo 1: Casos predefinidos
 * ./bin_packing.exe
 * // Salida: Caso1 y Caso2
 * 
 * @example
 * // Modo 2: Instancia desde archivo
 * ./bin_packing.exe ../instances/instance_uniform_n50.txt
 * // Salida: Caso1, Caso2 y archivo
 */
int main(int argc, char* argv[]) {
    std::cout << "╔═══════════════════════════════════════════════════════════╗\n";
    std::cout << "║  Bin Packing: Heurísticas vs Solución Exacta (Gurobi)     ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════════╝\n";
    
    // ==========================================
    // CASO 1: Ejemplo del paper original
    // ==========================================
    {
        std::vector<Item> items = {
            Item(1, 7), Item(2, 5), Item(3, 6), Item(4, 4),
            Item(5, 2), Item(6, 3), Item(7, 8), Item(8, 9)
        };
        int capacity = 10;
        
        runComparison(items, capacity, "Caso1");
    }
    
    // ==========================================
    // CASO 2: Ejemplo más grande
    // ==========================================
    {
        std::vector<Item> items;
        std::vector<int> sizes = {8, 7, 6, 5, 5, 4, 4, 3, 3, 3, 2, 2, 2, 1, 1, 1, 1};
        for (size_t i = 0; i < sizes.size(); ++i) {
            items.push_back(Item(i + 1, sizes[i]));
        }
        int capacity = 10;
        
        runComparison(items, capacity, "Caso2");
    }
    
    // ==========================================
    // MODO ARCHIVO (opcional)
    // ==========================================
    if (argc >= 2) {
        std::string instance_file = argv[1];
        std::cout << "\n=== MODO ARCHIVO: Leyendo " << instance_file << " ===\n";
        
        Instance inst = loadInstanceFromFile(instance_file);
        runComparison(inst.items, inst.capacity, "archivo");
    }
    
    std::cout << "\n╔═══════════════════════════════════════════════════════════╗\n";
    std::cout << "║  Análisis completado. Revisa los archivos JSON generados  ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════════╝\n\n";
    
    return 0;
}