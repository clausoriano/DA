// ###### Config options ################

//#define PRINT_DEFENSE_STRATEGY 1    // generate map images

// #######################################

#define BUILDING_DEF_STRATEGY_LIB 1

#include "../simulador/Asedio.h"
#include "../simulador/Defense.h"

#ifdef PRINT_DEFENSE_STRATEGY
#include "ppm.h"
#endif

#ifdef CUSTOM_RAND_GENERATOR
RAND_TYPE SimpleRandomGenerator::a;
#endif

using namespace Asedio;

//Estructura celda, con sus coordenadas y su valor.

struct Cell {
	int x, y;
	double value;
};

//Funcion auxiliar que ayuda a ordenar las celdas en el vector segun su valor-
bool sortValue(const Cell& c1, const Cell& c2)
{
	return c1.value > c2.value;
}
typedef std::vector<Cell> Candidates;

//Funcion para pasar de celda a posicion cartesiana
Vector3 CellToPos(int i, int j, float cellWidth, float cellHeight)
{ return Vector3((j * cellWidth) + cellWidth * 0.5f, (i * cellHeight) + cellHeight * 0.5f, 0); }
//Return un Vector3 con la posicion del centro de la celda


//Funcion que le da un valor a una celda en funcion de la distancia que haya
//desde esta al centro del mapa
float cellValueRC(int row, int col, int nCellsWidth, int nCellsHeight, float cellWidth, float cellHeight) {

		Vector3 cell = CellToPos(row,col,cellWidth,cellHeight);
	  Vector3 center = CellToPos(nCellsWidth/2,nCellsHeight/2,cellWidth,cellHeight);
	  double distcenter = _distance(cell,center);
	  return (double)100/distcenter;
}

//Funcion que genera todos los candidatos para colocar el Centro de Recursos
//Ordena el vector de candidatos de mayor puntuacion a menor puntuacion.
Candidates candidatesRC(int nCellsWidth, int nCellsHeight, float cellWidth, float cellHeight){
  Candidates candidates;
  for(int i=0;i<nCellsWidth;i++)
    for(int j=0;j<nCellsHeight;j++){
			Cell cell;
			cell.x = i;
			cell.y = j;
      cell.value =cellValueRC(i,j,nCellsWidth,nCellsHeight,cellWidth,cellHeight);
      candidates.insert(candidates.begin(),cell);
    }

		sort(candidates.begin(), candidates.end(), sortValue); //Ya funciona
    return candidates;
}

//Genera los candidatos para colocar las defensas.
//Llama a cellValueRC para calcular el valor de cada celda.
//junto con sus coordenadas en un vector. Ordena ese vector de mayor a menor.
Candidates candidatesD(Vector3 CEpos, int nCellsWidth, int nCellsHeight, float cellWidth, float cellHeight){
  Candidates candidatesD;
  for(int i=0;i<nCellsWidth;i++)
    for(int j=0;j<nCellsHeight;j++){
			Cell cell;
			cell.x = i;
			cell.y = j;
      cell.value =cellValueRC(i,j,nCellsWidth,nCellsHeight,cellWidth,cellHeight);
      candidatesD.insert(candidatesD.begin(),cell);
    }
		sort(candidatesD.begin(), candidatesD.end(), sortValue);
    return candidatesD;
}

//Funcion de factibilidad, comprueba que no haya otro obstaculo o defensa y que no se salga del mapa
bool funfact(float radio, int row, int col, int cellWidth, int cellHeight, float mapWidth, float mapHeight, List<Object*> obstacles, List<Defense*> defenses)
{
	bool factible = true;
  Vector3 celda = CellToPos(row,col,cellWidth,cellHeight);

	//Vemos si se sale del mapa
  if(celda.x + radio >= mapWidth || celda.x - radio <= 0 ||
   celda.y + radio >= mapHeight || celda.y - radio <= 0 ) factible = false;

  //Comprobamos si se puede colocar el objeto en la celda o si choca con algo
  for (List<Object*>::iterator ObsIter= obstacles.begin(); ObsIter != obstacles.end(); ObsIter++)
    if(_distance(celda,(*ObsIter)->position) <= (float)((*ObsIter)->radio+radio)) factible = false;
  for (List<Defense*>::iterator DefIter = defenses.begin(); DefIter != defenses.end(); DefIter++)
    if(_distance(celda,(*DefIter)->position) <= (float)((*DefIter)->radio+radio)) factible = false;



  return factible;
}

void DEF_LIB_EXPORTED placeDefenses(bool** freeCells, int nCellsWidth, int nCellsHeight, float mapWidth, float mapHeight, std::list<Object*> obstacles, std::list<Defense*> defenses)
{
	//PARTE ALGORITMO CENTRO RECURSOS
    int maxAttemps = 1000;
    List<Defense*>::iterator currentDefense = defenses.begin();

    Candidates candidatesRCC = candidatesRC(nCellsWidth,nCellsHeight,mapWidth / nCellsWidth, mapHeight / nCellsHeight);
    Cell possible;
    bool solution = false;

    while(!solution && !candidatesRCC.empty())
		{

      possible = *candidatesRCC.begin();

      candidatesRCC.erase(candidatesRCC.begin());

      if(funfact((*currentDefense)->radio,possible.x,possible.y,mapWidth / nCellsWidth,mapHeight / nCellsHeight,mapWidth,mapHeight,obstacles,defenses))
			{
        solution = true;

        (*currentDefense)->position = CellToPos(possible.x,possible.y,mapWidth / nCellsWidth,mapHeight / nCellsHeight);
      }
    }
		//PARTE ALGORITMO DEFENSAS
    ++currentDefense;
    Candidates candidatesDE = candidatesD((*defenses.begin())->position, nCellsWidth, nCellsHeight, mapWidth / nCellsWidth, mapHeight / nCellsHeight);

    while(currentDefense != defenses.end() && maxAttemps > 0 && !candidatesDE.empty()){

      possible = *candidatesDE.begin();

      candidatesDE.erase(candidatesDE.begin());

      if(funfact((*currentDefense)->radio,possible.x,possible.y,mapWidth / nCellsWidth,mapWidth / nCellsWidth,mapWidth,mapHeight,obstacles,defenses))
			{
				(*currentDefense)->position = CellToPos(possible.x,possible.y,mapWidth / nCellsWidth,mapHeight / nCellsHeight);
				++currentDefense;
        --maxAttemps;
      }
      else
        --maxAttemps;
    }

		#ifdef PRINT_DEFENSE_STRATEGY

		    float** cellValues = new float* [nCellsHeight];
		    for(int i = 0; i < nCellsHeight; ++i) {
		       cellValues[i] = new float[nCellsWidth];
		       for(int j = 0; j < nCellsWidth; ++j) {
		           cellValues[i][j] = ((int)(cellValue(i, j))) % 256;
		       }
		    }
		    dPrintMap("strategy.ppm", nCellsHeight, nCellsWidth, cellHeight, cellWidth, freeCells
		                         , cellValues, std::list<Defense*>(), true);

		    for(int i = 0; i < nCellsHeight ; ++i)
		        delete [] cellValues[i];
			delete [] cellValues;
			cellValues = NULL;

		#endif
    }
