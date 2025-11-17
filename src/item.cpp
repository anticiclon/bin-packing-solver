#include "item.h"


/**
 * @brief Implementación del constructor de Item
 * 
 * @param id Identificador único del item
 * @param size Tamaño del item
 * 
 * @note Usa lista de inicialización para eficiencia
 * @note No realiza validación de parámetros (id y size deberían ser > 0)
 * 
 * @implementation
 * Formato de lista de inicialización:
 * - id_(id): Inicializa id_ con el valor del parámetro id
 * - size_(size): Inicializa size_ con el valor del parámetro size
 * - Cuerpo vacío: No se requiere lógica adicional
 */
Item::Item(int id, int size) 
    : id_(id)
    , size_(size)
{
}