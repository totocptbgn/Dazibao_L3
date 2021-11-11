# 大字报

Repository  importé du GitLab de l'Université. Réalisé en Printemps 2020 avec [Dao Thauvin](https://github.com/daothauvin).

> Projet de Programmation Réseau
> Dao Thauvin & Thomas Bignon

Le sujet est présent [ici](docs/projet.pdf) et le rapport [là](docs/rapport.pdf).

## Compilation

Un `Makefile` est à votre disposition.
Les options de debug sont indiquées dans le Makefile et doivent être ajoutées à la variable `CFLAGS`.
 
## Exécution

Chaque script contient une option --help permettant de connaitre les différentes options d'éxécution.

### dazibao_node

```
./dazibao_node
```

Permet de lancer un noeud.
On peut notamment spécifier l'id du noeud, le port et le numero de sequence avec respectivement les options -i, -p, -s suivi de la valeur voulue.  
D'autres options sont disponibles et sont détaillées plus bas.

### dazibao_msg

```
./dazibao_msg {port} "message" {IP}
```

Permet de modifier le message du noeud présent sur le port `Port`, il faut au préalable que `.keys/public.pem` soit la bonne clé publique.  
Elle sera initialisé lors de la création d'un noeud et modifié à chaque fois que la valeur d'un noeud est changé.  
L'IP donnée doit correspondre à une IP de la machine hébergeant le noeud, 
si celui-ci n'est pas rempli il s’agira automatiquement de la machine où est exécutée le script.

### dazibao_zoo

```
./dazibao_zoo {Port}
```
Un petit code qui modifie régulièrement le message du noeud de cette machine sur le port `Port`.

### dazibao_read

```
./dazibao_read Port {ID}
```
Permet de récuperer les données que le noeud voit.  
L'éxécution de cette commande doit se faire au même emplacement que l'éxécution de la création du noeud. Le noeud doit se trouver sur le port `Port`.  
Si `ID` est spécifié, seul la valeur du noeud d'ID `ID` sera affiché, sinon toutes les valeurs sont affichées.

## Valeurs par défaut

* Port : 42069
* ID node : 666 (en big endian)
* Seqno : 0

## Prérequis

* `gcc`
* `make`
* `OpenSSL`

## Changer d'interfaces persistantes

A la suite de la commande dazibao_node,
il est possible de spécifier des interfaces suivi d'un port pour changer les adresses persistantes (une adresse suivi d'un port et ainsi de suite).  

## Devenir un serveur

Notre code recherche des adresses sur les interfaces données ou sur l'adresse http://jch.irif.fr/ si rien n'est spécifié, 
s'il ne trouve aucunes adresses, il s'arête.
Pour éviter cela, l'option -n de dazibao_node est nécessaire.

## Fork

L'option -f de dazibao_node permet de lancer l'application en arrière plan à l'aide d'un fork.
