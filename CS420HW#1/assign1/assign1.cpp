/*
  CSCI 420 Computer Graphics
  Assignment 1: Height Fields
  C++ starter code
  Student: YUZHOU GE
  ID: 7057669325
*/

#include <stdlib.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include <pic.h>
#include <ctime>
#include <iostream>
#include <stdio.h>
#include <fstream>
bool stopIdle = true; //boolean controller for idle function
int g_iMenuId;

int g_vMousePos[2] = {0, 0};
int g_iLeftMouseButton = 0;    /* 1 if pressed, 0 if not */
int g_iMiddleMouseButton = 0;
int g_iRightMouseButton = 0;

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROLSTATE;
typedef enum { POINT, LINE, FILL} DisplayMode;

CONTROLSTATE g_ControlState = ROTATE;
DisplayMode dm = FILL;

/* state of the world */
float g_vLandRotate[3] = {0.0, 0.0, 0.0};
float g_vLandTranslate[3] = {0.0, -8, -200};
float g_vLandScale[3] = {1.0, 1.0, 1.0};

/* see <your pic directory>/pic.h for type Pic */
Pic * g_pHeightData;

//these are the data for idle function
GLfloat delta = 2.0;
GLint axis = 2;


//boolean for control screen shots
bool isScreenShot = false;
int numScreenShots = 0;

//time variables
clock_t current_time;
/* Write a screenshot to the specified filename */
void saveScreenshot (char *filename)
{
  int i, j;
  Pic *in = NULL;

  if (filename == NULL)
    return;

  /* Allocate a picture buffer */
  in = pic_alloc(640, 480, 3, NULL);

  printf("File to save to: %s\n", filename);

  for (i=479; i>=0; i--) {
    glReadPixels(0, 479-i, 640, 1, GL_RGB, GL_UNSIGNED_BYTE,
                 &in->pix[i*in->nx*in->bpp]);
  }

  if (jpeg_write(filename, in))
    printf("File saved Successfully\n");
  else
    printf("Error in Saving\n");

  pic_free(in);
}

void myinit()
{
  /* setup gl view here */
  glClearColor(0.0, 0.0, 0.0, 1.0);
  
  //enable z-buffer
  glEnable(GL_DEPTH_TEST);

  //enable smooth gradient
  glShadeModel(GL_SMOOTH);
}


//this is the function for drawing points
void drawPoints() {
  //retrive the height and width of input image
  int height = g_pHeightData->ny;
  int width = g_pHeightData->nx;
    
    //start draw points
    glBegin(GL_POINTS);
    
    //set our point size to 30
    glPointSize(30);

    //loop through each pixal, draw the corresponding vertex
    for (int i = 0; i <= width - 1; i++) {
      for (int j = 0; j <= height - 1; j++) {
        //set vertex color based on each pixal
        float color = (float)(i+j)/500.0;
        glColor3f(0.5, color, 0.66);
        
        //draw the vertex
        glVertex3f(i, j, PIC_PIXEL(g_pHeightData, j, i, 0));
      }
    }

    glEnd();
}

//this is the function for drawing wireframe: LINE-LOOP
void drawWireframe() {
  int height = g_pHeightData->ny;
  int width = g_pHeightData->nx;

    glColor3f(1,1,1); //set initial color to white
    int density = 3; //this parameter controlls the density of wireframe
                     //higher the value, the sparser

    //loop through each pixel based on density and draw the line-loops;
    for (int i = density; i <= height - density; i = i + density) {
      for (int j = density + 1; j < width - density; j = j + density) {
        glBegin(GL_LINE_LOOP);
          glVertex3f(j, i, PIC_PIXEL(g_pHeightData, i, j, 0));
          glColor3f(0, 1, 0);
          glVertex3f(j - density, i, PIC_PIXEL(g_pHeightData, i, j - density, 0));
          glColor3f(1, 1, 1);
          glVertex3f(j - density, i - density, PIC_PIXEL(g_pHeightData, i - density, j - density, 0));
          glColor3f(0,0,1);
        glEnd();  
      } 
    }
}

//helper function used to find the max/min z-value
unsigned int max_z = -1;
unsigned int min_z = 999;
void findMax() {
  int height = g_pHeightData->ny;
  int width = g_pHeightData->nx;
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < height; x++) {
      unsigned char z = PIC_PIXEL(g_pHeightData, y, x, 0);
        if(z > max_z) {
          max_z = z;
        }
        if (z < min_z) {
          min_z = z;
        }
    }
  }
}

//helper function used to draw vertices for drawFill()
void fillhelper(int x, int y) {
  //get the height value based on pixal provided
  unsigned char heightVal = PIC_PIXEL(g_pHeightData, y, x, 0);
  
  //get color for each pixal based on pixal
  float delta = max_z + min_z;
  float color = (float)heightVal/delta;
  glColor3f(color, color, 0.666);

  //draw the vertex
  glVertex3f(x,y,heightVal);
}


void drawFill() {
  //get the height/width of provided image
  int height = g_pHeightData->ny;
  int width = g_pHeightData->nx;
  
  glPushMatrix();
  for (int y = 1; y <= height -2; ++y) {
      for (int x = 1; x < width - 2; ++x) {
        //first triangle
        glBegin(GL_TRIANGLE_STRIP);
          fillhelper(x-1,y);
          fillhelper(x-1,y+1);
          fillhelper(x,y);
          fillhelper(x,y+1);
          fillhelper(x+1,y+1);
          fillhelper(x+1,y);
        glEnd();

        //second triangle
        glBegin(GL_TRIANGLE_STRIP);
          fillhelper(x-1,y);
          fillhelper(x-1,y-1);
          fillhelper(x,y);
          fillhelper(x,y-1);
          fillhelper(x+1,y-1);
          fillhelper(x+1,y);
        glEnd();

      }
  }
  glPopMatrix();
}



void displaySwitch() {
  

  glTranslatef(0,0,-100);
  glScalef(0.1,0.1,0.1);

  //perform rotation for each frame
  glRotatef(g_vLandRotate[0], 1.0, 0.0, 0.0); 
  glRotatef(g_vLandRotate[1], 0.0, 1.0, 0.0); 
  glRotatef(g_vLandRotate[2], 0.0, 0.0, 1.0); 
  
  //perform translation
  glTranslatef(int(g_pHeightData->nx)*-0.5, 0, int(g_pHeightData->ny) * -0.25);

  //Display mode is Fill (Solid Triangles)
  if (dm == FILL) {
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL);
    drawFill();
  }

  //Display mode is Line-Loop
  else if (dm == LINE) {
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE);
    drawWireframe();
  }

  //Display mode is points
  else if (dm == POINT) {
    glPolygonMode( GL_FRONT_AND_BACK, GL_POINT);
    drawPoints();
  }

}


//when we reshape the window, adjust the projection position.
void myReshape(int w, int h) {
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(12.0f, w / h, 0.01, 1000.0);
  glMatrixMode(GL_MODELVIEW);
}

void display()
{
  //call screen shots function here
  clock_t this_time = clock(); //get current time

  //if there is more than 15ms elapsed and we can do the screen shots now
  if (this_time - current_time >= (double)1/15 * 1000 && numScreenShots <= 299 && isScreenShot == true) {
    char fname [2048];
    sprintf(fname, "imageOutput/anim%04d.jpg", numScreenShots);
    saveScreenshot(fname);
    numScreenShots += 1;
  }

  glMatrixMode(GL_MODELVIEW);

  glLoadIdentity();
  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glTranslatef(g_vLandTranslate[0], g_vLandTranslate[1], g_vLandTranslate[2]);

  glScalef(g_vLandScale[0], g_vLandScale[1], g_vLandScale[2]);

  //this links to drawing functions
  displaySwitch();//call the function which decides with style to draw.
  
  glutSwapBuffers();

}

void menufunc(int value)
{
  switch (value)
  {
    case 0:
      exit(0);
      break;
  }
}

//the idle function for auto-rotation
void doIdle()
{
  /* do some stuff... */
  //roate the objects
  g_vLandRotate[2] += delta;
  //g_vLandRotate[1] += delta;
  //g_vLandRotate[0] += delta;

  //end one period
  if (g_vLandRotate[axis] > 360.0) {
  	g_vLandRotate[axis] -= 360.0;
  }

  /* make the screen update */
  glutPostRedisplay();
}

//the idle function for doing nothing
void doIdleNULL() {
	glutPostRedisplay();
}

/* converts mouse drags into information about 
rotation/translation/scaling */
void mousedrag(int x, int y)
{
  int vMouseDelta[2] = {x-g_vMousePos[0], y-g_vMousePos[1]};
  
  switch (g_ControlState)
  {
    case TRANSLATE:  
      if (g_iLeftMouseButton)
      {
        g_vLandTranslate[0] += vMouseDelta[0]*0.10;
        g_vLandTranslate[1] -= vMouseDelta[1]*0.10;
      }
      if (g_iMiddleMouseButton)
      {
        g_vLandTranslate[2] += vMouseDelta[1]*0.01;
      }
      break;
    case ROTATE:
      if (g_iLeftMouseButton)
      {
        g_vLandRotate[0] += vMouseDelta[1];
        g_vLandRotate[1] += vMouseDelta[0];
      }
      if (g_iMiddleMouseButton)
      {
        g_vLandRotate[2] += vMouseDelta[1];
      }
      break;
    case SCALE:
      if (g_iLeftMouseButton)
      {
        g_vLandScale[0] *= 1.0+vMouseDelta[0]*0.01;
        g_vLandScale[1] *= 1.0-vMouseDelta[1]*0.01;
      }
      if (g_iMiddleMouseButton)
      {
        g_vLandScale[2] *= 1.0-vMouseDelta[1]*0.01;
      }
      break;
  }
  g_vMousePos[0] = x;
  g_vMousePos[1] = y;
}

void mouseidle(int x, int y)
{
  g_vMousePos[0] = x;
  g_vMousePos[1] = y;
}

void mousebutton(int button, int state, int x, int y)
{

  switch (button)
  {
    case GLUT_LEFT_BUTTON:
      g_iLeftMouseButton = (state==GLUT_DOWN);
      break;
    case GLUT_MIDDLE_BUTTON:
      g_iMiddleMouseButton = (state==GLUT_DOWN);
      break;
    case GLUT_RIGHT_BUTTON:
      g_iRightMouseButton = (state==GLUT_DOWN);
      break;
  }
 
  switch(glutGetModifiers())
  {
    case GLUT_ACTIVE_CTRL:
      g_ControlState = TRANSLATE;
      break;
    case GLUT_ACTIVE_SHIFT:
      g_ControlState = SCALE;
      break;
    default:
      g_ControlState = ROTATE;
      break;
  }

  g_vMousePos[0] = x;
  g_vMousePos[1] = y;
}


void keyPress(unsigned char key, int x, int y) {
	//trangile--"T" update display mode to fill
	if (key == 116 || key == 84) {
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL);
		dm = FILL;

	}
	//point -- "P", update display mode to line
	if (key == 112 || key == 80) {
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE);
		dm = POINT;
	}

	//wirefram -- "W" update display mode to point
	if (key == 119 || key == 87) {
		glPolygonMode( GL_FRONT_AND_BACK, GL_POINT);
		dm = LINE;
	}

	//stop/start doIdle function
	if (key == ' ') {
		stopIdle = !stopIdle;
	}

	if (stopIdle) {
		glutIdleFunc(doIdleNULL); //do nothing
	} else {
		glutIdleFunc(doIdle); //auto-rotation
	}

	//switch control state;
	if (key == 49) { //1
		g_ControlState = ROTATE;
	}
	if (key == 50) { //2
		g_ControlState = TRANSLATE;
	}
	if (key == 51) { //3
		g_ControlState = SCALE;
	}

  //key 'X' or 'x' for take screen shot.s
  if (key == 120 || key == 88) {
      isScreenShot = true;
      current_time = clock();
      std::cout << current_time << std::endl;
  }
}

int main (int argc, char ** argv)
{ 

  if (argc<2)
  {  
    printf ("usage: %s heightfield.jpg\n", argv[0]);
    exit(1);
  }

  //g_pHeightData is a pointer to pic object
  g_pHeightData = jpeg_read(argv[1], NULL);
  
  //if doesn't read successfully, quit the program
  if (!g_pHeightData)
  {
    printf ("error reading %s.\n", argv[1]);
    exit(1);
  }

  glutInit(&argc,argv);
  
  /*
    create a window here..should be double buffered and use depth testing
  
    the code past here will segfault if you don't have a window set up....
    replace the exit once you add those calls.
  */
  //exit(0);
  //initialize the window mode
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);

  //set window size
  glutInitWindowSize(640, 480);

  //set window position
  glutInitWindowPosition(0,0);

  //now create a window
  glutCreateWindow("Height Field of Yuzhou Ge");

  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL);

  /* allow the user to quit using the right mouse button menu */
   g_iMenuId = glutCreateMenu(menufunc);
   glutSetMenu(g_iMenuId);
   glutAddMenuEntry("Quit",0);
   glutAttachMenu(GLUT_RIGHT_BUTTON);
  
  /* do initialization */
  myinit();
  
  //reshap function, reset the cameras when the window size changed.
  glutReshapeFunc(myReshape);
  
  /* tells glut to use a particular display function to redraw */
  glutDisplayFunc(display);

  /* replace with any animate code */
  glutIdleFunc(doIdleNULL);

  //allow user to press key to switch display mode
  glutKeyboardFunc(keyPress);

  /* callback for mouse drags */
  glutMotionFunc(mousedrag);
  /* callback for idle mouse movement */
  glutPassiveMotionFunc(mouseidle);
  /* callback for mouse button changes */
  glutMouseFunc(mousebutton);

  glutMainLoop();
  return(0);
}
