/*
CSCI 420
Assignment 3 Raytracer
OS: MAC OS X

Name: <Yuzhou Ge>
ID: 7057669325
Email: yuzhouge@usc.edu
*/

#include <stdlib.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include <pic.h>
#include <string.h>
#include <cmath>
#include <float.h>
#include <algorithm>
#include <iostream>
#include "math.cpp"
using namespace std;
using namespace Yuzhou_Math;

#define MAX_TRIANGLES 2000
#define MAX_SPHERES 10
#define MAX_LIGHTS 10

char *filename=0;

//different display modes
#define MODE_DISPLAY 1
#define MODE_JPEG 2
int mode=MODE_DISPLAY;

//supersampling
#define SUPER_SAMPLING 3

//you may want to make these smaller for debugging purposes
#define WIDTH 640
#define HEIGHT 480

//the field of view of the camera
#define fov 60.0

unsigned char buffer[HEIGHT][WIDTH][3];


#define RECURSION_MAX 3 //this defines the level of recursion
bool openRecursion = false;
bool openReflection = false;

struct Vertex
{
  double position[3];
  double color_diffuse[3];
  double color_specular[3];
  double normal[3];
  double shininess;
};

typedef struct _Triangle
{
  struct Vertex v[3];
} Triangle;

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


Triangle triangles[MAX_TRIANGLES];
Sphere spheres[MAX_SPHERES];
Light lights[MAX_LIGHTS];
double ambient_light[3];

int num_triangles=0;
int num_spheres=0;
int num_lights=0;

//function prototypes
void plot_pixel_display(int x,int y,unsigned char r,unsigned char g,unsigned char b);
void plot_pixel_jpeg(int x,int y,unsigned char r,unsigned char g,unsigned char b);
void plot_pixel(int x,int y,unsigned char r,unsigned char g,unsigned char b);
void traceRay(Vec3 &color, Ray r, int depth);
bool intersectLight(Ray r, int thisLight, double &dist);
bool intersectSphere(Ray r, int thisSphere, Vec3 &n, double &dist);
void phongModel(Vec3 intersect, Vec3 n, Vec3 &c, Vec3 kd, Vec3 ks, double a, Vec3 v);
bool intersectTriangle(Ray r, Vec3 &n, int thisTriangle, double &dist, double &a, double &b, double &g);
void save_jpg();
/*camera positions*/
Vec3 cameraPos;
double CAMERA_POS[3] = {0.0, 0.0, 0.0};

//image plane 
Vec3 top_L, top_R, bottom_L, bottom_R; //four corners

//image plane width and height
double image_w = 0, image_h = 0;

//image plane color matrix
Vec3** image_plane;

//pi const number
const double PI = 3.14159265;

const double F = 255.0; //For conveniency, define max color number


void outPut() {
  //output the image.
  int col = 0;
  for (int k = 0; k < WIDTH * SUPER_SAMPLING; k += SUPER_SAMPLING) {
  	glPointSize(2);
  	glBegin(GL_POINTS);
  	int row = 0;
  	for (int t = 0; t < HEIGHT * SUPER_SAMPLING; t += SUPER_SAMPLING) {
  		double r = 0.0, g = 0.0, b = 0.0;
  		for (int i = 0; i < SUPER_SAMPLING; i++) {
  			for (int j = 0; j < SUPER_SAMPLING; j++) {
  				r += image_plane[t+j][k+i].x;
  				g += image_plane[t+j][k+i].y;
  				b += image_plane[t+j][k+i].z;
  			}
  		}
  		r /= pow(SUPER_SAMPLING, 2);
  		g /= pow(SUPER_SAMPLING, 2);
  		b /= pow(SUPER_SAMPLING, 2);
  		plot_pixel(col,row,r,g,b);
  		row++;
  	}
  	glEnd();
  	glFlush();
  	col++;
  }
}


//MODIFY THIS FUNCTION
void draw_scene()
{

  cout << "calculating......" << endl;
  double y = bottom_L.y;
  for (int i = 0; i < HEIGHT * SUPER_SAMPLING; i++) {
  	double x = bottom_L.x;
  	for (int j = 0; j < WIDTH * SUPER_SAMPLING; j++) {
  		//shoot a ray for each pixel
  		Vec3 px(x, y, -1.0);
  		Vec3 direction = normalize(Vminus(px, cameraPos)); //get normalized ray direction
  		Ray r;
  		r.o = cameraPos;
  		r.d = direction;

  		//coloring the pixel
  		Vec3 color(0.0, 0.0, 0.0);
  		traceRay(color, r, 0);
  		
  		//set the color for each pixel
  		image_plane[i][j].x = color.x;
  		image_plane[i][j].y = color.y;
  		image_plane[i][j].z = color.z;

  		x += image_w / (WIDTH * SUPER_SAMPLING);
  	}
  		y += image_h / (HEIGHT * SUPER_SAMPLING);
  }

  //output the image in OPENGL
  outPut();

  if(mode == MODE_JPEG) {
  	save_jpg();
  }
  printf("Done!\n"); fflush(stdout);
}


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
				color.x = lights[i].color[0] * F;
				color.y = lights[i].color[1] * F;
				color.z = lights[i].color[2] * F;
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

				//without recursive reflection
				if(!openReflection) {
					color.x = phongColor.x * F;
					color.y = phongColor.y * F;
					color.z = phongColor.z * F;
					
				} else {
					color.x = pow(1-ks.x, depth+1) * phongColor.x * F;
					color.y = pow(1-ks.y, depth+1) * phongColor.y * F;
					color.z = pow(1-ks.z, depth+1) * phongColor.z * F;
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

				//applying the phong model
				phongModel(intersection, n, phongColor, kd, ks, alpha, v);

				//if not using recursive reflection, just set the color to phong color
				if(!openReflection) {
					color.x = phongColor.x * F;
					color.y = phongColor.y * F;
					color.z = phongColor.z * F;
				} else { //use recursive reflection
					color.x = pow(1-ks.x, depth+1) * phongColor.x * F;
					color.y = pow(1-ks.y, depth+1) * phongColor.y * F;
					color.z = pow(1-ks.z, depth+1) * phongColor.z * F;
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
		color.x += ambient_light[0] * F;
		color.y += ambient_light[1] * F;
		color.z += ambient_light[2] * F;
	} 
	if(!checkIntersect) {
		color.x = color.y = color.z = F;
	}

	//clamp to between 0 to 255
	color.x = max(min(color.x, F), 0.0);
	color.y = max(min(color.y, F), 0.0);
	color.z = max(min(color.z, F), 0.0);
	
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

		//here is the shadow ray
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



void plot_pixel_display(int x,int y,unsigned char r,unsigned char g,unsigned char b)
{
  glColor3f(((double)r)/256.f,((double)g)/256.f,((double)b)/256.f);
  glVertex2i(x,y);
}

void plot_pixel_jpeg(int x,int y,unsigned char r,unsigned char g,unsigned char b)
{
  buffer[HEIGHT-y-1][x][0]=r;
  buffer[HEIGHT-y-1][x][1]=g;
  buffer[HEIGHT-y-1][x][2]=b;
}

void plot_pixel(int x,int y,unsigned char r,unsigned char g, unsigned char b)
{
  plot_pixel_display(x,y,r,g,b);
  if(mode == MODE_JPEG)
      plot_pixel_jpeg(x,y,r,g,b);
}

void save_jpg()
{
  Pic *in = NULL;

  in = pic_alloc(640, 480, 3, NULL);
  printf("Saving JPEG file: %s\n", filename);

  memcpy(in->pix,buffer,3*WIDTH*HEIGHT);
  if (jpeg_write(filename, in))
    printf("File saved Successfully\n");
  else
    printf("Error in Saving\n");

  pic_free(in);      

}

void parse_check(char *expected,char *found)
{
  if(strcasecmp(expected,found))
    {
      char error[100];
      printf("Expected '%s ' found '%s '\n",expected,found);
      printf("Parse error, abnormal abortion\n");
      exit(0);
    }

}

void parse_doubles(FILE*file, char *check, double p[3])
{
  char str[100];
  fscanf(file,"%s",str);
  parse_check(check,str);
  fscanf(file,"%lf %lf %lf",&p[0],&p[1],&p[2]);
  printf("%s %lf %lf %lf\n",check,p[0],p[1],p[2]);
}

void parse_rad(FILE*file,double *r)
{
  char str[100];
  fscanf(file,"%s",str);
  parse_check("rad:",str);
  fscanf(file,"%lf",r);
  printf("rad: %f\n",*r);
}

void parse_shi(FILE*file,double *shi)
{
  char s[100];
  fscanf(file,"%s",s);
  parse_check("shi:",s);
  fscanf(file,"%lf",shi);
  printf("shi: %f\n",*shi);
}

int loadScene(char *argv)
{
  FILE *file = fopen(argv,"r");
  int number_of_objects;
  char type[50];
  int i;
  Triangle t;
  Sphere s;
  Light l;
  fscanf(file,"%i",&number_of_objects);

  printf("number of objects: %i\n",number_of_objects);
  char str[200];

  parse_doubles(file,"amb:",ambient_light);

  for(i=0;i < number_of_objects;i++)
    {
      fscanf(file,"%s\n",type);
      printf("%s\n",type);
      if(strcasecmp(type,"triangle")==0)
	{

	  printf("found triangle\n");
	  int j;

	  for(j=0;j < 3;j++)
	    {
	      parse_doubles(file,"pos:",t.v[j].position);
	      parse_doubles(file,"nor:",t.v[j].normal);
	      parse_doubles(file,"dif:",t.v[j].color_diffuse);
	      parse_doubles(file,"spe:",t.v[j].color_specular);
	      parse_shi(file,&t.v[j].shininess);
	    }

	  if(num_triangles == MAX_TRIANGLES)
	    {
	      printf("too many triangles, you should increase MAX_TRIANGLES!\n");
	      exit(0);
	    }
	  triangles[num_triangles++] = t;
	}
      else if(strcasecmp(type,"sphere")==0)
	{
	  printf("found sphere\n");

	  parse_doubles(file,"pos:",s.position);
	  parse_rad(file,&s.radius);
	  parse_doubles(file,"dif:",s.color_diffuse);
	  parse_doubles(file,"spe:",s.color_specular);
	  parse_shi(file,&s.shininess);

	  if(num_spheres == MAX_SPHERES)
	    {
	      printf("too many spheres, you should increase MAX_SPHERES!\n");
	      exit(0);
	    }
	  spheres[num_spheres++] = s;
	}
      else if(strcasecmp(type,"light")==0)
	{
	  printf("found light\n");
	  parse_doubles(file,"pos:",l.position);
	  parse_doubles(file,"col:",l.color);

	  if(num_lights == MAX_LIGHTS)
	    {
	      printf("too many lights, you should increase MAX_LIGHTS!\n");
	      exit(0);
	    }
	  lights[num_lights++] = l;
	}
      else
	{
	  printf("unknown type in scene description:\n%s\n",type);
	  exit(0);
	}
    }
  return 0;
}

void display()
{

}

//the helper function used to define image plane size and corner
void setImagePlane(double a, double FOV) {
  double x = a * tan(fov / 2 * (PI / 180)); //need to convert to radiant 1 degree = pi/180 rad
  double y = tan(FOV / 2 * (PI/180));
  double z = -1.0;

  //set values for four corners
  top_L.x = -x; top_L.y = y; top_L.z = z;
  top_R.x = x ; top_R.y = y; top_R.z = z;
  bottom_L.x = -x; bottom_L.y = -y; bottom_L.z = z;
  bottom_R.x = x ; bottom_R.y = -y; bottom_R.z = z;

  image_w = 2.0 * x;
  image_h = 2.0 * y;
}

//allocate memory for screen
void allocateScreen() {
  image_plane = new Vec3* [HEIGHT*SUPER_SAMPLING];
  for (int i = 0; i < HEIGHT * SUPER_SAMPLING; ++i) {
    image_plane[i] = new Vec3[WIDTH * SUPER_SAMPLING];
  }
}

void init()
{
  glMatrixMode(GL_PROJECTION);
  glOrtho(0,WIDTH,0,HEIGHT,1,-1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glClearColor(0,0,0,0);
  glClear(GL_COLOR_BUFFER_BIT);

  //set camera position
  cameraPos.x = CAMERA_POS[0];
  cameraPos.y = CAMERA_POS[1];
  cameraPos.z = CAMERA_POS[2];

  //get aspect ratio, used to define image plane
  double aspect_ratio = (double) WIDTH / (double) HEIGHT;

  //calculate the info of image plane
  setImagePlane(aspect_ratio, fov);

  //allocate screen
  allocateScreen();

}

void idle()
{
  //hack to make it only draw once
  static int once=0;
  if(!once)
  {
      draw_scene();
      if(mode == MODE_JPEG)
	save_jpg();
  }
  once=1;
}

int main (int argc, char ** argv)
{
  if (argc<2 || argc > 4)
  {  
    printf ("usage: %s <scenefile> [jpegname]\n", argv[0]);
    exit(0);
  }
  if(argc == 3)
    {
      mode = MODE_JPEG;
      filename = argv[2];
    }
  else if (argc == 4) { //open reflection functionality
  	string s = argv[argc - 1];
  	if (s == "-OR") {
  		openReflection = true;
  		mode = MODE_JPEG;
      	filename = argv[2];
  	}
  }
  else if(argc == 2)
    mode = MODE_DISPLAY;

  glutInit(&argc,argv);
  loadScene(argv[1]);

  glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE);
  glutInitWindowPosition(0,0);
  glutInitWindowSize(WIDTH,HEIGHT);
  int window = glutCreateWindow("Ray Tracer-Yuzhou Ge");
  glutDisplayFunc(display);
  glutIdleFunc(idle);
  init();
  glutMainLoop();
}
