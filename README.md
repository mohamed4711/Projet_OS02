# Projet_OS02

1)Temps 
L'initialisation du paysage fractal (~0.12 s) et de la colonie (~0.02 s) n'étant effectuée qu'une seule fois, l'analyse se concentre sur la boucle principale.

Temps moyen par itération :

Rendu graphique (SDL) : ~10.0 ms. C'est l'étape la plus chronophage de la boucle (avoir si on peut  paralleliser SDL :/ ).

Mouvement et logique des fourmis : ~5.4 ms. Représente 90% du temps de calcul physique. C'est le goulot d'étranglement principal et la cible prioritaire pour la vectorisation et l'accélération via OpenMP.


Évaporation des phéromones : ~0.6 ms. Opération matricielle très rapide (10% du calcul), qui bénéficiera facilement d'une parallélisation en mémoire partagée.

2) Parallélisation de l'avancement des fourmies
A cette étape, nous avons effectué la modification : 
#pragma omp parallel for
    for ( size_t i = 0; i < ants.size(); ++i ) {
        ants[i].advance(phen, land, pos_food, pos_nest, cpteur);// calcule le chemin et met a jour pheronome 
    }

Temps pour faire l'avancement de toutes les fourmies : 0.00536794
Speed up : sans parallélisation  0.00250297
    -> S = 0.00536794/0.00250297 = 2.14