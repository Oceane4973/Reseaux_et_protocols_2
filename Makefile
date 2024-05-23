CC = gcc
CFLAGS = -Wall -Wextra
SRCDIR = src
BUILDDIR = build

# Liste de tous les fichiers source dans le répertoire src
SRCS = $(wildcard $(SRCDIR)/*.c)

# Liste de tous les fichiers objet dans le répertoire build
OBJS = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SRCS))

# Règle de compilation pour chaque fichier source
$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Règle pour construire le programme main
main: $(BUILDDIR)/main.o $(BUILDDIR)/router.o $(BUILDDIR)/device.o 
	$(CC) $(CFLAGS) $^ -o main

# Règle pour construire le programme client
client: $(BUILDDIR)/client.o
	$(CC) $(CFLAGS) $^ -o client

# Règle pour nettoyer les fichiers objets
clean:
	rm -f $(BUILDDIR)/*.o main client

# Créer le dossier build s'il n'existe pas
$(shell mkdir -p $(BUILDDIR))

