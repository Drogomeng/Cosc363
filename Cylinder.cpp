/*----------------------------------------------------------
* COSC363  Ray Tracer 
*  msh217
*  The Cone class
*  This is a subclass of Object, and hence implements the
*  methods intersect() and normal().
-------------------------------------------------------------*/

#include "Cylinder.h"
#include <math.h>

/**
* Cone's intersection method.  The input is a ray (pos, dir).
*/
float Cylinder::intersect(glm::vec3 p0, glm::vec3 dir)
{
    float r = radius;
    float h = height;

    float dx = dir[0];
    float dy = dir[1];
    float dz = dir[2];

    float x0 = p0[0];
    float y0 = p0[1];
    float z0 = p0[2];

    float xc = center[0];
    float yc = center[1];
    float zc = center[2];

    float a = dx*dx + dz*dz;
    float b = 2*(dx*(x0 - xc) + dz*(z0 - zc));
    float c = (x0 - xc)*(x0 - xc) + (z0 - zc)*(z0 - zc) - r*r;

    float delta = b*b - 4*a*c;

    if(delta < 0.001) return -1.0;    //includes zero and negative values

    float t1 = (-b - sqrt(delta)) / (2 * a);
    float t2 = (-b + sqrt(delta)) / (2 * a);

    float cylinder_h = yc + h;
	float intersct_h1 = y0 + dy * t1;
    float intersct_h2 = y0 + dy * t2;

    if (intersct_h1 > cylinder_h && intersct_h2 > cylinder_h) {return -1;} 
    else if (intersct_h1 < yc && intersct_h2 < yc) {return -1;}
    else if (intersct_h1 > cylinder_h && intersct_h2 < cylinder_h){
        return (yc + h - y0) / dy;  //with cap t2; 
    }else if (intersct_h2 > cylinder_h && intersct_h1 < cylinder_h)
    {
        return (yc + h - y0) / dy; //with cap t1; 
    }else if (t1 < 0) { return (t2 > 0) ? t2 : -1;}
	else return t1;
}

/**
* Returns the unit normal vector at a given point.
* Assumption: The input point p lies on the Cylinder.
*/
glm::vec3 Cylinder::normal(glm::vec3 p)
{
    float r = radius;

    float x0 = p[0];
    float y0 = p[1];
    float z0 = p[2];

    float xc = center[0];
    float yc = center[1];
    float zc = center[2];
    
    float x = (x0 - xc)/r;
    float y = 0;
    float z = (z0-zc)/r;
    
    glm::vec3 n = glm::vec3(x, y, z);

    return n;
}
