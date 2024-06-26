
# Simulation de réseau avec adresses IP virtuelles et protocole RIP

Ce projet a pour objectif de simuler un réseau informatique en utilisant des adresses IP virtuelles sur des interfaces réseau spécifiques, tout en implémentant le protocole RIP (Routing Information Protocol) pour le routage des paquets.

## Objectif

L'objectif principal de ce projet est de créer une simulation réaliste d'un réseau informatique utilisant des adresses IP virtuelles, en utilisant le protocole RIP pour le routage des paquets entre les routeurs et les appareils du réseau.

## DEmonstration

Observez le comportement du code en visionnant la vidéo suivante :[Vidéo de démonstration](https://youtu.be/cL6CKh77HVs)

## Structure du projet

- `config/routers_config.yaml` : Fichier de configuration YAML contenant les informations sur les routeurs, les appareils connectés et les adresses IP virtuelles à configurer.
- `config/server_config.yaml` : Fichier de configuration YAML contenant les informations sur le serveur.
- `scripts/deploy_virtual_ips.sh` : Script Bash pour configurer les adresses IP virtuelles.
- `src/` : Code principal en C permettant le déploiement des routeurs.
- `src/unitTest.c` : Script C pour exécuter des tests unitaires du code principal en C.
- `src/main.c` : Script principal en C qui démare le serveurs, et les sockets (representant les interfaces des routers).
- `src/client.c` : Script C permettant de jouer le "client". Ce script est à éxécuter après le main.c.
- `build/` : Dossier comportant les fichiers C compilés.
- `Makefile` : Script de construction des fichiers C.
- `deploy_environment.sh` : Script Bash pour créer l’environnement utile au bon fonctionnement du programme.

## Fonctionnement

Chaque adresse IP virtuelle sera associée à un Socket écoutant sur le port 8520. Pour partager les tables de routage, chaque interface du routeur disposera également d'un autre Socket écoutant sur l'adresse de diffusion (broadcast) du réseau auquel il est connecté. En résumé, deux sockets seront déployés par interface pour faciliter la communication et la gestion des routes dans le réseau simulé.

De plus, un serveur est déployé, initialement configuré sur l'adresse 172.16.180.2 et accessible au port 8080.

## Utilisation générale

1. Modifiez les fichiers `config/server_config.yaml` et `config/routers_config.yaml`pour définir les configurations des routeurs, du serveurs et les adresses IP virtuelles selon vos besoins. Le fichier yaml doit respecter la forme suivante :

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
2. Exécutez le script `deploy_environment.sh` pour configurer les adresses IP virtuelles en utilisant les configurations spécifiées dans le fichier `config.yaml`.
```bash
$ chmod +x deploy_environment.sh
$ sudo ./deploy_environment.sh
```
3. Le code utilise la librairie yaml.h, assurez vous de l'avoir installé. Sinon, exécutez la commande suivante :
```bash
$ sudo apt-get install libyaml-dev
```
4. Vous pouvez changer la verbosité des logs, en modifiant la variable enable_logs dans le fichier src/enable_logs.c (Facultatif) :
```c
bool enable_logs = true;
```
5. Compilez le projet à l'aide du Makefile.
```bash
$ make main
```
6. Lancez le programme.
```bash
$ ./main
```
7. Désormais, nous pouvons nous concentréer sur le client. Vous pouvez d'ailleurs modifier le comportement du client, en commentant ou non, l'une des lignes suivantes : 
```c
int main() {
    //hello_world();
    send_tram();

    return 0;
}
```
8. Compiler le programme du client.
```bash
$ make client
```
9. Lancez le programme du client.
```bash
$ ./client
```
## Nettoyer le répertoire
Utiliser le fichier Makefile a votre disposition :
```bash
$ make clean
```
## Tests unitaires
Des tests unitaires ont été rédigés afin de s'assurer de la robustesse du programme. Pour lancer les tests suivez les étapes suivantes :
1. Le programme utilise la librairie yaml.h, assurez vous de l'avoir installé. Sinon, exécutez la commande suivante :
```bash
$ sudo apt-get install libyaml-dev
```
2. Compilez le projet à l'aide du Makefile.
```bash
$ make unitTest
```
3. Lancez le programme.
```bash
$ ./unitTest
```
## Auteur

Ce projet a été développé par Océane MONGES dans le cadre du cours "Réseaux et protocoles 2".
