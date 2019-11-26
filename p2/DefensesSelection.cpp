// ###### Config options ################


// #######################################

#define BUILDING_DEF_STRATEGY_LIB 1

#include "../simulador/Asedio.h"
#include "../simulador/Defense.h"
#include <algorithm>

using namespace Asedio;

/*std::vector<float> obtenerPuntuaciones( std::list<Defense*> defenses)
{
    std::vector<float> puntuaciones;
    std::list<Defense*>::iterator it = defenses.begin();
    ++it;

    while(it != defenses.end()) {
       
       // float puntuacion = ((*it)->range)*10 + ((*it)->dispersion)*100 +((*it)->damage)*100 +((*it)->attacksPerSecond)*100 +((*it)->health) - ((*it)->cost);

        float puntuacion2 = ((*it)->health) + ((*it)->damage) *((*it)->attacksPerSecond);
        float puntuacion = (puntuacion2)/((*it)->cost);
        puntuaciones.push_back(puntuacion);
        
        ++it;
    }


    return puntuaciones;
}*/

/*std::vector<unsigned int> obtenerCostos (std::list<Defense*> defenses)
{
    std::vector<unsigned int> costos;
    std::list<Defense*>::iterator itdefenses = defenses.begin();
    ++itdefenses;
    while(itdefenses != defenses.end())
    {
        costos.push_back((*itdefenses)->cost);
        ++itdefenses;
    } 
    return costos;
}*/

void DEF_LIB_EXPORTED selectDefenses(std::list<Defense*> defenses, unsigned int ases, std::list<int> &selectedIDs
            , float mapWidth, float mapHeight, std::list<Object*> obstacles) {

    selectedIDs.push_front(0);
    unsigned int i = defenses.size() - 1;
    unsigned int j = ases;
    std::list<Defense*>::iterator primeraDefensa = defenses.begin();
    ases = ases - (*primeraDefensa)->cost;


    std::vector<float> puntuaciones;
    std::list<Defense*>::iterator it = defenses.begin();
    ++it;

    while(it != defenses.end()) {
       
       // float puntuacion = ((*it)->range)*10 + ((*it)->dispersion)*100 +((*it)->damage)*100 +((*it)->attacksPerSecond)*100 +((*it)->health) - ((*it)->cost);

        float puntuacion2 = ((*it)->health) + ((*it)->damage) *((*it)->attacksPerSecond);
        float puntuacion = (puntuacion2)/((*it)->cost);
        puntuaciones.push_back(puntuacion);
        
        ++it;
    }

    

    std::vector<unsigned int> costos;
    std::list<Defense*>::iterator itdefenses = defenses.begin();
    ++itdefenses;
    while(itdefenses != defenses.end())
    {
        costos.push_back((*itdefenses)->cost);
        ++itdefenses;
    } 
    
    float matriz [defenses.size()][ases+1];

    

    for(int col=0; col <= ases; ++col)
    {
        if(col < costos[0])
        {
            matriz[0][col] = 0;

        } else matriz[0][col] = puntuaciones[0];

    }

    for(int i=1; i < defenses.size(); ++i)
    {
        for(int col=0; col<=ases; ++col)
        {
            if (col < costos[i])
            {
                matriz[i][col] = matriz[i-1][col];
            } else matriz[i][col] = std::max(matriz[i-1][col], matriz[i-1][col-costos[i]]+puntuaciones[i]);
        }

    }

    /* for(int i=1; i < defenses.size(); ++i)
    {
        for(int col=0; col<=ases; ++col)
        {
            std::cout<<matriz[i][col]<<" ";
        }
        std::cout<<std::endl;

    }*/

    
    std::vector<Defense*> defensas(defenses.size());
    std::copy(defenses.begin(), defenses.end(), defensas.begin());

    int i2 = defenses.size() - 1;
    while(i2 > 0 && ases > 0) 
    {

        if(matriz[i2][ases] != matriz[i2-1][ases])
        {
            std::cout << i2 << "," << ases << std::endl;
            selectedIDs.push_back(i2);
            ases = ases - costos[i2];
        }  
        i2--;

    }
    
}


