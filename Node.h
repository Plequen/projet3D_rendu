#ifndef NODE_H
#define NODE_H

#include <iostream>
#include <vector>
#include "Mesh.h"
#include "BoundingBox.h"

enum Axis {X = 0, Y = 1, Z = 2};
using namespace std;

class KDTree;

class Node
{
	friend class KDTree;
	public :
	Node();
	Node(BoundingBox bb, unsigned d);
	inline const BoundingBox & getBoundingBox() const {return bBox;}
	inline const unsigned* getTriangles() const {return triangles;}
	inline Node* getRightChild() const {return rightChild;}
	inline Node* getLeftChild() const {return leftChild;}
	inline unsigned getNbTriangles() const {return nbTriangles;}
	inline bool isLeaf() {return (leftChild==NULL && rightChild == NULL);}
	void setTriangles(const vector<unsigned>& t);
	void print();

	private :
	unsigned nbTriangles;
	unsigned depth;
	Node* leftChild;
	Node* rightChild;
	BoundingBox bBox;
	unsigned *triangles;
};
	
#endif

