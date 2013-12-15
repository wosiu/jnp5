/*
* JNP, zadanie 5
* Łukasz Dunaj [indeks TODO]
* Michał Woś mw336071
*/
#ifndef __VIRUS_GENEALOGY_H
#define __VIRUS_GENEALOGY_H

#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <iostream>

/* Drogi Łukaszu! Informacje o stylu, jakiego używałem poniżej:
1. po nawiasie i przed nawiasem zawsze spacja, no chyba że puste ()
2. 80 znaków w lini i ani źdźbłą wincej! :P
3.
if/for/while/switch[spacja]([spacja]warunek[spacja])[spacja]{
	instrukcje
}
klamry {} robimy nawet jak jedna instrukcja
4. class, fucnkcja, metoda - klamra rozpoczynajaca w nowej lini w przeciwienstwie
do if/for/while/switch
*/

// TODO jak bedize skonczone usunac to i dodac wszedzie gdzie trzeba std::*
using namespace std;

/* ===================== WYJATKI =============================================*/

class VirusNotFound: public std::exception
{
    virtual const char* what() const noexcept
    {
        return "VirusNotFound";
    }
};

class VirusAlreadyCreated: public std::exception
{
    virtual const char* what() const noexcept
    {
        return "VirusAlreadyCreated";
    }
};

class TriedToRemoveStemVirus: public std::exception
{
    virtual const char* what() const noexcept
    {
        return "TriedToRemoveStemVirus";
    }
};


/* ===================== VIRUS GENEALOGY =====================================*/

template <class Virus>
class VirusGenealogy
{
public:
	typedef typename Virus::id_type id_type;
	id_type rootId;
	struct Node;
	// komparator do grafu (ktory jest set'em), porownuje wierzcholki wzgledem
	// id wirusa
	#define node second
	typedef std::map <id_type, Node> graph_type;
	graph_type graph;
	typedef typename graph_type::iterator graph_it;

	/* ===================== NODE ============================================*/
	struct Node
	{
		Virus virus;
		// komparator do porównywania iteratorów w setach ojców / dzieci
		struct classcomp
		{
			bool operator() ( const graph_it node1, const graph_it node2 ) const
			{
				return node1->node.get_id() < node2->node.get_id();
			}
		};
		// w secie trzymamy iteratory do grafu (czyli set'a wszystkich wieszchołków)
		// dzieki temu oszczedzamy log(n) na wyszkuiwaniu pojedynczego ojca / syna
		// np podczas usuwania
		std::set <graph_it, classcomp> _parents;
		std::set <graph_it, classcomp> _children;

		Node( id_type const &id ) noexcept : virus( Virus(id) )
		{
		}

		/*
		Virus& get_virus() noexcept
		{
			return virus;
		}
	*/
		typename Virus::id_type get_id() const
		{
			return virus.get_id();
		}

		void add_parent( graph_it parent )
		{
			_parents.insert( parent );
		}

		void add_children( graph_it children )
		{
			_children.insert( children );
		}

		vector<id_type> get_children_ids() const
		{
			std::vector<id_type> res;
			for ( auto child : _children ) {
				res.push_back( child->node.get_id() );
			}
			return res;
		}

		vector<id_type> get_parents_ids() const
		{
			std::vector<id_type> res;
			for ( graph_it parent : _parents ) {
				res.push_back( parent->node.get_id() );
			}
			return res;
		}
	};

/*	void bug()
	{
		graph.insert( make_pair( "bla", Node( "bla" ) ) );
		cout << graph.size() << endl;
		graph_it node1,node2;
		node1 = graph.begin();
		node2 = graph.begin();
		node2++;
		cout << node1->second.get_id() << endl;
		cout << node2->second.get_id() << endl;
		cout << node1->second.tmp.size() << endl;

		//vector<graph_it>bla;
		//bla.push_back(node2);
		// i niby to samo robie ponizej, tylko wewnatrz node1, a sie wyklada:
		node1->second.tmp.push_back( node2 );
		node1->second.tmp_int = 42;
		//graph_it = graph.begin();
		//newNode->_parents.insert( graph.begin() );

		Node lokalny_node("asd");
		lokalny_node.tmp_int = 42;
	}
*/


//public:
	// Tworzy nową genealogię.
	// Tworzy także węzęł wirusa macierzystego o identyfikatorze stem_id.
	VirusGenealogy( id_type const &stem_id ) noexcept
	{
		graph.insert( make_pair( stem_id, Node( stem_id ) ) );
		rootId = stem_id;
	}

	// Zwraca identyfikator wirusa macierzystego.
	id_type get_stem_id() const noexcept
	{
		return rootId;
	}

	// Zwraca listę identyfikatorów bezpośrednich następników wirusa
	// o podanym identyfikatorze.
	// Zgłasza wyjątek VirusNotFound, jeśli dany wirus nie istnieje.
	std::vector<id_type> get_children( id_type const &id ) const
	{
		auto it = graph.find( id );
		if ( it == graph.end() ) {
			throw VirusNotFound();
		}
		return it->node.get_children_ids();
	}

	// Zwraca listę identyfikatorów bezpośrednich poprzedników wirusa
	// o podanym identyfikatorze.
	// Zgłasza wyjątek VirusNotFound, jeśli dany wirus nie istnieje.
	std::vector<id_type> get_parents( id_type const &id ) const
	{
		auto it = graph.find( id );
		if ( it == graph.end() ) {
			throw VirusNotFound();
		}
		return it->node.get_parents_ids();
	}

	// Sprawdza, czy wirus o podanym identyfikatorze istnieje.
	bool exists( id_type const &id ) const noexcept
	{
		return graph.find( id ) != graph.end();
	}

	// Zwraca referencję do obiektu reprezentującego wirus o podanym
	// identyfikatorze.
	// Zgłasza wyjątek VirusNotFound, jeśli żądany wirus nie istnieje.
	// TODO: ma zwracac referencje ( Virus& ) a nie kopie obiektu..
	// tyle ze z referencja daje blad kompilacji
	// "If the map object is const-qualified, the function returns a const_iterator. Otherwise, it returns an iterator."
	// no i u nas jest wlasnie to nieszczesne "const-qualified"
	Virus operator[]( id_type const &id ) const
	{
		auto it = graph.find( id );
		if ( it == graph.end() ) {
			throw VirusNotFound();
		}
		return it->node.virus;
	}
	// Tworzy węzęł reprezentujący nowy wirus o identyfikatorze id
	// powstały z wirusów o podanym identyfikatorze parent_id lub
	// podanych identyfikatorach parent_ids.
	// Zgłasza wyjątek VirusAlreadyCreated, jeśli wirus o identyfikatorze
	// id już istnieje.
	// Zgłasza wyjątek VirusNotFound, jeśli któryś z wyspecyfikowanych
	// poprzedników nie istnieje.
	void create( id_type const &id, std::vector<id_type> const &parent_ids )
	{
		if ( parent_ids.empty() ) {
			throw VirusNotFound();
		}
		// sprawdzamy, czy wierzcholek nie istnial wczesniej
		if ( graph.find( id ) != graph.end() ) {
			throw VirusAlreadyCreated();
		}
		// sprawdzamy czy wszyscy ojcowie istnieja
		size_t n = parent_ids.size();
		graph_it parents_it[n];
		for ( size_t i = 0; i < n; i++ ) {
			parents_it[i] = graph.find( parent_ids[i] );
			if ( parents_it[i] == graph.end() ) {
				throw VirusNotFound();
			}
		}
		// stworzenie nowego wezla o id w grafie
		// first daje nam iterator na umieszczony w grafie element
		graph_it newNode = graph.insert( make_pair( id, Node( id ) ) ).first;

		for ( size_t i = 0; i < n; i++ ) {
			// przypisanie mu podanych ojcow
			newNode->node.add_parent( parents_it[i] );
			// a ojcom dodanie dziecka - obecnego, nowego wezla
			parents_it[i]->node.add_children( newNode );
		}
	}

	void create( id_type const &id, id_type const &parent_id )
	{
		create( id, std::vector<id_type>( { parent_id } ) );
	}

	// Dodaje nową krawędź w grafie genealogii.
	// Zgłasza wyjątek VirusNotFound, jeśli któryś z podanych wirusów nie istnieje.
	void connect( id_type const &child_id, id_type const &parent_id )
	{
		graph_it child = graph.find( child_id );
		if ( child == graph.end() ) {
			throw VirusNotFound();
		}
		graph_it parent = graph.find( parent_id );
		if ( parent == graph.end() ) {
			throw VirusNotFound();
		}
		child->node.add_parent( parent );
		parent->node.add_children( child );
	}

	// Usuwa wirus o podanym identyfikatorze.
	// Zgłasza wyjątek VirusNotFound, jeśli żądany wirus nie istnieje.
	// Zgłasza wyjątek TriedToRemoveStemVirus przy próbie usunięcia
	// wirusa macierzystego.
	void remove( id_type const &id )
	{
		if ( id == rootId ) {
			throw TriedToRemoveStemVirus();
		}
		graph_it it = graph.find( id );
		if ( it == graph.end() ) {
			throw VirusNotFound();
		}
		remove( it );
	}

private:
	void remove( graph_it it )
	{
		// usuwamy wierzcholek z listy dzieci jego ojcow
		while ( !it->node._parents.empty() ) {
			// bierzemy pierwszy iterator z set'a ojcow
			graph_it parent = *( it->node._parents.begin() );
			parent->node._children.erase( it );
			it->node._parents.erase( parent );
		}
		// usuwanie wierzcholek z listy ojcow jego dzieci
		while ( !it->node._children.empty() ) {
			graph_it child = *( it->node._children.begin() );
			child->node._parents.erase( it );
			it->node._children.erase( child );
			// jesli spowodowalo to odciecie ostatniego poprzednika, usuwamy
			// syna z grafu
			if ( child->node._parents.empty() ) {
				remove( child );
			}
		}
		// ojcowie oraz synowie zaktualizowani, wiec mozemy usunac z grafu
		graph.erase( it );
	}
/**/
};

#endif
