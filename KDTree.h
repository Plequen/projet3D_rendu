#ifndef KDTREE_H
#define KDTREE_H

#include "Node.h"

using namespace std;

class KDTree
{
	public :
	KDTree();
	void buildKDTree(const Mesh& m);
	Node* getRoot() const;

	private :

	Node* build(const vector<unsigned>& t, const Mesh& m, const BoundingBox & bbToFitIn, unsigned depth);
	Axis findMaxAxis(const vector<unsigned>& triangles, const Mesh& m);
	void split(vector<unsigned>& trianglesLeft,vector<unsigned>& trianglesRight,const vector<unsigned>& triangles, const Vec3Df& medianSample, const Axis & maxAxis, const Mesh& m);
        void splitBBox(const BoundingBox& bBox, BoundingBox& bBoxRigth, BoundingBox& bBoxLeft, const Axis& axis, const Vec3Df& median);
	Vec3Df findMedianSample(const vector<unsigned>& triangles, const Mesh& m, const Axis& axis);
	void swap(vector<Vec3Df>& tab, int i, int j);
	void quickSort(vector<Vec3Df>& tab, int left, int right, Axis axis);
	int partition(vector<Vec3Df>& tab, int left, int right, int pivot, Axis axis);
	void printTree();


	Node* root;
	unsigned depthMax;
};
#endif

