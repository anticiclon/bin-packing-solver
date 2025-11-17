
#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <string>
#include <filesystem>

/**
 * @brief Clase para generar instancias de bin packing controladas
 * 
 * Genera diferentes tipos de instancias con características específicas
 * para probar el rendimiento de algoritmos de bin packing.
 */
class InstanceGenerator {
private:
    std::mt19937 rng;  ///< Generador de números aleatorios (Mersenne Twister)
    
public:
    /**
     * @brief Constructor con semilla configurable
     * 
     * @param seed Semilla para reproducibilidad (default=42)
     * 
     * @note Usar la misma semilla genera las mismas instancias
     */
    InstanceGenerator(unsigned int seed = 42) : rng(seed) {}
    
    /**
     * @brief Genera instancia con distribución uniforme
     * 
     * Tamaños de items distribuidos uniformemente en [min_ratio*capacity, max_ratio*capacity].
     * 
     * @param n_items Número de items a generar
     * @param capacity Capacidad de cada bin
     * @param min_ratio Ratio mínimo del tamaño (default=0.1 → 10% de capacidad)
     * @param max_ratio Ratio máximo del tamaño (default=0.9 → 90% de capacidad)
     * @param filename Nombre del archivo (auto-generado si vacío)
     * 
     * @complexity O(n) donde n = n_items
     * 
     * @note Instancias estándar, útiles para evaluar comportamiento promedio
     */
    void generateUniform(int n_items, int capacity, 
                        double min_ratio = 0.1, double max_ratio = 0.9,
                        const std::string& filename = "") {
        
        std::uniform_real_distribution<double> dist(min_ratio, max_ratio);
        
        std::vector<int> sizes;
        for (int i = 0; i < n_items; ++i) {
            int size = static_cast<int>(dist(rng) * capacity);
            if (size < 1) size = 1;
            if (size > capacity) size = capacity;
            sizes.push_back(size);
        }
        
        if (filename.empty()) {
            saveInstance(sizes, capacity, 
                        "instance_uniform_n" + std::to_string(n_items) + ".txt");
        } else {
            saveInstance(sizes, capacity, filename);
        }
    }
    
    /**
     * @brief Genera instancia con items grandes (50-90% de capacidad)
     * 
     * Items grandes son difíciles de combinar, desafiantes para heurísticas.
     * Tienden a dejar mucho espacio desperdiciado en cada bin.
     * 
     * @param n_items Número de items
     * @param capacity Capacidad de cada bin
     * @param filename Nombre del archivo (auto-generado si vacío)
     * 
     * @complexity O(n) donde n = n_items
     * 
     * @note Casos difíciles: pocas combinaciones válidas de items por bin
     */
    void generateLargeItems(int n_items, int capacity,
                           const std::string& filename = "") {
        
        std::uniform_real_distribution<double> dist(0.5, 0.9);
        
        std::vector<int> sizes;
        for (int i = 0; i < n_items; ++i) {
            int size = static_cast<int>(dist(rng) * capacity);
            sizes.push_back(size);
        }
        
        if (filename.empty()) {
            saveInstance(sizes, capacity,
                        "instance_large_n" + std::to_string(n_items) + ".txt");
        } else {
            saveInstance(sizes, capacity, filename);
        }
    }
    
    /**
     * @brief Genera instancia con items pequeños (10-30% de capacidad)
     * 
     * Items pequeños permiten muchas combinaciones, generalmente más
     * fáciles para heurísticas pero pueden crear fragmentación.
     * 
     * @param n_items Número de items
     * @param capacity Capacidad de cada bin
     * @param filename Nombre del archivo (auto-generado si vacío)
     * 
     * @complexity O(n) donde n = n_items
     * 
     * @note Típicamente mejor utilización de bins que con items grandes
     */
    void generateSmallItems(int n_items, int capacity,
                           const std::string& filename = "") {
        
        std::uniform_real_distribution<double> dist(0.1, 0.3);
        
        std::vector<int> sizes;
        for (int i = 0; i < n_items; ++i) {
            int size = static_cast<int>(dist(rng) * capacity);
            if (size < 1) size = 1;
            sizes.push_back(size);
        }
        
        if (filename.empty()) {
            saveInstance(sizes, capacity,
                        "instance_small_n" + std::to_string(n_items) + ".txt");
        } else {
            saveInstance(sizes, capacity, filename);
        }
    }
    
    /**
     * @brief Genera instancia trimodal (3 categorías de tamaños)
     * 
     * Distribución con tres modas:
     * - Pequeños: 10-20% de capacidad
     * - Medianos: 40-50% de capacidad
     * - Grandes: 70-80% de capacidad
     * 
     * Simula escenarios reales con categorías de productos.
     * 
     * @param n_items Número de items
     * @param capacity Capacidad de cada bin
     * @param filename Nombre del archivo (auto-generado si vacío)
     * 
     * @complexity O(n) donde n = n_items
     * 
     * @note Cada item se asigna aleatoriamente a una de las tres categorías
     */
    void generateTrimodal(int n_items, int capacity,
                         const std::string& filename = "") {
        
        std::uniform_int_distribution<int> mode_dist(0, 2);
        
        std::vector<int> sizes;
        for (int i = 0; i < n_items; ++i) {
            int mode = mode_dist(rng);
            int size;
            
            if (mode == 0) {
                // Items pequeños (10-20% de capacidad)
                size = static_cast<int>(capacity * (0.1 + 0.1 * (rng() % 100) / 100.0));
            } else if (mode == 1) {
                // Items medianos (40-50% de capacidad)
                size = static_cast<int>(capacity * (0.4 + 0.1 * (rng() % 100) / 100.0));
            } else {
                // Items grandes (70-80% de capacidad)
                size = static_cast<int>(capacity * (0.7 + 0.1 * (rng() % 100) / 100.0));
            }
            
            if (size < 1) size = 1;
            if (size > capacity) size = capacity;
            sizes.push_back(size);
        }
        
        if (filename.empty()) {
            saveInstance(sizes, capacity,
                        "instance_trimodal_n" + std::to_string(n_items) + ".txt");
        } else {
            saveInstance(sizes, capacity, filename);
        }
    }
    
    /**
     * @brief Genera instancia perfectamente empaquetable
     * 
     * Crea items que suman exactamente n_bins * capacity, garantizando
     * que existe una solución óptima con exactamente n_bins bins llenos.
     * 
     * Útil para verificar que el algoritmo exacto encuentra el óptimo.
     * 
     * @param n_bins Número de bins óptimo deseado
     * @param capacity Capacidad de cada bin
     * @param filename Nombre del archivo (auto-generado si vacío)
     * 
     * @complexity O(n*capacity) donde n = n_bins
     * 
     * @note Los items se mezclan (shuffle) para evitar ordenamiento trivial
     * @note Solución óptima conocida: exactamente n_bins bins al 100%
     * 
     * @example
     * generatePerfectPacking(10, 100);
     * // Genera items que llenan exactamente 10 bins (suma = 1000)
     */
    void generatePerfectPacking(int n_bins, int capacity,
                               const std::string& filename = "") {
        
        std::vector<int> sizes;
        
        // Llenar cada bin con items que sumen exactamente capacity
        for (int bin = 0; bin < n_bins; ++bin) {
            int remaining = capacity;
            
            // Llenar este bin con items que sumen exactamente capacity
            while (remaining > 0) {
                int size;
                if (remaining <= 5) {
                    size = remaining;  // Completar exactamente
                } else {
                    std::uniform_int_distribution<int> dist(1, std::min(remaining, capacity/3));
                    size = dist(rng);
                }
                sizes.push_back(size);
                remaining -= size;
            }
        }
        
        // Mezclar los items para evitar ordenamiento obvio
        std::shuffle(sizes.begin(), sizes.end(), rng);
        
        if (filename.empty()) {
            saveInstance(sizes, capacity,
                        "instance_perfect_b" + std::to_string(n_bins) + ".txt");
        } else {
            saveInstance(sizes, capacity, filename);
        }
    }
    
private:
    /**
     * @brief Guarda una instancia en formato de archivo
     * 
     * Formato del archivo:
     * - Línea 1: n_items capacity
     * - Línea 2: size_1 size_2 ... size_n (separados por espacios)
     * 
     * @param sizes Vector de tamaños de items
     * @param capacity Capacidad de cada bin
     * @param filename Nombre del archivo (relativo a ../instances/)
     * 
     * @implementation
     * - Crea directorio ../instances/ si no existe
     * - Guarda en formato texto plano compatible con loadInstanceFromFile()
     * 
     * @complexity O(n) donde n = tamaño del vector sizes
     */
    void saveInstance(const std::vector<int>& sizes, int capacity, 
                     const std::string& filename) {
        
        // Crear carpeta instances si no existe
        std::filesystem::create_directories("../instances");
        
        std::string full_path = "../instances/" + filename;
        std::ofstream file(full_path);
        
        if (!file.is_open()) {
            std::cerr << "Error: No se pudo crear " << full_path << std::endl;
            return;
        }
        
        // Formato: n_items capacity
        file << sizes.size() << " " << capacity << "\n";
        
        // Items en una línea separados por espacios
        for (size_t i = 0; i < sizes.size(); ++i) {
            file << sizes[i];
            if (i < sizes.size() - 1) file << " ";
        }
        file << "\n";
        
        file.close();
        std::cout << "✓ Creada: " << full_path 
                  << " (" << sizes.size() << " items, cap=" << capacity << ")\n";
    }
};

/**
 * @brief Programa principal - genera conjunto de instancias de prueba
 * 
 * Genera un conjunto variado de instancias para experimentación:
 * - Pequeñas (20 items): verificación rápida
 * - Medianas (50 items): casos típicos
 * - Grandes (100 items): casos desafiantes
 * - Muy grandes (200-500 items): límites del solver exacto
 * 
 * Todas las instancias se guardan en ../instances/
 * 
 * @return 0 si la generación fue exitosa
 * 
 * @note Usa semilla fija (12345) para reproducibilidad
 */
int main() {
    std::cout << "=== Generador de Instancias de Bin Packing ===\n\n";
    
    InstanceGenerator gen(12345); // Seed para reproducibilidad
    
    int capacity = 100;
    
    std::cout << "Generando instancias con capacidad = " << capacity << "\n\n";
    
    // Instancias pequeñas (para verificar correctitud)
    gen.generateUniform(20, capacity);
    gen.generateLargeItems(20, capacity);
    gen.generateSmallItems(20, capacity);
    
    // Instancias medianas (casos típicos)
    gen.generateUniform(50, capacity);
    gen.generateTrimodal(50, capacity);
    gen.generatePerfectPacking(10, capacity);  // Óptimo conocido: 10 bins
    
    // Instancias grandes (desafiantes para heurísticas)
    gen.generateUniform(100, capacity);
    gen.generateLargeItems(100, capacity);
    gen.generateTrimodal(100, capacity);
    
    // Instancias muy grandes (límites de Gurobi con time limit)
    gen.generateUniform(200, capacity);
    gen.generateUniform(500, capacity);
    
    std::cout << "\n¡Instancias generadas en la carpeta ../instances/!\n";
    
    return 0;
}