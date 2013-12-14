#ifndef __VIRUS_GENEALOGY_H
#define __VIRUS_GENEALOGY_H

#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <string>


// TODO usunac to i dodac wszedzie gdzie trzeba std::*
using namespace std;

/* ===================== WYJATKI =============================================*/

class VirusNotFound: public std::exception {
    virtual const char* what() const noexcept {
        return "VirusNotFound";
    }
};

class VirusAlreadyCreated: public std::exception {
    virtual const char* what() const noexcept {
        return "VirusAlreadyCreated";
    }
};

class TriedToRemoveStemVirus: public std::exception {
    virtual const char* what() const noexcept {
        return "TriedToRemoveStemVirus";
    }
};

/* ===================== NODE ================================================*/

template<class Virus>
class Node
{
//private:
public:
	Virus virus;
	// iterator do grafu
	typedef typename std::set <Node<Virus > >::iterator graph_it;
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
	std::set <graph_it, classcomp > _parents;
	std::set <graph_it, classcomp > _children;

public:
	Node( Virus v ) : virus( v )
	{}

	Virus& get_virus() const
	{
		return virus;
	}

	typename Virus::id_type get_id() const
	{
		return virus.get_id();
	}

	void insert_parent( graph_it parent )
	{
		_parents.insert( parent );
	}

	void insert_children( graph_it children )
	{
		_children.insert( children );
	}

	vector<typename Virus::id_type> get_children_ids() const
	{
		std::vector<typename Virus::id_type> res;
		for ( auto child : _children ) {
			res.push_back( child->get_id() );
		}
		return res;
	}

	vector<typename Virus::id_type> get_parents_ids() const
	{
		std::vector<typename Virus::id_type> res;
		for ( auto parent : _parents ) {
			res.push_back( parent->get_id() );
		}
		return res;
	}
};




#endif
