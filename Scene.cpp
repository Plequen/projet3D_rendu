
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

// Changer ce code pour creer des scenes originales
void Scene::buildDefaultScene () {
    Mesh groundMesh;
    groundMesh.loadOFF ("models/ground.off");
	// gloss, reflec, blending, refrac, transp
    Material groundMat(0.8f, 0.2f, Vec3Df(0.5f,0.5f,0.5f), 0.0f, 0.f, 0.f, 1.0f, 0.0f);
    Material groundMat2(0.8f, 0.2f, Vec3Df(0.5f,0.5f,0.5f), 0.0f, 1.f, 1.f, 1.0f, 0.0f);
    Material groundMat3(0.8f, 0.2f, Vec3Df(1.f,1.f,1.f), 0.0f, 1.0f, 1.0f, 1.0f, 0.0f);

	Mesh groundMesh2;
	groundMesh2.loadOFF("models/ground2.off");
	Mesh groundMesh3;
	groundMesh3.loadOFF("models/ground3.off");

    Object ground (groundMesh, groundMat);    
    Object ground2 (groundMesh2, groundMat3);    
   ground2.setTrans (Vec3Df (0.f, 1.95f, 1.95f));

   Object ground3 (groundMesh3, groundMat3);    
    ground3.setTrans (Vec3Df (-1.95f, 0.0f, 1.95f));

    objects.push_back (ground);
    objects.push_back (ground2);
    objects.push_back (ground3);


    Mesh ramMesh;
    ramMesh.loadOFF ("models/sphere.off");
    Material ramMat (1.f, 1.f, Vec3Df (1.f, .6f, .2f), 0.0f, 0.f, 1.0f, 1.2f, 0.8f);
    Object ram (ramMesh, ramMat);
    ram.setTrans (Vec3Df (0.f, 0.0f, 1.f));
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
    garg.setTrans (Vec3Df (-1.f, 1.0f, 0.1f));
    objects.push_back (garg);

    Light l (Vec3Df (3.0f, 3.0f, 3.0f), Vec3Df (1.0f, 1.0f, 1.0f), 1.0f);
    lights.push_back (l);

    AreaLight al (Vec3Df (4.0f, -2.0f, 5.0f), Vec3Df (1.0f, 1.f, 1.0f), 1.f, 2.f, Vec3Df(-4.0f,2.0f,-3.0f));
    areaLights.push_back (al);

	AreaLight sun(Vec3Df(4.f, 2.f, 5.f), Vec3Df(1.f, 1.f, 1.f), 1.f, 2.f, Vec3Df(-4.f, 2.f, -5.f));
	areaLights.push_back(sun);
}
