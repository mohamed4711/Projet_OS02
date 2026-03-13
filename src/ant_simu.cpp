#include <algorithm>
#include <chrono>
#include <vector>
#include <iostream>
#include <random>
#include "fractal_land.hpp"
#include "pheronome.hpp"
# include "renderer.hpp"
# include "window.hpp"
# include "../src/rand_generator.hpp"
#include<stdio.h>
#include"ant_vectorised.hpp"
#include <chrono>
#include <mpi.h>

void compute_chunk( int total, int rank, int size, int& begin, int& count )
{
    count = total / size;
    int reste = total % size;
    if ( rank < reste ) count += 1;
    begin = rank * ( total / size ) + std::min( rank, reste );
}

void advance_time( const fractal_land& land, pheronome& phen,
                   const position_t& pos_nest, const position_t& pos_food,
                   ant_vectorised& ants , std::size_t& cpteur, double & vapo_time,
                   double& advance_time ,double m_eps, int rank,
                   const std::vector<int>& row_counts,
                   const std::vector<int>& row_displs,
                   std::vector<double>& phen_buffer )
{
    phen.copy_map_to_buffer();

    auto t1 = std::chrono::high_resolution_clock::now();
    for ( size_t i = 0; i < ants.nb_fourmi; ++i )
      ants.advance(phen,  land, pos_food,  pos_nest,cpteur ,ants,  i , m_eps) ;
    auto t2 =std::chrono::high_resolution_clock::now();
    advance_time+= std::chrono::duration<double>(t2-t1 ).count();

    MPI_Allreduce( MPI_IN_PLACE, phen.buffer_data(), static_cast<int>( phen.double_size() ),
                   MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD );
    std::copy( phen.buffer_data(), phen.buffer_data() + phen.double_size(), phen_buffer.begin() );

    int row_width = static_cast<int>( phen.stride() ) * 2;
    int row_begin = row_displs[rank] / row_width;
    int row_count = row_counts[rank] / row_width;
    phen.do_evaporation( row_begin, row_begin + row_count );

    MPI_Allgatherv( phen.buffer_data() + row_displs[rank], row_counts[rank], MPI_DOUBLE,
                    phen_buffer.data(), row_counts.data(), row_displs.data(),
                    MPI_DOUBLE, MPI_COMM_WORLD );
    std::copy( phen_buffer.begin(), phen_buffer.end(), phen.buffer_data() );

    auto t3 = std::chrono::high_resolution_clock::now();
    vapo_time+= std::chrono::duration<double>(t3-t2).count();

    phen.update();
}

int main(int argc, char* argv[])
{
    MPI_Init( &argc, &argv );

    int rank = 0;
    int size = 1;
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &size );

    std::size_t seed = 2026;
    const int nb_ants = 50000;
    const double eps = 0.8;
    const double alpha=0.7;
    const double beta=0.999;
    position_t pos_nest{256,256};
    position_t pos_food{500,500};

    int begin_ant = 0;
    int nb_local_ants = 0;
    compute_chunk( nb_ants, rank, size, begin_ant, nb_local_ants );
    seed += begin_ant;

    auto start= std::chrono::high_resolution_clock::now();
    fractal_land land(8,2,1.,1024);
    double max_val = 0.0;
    double min_val = 0.0;
    for ( fractal_land::dim_t i = 0; i < land.dimensions(); ++i )
        for ( fractal_land::dim_t j = 0; j < land.dimensions(); ++j ) {
            max_val = std::max(max_val, land(i,j));
            min_val = std::min(min_val, land(i,j));
        }

    double delta = max_val - min_val;
    for ( fractal_land::dim_t i = 0; i < land.dimensions(); ++i )
        for ( fractal_land::dim_t j = 0; j < land.dimensions(); ++j )  {
            land(i,j) = (land(i,j)-min_val)/delta;
        }

   auto end =std::chrono::high_resolution_clock::now();
   double duration  = std::chrono::duration<double >(end- start).count();
   if ( rank == 0 ) printf( "Generation fractal  %f \n",duration);

    auto start1=std::chrono::high_resolution_clock::now();
    ant_vectorised ant_colony(nb_local_ants);

    auto gen_ant_pos = [&land, &seed] () { return rand_int32(0, land.dimensions()-1, seed); };
    for ( int i = 0; i < nb_local_ants; ++i ){
        ant_colony.fourmi_grain[i]=seed;
        ant_colony.fourmi_pos[i]=position_t{gen_ant_pos(),gen_ant_pos()};
    };

    pheronome phen(land.dimensions(), pos_food, pos_nest, alpha, beta);

    std::vector<int> row_counts( size );
    std::vector<int> row_displs( size );
    std::vector<int> ant_counts( size );
    std::vector<int> ant_displs( size );

    for ( int r = 0; r < size; ++r ) {
        int begin_row = 0;
        int nb_rows = 0;
        int begin_ant_r = 0;
        int nb_ant_r = 0;
        compute_chunk( static_cast<int>( land.dimensions() ), r, size, begin_row, nb_rows );
        compute_chunk( nb_ants, r, size, begin_ant_r, nb_ant_r );
        row_counts[r] = nb_rows * static_cast<int>( phen.stride() ) * 2;
        row_displs[r] = ( begin_row + 1 ) * static_cast<int>( phen.stride() ) * 2;
        ant_counts[r] = nb_ant_r * 2;
        ant_displs[r] = begin_ant_r * 2;
    }

    std::vector<double> phen_buffer( phen.double_size() );

    auto end1= std::chrono::high_resolution_clock::now();
    double duration1 = std::chrono::duration<double >(end1- start1).count();
    if ( rank == 0 ) printf( "genaration fourmis %f \n ",duration1);

    Window* win = nullptr;
    Renderer* renderer = nullptr;
    std::vector<position_t> all_ant_positions;

    if ( rank == 0 ) {
        SDL_Init( SDL_INIT_VIDEO );
        all_ant_positions.resize( nb_ants );
        win = new Window("Ant Simulation", 2*land.dimensions()+10, land.dimensions()+266);
        renderer = new Renderer( land, phen, pos_nest, pos_food, all_ant_positions, nb_ants );
    }

    size_t food_quantity = 0;
    unsigned long long total_food_quantity = 0;
    SDL_Event event;
    bool cont_loop = true;
    bool not_food_in_nest = true;
    std::size_t it = 0;
    double time_calculating =0 ;
    double time_displaying =0;
    double advanc_time=0;
    double vapo_time=0 ;

    while (cont_loop) {

        ++it;
        int keep_running = 1;
        if ( rank == 0 ) {
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT)
                    keep_running = 0;
            }
        }
        MPI_Bcast( &keep_running, 1, MPI_INT, 0, MPI_COMM_WORLD );
        if ( keep_running == 0 )
            cont_loop = false;
        if ( !cont_loop )
            break;

        auto start2=std::chrono::high_resolution_clock::now();
        advance_time( land, phen, pos_nest, pos_food, ant_colony, food_quantity,
                      vapo_time, advanc_time ,eps, rank, row_counts, row_displs, phen_buffer );
        auto end2=std::chrono::high_resolution_clock::now();

        double duration2  = std::chrono::duration<double >(end2- start2).count();
        time_calculating=time_calculating+duration2;

        unsigned long long local_food = static_cast<unsigned long long>( food_quantity );
        MPI_Allreduce( &local_food, &total_food_quantity, 1, MPI_UNSIGNED_LONG_LONG,
                       MPI_SUM, MPI_COMM_WORLD );

        MPI_Gatherv( reinterpret_cast<int*>( ant_colony.fourmi_pos.data() ), ant_counts[rank], MPI_INT,
                     rank == 0 ? reinterpret_cast<int*>( all_ant_positions.data() ) : nullptr,
                     ant_counts.data(), ant_displs.data(), MPI_INT, 0, MPI_COMM_WORLD );

        if ( rank == 0 ) {
            renderer->display( *win, total_food_quantity );
            win->blit();
        }

        auto end3 = std::chrono::high_resolution_clock::now();
        if ( rank == 0 ) {
            double duration3=std::chrono::duration<double>(end3-end2 ).count();
            time_displaying=time_displaying+duration3;
        }

        if ( rank == 0 && not_food_in_nest && total_food_quantity > 0 ) {
             std::cout << "MPI ranks              = " << size << std::endl;
             std::cout << "La première nourriture est arrivée au nid a l'iteration " << it << std::endl;
             double time_cal_per_it =time_calculating/it;
             double time_disp_per_ot=time_displaying/it;
             vapo_time=vapo_time/it ;
             advanc_time=advanc_time/it ;

             std::cout<<"temps advanc_time  ="<<advanc_time<<std::endl;
             std::cout<<"temps vaporisation  ="<<vapo_time<<std::endl;
             std::cout<<"temps calcule vapo+advancing  ="<<time_cal_per_it<<std::endl;
             std::cout<<"time per affichage ="<<time_disp_per_ot<<std::endl;

            not_food_in_nest = false;
        }
    }

    if ( rank == 0 ) {
        delete renderer;
        delete win;
        SDL_Quit();
    }

    MPI_Finalize();
    return 0;
}
