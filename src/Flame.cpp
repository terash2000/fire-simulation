//HACK TO FORCE COMPILE AS WIN32
#define _WIN32

#ifdef _WIN32
#	include <windows.h>
#else
#	include <sys/time.h>
#endif

#include <GL/glut.h>

class Flame
{
    public:
        inline static const int STATE_IDLE = 0;
        inline static const int STATE_FLOATING = 1;

        int currentState;

        Flame(){
            sphere = gluNewQuadric();
            gluQuadricTexture(sphere, TRUE);
            gluQuadricNormals(sphere, GLU_SMOOTH);
            reset();
        }

        void update(float deltaTime, float flyX, float flyZ){
            if (currentState == STATE_IDLE) return;
            time += deltaTime;
            updateState();
            switch (currentState) {
            case STATE_FLOATING:
                dx += flyX * deltaTime;
                dz += flyZ * deltaTime;
                x += dx * deltaTime;
                z += dz * deltaTime;
                y += floatSpeed * deltaTime;
                sphereSize += inflateSpeed * deltaTime;
                return;
            default:
                return;
            }
        }

        void render(){
            switch (currentState) {
            case STATE_FLOATING:
                glBindTexture(GL_TEXTURE_2D, time*100/FLOATING_INTERVAL);
                break;
            default:
                return;
            }

            glPushMatrix();
            glTranslated(x, y, z);
            glRotatef(rotation, 0, 1, 0);

            gluSphere(sphere, sphereSize, 15, 8);

            glPopMatrix();
        }

        void reset(){
            x = 0.0;
            y = -0.5;
            z = 0.0;
            dx = 0.0;
            dz = 0.0;
            rotation = (((float) rand()) / (float) RAND_MAX) * 360;
            sphereSize = defaultSize;
            time = 0.0;
            currentState = STATE_FLOATING;
        }

    private:
        inline static float FLOATING_INTERVAL = 2.5;

        GLUquadricObj *sphere;
        GLfloat x, y, z, dx, dz, rotation, sphereSize;
        GLfloat floatSpeed = 3.0;
        GLfloat defaultSize = 0.4;
        GLfloat inflateSpeed = 0.2;
        float time;

        void updateState(){
            if (time < FLOATING_INTERVAL) {
                currentState = STATE_FLOATING;
            } else {
                currentState = STATE_IDLE;
            }
        }
};
