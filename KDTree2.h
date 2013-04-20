#ifndef KDTREE2_H
#define KDTREE2_H

#include "Mesh.h"
#include "BoundingBox.h"
#include "Node.h"

/**
 * KDTree class.
**/
class KDTree2 {
public :
	KDTree2();
	KDTree2(Mesh* mesh);
	KDTree2(Mesh* mesh, unsigned int depthMax, unsigned int minimalLeafSize);
	~KDTree2();
	void buildKDTree();
	void buildKDTree(Mesh* mesh);
	void quickSort(std::vector<Vec3Df>& points, int left, int right, Axis axis);
	int partition(std::vector<Vec3Df>& points, int left, int right, int pivot, Axis axis);
	void swap(std::vector<Vec3Df>& points, int i, int j);

private :
	class KDTreeNode { 
	public :
		KDTreeNode();
		KDTreeNode(BoundingBox& boundingBox, unsigned int* tIndices, unsigned int tNumber);
		KDTreeNode(BoundingBox& boundingBox, KDTreeNode* leftChild, KDTreeNode* rightChild);
		~KDTreeNode();
		bool isLeaf();
		
	private :
		BoundingBox boundingBox;
		unsigned int* tIndices; // triangles indices
		unsigned int tNumber;
		KDTreeNode* leftChild;
		KDTreeNode* rightChild;
	};

public :
	KDTreeNode* buildNode(BoundingBox& boundingBox, unsigned int* tIndices, unsigned int tSize, unsigned int depth);

private :	
	KDTreeNode* root;
	Mesh* mesh;
	unsigned int depthMax;
	unsigned int minimalLeafSize;
};
#endif

