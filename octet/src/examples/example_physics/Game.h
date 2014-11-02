#pragma once
#include "Player.h"

namespace octet{

   enum class GameState { IDLE, PLAY, END };

   class Game : public resource
   {

   private:

      GameState gs;
      int winneridx;
      ref<mesh_text> winnerText;
      string winnerString;

      Game(){}

   public:

      void SetWinner(int widx){
         winneridx = widx;
      }

      int GetWinner(int win){
         return winneridx;
      }

      mesh_text* GetWinnerText(){
         return winnerText;
      }

      void SetState(GameState state){
         gs = state;
      }

      GameState GetState(){
         return gs;
      }
      
      void CheckGameStatus(dynarray<Player*> players){
         
         int num_winners = 0;
         int winnerIdx = -1;

         for (unsigned i = 0; i < players.size(); i++){

            if (players[i]->GetState() != PlayerState::Dead){
               winnerIdx = i;
               num_winners++;
            }
         }

         if (num_winners == 1){
            this->winneridx = winnerIdx;
            winnerString.format("PLAYER %s WINS, PRESS SPACE FOR ANOTHER GAME\n", players[winnerIdx]->GetColorString());
            gs = GameState::END;
         }
         //else if (num_winners == 0){
         //   winnerString.format("DRAW!! PRESS SPACE FOR ANOTHER GAME\n");
         //   this->winneridx = -1; //Chuck: means that there is a draw
         //   gs = GameState::END;
         //}
      }
       
      void RenderText(){

         if (gs == GameState::END)
         {
            winnerText->clear();
            winnerText->format("%s", winnerString);
            winnerText->update();
         }
         else{
            winnerText->clear();
            winnerText->update();
         }
      }

      void Reset(){
         winnerText->clear();
         winnerText->update();
         gs = GameState::PLAY;
      }

      Game(GameState state, bitmap_font* font){
         gs = state;
         aabb bb(vec3(0), vec3(200, 50, 0));
         winnerString = "";
         winnerText = new mesh_text(font, "", &bb);
         winnerText->clear();
         winnerText->update();
      }

      ~Game(){}

   };
}

