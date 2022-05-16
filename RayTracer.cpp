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
const int NUMDIV = 300; //500
const int MAX_STEPS = 5;
const float XMIN = -10.0;
const float XMAX = 10.0;
const float YMIN = -10.0;
const float YMAX = 10.0;
// //fog range
// const float Z1 = -80;
// const float Z2 = -200;
//NO FOG
const float Z1 = 0;
const float Z2 = 0;

TextureBMP texture;

vector<SceneObject*> sceneObjects;


//---The most important function in a ray tracer! ---------------------------------- 
//   Computes the colour value obtained by tracing a ray and finding its 
//     closest point of intersection with objects in the scene.
//----------------------------------------------------------------------------------
glm::vec3 trace(Ray ray, int step)
{
	glm::vec3 backgroundCol(0);						//Background colour = (0,0,0)
	glm::vec3 lightPos1(15, 29, 3);					//Light's position
	glm::vec3 lightPos2(-15, 29, 3);	
	glm::vec3 color(0);
	SceneObject* obj;

    ray.closestPt(sceneObjects);					//Compare the ray with all objects in the scene
    if(ray.index == -1) return backgroundCol;		//no intersection
	obj = sceneObjects[ray.index];					//object on which the closest point of intersection is found

	
	if (ray.index == 16){ //first earth spere index 16
		glm::vec3 nv = sceneObjects[ray.index]->normal(ray.hit);
		glm::vec3 n1 = glm::normalize(nv);
        // float s = (n1.x) / (2 * M_PI) + 0.5;
        // float t = (n1.y) / (2 * M_PI) + 0.5; atan2 

		float s = 0.5 + (atan2(n1.z, n1.x)) / (2 * M_PI) ;
        float t = 0.5 - (asin(n1.y)) / (M_PI); 

        color = texture.getColorAt(s, t);
		obj->setColor(color);
	}
	
	if (ray.index == 4) //table_plane index  4
	{
		//Stripe pattern
		int stripeWidth = 5;
		int iz = (ray.hit.z) / stripeWidth;
		int k = iz % 2; //2 colors
		if (k == 0) color = glm::vec3(1, 0, 0);
		else color = glm::vec3(0, 0.5, 0.5);
		obj->setColor(color); 
	}

	if (ray.index == 5) //floor index 5
	{
		//Stripe pattern
		int stripeWidth = 5;
		int iz = (ray.hit.z) / stripeWidth;
		int k = iz % 2; //2 colors
		if (k == 0) color = glm::vec3(0, 1, 0);
		else color = glm::vec3(1, 1, 0.5);
		obj->setColor(color); 
	}

	if (ray.index == 6)  //backbrounf
	{
		float x1 = 40, x2 = -40, y1=30,y2=-15;
		//Add code for texture mapping here
		float texcoords = (ray.hit.x - x1)/(x2-x1);
		float texcoordt = (ray.hit.y - y1)/(y2-y1);
		if(texcoords > 0 && texcoords < 1 &&
		texcoordt > 0 && texcoordt < 1)
		{
			color=texture.getColorAt(texcoords, texcoordt);
			obj->setColor(color);
		}
	}
	
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
	float ft = (ray.hit.z - Z1) / (Z2 - Z1); //liner
	//float ft = 1 - exp(ray.hit.z-FMAX);
	glm::vec3 white(1,1,1);
	if (ray.hit.z < Z2 || ray.hit.z > Z1) {ft = 0;}
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
			
			// //normal 
			// glm::vec3 dir1(xp + 0.5 * cellX, yp + 0.5 * cellY, -EDIST);	
			// Ray ray1 = Ray(eye, dir1);
			// glm::vec3 col = trace(ray1, 1);
			// glColor3f(col.r, col.g, col.b);
			
			//adptive anti-analising
			glm::vec3 dir1(xp + 0.25 * cellX, yp + 0.75 * cellY, -EDIST);	
			glm::vec3 dir2(xp + 0.25 * cellX, yp + 0.25 * cellY, -EDIST);	
			glm::vec3 dir3(xp + 0.75 * cellX, yp + 0.25 * cellY, -EDIST);	
			glm::vec3 dir4(xp + 0.75 * cellX, yp + 0.75 * cellY, -EDIST);	

			Ray ray1 = Ray(eye, dir1); glm::vec3 col1 = trace(ray1, 1);
			Ray ray2 = Ray(eye, dir2); glm::vec3 col2 = trace(ray2, 1);
			Ray ray3 = Ray(eye, dir3); glm::vec3 col3 = trace(ray3, 1);
			Ray ray4 = Ray(eye, dir4); glm::vec3 col4 = trace(ray4, 1);

			float average_col = (glm::length(col1) + glm::length(col2) + glm::length(col3) + glm::length(col4))/4;
			float diff1_mean = glm::length(col1) - average_col;
			float diff2_mean = glm::length(col2) - average_col;
			float diff3_mean = glm::length(col3) - average_col;
			float diff4_mean = glm::length(col4) - average_col;
			glm::vec3 avg(4,4,4);

			if (diff1_mean > diff2_mean && diff1_mean > diff3_mean && diff1_mean > diff4_mean && diff1_mean >= 0.1f*average_col)
			{
				//point (0.25, 0.75)
				glm::vec3 dir11(xp + 0.125 * cellX, yp + 0.875 * cellY, -EDIST);	
				glm::vec3 dir12(xp + 0.125 * cellX, yp + 0.625 * cellY, -EDIST);	
				glm::vec3 dir13(xp + 0.35 * cellX, yp + 0.625 * cellY, -EDIST);	
				glm::vec3 dir14(xp + 0.35 * cellX, yp + 0.875 * cellY, -EDIST);	

				Ray ray11 = Ray(eye, dir11); glm::vec3 col11 = trace(ray11, 1);
				Ray ray12 = Ray(eye, dir12); glm::vec3 col12 = trace(ray12, 1);
				Ray ray13 = Ray(eye, dir13); glm::vec3 col13 = trace(ray13, 1);
				Ray ray14 = Ray(eye, dir14); glm::vec3 col14 = trace(ray14, 1);

				glm::vec3 col1 = col11 + col12 + col13 + col14;
				col1 = col1/avg;
			}

			else if (diff2_mean > diff1_mean && diff2_mean > diff3_mean && diff2_mean > diff4_mean && diff2_mean >= 0.1f*average_col)
			{
				//point (0.25, 0.25)
				glm::vec3 dir21(xp + 0.125 * cellX, yp + 0.375 * cellY, -EDIST);	
				glm::vec3 dir22(xp + 0.125 * cellX, yp + 0.125 * cellY, -EDIST);	
				glm::vec3 dir23(xp + 0.375 * cellX, yp + 0.125 * cellY, -EDIST);	
				glm::vec3 dir24(xp + 0.375 * cellX, yp + 0.375 * cellY, -EDIST);	

				Ray ray21 = Ray(eye, dir21); glm::vec3 col21 = trace(ray21, 1);
				Ray ray22 = Ray(eye, dir22); glm::vec3 col22 = trace(ray22, 1);
				Ray ray23 = Ray(eye, dir23); glm::vec3 col23 = trace(ray23, 1);
				Ray ray24 = Ray(eye, dir24); glm::vec3 col24 = trace(ray24, 1);

				glm::vec3 col2 = col21 + col22 + col23 + col24;
				col2 = col2/avg;
			}

			else if (diff3_mean > diff1_mean && diff3_mean > diff2_mean && diff3_mean > diff4_mean && diff3_mean >= 0.1f*average_col)
			{
				//point (0.75, 0.25)
				glm::vec3 dir31(xp + 0.625 * cellX, yp + 0.375 * cellY, -EDIST);	
				glm::vec3 dir32(xp + 0.625 * cellX, yp + 0.125 * cellY, -EDIST);	
				glm::vec3 dir33(xp + 0.875 * cellX, yp + 0.125 * cellY, -EDIST);	
				glm::vec3 dir34(xp + 0.875 * cellX, yp + 0.375 * cellY, -EDIST);	

				Ray ray31 = Ray(eye, dir31); glm::vec3 col31 = trace(ray31, 1);
				Ray ray32 = Ray(eye, dir32); glm::vec3 col32 = trace(ray32, 1);
				Ray ray33 = Ray(eye, dir33); glm::vec3 col33 = trace(ray33, 1);
				Ray ray34 = Ray(eye, dir34); glm::vec3 col34 = trace(ray34, 1);

				glm::vec3 col3 = col31 + col32 + col33 + col34;
				col3 = col3/avg;
			}

			else if (diff4_mean > diff1_mean && diff4_mean > diff2_mean && diff4_mean > diff3_mean && diff4_mean >= 0.1f*average_col)
			{
				//point (0.75, 0.75)
				glm::vec3 dir41(xp + 0.625 * cellX, yp + 0.875 * cellY, -EDIST);	
				glm::vec3 dir42(xp + 0.625 * cellX, yp + 0.625 * cellY, -EDIST);	
				glm::vec3 dir43(xp + 0.875 * cellX, yp + 0.625 * cellY, -EDIST);	
				glm::vec3 dir44(xp + 0.875 * cellX, yp + 0.875 * cellY, -EDIST);	

				Ray ray41 = Ray(eye, dir41); glm::vec3 col41 = trace(ray41, 1);
				Ray ray42 = Ray(eye, dir42); glm::vec3 col42 = trace(ray42, 1);
				Ray ray43 = Ray(eye, dir43); glm::vec3 col43 = trace(ray43, 1);
				Ray ray44 = Ray(eye, dir44); glm::vec3 col44 = trace(ray44, 1);

				glm::vec3 col4 = col41 + col42 + col43 + col44;
				col4 = col4/avg;
			}

			glm::vec3 col = col1 + col2 + col3 + col4;
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
void creat_table()
{
	//table
	//4 table foot 
	Cylinder *table_foot1 = new Cylinder(glm::vec3(-5, -10, -50), 0.2, 4.5); table_foot1->setColor(glm::vec3(0, 1, 1));
	Cylinder *table_foot2 = new Cylinder(glm::vec3(-5, -10, -60), 0.2, 4.5); table_foot2->setColor(glm::vec3(0, 1, 1));
	Cylinder *table_foot3 = new Cylinder(glm::vec3(5, -10, -50), 0.2, 4.5); table_foot3->setColor(glm::vec3(0, 1, 1));
	Cylinder *table_foot4 = new Cylinder(glm::vec3(5, -10, -60), 0.2, 4.5); table_foot4->setColor(glm::vec3(0, 1, 1));
	//plane table
	Plane *table_plane = new Plane (glm::vec3(-10., -5, -45), //Point A
	glm::vec3(10., -5, -45), //Point B
	glm::vec3(10., -5, -65), //Point C
	glm::vec3(-10., -5, -65)); //Point D
	table_plane->setColor(glm::vec3(1, 1, 1));
	table_plane->setSpecularity(false);

	sceneObjects.push_back(table_foot1); //index0
	sceneObjects.push_back(table_foot2);
	sceneObjects.push_back(table_foot3);
	sceneObjects.push_back(table_foot4);
	sceneObjects.push_back(table_plane); //index 4

}

void create_box()
{
	//plane floor
	Plane *plane = new Plane (glm::vec3(-40., -15, 5), //Point A
	glm::vec3(40., -15, 5), //Point B
	glm::vec3(40., -15, -350), //Point C
	glm::vec3(-40., -15, -350)); //Point D
	plane->setColor(glm::vec3(1, 0, 0));
	plane->setSpecularity(false);

	//plane Background
    Plane* background = new Plane(glm::vec3(-40., 30, -200), //Point A
	glm::vec3(-40., -15, -200), //Point B
	glm::vec3(40., -15, -200), //Point C
	glm::vec3(40., 30, -200));   //Point D
	background->setColor(glm::vec3(1, 0, 0));
	background->setSpecularity(false);

	//plane left
    Plane* left = new Plane(glm::vec3(-40., 30, 5), //Point A
	glm::vec3(-40., -15, 5), //Point B
	glm::vec3(-40., -15, -350), //Point C
	glm::vec3(-40., 30, -350));   //Point D
	left->setColor(glm::vec3(1, 0, 0));
	left->setSpecularity(false);

	//plane right
    Plane* right = new Plane(glm::vec3(40., 30, 5), //Point A
	glm::vec3(40., 30, -350),   //Point D
	glm::vec3(40., -15, -350), //Point C
	glm::vec3(40., -15, 5));   //Point D
	right->setColor(glm::vec3(1, 0, 0));
	right->setSpecularity(false);

	//plane top
    Plane *top = new Plane (glm::vec3(-40., 30, 5), //Point A
	glm::vec3(-40., 30, -350),
	glm::vec3(40., 30, -350),
	glm::vec3(40., 30, 5)); //Point D
	top->setColor(glm::vec3(1,0,0));
	top->setSpecularity(false);

	//plane back  
    Plane* back = new Plane(glm::vec3(-40., 30, 5), //Point A
	glm::vec3(40., 30, 5),
	glm::vec3(40., -15, 5), 
	glm::vec3(-40., -15, 5));   //Point D
	back->setColor(glm::vec3(1, 0, 0));
	back->setSpecularity(false);

	sceneObjects.push_back(plane); //index5
	sceneObjects.push_back(background);
	sceneObjects.push_back(top);
	sceneObjects.push_back(left);
	sceneObjects.push_back(right);
	sceneObjects.push_back(back); //index10

}

void create_mirror_at_top()
{
	Plane* mirror = new Plane(glm::vec3(-10., 10, -80.5), //Point A
	glm::vec3(10., 10, -80.5), //Point B
	glm::vec3(10., 20, -76), //Point C
	glm::vec3(-10., 20, -76));   //Point D
	mirror->setColor(glm::vec3(0, 0, 0));
	mirror->setReflectivity(true, 1.01);

	sceneObjects.push_back(mirror); //index 11

}

void creat_objs_on_table()
{
	Sphere *sphere1 = new Sphere(glm::vec3(0, 0, -60.0), 5); 
	sphere1->setColor(glm::vec3(0, 0, 0));   //Set colour to blue
	sphere1->setRefractivity(true, 1, 1.5);

	Sphere *sphere2 = new Sphere(glm::vec3(-5, -3, -50), 2); 
	sphere2->setColor(glm::vec3(1, 0, 0));   //Set colour to blue
	sphere2->setTransparency(true);

	Cylinder *clind1 = new Cylinder(glm::vec3(-8, -5, -60), 2, 10);
	clind1->setColor(glm::vec3(0, 1, 1));   //Set colour to clan

	Cone *cone1 = new Cone(glm::vec3(7, -5, -60), 2, 5);
	cone1->setColor(glm::vec3(0, 1, 1));   //Set colour to clan

	//earth
	Sphere *sphere3 = new Sphere(glm::vec3(7, 3, -60.0), 3); 
	sphere3->setColor(glm::vec3(0, 0, 1));   //Set colour to blue  -8, 0

	sceneObjects.push_back(sphere1); //index 12
	sceneObjects.push_back(sphere2);
	sceneObjects.push_back(clind1);
	sceneObjects.push_back(cone1);  //index 15
	sceneObjects.push_back(sphere3); //index16
}


void initialize()
{
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(XMIN, XMAX, YMIN, YMAX);
    glClearColor(0, 0, 0, 1);

	texture = TextureBMP("Butterfly.bmp");

	creat_table(); //index 0-4
	create_box();  //index 5-10
	create_mirror_at_top(); //index 11
	creat_objs_on_table(); //index 12-16

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
