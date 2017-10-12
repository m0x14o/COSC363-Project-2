// ========================================================================
// COSC 363  Computer Graphics  Lab07
// A simple ray tracer
// ========================================================================

#include <iostream>
#include <cmath>
#include <vector>
#include "Vector.h"
#include "Sphere.h"
#include "Color.h"
#include "Object.h"
#include "Plane.h"
#include "Cylinder.h"
#include "Cone.h"
#include <GL/glut.h>
using namespace std;

const float WIDTH = 20.0;  
const float HEIGHT = 20.0;
const float EDIST = 40.0;
const int PPU = 30;     //Total 600x600 pixels
const int MAX_STEPS = 5;
const float XMIN = -WIDTH * 0.5;
const float XMAX =  WIDTH * 0.5;
const float YMIN = -HEIGHT * 0.5;
const float YMAX =  HEIGHT * 0.5;
bool refracted = false;


vector<Object*> sceneObjects;

Vector light;
Vector light2;

Color backgroundCol;

//A useful struct
struct PointBundle   
{
	Vector point;
	int index;
	float dist;
};

/*
* This function compares the given ray with all objects in the scene
* and computes the closest point  of intersection.
*/
PointBundle closestPt(Vector pos, Vector dir)
{
    Vector  point(0, 0, 0);
	float min = 10000.0;

	PointBundle out = {point, -1, 0.0};

    for(int i = 0;  i < sceneObjects.size();  i++)
	{
        float t = sceneObjects[i]->intersect(pos, dir);
		if(t > 0)        //Intersects the object
		{
			point = pos + dir*t;
			if(t < min)
			{
				out.point = point;
				out.index = i;
				out.dist = t;
				min = t;
			}
		}
	}

	return out;
}

/*
* Computes the colour value obtained by tracing a ray.
* If reflections and refractions are to be included, then secondary rays will 
* have to be traced from the point, by converting this method to a recursive
* procedure.
*/

Color trace(Vector pos, Vector dir, int step)
{
    PointBundle q = closestPt(pos, dir);

    if(q.index == -1) return backgroundCol;        //no intersection


    Vector n = sceneObjects[q.index] -> normal(q.point);
    Vector l = light - q.point;
    l.normalise();
    float lDotn = l.dot(n);
    Color col = sceneObjects[q.index]->getColor();
    Color colorSum;
    
    Vector l2 = light2 - q.point;
    l2.normalise();
    float lDotn2 = l2.dot(n);

    Vector lightVector = light - q.point;
    Vector lightVector2 = light2 - q.point;
    float lightDist = lightVector.length();
    float lightDist2 = lightVector2.length();
    lightVector.normalise();
    lightVector2.normalise();
    Vector r = ((n * 2) * lDotn) - l;
    Vector r2 = ((n * 2) * lDotn2) - l2;
    r.normalise();
    r2.normalise();
    Vector v(-dir.x, -dir.y, -dir.z);
    PointBundle s = closestPt(q.point, lightVector);
    PointBundle s2 = closestPt(q.point, lightVector2);
    
    
    if ((s.index > -1 && s.dist < lightDist) || (s2.index > -1 && s2.dist < lightDist2))
		colorSum = col.phongLight(backgroundCol, 0.0, 0.0);
	else{
		if (lDotn <= 0 || lDotn2 <= 0){
			colorSum = col.phongLight(backgroundCol, 0.0, 0.0);
		}
		else{
			
			float rDotv = r.dot(v);
			float rDotv2 = r2.dot(v);
			float spec;
			float spec2;
			if (rDotv < 0) spec = 0.0;
			else spec = pow(rDotv, 10);
			if (rDotv2 < 0) spec2 = 0.0;
			else spec2 = pow(rDotv2, 10);
			colorSum = col.phongLight(backgroundCol, lDotn, spec);
			colorSum = col.phongLight(backgroundCol, lDotn2, spec2);
			if (q.index == 2)
				colorSum = ((int(q.point.x) - int(q.point.z)) % 2 == 1) ? Color(0, 0, 0) : Color(1, 1, 1);					
		}
	}
	if (q.index == 1 && step < MAX_STEPS){
		Vector reflectionVector = ((n * 2) * (n.dot(v))) - v;
		reflectionVector.normalise();
		Color reflectionCol = trace(q.point, reflectionVector, step+1);
		colorSum.combineColor(reflectionCol, 0.8);
	}
	
	if (q.index == 3 && step < MAX_STEPS){
		float k;
		float theta;
		if (refracted){
			k = 1.015 / 1;
			theta = sqrt(1 - pow(k, 2) * (1 - pow(dir.dot(n), 2)));
			n = n * (-1);
			refracted = false;
		}
		else{
			k = 1 / 1.015;
			theta = sqrt(1 - pow(k, 2) * (1 - pow(dir.dot(n), 2)));
			refracted = true;
		}
		
		Vector g = dir * k - n * (k * (dir.dot(n)) + theta);
		g.normalise();
		return trace(q.point, g, step+1);
		
	}
	
	if (q.index == 4 && step < MAX_STEPS){
		
		float k = 1;
		float theta;
		if (refracted){
			
			theta = sqrt(1 - pow(k, 2) * (1 - pow(dir.dot(n), 2)));
			n = n * (-1);
			refracted = false;
		}
		else{
			
			theta = sqrt(1 - pow(k, 2) * (1 - pow(dir.dot(n), 2)));
			refracted = true;
		}
		
		Vector transparentVector = dir * k - n * (k * (dir.dot(n)) + theta);
		transparentVector.normalise();
		Color transparentCol = trace(q.point, transparentVector, step+1);
		colorSum.combineColor(transparentCol, 0.8);
		
	}

	return colorSum;

}


Color antialiasing(Vector eye, Vector dir, float pixel, float x, float y)
{
	Vector dir1;
	Vector dir2;
	Vector dir3;
	Vector dir4;
	Vector dir5;
	float totalr = 0;
	float totalg = 0;
	float totalb = 0;
	Color aPixel[5];
	
	
	dir1 = Vector(x + pixel / 4, y + pixel / 4, -EDIST);
	dir1.normalise();
	aPixel[0] = trace(eye, dir1, 1);
	
	dir2 = Vector(x + pixel / 2 + pixel / 4, y + pixel / 4, -EDIST);
	dir2.normalise();
	aPixel[1] = trace(eye, dir2, 1);
	
	dir3 = Vector(x + pixel / 4, y + pixel / 2 + pixel / 4, -EDIST);
	dir3.normalise();
	aPixel[2] = trace(eye, dir3, 1);
	
	dir4 = Vector(x + pixel / 2 + pixel / 4, y + pixel / 2 + pixel / 4, -EDIST);
	dir4.normalise();
	aPixel[3] = trace(eye, dir4, 1);
	
	dir5 = Vector(x + pixel / 2, y + pixel / 2, -EDIST);
	dir5.normalise();
	aPixel[4] = trace(eye, dir5, 1);
	/*
	if (fabs(aPixel[0].r - aPixel[4].r) > 0.1 || fabs(aPixel[0].g - aPixel[4].g) > 0.1 || fabs(aPixel[0].b - aPixel[4].b) > 0.1)
		antialiasing(eye, dir1, pixel / 2, x + pixel / 4, y + pixel / 4);
	else{
		if (fabs(aPixel[1].r - aPixel[4].r) > 0.1 || fabs(aPixel[1].g - aPixel[4].g) > 0.1 || fabs(aPixel[1].b - aPixel[4].b) > 0.1)
			antialiasing(eye, dir2, pixel / 2, x + pixel / 2 + pixel / 4, y + pixel / 4);
		else{
			if (fabs(aPixel[2].r - aPixel[4].r) > 0.1 || fabs(aPixel[2].g - aPixel[4].g) > 0.1 || fabs(aPixel[2].b - aPixel[4].b) > 0.1)
				antialiasing(eye, dir3, pixel / 2, x + pixel / 4, y + pixel / 2 + pixel / 4);
			else{
				if (fabs(aPixel[3].r - aPixel[4].r) > 0.1 || fabs(aPixel[3].g - aPixel[4].g) > 0.1 || fabs(aPixel[3].b - aPixel[4].b) > 0.1)
					antialiasing(eye, dir4, pixel / 2, x + pixel / 2 + pixel / 4, y + pixel / 2 + pixel / 4);
				//else{
				//	for (int i = 0; i < 5; i++){
				//		totalr += aPixel[i].r;
				//		totalg += aPixel[i].g;
				//		totalb += aPixel[i].b;
				//	}
				//	return Color(totalr / 5, totalg / 5, totalb / 5);
					
				//}
			}
		}
	}*/
	
	for (int i = 0; i < 5; i++){
		totalr += aPixel[i].r;
		totalg += aPixel[i].g;
		totalb += aPixel[i].b;
	}
	
	return Color(totalr / 5, totalg / 5, totalb / 5);
}


//---The main display module -----------------------------------------------------------
// In a ray tracing application, it just displays the ray traced image by drawing
// each pixel as quads.
//---------------------------------------------------------------------------------------
void display()
{
	int widthInPixels = (int)(WIDTH * PPU);
	int heightInPixels = (int)(HEIGHT * PPU);
	float pixelSize = 1.0/PPU;
	float halfPixelSize = pixelSize/2.0;
	float x1, y1, xc, yc;
	Vector eye(0., 0., 0.);

	glClear(GL_COLOR_BUFFER_BIT);

	glBegin(GL_QUADS);  //Each pixel is a quad.

	for(int i = 0; i < widthInPixels; i++)	//Scan every "pixel"
	{
		x1 = XMIN + i*pixelSize;
		xc = x1 + halfPixelSize;
		for(int j = 0; j < heightInPixels; j++)
		{
			y1 = YMIN + j*pixelSize;
			yc = y1 + halfPixelSize;

		    Vector dir(xc, yc, -EDIST);	//direction of the primary ray
			
		    dir.normalise();			//Normalise this direction
			Color aacolor = antialiasing(eye, dir, pixelSize, x1, y1);
			
		    Color col = aacolor; //trace (eye, dir, 1); //Trace the primary ray and get the colour value
			glColor3f(col.r, col.g, col.b);
			glVertex2f(x1, y1);				//Draw each pixel with its color value
			glVertex2f(x1 + pixelSize, y1);
			glVertex2f(x1 + pixelSize, y1 + pixelSize);
			glVertex2f(x1, y1 + pixelSize);
        }
    }

    glEnd();
    glFlush();
}



void initialize()
{
	//Iniitialize background colour and light's position
	backgroundCol = Color::GRAY;
	light = Vector(10.0, 40.0, -5.0);
	light2 = Vector(-15.0, 40.0, -5.0);
	
	//Add spheres to the list of scene objects here.

	//The following are OpenGL functions used only for drawing the pixels
	//of the ray-traced scene.
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(XMIN, XMAX, YMIN, YMAX);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClearColor(0, 0, 0, 1);
    
    Sphere *sphere1 = new Sphere(Vector(5, 6, -70), 3.0, Color::RED);
    sceneObjects.push_back(sphere1);
    Sphere *sphere2 = new Sphere(Vector(-3, -3, -100), 15.0, Color::BLUE);
    sceneObjects.push_back(sphere2);
    
    Plane *plane = new Plane(Vector(-15, -20, -50), Vector(15, -20, -50), Vector(15, -20, -120), Vector(-15, -20, -120), Color(0, 0, 0));
    sceneObjects.push_back(plane);
    
    Sphere *sphere3 = new Sphere(Vector(-6, -12, -75), 4.0, Color(0, 0, 0));
    sceneObjects.push_back(sphere3);
    
    Sphere *sphere4 = new Sphere(Vector(10, -16, -75), 4.5, Color(0.1, 0.2, 0.2));
    sceneObjects.push_back(sphere4);
    
    Cylinder *cylinder1 = new Cylinder(Vector(13.5, 0, -92), 2.5, 5.0, Color(1, 0, 1));
    sceneObjects.push_back(cylinder1);
    
    Cone *cone1 = new Cone(Vector(8.5, -20, -95), 2.5, 5.0, Color(1, 0, 1));
    sceneObjects.push_back(cone1);
    
    Plane *lef = new Plane(Vector(3, -12, -66), Vector(3, -9, -66), Vector(0, -9, -66), Vector(0, -12, -66), Color(1, 1, 1));
    sceneObjects.push_back(lef);
    Plane *bot = new Plane(Vector(3, -12, -66), Vector(3, -12, -69), Vector(0, -12, -69), Vector(0, -12, -66), Color(1, 1, 1));
    sceneObjects.push_back(bot);
    Plane *bak = new Plane(Vector(0, -12, -66), Vector(0, -12, -69), Vector(0, -9, -69), Vector(0, -9, -66), Color(1, 1, 1));
    sceneObjects.push_back(bak);
    Plane *top = new Plane(Vector(3, -9, -66), Vector(3, -9, -69), Vector(0, -9, -69), Vector(0, -9, -66), Color(1, 1, 1));
    sceneObjects.push_back(top);
    Plane *rit = new Plane(Vector(3, -12, -69), Vector(3, -9, -69), Vector(0, -9, -69), Vector(0, -12, -69), Color(1, 1, 1));
    sceneObjects.push_back(rit);
    Plane *frt = new Plane(Vector(3, -12, -66), Vector(3, -12, -69), Vector(3, -9, -69), Vector(3, -9, -66), Color(1, 1, 1));
    sceneObjects.push_back(frt);
}


int main(int argc, char *argv[]) 
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB );
    glutInitWindowSize(600, 600);
    glutInitWindowPosition(20, 20);
    glutCreateWindow("Raytracing");

    glutDisplayFunc(display);
    initialize();

    glutMainLoop();
    return 0;
}
