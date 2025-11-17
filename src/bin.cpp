#include "bin.h"
#include <iostream>


/**
 * @brief Implementación del constructor de Bin
 * 
 * Inicializa el bin con la capacidad especificada y estado vacío.
 * Usa lista de inicialización para eficiencia.
 * 
 * @param capacity Capacidad máxima del bin
 * 
 * @implementation
 * - capacity_: Se inicializa con el parámetro recibido
 * - used_: Se inicializa en 0 (bin vacío)
 * - items_: Vector vacío por defecto (no necesita inicialización explícita)
 * 
 */
Bin::Bin(int capacity) 
    : capacity_(capacity)
    , used_(0)
{
}


/**
 * @brief Implementación de addItem
 * 
 * Algoritmo de inserción con verificación de capacidad:
 * 
 * 1. Verifica si el item cabe comparando su tamaño con el espacio restante
 * 2. Si cabe:
 *    a. Añade el item al vector items_ (O(1) amortizado)
 *    b. Incrementa used_ por el tamaño del item
 *    c. Retorna true indicando éxito
 * 3. Si no cabe:
 *    - No modifica el bin (operación segura)
 *    - Retorna false indicando fallo
 * 
 * @param item Item a intentar añadir
 * @return true si se añadió, false si no cabía
 * 
 * @implementation
 * - Usa getRemaining() en lugar de (capacity_ - used_) para claridad
 * - push_back tiene complejidad O(1) amortizada (puede realocar el vector)
 * - No usa excepciones: retorna false en lugar de lanzar error
 * 
 * @invariant Si retorna true: used_ aumenta exactamente en item.getSize()
 * @invariant Si retorna false: el bin no se modifica en absoluto
 * 
 * @note Método seguro: nunca viola la capacidad del bin
 */
bool Bin::addItem(const Item& item) {
    // 1. Verificar si cabe
    if (item.getSize() <= getRemaining()) {
        // 2. Añadir a items_
        items_.push_back(item);
        
        // 3. Actualizar used_
        used_ += item.getSize();
        
        // 4. Retornar true
        return true;
    }
    
    // 3. No cabe, retornar false
    return false;
}



/**
 * @brief Implementación de print
 * 
 * Imprime el bin en formato: "Bin: [id:size, id:size, ...] = used/capacity"
 * 
 * Formato de salida:
 * - Lista de items entre corchetes []
 * - Cada item como "id:size"
 * - Items separados por ", " (coma y espacio)
 * - Resumen: "used/capacity" al final
 * - Nueva línea al terminar
 * 
 * Ejemplos de salida:
 * - Bin vacío: "Bin: [] = 0/100"
 * - Con items: "Bin: [1:30, 2:50, 3:20] = 100/100"
 * 
 * @implementation
 * - Usa size_t para el índice del bucle (compatibilidad con vector::size())
 * - Evita coma después del último item con condicional
 * - Usa std::endl para flush del buffer de salida
 * 
 * @see BinPackingUtils::printSolution() para imprimir múltiples bins
 */
void Bin::print() const {
    // Formato: "Bin: [item_id:size, item_id:size] = used/capacity"
    std::cout << "Bin: [";
    
    // Iterar sobre todos los items
    for (size_t i = 0; i < items_.size(); ++i) {
        // Imprimir item en formato "id:size"
        std::cout << items_[i].getId() << ":" << items_[i].getSize();
        // Añadir coma solo si no es el último item
        if (i != items_.size() - 1) std::cout << ", ";
    }

    // Cerrar corchetes e imprimir resumen
    std::cout << "] = " << used_ << "/" << capacity_ << std::endl;
}