/*
* JNP, zadanie 5
* Łukasz Dunaj [indeks TODO]
* Michał Woś mw336071
*/
#ifndef __VIRUS_GENEALOGY_H
#define __VIRUS_GENEALOGY_H

#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <string>

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

/* ===================== NODE ================================================*/

template <class Virus>
class VirusGenealogy;

// Wierzcholek w grafie - "opakowanie" na wirusa
template<class Virus>
class Node
{
	friend class VirusGenealogy<Virus>;
private:
	Virus virus;
	// iterator do grafu
	typedef typename std::set <Node<Virus> >::iterator graph_it;
	typedef typename Virus::id_type id_type;
	// komparator do porównywania iteratorów w setach ojców / dzieci
	struct classcomp
	{
		bool operator() ( const graph_it node1, const graph_it node2 ) const
		{
			return node1->get_id() < node2->get_id();
		}
	};
	// w secie trzymamy iteratory do grafu (czyli set'a wszystkich wieszchołków)
	// dzieki temu oszczedzamy log(n) na wyszkuiwaniu pojedynczego ojca / syna
	// np podczas usuwania
	std::set <graph_it, classcomp> _parents;
	std::set <graph_it, classcomp> _children;

public:
	Node( Virus v ) noexcept : virus( v )
	{}

	Virus& get_virus() const noexcept
	{
		return virus;
	}

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
			res.push_back( child->get_id() );
		}
		return res;
	}

	vector<id_type> get_parents_ids() const
	{
		std::vector<id_type> res;
		for ( auto parent : _parents ) {
			res.push_back( parent->get_id() );
		}
		return res;
	}

	/*void deleteNode( )
	{
		//Usuwa wierzchołek z listy ojców swoich synów
		for ( auto v : _children ) {
			v->_parents.erase(  );
		}
		//Usuwa wierzchołek z listy synów swoich ojców
		for ( auto v : parent ) {
			v->descendants.erase(this->shared_from_this( ) );
		}
	}*/
};


/* ===================== VIRUS GENEALOGY =====================================*/

template <class Virus>
class VirusGenealogy
{
	typedef typename Virus::id_type id_type;

	// komparator do grafu (ktory jest set'em), porownuje wierzcholki wzgledem
	// id wirusa
	struct classcomp
	{
		bool operator() ( const Node<Virus> &lhs, const Node<Virus> &rhs ) const
		{
			return lhs.get_id() < rhs.get_id();
		}
	};

	typedef std::set <Node<Virus>, classcomp> graph_type;
	graph_type graph;
	typedef typename graph_type::iterator graph_it;
	id_type rootId;

public:
	// Tworzy nową genealogię.
	// Tworzy także węzęł wirusa macierzystego o identyfikatorze stem_id.
	VirusGenealogy( id_type const &stem_id ) noexcept
	{
		graph.insert( Node<Virus>( stem_id ) );
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
		// wyszukuje wirusa o id tworzac tymczasowego wirusa o id
		graph_it node = graph.find( Node<Virus>(id) );
		if ( node == graph.end() ) {
			throw VirusNotFound();
		}
		return node->get_children_ids();
	}

	// Zwraca listę identyfikatorów bezpośrednich poprzedników wirusa
	// o podanym identyfikatorze.
	// Zgłasza wyjątek VirusNotFound, jeśli dany wirus nie istnieje.
	std::vector<id_type> get_parents( id_type const &id ) const
	{
		graph_it node = graph.find( Node<Virus>(id) );
		if ( node == graph.end() ) {
			throw VirusNotFound();
		}
		return node->get_parents_ids();	}

	// Sprawdza, czy wirus o podanym identyfikatorze istnieje.
	bool exists( id_type const &id ) const noexcept
	{
		return graph.find( Node<Virus>(id) ) != graph.end();
	}

	// Zwraca referencję do obiektu reprezentującego wirus o podanym
	// identyfikatorze.
	// Zgłasza wyjątek VirusNotFound, jeśli żądany wirus nie istnieje.
	Virus& operator[]( id_type const &id ) const
	{
		graph_it node = graph.find( Node<Virus>(id) );
		if ( node == graph.end() ) {
			throw VirusNotFound();
		}
		return node->get_virus();
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
		if ( graph.find( Node<Virus>(id) ) != graph.end() ) {
			throw VirusAlreadyCreated();
		}
		// sprawdzamy czy wszyscy ojcowie istnieja
		size_t n = parent_ids.size();
		graph_it parents_it[n];
		for ( size_t i = 0; i < n; i++ ) {
			parents_it[i] = graph.find( Node<Virus>( i ) );
			if ( parents_it[i] == graph.end() ) {
				throw VirusNotFound();
			}
		}
		// stworzenie nowego wezla o id w grafie
		// first daje nam iterator na umieszczony w grafie element
		graph_it newNode = graph.insert( Node<Virus>( id ) ).first;

		for ( size_t i = 0; i < n; i++ ) {
			// przypisanie mu podanych ojcow
			newNode->add_parent( parents_it[i] );
			// a ojcom dodanie dziecka - obecnego, nowego wezla
			parents_it[i]->add_children( newNode );
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
		graph_it child = graph.find( Node<Virus>( child_id ) );
		if ( child == graph.end() ) {
			throw VirusNotFound();
		}
		graph_it parent = graph.find( Node<Virus>( parent_id ) );
		if ( parent == graph.end() ) {
			throw VirusNotFound();
		}
		child->add_parent( parent );
		parent->add_children( child );
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
		graph_it node = graph.find( Node<Virus>(id) );
		if ( node == graph.end() ) {
			throw VirusNotFound();
		}
		remove(node);
	}

private:
	void remove( graph_it node )
	{
		// usuwamy wierzcholek z listy dzieci jego ojcow
		while ( !node->_parents.empty() ) {
			graph_it parent = node->_parents.begin();
			parent->_children.erase( node );
			node->_parents.erase( parent );
		}
		// usuwanie wierzcholek z listy ojcow jego dzieci
		while ( !node->_children.empty() ) {
			graph_it child = node->_children.begin();
			child->_parents.erase( node );
			// jesli spowodowalo to odciecie ostatniego poprzednika, usuwamy
			// syna z grafu
			if ( child->_parents.empty() ) {
				remove( child );
			}
		}
		// ojcowie oraz synowie zaktualizowani, wiec mozemy usunac z grafu
		graph.erase( node );
	}
};

#endif
