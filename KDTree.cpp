#include "KDTree.h"

KDTree::KDTree() : root(NULL), depthMax(10)
{}

Node* KDTree::getRoot() const
{
	return root;
}

void KDTree::buildKDTree(const Mesh& m)
{
	depthMax=10;
	const vector<Vertex>& vertices = m.getVertices();

	vector<unsigned> triangles;
	triangles.resize(m.getTriangles().size());
	for(unsigned i=0; i<triangles.size(); i++)
		triangles[i]=i;	

	BoundingBox bbox;
	bbox = BoundingBox (vertices[0].getPos ());
	for (unsigned int i = 1; i < vertices.size (); i++)
		bbox.extendTo (vertices[i].getPos ());

	root = build(triangles, m, bbox, 0);
}

Node* KDTree::build(const vector<unsigned>& triangles, const Mesh& m, const BoundingBox & bbToFitIn, unsigned depth)
{
	Node* n = new Node(bbToFitIn, depth);
	if(depth>=depthMax-1 || triangles.size()<10)
	{
		n->setTriangles(triangles);
		n->depth=depthMax;
	}
	else
	{
		Axis maxAxis = findMaxAxis(triangles, m);
		Vec3Df medianSample = findMedianSample(triangles, m, maxAxis);

		BoundingBox bBoxRight, bBoxLeft;

		splitBBox(bbToFitIn, bBoxRight, bBoxLeft, maxAxis, medianSample);

		n = new Node(bbToFitIn, depth);

		vector<unsigned> trianglesRight;
		vector<unsigned> trianglesLeft;

		split(trianglesLeft, trianglesRight, triangles, medianSample, maxAxis, m);

		n->leftChild=build(trianglesLeft, m, bBoxLeft, depth+1);
		n->rightChild=build(trianglesRight, m, bBoxRight, depth+1);
	}

	return n;
}

void KDTree::splitBBox(const BoundingBox& bBox, BoundingBox& bBoxRight, BoundingBox& bBoxLeft, const Axis& axis, const Vec3Df& median)
{
	Vec3Df maxbb=bBox.getMax();
	Vec3Df minbb=bBox.getMin();
	Vec3Df maxLeft = maxbb;
	maxLeft[axis]=median[axis];
	Vec3Df minRight = minbb;
	minRight[axis]=median[axis];
	bBoxRight = BoundingBox(minRight, maxbb);
	bBoxLeft = BoundingBox(minbb, maxLeft);
}

void KDTree::split(vector<unsigned>& trianglesLeft,vector<unsigned>& trianglesRight,const vector<unsigned>& triangles, const Vec3Df& medianSample, const Axis & maxAxis, const Mesh& m)
{
	const vector<Triangle>& t = m.getTriangles();
	const vector<Vertex>& v = m.getVertices();
	for(unsigned i=0; i<triangles.size(); i++)
	{
		bool addRight=false;
		bool addLeft=false;
		for(unsigned j=0; j<3; j++)
		{
			if(addRight && addLeft) 
				break;
			bool b=v[t[triangles[i]].getVertex(j)].getPos()[maxAxis]<medianSample[maxAxis];
			if(b && !addLeft)
			{
				addLeft=true;
				trianglesLeft.push_back(triangles[i]);
			}
			else if(!b && !addRight)
			{
				addRight=true;
				trianglesRight.push_back(triangles[i]);
			}
		}
	}
}

Vec3Df KDTree::findMedianSample(const vector<unsigned>& triangles, const Mesh& m, const Axis& axis)
{
	vector<Vec3Df> barycentres;
	barycentres.resize(triangles.size());
	for(unsigned i=0; i<triangles.size(); i++)
	{
		barycentres[i]=(m.getVertices()[m.getTriangles()[triangles[i]].getVertex(0)].getPos()
				+ m.getVertices()[m.getTriangles()[triangles[i]].getVertex(1)].getPos()
				+ m.getVertices()[m.getTriangles()[triangles[i]].getVertex(2)].getPos())/3;
	}
	quickSort(barycentres, 0, barycentres.size()-1, axis);
	unsigned k;
	if(barycentres.size()%2==0)
		k = barycentres.size()/2-1;
	else
		k = (unsigned)floor(barycentres.size()/2);
	return barycentres[k];
}

Axis KDTree::findMaxAxis(const vector<unsigned>& triangles, const Mesh& m)
{
	BoundingBox bbox = BoundingBox (m.getVertices()[m.getTriangles()[triangles[0]].getVertex(0)].getPos ());
	for (unsigned int i = 1; i < triangles.size (); i++)
	{
		for(unsigned j=1; j<3; j++)
			bbox.extendTo(m.getVertices()[m.getTriangles()[triangles[i]].getVertex(j)].getPos ());
	}

	Axis maxAxis=X;
	if(bbox.getMax()[1]-bbox.getMin()[1]>bbox.getMax()[0]-bbox.getMin()[0])
		maxAxis = Y;
	if(bbox.getMax()[2]-bbox.getMin()[2]>bbox.getMax()[1]-bbox.getMin()[1])
		maxAxis = Z;

	return maxAxis;

}

int KDTree::partition(vector<Vec3Df>& tab, int left, int right, int pivot, Axis axis)
{
	float pivotValue = tab[pivot][axis];
	swap(tab, pivot, right);
	int storeIndex = left;
	for(int i=left; i<right; i++)
	{
		if(tab[i][axis]<pivotValue)
		{
			swap(tab, i, storeIndex);
			storeIndex++;
		}
	}
	swap(tab,storeIndex,right);
	return storeIndex;
}
void KDTree::quickSort(vector<Vec3Df>& tab, int left, int right, Axis axis)
{
	if(left<right)
	{
		int pivotIndex=rand()%(left-right)+left;
		int pivotNewIndex = partition(tab, left, right, pivotIndex, axis);
		quickSort(tab,left, pivotNewIndex-1, axis);
		quickSort(tab, pivotNewIndex+1, right, axis);
	}
}

void KDTree::swap(vector<Vec3Df>& tab, int i, int j)
{
	Vec3Df temp = tab[i];
	tab[i]=tab[j];
	tab[j]=temp;
}

void KDTree::printTree()
{
	root->print();
}
