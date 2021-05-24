#define _WIN32

#ifdef _WIN32
#	include <windows.h>
#else
#	include <sys/time.h>
#endif

#include <GL/glut.h>

#include <vector>
#include <random>
#include <math.h>

class FlareParticle
{
    public:
        flareParticle();

        void update(float deltaTime){
            for (int i = 0; i < particlesNumber; i++) {
                updateParticle(i, deltaTime);
            }

            spawnTime += deltaTime;
            while(spawnTime > spawnInterval) {
                spawnTime -= spawnInterval;
                spawnParticle(spawnTime);
            }
        }

        void render(float color[4]){
            float colorWhite[4] = { 1.0, 1.0, 1.0, 1.0 };
            glColor3fv(color);
            glBindTexture(GL_TEXTURE_2D, -1);
            glBegin( GL_POINTS);
                for (int i = 0; i < particlesNumber; i++) {
                    if (particleTime[i] < particleInterval)
                        glVertex3f(positions[3*i], positions[3*i + 1], positions[3*i + 2]);
                }
            glEnd();
            glColor3fv(colorWhite);
        }

        void reset(){
            positions.clear();
            moveDest.clear();
            particleTime.clear();
            particlesNumber = 0;
        }

    private:
        inline static float spawnInterval = 0.3;
        inline static float particleInterval = 3.0;
        inline static float floatSpeed = 2.5;

        inline static std::default_random_engine generator;
        inline static float spawnTime;
        inline static int particlesNumber;
        inline static std::vector<float> positions;
        inline static std::vector<float> moveDest;
        inline static std::vector<float> particleTime;

        void spawnParticle(float time){
            float randomX = (((float)rand() / (RAND_MAX)) - 0.5) * 7;
            float randomZ = (((float)rand() / (RAND_MAX)) - 0.5) * 7;

            for (int i = 0; i < particlesNumber; i++) {
                if (particleTime[i] >= particleInterval) {
                    positions[3*i] = 0.0;
                    positions[3*i + 1] = 0.0;
                    positions[3*i + 2] = 0.0;

                    moveDest[3*i] = randomX;
                    moveDest[3*i + 1] = 2.0;
                    moveDest[3*i + 2] = randomZ;

                    particleTime[i] = 0.0;
                    updateParticle(i, time);
                    return;
                }
            }
            // create new flare
            positions.insert(positions.end(), { 0.0, 0.0, 0.0 });
            moveDest.insert(moveDest.end(), { randomX, floatSpeed, randomZ });
            particleTime.push_back(0.0);
            updateParticle(particlesNumber, time);
            particlesNumber++;
        }

        void updateParticle(int i, float deltaTime){
            particleTime[i] += deltaTime;
            if (particleTime[i] < particleInterval) {
                std::normal_distribution<float> distribution(0.0, 1.0);

                moveDest[3*i] += (distribution(generator)/sqrt(deltaTime) - 2*moveDest[3*i]) * deltaTime;
                moveDest[3*i + 2] += (distribution(generator)/sqrt(deltaTime) - 2*moveDest[3*i + 2]) * deltaTime;

                positions[3*i] += moveDest[3*i] * deltaTime;
                positions[3*i + 1] += moveDest[3*i + 1] * deltaTime;
                positions[3*i + 2] += moveDest[3*i + 2] * deltaTime;
            }
        }
};
