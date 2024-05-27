
# Simulation de réseau avec adresses IP virtuelles et protocole RIP

Ce projet a pour objectif de simuler un réseau informatique en utilisant des adresses IP virtuelles sur des interfaces réseau spécifiques, tout en implémentant le protocole RIP (Routing Information Protocol) pour le routage des paquets. 

## Objectif

L'objectif principal de ce projet est de créer une simulation réaliste d'un réseau informatique utilisant des adresses IP virtuelles, en utilisant le protocole RIP pour le routage des paquets entre les routeurs et les appareils du réseau.

## Structure du projet

- `config.yaml` : Fichier de configuration YAML contenant les informations sur les routeurs, les appareils connectés et les adresses IP virtuelles à configurer.
- `scripts/deploy_virtual_ips.sh` : Script Bash pour configurer les adresses IP virtuelles.
- `src/` : Code principal en C permettant le déploiement des routeurs.
- `src/unitTest.c` : Script Bash pour exécuter des tests unitaires du code principal en C.
- `build/` : Dossier comportant les fichiers C compilés.
-  `Makefile` : Script de construction des fichiers C
- `deploy_environment.sh` : Script Bash pour créer l’environnement utile au bon fonctionnement du programme.

## Fonctionnement
Chaque adresse IP virtuelle sera associée à un Socket écoutant sur le port 8520. Pour partager les tables de routage, chaque interface du routeur disposera également d'un autre Socket écoutant sur l'adresse de diffusion (broadcast) du réseau auquel il est connecté. En résumé, deux stockes seront déployés par interface pour faciliter la communication et la gestion des routes dans le réseau simulé.

## Utilisation générale

1. Modifier le fichier `config.yaml` pour définir les configurations des routeurs, des appareils et des adresses IP virtuelles selon vos besoins. Le fichier yaml devra respecter la forme suivante :
```yaml
routers:
  - name: R1
        port: 8520
        devices:
          - interface: eth0
                ip: 127.0.0.1
                mask: 24
          - interface: eth1
                ip: 192.1.1.2
                mask: 24
  - name: R2
        port: 8520
        devices:
          - interface: eth0
                ip: 192.1.1.3
                mask: 24
          - interface: eth1
                ip: 192.1.2.3
                mask: 24
```
2. Exécuter le script `deploy_environment.sh` pour configurer les adresses IP virtuelles en utilisant les configurations spécifiées dans le fichier `config.yaml`.
```bash
$ chmod +x deploy_environment.sh
$ sudo ./deploy_environment.sh
```
3. Afin d'utiliser le fichier de configuration, le code utiliser la librairie yaml.h, assurer vous de l'avoir installé. Sinon, installer le à l'aide de la commande suivante :
```bash
$ sudo apt-get install libyaml-dev
```
4. Compiler le projet à l'aide du Makefile.
```bash
$ make main
```
4. Lancer le programme.
```bash
$ ./main
```
## Nettoyer le répertoire
Utiliser le fichier Makefile a votre disposition :
```bash
$ make clean
```
## Tests unitaires
Des tests unitaires ont été rédiger afin de s'assurer de la robustesse du programme. Pour lancer les tests suivez les étapes suivantes :
1. Afin d'utiliser le fichier de configuration, le code utiliser la librairie yaml.h, assurer vous de l'avoir installé. Sinon, installer le à l'aide de la commande suivante :
```bash
$ sudo apt-get install libyaml-dev
```
2. Compiler le projet à l'aide du Makefile.
```bash
$ make unitTest
```
3. Lancer le programme.
```bash
$ ./unitTest
```
## Auteur

Ce projet a été développé par Océane MONGES dans le cadre du cours "Réseaux et protocoles 2".
