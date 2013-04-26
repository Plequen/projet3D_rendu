
// *********************************************************
// Scene Class
// Author : Tamy Boubekeur (boubek@gmail.com).
// Copyright (C) 2010 Tamy Boubekeur.
// All rights reserved.
// *********************************************************

#include "Scene.h"

using namespace std;

static Scene * instance = NULL;

Scene * Scene::getInstance () {
    if (instance == NULL)
        instance = new Scene ();
    return instance;
}

void Scene::destroyInstance () {
    if (instance != NULL) {
        delete instance;
        instance = NULL;
    }
}

Scene::Scene () {
    buildDefaultScene ();
	//scenePTRefraction();
	//scenePTReflexion();
	//scenePTCaustique();
    updateBoundingBox ();
}

Scene::~Scene () {
}

void Scene::updateBoundingBox () {
    if (objects.empty ())
        bbox = BoundingBox ();
    else {
        bbox = objects[0].getBoundingBox ();
        for (unsigned int i = 1; i < objects.size (); i++)
            bbox.extendTo (objects[i].getBoundingBox ());
    }
}

Vec3Df animation1(Vec3Df& initial, unsigned int t) {
	return initial + t * Vec3Df(0.f, 0.02f, 0.f);
}

Vec3Df animation2(Vec3Df& initial, unsigned int t) {
	return initial + t * Vec3Df(-0.02f, 0.f, 0.f);
}

void Scene::scenePTCaustique() {
	// gloss, reflec, blending, refrac, transp
	Material matMirror(0.8f, 0.2f, Vec3Df(1.f, 1.f, 1.f), 0.0f, 1.f, 1.f, 1.0f, 0.0f);
	Material matWhite(0.5f, 0.f, Vec3Df(1.f, 1.f, 1.f), 0.0f, 0.f, 0.f, 1.0f, 0.0f);
	Material matRed(0.8f, 0.2f, Vec3Df(1.f, 0.f, 0.f), 0.0f, 0.f, 0.f, 1.0f, 0.0f);
	Material matGreen(0.8f, 0.2f, Vec3Df(0.f, 1.f, 0.f), 0.0f, 0.f, 0.f, 1.0f, 0.0f);
	Material matBlue(0.8f, 0.2f, Vec3Df(0.f, 0.f, 1.f), 0.0f, 0.f, 0.f, 1.0f, 0.0f);
	Material matOrange(0.8f, 0.f, Vec3Df(0.f, 0.207f, 0.27f), 0.0f, 0.f, 0.f, 1.0f, 0.0f);
	matOrange = matWhite;

	// box
	Mesh groundMesh;
    	groundMesh.loadOFF("models/ground.off");
	Object ground(groundMesh, matOrange);

	Mesh groundMesh2;
    	groundMesh2.loadOFF("models/ground2.off");
	Object back(groundMesh2, matOrange);
	back.setTrans(Vec3Df(0.f, 8.f, 8.f));

	Mesh groundMesh3;
    	groundMesh3.loadOFF("models/ground3.off");
	Object left(groundMesh3, matOrange);
	left.setTrans(Vec3Df(-8.f, 0.0f, 8.f));

	Mesh groundMesh4;
    	groundMesh4.loadOFF("models/ground4.off");
	Object right(groundMesh4, matGreen);
	right.setTrans(Vec3Df(8.f, 0.0f, 8.f));

	Mesh groundMesh5;
    	groundMesh5.loadOFF("models/ground5.off");
	Object front(groundMesh5, matWhite);
	front.setTrans(Vec3Df(0.f, -8.0f, 8.f));

	Mesh groundMesh6;
    	groundMesh6.loadOFF("models/ground6.off");
	Object top(groundMesh6, matWhite);
	top.setTrans(Vec3Df(0.f, 0.0f, 16.f));


	Material matGlass(1.f, 1.f, Vec3Df(1.f, 1.f, 1.f), 0.0f, 0.f, 1.f, 1.5f, 1.f);
	Mesh glassMesh;
    	glassMesh.loadOFF("models/glass.off");
	Object glass(glassMesh, matGlass);
	glass.setTrans(Vec3Df(0.f, 0.f, 4.5f));

	Material matWine(1.f, 1.f, Vec3Df(0.886f, 0.239f, 0.412f), 0.0f, 0.f, 1.f, 1.3f, 0.5f);
	Mesh wineMesh;
    	wineMesh.loadOFF("models/wine.off");
	Object wine(wineMesh, matWine);
	wine.setTrans(Vec3Df(1.08f, 0.0f, 1.f));

	objects.push_back(ground);
	objects.push_back(back);
	//objects.push_back(left);
	//objects.push_back(right);
	//objects.push_back(front);
	//objects.push_back(top);
	objects.push_back(glass);
	objects.push_back(wine); 

	// lights
	AreaLight light0(Vec3Df(5.0f, 0.0f, 30.0f), Vec3Df(1.0f, 1.f, 1.0f), 0.9f, 1.f, Vec3Df(-5.0f, 0.0f, -30.0f));
	areaLights.push_back(light0);
}

void Scene::scenePTReflexion() {
	// gloss, reflec, blending, refrac, transp
	Material matMirror(0.8f, 0.2f, Vec3Df(1.f, 1.f, 1.f), 0.0f, 1.f, 1.f, 1.0f, 0.0f);
	Material matWhite(0.8f, 0.2f, Vec3Df(0.7f, 0.7f, 0.7f), 0.0f, 0.f, 0.f, 1.0f, 0.0f);
	Material matRed(0.8f, 0.2f, Vec3Df(1.f, 0.f, 0.f), 0.0f, 0.f, 0.f, 1.0f, 0.0f);
	Material matGreen(0.8f, 0.2f, Vec3Df(0.f, 1.f, 0.f), 0.0f, 0.f, 0.f, 1.0f, 0.0f);
	Material matBlue(0.8f, 0.2f, Vec3Df(0.f, 0.f, 1.f), 0.0f, 0.f, 0.f, 1.0f, 0.0f);

	// box
	Mesh groundMesh;
    	groundMesh.loadOFF("models/ground.off");
	Object ground(groundMesh, matWhite);

	Mesh groundMesh2;
    	groundMesh2.loadOFF("models/ground2.off");
	Object back(groundMesh2, matWhite);
	back.setTrans(Vec3Df(0.f, 8.f, 8.f));

	Mesh groundMesh3;
    	groundMesh3.loadOFF("models/ground3.off");
	Object left(groundMesh3, matRed);
	left.setTrans(Vec3Df(-8.f, 0.0f, 8.f));

	Mesh groundMesh4;
    	groundMesh4.loadOFF("models/ground4.off");
	Object right(groundMesh4, matGreen);
	right.setTrans(Vec3Df(8.f, 0.0f, 8.f));

	Mesh groundMesh5;
    	groundMesh5.loadOFF("models/ground5.off");
	Object front(groundMesh5, matWhite);
	front.setTrans(Vec3Df(0.f, -8.0f, 8.f));

	Mesh groundMesh6;
    	groundMesh6.loadOFF("models/ground6.off");
	Object top(groundMesh6, matWhite);
	top.setTrans(Vec3Df(0.f, 0.0f, 16.f));


	// reflection example
	Mesh sphereMesh;
    	sphereMesh.loadOFF("models/sphere.off");
	Object sphere(sphereMesh, matMirror);
	sphere.setTrans(Vec3Df(-2.f, 2.f, 1.f));

	Material matPave(0.7f, 0.4f, Vec3Df(0.5f, 0.5f, 0.5f));
	Mesh paveMesh;
    	paveMesh.loadOFF("models/paveDebout.off");
	Object pave(paveMesh, matPave);
	pave.setTrans(Vec3Df(5.f, 5.0f, 2.f));

	Material matTrans(1.f, 1.f, Vec3Df(1.f, 1.f, 1.f), 0.0f, 0.f, 1.f, 1.3f, 1.f);
	Object sphere2(sphereMesh, matTrans);
	sphere2.setTrans(Vec3Df(5.f, 0.f, 1.f));

	objects.push_back(ground);
	objects.push_back(back);
	objects.push_back(left);
	objects.push_back(right);
	objects.push_back(front);
	objects.push_back(top);
	objects.push_back(sphere);
	//objects.push_back(pave); 
	objects.push_back(sphere2);

	// lights
	AreaLight light0(Vec3Df(0.0f, 0.0f, 16.0f), Vec3Df(1.0f, 1.f, 1.0f), 1.f, 1.f, Vec3Df(0.0f, 0.0f, -1.0f));
	areaLights.push_back(light0);
}

void Scene::scenePTRefraction() {
	// gloss, reflec, blending, refrac, transp
	Material matMirror(0.8f, 0.2f, Vec3Df(1.f, 1.f, 1.f), 0.0f, 1.f, 1.f, 1.0f, 0.0f);
	Material matWhite(0.8f, 0.2f, Vec3Df(1.f, 1.f, 1.f), 0.0f, 0.f, 0.f, 1.0f, 0.0f);
	Material matRed(0.8f, 0.2f, Vec3Df(1.f, 0.f, 0.f), 0.0f, 0.f, 0.f, 1.0f, 0.0f);
	Material matGreen(0.8f, 0.2f, Vec3Df(0.f, 1.f, 0.f), 0.0f, 0.f, 0.f, 1.0f, 0.0f);
	Material matBlue(0.8f, 0.2f, Vec3Df(0.f, 0.f, 1.f), 0.0f, 0.f, 0.f, 1.0f, 0.0f);

	// box
	Mesh groundMesh;
    	groundMesh.loadOFF("models/ground.off");
	Object ground(groundMesh, matWhite);

	Mesh groundMesh2;
    	groundMesh2.loadOFF("models/ground2.off");
	Object back(groundMesh2, matWhite);
	back.setTrans(Vec3Df(0.f, 8.f, 8.f));

	Mesh groundMesh3;
    	groundMesh3.loadOFF("models/ground3.off");
	Object left(groundMesh3, matRed);
	left.setTrans(Vec3Df(-8.f, 0.0f, 8.f));

	Mesh groundMesh4;
    	groundMesh4.loadOFF("models/ground4.off");
	Object right(groundMesh4, matGreen);
	right.setTrans(Vec3Df(8.f, 0.0f, 8.f));

	Mesh groundMesh5;
    	groundMesh5.loadOFF("models/ground5.off");
	Object front(groundMesh5, matWhite);
	front.setTrans(Vec3Df(0.f, -8.0f, 8.f));

	Mesh groundMesh6;
    	groundMesh6.loadOFF("models/ground6.off");
	Object top(groundMesh6, matWhite);
	top.setTrans(Vec3Df(0.f, 0.0f, 16.f));


	// refraction example
	Material matWater(1.f, 1.f, Vec3Df(0.f, 1.f, 1.f), 0.0f, 0.f, 1.f, 1.3f, 0.8f);
	Mesh cubeMesh;
    	cubeMesh.loadOFF("models/pave1.off");
	Object cube(cubeMesh, matWater);
	cube.setTrans(Vec3Df(0.f, 2.f, 4.f));

	Material matStick(0.7f, 0.4f, Vec3Df(0.5f, 0.8f, 0.5f));
	Mesh stickMesh;
    	stickMesh.loadOFF("models/baton.off");
	Object stick(stickMesh, matStick);
	stick.setTrans(Vec3Df(0.f, 2.0f, 4.f));

	objects.push_back(ground);
	objects.push_back(back);
	objects.push_back(left);
	objects.push_back(right);
	objects.push_back(front);
	objects.push_back(top);
	objects.push_back(cube);
	objects.push_back(stick); 

	// lights
	AreaLight light0(Vec3Df(0.0f, 2.0f, 14.0f), Vec3Df(1.0f, 1.f, 1.0f), 1.f, 2.f, Vec3Df(0.0f, -2.0f, -14.0f));
	areaLights.push_back(light0);
	AreaLight light1(Vec3Df(0.0f, -2.0f, 14.0f), Vec3Df(1.0f, 1.f, 1.0f), 1.f, 2.f, Vec3Df(0.0f, 2.0f, -14.0f));
	areaLights.push_back(light1);
}

// Changer ce code pour creer des scenes originales
void Scene::buildDefaultScene () {
    Mesh groundMesh;
    groundMesh.loadOFF ("models/ground.off");
	// gloss, reflec, blending, refrac, transp
    Material groundMat(0.8f, 0.2f, Vec3Df(1.f,1.f,1.f), 0.0f, 0.f, 0.f, 1.0f, 0.0f);
    Material groundGreen(0.8f, 0.2f, Vec3Df(0.f,1.f,0.f), 0.0f, 0.f, 0.f, 1.0f, 0.0f);
    Material groundRed(0.8f, 0.2f, Vec3Df(1.f,0.f,0.f), 0.0f, 0.f, 0.f, 1.0f, 0.0f);
    Material groundBlue(0.8f, 0.2f, Vec3Df(0.f,0.f,1.f), 0.0f, 0.f, 0.f, 1.0f, 0.0f);
    Material groundMat2(0.8f, 0.2f, Vec3Df(0.5f,0.5f,0.5f), 0.0f, 1.f, 1.f, 1.0f, 0.0f);
    Material groundMat3(0.8f, 0.2f, Vec3Df(1.f,1.f,1.f), 0.0f, 1.0f, 1.0f, 1.0f, 0.0f);

	Mesh groundMesh2;
	groundMesh2.loadOFF("models/ground2.off");
	Mesh groundMesh3;
	groundMesh3.loadOFF("models/ground3.off");

	Mesh groundMesh4;
	groundMesh4.loadOFF("models/ground4.off");
	Object ground4(groundMesh4, groundMat);
	ground4.setTrans(Vec3Df(8.f, 0.0f, 8.f));

	Mesh groundMesh5;
	groundMesh5.loadOFF("models/ground5.off");
	Object ground5(groundMesh5, groundMat);
	ground5.setTrans(Vec3Df(0.f, -8.0f, 8.f));

    Object ground (groundMesh, groundMat);    
    Object ground2 (groundMesh2, groundMat2);    
   ground2.setTrans (Vec3Df (0.f, 8.f, 8.f));

   Object ground3 (groundMesh3, groundMat2);    
    ground3.setTrans (Vec3Df (-8.f, 0.0f, 8.f));

    objects.push_back (ground);
    objects.push_back (ground2);
    objects.push_back (ground3);
    //objects.push_back (ground4);
    //objects.push_back (ground5);


    Mesh ramMesh;
    ramMesh.loadOFF ("models/ram.off");
    Material ramMat (1.f, 1.f, Vec3Df (0.f, 1.f, 1.f), 0.0f, 0.0f, 1.0f, 1.5f, 0.8f);
    Object ram (ramMesh, ramMat);
    ram.setTrans (Vec3Df (2.f, 1.0f, 0.f));
	ram.setAnimationFunction(animation1);
    objects.push_back (ram);

    Mesh rhinoMesh;
    rhinoMesh.loadOFF ("models/rhino.off");
    Material rhinoMat (1.0f, 0.2f, Vec3Df (0.6f, 0.6f, 0.7f));
    Object rhino (rhinoMesh, rhinoMat);
    rhino.setTrans (Vec3Df (-1.f, -1.0f, 0.4f));
	rhino.setAnimationFunction(animation2);
    objects.push_back (rhino);
    Mesh gargMesh;
    gargMesh.loadOFF ("models/gargoyle.off");
    Material gargMat (0.7f, 0.4f, Vec3Df (0.5f, 0.8f, 0.5f));
    Object garg (gargMesh, gargMat);
    garg.setTrans (Vec3Df (0.f, 0.0f, 0.4f));
    objects.push_back (garg);

    Light l (Vec3Df (3.0f, 3.0f, 3.0f), Vec3Df (1.0f, 1.0f, 1.0f), 1.0f);
    lights.push_back (l);

    AreaLight al (Vec3Df (-1.0f, -4.0f, 16.0f), Vec3Df (1.0f, 1.f, 1.0f), 0.5f, 1.f, Vec3Df(1.0f,4.0f,-16.0f));
    areaLights.push_back (al);

	AreaLight sun(Vec3Df(1.f, -4.f, 16.f), Vec3Df(1.f, 1.f, 1.f), 0.5f, 1.f, Vec3Df(-1.f, 4.f, -16.f));
	areaLights.push_back(sun);
}
