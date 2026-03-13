# Rapport de Benchmark

## Contexte

Objectif: comparer `main`, `vectorisation` et `MPI_parallelisation` sur deux
charges (`50000` et `100000` fourmis), puis mesurer l'effet du nombre de
processus MPI.

Le point important pour MPI est que le résultat dépend du nombre de processus.
Du coup, le rapport distingue deux niveaux d'analyse:

1. comparaison "branche contre branche" avec MPI en `np=1`
2. étude de montée en charge MPI avec `np=1,2,4,6`

## Machine de test

- CPU: `AMD Ryzen 5 4600H`
- Coeurs physiques: `6`
- Threads logiques: `12`

Le tableau MPI s'arrête donc à `np=6` pour respecter la contrainte
"1 coeur par processus".

## Méthodologie

- Date de campagne: `2026-03-13`
- Compilation: `-std=c++17 -O2 -march=native -Wall`
- `OMP_NUM_THREADS=1` sur tous les runs
- MPI lancé avec:

```bash
/usr/bin/mpirun.openmpi --bind-to core --map-by core -np N ./ant_simu.exe
```

- Les branches réelles n'ont pas été modifiées pour la mesure
- Le benchmark a été fait dans des copies temporaires
- Pour éviter l'aléa "jusqu'à la première nourriture", chaque run a été arrêté
  après `300` itérations fixes

Ce protocole est plus propre pour comparer les temps moyens par itération.

## Métriques utilisées

- `temps advanc_time`: temps moyen passé à faire avancer les fourmis
- `temps vaporisation`: temps moyen de la phase d'évaporation
- `temps calcule vapo+advancing`: temps moyen principal de calcul
- `time per affichage`: temps moyen d'affichage
- `temps total estimé`: `temps calcule vapo+advancing + time per affichage`

Le `temps total estimé` est la meilleure approximation du coût complet d'une
itération avec les chronos actuellement présents dans le code.

## Résultats bruts

### Initialisation

#### `50000` fourmis

| Version | Generation fractal (s) | Generation fourmis (s) |
| --- | ---: | ---: |
| `main` | 0.063843 | 0.008337 |
| `vectorisation` | 0.087944 | 0.007831 |
| `mpi np=1` | 0.082741 | 0.011471 |

#### `100000` fourmis

| Version | Generation fractal (s) | Generation fourmis (s) |
| --- | ---: | ---: |
| `main` | 0.080870 | 0.010297 |
| `vectorisation` | 0.083510 | 0.009006 |
| `mpi np=1` | 0.077564 | 0.013413 |

L'initialisation varie un peu d'un run à l'autre, mais ce n'est pas la partie
dominante du problème.

### Comparaison directe des trois versions

Ici, `MPI` est pris en `np=1` pour comparer le surcoût de la branche MPI elle-même.

#### `50000` fourmis

| Version | Adv (s/it) | Vapo (s/it) | Calcul (s/it) | Affichage (s/it) | Total estimé (s/it) |
| --- | ---: | ---: | ---: | ---: | ---: |
| `main` | 0.035070 | 0.000479 | 0.035563 | 0.016234 | 0.051796 |
| `vectorisation` | 0.055545 | 0.000526 | 0.056104 | 0.011294 | 0.067399 |
| `mpi np=1` | 0.051368 | 0.003152 | 0.055861 | 0.017222 | 0.073083 |

#### `100000` fourmis

| Version | Adv (s/it) | Vapo (s/it) | Calcul (s/it) | Affichage (s/it) | Total estimé (s/it) |
| --- | ---: | ---: | ---: | ---: | ---: |
| `main` | 0.078426 | 0.000524 | 0.078964 | 0.026389 | 0.105353 |
| `vectorisation` | 0.080484 | 0.000442 | 0.080938 | 0.024489 | 0.105428 |
| `mpi np=1` | 0.086939 | 0.002825 | 0.090972 | 0.020115 | 0.111088 |

### Table MPI en fonction du nombre de processus

#### `50000` fourmis

| `np` | Adv (s/it) | Vapo (s/it) | Calcul (s/it) | Affichage (s/it) | Total estimé (s/it) | Accélération calcul vs `np=1` | Accélération totale vs `np=1` |
| ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | 0.038434 | 0.002839 | 0.042516 | 0.021584 | 0.064101 | 1.000 | 1.000 |
| 2 | 0.029031 | 0.009964 | 0.040905 | 0.027629 | 0.068534 | 1.039 | 0.935 |
| 4 | 0.010038 | 0.020577 | 0.033675 | 0.031341 | 0.065015 | 1.263 | 0.986 |
| 6 | 0.012751 | 0.022546 | 0.039559 | 0.033077 | 0.072635 | 1.075 | 0.882 |

#### `100000` fourmis

| `np` | Adv (s/it) | Vapo (s/it) | Calcul (s/it) | Affichage (s/it) | Total estimé (s/it) | Accélération calcul vs `np=1` | Accélération totale vs `np=1` |
| ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | 0.078215 | 0.002677 | 0.082051 | 0.033805 | 0.115856 | 1.000 | 1.000 |
| 2 | 0.041336 | 0.007827 | 0.050917 | 0.038660 | 0.089576 | 1.611 | 1.293 |
| 4 | 0.030674 | 0.017603 | 0.051118 | 0.044668 | 0.095786 | 1.605 | 1.210 |
| 6 | 0.023742 | 0.029108 | 0.056944 | 0.050387 | 0.107331 | 1.441 | 1.079 |

## Analyse détaillée

### 1. Comparaison `main` vs `vectorisation` vs `MPI np=1`

#### Cas `50000`

`main` est la meilleure version sur le temps total.

- `vectorisation` est `30.1 %` plus lente que `main` sur le total estimé
- `vectorisation` est `57.8 %` plus lente que `main` sur le temps principal de calcul
- `mpi np=1` est `41.1 %` plus lente que `main` sur le total estimé
- `mpi np=1` est `57.1 %` plus lente que `main` sur le temps principal de calcul

Conclusion pour `50000`:

- la charge est trop faible pour rentabiliser MPI
- la branche `vectorisation` ne donne pas de gain sur cette machine
- la meilleure version reste `main`

#### Cas `100000`

`main` et `vectorisation` sont quasiment équivalentes.

- `vectorisation` est `0.07 %` plus lente que `main` sur le total estimé
- `vectorisation` est `2.5 %` plus lente que `main` sur le calcul
- `mpi np=1` est `5.4 %` plus lente que `main` sur le total estimé
- `mpi np=1` est `15.2 %` plus lente que `main` sur le calcul

Conclusion pour `100000`:

- la branche `vectorisation` ne détériore presque plus les performances
- mais elle n'apporte pas non plus d'amélioration mesurable
- la branche MPI en `np=1` a un surcoût clair, donc il faut au moins `np>1`
  pour espérer gagner quelque chose

### 2. Ce que montre réellement MPI

#### Tendance générale

Quand `np` augmente:

- `temps advanc_time` baisse fortement
- `temps vaporisation` augmente fortement
- `time per affichage` augmente aussi

Autrement dit:

- la distribution des fourmis fonctionne bien
- mais la communication MPI et la reconstruction de l'état global coûtent cher

Cela correspond exactement à l'idée de départ:

- chaque processus garde tout l'environnement
- chaque processus ne gère qu'une partie des fourmis
- il faut ensuite synchroniser les phéromones et les positions

#### Pourquoi `temps vaporisation` explose avec MPI

Dans la version MPI, ce chrono ne représente plus seulement l'évaporation locale.
Il contient aussi le coût des communications collectives utilisées pour remettre
tous les environnements d'accord:

- `MPI_Allreduce` sur les phéromones
- `MPI_Allgatherv` sur les sous-parties évaporées

La montée de ce chrono est très nette:

- `50000`: `0.002839` en `np=1`, `0.020577` en `np=4`, `0.022546` en `np=6`
- `100000`: `0.002677` en `np=1`, `0.017603` en `np=4`, `0.029108` en `np=6`

Donc plus on augmente `np`, plus la communication annule une partie du gain
obtenu sur l'avancement des fourmis.

#### Pourquoi l'affichage augmente aussi

L'affichage est centralisé sur le rang 0. Il faut donc rassembler les positions
des fourmis avant de dessiner.

On le voit bien dans les chiffres:

- `50000`: `0.021584` en `np=1`, `0.031341` en `np=4`, `0.033077` en `np=6`
- `100000`: `0.033805` en `np=1`, `0.044668` en `np=4`, `0.050387` en `np=6`

Le calcul peut s'améliorer tout en perdant du temps global si cette partie
devient trop coûteuse.

### 3. Quel est le meilleur nombre de processus MPI

#### Pour `50000` fourmis

Sur le calcul pur:

- meilleur `np`: `4`
- temps calcul: `0.033675 s/it`
- gain vs `main`: environ `5.3 %`
- accélération vs `MPI np=1`: `1.263x`

Sur le temps total:

- meilleur `np`: `1`
- temps total estimé: `0.064101 s/it`

Conclusion:

- oui, `np=4` améliore le calcul pur
- non, cela ne suffit pas pour gagner sur le temps global
- à `50000`, MPI n'est pas rentable sur cette machine

Par rapport à `main`:

- meilleur MPI en total reste `23.8 %` plus lent que `main`
- meilleur MPI en calcul est seulement `5.6 %` plus rapide que `main`

Le gain calcul est donc trop petit pour compenser la communication et
l'affichage.

#### Pour `100000` fourmis

Sur le calcul pur:

- meilleur `np`: `2`
- temps calcul: `0.050917 s/it`
- gain vs `main`: environ `35.5 %`
- accélération vs `MPI np=1`: `1.611x`

Sur le temps total:

- meilleur `np`: `2`
- temps total estimé: `0.089576 s/it`
- accélération vs `MPI np=1`: `1.293x`

Par rapport à `main`:

- gain total: environ `15.0 %`
- vitesse relative: `1.176x` plus rapide que `main`

Par rapport à `vectorisation`:

- gain total: environ `15.0 %`

Conclusion:

- à `100000`, la charge devient suffisante pour rentabiliser MPI
- la meilleure configuration ici est `np=2`
- `np=4` améliore encore fortement `advanc_time`, mais le surcoût
  communication + affichage annule une partie du bénéfice
- `np=6` est déjà trop haut pour ce problème sur cette machine

### 4. Ce que l'on peut conclure sur chaque branche

#### Branche `main`

- c'est la meilleure base à `50000`
- elle reste très compétitive à `100000`
- elle sert de vraie référence de temps

#### Branche `vectorisation`

- elle n'apporte pas d'amélioration nette dans cette campagne
- elle est franchement moins bonne à `50000`
- elle devient presque neutre à `100000`

Conclusion:

- sur cette machine et sur ce code, la vectorisation seule ne justifie pas
  de remplacer `main`

#### Branche `MPI_parallelisation`

- en `np=1`, elle a un surcoût
- en `np=2`, elle devient intéressante à `100000`
- en `np=4`, elle donne le meilleur calcul pur à `50000`, mais pas le meilleur total
- en `np=6`, le coût des collectives devient trop important

Conclusion:

- l'approche MPI est pertinente seulement si la charge est assez grande
- il existe bien un domaine où MPI améliore le temps
- mais ce domaine est limité par les coûts de synchronisation

## Réponse claire à la question "est-ce qu'il y a amélioration ?"

### Oui, mais pas toujours

#### `50000` fourmis

- `main` reste la meilleure version globale
- `vectorisation` n'améliore pas
- MPI n'améliore pas le temps total

#### `100000` fourmis

- `vectorisation` n'améliore pratiquement pas
- MPI améliore le temps global si on choisit bien le nombre de processus
- meilleure config observée: `MPI np=2`

## Conclusion finale

La conclusion expérimentale la plus importante est la suivante:

- à petite charge (`50000`), MPI ne vaut pas le coût de communication
- à charge plus élevée (`100000`), MPI devient rentable
- la meilleure configuration mesurée ici est `MPI np=2`

En résumé:

- meilleur choix à `50000`: `main`
- meilleur choix à `100000`: `MPI_parallelisation` avec `2` processus
- la branche `vectorisation` n'apporte pas de gain convaincant dans cette campagne

## Fichiers produits

- résultats bruts JSON: `benchmark_results.json`
- rapport détaillé: `RAPPORT_BENCHMARK.md`
