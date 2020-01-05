// ###### Config options ################

//#define PRINT_DEFENSE_STRATEGY 1    // generate map images

// #######################################

#define BUILDING_DEF_STRATEGY_LIB 1

#include "../simulador/Asedio.h"
#include "../simulador/Defense.h"
#include "cronometro.h"
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


//Sobrecarga operadores
bool operator <(const Cell &p1,const Cell &p2)
{
  return  (p1.value < p2.value);
}

bool operator >(const Cell &p1,const Cell &p2)
{
  return  (p1.value > p2.value);
}

//Funcion auxiliar que ayuda a ordenar las celdas en el vector segun su valor-
bool sortValue(const Cell& c1, const Cell& c2)
{
	return c1.value > c2.value;
}

typedef std::vector<Cell> Candidates;

struct greaters{ 
  bool operator()(const Cell& a,const Cell& b) const{ 
    return a.value > b.value; 
  } 
}; 

//Funcion para pasar de celda a posicion cartesiana
Vector3 CellToPos(int i, int j, float cellWidth, float cellHeight)
{ return Vector3((j * cellWidth) + cellWidth * 0.5f, (i * cellHeight) + cellHeight * 0.5f, 0); }
//Return un Vector3 con la posicion del centro de la celda


//Funcion default que le da el valor a las celdas
float defaultCellValue(int row, int col, bool** freeCells, int nCellsWidth, int nCellsHeight, float mapWidth, float mapHeight, List<Object*> obstacles, List<Defense*> defenses)
{

    float cellWidth = mapWidth / nCellsWidth;
    float cellHeight = mapHeight / nCellsHeight;

    Vector3 cellPosition((col * cellWidth) + cellWidth * 0.5f, (row * cellHeight) + cellHeight * 0.5f, 0);

    float val = 0;
    for (List<Object*>::iterator it=obstacles.begin(); it != obstacles.end(); ++it) {
	    val += _distance(cellPosition, (*it)->position);
    }

    return val;
}

//Genera los candidatos para colocar las defensas.
//Llama a cellValueRC para calcular el valor de cada celda.
//junto con sus coordenadas en un vector. Ordena ese vector de mayor a menor.
Candidates candidatesD(bool** freeCells, int nCellsWidth, int nCellsHeight, float mapWidth, float mapHeight, List<Object*> obstacles, List<Defense*> defenses){
  Candidates candidatesD;
  for(int i=0;i<nCellsWidth;i++)
    for(int j=0;j<nCellsHeight;j++){
			Cell cell;
			cell.x = i;
			cell.y = j;
      cell.value = defaultCellValue(i, j, freeCells, nCellsWidth, nCellsHeight,  mapWidth,  mapHeight, obstacles, defenses); //Llama a la funcion default
      candidatesD.insert(candidatesD.begin(),cell);
    }

    return candidatesD; //VECTOR DESORDENADO
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

//Algoritmo que encuentra la celda con mayor puntuacion en el vector SIN PREORDENACION
int buscaMaximo(Candidates candidatos)
{
	double valor = 0;
	int i = 0;
	int posible = 0;

	for(i; i <= candidatos.size(); ++i)
	{
		if(candidatos[i].value > valor)
		{
			valor = candidatos[i].value;
			posible = i;
		}
	}
	return posible;
}

void ordenacionInsercion(Candidates& candidatos, int n)
{
 	int i, j; 
	Cell temp;
 	for(i=0; i<n; i++)
 	{
 		temp=candidatos[i];
 		j=i-1;
 		while(j>=0 && candidatos[j].value >temp.value)
 		{
 			candidatos[j+1] = candidatos[j];
 			j--;
 		}

		candidatos[j+1] = temp;
	}
}

void fusion(Candidates& candidatos, int l, int m, int r)
{
	int i, j , k;
	int n1 = m - l  + 1;
	int n2 = r - m;

	Cell L[n1], R[n2];

	for (i = 0; i < n1; i++) 
	{
        L[i] = candidatos[l + i]; 
	}
    for (j = 0; j < n2; j++)
	{ 
        R[j] = candidatos[m + 1+ j];
	} 

	i = 0;
	j = 0;
	k = l;

	while (i < n1 && j < n2) 
    { 
        if(L[i].value > R[j].value) 
        { 
            candidatos[k] = L[i]; 
            i++; 
        } 
        else
        { 
            candidatos[k] = R[j]; 
            j++; 
        } 
        k++; 
    }

	while(i < n1) 
    { 
        candidatos[k] = L[i]; 
        i++; 
        k++; 
    }  

	 while(j < n2) 
    { 
        candidatos[k] = R[j]; 
        j++; 
        k++; 
    } 
}

//Algoritmo CON PREORDENACION - FUSION
void ordenacionFusion(Candidates& candidatos, int inicio, int fin)
{
	int n = fin - inicio + 1;
	if (n==0 || n==1)
	{
		ordenacionInsercion(candidatos, n);
	}
	else
	{
		int k = inicio - 1 + (n/2);
		ordenacionFusion(candidatos, inicio, k);
		ordenacionFusion(candidatos, k+1, fin);
		fusion(candidatos, inicio, k, fin);
	}
	

}


int pivote(Candidates& candidatos, int izq, int der)
{
	int i;
	int pivote;
	double valor_pivote;
    Cell aux;

    pivote = izq;
    valor_pivote = candidatos[pivote].value;
    for (i=izq+1; i<=der; i++){
        if (candidatos[i].value >= valor_pivote){
                pivote++;
                aux=candidatos[i];
                candidatos[i]=candidatos[pivote];
                candidatos[pivote]=aux;

        }
    }
    aux=candidatos[izq];
    candidatos[izq]=candidatos[pivote];
    candidatos[pivote]=aux;
    return pivote;
}


//Algoritmo CON PREORDENACION - RAPIDA
void ordenacionRapida(Candidates& candidatos, int inicio, int fin )
{
	int n = fin - inicio + 1;
	if (n==0 || n==1)
	{
		ordenacionInsercion(candidatos, n);
	}
	else
	{
		int p = pivote(candidatos, inicio, fin);
		ordenacionRapida(candidatos, inicio, p-1);
		ordenacionRapida(candidatos, p+1, fin);
	}
	
}

template< class RandomIt >
void sort_heap(RandomIt first, RandomIt last  )
{
	while(first != last)
	{

		std::pop_heap(first,last--,greaters());
	}
}

//Algoritmo CON PREORDENACION - MONTICULO
void ordenacionMonticulo(Candidates& candidatos)
{
	std::make_heap(candidatos.begin(), candidatos.end(), greaters());
	std::sort_heap(candidatos.begin(), candidatos.end(), greaters());
}

bool pruebaFusion()
{
	Candidates candidatos;
	candidatos.resize(20);
	int inicio = 0;
	int fin = 19;
	double max = 3000;
	bool correcto = true;

	for(int i = 0; i < 20; ++i)
	{
		candidatos[i].value = rand() % (50+1);
	}
	//std::cout << "ORDENACION POR FUSION" << std::endl;
	ordenacionFusion(candidatos,inicio,fin);
	for(int j = 0; j < 20; ++j)
	{
		if(candidatos[j].value > max)
		{
			return false;
		}
		max = candidatos[j].value;
		//std::cout << candidatos[j].value << std::endl;

	}
	return true;
}

bool pruebaRapida()
{
	Candidates candidatos;
	candidatos.resize(20);
	int inicio = 0;
	int fin = 19;
	double max = 3000;
	bool correcto = true;

	for(int i = 0; i < 20; ++i)
	{
		candidatos[i].value = rand() % (50+1);
	}
	//std::cout << "ORDENACION RAPIDA" << std::endl;
	ordenacionRapida(candidatos,inicio,fin);
	for(int j = 0; j < 20; ++j)
	{
		if(candidatos[j].value > max)
		{
			return false;
		}
		
		max = candidatos[j].value;
		//std::cout << candidatos[j].value << std::endl;
		
	}
	return true;
}

bool pruebaMonticulo()
{
	Candidates candidatos;
	candidatos.resize(20);
	double max = 3000;
	bool correcto = true;

	for(int i = 0; i < 20; ++i)
	{
		candidatos[i].value = rand() % (50+1);
	}
	//std::cout << "ORDENACION MONTICULO" << std::endl;
	ordenacionMonticulo(candidatos);
	for(int j = 0; j < 20; ++j)
	{
		if(candidatos[j].value > max)
		{
			return false;
		}
		//std::cout << candidatos[j].value << std::endl;
		max = candidatos[j].value;
	}
	return true;
}


void DEF_LIB_EXPORTED placeDefenses3(bool** freeCells, int nCellsWidth, int nCellsHeight, float mapWidth, float mapHeight
              , List<Object*> obstacles, List<Defense*> defenses) {

	
	std::cout << "PRINCIPIO DEL ALGORITMO "<< std::endl;
    float cellWidth = mapWidth / nCellsWidth;
    float cellHeight = mapHeight / nCellsHeight;
	const double e_abs = 0.01;
    const double e_rel = 0.1;
	double tiempos[4]; 

	//bool b1 = pruebaFusion();
	//bool b2 = pruebaRapida();
	//bool b3 = pruebaMonticulo();

	//std::cout << "RESULTADOS PRUEBAS: " << b1 << " " << b2 << " " << b3 << std::endl;

	//ALGORITMO SIN PREORDENACION
	cronometro c;
    long int r = 0;
    c.activar();
    do {	
		int maxAttemps = 1000;
		List<Defense*>::iterator currentDefense = defenses.begin();
    	Candidates candidatesDE = candidatesD(freeCells, nCellsWidth, nCellsHeight, mapWidth, mapHeight, obstacles, defenses);
		while(currentDefense != defenses.end() && maxAttemps > 0 && !candidatesDE.empty())
		{

      		int i = buscaMaximo(candidatesDE);
     		if(funfact((*currentDefense)->radio,candidatesDE[i].x,candidatesDE[i].y,mapWidth / nCellsWidth,mapWidth / nCellsWidth,mapWidth,mapHeight,obstacles,defenses))
			{
				(*currentDefense)->position = CellToPos(candidatesDE[i].x,candidatesDE[i].y,mapWidth / nCellsWidth,mapHeight / nCellsHeight);
				++currentDefense;
        		--maxAttemps;
      		}
			else {--maxAttemps;}

			candidatesDE.erase(candidatesDE.begin() + i);
		
		}
		++r;
    } while(c.tiempo() < e_abs/(e_rel+e_abs));
    c.parar();
	tiempos[0] = c.tiempo()/r;

	r = 0;
	std::cout << "Pasa sin preordenacion" << std::endl;

	//ALGORITMO CON PREORDENACION FUSION
	c.activar();
    do {	
		int maxAttemps = 1000;
		List<Defense*>::iterator currentDefense = defenses.begin();
    	Candidates candidatesDE = candidatesD(freeCells, nCellsWidth, nCellsHeight, mapWidth, mapHeight, obstacles, defenses);
		ordenacionFusion(candidatesDE,0,candidatesDE.size());
		while(currentDefense != defenses.end() && maxAttemps > 0 && !candidatesDE.empty())
		{
      		
			if(funfact((*currentDefense)->radio,candidatesDE[0].x,candidatesDE[0].y,mapWidth / nCellsWidth,mapWidth / nCellsWidth,mapWidth,mapHeight,obstacles,defenses))
			{
				(*currentDefense)->position = CellToPos(candidatesDE[0].x,candidatesDE[0].y,mapWidth / nCellsWidth,mapHeight / nCellsHeight);
				++currentDefense;
        		--maxAttemps;
      		}
			else {--maxAttemps;}

			candidatesDE.erase(candidatesDE.begin());
		}
		++r;
    } while(c.tiempo() < e_abs/(e_rel+e_abs));
    c.parar();
	tiempos[1] = c.tiempo()/r;

	r = 0;
	std::cout << "Pasa fusion" << std::endl;

	//ALGORITMO CON PREORDENACION RAPIDA
	c.activar();
    do {	
		int maxAttemps = 1000;
		List<Defense*>::iterator currentDefense = defenses.begin();
    	Candidates candidatesDE = candidatesD(freeCells, nCellsWidth, nCellsHeight, mapWidth, mapHeight, obstacles, defenses);
		ordenacionRapida(candidatesDE,0,candidatesDE.size());
		while(currentDefense != defenses.end() && maxAttemps > 0 && !candidatesDE.empty())
		{
			if(funfact((*currentDefense)->radio,candidatesDE[0].x,candidatesDE[0].y,mapWidth / nCellsWidth,mapWidth / nCellsWidth,mapWidth,mapHeight,obstacles,defenses))
			{
				(*currentDefense)->position = CellToPos(candidatesDE[0].x,candidatesDE[0].y,mapWidth / nCellsWidth,mapHeight / nCellsHeight);
				++currentDefense;
        		--maxAttemps;
      		}
			else {--maxAttemps;}

			candidatesDE.erase(candidatesDE.begin());
		}
		++r;
    } while(c.tiempo() < e_abs/(e_rel+e_abs));
    c.parar();
	tiempos[2] = c.tiempo()/r;

	r = 0;
	std::cout << "Pasa rapida" << std::endl;

	//ALGORITMO CON PREORDENACION MONTICULO
	c.activar();
    do {	
		int maxAttemps = 1000;
		List<Defense*>::iterator currentDefense = defenses.begin();
    	Candidates candidatesDE = candidatesD(freeCells, nCellsWidth, nCellsHeight, mapWidth, mapHeight, obstacles, defenses);
		ordenacionMonticulo(candidatesDE);
		while(currentDefense != defenses.end() && maxAttemps > 0 && !candidatesDE.empty())
		{
			if(funfact((*currentDefense)->radio,candidatesDE[0].x,candidatesDE[0].y,mapWidth / nCellsWidth,mapWidth / nCellsWidth,mapWidth,mapHeight,obstacles,defenses))
			{
				(*currentDefense)->position = CellToPos(candidatesDE[0].x,candidatesDE[0].y,mapWidth / nCellsWidth,mapHeight / nCellsHeight);
				++currentDefense;
        		--maxAttemps;
      		}
			else {--maxAttemps;}

			candidatesDE.erase(candidatesDE.begin());
		}
		++r;
    } while(c.tiempo() < e_abs/(e_rel+e_abs));
    c.parar();
	tiempos[3] = c.tiempo()/r;

	std::cout << "Pasa monticulo" << std::endl;


	/*std::cout<<"TIEMPO SIN ORDENAR: "<<tiempos[0]<<" s."<<std::endl;
    std::cout<<"TIEMPO FUSION:      "<<tiempos[1]<<" s."<<std::endl;
	std::cout<<"TIEMPO O.RAPIDA:    "<<tiempos[2]<<" s."<<std::endl;
	std::cout<<"TIEMPO MONTICULO:    "<<tiempos[3]<<" s."<<std::endl;*/

	std::cout << (nCellsWidth * nCellsHeight) << '\t' << tiempos[0] << '\t' << tiempos[1] << '\t' << tiempos[2] << '\t' << tiempos[3] << std::endl;




	
}

