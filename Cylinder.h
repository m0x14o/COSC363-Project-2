#ifndef H_CYLINDER
#define H_CYLINDER

#include "Object.h"

class Cylinder : public Object
{
	private:
		Vector center;
		float radius;
		float height;
	
	public:
		Cylinder()
			: center(Vector()), radius(1), height(1)
		{
			color = Color::WHITE;
		};
		
		Cylinder(Vector c, float r, float h, Color col)
			: center(c), radius(r), height(h)
		{
			color = col;
		};
		
		float intersect(Vector pos, Vector dir);
		
		Vector normal(Vector p);
};
#endif
