#include "Ara.h"
//using namespace std;
/********************************************************************
*        METHODS                                                    *
********************************************************************/
Ara* ara;

Ara::Ara(string co_filename, string dist_filename) : PI(3.14159265), DG_TO_RAD(PI / 180.0), INFINITO(std::numeric_limits<Edge_Weight>::max() / 2)
{
	ara = this;
	load(co_filename, dist_filename);
}

void Ara::initARA(){
	for(int i = 0; i <= num_nodes; i++)
	{
		f[i] = g[i] = v[i] = INFINITO;
		set[i] = NONE;
	}
}
// initialize diferente do AD
void Ara::initialize(){
	edge_array = new Edge[num_edges]();;
	weights = new Edge_Weight[num_edges]();
	x = new double[num_nodes+1]();
	y = new double[num_nodes+1]();

	f = new Edge_Weight[num_nodes+1]();
	g = new Edge_Weight[num_nodes+1]();
	v = new Edge_Weight[num_nodes+1]();
	backPointer = new int[num_nodes+1]();
	set = new Sets[num_nodes+1]();
	open = new PriorityQueue<Edge_Weight>(num_nodes+1, f);
}
void Ara::initializeEdges(){
	graph = new G(edge_array, edge_array + num_edges, weights, num_nodes);
}
void Ara::load(string co_filename, string dist_filename){

	cout << "carregando arquivo de distancias" << endl;
	ifstream dist_file(dist_filename.c_str());

	char c[100];
	int i = 0;
	int a,b;
	int u,v;
	vector< vector < int > > edges;

	while( dist_file.good() ){
		dist_file >> c;
		if( c[0] == 'p'){
			dist_file >> c >> num_nodes >> num_edges;
			initialize();
			edges.resize(num_edges);
		}
		else if( c[0] == 'a'){
			bool adiciona = true;
			dist_file >> u >> v >> weights[i];

			for(unsigned int k = 0; k < edges[u].size(); k++)
			{
				if( edges[u][k] == v )
					adiciona = false;
			}

			if( adiciona ){
				edge_array[i].first = u;
				edge_array[i].second = v; 
				i++;
				edges[u].push_back(v);
			}
		}
		else{
			dist_file.getline(c,100);
		}
	}
	dist_file.close();
	initializeEdges();
	cout << "carregando arquivo de coordenadas" << endl;
	ifstream co_file(co_filename.c_str());

	while( co_file.good() ){
		co_file >> c;
		if( c[0] == 'v'){
			co_file >> i >> a >> b;
			
#ifdef DIST_EUCLIDIANA
			x[i] = a;
			y[i] = b;
#else       // transforma em graus, depois em radianos
			x[i] = ((double) a) / 1000000.0; 
			x[i] *= DG_TO_RAD;
			y[i] = ((double) b) / 1000000.0;
			y[i] *= DG_TO_RAD;
#endif
		}
		else{
			co_file.getline(c,100);
		}
	}
	co_file.close();
}


double Ara::h(int id, const int goal){
#ifdef DIST_EUCLIDIANA
	return sqrt( pow(x[id] - x[goal],2) + pow(y[id] - y[goal],2));
#else
	double d = acos((sin(y[id])*sin(y[goal]))+(cos(y[id])*cos(y[goal])*cos(x[id] - x[goal])));
	return 63787000.0 * d;
#endif
}

/********************************************************************
*        ARA*                                                       *
********************************************************************/
double Ara::key(int id, const int goal, const double &e)
{
	if( g[id] == INFINITO )
		return f[id] = INFINITO; 
	
	f[id] = g[id] + (Edge_Weight)( e * h(id, goal) ) ;
	return f[id];
}


void Ara::update_open(const int goal, const double &e){
	for(int i = 0; i <= num_nodes; i++)
	{
		if(	set[i] == OPEN){
			key(i,goal,e);
			open->decrease(i);
		} 
	}
}
void Ara::empty_closed(){
	for(int i = 0; i <= num_nodes; i++)
	{
		if(	set[i] == CLOSED){
			set[i] = NONE;
		} 
	}
}
void Ara::move_incons_open(){
	while( ! incons.empty() )
	{
		int v = incons.front();
		open->push(v);
		set[v] = OPEN;
		incons.pop_front();			
	}
}
void Ara::greedyPath(const int &start, const int &goal)
{
	double custo = 0;
	for( int v = goal; v != start;)
	{
		out_edge_iterator vi,vend;
		double minG = INFINITO;
		int argMinG;
		double minCost;
		for(tie(vi,vend) = out_edges(v, *graph); vi != vend; vi++)
		{		
			if((g[(target(*vi,*graph))] + get(edge_weight, *graph, *vi)) < minG)
			{
				minCost = get(edge_weight, *graph, *vi);
				minG = g[(target(*vi,*graph))] + minCost;
				argMinG = (target(*vi,*graph));
			}
		}
		custo += minCost;
		//cout << argMinG << " - ";
		v = argMinG;
	}
	cout << custo;
}

void Ara::computePath(const double &e, int start, int goal)
{

	int min_id = open->key_top();
	while(key(goal, goal, e) > key(min_id, goal, e) )
	{

		int s = open->key_top();
		open->pop();

		set[s] = CLOSED;
#ifdef DEBUG
		estadosChecados++;
#endif
		out_edge_iterator vi,vend;
		for(tie(vi,vend) = out_edges(s, *graph); vi != vend; vi++)
		{			

			int custo = get(edge_weight, *graph, *vi);
			vertex_iterator p1 = target(*vi, *graph);		
			int id_neighbour = *p1;
			if( g[id_neighbour] > g[s] + custo)
			{
				g[id_neighbour] = g[s] + custo;
				
				if( set[id_neighbour] != CLOSED )
				{
					//recalcula f
					key(id_neighbour,goal,e);
					if( set[id_neighbour] != OPEN )
						open->push(id_neighbour);
					else
						open->decrease(id_neighbour);
					set[id_neighbour] = OPEN;
				}
				else
				{
					key(id_neighbour,goal,e);
					set[id_neighbour] = INCONS;
					incons.push_back(id_neighbour);
				}
			}
		}
		min_id = open->key_top(); // recupera o minimo para teste do while
	}
	// se o estado objetivo foi adicionado ao OPEN ele deve ser removido
	if( min_id == goal ){
		open->pop();
		set[goal] = NONE;
	}
}
bool firstPubl;
double aTime;
double dijTime;
double adijt;
double accTimer[11];

double valores[] = { 0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};


/***************************************************************
*	ARA
*	start = nodo inicial
*	goal  = nodo final
*	e0    = multiplicador da heuristica inicial 
*	delta = Velocidade da diminui��o de e0 a cada passo

ps modifiquei um pouco o publish solution e o main pode nao funcionar!
cabe ressaltar que a fun��o construtora deve ser re-estruturada
***************************************************************/

static mt19937 gen((boost::uint32_t)time(0));

void Ara::main(int start, int goal, double e0, double delta){

	if(start == 0)
		start = (gen()%num_nodes) + 1;
	if(goal == 0)
		goal = (gen()%num_nodes) + 1;

	firstPubl = true;
	initARA();

#ifdef DEBUG
	iter = 1;
	cout << "rodando algoritmo" << endl;
	estadosChecados = 0;
	lTimer.start();
#endif

	double e = e0;

	g[goal] = INFINITO;
	g[start] = 0;

	key(start,goal,e);
	set[start] = OPEN;
	open->push(start);
	
	computePath(e,start,goal);
#ifdef DEBUG
	publishSolution(iter,e,start,goal);
#endif
	while( e > 1 )
	{
		e = e - delta;

		move_incons_open();
		update_open(goal,e);
		empty_closed();

		computePath(e,start,goal);
#ifdef DEBUG		
		publishSolution(iter,e,start,goal);
#endif
	}
}
/********************************************************************
*        SPEED UP TEST                                              *
********************************************************************/
int accEstados;
double en[] = { 1.0, 1.4, 1.8, 2.2, 2.6, 3.0, 3.4, 3.8, 4.2, 4.6, 5.0 };
double mini[13], maxi[13];
void Ara::publishSolution(int &iter, int et, int start, int goal, int repeat)
{
	lTimer.stop();
	if( firstPubl )
	{
		firstPubl = false;
		astar(start,goal,false);	
		dijkstra(start,goal);
		adijt += (aTime / dijTime);
	}
	double thisTime = aTime / (lTimer.show()/repeat);
	mini[et] = min(mini[et],thisTime);
	maxi[et] = max(maxi[et],thisTime);
	valores[et] += thisTime;

	if(iter == repeat && (et == 10)){
		cout << ( adijt / repeat ) << endl;
		for(int ei = 0; ei < 11; ei++){
			//sort(valores[ei].begin(), valores[ei].end());
			//x minimum lower-quartile median upper-quartile maximum
			//int size = valores[ei].size();
			cout << en[ei] << " " << mini[ei] << " " << valores[ei]/repeat << " " << maxi[ei] << endl;
			
		}
	}
	//greedyPath(start,goal);
	estadosChecados = 0;

	lTimer.start();
}

// abaixo teste usando mediana

//void Ara::publishSolution(int &iter, int et, int start, int goal, int max)
//{
//	lTimer.stop();
//
//	if( firstPubl )
//	{
//		firstPubl = false;
//		astar(start,goal,false);	
//		dijkstra(start,goal);
//		adijt += (aTime / dijTime);
//	}
//	valores[et].push_back(aTime / (lTimer.show() + 0.001));
//	//accTimer[et] += lTimer.show();
//
//	if(iter == max && (et == 10)){
//		cout << ( adijt / max ) << endl;
//		for(int ei = 0; ei < 11; ei++){
//			sort(valores[ei].begin(), valores[ei].end());
//			//x minimum lower-quartile median upper-quartile maximum
//			int size = valores[ei].size();
//			cout << en[ei] << " " <<  valores[ei][0] << " " << valores[ei][size/4] << " " << valores[ei][size/2] << " " << valores[ei][(size*3)/4] << " " << valores[ei][size-1] << endl;
//			
//		}
//	}
//	//greedyPath(start,goal);
//	estadosChecados = 0;
//
//	lTimer.start();
//}

//speed up test com m�dia
void Ara::araSpeedUpTest(int repeat){
	int start,goal;
	
	adijt = 0;
	for( int i = 0; i < 11; i++ ){
		 mini[i] = INFINITO;
		 maxi[i] = 0.0;
	}
	for(iter = 1; iter <= repeat; iter++){
		do{
			start = (gen()%num_nodes) + 1;
			goal = (gen()%num_nodes) + 1;
		}while( start == goal );

		firstPubl = true;

		for(int et = 0; et < 11; et++){
			lTimer.start();
			// repetir "repeat" vezes para eliminar erros de relogio
			for(int t = 1; t <= repeat; t++){
				estadosChecados = 0;
				open->clear();
				
				initARA();
				
				g[goal] = INFINITO;
				g[start] = 0;

				key(start,goal,en[et]);
				set[start] = OPEN;
				open->push(start);

				computePath(en[et],start,goal);
			}
			publishSolution(iter,et,start,goal,repeat);
		}
	}
}

// abaixo o speed up test com mediana

//void Ara::araSpeedUpTest(int repeat){
//	int start,goal;
//	
//	adijt = 0;
//	//for( int i = 0; i < 11; i++ )
//	//	accTimer[i] = 0.0001;
//
//	for(iter = 1; iter <= repeat; iter++){
//		do{
//			start = (gen()%num_nodes) + 1;
//			goal = (gen()%num_nodes) + 1;
//		}while( start == goal );
//
//		firstPubl = true;
//
//		for(int et = 0; et < 11; et++){
//			estadosChecados = 0;
//			open->clear();
//			lTimer.start();
//			initARA();
//			
//			g[goal] = INFINITO;
//			g[start] = 0;
//
//			key(start,goal,en[et]);
//			set[start] = OPEN;
//			open->push(start);
//
//			computePath(en[et],start,goal);
//
//			publishSolution(iter,et,start,goal,repeat);
//		}
//	}
//}
/********************************************************************
*        DIJKSTRA                                                   *
********************************************************************/
void Ara::dijkstra(int start, int goal){
	std::vector<vertex_descriptor> p(num_vertices(*graph));
	std::vector<int> d(num_vertices(*graph)+1);
	vertex_descriptor s = vertex(start,*graph);
	
	Timer dTimer;
	dTimer.start();
	dijkstra_shortest_paths(*graph, s, predecessor_map(&p[0]).distance_map(&d[0]));
	dTimer.stop();

	//int custo = 0;
	//for( int v = goal; v != start;)
	//{
	//	out_edge_iterator vi,vend;
	//	int minG = numeric_limits<int>::max();
	//	int argMinG;
	//	int minCost;

	//	for(tie(vi,vend) = out_edges(v, *graph); vi != vend; vi++)
	//	{		

	//		if((d[(target(*vi,*graph))] + get(edge_weight, *graph, *vi) < minG))
	//		{
	//			minCost = get(edge_weight, *graph, *vi);
	//			minG = d[(target(*vi,*graph))] + minCost;
	//			argMinG = (target(*vi,*graph));	
	//		}
	//	}
	//	custo += minCost;
	//	v = argMinG;
	//}
	dijTime = dTimer.show();
	//cout << dTimer << ";";
}
/********************************************************************
*        A*                                                         *
********************************************************************/

int aNodesVisited;

template <class Graph, class CostType>
class distance_heuristic : public astar_heuristic<Graph, CostType>
{
public:
  typedef typename graph_traits<Graph>::vertex_descriptor Vertex;
  distance_heuristic(Vertex goal)
    : m_goal(goal) {}
  CostType operator()(Vertex u)
  {
    return (CostType) ara->h(u,m_goal);
  }
private:
  int m_goal;
};

struct found_goal {}; // exception for termination

// visitor that terminates when we find the goal
template <class Vertex>
class astar_goal_visitor : public boost::default_astar_visitor
{
public:
  astar_goal_visitor(Vertex goal) : m_goal(goal) {}
  template <class Graph>
  void examine_vertex(Vertex u, Graph& g) {
	aNodesVisited++;
	if(u == m_goal)
      throw found_goal();
  }
private:
  Vertex m_goal;
};


Ara::Edge_Weight Ara::astar(int start, int goal, bool verbose){
	std::vector<vertex_descriptor> p(num_vertices(*graph));
	std::vector<Edge_Weight> d(num_vertices(*graph)+1);
	vertex_descriptor s = vertex(start,*graph),
					  g = vertex(goal,*graph);
	
	Timer timer;
	timer.start();

	aNodesVisited = 0;

	try{
	astar_search
      (*graph, s, distance_heuristic<G, Edge_Weight> (goal),
       predecessor_map(&p[0]).distance_map(&d[0]).
       visitor(astar_goal_visitor<vertex_descriptor>(g)));
	  } catch(found_goal) {
#ifdef SHOW_PATH		
		cout << "A*  : " ;

		bool detec = false;
		for(int t = goal, i = 0; t != start; t = p[t])
		{
			if( ! detec &&  (t != ad::path[i++])){
				cout << endl << "erro => " << t << " != " << ad::path[i-1]<< endl;
				detec = true;
			}
			cout << t << " -> ";
		}
#endif  

		//if( ad::eg <= 1 ){
		//	for(int i = 0; i <= num_nodes; i++)
		//	{
		//		if( h(start,i) > d[i] )
		//			exit(-9);
		//	}
		//}

		timer.stop();	
		aTime = timer.show();
		if(verbose)
		{
			cout << aNodesVisited << ";" << timer << ";";
		}
	}
	return d[goal];
}