/*==================================================================================
* COSC 363  Computer Graphics (2022)
* Department of Computer Science and Software Engineering, University of Canterbury.
*
* A basic ray tracer
* See Lab07.pdf  for details.
*===================================================================================
*/
#include <iostream>
#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include "Sphere.h"
#include "Cone.h"
#include "Cylinder.h"
#include "SceneObject.h"
#include "Ray.h"
#include "Plane.h"
#include "TextureBMP.h"
#include <GL/freeglut.h>
using namespace std;

const float EDIST = 40.0;
const int NUMDIV = 500;
const int MAX_STEPS = 5;
const float XMIN = -10.0;
const float XMAX = 10.0;
const float YMIN = -10.0;
const float YMAX = 10.0;
//fog range
const float FMAX = -70;
const float FMIN = -120;

TextureBMP texture;

vector<SceneObject*> sceneObjects;


//---The most important function in a ray tracer! ---------------------------------- 
//   Computes the colour value obtained by tracing a ray and finding its 
//     closest point of intersection with objects in the scene.
//----------------------------------------------------------------------------------
glm::vec3 trace(Ray ray, int step)
{
	glm::vec3 backgroundCol(0);						//Background colour = (0,0,0)
	glm::vec3 lightPos1(20, 30, -50);					//Light's position
	glm::vec3 lightPos2(-20, 30, -50);	
	glm::vec3 color(0);
	SceneObject* obj;

    ray.closestPt(sceneObjects);					//Compare the ray with all objects in the scene
    if(ray.index == -1) return backgroundCol;		//no intersection
	obj = sceneObjects[ray.index];					//object on which the closest point of intersection is found

	
	if (ray.index == 0){ //first earth spere
		glm::vec3 nv = sceneObjects[ray.index]->normal(ray.hit);
		glm::vec3 n1 = glm::normalize(nv);
        float s = (n1.x) / (2 * M_PI) + 0.5;
        float t = (n1.y) / (2 * M_PI) + 0.5;

        color = texture.getColorAt(s, t);
		obj->setColor(color);
	}

	if (ray.index == 9) //plan florr index 
	{
		//Stripe pattern
		int stripeWidth = 5;
		int iz = (ray.hit.z) / stripeWidth;
		int k = iz % 2; //2 colors
		if (k == 0) color = glm::vec3(0, 1, 0);
		else color = glm::vec3(1, 1, 0.5);
		obj->setColor(color); 
	}


	

	//color = obj->getColor();						//Object's colour
	color = obj->lighting(lightPos1, lightPos2, -ray.dir, ray.hit);

	//shadow colr
	glm::vec3 lightVec1 = lightPos1 - ray.hit;
	Ray shadowRay1(ray.hit, lightVec1); 
	shadowRay1.closestPt(sceneObjects);
	float lightDist1 = glm::length(lightVec1) ;

	glm::vec3 lightVec2 = lightPos2 - ray.hit;
	Ray shadowRay2(ray.hit, lightVec2); 
	shadowRay2.closestPt(sceneObjects);
	float lightDist2 = glm::length(lightVec2) ;

	if (shadowRay1.index > -1 && shadowRay2.index > -1 && shadowRay1.dist < lightDist1 && shadowRay2.dist < lightDist2)
	{
		glm::vec3 black(0.1,0.1,0.1);
		color = black;
	}

	else if (shadowRay2.index > -1 && shadowRay2.dist < lightDist2)
	{
		SceneObject* shadowobject = sceneObjects[shadowRay2.index];
		if (shadowobject->isRefractive() || shadowobject->isReflective()) {
			color = 0.5f * obj->getColor();
		}else {
			color = 0.2f * obj->getColor();
		}
	}
	else if (shadowRay1.index > -1  && shadowRay1.dist < lightDist1)
	{
		SceneObject* shadowobject = sceneObjects[shadowRay2.index];
		if (shadowobject->isRefractive() || shadowobject->isReflective()) {
			color = 0.5f * obj->getColor();
		}else {
			color = 0.2f * obj->getColor();
		}
	}
	

	//reflaction
	if (obj->isReflective() && step < MAX_STEPS)
	{
		float rho = obj->getReflectionCoeff();
		glm::vec3 normalVec = obj->normal(ray.hit);
		glm::vec3 reflectedDir = glm::reflect(ray.dir, normalVec);
		Ray reflectedRay(ray.hit, reflectedDir);
		glm::vec3 reflectedColor = trace(reflectedRay, step + 1);
		color = color + (rho * reflectedColor);
	}

	//refreactive
	if (obj->isRefractive() && step < MAX_STEPS)
	{
		float rho = obj->getRefractionCoeff();
		float n2= obj->getRefractiveIndex();
		float n1 = 1.0;
		float eta = n1/n2;

		glm::vec3 normalVec = obj->normal(ray.hit);
		glm::vec3 refractDir= glm::refract(ray.dir, normalVec, eta);
		Ray refractRayG(ray.hit, refractDir);

		refractRayG.closestPt(sceneObjects);

		glm::vec3 normalVecM = obj->normal(refractRayG.hit);
		glm::vec3 refractDirH = glm::refract(refractRayG.dir, -normalVecM, 1.0f/eta);
		Ray refractRayH(refractRayG.hit, refractDirH);

		glm::vec3 refractColor = trace(refractRayH, step + 1);
		color = color + (rho * refractColor);
	}

	if (obj->isTransparent() && step < MAX_STEPS)
	{
		float rho = obj->getTransparencyCoeff();
		Ray transpRay1(ray.hit, ray.dir);
		transpRay1.closestPt(sceneObjects);
		Ray transpRay2(transpRay1.hit, transpRay1.dir);
		glm::vec3 transpColor = trace(transpRay2, step + 1);
		color = color + (rho * transpColor);
	}

	//fog
	float ft = (ray.hit.z - FMAX) / (FMIN - FMAX); //liner
	//float ft = 1 - exp(ray.hit.z-FMAX);
	glm::vec3 white(1,1,1);
	if (ray.hit.z < FMIN || ray.hit.z > FMAX) {ft = 0;}
	color = (1 - ft) * color + ft * white;
	return color;
}

//---The main display module -----------------------------------------------------------
// In a ray tracing application, it just displays the ray traced image by drawing
// each cell as a quad.
//---------------------------------------------------------------------------------------
void display()
{
	float xp, yp;  //grid point
	float cellX = (XMAX - XMIN) / NUMDIV;  //cell width
	float cellY = (YMAX - YMIN) / NUMDIV;  //cell height
	glm::vec3 eye(0., 0., 0.);

	glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	glBegin(GL_QUADS);  //Each cell is a tiny quad.

	for (int i = 0; i < NUMDIV; i++)
	{
		xp = XMIN + i * cellX;
		for (int j = 0; j < NUMDIV; j++)
		{
			yp = YMIN + j * cellY;
			glm::vec3 dir1(xp + 0.25 * cellX, yp + 0.75 * cellY, -EDIST);	
			glm::vec3 dir2(xp + 0.25 * cellX, yp + 0.25 * cellY, -EDIST);	
			glm::vec3 dir3(xp + 0.75 * cellX, yp + 0.25 * cellY, -EDIST);	
			glm::vec3 dir4(xp + 0.75 * cellX, yp + 0.75 * cellY, -EDIST);	

			Ray ray1 = Ray(eye, dir1);
			Ray ray2 = Ray(eye, dir2);
			Ray ray3 = Ray(eye, dir3);
			Ray ray4 = Ray(eye, dir4);

			//glm::vec3 dir1(xp + 0.5 * cellX, yp + 0.5 * cellY, -EDIST);	
			//glm::vec3 col = trace(ray1, 1)
			//glColor3f(col.r, col.g, col.b);

			glm::vec3 col = trace(ray1, 1)+trace(ray2, 1)+trace(ray3, 1)+trace(ray4, 1); 
			glColor3f(col.r/4, col.g/4, col.b/4);
			glVertex2f(xp, yp);
			glVertex2f(xp + cellX, yp);
			glVertex2f(xp + cellX, yp + cellY);
			glVertex2f(xp, yp + cellY);
		}
}
    glEnd();
    glFlush();
}



//---This function initializes the scene ------------------------------------------- 
//   Specifically, it creates scene objects (spheres, planes, cones, cylinders etc)
//     and add them to the list of scene objects.
//   It also initializes the OpenGL 2D orthographc projection matrix for drawing the
//     the ray traced image.
//----------------------------------------------------------------------------------
void initialize()
{
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(XMIN, XMAX, YMIN, YMAX);

    glClearColor(0, 0, 0, 1);

	texture = TextureBMP("Butterfly.bmp");

	


	Sphere *sphere1 = new Sphere(glm::vec3(-12, -8, -70.0), 3);        //earth on a cone
	sphere1->setColor(glm::vec3(0, 0, 1));   //Set colour to blue

	Sphere *sphere2 = new Sphere(glm::vec3(0, 0, -105.0), 15);       //mian big boll reflactive
	sphere2->setColor(glm::vec3(1, 0, 0));   //Set colour to red
	sphere2->setReflectivity(true, 0.6);

	Sphere *sphere3 = new Sphere(glm::vec3(-5, -8, -70), 3);    	   //transpartent infront of sphere2
	sphere3->setColor(glm::vec3(1, 0, 0));   //Set colour to red
	sphere3->setTransparency(true, 0.6);
	sphere3->setSpecularity(false);

	Sphere *sphere4 = new Sphere(glm::vec3(11, -5, -70.0), 3);    	   //refreactive with 1.5
	sphere4->setColor(glm::vec3(0, 0, 0));   //Set colour to green
	sphere4->setRefractivity(true, 0.6, 1.5);
	sphere2->setReflectivity(true, 0.6);

	Sphere *sphere5 = new Sphere(glm::vec3(3, -8, -70.0), 3);    	   //refreactive with 1.01
	sphere5->setColor(glm::vec3(0, 0, 0));   //Set colour to green
	sphere5->setRefractivity(true, 1, 1.01);
	sphere2->setReflectivity(true, 0.6);

	//Cone 
	Cone *cone1 = new Cone(glm::vec3(-12, -9, -70), 2, 3);
	cone1->setColor(glm::vec3(0, 1, 1));   //Set colour to clan
	Cone *cone2 = new Cone(glm::vec3(3, -10, -70.0), 2, 3);
	cone2->setColor(glm::vec3(0, 1, 1));   //Set colour to clan

	//cylinder 
	Cylinder *clind1 = new Cylinder(glm::vec3(-5, -10, -70), 1, 1.5);
	clind1->setColor(glm::vec3(0, 1, 1));   //Set colour to clan
	Cylinder *clind2 = new Cylinder(glm::vec3(11, -8, -70.0), 1, 1.5);
	clind2->setColor(glm::vec3(0, 1, 1));   //Set colour to clan

	//plane floor
	Plane *plane = new Plane (glm::vec3(-40., -15, 5), //Point A
	glm::vec3(40., -15, 5), //Point B
	glm::vec3(40., -15, -200), //Point C
	glm::vec3(-40., -15, -200)); //Point D
	plane->setColor(glm::vec3(0.8, 0.8, 0));
	plane->setSpecularity(false);

	//plane Background
    Plane* background = new Plane(glm::vec3(-40., 80, -200), //Point A
	glm::vec3(-40., -20, -200), //Point B
	glm::vec3(40., -20, -200), //Point C
	glm::vec3(40., 80, -200));   //Point D
	background->setColor(glm::vec3(0.2, 0.2, 0.2));
	background->setSpecularity(false);

	//plane left
    Plane* left = new Plane(glm::vec3(-40., 80, 5), //Point A
	glm::vec3(-40., -20, 5), //Point B
	glm::vec3(-40., -20, -200), //Point C
	glm::vec3(-40., 80, -200));   //Point D
	left->setColor(glm::vec3(0.2, 0.2, 0));
	left->setSpecularity(false);

	//plane right
    Plane* right = new Plane(glm::vec3(40., 80, 5), //Point A
	glm::vec3(40., -20, 5), //Point B
	glm::vec3(40., -20, -200), //Point C
	glm::vec3(40., 80, -200));   //Point D
	right->setColor(glm::vec3(0.2, 0, 0.2));
	right->setSpecularity(false);

	//plane top
    Plane *top = new Plane (glm::vec3(-40., 80, 5), //Point A
	glm::vec3(40., 80, 5), //Point B
	glm::vec3(40., 80, -200), //Point C
	glm::vec3(-40., 80, -200)); //Point D
	top->setColor(glm::vec3(0, 0.2, 0.2));
	top->setSpecularity(false);

	//plane back
    Plane* back = new Plane(glm::vec3(-40., 80, 5), //Point A
	glm::vec3(-40., -20, 5), //Point B
	glm::vec3(40., -20, 5), //Point C
	glm::vec3(40., 80, 5));   //Point D
	back->setColor(glm::vec3(0.2, 0.2, 0.2));
	back->setSpecularity(false);

	
	sceneObjects.push_back(sphere1);		 //Add sphere to scene objects
	sceneObjects.push_back(sphere2);		 //Add sphere to scene objects
	sceneObjects.push_back(sphere3);		 //Add sphere to scene objects
	sceneObjects.push_back(sphere4);		 //Add sphere to scene objects
	sceneObjects.push_back(sphere5);		 //Add sphere to scene objects
	sceneObjects.push_back(clind1);
	sceneObjects.push_back(clind2);
	sceneObjects.push_back(cone1);
	sceneObjects.push_back(cone2);
	sceneObjects.push_back(plane);
	sceneObjects.push_back(background);
	sceneObjects.push_back(top);
	sceneObjects.push_back(left);
	sceneObjects.push_back(right);
	sceneObjects.push_back(back);

	Cylinder *clind4 = new Cylinder(glm::vec3(0, 0, -50), 2, 3);
	clind4->setColor(glm::vec3(0, 1, 1));   //Set colour to clan
	sceneObjects.push_back(clind4);
}


int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB );
    glutInitWindowSize(1000, 1000);
    glutInitWindowPosition(20, 20);
    glutCreateWindow("Raytracing");

    glutDisplayFunc(display);
    initialize();

    glutMainLoop();
    return 0;
}
