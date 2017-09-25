/*
  CSCI 420 Computer Graphics
  Assignment 1: Height Fields
  C++ starter code
*/

#include <stdlib.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include <pic.h>
bool stopIdle = false;
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

//float cameraDistance = 50.0;
//float cameraRotation[2] = {0.0, 0.0};


/* see <your pic directory>/pic.h for type Pic */
Pic * g_pHeightData;

//float cameraPos[3] = {0.0f, 0.0f,  300.0f}; // eye
//float cameraFocus[3] = {0.0f, 0.0f, -1.0f}; // focus
//float cameraUp[3] = {0.0f, 1.0f,  0.0f}; // up vector

GLfloat delta = 2.0;
GLint axis = 1;

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
  glEnable(GL_DEPTH_TEST);
  glShadeModel(GL_SMOOTH);
}


void drawPoints() {
  int height = g_pHeightData->ny;
  int width = g_pHeightData->nx;
  // initialize points
    glBegin(GL_POINTS); 
    // initialize the point size etc  
    glPointSize(30.0);//set the point size for each dot drawn

    // cache a color array
    float color;

    // loop through each element and draw the correct point
    for (int x = width - 1; x >= 0; --x) {

      // loop through each y value
      for (int y = height - 1; y >= 0; --y) {

        // grab the proper color from the heightfield
        color = 1.0;

        // now that the color has been determined, draw it out
        glColor3f(color, color, color);

        // grab the vertice and draw it on screen 
        GLfloat glf[3] = {x, y, PIC_PIXEL(g_pHeightData, y, x, 0)};
        glVertex3fv(&glf[0]);//draw the proper vertex on screen 
      }
    } //end of the y for loop

    glEnd();//end the points drawing

}


void drawWireframe() {
  int height = g_pHeightData->ny;
  int width = g_pHeightData->nx;

  glColor3f(1,1,1);

  int wireframeDensity = 10;
  for (int y = height - wireframeDensity -1; y >= wireframeDensity; y = y-wireframeDensity ) {
      // we want to circle for each element in the next for loop and counter-clockwise add our vertices to the point
      for (int x = wireframeDensity + 1; x < width - wireframeDensity; x = x + wireframeDensity) {

        // draw out the top loops
        glBegin(GL_LINE_LOOP);

          glVertex3f(x,y,PIC_PIXEL(g_pHeightData, y, x, 0));
          glVertex3f(x-wireframeDensity, y, PIC_PIXEL(g_pHeightData, y, x-wireframeDensity, 0));
          glVertex3f(x-wireframeDensity, y+ wireframeDensity, PIC_PIXEL(g_pHeightData, y+wireframeDensity, x-wireframeDensity, 0));  

        glEnd();
      }
    }
}

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


void fillhelper(int x, int y) {
  unsigned char heightVal = PIC_PIXEL(g_pHeightData, y, x, 0);
  
  float delta = max_z + min_z;
  float color = (float)heightVal/delta;
  glColor3f(color, color, color);

  glVertex3f(x,y,heightVal);
}


void drawFill() {
  int height = g_pHeightData->ny;
  int width = g_pHeightData->nx;
  
  float tempColor;
  glPushMatrix();

  for (int y = height - 2; y >= 1; --y) {
      for (int x = 1; x < width - 2; ++x) {
        glBegin(GL_TRIANGLE_STRIP);
          fillhelper(x-1,y);
          fillhelper(x-1,y+1);
          fillhelper(x,y);
          fillhelper(x,y+1);
          fillhelper(x+1,y+1);
          fillhelper(x+1,y);
        glEnd();

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
  glPushMatrix();
  glTranslatef(0,0,-100);
  glScalef(0.1,0.1,0.1);

  glRotatef(g_vLandRotate[0], 1.0, 0.0, 0.0);
  glRotatef(g_vLandRotate[1], 0.0, 1.0, 0.0);
  glRotatef(g_vLandRotate[2], 0.0, 0.0, 1.0);

  glRotatef(90, -1, 0, 0);

  glTranslatef(int(g_pHeightData->nx)*-0.5, 0, int(g_pHeightData->ny) * -0.25);

  glRotatef(30, 1, 0, 0);

  if (dm == FILL) {
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL);
    drawFill();
  }
  else if (dm == LINE) {
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE);
    drawWireframe();
  }
  else if (dm == POINT) {
    glPolygonMode( GL_FRONT_AND_BACK, GL_POINT);
    drawPoints();
  }

  glPopMatrix();
}



void display()
{

  glViewport(0,0, 640, 480);
  
  glMatrixMode(GL_MODELVIEW);

  glLoadIdentity();
  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glTranslatef(g_vLandTranslate[0], g_vLandTranslate[1], g_vLandTranslate[2]);

  glScalef(g_vLandScale[0], g_vLandScale[1], g_vLandScale[2]);


  displaySwitch();
  

  glMatrixMode(GL_PROJECTION);
  
  glLoadIdentity();

  // fov angle, aspect ratio = width / height, near z, far z
  gluPerspective(10.0f, 640.0 / 480.0, 0.01, 1000.0);

  // swap the buffers and bring the second one out from hiding
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

void doIdle()
{
  /* do some stuff... */
  g_vLandRotate[axis] += delta;
  if (g_vLandRotate[axis] > 360.0) {
  	g_vLandRotate[axis] -= 360.0;
  }

  /* make the screen update */
  glutPostRedisplay();
}

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

// void myReshape(int w, int h) {
//   glViewport(0,0, 640, 480);
  
//   glMatrixMode(GL_MODELVIEW);

//   glLoadIdentity();
  
//   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//   glTranslatef(g_vLandTranslate[0], g_vLandTranslate[1], g_vLandTranslate[2]);

//   glScalef(g_vLandScale[0], g_vLandScale[1], g_vLandScale[2]);



//   glMatrixMode(GL_PROJECTION);
//     glLoadIdentity();

//     // fov angle, aspect ratio = width / height, near z, far z
//     gluPerspective(10.0f, 640.0 / 480.0, 0.01, 1000.0);

// }

void keyPress(int key, int x, int y) {
	//trangile--"T"
	if (key == 116 || key == 84) {
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL);
		dm = FILL;

	}
	//point -- "P"
	if (key == 112 || key == 80) {
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE);
		dm = POINT;
	}

	//wirefram -- "W"
	if (key == 119 || key == 87) {
		glPolygonMode( GL_FRONT_AND_BACK, GL_POINT);
		dm = LINE;
	}

	//stop idle function
	if (key == ' ') {
		stopIdle = !stopIdle;
	}
	if (stopIdle) {
		glutIdleFunc(doIdleNULL);
	} else {
		glutIdleFunc(doIdle);
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
  //glutReshapeFunc(myReshape);
  
  /* tells glut to use a particular display function to redraw */
  glutDisplayFunc(display);

  /* replace with any animate code */
  glutIdleFunc(doIdle);

  //allow user to press key to switch display mode
  glutSpecialFunc(keyPress);

  /* callback for mouse drags */
  glutMotionFunc(mousedrag);
  /* callback for idle mouse movement */
  glutPassiveMotionFunc(mouseidle);
  /* callback for mouse button changes */
  glutMouseFunc(mousebutton);



  glutMainLoop();
  return(0);
}
