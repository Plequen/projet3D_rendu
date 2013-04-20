#ifndef KDTREE2_H
#define KDTREE2_H

#include "Ray.h"
#include "Mesh.h"
#include "BoundingBox.h"
#include "Node.h"

/**
 * KDTree class.
**/
class KDTree2 {
public :
	KDTree2();
	KDTree2(unsigned int depthMax, unsigned int minimalLeafSize);
	~KDTree2();
	void buildKDTree(const Mesh& mesh);
	void quickSort(std::vector<Vec3Df>& points, int left, int right, Axis axis);
	int partition(std::vector<Vec3Df>& points, int left, int right, int pivot, Axis axis);
	void swap(std::vector<Vec3Df>& points, int i, int j);
	bool intersectsRay(const Mesh& mesh, Ray& ray, Vertex& intersectionPoint, unsigned int& leafId) const;

private :
	class KDTreeNode { 
	public :
		KDTreeNode();
		KDTreeNode(BoundingBox& boundingBox, unsigned int* tIndices, unsigned int tNumber, unsigned int leafId);
		KDTreeNode(BoundingBox& boundingBox, KDTreeNode* leftChild, KDTreeNode* rightChild);
		~KDTreeNode();
		bool isLeaf();
		KDTreeNode* getLeftChild() const { return leftChild; }
		KDTreeNode* getRightChild() const { return rightChild; }
		unsigned int& getLeafId() { return leafId; }
		unsigned int& getTNumber() { return tNumber; }
		unsigned int* getTIndices() { return tIndices; }
		bool rayIntersectsBB(Ray& ray) const;
		
	private :
		BoundingBox boundingBox;
		unsigned int* tIndices; // triangles indices
		unsigned int tNumber;
		KDTreeNode* leftChild;
		KDTreeNode* rightChild;
		unsigned int leafId;
	};

public :
	KDTreeNode* buildNode(const Mesh& mesh, BoundingBox& boundingBox, unsigned int* tIndices, unsigned int tSize, unsigned int depth);
	KDTreeNode* getRoot() const { return root; }
	bool rayIntersectsNode(const Mesh& mesh, Ray& ray, KDTreeNode* node, Vertex& intersectionPoint, float& t, unsigned int& leafId) const;

private :	
	KDTreeNode* root;
	unsigned int depthMax;
	unsigned int minimalLeafSize;
	unsigned int leafsNumber;
};
#endif

