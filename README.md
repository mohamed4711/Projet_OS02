## Organisation du projet

Le dépôt a été structuré par branches pour séparer les différentes étapes :

- `main` : version de référence du projet, basée sur la représentation orientée objet des fourmis. Elle sert de point de comparaison pour les mesures de base.
- `vectorisation` : version où les données des fourmis ont été réorganisées en tableaux séparés (`positions`, `graines`, `états`) afin d'améliorer la localité mémoire et de préparer les parallélisations suivantes.
- `OpenMP_parallelisation` : version construite à partir de `vectorisation`, avec parallélisation en mémoire partagée des boucles pertinentes à l'aide d'OpenMP.
- `MPI_parallelisation` : version construite à partir de `vectorisation`, avec une première stratégie de parallélisation distribuée où chaque processus possède l'environnement complet et ne gère qu'une partie des fourmis.

Le présent rapport synthétise les mesures obtenues sur ces différentes branches.

## Périmètre

Ce rapport suit les points demandés  :

- mesure du temps passé dans les différentes parties du code
- comparaison entre la version de référence, la version vectorisée et la version OpenMP
- tableau d'accélération OpenMP en fonction du nombre de threads
- mesure de la première stratégie MPI en fonction du nombre de processus
- description d'une stratégie pour la seconde façon de paralléliser le code

## Configuration de test

- Machine : AMD Ryzen 5 4600H
- Coeurs physiques : 6
- Threads logiques : 12
- Système vu par le programme : `nproc = 12`
- Build : `env -u DEBUG make clean all`
- Exécutions locales automatisées avec `SDL_VIDEODRIVER=dummy`

Commandes utilisées :

```bash
cd src
env -u DEBUG make clean all
env SDL_VIDEODRIVER=dummy OMP_NUM_THREADS=<N> ./ant_simu.exe
env SDL_VIDEODRIVER=dummy OMP_NUM_THREADS=1 /usr/bin/mpirun.openmpi --bind-to core --map-by core -np <P> ./ant_simu.exe
```

Remarque importante:

- Les temps affichés par le programme sont des moyennes par itération jusqu'à l'arrivée de la première nourriture au nid.

## Résumé des mesures

| Version | Branche | Nb fourmis | Itération 1re nourriture | Init fractal (s) | Init fourmis (s) | `advanc_time` (s) | `vaporisation` (s) | `calcule vapo+advancing` (s) | `affichage` (s) |
|---|---|---:|---:|---:|---:|---:|---:|---:|---:|
| Référence objet | `main`, 1 thread | 50000 | 1203 | 0.076905 | 0.007549 | 0.035403 | 0.000455 | 0.035869 | 0.019968 |
| Vectorisée | `vectorisation` | 50000 | 807 | 0.077880 | 0.008150 | 0.045164 | 0.000512 | 0.045696 | 0.020977 |
| OpenMP meilleur cas | `OpenMP_parallelisation`, 6 threads | 50000 | 807 | 0.075663 | 0.008488 | 0.016413 | 0.000237 | 0.016667 | 0.022787 |
| MPI meilleur calcul | `MPI_parallelisation`, 4 processus | 50000 | 601 | 0.073693 | 0.015492 | 0.013248 | 0.015656 | 0.031214 | 0.031727 |

Le temps total de boucle peut être approximé par :

```text
temps total ≈ temps calcule vapo+advancing + time per affichage
```

Avec cette approximation :

- `main` : `0.035869 + 0.019968 = 0.055837 s`
- `vectorisation` : `0.045696 + 0.020977 = 0.066673 s`
- `OpenMP 6 threads` : `0.016667 + 0.022787 = 0.039454 s`
- `MPI 4 processus` : `0.031214 + 0.031727 = 0.062941 s`

## Vectorisation

La vectorisation demandée par l'énoncé a bien été réalisée dans la branche `vectorisation` en remplaçant la structure orientée objet par des tableaux séparés :

- positions des fourmis
- graines pseudo-aléatoires
- états chargée / non chargée

Cette organisation améliore la localité mémoire et prépare naturellement la parallélisation OpenMP.

La comparaison est maintenant faite sur le même cas `50000` fourmis pour `main` et `vectorisation`.

Résultat mesuré sur cette implémentation :

- temps de calcul : `0.035869 s` -> `0.045696 s`
- rapport calcul `main / vectorisation` : `0.78`
- temps total approx : `0.055837 s` -> `0.066673 s`
- rapport total `main / vectorisation` : `0.84`

Autrement dit, sur ces mesures, la version vectorisée seule n'améliore pas les performances par rapport à la version objet de référence exécutée avec le même nombre de fourmis.

Malgré cela, cette transformation reste utile comme base de travail pour les branches `OpenMP_parallelisation` et `MPI_parallelisation`, qui partent toutes deux de `vectorisation`.

## OpenMP

### Boucles parallélisées

Les boucles parallélisées sont celles qui apportent un gain net sans modifier lourdement le code :

- boucle d'avancement des fourmis
- boucle d'évaporation des phéromones

Le rendu SDL n'a pas été parallélisé, car il ne s'y prête pas bien et il devient rapidement le nouveau goulot d'étranglement.

### Résultats OpenMP

Référence utilisée pour l'accélération :

- `OpenMP_parallelisation` avec `OMP_NUM_THREADS=1`
- `T1 = 0.0442733 s` pour `calcule vapo+advancing`
- `T1_total = 0.0651167 s` pour `calcul + affichage`

| Threads | `advanc_time` (s) | `vaporisation` (s) | `calcule vapo+advancing` (s) | `affichage` (s) | Total approx (s) | Accélération calcul |
|---:|---:|---:|---:|---:|---:|---:|
| 1 | 0.043779 | 0.000479 | 0.044273 | 0.020843 | 0.065117 | 1.00 |
| 2 | 0.029479 | 0.000352 | 0.030519 | 0.020546 | 0.051065 | 1.45 |
| 4 | 0.019158 | 0.000249 | 0.019420 | 0.020577 | 0.039997 | 2.28 |
| 6 | 0.016413 | 0.000237 | 0.016667 | 0.022787 | 0.039454 | 2.66 |
| 12 | 0.023283 | 0.002519 | 0.025840 | 0.025048 | 0.050888 | 1.71 |

Accélération totale approximative sur `calcul + affichage` :

| Threads | Accélération totale approx |
|---:|---:|
| 1 | 1.00 |
| 2 | 1.28 |
| 4 | 1.63 |
| 6 | 1.65 |
| 12 | 1.28 |

### Analyse OpenMP

Observations principales :

- Le meilleur point mesuré est `6 threads`, ce qui correspond exactement au nombre de coeurs physiques de la machine.
- Le calcul seul gagne environ `2.66x` entre `1` et `6` threads.
- Le gain total est plus faible, environ `1.65x`, car l'affichage SDL reste séquentiel et devient dominant.
- Le passage à `12 threads` dégrade les performances par rapport à `6 threads`.

Interprétation :

- `12` threads utilisent aussi les coeurs logiques SMT, pas uniquement les `6` coeurs physiques.
- Le calcul des fourmis est irrégulier et dépend du terrain ainsi que du tirage aléatoire.
- Une partie du travail reste séquentielle dans l'implémentation OpenMP actuelle, en particulier l'application finale du marquage des phéromones.

Comparaison directe avec la branche `vectorisation` sur le même cas `50000` :

- temps de calcul : `0.045696 s` -> `0.016667 s`
- accélération calcul : `2.74x`
- temps total approx : `0.066673 s` -> `0.039454 s`
- accélération totale approx : `1.69x`

Comparaison avec la référence objet `main` homogène à `50000` fourmis :

- temps de calcul : `0.035869 s` -> `0.016667 s`
- accélération calcul : `2.15x`
- temps total approx : `0.055837 s` -> `0.039454 s`
- accélération totale approx : `1.42x`

Conclusion OpenMP :

- l'amélioration est nette sur la partie calcul
- le meilleur compromis observé ici est `6 threads`
- au-delà, l'affichage et les coûts résiduels limitent le gain global

## MPI : première façon

### Principe implémenté

La branche `MPI_parallelisation` suit la première stratégie suivante  :

- chaque processus possède l'environnement complet
- les fourmis sont réparties entre les processus
- les phéromones sont fusionnés globalement par `max`
- l'évaporation est découpée entre processus
- le rang `0` rassemble les positions nécessaires à l'affichage

### Résultats MPI

Référence utilisée pour l'accélération :

- `MPI_parallelisation` avec `1` processus
- `T1 = 0.0425439 s` pour `calcule vapo+advancing`
- `T1_total = 0.0651285 s` pour `calcul + affichage`

| Processus | `advanc_time` (s) | `vaporisation` (s) | `calcule vapo+advancing` (s) | `affichage` (s) | Total approx (s) | Accélération calcul | Accélération totale approx |
|---:|---:|---:|---:|---:|---:|---:|---:|
| 1 | 0.039033 | 0.002783 | 0.042544 | 0.022585 | 0.065129 | 1.00 | 1.00 |
| 2 | 0.022038 | 0.007996 | 0.031237 | 0.027405 | 0.058642 | 1.36 | 1.11 |
| 4 | 0.013248 | 0.015656 | 0.031214 | 0.031727 | 0.062941 | 1.36 | 1.03 |

### Analyse MPI

Observations :

- `advanc_time` baisse fortement quand on augmente le nombre de processus.
- En revanche, `vaporisation` augmente fortement avec `2` puis `4` processus.
- Le temps de calcul global plafonne entre `2` et `4` processus.
- Le temps d'affichage augmente aussi, car le rang `0` doit rassembler plus d'informations.

Interprétation :

- le coût MPI n'est pas isolé par un chrono dédié
- le chrono `vaporisation` inclut en pratique une part importante de synchronisation et de rassemblement
- la stratégie "chaque processus a tout l'environnement" simplifie l'algorithme mais augmente fortement les communications

Conclusion MPI sur cette première stratégie :

- il existe un gain mesurable à `2` processus
- ce gain devient faible sur le temps total à `4` processus
- le coût de communication limite déjà l'accélération

Comparaison avec la référence objet `main` homogène à `50000` fourmis :

- temps de calcul : `0.035869 s` -> `0.031214 s`
- accélération calcul : `1.15x`
- temps total approx : `0.055837 s` -> `0.062941 s`
- le temps total de boucle reste moins bon que la référence à cause des communications et du surcoût d'affichage

## Seconde façon : stratégie proposée seulement

Le bonus d'implémentation n'est pas réalisé, mais voici une stratégie cohérente pour programmer la seconde approche demandée.

Principe :

- découper la carte en sous-domaines 2D, de préférence en blocs plutôt qu'en bandes simples
- chaque processus ne stocke que sa sous-carte, avec une couche de cellules fantômes
- chaque processus gère uniquement les fourmis présentes sur son sous-domaine
- à chaque itération, échanger les bords de phéromones avec les voisins
- lorsqu'une fourmi sort du sous-domaine, la transférer au processus voisin correspondant
- conserver un `MPI_Allreduce` uniquement pour les compteurs globaux nécessaires, par exemple la quantité totale de nourriture

Pourquoi cette stratégie est intéressante :

- elle réduit fortement la mémoire occupée par rapport à la première façon
- elle évite une synchronisation globale complète de toute la carte à chaque itération
- le volume de communication dépend surtout du périmètre des sous-domaines

Difficultés attendues :

- gestion correcte des cellules fantômes
- migration des fourmis entre processus
- déséquilibre de charge possible autour de la fourmilière

Piste raisonnable pour limiter ce dernier point :

- utiliser une décomposition cartésienne 2D
- garder un coefficient d'exploration suffisant pour disperser les fourmis
- si besoin, remailler plus tard la carte ou déplacer la fourmilière vers une zone moins extrême

## Conclusion générale

Ce que montrent les mesures :

- la vectorisation fournit la base de travail utilisée par les branches parallèles, mais n'améliore pas seule les temps sur ces mesures
- OpenMP est la meilleure optimisation obtenue sur cette base
- sur cette machine, le meilleur point mesuré est `6 threads`
- l'accélération calcul OpenMP atteint environ `2.66x`
- l'accélération totale OpenMP reste limitée à environ `1.65x` par l'affichage et les parties séquentielles restantes
- la première stratégie MPI fonctionne, mais son coût de communication limite rapidement les gains et annule le bénéfice sur le temps total

Conclusion pratique :

- pour le calcul local sur une seule machine, OpenMP est la solution la plus rentable ici
- pour MPI, la première stratégie reste valide mais peu scalable
- la suite logique du projet serait soit d'optimiser encore le dépôt des phéromones côté OpenMP, soit de passer à la seconde stratégie MPI si l'objectif est la montée en nombre de processus
