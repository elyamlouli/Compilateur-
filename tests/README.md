
# Jeu de tests pour DECAF

Ce dépôt contient un jeu de tests pour un compilateur DECAF. Les deux
tiers des tests environ sont des tests de compilation uniquement
(analyses syntaxique et sémantique), le tiers restant étant constitué
de tests d'exécution. Il y a au total environ 150 tests : ils sont
tous très simples.

D'autres tests seront ajoutés dans les prochains jours (entre autres
dans la section `20.big`). Vous serez prévenus de toute modification
importante. Vos contributions sont bienvenues.

Si vous constatez une erreur dans les tests ou dans le script de
lancement, merci de me prévenir le plus rapidement possible.

Le code de lancement des tests est assez rudimentaire, et certainement
pas robuste. Vous trouverez dans ce document quelques directives pour
que ce jeu de tests soit utilisable.

## Obtenir le jeu de tests

La première étape est de récupérer ce dépôt dans votre répertoire de
travail, celui qui contient l'exécutable de votre compilateur :

```
cd /mon/répertoire/de/travail/projet
git clone ... # choisissez une des url proposées par gitlab
```

Cela doit avoir pour effet de créer un sous-répertoire `dts`.

## Lancer les tests

La seconde étape consiste à lancer les tests en utilisant un script
fourni, toujours dans votre répertoire de travail :

```
dts/run.sh
```

Cette commande :

- invoque votre compilateur sur chaque test, et vérifie que le
  résultat de la compilation est conforme à ce qui est attendu

- (pour les tests d'exécution uniquement) exécute le code MIPS produit
  et vérifie que le résultat de l'exécution est conforme à ce qui est
  attendu.

Le script `dts/run.sh` fait les hypothèses suivantes :

- le compilateur s'appelle `decaf` et se trouve dans le répertoire
  courant

- le compilateur est appelé par `./decaf fichier.decaf -o
  fichier.mips`

- le code MIPS est accepté par SPIM avec `spim -file fichier.mips`

Ce document explique plus bas comment utiliser le jeu de tests dans
des circonstances différentes.

## Consulter les fichiers produits

Les tests sont répartis dans différents répertoires. Chaque test est
un fichier `<nom>.decaf`. La compilation produit les fichiers
`<nom>.mips` et `<nom>.cerr` dans le même répertoire. L'exécution (si
elle a lieu) produit le fichier `<nom>.xout` ; c'est le contenu de ce
fichier qui sert à décider du succès ou de l'échec de l'exécution
(voir section suivante). Le script ne supprime aucun de ces fichiers.

Dans tous les cas, ces tests ne sont que de petits fichiers source
DECAF, que vous pouvez utiliser individuellement. Vous trouverez plus
bas des détails sur l'organisation du répertoire `dts/`.


# Succès et échec pour les tests

## Diagnostic de la compilation

Le script `run.sh` doit être capable de déterminer si une commande de
compilation a réussi ou échoué. Il utilise pour cela le code de retour
du programme/processus de compilation. Votre compilateur doit donc :

- se terminer par `exit (1)` si il y a une erreur de compilation (et
  donc pas de programme MIPS utilisable)

- se terminer par `exit (0)` si il produit un programme MIPS

Cela se fait typiquement dans votre programme principal, lors de
l'appel à `yyparse ()`, par exemple avec

```
...
if (yyparse() != 0) {
    ...
    exit (1); // ou exit (EXIT_FAILURE);
}
...
exit (0); // ou exit (EXIT_SUCCESS);
```

D'autre part, si vous détectez une erreur dans une action de la
grammaire, la macro `YYERROR` peut servir à terminer l'analyse
syntaxique et demander à `yyparse ()` de renvoyer une valeur non
nulle. Vous trouverez le détail dans le manuel de Bison.

Attention : certains tests de compilation doivent échouer, pour
vérifier que le compilateur fait toutes les vérification prescrites.
Il est donc crucial que votre compilateur renvoie un code de retour
cohérent.

## Diagnostic de l'exécution

Si la compilation se termine avec succès, elle produit un fichier
MIPS. Pour les tests d'exécution, ce fichier MIPS est passé au
simulateur `spim`, qui l'exécute. Pour juger de la correction de
l'exécution, les règles suivantes sont appliquées

- la sortie doit contenir des lignes de la forme `xxx=xxx` (avec un
  caractère de fin de ligne)

- les deux valeurs autour du signe `=` doivent être identiques,
  caractère par caractère

- en cas de vérification dynamique qui échoue, la sortie doit contenir
  une ligne qui commence 4 étoiles et une espace (`'**** '`) et peut
  contenir n'importe quel texte avant la fin de ligne

- si une vérification échoue, la ligne d'étoiles doit être la dernière
  affichée par votre programme

- il ne doit pas y avoir d'autre ligne dans la sortie du programme
  MIPS

Tous les tests produisent une sortie de cette forme, avec les
fonctions `WriteInt`, `WriteBool` et `WriteString` (la fonction
`ReadInt` n'est jamais utilisée). Il faut donc que votre compilateur
traduise correctement les appels de ces fonctions. La ligne signalant
l'échec d'une vérification dynamique doit être produite par votre
programme MIPS.

Cette partie est certainement la plus fragile du script. N'hésitez pas
à regarder les détails (dans la fonction `do_execute`), et à proposer
des améliorations.

Enfin, l'utilisation du simulateur `mars` n'est pas prévue. Si
quelqu'un a une expérience avec ce simulateur, les contributions sont
bienvenues.


# Arguments de `run.sh`

Le script `run.sh` respecte un certain nombre d'options, dont le
détail est expliqué ci-dessous.

## Nom et usage du compilateur

L'option `-c <chemin>` permet de modifier la commande de compilation
utilisée. Par exemple

```
dts/run.sh -c ../bin/tisane
```

utilisera des commandes de la forme `../bin/tisane fichier.decaf -o
fichier.mips`.

L'option `-s` indique que le compilateur utilise les canaux standard
(il lit le source DECAF sur l'entrée standard et produit un résultat
MIPS sur la sortie standard). Dans ce cas, les commandes de
compilation sont de la forme `./decaf < fichier.decaf > fichier.mips`.
On peut bien sûr la conjuguer avec l'option `-c` pour utiliser un
compilateur nommé autrement : dans notre exemple la commande

```
dts/run.sh -c ../bin/tisane -s
```

utilisera des commandes de la forme

```
../bin/tisane < fichier.decaf > fichier.mips
```

## Sélection des tests à exécuter

L'option `-e` arrête le script à la première erreur (de compilation ou
d'exécution).

L'option `-C` (C majuscule) ne teste pas l'exécution (mais évalue la
compilation des tests d'exécution). L'option `-X` (majuscule) réalise
uniquement les tests d'exécution.

La section suivante décrit des méthodes plus élaborées pour choisir
finement les tests à effectuer.

## Affichage

L'option `v <niveau>` permet de modifier l'affichage. Il y a quatre
niveaux

- le niveau 0 affiche uniquement les statistiques finales

- le niveau 1 affiche les tests qui échouent

- le niveau 2 ajoute des marqueurs des différentes sections

- le niveau 3 affiche une ligne pour chaque test, réussi ou non

Par défaut, le niveau 3 est utilisé.


# Structure du répertoire `dts`

Le répertoire `dts` a le contenu suivant

```
dts/
+-- 00.lexical/
+-- 01.program/
|   ...
+-- 19.dynamic/
+-- 20.big/
+-- dts.list
+-- README.md
+-- run.sh
```

Les 21 répertoires contiennent chacun plusieurs fichiers source DECAF.
Le script `run.sh` a été décrit plus haut. Le fichier `README.md` est
le document que vous êtes en train de lire.

La liste des tests se trouve dans le fichier `dts.list`. C'est cette
liste qui est utilisée par `run.sh`. Elle contient une ligne pour
chaque section (ces lignes sont purement indicatives) et une ligne
pour chaque test.

## Liste des tests

La ligne d'un test dans `dts.list` est de la forme

```
+ C0 ? TEST 00.lexical/Commentaire
```

Le premier caractère de la ligne (`+` ou `-`) indique si le test doit
être effectué.

Le second mot est composé de deux lettres : `C` ou `X` selon qu'il
s'agit d'un test de compilation ou d'exécution, et `0` ou `1` selon
que la compilation ou exécution doit réussir ou échouer (c'est le code
de retour du compilateur, ou du programme qui teste la sortie de
l'exécution). Pour les tests d'exécution, la compilation doit
implicitement réussir.

Le troisième mot (`?`) n'est pas utilisé ici, et la signification des
suivants est évidente.

## Utiliser une liste personnalisée

Il est possible d'utiliser une liste autre que celle de
`dts/dts.list`, avec l'option `-l <chemin>` de `dts/run.sh`. Par
exemple

```
cp dts/dts.list ./mydts.list
... (éditer le fichier mydts.list)
dts/run.sh -l mydts.list
```

utilisera le fichier indiqué au lieu de `dts/dts.list`.

On peut effectuer deux opérations sur une liste personnalisée

- remplacer le caractère `+` en tête de ligne par `-` : si la ligne
  est un test, il sera ignoré, et si la ligne est une section, tous
  les tests de la section seront ignorés

- supprimer des lignes : dans ce cas, les tests supprimés ne sont plus
  visibles pour le script, et ne seront évidemment pas effectués

Il est recommandé de ne jamais modifier la liste originale
(`dts/dts.list`), et si cela arrive d'utiliser `git` pour en récupérer
une version complète.
