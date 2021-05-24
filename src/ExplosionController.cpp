#include "Flame.cpp"
#include "FlareParticle.cpp"
#include <vector>
#include <algorithm>

class ExplosionController{
    public:
        ExplosionController(){

        }

        static void update(float deltaTime){
            std::normal_distribution<float> distribution(0.0, 0.5);

            for (auto flame: flames) {
                flame->update(deltaTime, distribution(generator)/sqrt(deltaTime),
                              distribution(generator)/sqrt(deltaTime));
            }

            spawnTime += deltaTime;
            while(spawnTime > spawnInterval) {
                spawnTime -= spawnInterval;
                spawnNewFlame(spawnTime);
            }

            flareParticle.update(deltaTime);
        }

        static void render(float color[4]){
            for (auto flame: flames) {
                flame->render();
            }

            flareParticle.render(color);
        }

        static void restart(){
            for (auto flame: flames) {
                flame->currentState = flame->STATE_IDLE;
            }

            flareParticle.reset();
        }

    private:
        inline static float spawnInterval = 0.05;

        inline static FlareParticle flareParticle;
        inline static float spawnTime;
        inline static std::vector<Flame*> flames;
        inline static std::default_random_engine generator;

        static void spawnNewFlame(float time){
            std::normal_distribution<float> distribution(0.0, 0.5);

            for (int i = flames.size()-1; i >= 0; i--) {
                if (flames[i]->currentState == flames[i]->STATE_IDLE) {
                    flames[i]->reset();
                    flames[i]->update(time, distribution(generator)/sqrt(time),
                                      distribution(generator)/sqrt(time));
                    return;
                }
            }
            // create new flame
            flames.insert(flames.begin(), new Flame());
            flames.back()->update(time, distribution(generator), distribution(generator));
        }
};
