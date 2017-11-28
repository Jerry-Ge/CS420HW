/*
this file contains all math helpfer functions
including: 
dot,cross product, 
normalization vector,
vector minus,
vector add,
calculate refelction vector,
get position of a point given parameter t
*/

namespace Yuzhou_Math {
//self-defined vector container
struct Vec3 {
  double x, y, z;
  Vec3() {x=y=z=0;}
  Vec3(double x, double y, double z) : x(x), y(y), z(z) {}
};

struct Ray {
  Vec3 o; //origin of ray
  Vec3 d; //direction of ray
};

/*math helper functions*/
//dot product
double dot(Vec3 i, Vec3 j) {
	return i.x*j.x + i.y*j.y + i.z*j.z;
}

//cross product
Vec3 cross(Vec3 a, Vec3 b) {
	double i = a.y*b.z - b.y*a.z;
	double j = b.x*a.z - a.x*b.z;
	double k = a.x*b.y - b.x*a.y;
	Vec3 v(i, j, k);
	return v;
}

//turn a vector into unit vector
Vec3 normalize(Vec3 v) {
	double magnitude = sqrt(pow(v.x, 2) + pow(v.y,2) + pow(v.z,2));
	Vec3 vec(v.x / magnitude, v.y / magnitude, v.z / magnitude);
	return vec;
}

//Vminus operator
Vec3 Vminus(Vec3 a, Vec3 b) {
	Vec3 v(a.x - b.x, a.y - b.y, a.z - b.z);
	return v;
}

//add operator
Vec3 add(Vec3 a, Vec3 b) {
	Vec3 v(a.x + b.x, a.y + b.y, a.z + b.z);
	return v;
}

//return distance between two vectors
double distance(Vec3 a, Vec3 b) {
	return sqrt(pow(a.x-b.x, 2) + pow(a.y-b.y,2) + pow(a.z-b.z,2));
}

//get position vector of ray given parameter t
Vec3 getPos(Ray r, double t) {
	Vec3 v(r.o.x + t*r.d.x, r.o.y + t*r.d.y, r.o.z + t*r.d.z); //postion vector
	return v;
}

//given incoming and normal vector, get reflection vector
Vec3 getReflection(Vec3 i, Vec3 n) {
	double dotScalar = dot(i,n);
	Vec3 r;
	r.x = 2 * dotScalar * n.x - i.x;
	r.y = 2 * dotScalar * n.y - i.y;
	r.z = 2 * dotScalar * n.z - i.z;
	return r;
}
}