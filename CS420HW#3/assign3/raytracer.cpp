namespace Yuzhou_RayTracer {
	#include "math.cpp";
	using namespace Yuzhou_Math;
	
	typedef struct _Sphere
	{
	  double position[3];
	  double color_diffuse[3];
	  double color_specular[3];
	  double shininess;
	  double radius;
	} Sphere;

	typedef struct _Light
	{
	  double position[3];
	  double color[3];
	} Light;

	typedef struct _Triangle
	{
	  struct Vertex v[3];
	} Triangle;

	extern int num_triangles;
	extern int num_spheres;
	extern int num_lights;


	extern Triangle triangles[MAX_TRIANGLES];
	extern Sphere spheres[MAX_SPHERES];
	extern Light lights[MAX_LIGHTS];
	extern double ambient_light[3];

	void traceRay(Vec3 &color, Ray r, int depth) {
	//recursion base case
	if (depth > RECURSION_MAX) {
		return;
	}

	bool checkIntersect = false;

	double max_distance = DBL_MAX;

	//lights intersection
	for (int i = 0; i < num_lights; ++i) {
		double distLight;
		if(intersectLight(r, i, distLight)) {
			if(distLight < max_distance) {
				checkIntersect = true;
				max_distance = distLight;
				color.x = lights[i].color[0] * 255.0;
				color.y = lights[i].color[1] * 255.0;
				color.z = lights[i].color[2] * 255.0;
			}
		}
	}

	//sphere intersection
	for (int i = 0; i < num_spheres; ++i) {
		double distSphere = 0.0; 
		Vec3 n; //normal vector
		if (intersectSphere(r, i, n, distSphere)) {
			if(distSphere < max_distance) {
				checkIntersect = true;
				max_distance = distSphere;

				Vec3 intersection = getPos(r, distSphere);
				Vec3 phongColor(0.0, 0.0, 0.0);
				Vec3 kd(spheres[i].color_diffuse[0], spheres[i].color_diffuse[1], spheres[i].color_diffuse[2]);
				Vec3 ks(spheres[i].color_specular[0], spheres[i].color_specular[1], spheres[i].color_specular[2]);
				double alpha = spheres[i].shininess; //shininess coefficient
				Vec3 v(-r.d.x, -r.d.y, -r.d.z);
				v = normalize(v);

				//determine the color
				phongModel(intersection, n, phongColor, kd, ks, alpha, v);

				if(!openReflection) {
					color.x = phongColor.x * 255.0;
					color.y = phongColor.y * 255.0;
					color.z = phongColor.z * 255.0;
					
				} else {
					color.x = pow(1-ks.x, depth+1) * phongColor.x * 255.0;
					color.y = pow(1-ks.y, depth+1) * phongColor.y * 255.0;
					color.z = pow(1-ks.z, depth+1) * phongColor.z * 255.0;
				}

				//recursivley calculating reflection color
				if(openReflection) {
					Vec3 rColor; //reflection color
					traceRay(rColor, r, depth+1);
					color.x += ks.x * rColor.x;
					color.y += ks.y * rColor.y;
					color.z += ks.z * rColor.z;
				}
			}
		}
	}

	//triangle intersection
	for (int i = 0; i < num_triangles; ++i) {
		double distTriangle = 0.0;
		double a,b,g;
		Vec3 n;
		if(intersectTriangle(r, n, i, distTriangle, a, b, g)) {
			if (distTriangle < max_distance) {
				checkIntersect = true;
				max_distance = distTriangle;
				Vec3 intersection(getPos(r, distTriangle));
				
				Vec3 phongColor(0.0, 0.0, 0.0);
				Vec3 kd(triangles[i].v[0].color_diffuse[0] * a + triangles[i].v[1].color_diffuse[0] * b + triangles[i].v[2].color_diffuse[0] * g,
						triangles[i].v[0].color_diffuse[1] * a + triangles[i].v[1].color_diffuse[1] * b + triangles[i].v[2].color_diffuse[1] * g,
						triangles[i].v[0].color_diffuse[2] * a + triangles[i].v[1].color_diffuse[2] * b + triangles[i].v[2].color_diffuse[2] * g
					);

				Vec3 ks(triangles[i].v[0].color_specular[0] * a + triangles[i].v[1].color_specular[0] * b + triangles[i].v[2].color_specular[0] * g,
						triangles[i].v[0].color_specular[1] * a + triangles[i].v[1].color_specular[1] * b + triangles[i].v[2].color_specular[1] * g,
						triangles[i].v[0].color_specular[2] * a + triangles[i].v[1].color_specular[2] * b + triangles[i].v[2].color_specular[2] * g
					);

				double alpha = triangles[i].v[0].shininess * a + triangles[i].v[0].shininess * b + triangles[i].v[0].shininess * g;

				Vec3 v(-r.d.x, -r.d.y, -r.d.z);
				v = normalize(v);

				//calculate interial normal with interpolation using barycentric coordinate
				n.x = triangles[i].v[0].normal[0] * a +
					  triangles[i].v[1].normal[0] * b +
					  triangles[i].v[2].normal[0] * g;

				n.y = triangles[i].v[0].normal[1] * a +
					  triangles[i].v[1].normal[1] * b +
					  triangles[i].v[2].normal[1] * g;

				n.z = triangles[i].v[0].normal[2] * a +
					  triangles[i].v[1].normal[2] * b +
					  triangles[i].v[2].normal[2] * g;

				phongModel(intersection, n, phongColor, kd, ks, alpha, v);

				if(!openReflection) {
					color.x = phongColor.x * 255.0;
					color.y = phongColor.y * 255.0;
					color.z = phongColor.z * 255.0;
				} else {
					color.x = pow(1-ks.x, depth+1) * phongColor.x * 255.0;
					color.y = pow(1-ks.y, depth+1) * phongColor.y * 255.0;
					color.z = pow(1-ks.z, depth+1) * phongColor.z * 255.0;
				}

				//recursivley calculating reflection color
				if(openReflection) {
					Vec3 rColor; //reflection color
					traceRay(rColor, r, depth+1);
					color.x += ks.x * rColor.x;
					color.y += ks.y * rColor.y;
					color.z += ks.z * rColor.z;
				}

			}
		}
	}

	if(checkIntersect) {
		color.x += ambient_light[0] * 255.0;
		color.y += ambient_light[1] * 255.0;
		color.z += ambient_light[2] * 255.0;
	} 
	if(!checkIntersect) {
		color.x = color.y = color.z = 255.0;
	}

	//clamp to between 0 to 255
	color.x = max(min(color.x, 255.0), 0.0);
	color.y = max(min(color.y, 255.0), 0.0);
	color.z = max(min(color.z, 255.0), 0.0);
	
}

//calculate phong color if the intersect is not under shadow
void phongModel(Vec3 intersect, Vec3 n, Vec3 &c, Vec3 kd, Vec3 ks, double a, Vec3 v) {
	//for each light source, check if in shadow, then apply phong model
	for (int i = 0; i < num_lights; ++i) {
		bool underShadow = false;

		Vec3 lightPos(lights[i].position[0], lights[i].position[1], lights[i].position[2]);
		Vec3 origin(intersect.x, intersect.y, intersect.z);
		Vec3 direction(lightPos.x - origin.x, lightPos.y - origin.y, lightPos.z - origin.z);
		direction = normalize(direction);

		Ray sRay; sRay.o = origin; sRay.d = direction; //create the shadow ray

		double distToLight = distance(lightPos, origin); //distance between intersection point and light source

		//check intersection with mid spheres
		for (int j = 0; j < num_spheres; ++j) {
			double distToSphere = 0.0;
			Vec3 dummyN; //dummy normal
			if (intersectSphere(sRay, j, dummyN, distToSphere)) {
				Vec3 p = getPos(sRay, distToSphere); //intersection position
				distToSphere = distance(p, origin); //distance to intersection position
				if (distToSphere <= distToLight) { //is under shadow
					underShadow = true;
				}
			}
		}

		//check intersection with mid triangles.
		for (int k = 0; k < num_triangles; ++k) {
			double distToTriangle = 0.0;
			double a,b,g = 0.0;
			Vec3 dummyN;
			if (intersectTriangle(sRay, dummyN, k, distToTriangle, a, b, g)) {
				Vec3 p = getPos(sRay, distToTriangle);
				distToTriangle = distance(p, origin);
				if (distToTriangle <= distToLight) {
					underShadow = true;
				}
			}
		}

		if (!underShadow) { //apply phong model
			//LN component
			double LN = dot(direction, n);
			if (LN < 0) LN = 0.0;

			//reflection vector
			Vec3 r = getReflection(direction, n);
			r = normalize(r);

			//RV component
			double RV = dot(r, v);
			if(RV < 0.0) RV = 0.0;

			//sum up the values
			c.x += lights[i].color[0] * (kd.x * LN + ks.x * pow(RV, a));
			c.y += lights[i].color[1] * (kd.y * LN + ks.y * pow(RV, a));
			c.z += lights[i].color[2] * (kd.z * LN + ks.z * pow(RV, a));
		}
	}
}


//check ray intersects a single triangle
bool intersectTriangle(Ray r, Vec3 &n, int thisTriangle, double &dist, double &a, double &b, double &g) {
	//retrive triangle vertices
	Vec3 p1(triangles[thisTriangle].v[0].position[0], triangles[thisTriangle].v[0].position[1], triangles[thisTriangle].v[0].position[2]);
	Vec3 p2(triangles[thisTriangle].v[1].position[0], triangles[thisTriangle].v[1].position[1], triangles[thisTriangle].v[1].position[2]);
	Vec3 p3(triangles[thisTriangle].v[2].position[0], triangles[thisTriangle].v[2].position[1], triangles[thisTriangle].v[2].position[2]);

	n = normalize(cross(Vminus(p2, p1), Vminus(p3,p1))); //calculate unit vector

	double ND = dot(n, r.d);
	if(ND == 0) { //ray parallel to plane
		return false;
	}
	dist = -1 * (dot(Vminus(r.o, p1), n)) / ND;
	if (dist <= 0.01) {
		return false;
	}

	//intersection point
	Vec3 s(getPos(r, dist));

	//test point if in triangle using barycentric coordinates
	double area = 0.5 * dot(cross(Vminus(p2, p1), Vminus(p3, p1)), n);
	a = 0.5 * dot(cross(Vminus(p2, p1), Vminus(s, p1)), n) / area;
	b = 0.5 * dot(cross(Vminus(p3, p2), Vminus(s, p2)), n) / area;
	g = 0.5 * dot(cross(Vminus(p1, p3), Vminus(s, p3)), n) / area;
	if(a >= 0 && b >= 0 && g >= 0) {
		g = 1.0 - a - b;
		return true;
	}
	return false;
}


//check ray intersects a single sphere
bool intersectSphere(Ray r, int thisSphere, Vec3 &n, double &dist) {
	//calculate coefficients
	double radius  = spheres[thisSphere].radius;
	double a = 1.0;
	double b = 2.0 * (
			r.d.x * (r.o.x - spheres[thisSphere].position[0]) + 
			r.d.y * (r.o.y - spheres[thisSphere].position[1]) + 
			r.d.z * (r.o.z - spheres[thisSphere].position[2])
		);
	double c = pow((r.o.x - spheres[thisSphere].position[0]), 2.0) + 
			   pow((r.o.y - spheres[thisSphere].position[1]), 2.0) + 
			   pow((r.o.z - spheres[thisSphere].position[2]), 2.0) - pow(radius, 2.0);

	double delta = pow(b,2.0) - 4.0*c;

	if (delta < 0) return false;

	double t0 = (-b + sqrt(delta)) / 2;
	double t1 = (-b - sqrt(delta)) / 2;

	if (t0 <= 0 && t1 <= 0) return false;

	if (t0 > 0 && t1 > 0) dist = min(t0, t1);
	else dist = max(t0, t1); //special case, ray inside of sphere

	if (dist < 0.0001) return false;

	//get the unit normal for phong shading
	Vec3 v = getPos(r, dist);
	n.x = v.x - spheres[thisSphere].position[0];
	n.y = v.y - spheres[thisSphere].position[1];
	n.z = v.z - spheres[thisSphere].position[2];

	//normalize to unit vector
	n = normalize(n);
	return true;
}

//check ray intersects a single light source
bool intersectLight(Ray r, int thisLight, double &dist) {
	//light at ray origin
	if (lights[thisLight].position[0] == r.o.x && 
		lights[thisLight].position[1] == r.o.y && 
		lights[thisLight].position[2] == r.o.z 
		) {return false;}

	//check if intersect
	dist = (lights[thisLight].position[0] - r.o.x) / r.d.x;
	if (dist != (lights[thisLight].position[1] - r.o.y) / r.d.y) {return false;}
	if (dist != (lights[thisLight].position[2] - r.o.z) / r.d.z) {return false;}
	return true;
}
}