#!/bin/bash

# Vérifier qu'au moins un fichier .c est fourni
if [ $# -eq 0 ]; then
    echo "Usage: $0 fichier1.c fichier2.c ..."
    exit 1
fi

# Créer les fichiers headers pour chaque .c
for file in "$@"; do
    # Vérifier que le fichier a l'extension .c
    if [[ ! "$file" =~ \.c$ ]]; then
        echo "Erreur: $file n'est pas un fichier .c"
        continue
    fi
    
    # Extraire le nom de base sans extension
    basename="${file%.c}"
    header="${basename}.h"
    
    # Créer le header s'il n'existe pas
    if [ ! -f "$header" ]; then
        # Convertir le nom en majuscules pour le define
        guard=$(echo "${basename}_H" | tr '[:lower:]' '[:upper:]' | tr '-' '_')
        
        cat > "$header" << EOF
#ifndef ${guard}
#define ${guard}

/* À compléter */

#endif /* ! ${guard} */
EOF
        echo "Header créé: $header"
    else
        echo "Header existe déjà: $header"
    fi
done

# Créer main.c s'il n'existe pas
if [ ! -f "main.c" ]; then
    cat > "main.c" << 'EOF'
#include <stdio.h>

int main(void)
{
    printf("Hello, World!\n");
    return 0;
}
EOF
    echo "Fichier main.c créé"
else
    echo "main.c existe déjà"
fi

# Générer la liste des fichiers sources et objets
sources="main.c"
objects="main.o"

for file in "$@"; do
    if [[ "$file" =~ \.c$ ]]; then
        sources="$sources $file"
        objects="$objects ${file%.c}.o"
    fi
done

# Créer le Makefile
cat > "Makefile" << EOF
CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99
TARGET = program

SOURCES = $sources
OBJECTS = $objects

all: \$(TARGET)

\$(TARGET): \$(OBJECTS)
	\$(CC) \$(CFLAGS) -o \$(TARGET) \$(OBJECTS)

%.o: %.c
	\$(CC) \$(CFLAGS) -c \$< -o \$@

test: all
	./\$(TARGET)

clean:
	rm -f \$(OBJECTS) \$(TARGET)

.PHONY: all test clean
EOF

echo "Makefile créé avec succès!"
echo ""
echo "Utilisation:"
echo "  make       - Compile le programme"
echo "  make test  - Compile et exécute le programme"
echo "  make clean - Supprime les fichiers binaires"