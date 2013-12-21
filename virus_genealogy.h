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
#include <memory>
#include <cassert>

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
	shared_ptr <Node>rootNode;
	// komparator do grafu (ktory jest set'em), porownuje wierzcholki wzgledem
	// id wirusa
	#define node second
	typedef Node* parent_type;
	typedef std::map <id_type, parent_type> graph_type;
	graph_type graph;
	typedef typename graph_type::iterator graph_it;

	/* ===================== NODE ============================================*/
	struct Node
	{
		Virus virus;
		std::set <parent_type > _parents;
		std::set <shared_ptr<Node> > _children;
		graph_it me_in_graph;
		graph_type* _my_graph = nullptr;

		Node( id_type const &id, graph_type &my_graph ) noexcept : virus( Virus(id) ), _my_graph( &my_graph )
		{
		}

		~Node()
		{
			// skoro uruchomil sie destruktor, to nic nie wskazuje na ten
			// wierzcholek nie powinien miec zadnych rodzicow
			assert( _parents.empty() );
			while ( !_children.empty() ) {
				auto sit = _children.begin();
				auto child = *sit; //shared_ptr
				// usuwam obecny wierzcholek w tablicach rodzicow jego dzieci
				child->_parents.erase( this );
				// usuwam dziecko tego wierzcholka
				// tu wywola sie destruktor owego dziecka, jesli obecny
				// wierzcholek byl jego ostatnim ojcem
				_children.erase( sit );
			}
			// usuwam obecne mapowanie id -> wskaznik na obecny wierzcholek
			// z grafu
			_my_graph->erase( me_in_graph );
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

		void add_parent( parent_type parent )
		{
			_parents.insert( parent );
		}

		void add_children( Node* children )
		{
			_children.insert( shared_ptr<Node>( children ) );
		}

		vector<id_type> get_children_ids() const
		{
			std::vector<id_type> res;
			for ( auto child : _children ) {
				assert ( child.use_count() > 0 );
				res.push_back( child->get_id() );
			}
			return res;
		}

		vector<id_type> get_parents_ids() const
		{
			std::vector<id_type> res;
			for ( auto parent : _parents ) {
				assert ( parent != NULL );
				res.push_back( parent->get_id() );
			}
			return res;
		}
	};

//public:
	// Tworzy nową genealogię.
	// Tworzy także węzęł wirusa macierzystego o identyfikatorze stem_id.
	VirusGenealogy( id_type const &stem_id ) noexcept
	{
		Node* nd = new Node( stem_id, graph );
		graph.insert( make_pair( stem_id, nd ) );
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
		return it->node->get_children_ids();
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
		return it->node->get_parents_ids();
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
	Virus& operator[]( id_type const &id ) const
	{
		auto it = graph.find( id );
		if ( it == graph.end() ) {
			throw VirusNotFound();
		}
		return it->node->virus;
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
			parent_type parents[n];
			for ( size_t i = 0; i < n; i++ ) {
				auto parent_it = graph.find( parent_ids[i] );
				if ( parent_it == graph.end() ) {
					throw VirusNotFound();
				}
				parents[i] = parent_it->node;
			}
			// stworzenie nowego wezla o id w grafie
			// fime_in_graphrst daje nam iterator na umieszczony w grafie element
			Node* newNode = new Node( id, graph );
			auto me_in_graph = graph.insert( make_pair( id, newNode ) ).first;
			// zapamietuje iterator na siebie w grafie - przyda sie przy usuwaniu
			// aby uniknac potencjalnego wyjatku przy wyszukiwaniu po id
			newNode->me_in_graph = me_in_graph;

			for ( size_t i = 0; i < n; i++ ) {
				// przypisanie mu podanych ojcow
				newNode->add_parent( parents[i] );
				// a ojcom dodanie dziecka - obecnego, nowego wezla
				parents[i]->add_children( newNode );
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
		child->node->add_parent( parent->node );
		parent->node->add_children( child->node );
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

		assert( !it->node->_parents.empty() );
		// usuwamy wierzcholek z listy dzieci jego ojcow (odcinamy od ojcow)
		while ( !it->node->_parents.empty() ) {
			// bierzemy pierwszy iterator z set'a ojcow
			auto parent_it = it->node->_parents.begin();
			parent_type parent = *parent_it;
			parent->_children.erase( shared_ptr<Node>(it->node) );
			it->node->_parents.erase( parent_it );
		}
		// w tym momencie - dzieki temu, ze juz zaden shared_ptr nie wskazuje na
		// ten wezel - wywola sie destruktor. Wszyscy ojcowie
		// zostali odcieci. W destruktorze zostaną odciete dzieci (i tam byc moze
		// beda rekurencyjnie usuwac sie wezly dalej.
	}

	void remove( graph_it it )
	{
		// ...

		// usuwanie wierzcholek z listy ojcow jego dzieci
		/*while ( !it->node._children.empty() ) {
			graph_it child = *( it->node._children.begin() );
			child->node._parents.erase( it );
			it->node._children.erase( child );
			// jesli spowodowalo to odciecie ostatniego poprzednika, usuwamy
			// syna z grafu
			if ( child->node._parents.empty() ) {
				remove( child );
			}
		}*/
		// ojcowie oraz synowie zaktualizowani, wiec mozemy usunac z grafu

		//to takze stanie sie destruktorze:
		//graph.erase( it );
	}
/**/
};

#endif
