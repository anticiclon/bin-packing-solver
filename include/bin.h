#pragma once
#include "item.h"
#include <vector>

/**
 * @brief Representa un contenedor (bin) que almacena items
 * Un bin tiene una capacidad fija y mantiene una lista de items
 * asignados. Provee métodos para verificar si un item cabe y
 * para añadir items respetando la capacidad.
 */
class Bin {
private:
    int capacity_;                 ///< Capacidad máxima del bin
    int used_;                     ///< Carga actual (suma de tamaños)
    std::vector<Item> items_;      ///< Items almacenados en el bin

public:
    /**
     * @brief Constructor con capacidad
     * 
     * @param capacity Capacidad máxima del bin (debe ser > 0)
     */
    Bin(int capacity);
    
    /**
     * @brief Añade un item al bin
     * @param item Item a añadir
     * @return true si se añadió exitosamente, false si no cabía
     * @warning No fuerza la inserción si el item no cabe
     */
    bool addItem(const Item& item);
    
    /**
     * @brief Obtiene la capacidad máxima del bin
     * @return Capacidad total del bin (valor fijo establecido en construcción)
     * @note La capacidad nunca cambia después de la construcción
     */
    int getCapacity() const { return capacity_; }

    /**
     * @brief Obtiene el espacio usado actual
     * @return Suma de los tamaños de todos los items en el bin
     * @invariant Siempre cumple: 0 <= used_ <= capacity_
     */
    int getUsed() const { return used_; }

    /**
     * @brief Obtiene el espacio restante disponible
     * 
     * Calcula la diferencia entre la capacidad total y el espacio usado.
     * Un item de tamaño <= getRemaining() puede ser añadido al bin.
     * 
     * @return Espacio disponible en el bin
     * 
     * @complexity O(1)
     * 
     * @note Equivalente a: capacity_ - used_
     * 
     * @example
     * Bin bin(100);
     * bin.addItem(Item(1, 30));
     * std::cout << bin.getRemaining() << std::endl;  // Imprime: 70
     * 
     * Item item(2, 60);
     * if (item.getSize() <= bin.getRemaining()) {
     *     bin.addItem(item);  // Esto funcionará
     * }
     */
    int getRemaining() const { return capacity_ - used_; }
    const std::vector<Item>& getItems() const { return items_; }
    
    /**
     * @brief Obtiene la lista de items en el bin
     * 
     * Retorna una referencia constante al vector interno de items.
     * No permite modificación de los items.
     * 
     * @return Vector constante de items almacenados

     * @note La referencia es constante: no se pueden añadir/eliminar items
     *       directamente. Use addItem() para añadir items.
     */
    void print() const;
};