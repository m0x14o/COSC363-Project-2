#include "Cone.h"
#include <math.h>

float Cone::intersect(Vector pos, Vector dir)
{
	float k = pow(radius / height, 2);
	
	float a = pow(dir.x, 2) + pow(dir.z, 2) - pow(dir.y, 2) * k;
	float b = 2 * (pos.x - center.x) * dir.x + 2 * (pos.z - center.z) * dir.z + 2 * (height - pos.y + center.y) * k * dir.y;
	float c = pow(pos.x - center.x, 2) + pow(pos.z - center.z, 2) - k * pow(height - pos.y + center.y, 2);
	
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



Vector Cone::normal(Vector p)
{
	Vector n = Vector(p.x - center.x, sqrt(pow(p.x - center.x, 2) + pow(p.z - center.z, 2)) * (radius / height), p.z - center.z);
	n.normalise();
	return n;
}
