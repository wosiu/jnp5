/*
* JNP, zadanie 5
* Lukasz Dunaj ld334437
* Michal Wos mw336071
*/
#ifndef __VIRUS_GENEALOGY_H
#define __VIRUS_GENEALOGY_H

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <memory>


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
private:
	typedef typename Virus::id_type id_type;
	struct Node;
	#define node second.lock()
	typedef std::unordered_map <id_type, std::weak_ptr<Node> > graph_type;
	typedef typename graph_type::iterator graph_it;
	typedef Node* parent_type;

	std::shared_ptr<Node> root;
	graph_type graph;

	VirusGenealogy( VirusGenealogy<Virus> const &other ) = delete;
	VirusGenealogy& operator=( VirusGenealogy<Virus> const &other ) = delete;

	/* ===================== NODE ============================================*/
	struct Node
	{
		Virus virus;
		std::unordered_set <parent_type > _parents;
		std::unordered_set <std::shared_ptr<Node> > _children;
		graph_it me_in_graph;
		graph_type* _my_graph;

		Node( id_type const &id, graph_type &my_graph )
            noexcept : virus( Virus(id) ), _my_graph( &my_graph )
		{}

		~Node() noexcept
		{
			// za pomoca iteratora na siebie (no-throw)
			// usuwa swoje mapowanie z id na wskaznik z graph'u
			_my_graph->erase( me_in_graph );

			// skoro uruchomil sie destruktor, to nic nie wskazuje na ten
			// wierzcholek nie ma juz rodzicow
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
		}

		void cut_fst_parent() noexcept
		{
			// bierzemy z iteratora wartosc, czyli wskaznik do pierwszego ojca
			parent_type parent = *( _parents.begin() );
			// usuwam w nodzie obecnego ojca
			_parents.erase( _parents.begin() );
			// a w obecnym ojcu tego node'a
			parent->_children.erase( me_in_graph->node );
		}

		void remove() noexcept
		{
			// usuwamy wierzcholek z listy dzieci jego ojcow (odcinamy od ojcow)
			// (>1), aby destruktor obecnego wierzcholka wywolal sie poza while
			while ( _parents.size() > 1 ) {
				cut_fst_parent();
			}
			// zostal nam ostatni ojciec
			cut_fst_parent();
			// w tym momencie - dzieki temu, ze juz zaden shared_ptr nie wskazuje
			// na ten wezel - wywola sie destruktor. Wszyscy ojcowie
			// zostali odcieci. W destruktorze zostana odciete dzieci (i tam byc
			// moze beda rekurencyjnie usuwac sie wezly dalej.
		}

		typename Virus::id_type get_id() const
		{
			return virus.get_id();
		}

		void add_parent( std::shared_ptr<Node> parent ) noexcept
		{
			_parents.insert( parent.get() );
		}

		void add_children( std::shared_ptr<Node> children ) noexcept
		{
			_children.insert( children );
		}

		std::vector<id_type> get_children_ids() const
		{
			std::vector<id_type> res;
			for ( auto child : _children ) {
				res.push_back( child->get_id() );
			}
			return res;
		}

		std::vector<id_type> get_parents_ids() const
		{
			std::vector<id_type> res;
			for ( auto parent : _parents ) {
				res.push_back( parent->get_id() );
			}
			return res;
		}
	};


public:
	// Tworzy nowa genealogie.
	// Tworzy takze wezel wirusa macierzystego o identyfikatorze stem_id.
	VirusGenealogy( id_type const &stem_id )
	{
		root = std::shared_ptr<Node>( new Node( stem_id, graph ) );
		graph_it me = graph.insert( make_pair( stem_id, root ) ).first;
		root->me_in_graph = me;
	}

	~VirusGenealogy() noexcept
	{
		// najpierw musimy wywolac rekurencyjne niszczenie sie sieci
		// aby graph jeszcze wtedy istnial i wierzcholki mogly
		// we wlasnych destruktorach usuwac swoje mapowania w tymze grafie
		root.reset();
		// gdybysmy bowiem nie dali tego resetu pointera, destruktor domyslny
		// VirusGenealogy moglby (a nawet to robil!) usunac najpierw graph
		// a potem dopiero root, w ktorym dochodziloby do drugiej proby
		// czyszczenia graph
	}

	// Zwraca identyfikator wirusa macierzystego.
	id_type get_stem_id()
	{
		return root->virus.get_id();
	}

	// Zwraca liste identyfikatorow bezposrednich nastepnikow wirusa
	// o podanym identyfikatorze.
	// Zglasza wyjatek VirusNotFound, jesli dany wirus nie istnieje.
	std::vector<id_type> get_children( id_type const &id ) const
	{
		auto it = graph.find( id );
		if ( it == graph.end() ) {
			throw VirusNotFound();
		}
		return it->node->get_children_ids();
	}

	// Zwraca liste identyfikatorow bezposrednich poprzednikow wirusa
	// o podanym identyfikatorze.
	// Zglasza wyjatek VirusNotFound, jesli dany wirus nie istnieje.
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

	// Zwraca referencje do obiektu reprezentujacego wirus o podanym
	// identyfikatorze.
	// Zglasza wyjatek VirusNotFound, jesli zadany wirus nie istnieje.
	Virus& operator[]( id_type const &id ) const
	{
		auto it = graph.find( id );
		if ( it == graph.end() ) {
			throw VirusNotFound();
		}
		return it->node->virus;
	}
	// Tworzy wezel reprezentujacy nowy wirus o identyfikatorze id
	// powstaly z wirusow o podanym identyfikatorze parent_id lub
	// podanych identyfikatorach parent_ids.
	// Zglasza wyjatek VirusAlreadyCreated, jesli wirus o identyfikatorze
	// id juz istnieje.
	// Zglasza wyjatek VirusNotFound, jesli ktorys z wyspecyfikowanych
	// poprzednikow nie istnieje.
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
			std::weak_ptr<Node> parents[n];
			for ( size_t i = 0; i < n; i++ ) {
				auto parent_it = graph.find( parent_ids[i] );
				if ( parent_it == graph.end() ) {
					throw VirusNotFound();
				}
				// parent_it->node to shared_ptr, ale zrzutuje sie na weak_ptr
				// przy operatorze przypisania do weak_ptr, jest ok.
				parents[i] = parent_it->node;
			}
			// stworzenie nowego wezla o id w grafie
			// me_in_graph daje nam iterator na umieszczony w grafie element
			std::shared_ptr<Node> newNode ( new Node( id, graph ) );
			// shared_ptr zrzutuje sie na weak_ptr do wrzucania do mapy graph,
			// wiec jest ok.
			auto me_in_graph = graph.insert( make_pair( id, newNode ) ).first;
			// zapamietuje iterator na siebie w grafie - przyda sie przy usuwa-
			// niu aby uniknac potencjalnego wyjatku przy wyszukiwaniu po id
			newNode->me_in_graph = me_in_graph;

			for ( size_t i = 0; i < n; i++ ) {
				// przypisanie mu podanych ojcow
				newNode->add_parent( parents[i].lock() );
				// a ojcom dodanie dziecka - obecnego, nowego wezla
				parents[i].lock()->add_children( newNode );
			}
	}

	void create( id_type const &id, id_type const &parent_id )
	{
		create( id, std::vector<id_type>( { parent_id } ) );
	}

	// Dodaje nowa krawedz w grafie genealogii.
	// Zglasza wyjatek VirusNotFound, jesli ktorys z podanych wirusow nie
	// istnieje.
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
	// Zglasza wyjatek VirusNotFound, jesli zadany wirus nie istnieje.
	// Zglasza wyjatek TriedToRemoveStemVirus przy probie usuniecia
	// wirusa macierzystego.
	void remove( id_type const &id )
	{
		if ( id == root->virus.get_id() ) {
			throw TriedToRemoveStemVirus();
		}
		graph_it it = graph.find( id );
		if ( it == graph.end() ) {
			throw VirusNotFound();
		}
		it->node->remove();
	}
};

#endif
