#include "Cylinder.h"
#include <math.h>

float Cylinder::intersect(Vector pos, Vector dir)
{
	float a = pow(dir.x, 2) + pow(dir.z, 2);
	float b = 2 * (dir.x * (pos.x - center.x) + dir.z * (pos.z - center.z));
	float c = pow(pos.x - center.x, 2) + pow(pos.z - center.z, 2) - pow(radius, 2);
	
	float delta = pow(b, 2) - (4 * a * c);
	
	if (fabs(delta) < 0.001) return -1.0;
	if (delta < 0.0) return -1.0;
	
	float t1 = (-b - sqrt(delta)) / (2 * a);
	float t2 = (-b + sqrt(delta)) / (2 * a);
	float t = t1;
	
	if (t1 > 0){
		if (t1 < t2) 
			t = t1;
		else{
			if (t2 > 0) 
				t = t2;
			else t = t1;
		}
	}
	else{
		if (t2 > 0)
			t = t1;
		else t = -1;
	}
	
	
	if ((pos.y + t * dir.y) >= center.y && (pos.y + t * dir.y) <= (center.y + height))
		return t;
	else 
		return -1;
}



Vector Cylinder::normal(Vector p)
{
	Vector n = Vector(p.x - center.x, 0, p.z - center.z);
	n.normalise();
	return n;
}
