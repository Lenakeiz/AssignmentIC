#pragma once

namespace octet{

   enum { NUM_SOURCES = 1 };

   class SoundManager
   {

      private:
      
         static SoundManager* instance;
         dictionary<ALuint> sounds;
         int cur_source;
         ALuint sources[NUM_SOURCES];

         SoundManager()
         {
            sounds["Dash"] = resource_dict::get_sound_handle(AL_FORMAT_MONO16, "assets/Dash.wav");
            sounds["Metal"] = resource_dict::get_sound_handle(AL_FORMAT_MONO16, "assets/Metal.wav");
            sounds["Collision"] = resource_dict::get_sound_handle(AL_FORMAT_MONO16, "assets/Collision.wav");
            cur_source = 0;
            alGenSources(NUM_SOURCES, sources);
         }

         SoundManager(SoundManager const&){};
         SoundManager& operator=(SoundManager const&){};

      public:
         
         static SoundManager* GetInstance(){
            if (!instance){
               instance = new SoundManager();
            }
            return instance;
         }

         ~SoundManager()
         {
         }

         void StartSound(string name){
         
            ALuint source = sources[cur_source++ % NUM_SOURCES];
            alSourcei(source, AL_BUFFER, sounds[name]);
            alSourcePlay(source);

         }

   };
   //Global Static
   SoundManager* SoundManager::instance = nullptr;
}