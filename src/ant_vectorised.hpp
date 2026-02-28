#if !defined ANT_VECTORISED 
#define ANT_VECTORISED


#include <chrono>
#include <vector>
#include <iostream>
#include <random>
#include "fractal_land.hpp"
#include "ant.hpp"
#include "pheronome.hpp"
# include "renderer.hpp"
# include "window.hpp"
# include "../src/rand_generator.hpp"

#include"basic_types.hpp"








enum{NONCHARGE=false,CHARGE=true}; 


class ant_vectorised{



public:


/*constructors */
ant_vectorised()=default; 
ant_vectorised(size_t nbfourmi ): nb_fourmi(nbfourmi){

    fourmi_pos.resize(nbfourmi); 
    fourmi_grain.resize(nbfourmi); 
    fourmi_etat.resize(nbfourmi, NONCHARGE);

} 



void advance( pheronome& phen, const fractal_land& land, const position_t& pos_food, const position_t& pos_nest,std::size_t& cpteur_food ,  ant_vectorised & Ant_colony ,  int id_ant ,double m_eps) ; 
/*methodes */


/*getters*/



size_t get_nb_fourmi(){
    return nb_fourmi;  }

std::vector<position_t> get_pos_fourmi(){
    return fourmi_pos; }

std::vector<size_t> get_grain_fourmi(){
    return fourmi_grain; }
std::vector<int>get_etat_fourmi(){
    return fourmi_etat; }






public: 

size_t  nb_fourmi; 

std::vector<position_t> fourmi_pos ; //tab de taille nb_fourmi*2pour garder position et indice fourmi 
std::vector<size_t> fourmi_grain ; // taille nb*1 
std::vector<int> fourmi_etat ; //taille nb*1


};



#endif