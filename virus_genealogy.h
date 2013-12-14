#ifndef __VIRUS_GENEALOGY_H
#define __VIRUS_GENEALOGY_H

#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <string>


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


using namespace std;

template<class Virus>
class Node
{
//private:
public:
	Virus virus;
	typedef typename std::set <Node<Virus> >::iterator graph_it;
	// komparator do porównywania iteratorów w setach ojców / dzieci
	struct classcomp
	{
		bool operator() (const graph_it node1, const graph_it node2) const
		{
			return node1->get_id() < node2->get_id();
		}
	};

	// w secie trzymamy iteratory do ojców w set'cie wszystkich wieszchołków
	// dzieki temu oszczedzamy log(n) na wyszkuiwaniu pojedynczego ojca / syna
	// np podczas usuwania
	std::set < graph_it, classcomp > parents, children;

public:
	Node(Virus v) : virus(v)
	{}

	Virus& get_virus() const
	{
		return virus;
	}

	typename Virus::id_type get_id() const
	{
		return virus.get_id();
	}

	//void insert_parent()

};

#endif
