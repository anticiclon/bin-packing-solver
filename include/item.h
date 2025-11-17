#pragma once

/**
 * @brief Representa un item individual para bin packing
 * 
 * Cada item tiene un identificador único y un tamaño que
 * debe caber dentro de la capacidad de un bin.
 */
class Item {
private:
    int id_;      ///< Identificador único del item
    int size_;    ///< Tamaño del item

public:
    /**
     * @brief Constructor parametrizado
     * 
     * @param id Identificador único (debe ser positivo)
     * @param size Tamaño del item (debe ser positivo)
     * 
     * @warning No valida que id y size sean positivos
     */
    Item(int id, int size);
    
    /**
     * @brief Obtiene el ID del item
     * @return Identificador del item
     */
    int getId() const { return id_; }

    /**
     * @brief Obtiene el tamaño del item
     * @return Tamaño del item
     */
    int getSize() const { return size_; }
    
    /**
     * @brief Operador de comparación menor que
     * @param other Item a comparar
     * @return true si este item es más pequeño que other, false en caso contrario
     */
    bool operator<(const Item& other) const {
        return size_ < other.size_;
    }

    /**
     * @brief Operador de comparación mayor que
     * @param other Item a comparar
     * @return true si este item es más grande que other, false en caso contrario
     */
    bool operator>(const Item& other) const {
        return size_ > other.size_;
    }
};