m.cheminot.plugin
=================

Le plugin [cordova](https://cordova.apache.org/) pour l'application mobile [cheminot.m](https://github.com/cheminotorg/m.cheminot.m).

Son rôle est donc de faire la passerelle entre l'interface web et le code natif (cf. le projet [cheminot.c](https://github.com/cheminotorg/m.cheminot.c)).

L'interface javascript que propose ce plugin est la suivante: 

* `Cheminot.gitVersion`: Retourne la version git du code natif [cheminot.c](https://github.com/cheminotorg/m.cheminot.c).
* `Cheminot.init`: Monte en mémoire la base de données [cheminot.db](https://github.com/cheminotorg/m.cheminot.db). 
* `Cheminot.lookForBestDirectTrip`: Trouve le trajet direct le plus rapide.
* `Cheminot.lookForBestTrip`: Trouve le trajet le plus rapide en prenant en compte les changements.
* `Cheminot.abort`: Arrête la tâche `lookForBestTrip`.
* `Cheminot.trace`: Donne des informations relative au parcourt de l'algorithme `lookForBestTrip`.
* `Cheminot.getStop`: Retourne les informations d'un arrêt spécifique.
