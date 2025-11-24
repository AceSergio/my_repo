#include "string.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 ** @brief Crée une nouvelle structure string à partir d'un char * et d'une
 *taille.
 ** Alloue et copie le contenu.
 ** "The argument str will not be deallocated" [cite: 6221] -> nous devons
 *copier.
 **
 ** @param str La chaîne source (peut contenir des octets nuls).
 ** @param size La taille du contenu à copier.
 **
 ** @return La nouvelle structure string allouée, ou NULL en cas d'échec
 *d'allocation.
 */
struct string *string_create(const char *str, size_t size)
{
    struct string *new_str = malloc(sizeof(struct string));
    if (!new_str)
    {
        return NULL;
    }

    new_str->size = size;
    new_str->data = NULL; // Initialiser pour la sécurité

    if (size > 0)
    {
        // Allouer de la mémoire pour les données + 1 octet pour le terminateur
        // '\0' (bonne pratique)
        new_str->data = malloc(size + 1);
        if (!new_str->data)
        {
            free(new_str);
            return NULL;
        }

        // Copier les octets exacts, y compris les potentiels '\0'
        memcpy(new_str->data, str, size);

        // Terminateur de chaîne après la section de données, mais non inclus
        // dans 'size'
        new_str->data[size] = '\0';
    }
    else
    {
        new_str->data = NULL;
    }

    return new_str;
}

/*
 ** @brief Se comporte comme memcmp(3) pour comparer une structure string et un
 *char *
 ** jusqu'à 'n' octets.
 **
 ** @param str1 La structure string à comparer.
 ** @param str2 La chaîne C (char *) à comparer.
 ** @param n Le nombre maximal d'octets à comparer.
 **
 ** @return La valeur de memcmp (entier < 0, 0 ou > 0).
 */
int string_compare_n_str(const struct string *str1, const char *str2, size_t n)
{
    // Limiter la comparaison à la taille de la struct string ou à n
    size_t len1 = str1->size;
    size_t len_to_compare = len1 < n ? len1 : n;

    // Utiliser memcmp pour comparer les octets directement
    int res = memcmp(str1->data, str2, len_to_compare);

    if (res != 0)
    {
        return res;
    }

    // Si les n octets (ou moins si str1 est plus courte) correspondent:
    // 1. Si str1 est plus courte que n et que str2 ne termine pas là, str1 est
    // "inférieure".
    if (len1 < n && str2[len1] != 0)
    {
        return -1; // str1 se termine avant la fin de la zone de comparaison,
                   // str2 continue.
    }

    // 2. Dans tous les autres cas (taille égale ou str1 plus longue que n),
    // considérer comme égal. L'implémentation de strncmp renverrait un résultat
    // basé sur la différence entre les caractères à la position
    // `len_to_compare` s'il y avait une différence. Ici, on s'aligne sur
    // memcmp(3) pour la comparaison de blocs de données.
    return 0;
}

/*
 ** @brief Concatène un char* avec sa taille à la fin de la structure string.
 ** Réalloue la mémoire si nécessaire et copie les octets.
 ** "Be careful, to_concat may contain null bytes" [cite: 6227]
 **
 ** @param str La structure string de destination (modifiée).
 ** @param to_concat La chaîne source à ajouter.
 ** @param size La taille du contenu à ajouter.
 */
void string_concat_str(struct string *str, const char *to_concat, size_t size)
{
    // "If the new size is equal to 0 you must do nothing" [cite: 6228]
    if (!str || size == 0)
    {
        return;
    }

    size_t old_size = str->size;
    size_t new_size = old_size + size;

    // Réallouer la mémoire pour la nouvelle taille + 1 pour le caractère nul de
    // sécurité
    char *new_data = realloc(str->data, new_size + 1);
    if (!new_data)
    {
        // En cas d'échec de realloc, on ne modifie rien et on retourne.
        return;
    }

    // Copier les nouvelles données à la fin du buffer
    memcpy(new_data + old_size, to_concat, size);

    // Mettre à jour la structure
    str->data = new_data;
    str->size = new_size;

    // Mettre à jour le terminateur de chaîne de sécurité
    str->data[new_size] = '\0';
}

/*
 ** @brief Détruit la structure string et libère son contenu data.
 **
 ** @param str La structure string à détruire.
 */
void string_destroy(struct string *str)
{
    if (!str)
    {
        return;
    }
    // Libérer le buffer de données si il existe
    if (str->data)
    {
        free(str->data);
    }
    // Libérer la structure elle-même
    free(str);
}
