#include "KDTree2.h"

#define	KDTREE_DEPTH_MAX	10
#define	MINIMAL_LEAF_SIZE	10 	// must be at least two
#define INFINITE_DISTANCE	1000000.0f

using namespace std;

KDTree2::KDTree2() : root(NULL), depthMax(KDTREE_DEPTH_MAX), minimalLeafSize(MINIMAL_LEAF_SIZE), leafsNumber(0)  { }

KDTree2::KDTree2(unsigned int depthMax, unsigned int minimalLeafSize) {
	root = NULL;
	this->depthMax = depthMax;
	this->minimalLeafSize = minimalLeafSize;
	leafsNumber = 0;
}

KDTree2::~KDTree2() {
//	delete root;
}

void KDTree2::buildKDTree(const Mesh& mesh) {
	unsigned int trianglesNumber = mesh.getTriangles().size();
	unsigned int* tIndices = new unsigned int[trianglesNumber];
	BoundingBox mainBB;
	for (unsigned int i = 0 ; i < trianglesNumber ; i++) {
		tIndices[i] = i;
		for (unsigned int j = 0 ; j < 3 ; j++)
			mainBB.extendTo(mesh.getVertices()[mesh.getTriangles()[tIndices[i]].getVertex(j)].getPos());
	}

	root = buildNode(mesh, mainBB, tIndices, trianglesNumber, 0); 
}

KDTree2::KDTreeNode* KDTree2::buildNode(const Mesh& mesh, BoundingBox& boundingBox, unsigned int* tIndices, unsigned int tSize, unsigned int depth) {
	// if the node to create is a leaf, fill the leaf with the triangles
	if (depth >= depthMax || tSize <= minimalLeafSize)
		return new KDTreeNode(boundingBox, tIndices, tSize, leafsNumber++);

	const vector<Vertex>& vertices = mesh.getVertices();
	const vector<Triangle>& triangles = mesh.getTriangles();

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
	KDTreeNode* leftChild = buildNode(mesh, leftChildBB, tIndicesLeft, tNumberLeft, depth + 1);
	KDTreeNode* rightChild = buildNode(mesh, rightChildBB, tIndicesRight, tNumberRight, depth + 1);
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
// Intersection with a Ray
//
bool KDTree2::intersectsRay(const Mesh& mesh, Ray& ray, Vertex& intersectionPoint, unsigned int& leafId) const {
	float t = INFINITE_DISTANCE;
	return rayIntersectsNode(mesh, ray, root, intersectionPoint, t, leafId);	
}

bool KDTree2::rayIntersectsNode(const Mesh& mesh, Ray& ray, KDTreeNode* node, Vertex& intersectionPoint, float& t, unsigned int& leafId) const {
	if (!node->rayIntersectsBB(ray))
		return false;

	if (!node->isLeaf()) {	
		Vertex intersectionLeft, intersectionRight;
		unsigned int leafIdLeft, leafIdRight;
		float tLeft = INFINITE_DISTANCE, tRight = INFINITE_DISTANCE;
		bool intersectsLeft = rayIntersectsNode(mesh, ray, node->getLeftChild(), intersectionLeft, tLeft, leafIdLeft);
		bool intersectsRight = rayIntersectsNode(mesh, ray, node->getRightChild(), intersectionRight, tRight, leafIdRight);
		if (intersectsLeft && tLeft < t && (!intersectsRight || tRight >= tLeft)) {
			t = tLeft;
			intersectionPoint = intersectionLeft;
			leafId = leafIdLeft;
			return true;
		}
		else if (intersectsRight && tRight < t && (!intersectsLeft || tLeft >= tRight)) {
			t = tRight;
			intersectionPoint = intersectionRight;
			leafId = leafIdRight;
			return true;
		}
		return false;
	}
	// the current node is a leaf
	bool hasIntersection = false;
	const vector<Vertex>& vertices = mesh.getVertices();
	const vector<Triangle>& triangles = mesh.getTriangles();
	unsigned int* tIndices = node->getTIndices();
	for (unsigned int i = 0 ; i < node->getTNumber() ; i++) {
		float tTemp = INFINITE_DISTANCE, alpha, beta; 
		const Vec3Df& v0 = vertices[triangles[tIndices[i]].getVertex(0)].getPos();
		const Vec3Df& v1 = vertices[triangles[tIndices[i]].getVertex(1)].getPos();
		const Vec3Df& v2 = vertices[triangles[tIndices[i]].getVertex(2)].getPos();
		bool intersects = ray.intersectTriangle(v0, v1, v2, tTemp, alpha, beta);
		if (intersects && tTemp < t) {
			hasIntersection = true;
			t = tTemp;
			leafId = node->getLeafId();
			intersectionPoint.setPos(ray.getOrigin() + t * ray.getDirection());
			intersectionPoint.setNormal((1-alpha-beta)*vertices[triangles[tIndices[i]].getVertex(0)].getNormal()+alpha*vertices[triangles[tIndices[i]].getVertex(1)].getNormal()+beta*vertices[triangles[tIndices[i]].getVertex(2)].getNormal()); 
		}
	}
	return hasIntersection;
}

//
// Implementation of the nested class KDTreeNode
//
KDTree2::KDTreeNode::KDTreeNode() : tIndices(NULL), leftChild(NULL), rightChild(NULL) { }

KDTree2::KDTreeNode::KDTreeNode(BoundingBox& boundingBox, unsigned int* tIndices, unsigned int tNumber, unsigned int leafId) {
	this->boundingBox = boundingBox;
	this->tIndices = tIndices;
	this->tNumber = tNumber;
	leftChild = NULL;
	rightChild = NULL;
	this->leafId = leafId;
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

bool KDTree2::KDTreeNode::rayIntersectsBB(Ray& ray) const {
	Vec3Df intersection;
	return ray.intersect(boundingBox, intersection);
}

