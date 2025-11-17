#include "solver.h"
#include <iostream>
#include <algorithm>


/**
 * @brief Implementación del constructor de BinPackingHeuristic
 * 
 * Inicializa el solver con la capacidad de bins y la lista de items.
 * Almacena copias de los parámetros para uso posterior.
 * 
 * @param capacity Capacidad de cada bin
 * @param items Vector de items a empaquetar
 * 
 * @implementation
 * - bin_capacity_: Almacena la capacidad para crear nuevos bins
 * - items_: Copia del vector de items (permite reutilización del solver)
 * 
 */
BinPackingHeuristic::BinPackingHeuristic(int capacity, const std::vector<Item>& items)
    : bin_capacity_(capacity)
    , items_(items)
{
}


/**
 * @brief Implementación de First Fit con vector de items personalizado
 * 
 * Algoritmo First Fit:
 * 1. Para cada item en el orden dado:
 *    a. Intenta colocarlo en el primer bin existente donde quepa
 *    b. Si no cabe en ninguno, crea un nuevo bin
 * 
 * Estrategia de búsqueda:
 * - Recorre bins en orden de creación (0, 1, 2, ...)
 * - Se detiene en el primer bin donde el item quepa
 * - Si ningún bin tiene espacio suficiente, abre uno nuevo
 * 
 * @param items_to_pack Vector de items a procesar (puede ser diferente a items_)
 * @return Vector de bins con la solución
 * 
 * @implementation
 * - Variable booleana 'placed' rastrea si el item fue colocado
 * - Usa addItem() que retorna false si el item no cabe
 * - break detiene la búsqueda al encontrar el primer bin válido
 * 
 * @complexity Tiempo: O(n²) en el peor caso
 *             - n items a procesar
 *             - Cada item puede revisar hasta n bins
 *             Espacio: O(n) para almacenar los bins
 * 
 * @see solveFirstFitDecreasing() que usa este método
 */
std::vector<Bin> BinPackingHeuristic::solveFirstFit(const std::vector<Item>& items_to_pack) {
    std::vector<Bin> bins;
    
    // Procesar cada item en orden
    for (const auto& item : items_to_pack) {
        bool placed = false;
        
        // Intentar colocar en bins existentes (primer bin donde quepa)
        for (auto& bin : bins) {
            if (bin.addItem(item)) {
                placed = true;
                break; // Salir inmediatamente al encontrar el primer bin válido
            }
        }
        
        // Si no cupo en ningún bin existente, crear nuevo bin
        if (!placed) {
            Bin new_bin(bin_capacity_);
            new_bin.addItem(item);
            bins.push_back(new_bin);
        }
    }
    
    return bins;
}



/**
 * @brief Implementación de First Fit usando items_ del solver
 * 
 * Wrapper que llama a solveFirstFit(items_to_pack) con los items
 * almacenados en el constructor.
 * 
 * @return Vector de bins con la solución First Fit
 * 
 * @implementation
 * Simplemente delega al método sobrecargado con items_
 */
std::vector<Bin> BinPackingHeuristic::solveFirstFit() {
    return solveFirstFit(items_); 
}


/**
 * @brief Implementación de Best Fit
 * 
 * Algoritmo Best Fit:
 * 1. Para cada item:
 *    a. Busca el bin más lleno (menor espacio restante) donde quepa
 *    b. Lo coloca en ese bin
 *    c. Si no cabe en ninguno, crea un nuevo bin
 * 
 * Estrategia de selección:
 * - Minimiza el espacio restante después de colocar el item
 * - Intenta llenar bins lo más posible antes de abrir nuevos
 * - Objetivo: reducir fragmentación de espacio
 * 
 * @return Vector de bins con la solución
 * 
 * @implementation
 * - best_bin_idx: Índice del mejor bin (-1 si no se encuentra ninguno)
 * - min_remaining: Espacio mínimo restante encontrado
 * - Inicializa min_remaining en capacity+1 para que cualquier bin sea mejor
 * - Recorre TODOS los bins para encontrar el óptimo (no se detiene en el primero)
 * 
 * @complexity Tiempo: O(n²)
 *             - n items a procesar
 *             - Cada item revisa TODOS los bins existentes
 *             Espacio: O(n) para almacenar los bins
 * 
 * @example
 * Capacidad: 10, Items: [6, 4, 3, 2]
 * - Item 6: Bin1 (espacio: 4)
 * - Item 4: Bin1 (espacio: 0) - llena completamente
 * - Item 3: Bin2 (espacio: 7)
 * - Item 2: Bin2 (espacio: 5) - elige Bin2 sobre Bin1 lleno
 * Resultado: 2 bins
 */
std::vector<Bin> BinPackingHeuristic::solveBestFit() {
    std::vector<Bin> bins;
    
    // Procesar cada item en orden
    for (const auto& item : items_) {
        int best_bin_idx = -1;
        int min_remaining = bin_capacity_ + 1;  // Inicializar con valor mayor que cualquier espacio posible
        
        // Buscar el bin MÁS LLENO (menor espacio restante) donde quepa
        for (size_t j = 0; j < bins.size(); ++j) {
            // El item debe caber
            if (bins[j].getRemaining() >= item.getSize()) {
                // Buscar el bin con MAYOR espacio restante
                if (bins[j].getRemaining() < min_remaining) {
                    min_remaining = bins[j].getRemaining();
                    best_bin_idx = j;
                }
            }
        }
        
        // Si encontramos un bin, añadir el item ahí
        if (best_bin_idx != -1) {
            bins[best_bin_idx].addItem(item);
        } else {
            // Si no cupo en ninguno, crear un nuevo bin
            Bin new_bin(bin_capacity_);
            new_bin.addItem(item);
            bins.push_back(new_bin);
        }
    }
    
    return bins;
}

/**
 * @brief Implementación de Worst Fit
 * 
 * Algoritmo Worst Fit:
 * 1. Para cada item:
 *    a. Busca el bin menos lleno (mayor espacio restante) donde quepa
 *    b. Lo coloca en ese bin
 *    c. Si no cabe en ninguno, crea un nuevo bin
 * 
 * Estrategia de selección:
 * - Maximiza el espacio restante después de colocar el item
 * - Distribuye items más uniformemente entre bins
 * - Objetivo: balancear la carga entre bins
 * 
 * @return Vector de bins con la solución
 * 
 * @implementation
 * - best_bin_idx: Índice del mejor bin (-1 si no se encuentra ninguno)
 * - max_remaining: Espacio máximo restante encontrado
 * - Inicializa max_remaining en -1 para detectar si encontramos algún bin válido
 * - Recorre TODOS los bins para encontrar el óptimo
 * 
 * @example
 * Capacidad: 10, Items: [6, 4, 3, 2]
 * - Item 6: Bin1 (espacio: 4)
 * - Item 4: Bin2 (espacio: 6) - crea nuevo en lugar de llenar Bin1
 * - Item 3: Bin2 (espacio: 3) - elige el más vacío
 * - Item 2: Bin1 (espacio: 2) - completa distribución
 * Resultado: 2 bins (pero con peor balance que BF en general)
 */
std::vector<Bin> BinPackingHeuristic::solveWorstFit() {
    std::vector<Bin> bins;
    
    // Procesar cada item en orden
    for (const auto& item : items_) {
        int best_bin_idx = -1;
        int max_remaining = -1;   // Inicializar en -1 para detectar si encontramos algún bin válido
        
        // Buscar el bin MENOS LLENO (mayor espacio restante) donde quepa el item
        for (size_t j = 0; j < bins.size(); ++j) {
            // El item debe caber
            if (bins[j].getRemaining() >= item.getSize()) {
                // Buscar el bin con MAYOR espacio restante
                if (bins[j].getRemaining() > max_remaining) {
                    max_remaining = bins[j].getRemaining();
                    best_bin_idx = j;
                }
            }
        }
        
        // Si encontramos un bin, añadir el item ahí
        if (best_bin_idx != -1) {
            bins[best_bin_idx].addItem(item);
        } else {
            // Si no cupo en ninguno, crear un nuevo bin
            Bin new_bin(bin_capacity_);
            new_bin.addItem(item);
            bins.push_back(new_bin);
        }
    }
    return bins;
}


/**
 * @brief Resuelve el problema de bin packing usando First Fit Decreasing
 * 
 * Implementación del algoritmo First Fit Decreasing (FFD):
 * 1. Ordena los items en orden decreciente de tamaño
 * 2. Para cada item, lo coloca en el primer bin donde quepa
 * 3. Si no cabe en ningún bin existente, abre uno nuevo
 * 
 * FFD generalmente produce soluciones dentro de 11/9 * OPT + 6/9,
 * donde OPT es la solución óptima (Johnson, 1973).
 * 
 * @return Vector de bins conteniendo la solución
 * 
 * @complexity Tiempo: O(n log n + n²) donde n es el número de items
 *             - O(n log n) para ordenar
 *             - O(n²) para asignar items en el peor caso
 *             Espacio: O(n) para almacenar los bins
 * 
 * @note Esta heurística suele producir resultados muy cercanos al óptimo
 *       y es significativamente mejor que First Fit simple
 * 
 * @see solveFirstFit() para la versión sin ordenamiento
 * 
 * @example
 * BinPackingHeuristic solver(10, items);
 * auto solution = solver.solveFirstFitDecreasing();
 * std::cout << "Bins usados: " << solution.size() << std::endl;
 */
std::vector<Bin> BinPackingHeuristic::solveFirstFitDecreasing() {
    // 1. Ordenar items de mayor a menor
    std::vector<Item> sorted_items = items_;
    std::sort(sorted_items.begin(), sorted_items.end(), std::greater<Item>());
    
    // 2. Aplicar First Fit sobre items ordenados
    return solveFirstFit(sorted_items);  
}