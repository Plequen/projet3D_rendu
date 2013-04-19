#include "Node.h"

Node::Node() : leftChild(NULL), rightChild(NULL), triangles(), bBox(), depth(0), nbTriangles(0)
{}

Node::Node(BoundingBox bb, unsigned d) : leftChild(NULL), rightChild(NULL), bBox(bb), depth(d), nbTriangles(0) {}

void Node::setTriangles(const vector<unsigned>& t)
{
	nbTriangles=t.size();
	triangles = new unsigned[nbTriangles];
	for(unsigned i=0; i<nbTriangles; i++)
		triangles[i]=t[i];
}

void Node::print()
{
	if(leftChild!=NULL)
		leftChild->print();
	if(rightChild!=NULL)
		rightChild->print();

	if(isLeaf())
	{
		cout << endl;
		for(unsigned i = 0; i<nbTriangles; i++)
			cout << triangles[i] << ", ";
	}
}
