


#include <stdio.h>

 /* Include the GLUT library. This file (glut.h) contains gl.h and glu.h */
#include <GL/glut.h>

/* This is a debug file */
FILE* infofile;

void drawTriangle() {
	glBegin(GL_TRIANGLES);
	glColor3f(1.0, 0.0, 0.0);
	glVertex2f(-0.2f, -0.2f);
	glColor3f(0.0, 1.0, 0.0);
	glVertex2f(0.2f, -0.2f);
	glColor3f(0.0, 0.0, 1.0);
	glVertex2f(0.0f, 0.2f);
	glEnd();
}
void drawPoly() {
	glBegin(GL_POLYGON);
	glVertex2f(-0.5, -0.5);
	glVertex2f(0.5, -0.5);
	glVertex2f(0.5, 0.5);
	glVertex2f(-0.5, 0.5);
	glEnd();
}
float rotationAngle = 0.0f;
float scale = 1.0f;
float scaleDelta = 0.01f;


void timer(int value) {
	rotationAngle += 1.0f;
	if (rotationAngle > 360.0f) {
		rotationAngle -= 360.0f;
	}

	scale += scaleDelta;
	if (scale >= 1.5f || scale <= 0.5f) {
		scaleDelta = -scaleDelta;
	}

	glutPostRedisplay();
	glutTimerFunc(16, timer, 0);
}



void display()
{
	fprintf(infofile, "display: Entered \n");
	fflush(infofile);

	/* clear the screen*/
	glClear(GL_COLOR_BUFFER_BIT);

	glPushMatrix();
	glScalef(scale, scale, 1.0f);
	glRotatef(rotationAngle, 0.0f, 0.0f, 1.0f);

	/* draw a filled polygon */
	glColor3f(1.0, 1.0, 0.0);
	drawPoly();

	glBegin(GL_POLYGON);
	glColor3f(0.0, 1.0, 0.0);
	glVertex2f(-0.3, -0.3);
	glVertex2f(0.3, -0.3);
	glVertex2f(0.3, 0.3);
	glVertex2f(-0.3, 0.3);
	glTranslated(-0.75, -0.75, 0.0);
	glEnd();

	drawTriangle();



	/* draw a fancy line -- red to blue variation in color */
	/* variation is created with linear interpolation */
	glBegin(GL_LINES);
	glColor3f(1.0, 0.0, 0.0);
	glVertex2f(-0.75, -0.75);
	glColor3f(1.0, 0.0, 0.0);
	glVertex2f(0.75, -0.75);
	glColor3f(0.0, 0.0, 1.0);
	glVertex2f(-0.75, 0.75);
	glColor3f(1.0, 0.0, 0.0);
	glVertex2f(0.75, 0.75);
	glColor3f(0.0, 1.0, 0.0);
	glVertex2f(-0.75, 0.75);
	glColor3f(1.0, 0.0, 0.0);
	glVertex2f(-0.75, -0.75);
	glColor3f(0.0, 0.0, 1.0);
	glVertex2f(0.75, 0.75);
	glColor3f(0.0, 1.0, 0.0);
	glVertex2f(0.75, -0.75);
	glEnd();

	glPopMatrix();

	/* ready to draw now! forces buffered OGL commands to execute */
	glFlush();

	fprintf(infofile, "display: Exit \n");
	fflush(infofile);
}

/* Initialize states -- called before */

void init()
{
	fprintf(infofile, "init: Entered \n");
	fflush(infofile);


	/* set background clear color to black */
	glClearColor(0.0, 0.0, 0.0, 0.0);
	/* set current color to white */
	glColor3f(1.0, 1.0, 1.0);

	/* identify the projection matrix that we would like to alter */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	/* the window will correspond to these world coorinates */
	gluOrtho2D(-1.0, 1.0, -1.0, 1.0);

	/* we can play ... see what happens if we swap out this one with that above */
	//gluOrtho2D(-10.0, 10.0, -10.0, 10.0);

	/* identify the modeling and viewing matrix that can be modified from here on */
	/* we leave the routine in this mode in case we want to move the object around */
	/* or specify the camera */
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	fprintf(infofile, "init: Exit \n");
	fflush(infofile);
}

/* The main program */

int main(int argc, char** argv)
{
	/* Always a good idea to create a file to help you understand the program flow */
	/* Graphics programs can crash, so fflush is a good idea -- even if it slows things */
	if (fopen_s(&infofile, "infofile.txt", "w") != 0) {
		perror("Error opening infofile.txt");
		return 1; // Exit the program if the file can't be opened
	}
	fprintf(infofile, "basics.c -- a basic OpenGL program\n");
	fflush(infofile);

	/* create a window, position it, and name it */
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
	glutInitWindowSize(450, 450);
	glutInitWindowPosition(500, 100);
	glutCreateWindow("basics");

	fprintf(infofile, "Created windows\n");
	fflush(infofile);

	/* create a callback routine for (re-)display */
	glutDisplayFunc(display);

	/* init some states */
	init();
	glutTimerFunc(0, timer, 0);  // Start the timer function


	/* entering the event loop */
	glutMainLoop();
	fclose(infofile);

	return 0;

	/* code here will not be executed */
}
