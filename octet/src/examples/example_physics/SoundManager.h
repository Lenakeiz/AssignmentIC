#pragma once

namespace octet{

   enum { NUM_SOURCES = 2 };

   class SoundManager : public resource
   {

      dictionary<ALuint> sounds;
      int cur_source;
      ALuint sources[8];
      public:

      SoundManager()
      {
         sounds["Dash"] = resource_dict::get_sound_handle(AL_FORMAT_MONO16, "assets/Dash.wav");
         sounds["Metal"] = resource_dict::get_sound_handle(AL_FORMAT_MONO16, "assets/Metal.wav");
         cur_source = 0;
         alGenSources(NUM_SOURCES, sources);
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

}