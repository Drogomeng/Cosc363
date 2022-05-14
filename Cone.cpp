/*----------------------------------------------------------
* COSC363  Ray Tracer 
*  msh217
*  The Cone class
*  This is a subclass of Object, and hence implements the
*  methods intersect() and normal().
-------------------------------------------------------------*/

#include "Cone.h"
#include <math.h>

/**
* Cone's intersection method.  The input is a ray (pos, dir).
*/
float Cone::intersect(glm::vec3 p0, glm::vec3 dir)
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

    float a = dx*dx + dz*dz - dy*dy*(r/h)*(r/h);
    float b = 2*dx*(x0 - xc) + 2*dz*(z0 - zc) - 2*dy*(r/h)*(r/h) *(y0 - yc - h);
    float c = (x0 - xc)*(x0 - xc) + (z0 - zc)*(z0 - zc) - (r/h)*(r/h) * (h - y0 + yc)*(h - y0 + yc);

    float delta = b*b - 4*a*c;

    if(delta < 0.001) return -1.0;    //includes zero and negative values

    float t1 = (-b - sqrt(delta)) / (2 * a);
    float t2 = (-b + sqrt(delta)) / (2 * a);

    float cone_h = yc + h;
	float intersct_h1 = y0 + dy * t1;
    float intersct_h2 = y0 + dy * t2;

    if (intersct_h1 > cone_h && intersct_h2 > cone_h) {return -1;} 
    else if (intersct_h1 < yc && intersct_h2 < yc) {return -1;} 
    else if (intersct_h1 > cone_h && intersct_h2 <= cone_h && intersct_h2 >= yc) {return t2;}
    else if (intersct_h1 <= cone_h && intersct_h2 > cone_h && intersct_h1 >= yc) {return t1;}
    else if (t1 < 0) { return (t2 > 0) ? t2 : -1;}
	else return t1;
}

/**
* Returns the unit normal vector at a given point.
* Assumption: The input point p lies on the Cone.
*/
glm::vec3 Cone::normal(glm::vec3 p)
{
    float r = radius;
    float h = height;
    
    float x0 = p[0];
    float z0 = p[2];

    float xc = center[0];
    float zc = center[2];

    float etha = atan(r/h); //half cone radius
    float apha = atan((x0 - xc) / (z0 - zc));
    
    float x = sin(apha) * cos(etha);
    float y = sin(etha);
    float z = cos(apha) * cos(etha);
    
    glm::vec3 n = glm::vec3(x, y, z);

    return n;
}
