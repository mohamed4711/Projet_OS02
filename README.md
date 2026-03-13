# Projet_OS02

## Objectif

Le but de cette étape était d'améliorer le temps de calcul de la simulation de fourmis
en gardant une approche minimale dans le code:

- chaque processus MPI possède tout l'environnement
- chaque processus ne pilote qu'une partie des fourmis
- les phéromones sont synchronisés entre processus par un `max`
- la phase d'évaporation est répartie entre les processus
- on garde autant que possible les chronomètres déjà présents
- on évite de réorganiser fortement le code existant

Le travail a été fait sur la branche `MPI_parallelisation`, en gardant la base de la
branche `vectorisation`.

## Ce qui a été changé

Les changements ont été limités à trois fichiers:

- `src/ant_simu.cpp`
- `src/pheronome.hpp`
- `src/Makefile`

### 1. `src/ant_simu.cpp`

Ajouts principaux:

- initialisation/finalisation MPI
- découpage du nombre total de fourmis entre les ranks
- chaque rank construit son propre `fractal_land` et son propre `pheronome`
- chaque rank avance seulement ses fourmis locales
- après l'avancement local, fusion globale des phéromones avec:

```cpp
MPI_Allreduce(..., MPI_MAX, MPI_COMM_WORLD);
```

- l'évaporation est faite seulement sur une partie des lignes de la carte par chaque rank
- les lignes évaporées sont rassemblées avec `MPI_Allgatherv`
- seule la fenêtre du rank 0 est affichée
- les positions de toutes les fourmis sont récupérées sur le rank 0 avec `MPI_Gatherv`
- les compteurs de nourriture sont combinés avec `MPI_Allreduce`

### 2. `src/pheronome.hpp`

Ajouts minimaux nécessaires pour MPI:

- évaporation sur une sous-plage de lignes
- copie de `m_map_of_pheronome` vers `m_buffer_pheronome` avant chaque itération
- accès brut au buffer sous forme de tableau de `double` pour MPI
- petits accesseurs `stride()` et `double_size()`

### 3. `src/Makefile`

Le build a été adapté pour compiler et lier avec `mpic++` au lieu de `g++`, sans
changer le reste du Makefile.

## Commandes d'exécution

Depuis la racine du projet:

```bash
make -C src clean
make -C src ant_simu.exe
```

Depuis `src/`:

```bash
OMP_NUM_THREADS=1 /usr/bin/mpirun.openmpi --bind-to core --map-by core -np 4 ./ant_simu.exe
```

Le `OMP_NUM_THREADS=1` est important pour garder un coeur par processus MPI pendant
les mesures.

## Résultats utilisés

### Référence vectorisée avant MPI

Mesure fournie pour `nb_fourmi = 50000`:

- `temps advanc_time = 0.0484853`
- `temps vaporisation = 0.000442545`
- `temps calcule vapo+advancing = 0.0489386`
- `time per affichage = 0.0217881`

### Version MPI obtenue

Mesure fournie pour `-np 4`:

- `temps advanc_time = 0.013531`
- `temps vaporisation = 0.0177877`
- `temps calcule vapo+advancing = 0.0342786`
- `time per affichage = 0.0259996`

## Analyse

### Ce qui s'améliore

Sur la partie calcul principale:

- `advanc_time` passe de `0.0484853` à `0.013531`
- gain d'environ `72.1 %`

Sur le chrono principal de calcul:

- `temps calcule vapo+advancing` passe de `0.0489386` à `0.0342786`
- gain d'environ `30.0 %`

Si on approxime le temps de boucle par:

```text
temps total ~ temps calcule vapo+advancing + time per affichage
```

alors:

- ancien total ~ `0.0707267`
- nouveau total ~ `0.0602782`
- gain global approximatif ~ `14.8 %`

### Ce qui ne s'améliore pas

L'affichage devient un peu plus coûteux:

- `time per affichage` passe de `0.0217881` à `0.0259996`
- surcoût d'environ `19.3 %`

Cela est logique, car le rank 0 doit maintenant:

- récupérer les positions des fourmis des autres ranks
- afficher une carte de phéromones synchronisée

### Attention sur `temps vaporisation`

Ce chrono n'est plus strictement comparable avec l'ancienne version.

Avant MPI, il mesurait seulement l'évaporation locale.

Avec la version MPI minimale, il couvre en pratique:

- la synchronisation globale des phéromones
- l'évaporation répartie
- le rassemblement des morceaux évaporés

Donc la forte hausse de `temps vaporisation` ne veut pas dire que l'évaporation seule
est devenue plus lente: elle inclut maintenant une partie importante du coût MPI.

## Conclusion

Oui, la parallélisation MPI minimale améliore bien le temps de calcul.

Conclusion mesurée avec les résultats fournis:

- amélioration nette sur le calcul pur
- amélioration modérée sur le temps total de boucle
- pas d'amélioration sur l'affichage
- le coût de communication MPI commence déjà à se voir

En résumé:

- pour `nb_fourmi = 50000`, le calcul est plus rapide avec `4` processus
- le gain existe, mais il n'est pas massif parce qu'on échange beaucoup de données
- l'idée "chaque processus a tout l'environnement" fonctionne, mais le coût de
  synchronisation limite l'accélération

## Suite naturelle

Les améliorations les plus simples à faire ensuite seraient:

- ajouter un chrono MPI séparé pour isoler proprement le coût des communications
- ajouter un mode sans affichage pour mesurer uniquement le calcul
- comparer `np = 1, 2, 4, 8` avec exactement la même version du code
