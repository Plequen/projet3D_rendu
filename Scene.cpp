
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
    //buildDefaultScene ();
    buildReflectionScene ();
    //buildMonkeyScene();
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
void Scene::buildDefaultScene () {

    //Material (float diffuse, float specular, const Vec3Df & color, float glossiness, float reflectivity, float colorBlendingFactor, float refraction, float transparency) 

    Mesh groundMesh;
    groundMesh.loadOFF ("models/ground.off");
    Material groundMat(0.8f, 0.2f, Vec3Df(1.0f,1.0f,1.0f), 0.0f, 0.8f, 0.4f, 1.0f, 0.0f);
    Object ground (groundMesh, groundMat);    
    objects.push_back (ground);
    Mesh ramMesh;
    ramMesh.loadOFF ("models/ram.off");
    Material ramMat (0.8f, 0.2f, Vec3Df (1.f, .6f, .2f));
    Object ram (ramMesh, ramMat);
    ram.setTrans (Vec3Df (1.f, 0.5f, 0.f));
    objects.push_back (ram);
    Mesh rhinoMesh;
    rhinoMesh.loadOFF ("models/rhino.off");
    Material rhinoMat (1.0f, 0.2f, Vec3Df (0.6f, 0.6f, 0.7f));
    Object rhino (rhinoMesh, rhinoMat);
    rhino.setTrans (Vec3Df (-1.f, -1.0f, 0.4f));
    objects.push_back (rhino);
    Mesh gargMesh;
    gargMesh.loadOFF ("models/gargoyle.off");
    Material gargMat (0.7f, 0.4f, Vec3Df (0.5f, 0.8f, 0.5f));
    Object garg (gargMesh, gargMat);
    garg.setTrans (Vec3Df (-1.f, 1.0f, 0.1f));
    objects.push_back (garg);
    Light l (Vec3Df (3.0f, 3.0f, 3.0f), Vec3Df (1.0f, 1.0f, 1.0f), 0.8f);
    lights.push_back (l);
    AreaLight al (Vec3Df (3.0f, 3.0f, 3.0f), Vec3Df (1.0f, 1.f, 1.0f), 0.8f, 2.f, Vec3Df(-1.0f,-1.0f,-1.0f));
    areaLights.push_back (al);
}

// Changer ce code pour creer des scenes originales
void Scene::buildReflectionScene () {
/*
    Material groundMat(0.8f, 0.2f, Vec3Df(1.0f,1.0f,1.0f));
    Mesh cubeMesh;
    cubeMesh.loadOFF ("models/pave.off");
    Object cube (cubeMesh, groundMat);    
    objects.push_back (cube);
    Mesh groundMesh;
    groundMesh.loadOFF ("models/ground.off");
    Object ground (groundMesh, groundMat);    
    objects.push_back (ground);
    Mesh ramMesh;
    ramMesh.loadOFF ("models/ram.off");
    Material ramMat (1.f, 1.f, Vec3Df (1.f, .6f, .2f));
    Object ram (ramMesh, ramMat);
    ram.setTrans (Vec3Df (1.0f, -1.0f, 0.f));
    objects.push_back (ram);
    Mesh wallLeft;
    wallLeft.loadOFF ("models/wall.off");
    Material wallRightMat(0.8f, 0.2f, Vec3Df(1.0f,1.0f,1.0f), 0.0f, 1.0f, 0.0f, 1.0f, 0.0f);
    Object wallRightOb (wallLeft, wallRightMat);    
    wallRightOb.setTrans(Vec3Df(0.0f, -3.0f,3.0f));
    objects.push_back (wallRightOb);
    Object wallLeftOb (wallLeft, wallRightMat);    
    wallLeftOb.setTrans(Vec3Df(0.0f, 3.0f,3.0f));
    objects.push_back (wallLeftOb);
    Mesh wallBackGround;
    wallBackGround.loadOFF ("models/wall.off");
    Material wallForgroundMat(0.8f, 0.2f, Vec3Df(1.0f, 1.0f,1.0f));
    Object wallForgroundOb (wallBackGround, wallForgroundMat);    
    wallForgroundOb.setTrans(Vec3Df(0.0f, 3.0f,3.0f));
    objects.push_back (wallForgroundOb);
*/
    Mesh sphereMesh;
    sphereMesh.loadOFF ("models/sphere.off");
    Material sphereMat(0.8f, 0.2f, Vec3Df(0.5f, 0.3f, 0.3f), 0.0f, 0.0f, 0.0f, 1.4f, 1.0f);
    Object sphere (sphereMesh, sphereMat);    
    sphere.setTrans(Vec3Df(-0.5f, 0.5f,4.0f));
    //sphere.setTrans(Vec3Df(0.0f, 2.8f,3.0f));
    objects.push_back (sphere);
    Mesh monkeyMesh;
    monkeyMesh.loadOFF ("models/monkey.off");
    Material monkeyMat(0.8f, 0.2f, Vec3Df(0.5f, 0.3f, 0.3f));
    Object monkey (monkeyMesh, monkeyMat);    
    monkey.setTrans(Vec3Df(0.2f, -0.2f,0.5f));
    objects.push_back (monkey);
    Mesh groundMesh;
    groundMesh.loadOFF ("models/ground.off");
    Material groundMat(0.8f, 0.2f, Vec3Df(1.0f,1.0f,1.0f));
    Object ground (groundMesh, groundMat);    
    objects.push_back (ground);
    //Mesh topMesh;
    //topMesh.loadOFF ("models/ground.off");
    //Material topMat(0.8f, 0.2f, Vec3Df(1.0f,1.0f,1.0f), 0.0f, 0.5f, 0.2f, 1.0f, 0.0f);
    //Object top (topMesh, topMat);    
    //top.setTrans(Vec3Df(0.0f, 0.0f,18.0f));
    //objects.push_back (top);
    Mesh wallLeft;
    wallLeft.loadOFF ("models/wall2.off");
    Material wallLeftMat(0.8f, 0.2f, Vec3Df(1.0f,0.0f,0.0f));
    Object wallLeftOb (wallLeft, wallLeftMat);    
    wallLeftOb.setTrans(Vec3Df(3.0f, 0.0f,3.0f));
    objects.push_back (wallLeftOb);
 
    Material wallRightMat(0.8f, 0.2f, Vec3Df(0.0f,1.0f,0.0f));
    Object wallRightOb (wallLeft, wallRightMat);    
    wallRightOb.setTrans(Vec3Df(-3.0f, 0.0f,3.0f));
    objects.push_back (wallRightOb);
    
    Mesh wallBackGround;
    wallBackGround.loadOFF ("models/wall.off");
    Material wallBackGroundMat(0.8f, 0.2f, Vec3Df(1.0f,1.0f,1.0f));
    Object wallBackGroundOb (wallBackGround, wallBackGroundMat);    
    wallBackGroundOb.setTrans(Vec3Df(0.0f, -3.0f,3.0f));
    objects.push_back (wallBackGroundOb);
 
    Material wallForgroundMat(0.8f, 0.2f, Vec3Df(1.0f, 1.0f,1.0f));
    Object wallForgroundOb (wallBackGround, wallForgroundMat);    
    wallForgroundOb.setTrans(Vec3Df(0.0f, 3.0f,3.0f));
    objects.push_back (wallForgroundOb);

    Light l (Vec3Df (0.0f, 2.8f,3.0f), Vec3Df (1.0f, 1.0f, 1.0f), 1.0f);
    lights.push_back (l);

    AreaLight al (Vec3Df (0.0f, 2.4f, 3.0f), Vec3Df (1.0f, 1.f, 1.0f), 0.5f, 2.0f, Vec3Df(.0f,-1.0f,0.0f));
    areaLights.push_back (al);

}

void Scene::buildMonkeyScene()
{
    Material groundMat(0.8f, 0.2f, Vec3Df(1.0f,1.0f,1.0f), 0.0f, 0.8f, 0.1f, 1.0f, 0.0f);
    Mesh groundMesh;
    groundMesh.loadOFF ("models/ground.off");
    Object ground (groundMesh, groundMat);    
    ground.setTrans(Vec3Df(0.0f, 0.0f, 2.0f));
    objects.push_back (ground);
    Mesh monkeyMesh;
    monkeyMesh.loadOFF ("models/monkey.off");
    Material monkeyMat(0.8f, 0.2f, Vec3Df(0.5f, 0.3f, 0.3f));
    Object monkey (monkeyMesh, monkeyMat);    
    monkey.setTrans(Vec3Df(1.0f, -1.0f,0.0f));
    objects.push_back (monkey);
    Light l (Vec3Df (2.0f, 2.0f,-1.0f), Vec3Df (1.0f, 1.0f, 1.0f), 1.0f);
    lights.push_back (l);

    AreaLight al (Vec3Df (2.0f, 2.0f, -1.0f), Vec3Df (1.0f, 1.f, 1.0f), 0.5f, 2.f, Vec3Df(.0f,.0f,-5.0f));
    areaLights.push_back (al);
}
