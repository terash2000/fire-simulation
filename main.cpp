// Modified by Nuttapong Chentanez

// glut_example.c
// Stanford University, CS248, Fall 2000
//
// Demonstrates basic use of GLUT toolkit for CS248 video game assignment.
// More GLUT details at http://reality.sgi.com/mjk_asd/spec3/spec3.html
// Here you'll find examples of initialization, basic viewing transformations,
// mouse and keyboard callbacks, menus, some rendering primitives, lighting,
// double buffering, Z buffering, and texturing.
//
// Matt Ginzton -- magi@cs.stanford.edu

#include "src/ExplosionController.cpp"
#include "src/perlin.cpp"

#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define TEXTURE_ID_DIRT 100
#define BLOCK_SIZE 0.5
#define TEXTURE_SIZE 2.0

#define FIELD_OF_VIEW  65
#define MIN_HEIGHT  1.0
#define MAX_HEIGHT  15.0
#define DEFAULT_TIMESCALE  1.0
#define MIN_TIMESCALE  0.2
#define MAX_TIMESCALE  3.0
#define MIN_VIEWDISTANCE  4.0
#define MAX_VIEWDISTANCE  20.0

#define WIDTH 128
#define HEIGTH 128

using namespace std;

enum {
	MENU_COLOR_ORANGE,
	MENU_COLOR_RED,
	MENU_COLOR_BLUE,
	MENU_COLOR_GREEN,
	MENU_COLOR_PINK,
	MENU_RESTART,
	MENU_EXIT
};

typedef int BOOL;
#define TRUE 1
#define FALSE 0

static BOOL g_bButton1Down = FALSE;
static GLfloat g_fViewDistance = 10;
static GLfloat cam_radian = 0;
static GLfloat cam_old_radian = 0;
static GLfloat cam_angular_velocity = 0;
static GLfloat cam_height = 3;
static GLfloat g_nearPlane = 1;
static GLfloat g_farPlane = 1000;
static int g_Width = 600;                          // Initial window width
static int g_Height = 600;                         // Initial window height
static int g_xClick = 0;
static int g_yClick = 0;
static float g_lightPos[4] = { 0, 1, 0, 1 };  // Position of light
static float timescale = DEFAULT_TIMESCALE;
static float lightColor[4] = { 1.0, 1.0, 0.0, 1.0 }; // default is yellow
static float normalColor[4] = { 1.0, 0.5, 0.0, 1.0 }; // default is orange
static int currentColor = MENU_COLOR_ORANGE;

static ExplosionController explosionController;
static Perlin *perlin = new Perlin(4,4,15,55);
static float* perlinResult;

#ifdef _WIN32
static DWORD last_idle_time;
#else
static struct timeval last_idle_time;
#endif

void DrawFloorBlock(float x, float y, float fSize)
{
	glBegin(GL_QUADS);
	glNormal3f(0,1,0);

	glTexCoord2f (x/TEXTURE_SIZE, y/TEXTURE_SIZE);
	glVertex3f(x, 0, y);

	glTexCoord2f (x/TEXTURE_SIZE, (y + fSize)/TEXTURE_SIZE);
	glVertex3f(x, 0, y + fSize);

	glTexCoord2f ((x + fSize)/TEXTURE_SIZE, (y + fSize)/TEXTURE_SIZE);
	glVertex3f(x + fSize, 0, y + fSize);

	glTexCoord2f ((x + fSize)/TEXTURE_SIZE, y/TEXTURE_SIZE);
	glVertex3f(x + fSize, 0, y);

	glEnd();
}

void DrawFloor(float fSize)
{
    fSize /= 2.0;
    for (float i = -fSize; i < fSize; i += BLOCK_SIZE) {
		for (float j = -fSize; j < fSize; j += BLOCK_SIZE) {
			DrawFloorBlock(i, j, BLOCK_SIZE);
		}
	}
}

void RenderObjects(void)
{
	float colorWhite[4] = { 0.7, 0.7, 0.7, 1.0 };
	float colorGray[4] = { 0.3, 0.3, 0.3, 1.0 };

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	// Main object (floor)
	glEnable(GL_LIGHTING);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, colorWhite);
	glMaterialfv(GL_FRONT, GL_SPECULAR, colorGray);
	glMaterialf(GL_FRONT, GL_SHININESS, 2.0);
	glBindTexture(GL_TEXTURE_2D, TEXTURE_ID_DIRT);
	DrawFloor(20.0);

	// Render flame
	glDisable(GL_LIGHTING);

    explosionController.render(normalColor);

	glPopMatrix();
}

void display(void)
{
	// Clear frame buffer and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set up viewing transformation, looking down -Z axis
	glLoadIdentity();

	// Set up camera
	gluLookAt(g_fViewDistance*sin(-cam_radian), cam_height, g_fViewDistance*cos(-cam_radian),
           0, 0, 0, 0, 1, 0);

	// Set up the stationary light
	glLightfv(GL_LIGHT0, GL_POSITION, g_lightPos);

	// Render the scene
	RenderObjects();

	// Make sure changes appear onscreen
	glutSwapBuffers();
}

void reshape(GLint width, GLint height)
{
	g_Width = width;
	g_Height = height;

	glViewport(0, 0, g_Width, g_Height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(FIELD_OF_VIEW, (float)g_Width / g_Height, g_nearPlane, g_farPlane);
	glMatrixMode(GL_MODELVIEW);
}

float interpolate(float percent, float x1, float x2) {
    return (x2 - x1)*percent + x1;
}

void setupPerlin() {
	perlinResult = new float[WIDTH*HEIGTH];
	float* ptr = perlinResult;
	for (float i = 0; i < HEIGTH; i++) {
		for (float j = 0; j < WIDTH; j++) {
            // t is between 70 and 100
            int t = perlin->Get(i/128, j/128);
            t += 85;
			ptr[0] = t;
			ptr++;
		}
	}
}

void* createFlameTexture(int* width, int* height, int* nComponents, float p) {
	*width = WIDTH;
	*height = HEIGTH;
	*nComponents = 4;
	unsigned char* data = new unsigned char[WIDTH*HEIGTH*4];
	unsigned char* ptr = data;
	int index = 0;
	for (float i = 0; i < HEIGTH; i++) {
		for (float j = 0; j < WIDTH; j++) {
            int t = perlinResult[index];
            index++;
            t -= 0.7 * p;
            if (t > 80) {
                // white to light color
                ptr[0] = interpolate((t - 80)/20.0, lightColor[0]*255, 255);
                ptr[1] = interpolate((t - 80)/20.0, lightColor[1]*255, 255);
                ptr[2] = interpolate((t - 80)/20.0, lightColor[2]*255, 255);
                ptr[3] = 255;
            } else if (t > 75) {
                // light color to normal color
                ptr[0] = interpolate((t - 75)/5.0, normalColor[0]*255, lightColor[0]*255);
                ptr[1] = interpolate((t - 75)/5.0, normalColor[1]*255, lightColor[1]*255);
                ptr[2] = interpolate((t - 75)/5.0, normalColor[2]*255, lightColor[2]*255);
                ptr[3] = 255;
            } else if (t > 70) {
                // normal color to dark color
                ptr[0] = interpolate((t - 70)/5.0, normalColor[0]*255/2, normalColor[0]*255);
                ptr[1] = interpolate((t - 70)/5.0, normalColor[1]*255/2, normalColor[1]*255);
                ptr[2] = interpolate((t - 70)/5.0, normalColor[2]*255/2, normalColor[2]*255);
                ptr[3] = 255;
            } else if (t > 55) {
                // dark color to black
                ptr[0] = interpolate((t - 55)/15.0, 20, normalColor[0]*255/2);
                ptr[1] = interpolate((t - 55)/15.0, 20, normalColor[1]*255/2);
                ptr[2] = interpolate((t - 55)/15.0, 20, normalColor[2]*255/2);
                ptr[3] = 255;
            } else if (t > 40) {
                // black to white
                ptr[0] = interpolate((t - 40)/15.0, 64, 20);
                ptr[1] = interpolate((t - 40)/15.0, 64, 20);
                ptr[2] = interpolate((t - 40)/15.0, 64, 20);
                ptr[3] = 255;
            } else if (t > 25) {
                // white to transparent
                ptr[0] = 64;
                ptr[1] = 64;
                ptr[2] = 64;
                ptr[3] = interpolate((t - 25)/15.0, 0, 255);
            } else  {
                // transparent
                ptr[0] = 0;
                ptr[1] = 0;
                ptr[2] = 0;
                ptr[3] = 0;
            }
			ptr+=4;
		}
	}
	return (void*) data;
}

void setFlameTexture() {
    int width, height, nrChannels;
    void* perlinFlame;
    for (int i = 0; i < 100; i++) {
        perlinFlame = createFlameTexture(&width, &height, &nrChannels, i);
        glBindTexture(GL_TEXTURE_2D, i);
        gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, width, height, GL_RGBA, GL_UNSIGNED_BYTE,	perlinFlame);
        ::operator delete(perlinFlame);
    }
}

void InitGraphics(void)
{
	int width, height, nrChannels;

    glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_LIGHT0);
	glEnable(GL_BLEND);
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glEnable(GL_POINT_SMOOTH);
	glPointSize(4.0);

    unsigned char *dirt = stbi_load("resource/dirt_texture.jpg", &width, &height, &nrChannels, 0);
	glBindTexture(GL_TEXTURE_2D, TEXTURE_ID_DIRT);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, width, height, GL_RGB, GL_UNSIGNED_BYTE,	dirt);

    glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		GL_LINEAR_MIPMAP_LINEAR);
	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    setupPerlin();
    setFlameTexture();
}

void MouseButton(int button, int state, int x, int y)
{
	// Respond to mouse button presses.
	if (button == GLUT_LEFT_BUTTON)
	{
		g_bButton1Down = (state == GLUT_DOWN) ? TRUE : FALSE;
		g_yClick = y - 30.0 * cam_height;
		g_xClick = x - 50.0 * cam_radian;
	}
}

void MouseMotion(int x, int y)
{
	if (g_bButton1Down)
	{

		cam_height = (y - g_yClick) / 30.0;
		if (cam_height < MIN_HEIGHT)
			cam_height = MIN_HEIGHT;
        if (cam_height > MAX_HEIGHT)
			cam_height = MAX_HEIGHT;

        cam_radian = (x - g_xClick) / 50.0;
	}
}

void AnimateScene(void)
{
    float dt;
#ifdef _WIN32
	DWORD time_now;
	time_now = GetTickCount();
	dt = (float) (time_now - last_idle_time) / 1000.0;
#else
	// Figure out time elapsed since last call to idle function
	struct timeval time_now;
	gettimeofday(&time_now, NULL);
	dt = (float)(time_now.tv_sec  - last_idle_time.tv_sec) +
		1.0e-6*(time_now.tv_usec - last_idle_time.tv_usec);
#endif

    if (dt > 0.05) dt = 0.05;
	// Save time_now for next time
	last_idle_time = time_now;

	// Camera's physic
	if (dt) {
        if (g_bButton1Down) {
            cam_angular_velocity = (cam_radian - cam_old_radian) / dt;
            // Save radian for next time
            cam_old_radian = cam_radian;
        } else {
            // Camera move free
            cam_angular_velocity -= 10*cam_angular_velocity*dt;
            cam_radian += cam_angular_velocity*dt;
        }

        // Update Fire
        explosionController.update(dt*timescale);
	}

	// Force redraw
	glutPostRedisplay();
}

void SelectFromMenu(int idCommand)
{
    if (currentColor == idCommand) return;
    currentColor = idCommand;

	switch (idCommand)
	{
	case MENU_COLOR_ORANGE:
	    // yellow
        lightColor[0] = 1.0; lightColor[1] = 1.0; lightColor[2] = 0.0;
        // orange
        normalColor[0] = 1.0; normalColor[1] = 0.5; normalColor[2] = 0.0;
        setFlameTexture();
		break;

	case MENU_COLOR_RED:
        // orange
        lightColor[0] = 1.0; lightColor[1] = 0.5; lightColor[2] = 0.0;
        // red
        normalColor[0] = 1.0; normalColor[1] = 0.0; normalColor[2] = 0.0;
        setFlameTexture();
		break;

    case MENU_COLOR_BLUE:
        // cyan
        lightColor[0] = 0.0; lightColor[1] = 0.8; lightColor[2] = 1.0;
        // blue
        normalColor[0] = 0.0; normalColor[1] = 0.0; normalColor[2] = 1.0;
        setFlameTexture();
		break;

    case MENU_COLOR_GREEN:
        // green
        lightColor[0] = 0.0; lightColor[1] = 1; lightColor[2] = 0;
        // dark green
        normalColor[0] = 0.0; normalColor[1] = 0.5; normalColor[2] = 0;
        setFlameTexture();
		break;

    case MENU_COLOR_PINK:
        // pink
        lightColor[0] = 1.0; lightColor[1] = 0.5; lightColor[2] = 1.0;
        // purple
        normalColor[0] = 0.8; normalColor[1] = 0.0; normalColor[2] = 0.8;
        setFlameTexture();
		break;

    case MENU_RESTART:
		timescale = DEFAULT_TIMESCALE;
		explosionController.restart();
		break;

	case MENU_EXIT:
		exit (0);
		break;
	}

	glutPostRedisplay();
}

void Keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:             // ESCAPE key
		exit (0);
		break;

    case 'r':
		SelectFromMenu(MENU_RESTART);
		break;
	case '1':
		SelectFromMenu(MENU_COLOR_ORANGE);
		break;
    case '2':
		SelectFromMenu(MENU_COLOR_RED);
		break;
    case '3':
		SelectFromMenu(MENU_COLOR_BLUE);
		break;
    case '4':
		SelectFromMenu(MENU_COLOR_GREEN);
		break;
    case '5':
		SelectFromMenu(MENU_COLOR_PINK);
		break;
    // Change timescale
    case '7':
		timescale -= 0.1;
		if (timescale < MIN_TIMESCALE)
            timescale = MIN_TIMESCALE;
		break;
    case '8':
		timescale += 0.1;
		if (timescale > MAX_TIMESCALE)
            timescale = MAX_TIMESCALE;
		break;
	// Change View Distance
    case '9':
		g_fViewDistance -= 0.1;
		if (g_fViewDistance < MIN_VIEWDISTANCE)
            g_fViewDistance = MIN_VIEWDISTANCE;
		break;
    case '0':
		g_fViewDistance += 0.1;
		if (g_fViewDistance > MAX_VIEWDISTANCE)
            g_fViewDistance = MAX_VIEWDISTANCE;
		break;
	}
}

int BuildPopupMenu (void)
{
	int menu;

	menu = glutCreateMenu (SelectFromMenu);
	glutAddMenuEntry ("Change Color to orange\t1", MENU_COLOR_ORANGE);
	glutAddMenuEntry ("Change Color to red\t2", MENU_COLOR_RED);
	glutAddMenuEntry ("Change Color to blue\t3", MENU_COLOR_BLUE);
	glutAddMenuEntry ("Change Color to green\t4", MENU_COLOR_GREEN);
	glutAddMenuEntry ("Change Color to pink\t5", MENU_COLOR_PINK);
	glutAddMenuEntry ("Restart\tr", MENU_RESTART);
	glutAddMenuEntry ("Exit demo\tEsc", MENU_EXIT);

	return menu;
}

int main(int argc, char** argv)
{
	// GLUT Window Initialization:
	glutInit (&argc, argv);
	glutInitWindowSize (g_Width, g_Height);
	glutInitDisplayMode ( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow ("fire simulation");

	// Initialize OpenGL graphics state
	InitGraphics();

	// Register callbacks:
	glutDisplayFunc (display);
	glutReshapeFunc (reshape);
	glutKeyboardFunc (Keyboard);
	glutMouseFunc (MouseButton);
	glutMotionFunc (MouseMotion);
	glutIdleFunc (AnimateScene);

	// Create our popup menu
	BuildPopupMenu ();
	glutAttachMenu (GLUT_RIGHT_BUTTON);

	// Get the initial time, for use by animation
#ifdef _WIN32
	last_idle_time = GetTickCount();
#else
	gettimeofday (&last_idle_time, NULL);
#endif

	// Turn the flow of control over to GLUT
	glutMainLoop ();
	return 0;
}



