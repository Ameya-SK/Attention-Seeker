/* Using the accumulation buffer for image blur. */
#include <stdio.h>
#include <Windows.h>
#include <glut.h>
#include <stdlib.h>

#include <conio.h>
#include "thinkgear.h"

int p=0.0, p1=0.0, errCode = 0, connectionId = 0;
char *comPortName = NULL;
const GLdouble FRUSTDIM = 100.f;
const GLdouble FRUSTNEAR = 320.f;
const GLdouble FRUSTFAR = 660.f;

int maxr = 25;
enum { SPHERE = 1 };
void senser(int);
void render(GLfloat dx, GLfloat dy, GLfloat dz)
{
	/* material properties for objects in scene */
	static GLfloat wall_mat[] = { 1.f, 1.f, 1.f, 1.f };

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	glPushMatrix();
	glTranslatef(-10.f + dx, -10.f + dy, -420.f + dz);
	glCallList(SPHERE);
	glPopMatrix();

	if (glGetError()) /* to catch programming errors; should never happen */
		printf("Oops! I screwed up my OpenGL calls somewhere\n");

	glFlush(); /* high end machines may need this */
}

enum { NONE, FIELD };
int rendermode = 0;
GLdouble focus = 420.;

/* Called when window needs to be redrawn */

void redraw(void)
{
	int i;
	GLfloat dx, dy, dz;

	dx = .5f;
	dy = .5f;
	dz = -.5f;

	glPushMatrix();
	switch (rendermode) {
		case 0: // unbluring
			maxr -= 1;
			printf("Inside key\n");
			printf("attention=%d\n",p);
			glClear(GL_ACCUM_BUFFER_BIT);
			if (maxr > 1)
			{
				for (i = 0; i < maxr - 1; i++) {
					render(dx * i, dy * i, dz * i);
					glAccum(GL_ACCUM, 1.f / maxr);
				}
				glAccum(GL_RETURN, 1.f);
			}
			else
			{
				render(0.f, 0.f, 0.f);
				maxr = 1;
			}

			glutTimerFunc(1, senser, 1);
			break;
		case 1: // bluring
			maxr += 1;
			printf("Inside key\n");
			printf("attention=%d\n",p);
			glClear(GL_ACCUM_BUFFER_BIT);
			for (i = 0; i < maxr; i++) {
				render(dx * i, dy * i, dz * i);
				glAccum(GL_ACCUM, 1.f / maxr);
			}
			glAccum(GL_RETURN, 1.f);
			glutTimerFunc(1, senser, 1);
			break;
	}

	glPopMatrix();
	glutSwapBuffers();
}

void redraw2(int)
{
	redraw();
}





/* ARGSUSED1 */
void key(int x)
{ 
	if (x == 0){
		
		rendermode = 1;
		glutPostRedisplay();
	}
	else{
		rendermode = 0;
		glutPostRedisplay();
	}
}

void senser(int)
{int ab;
for(int i=0;i<9999;i++)
{
	errCode = TG_ReadPackets(connectionId, 1);
	//printf("errorcode=%d\n",errCode);
	if (errCode != 1)
	{
		
		ab=TG_GetValueStatus(connectionId, TG_DATA_ATTENTION);
		//printf("value status=%d\n",ab);
		if (TG_GetValueStatus(connectionId, TG_DATA_ATTENTION) == 0)
		{
			
			p = TG_GetValue(connectionId, TG_DATA_ATTENTION);
			printf("Attention=%d\n",p);
			if (p >= 90)
			{
				printf("attention=%d\n",p);
				
				//key(1);
				rendermode=0;
				//glutPostRedisplay();
			}
			else
			{printf("attention=%d\n",p);
			
				//key(0);
			rendermode=1;
			//glutPostRedisplay();
			}
		}
	}
	glutTimerFunc(1000, redraw2, 1);
}
}


void wait() 
          {
                printf( "\n" ); 
                printf( "Press the ENTER key...\n" ); 
				fflush( stdout ); 
				getc( stdin ); 
		  }




const int TEXDIM = 256;
/* Parse arguments, and set up interface between OpenGL and window system */
int main(int argc, char *argv[])
{
	static GLfloat lightpos[] = { 50.f, 50.f, -320.f, 1.f };
	static GLfloat sphere_mat[] = { 1.f, .5f, 0.f, 1.f };
	GLUquadricObj *sphere;

	printf("Enter B (b or B) to Blure the image and to normal view N (n or N). \n To go ahead press enter.");
	_getche();
	glutInit(&argc, argv);
	glutInitWindowSize(512, 512);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_ACCUM | GLUT_DOUBLE);
	glutCreateWindow("image blur");
	glutDisplayFunc(redraw);
	

	connectionId = TG_GetNewConnectionId();
	if (connectionId < 0)
	{
		fprintf(stderr, "ERROR: TG_GetNewConnectionId() returned %d.\n", connectionId);
		wait();
		exit(EXIT_FAILURE);
	}
	/* Set/open stream (raw bytes) log file for connection */
	errCode = TG_SetStreamLog(connectionId, "stream2.txt");
	if (errCode < 0)
	{
		fprintf(stderr, "ERROR: TG_SetStreamLog() returned %d.\n", errCode);
		wait();
		exit(EXIT_FAILURE);
	}
	/* Set/open data (ThinkGear values) log file for connection */
	errCode = TG_SetDataLog(connectionId, "data2.txt");
	if (errCode < 0)
	{
		fprintf(stderr, "ERROR: TG_SetDataLog() returned %d.\n", errCode);
		wait();
		exit(EXIT_FAILURE);
	}
	/* Attempt to connect the connection ID handle to serial port "COM5" */
	comPortName = "\\\\.\\COM15";
	errCode = TG_Connect(connectionId, comPortName, TG_BAUD_9600, TG_STREAM_PACKETS);
	if (errCode < 0)
	{
		fprintf(stderr, "ERROR: TG_Connect() returned %d.\n", errCode);
		wait();
		exit(EXIT_FAILURE);
	}

	glutTimerFunc(1000, senser, 1);



	/* draw a perspective scene */
	glMatrixMode(GL_PROJECTION);
	glFrustum(-FRUSTDIM, FRUSTDIM, -FRUSTDIM, FRUSTDIM, FRUSTNEAR, FRUSTFAR);
	glMatrixMode(GL_MODELVIEW);

	/* turn on features */
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	/* place light 0 in the right place */
	glLightfv(GL_LIGHT0, GL_POSITION, lightpos);

	glNewList(SPHERE, GL_COMPILE);
	sphere = gluNewQuadric();
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, sphere_mat);
	gluSphere(sphere, 40.f, 20, 20);
	gluDeleteQuadric(sphere);
	glEndList();

	glutMainLoop();
	return 0;             /* ANSI C requires main to return int. */
}
