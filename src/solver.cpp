#include "solver.h"
#include "gurobi_c++.h"
#include <iostream>
#include <fstream>
#include <iomanip>


/**
 * @brief Constructor de BinPackingSolver
 * 
 * @param capacity Capacidad de cada bin
 * @param items Vector de items a empaquetar
 */
BinPackingSolver::BinPackingSolver(int capacity, const std::vector<Item>& items)
    : bin_capacity_(capacity)
    , items_(items)
{
}





/**
 * @brief Resuelve el problema de bin packing de forma exacta usando Gurobi
 * 
 * Implementa un modelo MIP con las siguientes características:
 * 
 * **Variables de decisión:**
 * - x[i][j]: binaria, 1 si item i está en bin j
 * - y[j]: binaria, 1 si bin j está siendo usado
 * 
 * **Función objetivo:**
 * - Minimizar: Σ y[j] (número total de bins usados)
 * 
 * **Restricciones:**
 * 1. Cada item en exactamente un bin: Σ_j x[i][j] = 1 ∀i
 * 2. Capacidad de bins: Σ_i size[i]*x[i][j] ≤ capacity*y[j] ∀j
 * 3. Romper simetría: y[j] ≥ y[j+1] ∀j (fuerza uso secuencial)
 * 
 * **Warm start:**
 * Si se proporciona una solución inicial (heurística), Gurobi la usa
 * como punto de partida, típicamente acelerando la convergencia.
 * 
 * @param warm_start Solución inicial opcional (típicamente de FFD)
 * @return ExactSolution con bins, tiempo y optimalidad
 * 
 * @implementation
 * Algoritmo:
 * 1. Crear entorno y modelo Gurobi
 * 2. Crear variables x[i][j] e y[j]
 * 3. Si hay warm start, inicializar variables con esa solución
 * 4. Definir función objetivo (minimizar bins)
 * 5. Añadir restricciones de asignación, capacidad y simetría
 * 6. Resolver con límite de 5 minutos
 * 7. Extraer solución (óptima o mejor encontrada)
 * 
 * @complexity Exponencial en el peor caso (NP-hard)
 *             Tiempo límite: 300 segundos (5 minutos)
 * 
 * @note Si alcanza el límite de tiempo:
 *       - Retorna la mejor solución encontrada (no necesariamente óptima)
 *       - is_optimal = false
 *       - Incluye información del gap
 * 
 * @warning Requiere licencia válida de Gurobi
 * @warning Upper bound = n (un item por bin) puede ser muy grande para n grande
 * 
 * @see https://www.gurobi.com para documentación de la API
 * 
 * @example
 * BinPackingSolver solver(100, items);
 * 
 * // Sin warm start
 * ExactSolution sol1 = solver.solveExact();
 * 
 * // Con warm start desde FFD
 * auto ffd_solution = heuristic.solveFirstFitDecreasing();
 * ExactSolution sol2 = solver.solveExact(&ffd_solution);
 */
ExactSolution BinPackingSolver::solveExact(const std::vector<Bin>* warm_start) {
    ExactSolution result;
    
    int n = items_.size();  // número de items
    
    // Caso trivial: sin items
    if (n == 0) {
        result.is_optimal = true;
        return result;
    }
    
    // Upper bound: en el peor caso, un item por bin
    int max_bins = n;
    
    try {
        // ========================================
        // PASO 1: Crear el modelo de Gurobi
        // ========================================
        // En C++ debes crear explícitamente el entorno (GRBEnv) antes del modelo
        GRBEnv env = GRBEnv(true);
        // Los parámetros se configuran en el entorno
        env.set(GRB_IntParam_OutputFlag, 0);  // Modo silencios
        //env.set(GRB_IntParam_OutputFlag, 1);  // Mostrar progreso
        env.set(GRB_DoubleParam_TimeLimit, 300.0);  // Límite de 5 minutos (300 segundos)
        // Debes llamar env.start() antes de crear el modelo
        env.start();
        
        GRBModel model = GRBModel(env);
        model.set(GRB_StringAttr_ModelName, "BinPacking1D");
        
        // ========================================
        // PASO 2: Crear las variables
        // ========================================
        // En C++ debes crear un vector de vectores y cada variable individualmente
        // GRBVar addVar(double lb,      // Lower Bound (límite inferior)
        //      double ub,      // Upper Bound (límite superior)  
        //      double obj,     // Coeficiente objetivo
        //      char vtype,     // Tipo de variable
        //      string name)    // Nombre de la variable

        // x[i][j]: item i asignado al bin j (binaria)
        std::vector<std::vector<GRBVar>> x(n);
        for (int i = 0; i < n; ++i) {
            x[i].resize(max_bins); // Reserva espacio para max_bins elementos en el vector x[i]
            for (int j = 0; j < max_bins; ++j) {
                x[i][j] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, 
                                       "x_" + std::to_string(i) + "_" + std::to_string(j));
            }
        }
        
        // y[j]: bin j está siendo usado (binaria)
        std::vector<GRBVar> y(max_bins);
        for (int j = 0; j < max_bins; ++j) {
            y[j] = model.addVar(0.0, 1.0, 1.0, GRB_BINARY, "y_" + std::to_string(j));
        }

        // ========================================
        // WARM START: Asignar solución inicial
        // ========================================
        if (warm_start != nullptr && !warm_start->empty()) {
            std::cout << "Usando warm start con " << warm_start->size() << " bins\n";
            
            // Inicializar todas las variables a 0
            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < max_bins; ++j) {
                    x[i][j].set(GRB_DoubleAttr_Start, 0.0);
                }
            }
            for (int j = 0; j < max_bins; ++j) {
                y[j].set(GRB_DoubleAttr_Start, 0.0);
            }
            
            // Asignar valores de la solución inicial
            int warm_bins = warm_start->size();
            for (int j = 0; j < warm_bins && j < max_bins; ++j) {
                const Bin& bin = warm_start->at(j);
                
                // Marcar este bin como usado
                y[j].set(GRB_DoubleAttr_Start, 1.0);
                
                // Asignar items a este bin
                const std::vector<Item>& items_in_bin = bin.getItems();
                for (const Item& item : items_in_bin) {
                    // Buscar el índice del item en items_
                    for (int i = 0; i < n; ++i) {
                        if (items_[i].getId() == item.getId()) {
                            x[i][j].set(GRB_DoubleAttr_Start, 1.0);
                            break;
                        }
                    }
                }
            }
        }
        
        // ========================================
        // PASO 3: Función objetivo
        // ========================================
        // Minimizar: sum(y[j] for j in range(max_bins))
        GRBLinExpr objective = 0;
        for (int j = 0; j < max_bins; ++j) {
            objective += y[j];
        }
        model.setObjective(objective, GRB_MINIMIZE);
        
        // ========================================
        // PASO 4: Restricciones
        // ========================================
        
        // Restricción 1: Cada item debe estar en exactamente un bin
        // sum(x[i][j] for j in range(max_bins)) == 1  para cada i
        for (int i = 0; i < n; ++i) {
            GRBLinExpr expr = 0;
            for (int j = 0; j < max_bins; ++j) {
                expr += x[i][j];
            }
            model.addConstr(expr == 1, "item_assignment_" + std::to_string(i));
        }
        
        // Restricción 2: Capacidad de cada bin
        // sum(items[i].size * x[i][j] for i in range(n)) <= capacity * y[j]  para cada j
        for (int j = 0; j < max_bins; ++j) {
            GRBLinExpr expr = 0;
            for (int i = 0; i < n; ++i) {
                expr += items_[i].getSize() * x[i][j];
            }
            model.addConstr(expr <= bin_capacity_ * y[j], "bin_capacity_" + std::to_string(j));
        }
        
        // Restricción 3: Romper simetría
        // y[j] >= y[j+1]  para cada j < max_bins-1
        for (int j = 0; j < max_bins - 1; ++j) {
            model.addConstr(y[j] >= y[j+1], "symmetry_breaking_" + std::to_string(j));
        }
        
        // ========================================
        // PASO 5: Resolver el modelo
        // ========================================
        std::cout << "\nResolviendo con Gurobi (máximo 5 minutos)...\n";
        model.optimize();
        std::cout << "Tiempo de resolución: " << model.get(GRB_DoubleAttr_Runtime) << " segundos\n";
        
        // ========================================
        // PASO 6: Extraer la solución
        // ========================================
        int status = model.get(GRB_IntAttr_Status);

        // Verificar si hay alguna solución disponible (óptima o no)
        if (status == GRB_OPTIMAL || status == GRB_TIME_LIMIT || status == GRB_INTERRUPTED) {
            
            // Verificar si realmente hay una solución
            int sol_count = model.get(GRB_IntAttr_SolCount);
            
            if (sol_count > 0) {
                // Hay al menos una solución disponible
                result.is_optimal = (status == GRB_OPTIMAL);
                result.solve_time = model.get(GRB_DoubleAttr_Runtime);
                
                // Si no es óptimo, mostrar información del gap
                if (!result.is_optimal) {
                    double obj_val = model.get(GRB_DoubleAttr_ObjVal);
                    double obj_bound = model.get(GRB_DoubleAttr_ObjBound);
                    double gap = 100.0 * (obj_val - obj_bound) / obj_val;
                    
                    std::cout << "\n⚠ Solución NO óptima (límite de tiempo alcanzado)\n";
                    std::cout << "  Mejor solución encontrada: " << (int)obj_val << " bins\n";
                    std::cout << "  Mejor cota inferior: " << (int)obj_bound << " bins\n";
                    std::cout << "  Gap: " << std::fixed << std::setprecision(2) << gap << "%\n\n";
                }
                
                // IMPORTANTE: Inicializar result.num_bins en 0
                result.num_bins = 0;
                
                // Extraer qué bins están siendo usados y qué items contienen
                for (int j = 0; j < max_bins; ++j) {
                    try {
                        double y_val = y[j].get(GRB_DoubleAttr_X);
                        if (y_val > 0.5) {  // Bin j está siendo usado
                            result.num_bins++;
                            std::vector<int> items_in_bin;
                            
                            for (int i = 0; i < n; ++i) {
                                double x_val = x[i][j].get(GRB_DoubleAttr_X);
                                
                                if (x_val > 0.5) {  // Item i está en bin j
                                    items_in_bin.push_back(items_[i].getSize());
                                }
                            }
                            
                            if (!items_in_bin.empty()) {
                                result.bins[j] = items_in_bin;
                            }
                        }
                    } catch (GRBException& e) {
                        // Si hay error accediendo a esta variable, continuar
                        continue;
                    }
                }
                
                // Verificación adicional: si no se extrajo ningún bin, reportar error
                if (result.num_bins == 0) {
                    std::cerr << "Advertencia: Se encontró solución pero no se pudieron extraer bins\n";
                    std::cerr << "Estado: " << status << ", Sol count: " << sol_count << "\n";
                }
                
            } else {
                std::cerr << "Error: No se encontró ninguna solución" << std::endl;
                result.is_optimal = false;
            }
            
        } else if (status == GRB_INFEASIBLE) {
            std::cerr << "Error: El modelo es infactible" << std::endl;
            result.is_optimal = false;
        } else {
            std::cerr << "Error: Estado de optimización desconocido: " << status << std::endl;
            result.is_optimal = false;
        }

    } catch (GRBException& e) {
        std::cerr << "Error de Gurobi: " << e.getMessage() << std::endl;
        result.is_optimal = false;
    } catch (...) {
        std::cerr << "Error desconocido durante la optimización" << std::endl;
        result.is_optimal = false;
    }
    
    return result;
}




/**
 * @brief Convierte ExactSolution a vector de Bin
 * 
 * Transforma la representación interna de la solución (map de tamaños)
 * en objetos Bin con items completos, compatible con BinPackingUtils.
 * 
 * @param solution Solución en formato ExactSolution
 * @return Vector de bins reconstruido
 * 
 * @implementation
 * 1. Itera sobre el map de bins en la solución
 * 2. Para cada bin, crea un objeto Bin vacío
 * 3. Para cada tamaño en el bin, busca el item original correspondiente
 * 4. Añade el item al bin usando addItem()
 * 
 * @note Reconstruye items originales buscando por tamaño
 * 
 * @warning Si múltiples items tienen el mismo tamaño, puede asignar
 *          items diferentes a los originales (pero del mismo tamaño)
 * 
 * @example
 * ExactSolution sol = solver.solveExact();
 * std::vector<Bin> bins = solver.solutionToBins(sol);
 * BinPackingUtils::printSolution("Optimal", bins);
 */
std::vector<Bin> BinPackingSolver::solutionToBins(const ExactSolution& solution) const {
    std::vector<Bin> bins;
    
    // Permitir convertir soluciones no óptimas también
    if (solution.num_bins == 0 || solution.bins.empty()) {
        return bins;  // Solo retornar vacío si no hay bins
    }
    
    // Convertir de map<int, vector<int>> a vector<Bin>
    for (const auto& pair : solution.bins) {
        Bin bin(bin_capacity_);
        const std::vector<int>& item_sizes = pair.second;
        
        // Crear items temporales y añadirlos al bin
        for (int size : item_sizes) {
            // Buscar el item original con ese tamaño
            for (const auto& original_item : items_) {
                if (original_item.getSize() == size) {
                    bin.addItem(original_item);
                    break;
                }
            }
        }
        
        bins.push_back(bin);
    }
    
    return bins;
}









