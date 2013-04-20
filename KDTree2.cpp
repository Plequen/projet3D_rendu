#include "KDTree2.h"

#define	KDTREE_DEPTH_MAX	10
#define	MINIMAL_LEAF_SIZE	10 	// must be at least two

using namespace std;

KDTree2::KDTree2() : root(NULL), depthMax(KDTREE_DEPTH_MAX), minimalLeafSize(MINIMAL_LEAF_SIZE) { }

KDTree2::KDTree2(Mesh* mesh) {
	root = NULL;
	this->mesh = mesh;
	this->depthMax = KDTREE_DEPTH_MAX;
	this->minimalLeafSize = MINIMAL_LEAF_SIZE;
}

KDTree2::KDTree2(Mesh* mesh, unsigned int depthMax, unsigned int minimalLeafSize) {
	root = NULL;
	this->mesh = mesh;
	this->depthMax = depthMax;
	this->minimalLeafSize = minimalLeafSize;
}

KDTree2::~KDTree2() {
//	delete root;
}

void KDTree2::buildKDTree() {
	unsigned int trianglesNumber = mesh->getTriangles().size();
	unsigned int* tIndices = new unsigned int[trianglesNumber];
	BoundingBox mainBB;
	for (unsigned int i = 0 ; i < trianglesNumber ; i++) {
		tIndices[i] = i;
		for (unsigned int j = 0 ; j < 3 ; j++)
			mainBB.extendTo(mesh->getVertices()[mesh->getTriangles()[tIndices[i]].getVertex(j)].getPos());
	}

	root = buildNode(mainBB, tIndices, trianglesNumber, 0); 
}

void KDTree2::buildKDTree(Mesh* mesh) {
	this->mesh = mesh;
	buildKDTree();
}

KDTree2::KDTreeNode* KDTree2::buildNode(BoundingBox& boundingBox, unsigned int* tIndices, unsigned int tSize, unsigned int depth) {
	// if the node to create is a leaf, fill the leaf with the triangles
	if (depth >= depthMax || tSize <= minimalLeafSize)
		return new KDTreeNode(boundingBox, tIndices, tSize);

	vector<Vertex>& vertices = mesh->getVertices();
	vector<Triangle>& triangles = mesh->getTriangles();

	// find the main axis
	float bbWidth = boundingBox.getWidth();		
	float bbHeight = boundingBox.getHeight();		
	float bbLength = boundingBox.getLength();		
	Axis axis = X;
	if (bbHeight >= bbWidth && bbHeight >= bbLength)
		axis = Y;
	else if	(bbLength >= bbWidth && bbLength >= bbWidth)
		axis = Z;

	// find the median point associated to that axis
	vector<Vec3Df> barycentres;
	barycentres.resize(tSize);
	for (unsigned int i = 0 ; i < tSize ; i++) {
		for (unsigned j = 0 ; j < 3 ; j++)
			barycentres[i] += vertices[triangles[tIndices[i]].getVertex(j)].getPos();
		barycentres[i] /= 3;
	}
	quickSort(barycentres, 0, barycentres.size() - 1, axis);

	unsigned int medianIndice = barycentres.size() / 2;
	if (barycentres.size() % 2 == 0)
		medianIndice--;

	// compute the bounding boxes of the children 
	Vec3Df minBB = boundingBox.getMin();	
	Vec3Df maxBB = boundingBox.getMax();
	Vec3Df maxLeftChildBB = maxBB;
	maxLeftChildBB[axis] = barycentres[medianIndice][axis];
	Vec3Df minRightChildBB = minBB;
	minRightChildBB[axis] = barycentres[medianIndice][axis];
	
	BoundingBox leftChildBB(minBB, maxLeftChildBB);
	BoundingBox rightChildBB(minRightChildBB, maxBB);

	// split the triangles in one (or both) of the children
	// count the number of triangles in each child
	unsigned int tNumberLeft = 0, tNumberRight = 0;
	for (unsigned int i = 0 ; i < tSize ; i++) {
		bool addRight = false, addLeft = false;
		for (unsigned int j = 0 ; j < 3 ; j++) {
			if (addRight && addLeft)
				break;
			bool inLeftChild = vertices[triangles[tIndices[i]].getVertex(j)].getPos()[axis] <= barycentres[medianIndice][axis];
			if (inLeftChild && !addLeft) {
				addLeft = true;
				tNumberLeft++;
			}
			else if (!inLeftChild && !addRight) {
				addRight = true;
				tNumberRight++;
			}
		}
	}
	// fill the array of triangles of each child
	unsigned int* tIndicesLeft = new unsigned int[tNumberLeft];
	unsigned int* tIndicesRight = new unsigned int[tNumberRight];
	unsigned int leftIndice = 0, rightIndice = 0;
	for (unsigned int i = 0 ; i < tSize ; i++) {
		bool addRight = false, addLeft = false;
		for (unsigned int j = 0 ; j < 3 ; j++) {
			if (addRight && addLeft)
				break;
			bool inLeftChild = vertices[triangles[tIndices[i]].getVertex(j)].getPos()[axis] <= barycentres[medianIndice][axis];
			if (inLeftChild && !addLeft) {
				addLeft = true;
				tIndicesLeft[leftIndice++] = tIndices[i];
			}
			else if (!inLeftChild && !addRight) {
				addRight = true;
				tIndicesRight[rightIndice++] = tIndices[i];
			}
				 
		}
	}

	// build the two children and build the current node 
	delete [] tIndices;
	KDTreeNode* leftChild = buildNode(leftChildBB, tIndicesLeft, tNumberLeft, depth + 1);
	KDTreeNode* rightChild = buildNode(rightChildBB, tIndicesRight, tNumberRight, depth + 1);
	return new KDTreeNode(boundingBox, leftChild, rightChild);
}

void KDTree2::quickSort(vector<Vec3Df>& points, int left, int right, Axis axis) {
	if (left < right) {
		int pivotIndex = rand() % (right - left) + left;
		int pivotNewIndex = partition(points, left, right, pivotIndex, axis);
		quickSort(points, left, pivotNewIndex - 1, axis);
		quickSort(points, pivotNewIndex + 1, right, axis);
	}
}

int KDTree2::partition(vector<Vec3Df>& points, int left, int right, int pivot, Axis axis) {
	float pivotValue = points[pivot][axis];
	swap(points, pivot, right);
	int storeIndex = left;
	for (int i = left ; i < right ; i++) {
		if (points[i][axis] < pivotValue) {
			swap(points, i, storeIndex);
			storeIndex++;
		}
	}
	swap(points, storeIndex, right);
	return storeIndex;
}

void KDTree2::swap(vector<Vec3Df>& points, int i, int j) {
	Vec3Df temp = points[i];
	points[i] = points[j];
	points[j] = temp;
}


//
// Implementation of the nested class KDTreeNode
//
KDTree2::KDTreeNode::KDTreeNode() : tIndices(NULL), leftChild(NULL), rightChild(NULL) { }

KDTree2::KDTreeNode::KDTreeNode(BoundingBox& boundingBox, unsigned int* tIndices, unsigned int tNumber) {
	this->boundingBox = boundingBox;
	this->tIndices = tIndices;
	this->tNumber = tNumber;
	leftChild = NULL;
	rightChild = NULL;
}

KDTree2::KDTreeNode::KDTreeNode(BoundingBox& boundingBox, KDTreeNode* leftChild, KDTreeNode* rightChild) {
	this->boundingBox = boundingBox;
	this->leftChild = leftChild;
	this->rightChild = rightChild;
	tIndices = NULL;
}

KDTree2::KDTreeNode::~KDTreeNode() {
//	delete [] tIndices;
//	delete leftChild;
//	delete rightChild;
}

bool KDTree2::KDTreeNode::isLeaf() {
	return leftChild == NULL && rightChild == NULL;
}

