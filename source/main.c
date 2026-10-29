/*

  reminders:
    when adding/subtracting to level points, check limits (0 <= lp <= 9999)
    make sure endless_highscore doesn't go over int limit (32bit for 3ds)

*/

#include <3ds.h>
#include <citro2d.h>
#include <citro3d.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "3ds/os.h"
#include "3ds/services/apt.h"
#include "3ds/services/hid.h"
#include "c2d/base.h"
#include "c2d/font.h"
#include "c2d/sprite.h"
#include "c2d/spritesheet.h"
#include "c2d/text.h"
#include "c3d/renderqueue.h"
#include "cJSON.h"


#define TOP_SCREEN_WIDTH 400
#define TOP_SCREEN_HEIGHT 240

#define BOTTOM_SCREEN_WIDTH 320
#define BOTTOM_SCREEN_HEIGHT 240

#define GAME_TITLE "Evangelion: Angel Attack"

#define ABILITY_COUNT 10


C2D_Font MAIN_FONT;


typedef struct GameModeOutput {
  int game_mode;
  bool valid;
} GameModeOutput;

typedef struct Colours {
  u32 white;
  u32 black;
  u32 red;
  u32 green;
  u32 blue;
  u32 yellow;
  u32 background;
} Colours;

typedef struct Sprite {
  C2D_SpriteSheet sprite_sheet;
  C2D_Sprite sprite;
  bool valid;
} Sprite;

typedef struct SaveFile {
  bool valid;
  struct {
    char* current_pilot;
    int angels_beat;
    bool main_game_beat;
    int level_points;
    int endless_highscore;
  } main;
  struct {
    struct {
      bool mastery_unlocked;
      int ability_slots;
      bool unlocked;
      char* current_skin;
      struct {
        int max_health;
        int attack;
        int at_field;
        int default_sync_rate;
        int bond;
      } stats;
      char* active_abilities[5];
      struct {
        bool divine_intervention;
        bool miracle_sync;
        bool celestial_gift;
        bool ruthless;
        bool counter;
        bool healing_mist;
        bool absolute_accuracy;
        bool overclock;
        bool double_or_nothing;
        bool eek_run;
      } unlocked_abilities;
      struct {
        bool default_skin;
        bool master;
      } unlocked_skins;
      struct {
        int angels_beat_hard;
        int ability_slots;
        bool max_stats;
        bool beat_1_helpless;
      } mastery;
    } shinji;

    struct {
      bool mastery_unlocked;
      int ability_slots;
      bool unlocked;
      char* current_skin;
      struct {
        int max_health;
        int attack;
        int at_field;
        int default_sync_rate;
        int bond;
      } stats;
      char* active_abilities[5];
      struct {
        bool divine_intervention;
        bool miracle_sync;
        bool celestial_gift;
        bool ruthless;
        bool counter;
        bool healing_mist;
        bool absolute_accuracy;
        bool overclock;
        bool double_or_nothing;
        bool eek_run;
      } unlocked_abilities;
      struct {
        bool default_skin;
        bool master;
      } unlocked_skins;
      struct {
        int angels_beat_hard;
        int ability_slots;
        bool max_stats;
        bool beat_1_helpless;
      } mastery;
    } asuka;

    struct {
      bool mastery_unlocked;
      int ability_slots;
      bool unlocked;
      char* current_skin;
      struct {
        int max_health;
        int attack;
        int at_field;
        int default_sync_rate;
        int bond;
      } stats;
      char* active_abilities[5];
      struct {
        bool divine_intervention;
        bool miracle_sync;
        bool celestial_gift;
        bool ruthless;
        bool counter;
        bool healing_mist;
        bool absolute_accuracy;
        bool overclock;
        bool double_or_nothing;
        bool eek_run;
      } unlocked_abilities;
      struct {
        bool default_skin;
        bool master;
      } unlocked_skins;
      struct {
        int angels_beat_hard;
        int ability_slots;
        bool max_stats;
        bool beat_1_helpless;
      } mastery;
    } rei;
  } pilots;
} SaveFile;

typedef struct Text {
  C2D_Text text;
  C2D_TextBuf buffer;
} Text;

typedef struct AbilitiesMenuOutput {
  int current_button;
  int current_button_data;
  int return_code;
} AbilitiesMenuOutput;

enum MenuOptions {
  MENU_NEW,
  MENU_CONTINUE,
  MENU_EXIT,
  MENU_ERROR
};

enum SelectedOption {
    S_PLAY,
    S_PILOT,
    S_OPTIONS,
    S_EXIT
  };

enum SettingsMenu {
  SM_BACKUP_DATA,
  SM_CREDITS,
  SM_QUIT
};

enum PilotSelection {
    PS_NAME,
    PS_STATS,
    PS_ABILITIES,
    PS_MASTERY,
    PS_SKINS,
    PS_ERROR
  };

enum AbilitySlots {
    ABILITY_LOCKED,
    ABILITY_UNLOCKED,
    ABSOLUTE_ACCURACY,
    CELESTIAL_GIFT,
    COUNTER,
    DIVINE_INTERVENTION,
    DOUBLE_OR_NOTHING,
    EEK_RUN,
    HEALING_MIST,
    MIRACLE_SYNC,
    OVERCLOCK,
    RUTHLESS
  };

enum Buttons {
    SLOT1,
    SLOT2,
    SLOT3,
    SLOT4,
    ABILITIES
};


void system_init(void);
bool audio_init(void);
Colours colours_init(void);
void system_exit(void);
Sprite load_sprite(char* path);
bool title_screen(C3D_RenderTarget* top_screen, C3D_RenderTarget* bottom_screen, Colours colour_palette);
void log_error(char* error_message);
int selection_menu(C3D_RenderTarget* top_screen, C3D_RenderTarget* bottom_screen, Colours colour_palette);
SaveFile load_save_file(void);
bool write_to_save_file(SaveFile save_data);
bool initialize_save_file(void);
bool select_character(C3D_RenderTarget* top_screen, C3D_RenderTarget* bottom_screen, Colours colour_palette, SaveFile* save_data);
Text load_text(char* message);
int main_menu(C3D_RenderTarget* top_screen, C3D_RenderTarget* bottom_screen, Colours colour_palette, SaveFile save_data);
int options_menu(C3D_RenderTarget* top_screen, C3D_RenderTarget* bottom_screen, Colours colour_palette);
bool credits(C3D_RenderTarget* top_screen, C3D_RenderTarget* bottom_screen, Colours colour_palette);
void close_game(SaveFile save_data);
bool create_backup(C3D_RenderTarget* top_screen, Colours colour_palette);
int pilot_menu(C3D_RenderTarget* top_screen, C3D_RenderTarget* bottom_screen, Colours colour_palette, SaveFile* save_data);
AbilitiesMenuOutput abilities_menu_main(C3D_RenderTarget* top_screen, C3D_RenderTarget* bottom_screen, Colours colour_palette, SaveFile* save_data);
bool ability_shop(SaveFile* save_data, C3D_RenderTarget* top_screen, C3D_RenderTarget* bottom_screen, Colours colour_palette, int input_button);
bool skins_menu(SaveFile* save_data, C3D_RenderTarget* top_screen, C3D_RenderTarget* bottom_screen, Colours colour_palette);
bool check_mastery(SaveFile* save_data, char* pilot);
bool mastery_menu(SaveFile* save_data, C3D_RenderTarget* top_screen, C3D_RenderTarget* bottom_screen, Colours colour_palette);
bool unlock_cutscene(char* pilot, Colours colour_palette, C3D_RenderTarget* top_screen, C3D_RenderTarget* bottom_screen);
bool stats_menu(SaveFile* save_data, C3D_RenderTarget* top_screen, C3D_RenderTarget* bottom_screen, Colours colour_palette);
GameModeOutput game_mode_menu(C3D_RenderTarget* top_screen, C3D_RenderTarget* bottom_screen, Colours colour_palette);
int wrap(int index, int range);


int main(void) {
  system_init();
  if (!audio_init()) {
    log_error("Failed to initialize audio");
    system_exit();
    return 1;
  }

  log_error("Game Started");

  C3D_RenderTarget* top_screen = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
  C3D_RenderTarget* bottom_screen = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
  
  Colours colour_palette = colours_init();

  if (!title_screen(top_screen, bottom_screen, colour_palette)) {
    log_error("ERROR: Failed to load title screen");
    system_exit();
    return 1;
  }

  int continuation = selection_menu(top_screen, bottom_screen, colour_palette);
  if (continuation == MENU_ERROR) {
    log_error("ERROR: Failed to load selection menu");
    system_exit();
    return 1;
  } else if (continuation == MENU_EXIT) {
    system_exit();
    return 0;
  }
  if (continuation == MENU_NEW) { initialize_save_file(); }

  SaveFile save_data = load_save_file();
  if (!save_data.valid) {
    log_error("ERROR: Failed to load save data");
    system_exit();
    return 1;
  }

  if (continuation == MENU_NEW) {
    int select_character_output = select_character(top_screen, bottom_screen, colour_palette, &save_data);
    if (!select_character_output) {
      log_error("ERROR: Failed to select character or user quit");
      system_exit();
      return 1;
    } else {
      write_to_save_file(save_data);
    }
  }

  bool in_main_menu = true;
  while (in_main_menu) {
    int main_menu_continuation = main_menu(top_screen, bottom_screen, colour_palette, save_data);
    if (main_menu_continuation == -1) {
      log_error("ERROR: Failed to process main menu");
      close_game(save_data);
      return 1;
    }
  
    switch (main_menu_continuation) {
      case S_EXIT:
        close_game(save_data);
        return 0;
        break;
      case S_OPTIONS:
        bool in_options_menu = true;
        while (in_options_menu) {
          int options_continuation = options_menu(top_screen, bottom_screen, colour_palette);
          if (options_continuation == -1) {
            log_error("ERROR: Failed to process options menu");
            close_game(save_data);
            return 1;
          }
          switch (options_continuation) {
            case SM_BACKUP_DATA:
              if (!create_backup(top_screen, colour_palette)) {
                log_error("ERROR: Failed to backup data");
                close_game(save_data);
                return 1;
              }
              break;
            case SM_CREDITS:
              if (!credits(top_screen, bottom_screen, colour_palette)) {
                log_error("ERROR: Failed to process credits menu");
                close_game(save_data);
                return 1;
              }
              break;
            case SM_QUIT:
              in_options_menu = false;
              break;
          }
        }
        break;
      case S_PLAY:
        // TEST
        GameModeOutput gmo = game_mode_menu(top_screen, bottom_screen, colour_palette);
        if (!gmo.valid) {
          log_error("ERROR: Failed to process game mode menu");
          close_game(save_data);
          return 1;
        } else {
          // TEMP
        }
        break;
      case S_PILOT:
        int pilot_menu_continuation = pilot_menu(top_screen, bottom_screen, colour_palette, &save_data);
        switch (pilot_menu_continuation) {
          case PS_ERROR:
            log_error("ERROR: Failed to process pilot menu");
            close_game(save_data);
            return 1;
            break;
          case PS_NAME:
            break;
          case PS_ABILITIES:
            AbilitiesMenuOutput abilities_main_menu_data = abilities_menu_main(top_screen, bottom_screen, colour_palette, &save_data);
            switch (abilities_main_menu_data.return_code) {
              case -1:
                log_error("ERROR: Failed to process abilities main menu");
                close_game(save_data);
                return 1;
                break;
              case 0:
                break;
              case 1:
                if (!ability_shop(&save_data, top_screen, bottom_screen, colour_palette, abilities_main_menu_data.current_button)) {
                  log_error("ERROR: Failed to load ability shop");
                  close_game(save_data);
                  return 1;
                }
              break;
            }
            break;
          case PS_SKINS:
            if (!skins_menu(&save_data, top_screen, bottom_screen, colour_palette)) {
              log_error("ERROR: Failed to load skins menu");
              close_game(save_data);
              return 1;
            }
            break;
          case PS_MASTERY:
            if (!mastery_menu(&save_data, top_screen, bottom_screen, colour_palette)) {
              log_error("ERROR: Failed to load mastery menu");
              close_game(save_data);
              return 1;
            }
            break;
          case PS_STATS:
            if (!stats_menu(&save_data, top_screen, bottom_screen, colour_palette)) {
              log_error("ERROR: Failed to load stats menu");
              close_game(save_data);
              return 1;
            }
            break;
        }
        break;
    }

  }
  
  close_game(save_data);
  return 0;
}

bool title_screen(C3D_RenderTarget* top_screen, C3D_RenderTarget* bottom_screen, Colours colour_palette) {
  Sprite title_screen = load_sprite("romfs:/title_screen.t3x");
  if (!title_screen.valid) {
    log_error("ERROR: Failed to load romfs:/title_screen.t3x");
    return false;
  }
  C2D_SpriteSetCenter(&title_screen.sprite, 0, 0);
  C2D_SpriteSetPos(&title_screen.sprite, 0, 0);

  Sprite press_start = load_sprite("romfs:/press_start.t3x");
  if (!press_start.valid) {
    C2D_SpriteSheetFree(title_screen.sprite_sheet);
    log_error("ERROR: Failed to load romfs:/press_start.t3x");
    return false;
  }
  C2D_Image press_start_image = press_start.sprite.image;
  int press_start_w = press_start_image.subtex->width;
  int press_start_h = press_start_image.subtex->height;
  C2D_ImageTint tint;
  float speed = 0.02;
  float transparency = 1.0;
  bool increasing = false;
  C2D_AlphaImageTint(&tint, transparency);

  while (aptMainLoop()) {
    hidScanInput();
    u32 key_pressed = hidKeysDown();
    if (key_pressed & KEY_START) {
      break;
    }

    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    C2D_TargetClear(top_screen, colour_palette.background);
    C2D_TargetClear(bottom_screen, colour_palette.background);
    
    C2D_SceneBegin(top_screen);
    
    C2D_DrawSprite(&title_screen.sprite);

    C2D_SceneBegin(bottom_screen);
    
    C2D_DrawImageAt(press_start_image, (BOTTOM_SCREEN_WIDTH - press_start_w) / 2, (BOTTOM_SCREEN_HEIGHT - press_start_h) / 2, 0, &tint, 1, 1);
    if (transparency >= 1.0) { 
      increasing = false; 
    } else if (transparency <= 0) { 
      increasing = true; 
    }
    if (increasing) { 
      transparency += speed;
      if (transparency > 1.0) { transparency = 1.0; }
    } else { 
      transparency -= speed;
      if (transparency < 0) { transparency = 0; }
    }
    C2D_AlphaImageTint(&tint, transparency);

    C3D_FrameEnd(0);
  }

  C2D_SpriteSheetFree(title_screen.sprite_sheet);
  C2D_SpriteSheetFree(press_start.sprite_sheet);
  
  return true;

}

int selection_menu(C3D_RenderTarget* top_screen, C3D_RenderTarget* bottom_screen, Colours colour_palette) {
  Sprite popup = load_sprite("romfs:/new_game_popup.t3x");
  if (!popup.valid) {
    log_error("ERROR: Failed to load romfs:/new_game_popup.t3x");
    return MENU_ERROR;
  }
  C2D_SpriteSetCenter(&popup.sprite, 0, 0);
  C2D_SpriteSetPos(&popup.sprite, 7*16, 4*16);

  Sprite continue_button = load_sprite("romfs:/continue_button.t3x");
  if (!continue_button.valid) {
    log_error("ERROR: Failed to load romfs:/continue_button.t3x");
    return MENU_ERROR;
  }
  C2D_Sprite continue_button_highlighted;
  C2D_SpriteFromSheet(&continue_button_highlighted, continue_button.sprite_sheet, 1);
  C2D_SpriteSetCenter(&continue_button.sprite, 0, 0);
  C2D_SpriteSetPos(&continue_button.sprite, 11*16, 7*16);
  C2D_SpriteSetCenter(&continue_button_highlighted, 0, 0);
  C2D_SpriteSetPos(&continue_button_highlighted, 11*16, 7*16);
  
  Sprite exit_button = load_sprite("romfs:/exit_button.t3x");
  if (!exit_button.valid) {
    log_error("ERROR: Failed to load romfs:/exit_button.t3x");
    C2D_SpriteSheetFree(continue_button.sprite_sheet);
    return MENU_ERROR;
  }
  C2D_Sprite exit_button_highlighted;
  C2D_SpriteFromSheet(&exit_button_highlighted, exit_button.sprite_sheet, 1);
  C2D_SpriteSetCenter(&exit_button.sprite, 0, 0);
  C2D_SpriteSetPos(&exit_button.sprite, 11*16, 9*16);
  C2D_SpriteSetCenter(&exit_button_highlighted, 0, 0);
  C2D_SpriteSetPos(&exit_button_highlighted, 11*16, 9*16);

  Sprite new_game_button = load_sprite("romfs:/new_game_button.t3x");
  if (!new_game_button.valid) {
    log_error("ERROR: Failed to load romfs:/new_game_button.t3x");
    C2D_SpriteSheetFree(continue_button.sprite_sheet);
    C2D_SpriteSheetFree(exit_button.sprite_sheet);
    return MENU_ERROR;
  }
  C2D_Sprite new_game_button_highlighted;
  C2D_SpriteFromSheet(&new_game_button_highlighted, new_game_button.sprite_sheet, 1);
  C2D_SpriteSetCenter(&new_game_button.sprite, 0, 0);
  C2D_SpriteSetPos(&new_game_button.sprite, 11*16, 5*16);
  C2D_SpriteSetCenter(&new_game_button_highlighted, 0, 0);
  C2D_SpriteSetPos(&new_game_button_highlighted, 11*16, 5*16);
  
  C2D_Sprite highlighted_buttons[MENU_EXIT+1];
  highlighted_buttons[MENU_NEW] = new_game_button_highlighted;
  highlighted_buttons[MENU_CONTINUE] = continue_button_highlighted;
  highlighted_buttons[MENU_EXIT] = exit_button_highlighted;

  int option = MENU_NEW;

  while (aptMainLoop()) {
    hidScanInput();
    u32 key_pressed = hidKeysDown();
    if (key_pressed & KEY_DDOWN) {
      if (option == MENU_EXIT) {
        continue;
      } else {
        option += 1;
      }
    } else if (key_pressed & KEY_DUP) {
      if (option == MENU_NEW) {
        continue;
      } else {
        option -= 1;
      }
    } else if (key_pressed & KEY_A) {
      if (option == MENU_NEW) {
        bool confirmed = false;
        while (!confirmed) {
          hidScanInput();
          u32 key_pressed_updated = hidKeysDown();
          if (key_pressed_updated & KEY_A) { confirmed = true; }
          if (key_pressed_updated & KEY_B) { break; }

          C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
          C2D_TargetClear(top_screen, colour_palette.background);
          C2D_TargetClear(bottom_screen, colour_palette.background);

          C2D_SceneBegin(top_screen);

          C2D_DrawSprite(&popup.sprite);

          C2D_SceneBegin(bottom_screen);

          C2D_DrawSprite(&new_game_button.sprite);
          C2D_DrawSprite(&continue_button.sprite);
          C2D_DrawSprite(&exit_button.sprite);
          C2D_DrawSprite(&highlighted_buttons[option]);

          C3D_FrameEnd(0);
        }
        if (confirmed) { break; } else { continue; }
      } else { break; }
    }

    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    C2D_TargetClear(top_screen, colour_palette.background);
    C2D_TargetClear(bottom_screen, colour_palette.background);
    
    C2D_SceneBegin(top_screen);
    
    C2D_SceneBegin(bottom_screen);

    C2D_DrawSprite(&new_game_button.sprite);
    C2D_DrawSprite(&continue_button.sprite);
    C2D_DrawSprite(&exit_button.sprite);
    C2D_DrawSprite(&highlighted_buttons[option]);

    C3D_FrameEnd(0);
  }

  
  C2D_SpriteSheetFree(popup.sprite_sheet);
  C2D_SpriteSheetFree(new_game_button.sprite_sheet);
  C2D_SpriteSheetFree(continue_button.sprite_sheet);
  C2D_SpriteSheetFree(exit_button.sprite_sheet);
  return option;
}

bool select_character(C3D_RenderTarget* top_screen, C3D_RenderTarget* bottom_screen, Colours colour_palette, SaveFile* save_data) {
  Sprite shinji = load_sprite("romfs:/shinji_selection.t3x");
  if (!shinji.valid) {
    log_error("ERROR: Failed to load romfs:/shinji_selection.t3x");
    return false;
  }
  C2D_Sprite shinji_highlighted;
  C2D_SpriteFromSheet(&shinji_highlighted, shinji.sprite_sheet, 1);
  C2D_SpriteSetCenter(&shinji.sprite, 0, 0);
  C2D_SpriteSetCenter(&shinji_highlighted, 0, 0);

  Sprite asuka = load_sprite("romfs:/asuka_selection.t3x");
  if (!asuka.valid) {
    log_error("ERROR: Failed to load romfs:/asuka_selection.t3x");
    C2D_SpriteSheetFree(shinji.sprite_sheet);
    return false;
  }
  C2D_Sprite asuka_highlighted;
  C2D_SpriteFromSheet(&asuka_highlighted, asuka.sprite_sheet, 1);
  C2D_SpriteSetCenter(&asuka.sprite, 0, 0);
  C2D_SpriteSetCenter(&asuka_highlighted, 0, 0);

  Sprite rei = load_sprite("romfs:/rei_selection.t3x");
  if (!rei.valid) {
    log_error("ERROR: Failed to load romfs:/rei_selection.t3x");
    C2D_SpriteSheetFree(shinji.sprite_sheet);
    C2D_SpriteSheetFree(asuka.sprite_sheet);
    return false;
  }
  C2D_Sprite rei_highlighted;
  C2D_SpriteFromSheet(&rei_highlighted, rei.sprite_sheet, 1);
  C2D_SpriteSetCenter(&rei.sprite, 0, 0);
  C2D_SpriteSetCenter(&rei_highlighted, 0, 0);

  Sprite shinji_info = load_sprite("romfs:/player_info_new.t3x");
  if (!shinji.valid) {
    log_error("ERROR: Failed to load romfs:/player_info_new.t3x");
    C2D_SpriteSheetFree(shinji.sprite_sheet);
    C2D_SpriteSheetFree(asuka.sprite_sheet);
    C2D_SpriteSheetFree(rei.sprite_sheet);
    return false;
  }
  C2D_Sprite asuka_info;
  C2D_Sprite rei_info;
  C2D_SpriteFromSheet(&asuka_info, shinji_info.sprite_sheet, 1);
  C2D_SpriteFromSheet(&rei_info, shinji_info.sprite_sheet, 2);
  C2D_SpriteSetCenter(&shinji_info.sprite, 0, 0);
  C2D_SpriteSetCenter(&asuka_info, 0, 0);
  C2D_SpriteSetCenter(&rei_info, 0, 0);

  enum CurrentCharacter {
    SHINJI,
    ASUKA,
    REI
  };

  int current_character = SHINJI;
  bool selected = false;

  while (aptMainLoop()) {
    hidScanInput();
    u32 key_pressed = hidKeysDown();
    if (key_pressed & KEY_DLEFT) {
      if (current_character == SHINJI) {
        current_character = REI;
      } else {
        current_character -= 1;
      }
    } else if (key_pressed & KEY_DRIGHT) {
      if (current_character == REI) {
        current_character = SHINJI;
      } else {
        current_character += 1;
      }
    } else if (key_pressed & KEY_A) {
      selected = true;
    } else if (key_pressed & KEY_B) {
      return false;
    }

    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    C2D_TargetClear(top_screen, colour_palette.background);
    C2D_TargetClear(bottom_screen, colour_palette.background);
    
    C2D_SceneBegin(top_screen);

    if (!selected) {
      if (current_character == SHINJI) {
        C2D_SpriteSetPos(&rei.sprite, 0+32, 56);
        C2D_DrawSprite(&rei.sprite);

        C2D_SpriteSetPos(&shinji_highlighted, 114+32, 56);
        C2D_DrawSprite(&shinji_highlighted);

        C2D_SpriteSetPos(&asuka.sprite, 228+32, 56);
        C2D_DrawSprite(&asuka.sprite);
      } else if (current_character == ASUKA) {
        C2D_SpriteSetPos(&shinji.sprite, 0+32, 56);
        C2D_DrawSprite(&shinji.sprite);

        C2D_SpriteSetPos(&asuka_highlighted, 114+32, 56);
        C2D_DrawSprite(&asuka_highlighted);

        C2D_SpriteSetPos(&rei.sprite, 228+32, 56);
        C2D_DrawSprite(&rei.sprite);
      } else {
        C2D_SpriteSetPos(&asuka.sprite, 0+32, 56);
        C2D_DrawSprite(&asuka.sprite);

        C2D_SpriteSetPos(&rei_highlighted, 114+32, 56);
        C2D_DrawSprite(&rei_highlighted);

        C2D_SpriteSetPos(&shinji.sprite, 228+32, 56);
        C2D_DrawSprite(&shinji.sprite);
      }
    }
    
    C2D_SceneBegin(bottom_screen);

    if (current_character == SHINJI) {
      C2D_SpriteSetPos(&shinji_info.sprite, 0, 0);
      C2D_DrawSprite(&shinji_info.sprite);
    } else if (current_character == ASUKA) {
      C2D_SpriteSetPos(&asuka_info, 0, 0);
      C2D_DrawSprite(&asuka_info);
   } else {
      C2D_SpriteSetPos(&rei_info, 0, 0);
      C2D_DrawSprite(&rei_info);
   }

    if (selected) {
      break;
    }


    C3D_FrameEnd(0);
  }

  C2D_SpriteSheetFree(shinji.sprite_sheet);
  C2D_SpriteSheetFree(asuka.sprite_sheet);
  C2D_SpriteSheetFree(rei.sprite_sheet);
  C2D_SpriteSheetFree(shinji_info.sprite_sheet);

  if (current_character == SHINJI) { 
    save_data->main.current_pilot = "shinji";
    save_data->pilots.shinji.unlocked = true;
  }
  else if (current_character == ASUKA) { 
    save_data->main.current_pilot = "asuka";
    save_data->pilots.asuka.unlocked = true;
  }
  else { 
    save_data->main.current_pilot = "rei";
    save_data->pilots.rei.unlocked = true;
  }

  return true;
}

int main_menu(C3D_RenderTarget* top_screen, C3D_RenderTarget* bottom_screen, Colours colour_palette, SaveFile save_data) {
  Sprite main_menu_buttons = load_sprite("romfs:/main_menu_buttons.t3x");
  if (!main_menu_buttons.valid) {
    log_error("ERROR: Failed to load romfs:/main_menu_buttons.t3x");
    return -1;
  }

  C2D_Sprite play_button = main_menu_buttons.sprite;
  C2D_Sprite play_button_highlighted;

  C2D_Sprite pilot_button;
  C2D_Sprite pilot_button_highlighted;

  C2D_Sprite options_button;
  C2D_Sprite options_button_highlighted;

  C2D_Sprite exit_button;
  C2D_Sprite exit_button_highlighted;

  C2D_SpriteFromSheet(&play_button_highlighted, main_menu_buttons.sprite_sheet, 1);
  C2D_SpriteFromSheet(&pilot_button, main_menu_buttons.sprite_sheet, 2);
  C2D_SpriteFromSheet(&pilot_button_highlighted, main_menu_buttons.sprite_sheet, 3);
  C2D_SpriteFromSheet(&options_button, main_menu_buttons.sprite_sheet, 4);
  C2D_SpriteFromSheet(&options_button_highlighted, main_menu_buttons.sprite_sheet, 5);
  C2D_SpriteFromSheet(&exit_button, main_menu_buttons.sprite_sheet, 6);
  C2D_SpriteFromSheet(&exit_button_highlighted, main_menu_buttons.sprite_sheet, 7);

  C2D_SpriteSetCenter(&play_button, 0, 0);
  C2D_SpriteSetCenter(&play_button_highlighted, 0, 0);
  C2D_SpriteSetCenter(&pilot_button, 0, 0);
  C2D_SpriteSetCenter(&pilot_button_highlighted, 0, 0);
  C2D_SpriteSetCenter(&options_button, 0, 0);
  C2D_SpriteSetCenter(&options_button_highlighted, 0, 0);
  C2D_SpriteSetCenter(&exit_button, 0, 0);
  C2D_SpriteSetCenter(&exit_button_highlighted, 0, 0);

  C2D_SpriteSetPos(&play_button, 11*16, 4*16);
  C2D_SpriteSetPos(&play_button_highlighted, 11*16, 4*16);
  C2D_SpriteSetPos(&pilot_button, 11*16, 6*16);
  C2D_SpriteSetPos(&pilot_button_highlighted, 11*16, 6*16);
  C2D_SpriteSetPos(&options_button, 11*16, 8*16);
  C2D_SpriteSetPos(&options_button_highlighted, 11*16, 8*16);
  C2D_SpriteSetPos(&exit_button, 11*16, 10*16);
  C2D_SpriteSetPos(&exit_button_highlighted, 11*16, 10*16);

  Sprite main_menu_bg = load_sprite("romfs:/main_menu_bg.t3x");
  if (!main_menu_bg.valid) {
    log_error("ERROR: Failed to load romfs:/main_menu_bg.t3x");
    C2D_SpriteSheetFree(main_menu_buttons.sprite_sheet);
    return -1;
  }
  C2D_SpriteSetCenter(&main_menu_bg.sprite, 0, 0);
  C2D_SpriteSetPos(&main_menu_bg.sprite, 0, 0);

  char* buf = malloc(sizeof(char)*15);
  Text level_points = load_text(strcat(itoa(save_data.main.level_points, buf, 10), " LP"));
  free(buf);
  float font_scale = 1.0;
  
  int selected_option = S_PLAY;
  bool selected = false;

  while (aptMainLoop()) {
    hidScanInput();
    u32 key_pressed = hidKeysDown();
    if (key_pressed & KEY_DDOWN) {
      if (selected_option == S_EXIT) { continue; } else { selected_option += 1; }
    } else if (key_pressed & KEY_DUP) {
      if (selected_option == S_PLAY) { continue; } else { selected_option -= 1; } 
    } else if (key_pressed & KEY_A) { selected = true; }

    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    C2D_TargetClear(top_screen, colour_palette.background);
    C2D_TargetClear(bottom_screen, colour_palette.background);
    
    C2D_SceneBegin(top_screen);
    
    C2D_SceneBegin(bottom_screen);
    
    C2D_DrawSprite(&main_menu_bg.sprite);
    C2D_DrawText(&level_points.text, C2D_WithColor, 35, 0, 0, font_scale, font_scale, colour_palette.white);

    if (!selected) {
      if (selected_option == S_PLAY) {
        C2D_DrawSprite(&play_button_highlighted);
        C2D_DrawSprite(&pilot_button);
        C2D_DrawSprite(&options_button);
        C2D_DrawSprite(&exit_button);
      } else if (selected_option == S_PILOT) {
        C2D_DrawSprite(&play_button);
        C2D_DrawSprite(&pilot_button_highlighted);
        C2D_DrawSprite(&options_button);
        C2D_DrawSprite(&exit_button);
      } else if (selected_option == S_OPTIONS) {
        C2D_DrawSprite(&play_button);
        C2D_DrawSprite(&pilot_button);
        C2D_DrawSprite(&options_button_highlighted);
        C2D_DrawSprite(&exit_button);
      } else if (selected_option == S_EXIT) {
        C2D_DrawSprite(&play_button);
        C2D_DrawSprite(&pilot_button);
        C2D_DrawSprite(&options_button);
        C2D_DrawSprite(&exit_button_highlighted);
      }
    } else { break; }

    C3D_FrameEnd(0);
  }

  C2D_SpriteSheetFree(main_menu_buttons.sprite_sheet);
  C2D_SpriteSheetFree(main_menu_bg.sprite_sheet);
  C2D_TextBufDelete(level_points.buffer);
  return selected_option;
}

int options_menu(C3D_RenderTarget* top_screen, C3D_RenderTarget* bottom_screen, Colours colour_palette) {
  Sprite options_menu_buttons = load_sprite("romfs:/options_menu.t3x");
  if (!options_menu_buttons.valid) {
    log_error("ERROR: Failed to load romfs:/options_menu.t3x");
    return -1;
  }
  C2D_Sprite backup_data_button = options_menu_buttons.sprite;
  C2D_Sprite backup_data_button_highlighted;
  C2D_Sprite credits_button;
  C2D_Sprite credits_button_highlighted;

  C2D_SpriteFromSheet(&backup_data_button_highlighted, options_menu_buttons.sprite_sheet, 1);
  C2D_SpriteFromSheet(&credits_button, options_menu_buttons.sprite_sheet, 2);
  C2D_SpriteFromSheet(&credits_button_highlighted, options_menu_buttons.sprite_sheet, 3);

  C2D_SpriteSetCenter(&backup_data_button, 0, 0);
  C2D_SpriteSetCenter(&backup_data_button_highlighted, 0, 0);
  C2D_SpriteSetCenter(&credits_button, 0, 0);
  C2D_SpriteSetCenter(&credits_button_highlighted, 0, 0);

  C2D_SpriteSetPos(&backup_data_button, 11*16, 7*16);
  C2D_SpriteSetPos(&backup_data_button_highlighted, 11*16, 7*16);
  C2D_SpriteSetPos(&credits_button, 11*16, (7+2)*16);
  C2D_SpriteSetPos(&credits_button_highlighted, 11*16, (7+2)*16);

  Sprite options_menu_bg = load_sprite("romfs:/options_menu_bg.t3x");
  if (!options_menu_bg.valid) {
    log_error("ERROR: Failed to laod romfs:/options_menu_bg.t3x");
    C2D_SpriteSheetFree(options_menu_buttons.sprite_sheet);
    return -1;
  }

  int option = SM_BACKUP_DATA;
  bool selected = false;

  while (aptMainLoop()) {
    hidScanInput();
    u32 key_pressed = hidKeysDown();
    if (key_pressed & KEY_DUP) {
      if (option == SM_BACKUP_DATA) { continue; } else { option -= 1; }
    } else if (key_pressed & KEY_DDOWN) {
      if (option == SM_CREDITS) { continue; } else { option += 1; } 
    } else if (key_pressed & KEY_A) { selected = true; }
    else if (key_pressed & KEY_B) {
      C2D_SpriteSheetFree(options_menu_bg.sprite_sheet);
      C2D_SpriteSheetFree(options_menu_buttons.sprite_sheet);
      return SM_QUIT;
    } 

    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    C2D_TargetClear(top_screen, colour_palette.background);
    C2D_TargetClear(bottom_screen, colour_palette.background);
    
    C2D_SceneBegin(top_screen);
    
    C2D_SceneBegin(bottom_screen);
    
    C2D_DrawSprite(&options_menu_bg.sprite);

    if (!selected) {
      if (option == SM_BACKUP_DATA) {
        C2D_DrawSprite(&backup_data_button_highlighted);
        C2D_DrawSprite(&credits_button);
      } else if (option == SM_CREDITS) {
        C2D_DrawSprite(&backup_data_button);
        C2D_DrawSprite(&credits_button_highlighted);
      }
    } else { break; }

    C3D_FrameEnd(0);
  }

  C2D_SpriteSheetFree(options_menu_buttons.sprite_sheet);
  C2D_SpriteSheetFree(options_menu_bg.sprite_sheet);
  return option;

}

bool credits(C3D_RenderTarget* top_screen, C3D_RenderTarget* bottom_screen, Colours colour_palette) {
  Sprite credits = load_sprite("romfs:/credits.t3x");
  if (!credits.valid) {
    log_error("ERROR: Failed to load romfs:/credits.t3x");
    return false;
  }
  C2D_Sprite credits_top = credits.sprite;
  C2D_Sprite credits_bottom;

  C2D_SpriteFromSheet(&credits_bottom, credits.sprite_sheet, 1);

  C2D_SpriteSetCenter(&credits_top, 0, 0);
  C2D_SpriteSetCenter(&credits_bottom, 0, 0);

  C2D_SpriteSetPos(&credits_top, 0, 0);
  C2D_SpriteSetPos(&credits_bottom, 0, 0);

  while (aptMainLoop()) {
    hidScanInput();
    u32 key_pressed = hidKeysDown();
    if (key_pressed & KEY_B) {
      break;
    } 

    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    C2D_TargetClear(top_screen, colour_palette.background);
    C2D_TargetClear(bottom_screen, colour_palette.background);
    
    C2D_SceneBegin(top_screen);

    C2D_DrawSprite(&credits_top);
    
    C2D_SceneBegin(bottom_screen);

    C2D_DrawSprite(&credits_bottom);
   
    C3D_FrameEnd(0);
  }

  C2D_SpriteSheetFree(credits.sprite_sheet);
  return true;
}

int pilot_menu(C3D_RenderTarget* top_screen, C3D_RenderTarget* bottom_screen, Colours colour_palette, SaveFile* save_data) {
  Sprite shinji_selection = load_sprite("romfs:/shinji_selection.t3x");
  if (!shinji_selection.valid) {
    log_error("ERROR: Failed to load romfs:/shinji_selection.t3x");
    return PS_ERROR;
  }
  C2D_Sprite shinji_highlighted;
  C2D_Sprite shinji = shinji_selection.sprite;
  C2D_SpriteFromSheet(&shinji_highlighted, shinji_selection.sprite_sheet, 1);
  C2D_SpriteSetCenter(&shinji, 0, 0);
  C2D_SpriteSetCenter(&shinji_highlighted, 0, 0);

  Sprite asuka_selection = load_sprite("romfs:/asuka_selection.t3x");
  if (!asuka_selection.valid) {
    log_error("ERROR: Failed to load romfs:/asuka_selection.t3x");
    C2D_SpriteSheetFree(shinji_selection.sprite_sheet);
    return PS_ERROR;
  }
  C2D_Sprite asuka_highlighted;
  C2D_Sprite asuka = asuka_selection.sprite;
  C2D_SpriteFromSheet(&asuka_highlighted, asuka_selection.sprite_sheet, 1);
  C2D_SpriteSetCenter(&asuka, 0, 0);
  C2D_SpriteSetCenter(&asuka_highlighted, 0, 0);

  Sprite rei_selection = load_sprite("romfs:/rei_selection.t3x");
  if (!rei_selection.valid) {
    log_error("ERROR: Failed to load romfs:/rei_selection.t3x");
    C2D_SpriteSheetFree(shinji_selection.sprite_sheet);
    C2D_SpriteSheetFree(asuka_selection.sprite_sheet);
    return PS_ERROR;
  }
  C2D_Sprite rei = rei_selection.sprite;
  C2D_Sprite rei_highlighted;
  C2D_SpriteFromSheet(&rei_highlighted, rei_selection.sprite_sheet, 1);
  C2D_SpriteSetCenter(&rei, 0, 0);
  C2D_SpriteSetCenter(&rei_highlighted, 0, 0);

  Sprite characters_master = load_sprite("romfs:/characters_selection_master.t3x");
  if (!characters_master.valid) {
    log_error("ERROR: Failed to load romfs:/characters_selection_master.t3x");
    C2D_SpriteSheetFree(shinji_selection.sprite_sheet);
    C2D_SpriteSheetFree(rei_selection.sprite_sheet);
    C2D_SpriteSheetFree(asuka_selection.sprite_sheet);
    return PS_ERROR;
  }
  C2D_Sprite shinji_master_highlighted = characters_master.sprite;
  C2D_Sprite shinji_master;
  C2D_Sprite asuka_master;
  C2D_Sprite asuka_master_highlighted;
  C2D_Sprite rei_master;
  C2D_Sprite rei_master_highlighted;
  C2D_SpriteFromSheet(&shinji_master, characters_master.sprite_sheet, 1);
  C2D_SpriteFromSheet(&asuka_master, characters_master.sprite_sheet, 3);
  C2D_SpriteFromSheet(&asuka_master_highlighted, characters_master.sprite_sheet, 2);
  C2D_SpriteFromSheet(&rei_master, characters_master.sprite_sheet, 5);
  C2D_SpriteFromSheet(&rei_master_highlighted, characters_master.sprite_sheet, 4);
  C2D_SpriteSetCenter(&shinji_master, 0, 0);
  C2D_SpriteSetCenter(&shinji_master_highlighted, 0, 0);
  C2D_SpriteSetCenter(&asuka_master, 0, 0);
  C2D_SpriteSetCenter(&asuka_master_highlighted, 0, 0);
  C2D_SpriteSetCenter(&rei_master, 0, 0);
  C2D_SpriteSetCenter(&rei_master_highlighted, 0, 0);


  Sprite characters_name = load_sprite("romfs:/characters_name_pilot.t3x");
  if (!characters_name.valid) {
    log_error("ERROR: Failed to load romfs:/characters_name_pilot.t3x");
    C2D_SpriteSheetFree(shinji_selection.sprite_sheet);
    C2D_SpriteSheetFree(rei_selection.sprite_sheet);
    C2D_SpriteSheetFree(characters_master.sprite_sheet);
    return PS_ERROR;
  }
  C2D_Sprite shinji_name = characters_name.sprite;
  C2D_Sprite shinji_name_highlighted;
  C2D_Sprite asuka_name;
  C2D_Sprite asuka_name_highlighted;
  C2D_Sprite rei_name;
  C2D_Sprite rei_name_highlighted;
  C2D_SpriteFromSheet(&shinji_name_highlighted, characters_name.sprite_sheet, 1);
  C2D_SpriteFromSheet(&asuka_name, characters_name.sprite_sheet, 2);
  C2D_SpriteFromSheet(&asuka_name_highlighted, characters_name.sprite_sheet, 3);
  C2D_SpriteFromSheet(&rei_name, characters_name.sprite_sheet, 4);
  C2D_SpriteFromSheet(&rei_name_highlighted, characters_name.sprite_sheet, 5);
  C2D_SpriteSetCenter(&shinji_name, 0, 0);
  C2D_SpriteSetCenter(&shinji_name_highlighted, 0, 0);
  C2D_SpriteSetCenter(&asuka_name, 0, 0);
  C2D_SpriteSetCenter(&asuka_name_highlighted, 0, 0);
  C2D_SpriteSetCenter(&rei_name, 0, 0);
  C2D_SpriteSetCenter(&rei_name_highlighted, 0, 0);
  C2D_SpriteSetPos(&shinji_name, 6*16, 3*16);
  C2D_SpriteSetPos(&shinji_name_highlighted, 6*16, 3*16);
  C2D_SpriteSetPos(&asuka_name, 6*16, 3*16);
  C2D_SpriteSetPos(&asuka_name_highlighted, 6*16, 3*16);
  C2D_SpriteSetPos(&rei_name, 6*16, 3*16);
  C2D_SpriteSetPos(&rei_name_highlighted, 6*16, 3*16);

  Sprite menu_buttons = load_sprite("romfs:/pilot_menu_buttons.t3x");
  if (!menu_buttons.valid) {
    log_error("ERROR: Failed to load romfs:/pilot_button_highlighted.t3x");
    C2D_SpriteSheetFree(shinji_selection.sprite_sheet);
    C2D_SpriteSheetFree(rei_selection.sprite_sheet);
    C2D_SpriteSheetFree(characters_master.sprite_sheet);
    C2D_SpriteSheetFree(characters_name.sprite_sheet);
    return PS_ERROR;
  }
  C2D_Sprite stats = menu_buttons.sprite;
  C2D_Sprite stats_highlighted;
  C2D_Sprite abilities;
  C2D_Sprite abilities_highlighted;
  C2D_Sprite mastery;
  C2D_Sprite mastery_highlighted;
  C2D_Sprite skins;
  C2D_Sprite skins_highlighted;
  C2D_SpriteFromSheet(&stats_highlighted, menu_buttons.sprite_sheet, 1);
  C2D_SpriteFromSheet(&abilities, menu_buttons.sprite_sheet, 2);
  C2D_SpriteFromSheet(&abilities_highlighted, menu_buttons.sprite_sheet, 3);
  C2D_SpriteFromSheet(&mastery, menu_buttons.sprite_sheet, 4);
  C2D_SpriteFromSheet(&mastery_highlighted, menu_buttons.sprite_sheet, 5);
  C2D_SpriteFromSheet(&skins, menu_buttons.sprite_sheet, 6);
  C2D_SpriteFromSheet(&skins_highlighted, menu_buttons.sprite_sheet, 7);
  C2D_SpriteSetCenter(&stats, 0, 0);
  C2D_SpriteSetCenter(&stats_highlighted, 0, 0);
  C2D_SpriteSetCenter(&abilities, 0, 0);
  C2D_SpriteSetCenter(&abilities_highlighted, 0, 0);
  C2D_SpriteSetCenter(&mastery, 0, 0);
  C2D_SpriteSetCenter(&mastery_highlighted, 0, 0);
  C2D_SpriteSetCenter(&skins, 0, 0);
  C2D_SpriteSetCenter(&skins_highlighted, 0, 0);
  C2D_SpriteSetPos(&stats, 6*16, 5*16);
  C2D_SpriteSetPos(&stats_highlighted, 6*16, 5*16);
  C2D_SpriteSetPos(&abilities, 6*16, 7*16);
  C2D_SpriteSetPos(&abilities_highlighted, 6*16, 7*16);
  C2D_SpriteSetPos(&mastery, 6*16, 9*16);
  C2D_SpriteSetPos(&mastery_highlighted, 6*16, 9*16);
  C2D_SpriteSetPos(&skins, 6*16, 11*16);
  C2D_SpriteSetPos(&skins_highlighted, 6*16, 11*16);

  Sprite pilot_menu_background = load_sprite("romfs:/pilot_menu_bg.t3x");
  if (!pilot_menu_background.valid) {
    log_error("ERROR: Failed to load romfs:/pilot_menu_bg.t3x");
    C2D_SpriteSheetFree(menu_buttons.sprite_sheet);
    C2D_SpriteSheetFree(shinji_selection.sprite_sheet);
    C2D_SpriteSheetFree(rei_selection.sprite_sheet);
    C2D_SpriteSheetFree(characters_master.sprite_sheet);
    C2D_SpriteSheetFree(characters_name.sprite_sheet);
    return PS_ERROR;
  }
  C2D_SpriteSetCenter(&pilot_menu_background.sprite, 0, 0);
  C2D_SpriteSetPos(&pilot_menu_background.sprite, 0, 0);


  C2D_Sprite buttons_highlighted[PS_SKINS];
  buttons_highlighted[PS_STATS-1] = stats_highlighted;
  buttons_highlighted[PS_ABILITIES-1] = abilities_highlighted;
  buttons_highlighted[PS_MASTERY-1] = mastery_highlighted;
  buttons_highlighted[PS_SKINS-1] = skins_highlighted;

  int current_button = PS_NAME;
  int selected_button = PS_NAME;
  char* current_character = save_data->main.current_pilot;

  while (aptMainLoop()) {
    hidScanInput();
    u32 key_pressed = hidKeysDown();
    if (key_pressed & KEY_DUP) {
      if (current_button == PS_NAME) { continue; } else { current_button--; }
    } else if (key_pressed & KEY_DDOWN) {
      if (current_button == PS_SKINS) { continue; } else { current_button++; }
    } else if (key_pressed & KEY_A) {
      selected_button = current_button;
      if (current_button == PS_NAME) {
        while (true) {
          hidScanInput();
          u32 key_pressed_updated = hidKeysDown();
          if (key_pressed_updated & KEY_DLEFT) {
            if (strcmp(current_character, "rei") == 0) {
              current_character = "asuka";
            } else if (strcmp(current_character, "shinji") == 0) {
              current_character = "rei";
            } else { current_character = "shinji"; }
          } else if (key_pressed_updated & KEY_DRIGHT) {
            if (strcmp(current_character, "rei") == 0) {
              current_character = "shinji";
            } else if (strcmp(current_character, "shinji") == 0) {
              current_character = "asuka";
            } else { current_character = "rei"; }
          } else if (key_pressed_updated & KEY_A || key_pressed_updated & KEY_B) {
            break;
          }
          C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
          C2D_TargetClear(top_screen, colour_palette.background);
          C2D_TargetClear(bottom_screen, colour_palette.background);
          C2D_SceneBegin(top_screen);
          if (strcmp(current_character, "shinji") == 0) {
            C2D_SpriteSetPos(&rei, 32, 56);
            C2D_SpriteSetPos(&rei_master, 32, 56);
            if (strcmp(save_data->pilots.rei.current_skin, "master") == 0) { C2D_DrawSprite(&rei_master); } else { C2D_DrawSprite(&rei); }

            C2D_SpriteSetPos(&shinji_highlighted, 114+32, 56);
            C2D_SpriteSetPos(&shinji_master_highlighted, 114+32, 56);
            if (strcmp(save_data->pilots.shinji.current_skin, "master") == 0) { C2D_DrawSprite(&shinji_master_highlighted); } else { C2D_DrawSprite(&shinji_highlighted); }

            C2D_SpriteSetPos(&asuka, 228+32, 56);
            C2D_SpriteSetPos(&asuka_master, 228+32, 56);
            if (strcmp(save_data->pilots.asuka.current_skin, "master") == 0) { C2D_DrawSprite(&asuka_master); } else { C2D_DrawSprite(&asuka); }
          } else if (strcmp(current_character, "asuka") == 0) {
            C2D_SpriteSetPos(&shinji, 32, 56);
            C2D_SpriteSetPos(&shinji_master, 32, 56);
            if (strcmp(save_data->pilots.shinji.current_skin, "master") == 0) { C2D_DrawSprite(&shinji_master); } else { C2D_DrawSprite(&shinji); }

            C2D_SpriteSetPos(&asuka_highlighted, 114+32, 56);
            C2D_SpriteSetPos(&asuka_master_highlighted, 114+32, 56);
            if (strcmp(save_data->pilots.asuka.current_skin, "master") == 0) { C2D_DrawSprite(&asuka_master_highlighted); } else { C2D_DrawSprite(&asuka_highlighted); }

            C2D_SpriteSetPos(&rei, 228+32, 56);
            C2D_SpriteSetPos(&rei_master, 228+32, 56);
            if (strcmp(save_data->pilots.rei.current_skin, "master") == 0) { C2D_DrawSprite(&rei_master); } else { C2D_DrawSprite(&rei); }
          } else if (strcmp(current_character, "rei") == 0) {
            C2D_SpriteSetPos(&asuka, 32, 56);
            C2D_SpriteSetPos(&asuka_master, 32, 56);
            if (strcmp(save_data->pilots.asuka.current_skin, "master") == 0) { C2D_DrawSprite(&asuka_master); } else { C2D_DrawSprite(&asuka); }

            C2D_SpriteSetPos(&rei_highlighted, 114+32, 56);
            C2D_SpriteSetPos(&rei_master_highlighted, 114+32, 56);
            if (strcmp(save_data->pilots.rei.current_skin, "master") == 0) { C2D_DrawSprite(&rei_master_highlighted); } else { C2D_DrawSprite(&rei_highlighted); }

            C2D_SpriteSetPos(&shinji, 228+32, 56);
            C2D_SpriteSetPos(&shinji_master, 228+32, 56);
            if (strcmp(save_data->pilots.shinji.current_skin, "master") == 0) { C2D_DrawSprite(&shinji_master); } else { C2D_DrawSprite(&shinji); }
          }
          C2D_SceneBegin(bottom_screen);
          if (strcmp(current_character, "shinji") == 0) {
            if (current_button == PS_NAME) {C2D_DrawSprite(&shinji_name_highlighted); } else { C2D_DrawSprite(&shinji_name); }
          } else if (strcmp(current_character, "asuka") == 0) {
            if (current_button == PS_NAME) {C2D_DrawSprite(&asuka_name_highlighted); } else { C2D_DrawSprite(&asuka_name); }
          } else if (strcmp(current_character, "rei") == 0) {
            if (current_button == PS_NAME) {C2D_DrawSprite(&rei_name_highlighted); } else { C2D_DrawSprite(&rei_name); }
          }
          C3D_FrameEnd(0);
        }
      } else { selected_button = current_button; break; }
    } else if (key_pressed & KEY_B) { break; }

    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    C2D_TargetClear(top_screen, colour_palette.background);
    C2D_TargetClear(bottom_screen, colour_palette.background);
    
    C2D_SceneBegin(top_screen);

    if (strcmp(current_character, "shinji") == 0) {
      C2D_SpriteSetPos(&rei, 32, 56);
      C2D_SpriteSetPos(&rei_master, 32, 56);
      if (strcmp(save_data->pilots.rei.current_skin, "master") == 0) { C2D_DrawSprite(&rei_master); } else { C2D_DrawSprite(&rei); }

      C2D_SpriteSetPos(&shinji_highlighted, 114+32, 56);
      C2D_SpriteSetPos(&shinji_master_highlighted, 114+32, 56);
      if (strcmp(save_data->pilots.shinji.current_skin, "master") == 0) { C2D_DrawSprite(&shinji_master_highlighted); } else { C2D_DrawSprite(&shinji_highlighted); }

      C2D_SpriteSetPos(&asuka, 228+32, 56);
      C2D_SpriteSetPos(&asuka_master, 228+32, 56);
      if (strcmp(save_data->pilots.asuka.current_skin, "master") == 0) { C2D_DrawSprite(&asuka_master); } else { C2D_DrawSprite(&asuka); }
    } else if (strcmp(current_character, "asuka") == 0) {
      C2D_SpriteSetPos(&shinji, 32, 56);
      C2D_SpriteSetPos(&shinji_master, 32, 56);
      if (strcmp(save_data->pilots.shinji.current_skin, "master") == 0) { C2D_DrawSprite(&shinji_master); } else { C2D_DrawSprite(&shinji); }

      C2D_SpriteSetPos(&asuka_highlighted, 114+32, 56);
      C2D_SpriteSetPos(&asuka_master_highlighted, 114+32, 56);
      if (strcmp(save_data->pilots.asuka.current_skin, "master") == 0) { C2D_DrawSprite(&asuka_master_highlighted); } else { C2D_DrawSprite(&asuka_highlighted); }

      C2D_SpriteSetPos(&rei, 228+32, 56);
      C2D_SpriteSetPos(&rei_master, 228+32, 56);
      if (strcmp(save_data->pilots.rei.current_skin, "master") == 0) { C2D_DrawSprite(&rei_master); } else { C2D_DrawSprite(&rei); }
    } else if (strcmp(current_character, "rei") == 0) {
      C2D_SpriteSetPos(&asuka, 32, 56);
      C2D_SpriteSetPos(&asuka_master, 32, 56);
      if (strcmp(save_data->pilots.asuka.current_skin, "master") == 0) { C2D_DrawSprite(&asuka_master); } else { C2D_DrawSprite(&asuka); }

      C2D_SpriteSetPos(&rei_highlighted, 114+32, 56);
      C2D_SpriteSetPos(&rei_master_highlighted, 114+32, 56);
      if (strcmp(save_data->pilots.rei.current_skin, "master") == 0) { C2D_DrawSprite(&rei_master_highlighted); } else { C2D_DrawSprite(&rei_highlighted); }

      C2D_SpriteSetPos(&shinji, 228+32, 56);
      C2D_SpriteSetPos(&shinji_master, 228+32, 56);
      if (strcmp(save_data->pilots.shinji.current_skin, "master") == 0) { C2D_DrawSprite(&shinji_master); } else { C2D_DrawSprite(&shinji); }
    }

    C2D_SceneBegin(bottom_screen);

    C2D_DrawSprite(&pilot_menu_background.sprite);

    if (strcmp(current_character, "shinji") == 0) {
      if (current_button == PS_NAME) {C2D_DrawSprite(&shinji_name_highlighted); } else { C2D_DrawSprite(&shinji_name); }
    } else if (strcmp(current_character, "asuka") == 0) {
      if (current_button == PS_NAME) {C2D_DrawSprite(&asuka_name_highlighted); } else { C2D_DrawSprite(&asuka_name); }
    } else if (strcmp(current_character, "rei") == 0) {
      if (current_button == PS_NAME) {C2D_DrawSprite(&rei_name_highlighted); } else { C2D_DrawSprite(&rei_name); }
    }

    C2D_DrawSprite(&stats);
    C2D_DrawSprite(&abilities);
    C2D_DrawSprite(&mastery);
    C2D_DrawSprite(&skins);
    if (current_button != PS_NAME) { C2D_DrawSprite(&buttons_highlighted[current_button-1]); }

    C3D_FrameEnd(0);
  }

  C2D_SpriteSheetFree(menu_buttons.sprite_sheet);
  C2D_SpriteSheetFree(shinji_selection.sprite_sheet);
  C2D_SpriteSheetFree(rei_selection.sprite_sheet);
  C2D_SpriteSheetFree(characters_master.sprite_sheet);
  C2D_SpriteSheetFree(characters_name.sprite_sheet);

  save_data->main.current_pilot = current_character;
  return selected_button;
}

AbilitiesMenuOutput abilities_menu_main(C3D_RenderTarget* top_screen, C3D_RenderTarget* bottom_screen, Colours colour_palette, SaveFile* save_data) {
  /*
     return codes: error: -1, success and do nothing: 0, success and go to abilities menu: 1

  */
  
  AbilitiesMenuOutput output;
  output.return_code = -1;


  Sprite background = load_sprite("romfs:/main_menu_bg.t3x");
  if (!background.valid) {
    log_error("ERROR: Failed to load romfs:/main_menu_bg.t3x");
    return output;
  }
  C2D_SpriteSetCenter(&background.sprite, 0, 0);
  C2D_SpriteSetPos(&background.sprite, 0, 0);

  Sprite ability_slots = load_sprite("romfs:/ability_slots.t3x");
  if (!ability_slots.valid) {
    log_error("ERROR: Failed to load romfs:/ability_slots.t3x");
    C2D_SpriteSheetFree(background.sprite_sheet);
    return output;
  }

  C2D_Sprite ability_locked = ability_slots.sprite;
  C2D_Sprite ability_unlocked;
  C2D_Sprite absolute_accuracy;
  C2D_Sprite celestial_gift;
  C2D_Sprite counter;
  C2D_Sprite divine_intervention;
  C2D_Sprite double_or_nothing;
  C2D_Sprite eek_run;
  C2D_Sprite healing_mist;
  C2D_Sprite miracle_sync;
  C2D_Sprite overclock;
  C2D_Sprite ruthless;

  C2D_Sprite ability_locked_highlighted;
  C2D_Sprite ability_unlocked_highlighted;
  C2D_Sprite absolute_accuracy_highlighted;
  C2D_Sprite celestial_gift_highlighted;
  C2D_Sprite counter_highlighted;
  C2D_Sprite divine_intervention_highlighted;
  C2D_Sprite double_or_nothing_highlighted;
  C2D_Sprite eek_run_highlighted;
  C2D_Sprite healing_mist_highlighted;
  C2D_Sprite miracle_sync_highlighted;
  C2D_Sprite overclock_highlighted;
  C2D_Sprite ruthless_highlighted;

  C2D_SpriteFromSheet(&ability_unlocked, ability_slots.sprite_sheet, 2);
  C2D_SpriteFromSheet(&absolute_accuracy, ability_slots.sprite_sheet, 4);
  C2D_SpriteFromSheet(&celestial_gift, ability_slots.sprite_sheet, 6);
  C2D_SpriteFromSheet(&counter, ability_slots.sprite_sheet, 8);
  C2D_SpriteFromSheet(&divine_intervention, ability_slots.sprite_sheet, 10);
  C2D_SpriteFromSheet(&double_or_nothing, ability_slots.sprite_sheet, 12);
  C2D_SpriteFromSheet(&eek_run, ability_slots.sprite_sheet, 14);
  C2D_SpriteFromSheet(&healing_mist, ability_slots.sprite_sheet, 16);
  C2D_SpriteFromSheet(&miracle_sync, ability_slots.sprite_sheet, 18);
  C2D_SpriteFromSheet(&overclock, ability_slots.sprite_sheet, 20);
  C2D_SpriteFromSheet(&ruthless, ability_slots.sprite_sheet, 22);

  C2D_SpriteFromSheet(&ability_locked_highlighted, ability_slots.sprite_sheet, 1);
  C2D_SpriteFromSheet(&ability_unlocked_highlighted, ability_slots.sprite_sheet, 3);
  C2D_SpriteFromSheet(&absolute_accuracy_highlighted, ability_slots.sprite_sheet, 5);
  C2D_SpriteFromSheet(&celestial_gift_highlighted, ability_slots.sprite_sheet, 7);
  C2D_SpriteFromSheet(&counter_highlighted, ability_slots.sprite_sheet, 9);
  C2D_SpriteFromSheet(&divine_intervention_highlighted, ability_slots.sprite_sheet, 11);
  C2D_SpriteFromSheet(&double_or_nothing_highlighted, ability_slots.sprite_sheet, 13);
  C2D_SpriteFromSheet(&eek_run_highlighted, ability_slots.sprite_sheet, 15);
  C2D_SpriteFromSheet(&healing_mist_highlighted, ability_slots.sprite_sheet, 17);
  C2D_SpriteFromSheet(&miracle_sync_highlighted, ability_slots.sprite_sheet, 19);
  C2D_SpriteFromSheet(&overclock_highlighted, ability_slots.sprite_sheet, 21);
  C2D_SpriteFromSheet(&ruthless_highlighted, ability_slots.sprite_sheet, 23);

  C2D_SpriteSetCenter(&ability_locked, 0, 0);
  C2D_SpriteSetCenter(&ability_unlocked, 0, 0);
  C2D_SpriteSetCenter(&absolute_accuracy, 0, 0);
  C2D_SpriteSetCenter(&celestial_gift, 0, 0);
  C2D_SpriteSetCenter(&counter, 0, 0);
  C2D_SpriteSetCenter(&divine_intervention, 0, 0);
  C2D_SpriteSetCenter(&double_or_nothing, 0, 0);
  C2D_SpriteSetCenter(&eek_run, 0, 0);
  C2D_SpriteSetCenter(&healing_mist, 0, 0);
  C2D_SpriteSetCenter(&miracle_sync, 0, 0);
  C2D_SpriteSetCenter(&overclock, 0, 0);
  C2D_SpriteSetCenter(&ruthless, 0, 0);

  C2D_SpriteSetCenter(&ability_locked_highlighted, 0, 0);
  C2D_SpriteSetCenter(&ability_unlocked_highlighted, 0, 0);
  C2D_SpriteSetCenter(&absolute_accuracy_highlighted, 0, 0);
  C2D_SpriteSetCenter(&celestial_gift_highlighted, 0, 0);
  C2D_SpriteSetCenter(&counter_highlighted, 0, 0);
  C2D_SpriteSetCenter(&divine_intervention_highlighted, 0, 0);
  C2D_SpriteSetCenter(&double_or_nothing_highlighted, 0, 0);
  C2D_SpriteSetCenter(&eek_run_highlighted, 0, 0);
  C2D_SpriteSetCenter(&healing_mist_highlighted, 0, 0);
  C2D_SpriteSetCenter(&miracle_sync_highlighted, 0, 0);
  C2D_SpriteSetCenter(&overclock_highlighted, 0, 0);
  C2D_SpriteSetCenter(&ruthless_highlighted, 0, 0);
  
  Sprite ability_info_1 = load_sprite("romfs:/abilities_main_info_1.t3x");
  if (!ability_info_1.valid) {
    log_error("ERROR: Failed to load romfs:/abilities_main_info_1.t3x");
    C2D_SpriteSheetFree(background.sprite_sheet);
    C2D_SpriteSheetFree(ability_slots.sprite_sheet);
    return output;
  }

  C2D_Sprite absolute_accuracy_info = ability_info_1.sprite;
  C2D_Sprite celestial_gift_info;
  C2D_Sprite counter_info;
  C2D_Sprite divine_intervention_info;
  C2D_Sprite double_or_nothing_info;

  C2D_SpriteFromSheet(&celestial_gift_info, ability_info_1.sprite_sheet, 1);
  C2D_SpriteFromSheet(&counter_info, ability_info_1.sprite_sheet, 2);
  C2D_SpriteFromSheet(&divine_intervention_info, ability_info_1.sprite_sheet, 3);
  C2D_SpriteFromSheet(&double_or_nothing_info, ability_info_1.sprite_sheet, 4);

  C2D_SpriteSetCenter(&celestial_gift_info, 0, 0);
  C2D_SpriteSetCenter(&counter_info, 0, 0);
  C2D_SpriteSetCenter(&divine_intervention_info, 0, 0);
  C2D_SpriteSetCenter(&double_or_nothing_info, 0, 0);

  C2D_SpriteSetPos(&celestial_gift_info, 0, 0);
  C2D_SpriteSetPos(&counter_info, 0, 0);
  C2D_SpriteSetPos(&divine_intervention_info, 0, 0);
  C2D_SpriteSetPos(&double_or_nothing_info, 0, 0);

  Sprite ability_info_2 = load_sprite("romfs:/abilities_main_info_2.t3x");
  if (!ability_info_2.valid) {
    log_error("ERROR: Failed to load romfs:/abilities_main_info_2.t3x");
    C2D_SpriteSheetFree(background.sprite_sheet);
    C2D_SpriteSheetFree(ability_slots.sprite_sheet);
    C2D_SpriteSheetFree(ability_info_1.sprite_sheet);
    return output;
  }

  C2D_Sprite eek_run_info = ability_info_2.sprite;
  C2D_Sprite healing_mist_info;
  C2D_Sprite miracle_sync_info;
  C2D_Sprite overclock_info;
  C2D_Sprite ruthless_info;

  C2D_SpriteFromSheet(&healing_mist_info, ability_info_2.sprite_sheet, 1);
  C2D_SpriteFromSheet(&miracle_sync_info, ability_info_2.sprite_sheet, 2);
  C2D_SpriteFromSheet(&overclock_info, ability_info_2.sprite_sheet, 3);
  C2D_SpriteFromSheet(&ruthless_info, ability_info_2.sprite_sheet, 4);

  C2D_SpriteSetCenter(&eek_run_info, 0, 0);
  C2D_SpriteSetCenter(&healing_mist_info, 0, 0);
  C2D_SpriteSetCenter(&miracle_sync_info, 0, 0);
  C2D_SpriteSetCenter(&overclock_info, 0, 0);
  C2D_SpriteSetCenter(&ruthless_info, 0, 0);

  C2D_SpriteSetPos(&eek_run_info, 0, 0);
  C2D_SpriteSetPos(&healing_mist_info, 0, 0);
  C2D_SpriteSetPos(&miracle_sync_info, 0, 0);
  C2D_SpriteSetPos(&overclock_info, 0, 0);
  C2D_SpriteSetPos(&ruthless_info, 0, 0);

  Sprite abilities_button = load_sprite("romfs:/abilities_button.t3x");
  if (!abilities_button.valid) {
    log_error("ERROR: Failed to load romfs:/abilities_button.t3x");
    C2D_SpriteSheetFree(background.sprite_sheet);
    C2D_SpriteSheetFree(ability_slots.sprite_sheet);
    C2D_SpriteSheetFree(ability_info_1.sprite_sheet);
    C2D_SpriteSheetFree(ability_info_2.sprite_sheet);
    return output;
  }
  C2D_Sprite abilities_button_highlighted;
  C2D_SpriteFromSheet(&abilities_button_highlighted, abilities_button.sprite_sheet, 1);
  C2D_SpriteSetCenter(&abilities_button.sprite, 0, 0);
  C2D_SpriteSetCenter(&abilities_button_highlighted, 0, 0);

  Sprite confirm_purchase = load_sprite("romfs:/purchase_slot_popup.t3x");
  if (!confirm_purchase.valid) {
    log_error("ERROR: Failed to load romfs:/purchase_slot_popup.t3x");
    C2D_SpriteSheetFree(background.sprite_sheet);
    C2D_SpriteSheetFree(ability_slots.sprite_sheet);
    C2D_SpriteSheetFree(ability_info_1.sprite_sheet);
    C2D_SpriteSheetFree(ability_info_2.sprite_sheet);
    C2D_SpriteSheetFree(abilities_button.sprite_sheet);
    return output;
  }
  C2D_SpriteSetCenter(&confirm_purchase.sprite, 0, 0);
  C2D_SpriteSetPos(&confirm_purchase.sprite, 7*16, 4*16);

  Sprite insufficiant_funds = load_sprite("romfs:/insufficiant_funds.t3x");
  if (!insufficiant_funds.valid) {
    log_error("ERROR: Failed to load romfs:/insufficiant_funds.t3x");
    C2D_SpriteSheetFree(background.sprite_sheet);
    C2D_SpriteSheetFree(ability_slots.sprite_sheet);
    C2D_SpriteSheetFree(ability_info_1.sprite_sheet);
    C2D_SpriteSheetFree(ability_info_2.sprite_sheet);
    C2D_SpriteSheetFree(abilities_button.sprite_sheet);
    return output;
  };
  C2D_SpriteSetCenter(&insufficiant_funds.sprite, 0, 0);
  C2D_SpriteSetPos(&insufficiant_funds.sprite, 7*16, 4*16);
 

  int level_points = save_data->main.level_points;
  bool level_points_changed = false;

  int slot_price = 2000;

  enum pilot {
    SHINJI,
    ASUKA,
    REI
  };

  int current_pilot = SHINJI;

  if (strcmp(save_data->main.current_pilot, "shinji") == 0) {
    current_pilot = SHINJI;
  } else if (strcmp(save_data->main.current_pilot, "asuka") == 0) {
    current_pilot = ASUKA;
  } else if (strcmp(save_data->main.current_pilot, "rei") == 0) {
    current_pilot = REI;
  }

  char* buf = malloc(sizeof(char)*15);
  Text level_points_text = load_text(strcat(itoa(level_points, buf, 10), " LP"));
  free(buf);
  float font_scale = 1.0;

  char* buf2 = malloc(sizeof(char)*15);
  Text slot_price_text = load_text(strcat(itoa(slot_price, buf2, 10), " LP"));
  free(buf2);

  C2D_Sprite ability_slot_icons[RUTHLESS+1];
  C2D_Sprite ability_slot_highlighted_icons[RUTHLESS+1];

  ability_slot_icons[ABILITY_LOCKED] = ability_locked;
  ability_slot_icons[ABILITY_UNLOCKED] = ability_unlocked;
  ability_slot_icons[ABSOLUTE_ACCURACY] = absolute_accuracy;
  ability_slot_icons[CELESTIAL_GIFT] = celestial_gift;
  ability_slot_icons[COUNTER] = counter;
  ability_slot_icons[DIVINE_INTERVENTION] = divine_intervention;
  ability_slot_icons[DOUBLE_OR_NOTHING] = double_or_nothing;
  ability_slot_icons[EEK_RUN] = eek_run;
  ability_slot_icons[HEALING_MIST] = healing_mist;
  ability_slot_icons[MIRACLE_SYNC] = miracle_sync;
  ability_slot_icons[OVERCLOCK] = overclock;
  ability_slot_icons[RUTHLESS] = ruthless;

  ability_slot_highlighted_icons[ABILITY_LOCKED] = ability_locked_highlighted;
  ability_slot_highlighted_icons[ABILITY_UNLOCKED] = ability_unlocked_highlighted;
  ability_slot_highlighted_icons[ABSOLUTE_ACCURACY] = absolute_accuracy_highlighted;
  ability_slot_highlighted_icons[CELESTIAL_GIFT] = celestial_gift_highlighted;
  ability_slot_highlighted_icons[COUNTER] = counter_highlighted;
  ability_slot_highlighted_icons[DIVINE_INTERVENTION] = divine_intervention_highlighted;
  ability_slot_highlighted_icons[DOUBLE_OR_NOTHING] = double_or_nothing_highlighted;
  ability_slot_highlighted_icons[EEK_RUN] = eek_run_highlighted;
  ability_slot_highlighted_icons[HEALING_MIST] = healing_mist_highlighted;
  ability_slot_highlighted_icons[MIRACLE_SYNC] = miracle_sync_highlighted;
  ability_slot_highlighted_icons[OVERCLOCK] = overclock_highlighted;
  ability_slot_highlighted_icons[RUTHLESS] = ruthless_highlighted;

  C2D_Sprite ability_info_array[RUTHLESS+1];
  ability_info_array[ABSOLUTE_ACCURACY] = absolute_accuracy_info;
  ability_info_array[CELESTIAL_GIFT] = celestial_gift_info;
  ability_info_array[COUNTER] = counter_info;
  ability_info_array[DIVINE_INTERVENTION] = divine_intervention_info;
  ability_info_array[DOUBLE_OR_NOTHING] = double_or_nothing_info;
  ability_info_array[EEK_RUN] = eek_run_info;
  ability_info_array[HEALING_MIST] = healing_mist_info;
  ability_info_array[MIRACLE_SYNC] = miracle_sync_info;
  ability_info_array[OVERCLOCK] = overclock_info;
  ability_info_array[RUTHLESS] = ruthless_info;

  int current_abilities[4];

  switch (current_pilot) {
    int locked_ability_count;
    case SHINJI:
      for (int i = 0; i < 4; i++) {
        if (strcmp(save_data->pilots.shinji.active_abilities[i], "null") == 0) { current_abilities[i] = ABILITY_UNLOCKED; }
        else if (strcmp(save_data->pilots.shinji.active_abilities[i], "absolute_accuracy") == 0) { current_abilities[i] = ABSOLUTE_ACCURACY; }
        else if (strcmp(save_data->pilots.shinji.active_abilities[i], "celestial_gift") == 0) { current_abilities[i] = CELESTIAL_GIFT; }
        else if (strcmp(save_data->pilots.shinji.active_abilities[i], "counter") == 0) { current_abilities[i] = COUNTER; }
        else if (strcmp(save_data->pilots.shinji.active_abilities[i], "divine_intervention") == 0) { current_abilities[i] = DIVINE_INTERVENTION; }
        else if (strcmp(save_data->pilots.shinji.active_abilities[i], "double_or_nothing") == 0) { current_abilities[i] = DOUBLE_OR_NOTHING; }
        else if (strcmp(save_data->pilots.shinji.active_abilities[i], "eek_run") == 0) { current_abilities[i] = EEK_RUN; }
        else if (strcmp(save_data->pilots.shinji.active_abilities[i], "healing_mist") == 0) { current_abilities[i] = HEALING_MIST; }
        else if (strcmp(save_data->pilots.shinji.active_abilities[i], "miracle_sync") == 0) { current_abilities[i] = MIRACLE_SYNC; }
        else if (strcmp(save_data->pilots.shinji.active_abilities[i], "overclock") == 0) { current_abilities[i] = OVERCLOCK; }
        else if (strcmp(save_data->pilots.shinji.active_abilities[i], "ruthless") == 0) { current_abilities[i] = RUTHLESS; }
      }
      locked_ability_count = 4 - save_data->pilots.shinji.ability_slots;
      for (int i = 0; i < 4; i++) { if (i >= 4 - locked_ability_count) { current_abilities[i] = ABILITY_LOCKED; } }
      break;
    case ASUKA:
      for (int i = 0; i < 4; i++) {
        if (strcmp(save_data->pilots.asuka.active_abilities[i], "null") == 0) { current_abilities[i] = ABILITY_UNLOCKED; }
        else if (strcmp(save_data->pilots.asuka.active_abilities[i], "absolute_accuracy") == 0) { current_abilities[i] = ABSOLUTE_ACCURACY; }
        else if (strcmp(save_data->pilots.asuka.active_abilities[i], "celestial_gift") == 0) { current_abilities[i] = CELESTIAL_GIFT; }
        else if (strcmp(save_data->pilots.asuka.active_abilities[i], "counter") == 0) { current_abilities[i] = COUNTER; }
        else if (strcmp(save_data->pilots.asuka.active_abilities[i], "divine_intervention") == 0) { current_abilities[i] = DIVINE_INTERVENTION; }
        else if (strcmp(save_data->pilots.asuka.active_abilities[i], "double_or_nothing") == 0) { current_abilities[i] = DOUBLE_OR_NOTHING; }
        else if (strcmp(save_data->pilots.asuka.active_abilities[i], "eek_run") == 0) { current_abilities[i] = EEK_RUN; }
        else if (strcmp(save_data->pilots.asuka.active_abilities[i], "healing_mist") == 0) { current_abilities[i] = HEALING_MIST; }
        else if (strcmp(save_data->pilots.asuka.active_abilities[i], "miracle_sync") == 0) { current_abilities[i] = MIRACLE_SYNC; }
        else if (strcmp(save_data->pilots.asuka.active_abilities[i], "overclock") == 0) { current_abilities[i] = OVERCLOCK; }
        else if (strcmp(save_data->pilots.asuka.active_abilities[i], "ruthless") == 0) { current_abilities[i] = RUTHLESS; }
      }
      locked_ability_count = 4 - save_data->pilots.asuka.ability_slots;
      for (int i = 0; i < 4; i++) { if (i >= 4 - locked_ability_count) { current_abilities[i] = ABILITY_LOCKED; } }
      break;
    case REI:
      for (int i = 0; i < 4; i++) {
        if (strcmp(save_data->pilots.rei.active_abilities[i], "null") == 0) { current_abilities[i] = ABILITY_UNLOCKED; }
        else if (strcmp(save_data->pilots.rei.active_abilities[i], "absolute_accuracy") == 0) { current_abilities[i] = ABSOLUTE_ACCURACY; }
        else if (strcmp(save_data->pilots.rei.active_abilities[i], "celestial_gift") == 0) { current_abilities[i] = CELESTIAL_GIFT; }
        else if (strcmp(save_data->pilots.rei.active_abilities[i], "counter") == 0) { current_abilities[i] = COUNTER; }
        else if (strcmp(save_data->pilots.rei.active_abilities[i], "divine_intervention") == 0) { current_abilities[i] = DIVINE_INTERVENTION; }
        else if (strcmp(save_data->pilots.rei.active_abilities[i], "double_or_nothing") == 0) { current_abilities[i] = DOUBLE_OR_NOTHING; }
        else if (strcmp(save_data->pilots.rei.active_abilities[i], "eek_run") == 0) { current_abilities[i] = EEK_RUN; }
        else if (strcmp(save_data->pilots.rei.active_abilities[i], "healing_mist") == 0) { current_abilities[i] = HEALING_MIST; }
        else if (strcmp(save_data->pilots.rei.active_abilities[i], "miracle_sync") == 0) { current_abilities[i] = MIRACLE_SYNC; }
        else if (strcmp(save_data->pilots.rei.active_abilities[i], "overclock") == 0) { current_abilities[i] = OVERCLOCK; }
        else if (strcmp(save_data->pilots.rei.active_abilities[i], "ruthless") == 0) { current_abilities[i] = RUTHLESS; }
      }
      locked_ability_count = 4 - save_data->pilots.rei.ability_slots;
      for (int i = 0; i < 4; i++) { if (i >= 4 - locked_ability_count) { current_abilities[i] = ABILITY_LOCKED; } }
      break;
  }

  int current_button = SLOT1;


  while (aptMainLoop()) {
    hidScanInput();
    u32 key_pressed = hidKeysDown();

    if (key_pressed & KEY_DDOWN) {
      if (current_button != ABILITIES) {
        if (current_button != SLOT4) {
          current_button += 2;
        } else { current_button++; }
      }
    } else if (key_pressed & KEY_DUP) {
      if (current_button != SLOT1 && current_button != SLOT2) { current_button -= 2; }
    } else if (key_pressed & KEY_DRIGHT) {
      if (current_button == SLOT1 || current_button == SLOT3) { current_button++; }
    } else if (key_pressed & KEY_DLEFT) {
      if (current_button == SLOT2 || current_button == SLOT4) { current_button--; }
    }

    else if (key_pressed & KEY_B) { output.return_code = 0; break; }

    else if (key_pressed & KEY_A) {
      if (current_abilities[current_button] == ABILITIES) {
        output.current_button = current_button;
        output.return_code = 1;
        break;
      } else {
        if (current_abilities[current_button] != ABILITY_LOCKED) {
          output.current_button = current_button;
          output.current_button_data = current_abilities[current_button];
          output.return_code = 1;
          break;
        } else {
          C2D_TextBufClear(slot_price_text.buffer);
          char* buf = malloc(sizeof(char)*15);
          slot_price_text = load_text(strcat(itoa(slot_price, buf, 10), " LP"));
          free(buf);
          bool should_run = true;
          switch (current_pilot) {
            case SHINJI:
              while (should_run) {
                hidScanInput();
                key_pressed = hidKeysDown();
  
                C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
                C2D_TargetClear(top_screen, colour_palette.background);
                C2D_SceneBegin(top_screen);

                if (key_pressed & KEY_B) { should_run = false; }
                else if (key_pressed & KEY_A) {
                  if (level_points - slot_price < 0) { 
                    C2D_DrawSprite(&insufficiant_funds.sprite);
                    C3D_FrameEnd(0);
                    while (true) {
                      hidScanInput();
                      key_pressed = hidKeysDown();
                      if (key_pressed & KEY_A) { break; }
                    }
                    should_run = false;
                  }
                  else {
                    level_points -= slot_price;
                    level_points_changed = true;
                    save_data->main.level_points = level_points;
                    slot_price += 1000;
                    current_abilities[current_button] = ABILITY_UNLOCKED;
                    save_data->pilots.shinji.ability_slots++;
                    should_run = false;
                  }
                }

                if (should_run) { 
                  C2D_DrawSprite(&confirm_purchase.sprite);
                  C2D_DrawText(&slot_price_text.text, C2D_WithColor, 11*16, 7*16, 0, font_scale, font_scale, colour_palette.white);
                }

                C3D_FrameEnd(0);
              }
              break;
            case ASUKA:
              while (should_run) {
                hidScanInput();
                key_pressed = hidKeysDown();
  
                C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
                C2D_TargetClear(top_screen, colour_palette.background);
                C2D_SceneBegin(top_screen);

                if (key_pressed & KEY_B) { should_run = false; }
                else if (key_pressed & KEY_A) {
                  if (level_points - slot_price < 0) { 
                    C2D_DrawSprite(&insufficiant_funds.sprite);
                    C3D_FrameEnd(0);
                    while (true) {
                      hidScanInput();
                      key_pressed = hidKeysDown();
                      if (key_pressed & KEY_A) { break; }
                    }
                    should_run = false;
                  }
                  else {
                    level_points -= slot_price;
                    level_points_changed = true;
                    save_data->main.level_points = level_points;
                    slot_price += 1000;
                    current_abilities[current_button] = ABILITY_UNLOCKED;
                    save_data->pilots.asuka.ability_slots++;
                    should_run = false;
                  }
                }

                if (should_run) { 
                  C2D_DrawSprite(&confirm_purchase.sprite); 
                  C2D_DrawText(&slot_price_text.text, C2D_WithColor, 11*16, 7*16, 0, font_scale, font_scale, colour_palette.white); 
                }

                C3D_FrameEnd(0);
              }
              break;
            case REI:
              while (should_run) {
                hidScanInput();
                key_pressed = hidKeysDown();
  
                C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
                C2D_TargetClear(top_screen, colour_palette.background);
                C2D_SceneBegin(top_screen);

                if (key_pressed & KEY_B) { should_run = false; }
                else if (key_pressed & KEY_A) {
                  if (level_points - slot_price < 0) { 
                    C2D_DrawSprite(&insufficiant_funds.sprite);
                    C3D_FrameEnd(0);
                    while (true) {
                      hidScanInput();
                      key_pressed = hidKeysDown();
                      if (key_pressed & KEY_A) { break; }
                    }
                    should_run = false;
                  }
                  else {
                    level_points -= slot_price;
                    level_points_changed = true;
                    save_data->main.level_points = level_points;
                    slot_price += 1000;
                    current_abilities[current_button] = ABILITY_UNLOCKED;
                    save_data->pilots.rei.ability_slots++;
                    should_run = false;
                  }
                }

                if (should_run) { 
                  C2D_DrawSprite(&confirm_purchase.sprite); 
                  C2D_DrawText(&slot_price_text.text, C2D_WithColor, 11*16, 7*16, 0, font_scale, font_scale, colour_palette.white);
                }

                C3D_FrameEnd(0);
              }
              break;
          }
        }
      }
    }

    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    C2D_TargetClear(top_screen, colour_palette.background);
    C2D_TargetClear(bottom_screen, colour_palette.background);

    C2D_SceneBegin(top_screen);
    
    if (current_button != ABILITIES) {
      if (current_abilities[current_button] != ABILITY_LOCKED && current_abilities[current_button] != ABILITY_UNLOCKED) {
        C2D_DrawSprite(&ability_info_array[current_abilities[current_button]]);
      }
    }

    C2D_SceneBegin(bottom_screen);

    C2D_DrawSprite(&background.sprite);

    if (level_points_changed) {
      C2D_TextBufClear(level_points_text.buffer);
      char* buf = malloc(sizeof(char)*15);
      level_points_text = load_text(strcat(itoa(level_points, buf, 10), " LP"));
      free(buf);
      level_points_changed = false;
    }
    C2D_DrawText(&level_points_text.text, C2D_WithColor, 35, 0, 0, font_scale, font_scale, colour_palette.white);

    C2D_SpriteSetPos(&ability_slot_icons[current_abilities[0]], 7*16, 4*16);
    C2D_DrawSprite(&ability_slot_icons[current_abilities[0]]);

    C2D_SpriteSetPos(&ability_slot_icons[current_abilities[1]], 10*16, 4*16);
    C2D_DrawSprite(&ability_slot_icons[current_abilities[1]]);

    C2D_SpriteSetPos(&ability_slot_icons[current_abilities[2]], 7*16, 7*16);
    C2D_DrawSprite(&ability_slot_icons[current_abilities[2]]);

    C2D_SpriteSetPos(&ability_slot_icons[current_abilities[3]], 10*16, 7*16);
    C2D_DrawSprite(&ability_slot_icons[current_abilities[3]]);

    C2D_SpriteSetPos(&abilities_button.sprite, 6*16, 10*16);
    C2D_DrawSprite(&abilities_button.sprite);

    switch (current_button) {
      case SLOT1:
        C2D_SpriteSetPos(&ability_slot_highlighted_icons[current_abilities[0]], 7*16, 4*16);
        C2D_DrawSprite(&ability_slot_highlighted_icons[current_abilities[0]]);
        break;
      case SLOT2:
        C2D_SpriteSetPos(&ability_slot_highlighted_icons[current_abilities[1]], 10*16, 4*16);
        C2D_DrawSprite(&ability_slot_highlighted_icons[current_abilities[1]]);
        break;
      case SLOT3:
        C2D_SpriteSetPos(&ability_slot_highlighted_icons[current_abilities[2]], 7*16, 7*16);
        C2D_DrawSprite(&ability_slot_highlighted_icons[current_abilities[2]]);
        break;
      case SLOT4:
        C2D_SpriteSetPos(&ability_slot_highlighted_icons[current_abilities[3]], 10*16, 7*16);
        C2D_DrawSprite(&ability_slot_highlighted_icons[current_abilities[3]]);
        break;
      case ABILITIES:
        C2D_SpriteSetPos(&abilities_button_highlighted, 6*16, 10*16);
        C2D_DrawSprite(&abilities_button_highlighted);
        break;
    }

    C3D_FrameEnd(0);
  }

  
  C2D_SpriteSheetFree(background.sprite_sheet);
  C2D_SpriteSheetFree(ability_slots.sprite_sheet);
  C2D_SpriteSheetFree(ability_info_1.sprite_sheet);
  C2D_SpriteSheetFree(ability_info_2.sprite_sheet);
  C2D_TextBufDelete(level_points_text.buffer);
  C2D_TextBufDelete(slot_price_text.buffer);
  C2D_SpriteSheetFree(confirm_purchase.sprite_sheet);
  C2D_SpriteSheetFree(insufficiant_funds.sprite_sheet);
  

  return output;

}

bool ability_shop(SaveFile* save_data, C3D_RenderTarget* top_screen, C3D_RenderTarget* bottom_screen, Colours colour_palette, int input_button) {
   enum {
    ABSOLUTE_ACCURACY,
    CELESTIAL_GIFT,
    COUNTER,
    DIVINE_INTERVENTION,
    DOUBLE_OR_NOTHING,
    EEK_RUN,
    HEALING_MIST,
    MIRACLE_SYNC,
    OVERCLOCK,
    RUTHLESS
  };

  char* abilities_string_array[RUTHLESS+1];
  abilities_string_array[ABSOLUTE_ACCURACY] = "absolute_accuracy";
  abilities_string_array[CELESTIAL_GIFT] = "celestial_gift";
  abilities_string_array[COUNTER] = "counter";
  abilities_string_array[DIVINE_INTERVENTION] = "divine_intervention";
  abilities_string_array[DOUBLE_OR_NOTHING] = "double_or_nothing";
  abilities_string_array[EEK_RUN] = "eek_run";
  abilities_string_array[HEALING_MIST] = "healing_mist";
  abilities_string_array[MIRACLE_SYNC] = "miracle_sync";
  abilities_string_array[OVERCLOCK] = "overclock";
  abilities_string_array[RUTHLESS] = "ruthless";

  Sprite ability_shop_bg = load_sprite("romfs:/ability_shop_bg.t3x");
  if (!ability_shop_bg.valid) {
    log_error("ERROR: Failed to load romfs:/ability_shop_bg.t3x");
    return false;
  }
  C2D_SpriteSetCenter(&ability_shop_bg.sprite, 0, 0);
  C2D_SpriteSetPos(&ability_shop_bg.sprite, 0, 0);

  Sprite ability_info = load_sprite("romfs:/ability_shop_info.t3x");
  if (!ability_info.valid) {
    C2D_SpriteSheetFree(ability_shop_bg.sprite_sheet);
    log_error("ERROR: Failed to load romfs:/ability_shop_info.t3x");
    return false;
  }

  C2D_Sprite absolute_accuracy_info = ability_info.sprite;
  C2D_Sprite celestial_gift_info;
  C2D_Sprite counter_info;
  C2D_Sprite divine_intervention_info;
  C2D_Sprite double_or_nothing_info;
  C2D_Sprite eek_run_info;
  C2D_Sprite healing_mist_info;
  C2D_Sprite miracle_sync_info;
  C2D_Sprite overclock_info;
  C2D_Sprite ruthless_info;

  C2D_SpriteFromSheet(&celestial_gift_info, ability_info.sprite_sheet, 1);
  C2D_SpriteFromSheet(&counter_info, ability_info.sprite_sheet, 2);
  C2D_SpriteFromSheet(&divine_intervention_info, ability_info.sprite_sheet, 3);
  C2D_SpriteFromSheet(&double_or_nothing_info, ability_info.sprite_sheet, 4);
  C2D_SpriteFromSheet(&eek_run_info, ability_info.sprite_sheet, 5);
  C2D_SpriteFromSheet(&healing_mist_info, ability_info.sprite_sheet, 6);
  C2D_SpriteFromSheet(&miracle_sync_info, ability_info.sprite_sheet, 7);
  C2D_SpriteFromSheet(&overclock_info, ability_info.sprite_sheet, 8);
  C2D_SpriteFromSheet(&ruthless_info, ability_info.sprite_sheet, 9);

  C2D_SpriteSetCenter(&absolute_accuracy_info, 0, 0);
  C2D_SpriteSetCenter(&celestial_gift_info, 0, 0);
  C2D_SpriteSetCenter(&counter_info, 0, 0);
  C2D_SpriteSetCenter(&divine_intervention_info, 0, 0);
  C2D_SpriteSetCenter(&double_or_nothing_info, 0, 0);
  C2D_SpriteSetCenter(&eek_run_info, 0, 0);
  C2D_SpriteSetCenter(&healing_mist_info, 0, 0);
  C2D_SpriteSetCenter(&miracle_sync_info, 0, 0);
  C2D_SpriteSetCenter(&overclock_info, 0, 0);
  C2D_SpriteSetCenter(&ruthless_info, 0, 0);

  C2D_SpriteSetPos(&absolute_accuracy_info, 3*16, 3*16);
  C2D_SpriteSetPos(&celestial_gift_info, 3*16, 3*16);
  C2D_SpriteSetPos(&counter_info, 3*16, 3*16);
  C2D_SpriteSetPos(&divine_intervention_info, 3*16, 3*16);
  C2D_SpriteSetPos(&double_or_nothing_info, 3*16, 3*16);
  C2D_SpriteSetPos(&eek_run_info, 3*16, 3*16);
  C2D_SpriteSetPos(&healing_mist_info, 3*16, 3*16);
  C2D_SpriteSetPos(&miracle_sync_info, 3*16, 3*16);
  C2D_SpriteSetPos(&overclock_info, 3*16, 3*16);
  C2D_SpriteSetPos(&ruthless_info, 3*16, 3*16);

  C2D_Sprite info[RUTHLESS+1];
  info[ABSOLUTE_ACCURACY] = absolute_accuracy_info;
  info[CELESTIAL_GIFT] = celestial_gift_info;
  info[COUNTER] = counter_info;
  info[DIVINE_INTERVENTION] = divine_intervention_info;
  info[DOUBLE_OR_NOTHING] = double_or_nothing_info;
  info[EEK_RUN] = eek_run_info;
  info[HEALING_MIST] = healing_mist_info;
  info[MIRACLE_SYNC] = miracle_sync_info;
  info[OVERCLOCK] = overclock_info;
  info[RUTHLESS] = ruthless_info;

  Sprite ability_icons = load_sprite("romfs:/ability_slots.t3x");
  if (!ability_icons.valid) {
    C2D_SpriteSheetFree(ability_info.sprite_sheet);
    C2D_SpriteSheetFree(ability_shop_bg.sprite_sheet);
    log_error("ERROR: Failed to load romfs:/ability_slots.t3x");
    return false;
  }

  C2D_Sprite absolute_accuracy;
  C2D_Sprite celestial_gift;
  C2D_Sprite counter;
  C2D_Sprite divine_intervention;
  C2D_Sprite double_or_nothing;
  C2D_Sprite eek_run;
  C2D_Sprite healing_mist;
  C2D_Sprite miracle_sync;
  C2D_Sprite overclock;
  C2D_Sprite ruthless;

  C2D_Sprite absolute_accuracy_highlighted;
  C2D_Sprite celestial_gift_highlighted;
  C2D_Sprite counter_highlighted;
  C2D_Sprite divine_intervention_highlighted;
  C2D_Sprite double_or_nothing_highlighted;
  C2D_Sprite eek_run_highlighted;
  C2D_Sprite healing_mist_highlighted;
  C2D_Sprite miracle_sync_highlighted;
  C2D_Sprite overclock_highlighted;
  C2D_Sprite ruthless_highlighted;

  C2D_SpriteFromSheet(&absolute_accuracy, ability_icons.sprite_sheet, 4);
  C2D_SpriteFromSheet(&celestial_gift, ability_icons.sprite_sheet, 6);
  C2D_SpriteFromSheet(&counter, ability_icons.sprite_sheet, 8);
  C2D_SpriteFromSheet(&divine_intervention, ability_icons.sprite_sheet, 10);
  C2D_SpriteFromSheet(&double_or_nothing, ability_icons.sprite_sheet, 12);
  C2D_SpriteFromSheet(&eek_run, ability_icons.sprite_sheet, 14);
  C2D_SpriteFromSheet(&healing_mist, ability_icons.sprite_sheet, 16);
  C2D_SpriteFromSheet(&miracle_sync, ability_icons.sprite_sheet, 18);
  C2D_SpriteFromSheet(&overclock, ability_icons.sprite_sheet, 20);
  C2D_SpriteFromSheet(&ruthless, ability_icons.sprite_sheet, 22);

  C2D_SpriteFromSheet(&absolute_accuracy_highlighted, ability_icons.sprite_sheet, 5);
  C2D_SpriteFromSheet(&celestial_gift_highlighted, ability_icons.sprite_sheet, 7);
  C2D_SpriteFromSheet(&counter_highlighted, ability_icons.sprite_sheet, 9);
  C2D_SpriteFromSheet(&divine_intervention_highlighted, ability_icons.sprite_sheet, 11);
  C2D_SpriteFromSheet(&double_or_nothing_highlighted, ability_icons.sprite_sheet, 13);
  C2D_SpriteFromSheet(&eek_run_highlighted, ability_icons.sprite_sheet, 15);
  C2D_SpriteFromSheet(&healing_mist_highlighted, ability_icons.sprite_sheet, 17);
  C2D_SpriteFromSheet(&miracle_sync_highlighted, ability_icons.sprite_sheet, 19);
  C2D_SpriteFromSheet(&overclock_highlighted, ability_icons.sprite_sheet, 21);
  C2D_SpriteFromSheet(&ruthless_highlighted, ability_icons.sprite_sheet, 23);

  C2D_SpriteSetCenter(&absolute_accuracy, 0, 0);
  C2D_SpriteSetCenter(&celestial_gift, 0, 0);
  C2D_SpriteSetCenter(&counter, 0, 0);
  C2D_SpriteSetCenter(&divine_intervention, 0, 0);
  C2D_SpriteSetCenter(&double_or_nothing, 0, 0);
  C2D_SpriteSetCenter(&eek_run, 0, 0);
  C2D_SpriteSetCenter(&healing_mist, 0, 0);
  C2D_SpriteSetCenter(&miracle_sync, 0, 0);
  C2D_SpriteSetCenter(&overclock, 0, 0);
  C2D_SpriteSetCenter(&ruthless, 0, 0);

  C2D_SpriteSetCenter(&absolute_accuracy_highlighted, 0, 0);
  C2D_SpriteSetCenter(&celestial_gift_highlighted, 0, 0);
  C2D_SpriteSetCenter(&counter_highlighted, 0, 0);
  C2D_SpriteSetCenter(&divine_intervention_highlighted, 0, 0);
  C2D_SpriteSetCenter(&double_or_nothing_highlighted, 0, 0);
  C2D_SpriteSetCenter(&eek_run_highlighted, 0, 0);
  C2D_SpriteSetCenter(&healing_mist_highlighted, 0, 0);
  C2D_SpriteSetCenter(&miracle_sync_highlighted, 0, 0);
  C2D_SpriteSetCenter(&overclock_highlighted, 0, 0);
  C2D_SpriteSetCenter(&ruthless_highlighted, 0, 0);
 
  
  C2D_SpriteSetPos(&absolute_accuracy, 2*16, 2*16);
  C2D_SpriteSetPos(&celestial_gift, 5*16, 2*16);
  C2D_SpriteSetPos(&counter, 8*16, 2*16);
  C2D_SpriteSetPos(&divine_intervention, 184, 2*16);
  C2D_SpriteSetPos(&double_or_nothing, 15*16, 2*16);
  C2D_SpriteSetPos(&eek_run, 18*16, 2*16);
  C2D_SpriteSetPos(&healing_mist, 21*16, 2*16);
  C2D_SpriteSetPos(&miracle_sync, 2*16, 4*16);
  C2D_SpriteSetPos(&overclock, 5*16, 4*16);
  C2D_SpriteSetPos(&ruthless, 8*16, 4*16);

  C2D_SpriteSetPos(&absolute_accuracy_highlighted, 2*16, 2*16);
  C2D_SpriteSetPos(&celestial_gift_highlighted, 5*16, 2*16);
  C2D_SpriteSetPos(&counter_highlighted, 8*16, 2*16);
  C2D_SpriteSetPos(&divine_intervention_highlighted, 184, 2*16);
  C2D_SpriteSetPos(&double_or_nothing_highlighted, 15*16, 2*16);
  C2D_SpriteSetPos(&eek_run_highlighted, 18*16, 2*16);
  C2D_SpriteSetPos(&healing_mist_highlighted, 21*16, 2*16);
  C2D_SpriteSetPos(&miracle_sync_highlighted, 2*16, 4*16);
  C2D_SpriteSetPos(&overclock_highlighted, 5*16, 4*16);
  C2D_SpriteSetPos(&ruthless_highlighted, 8*16, 4*16);
 
  Sprite ability_icons_locked = load_sprite("romfs:/ability_icons_locked.t3x");
  if (!ability_icons_locked.valid) {
    C2D_SpriteSheetFree(ability_shop_bg.sprite_sheet);
    C2D_SpriteSheetFree(ability_icons.sprite_sheet);
    C2D_SpriteSheetFree(ability_info.sprite_sheet);
    log_error("ERROR: Failed to load romfs:/ability_icons_locked.t3x");
    return false;
  }

  C2D_Sprite absolute_accuracy_locked = ability_icons_locked.sprite;
  C2D_Sprite celestial_gift_locked;
  C2D_Sprite counter_locked;
  C2D_Sprite divine_intervention_locked;
  C2D_Sprite double_or_nothing_locked;
  C2D_Sprite eek_run_locked;
  C2D_Sprite healing_mist_locked;
  C2D_Sprite miracle_sync_locked;
  C2D_Sprite overclock_locked;
  C2D_Sprite ruthless_locked;

  C2D_SpriteFromSheet(&celestial_gift_locked, ability_icons_locked.sprite_sheet, 1);
  C2D_SpriteFromSheet(&counter_locked, ability_icons_locked.sprite_sheet, 2);
  C2D_SpriteFromSheet(&divine_intervention_locked, ability_icons_locked.sprite_sheet, 3);
  C2D_SpriteFromSheet(&double_or_nothing_locked, ability_icons_locked.sprite_sheet, 4);
  C2D_SpriteFromSheet(&eek_run_locked, ability_icons_locked.sprite_sheet, 5);
  C2D_SpriteFromSheet(&healing_mist_locked, ability_icons_locked.sprite_sheet, 6);
  C2D_SpriteFromSheet(&miracle_sync_locked, ability_icons_locked.sprite_sheet, 7);
  C2D_SpriteFromSheet(&overclock_locked, ability_icons_locked.sprite_sheet, 8);
  C2D_SpriteFromSheet(&ruthless_locked, ability_icons_locked.sprite_sheet, 9);

  C2D_SpriteSetCenter(&absolute_accuracy_locked, 0, 0);
  C2D_SpriteSetCenter(&celestial_gift_locked, 0, 0);
  C2D_SpriteSetCenter(&counter_locked, 0, 0);
  C2D_SpriteSetCenter(&divine_intervention_locked, 0, 0);
  C2D_SpriteSetCenter(&double_or_nothing_locked, 0, 0);
  C2D_SpriteSetCenter(&eek_run_locked, 0, 0);
  C2D_SpriteSetCenter(&healing_mist_locked, 0, 0);
  C2D_SpriteSetCenter(&miracle_sync_locked, 0, 0);
  C2D_SpriteSetCenter(&overclock_locked, 0, 0);
  C2D_SpriteSetCenter(&ruthless_locked, 0, 0);

  Sprite ability_icons_locked_highlighted = load_sprite("romfs:/ability_icons_locked_highlighted.t3x");
  if (!ability_icons_locked_highlighted.valid) {
    C2D_SpriteSheetFree(ability_shop_bg.sprite_sheet);
    C2D_SpriteSheetFree(ability_icons.sprite_sheet);
    C2D_SpriteSheetFree(ability_info.sprite_sheet);
    C2D_SpriteSheetFree(ability_icons_locked.sprite_sheet);
    log_error("ERROR: Failed to load romfs:/ability_icons_locked_highlighted.t3x");
    return false;
  }

  C2D_Sprite absolute_accuracy_locked_highlighted = ability_icons_locked_highlighted.sprite;
  C2D_Sprite celestial_gift_locked_highlighted;
  C2D_Sprite counter_locked_highlighted;
  C2D_Sprite divine_intervention_locked_highlighted;
  C2D_Sprite double_or_nothing_locked_highlighted;
  C2D_Sprite eek_run_locked_highlighted;
  C2D_Sprite healing_mist_locked_highlighted;
  C2D_Sprite miracle_sync_locked_highlighted;
  C2D_Sprite overclock_locked_highlighted;
  C2D_Sprite ruthless_locked_highlighted;

  C2D_SpriteFromSheet(&celestial_gift_locked_highlighted, ability_icons_locked_highlighted.sprite_sheet, 1);
  C2D_SpriteFromSheet(&counter_locked_highlighted, ability_icons_locked_highlighted.sprite_sheet, 2);
  C2D_SpriteFromSheet(&divine_intervention_locked_highlighted, ability_icons_locked_highlighted.sprite_sheet, 3);
  C2D_SpriteFromSheet(&double_or_nothing_locked_highlighted, ability_icons_locked_highlighted.sprite_sheet, 4);
  C2D_SpriteFromSheet(&eek_run_locked_highlighted, ability_icons_locked_highlighted.sprite_sheet, 5);
  C2D_SpriteFromSheet(&healing_mist_locked_highlighted, ability_icons_locked_highlighted.sprite_sheet, 6);
  C2D_SpriteFromSheet(&miracle_sync_locked_highlighted, ability_icons_locked_highlighted.sprite_sheet, 7);
  C2D_SpriteFromSheet(&overclock_locked_highlighted, ability_icons_locked_highlighted.sprite_sheet, 8);
  C2D_SpriteFromSheet(&ruthless_locked_highlighted, ability_icons_locked_highlighted.sprite_sheet, 9);

  C2D_SpriteSetCenter(&absolute_accuracy_locked_highlighted, 0, 0);
  C2D_SpriteSetCenter(&celestial_gift_locked_highlighted, 0, 0);
  C2D_SpriteSetCenter(&counter_locked_highlighted, 0, 0);
  C2D_SpriteSetCenter(&divine_intervention_locked_highlighted, 0, 0);
  C2D_SpriteSetCenter(&double_or_nothing_locked_highlighted, 0, 0);
  C2D_SpriteSetCenter(&eek_run_locked_highlighted, 0, 0);
  C2D_SpriteSetCenter(&healing_mist_locked_highlighted, 0, 0);
  C2D_SpriteSetCenter(&miracle_sync_locked_highlighted, 0, 0);
  C2D_SpriteSetCenter(&overclock_locked_highlighted, 0, 0);
  C2D_SpriteSetCenter(&ruthless_locked_highlighted, 0, 0);

  C2D_SpriteSetPos(&absolute_accuracy_locked, 2*16, 2*16);
  C2D_SpriteSetPos(&celestial_gift_locked, 5*16, 2*16);
  C2D_SpriteSetPos(&counter_locked, 8*16, 2*16);
  C2D_SpriteSetPos(&divine_intervention_locked, 184, 2*16);
  C2D_SpriteSetPos(&double_or_nothing_locked, 15*16, 2*16);
  C2D_SpriteSetPos(&eek_run_locked, 18*16, 2*16);
  C2D_SpriteSetPos(&healing_mist_locked, 21*16, 2*16);
  C2D_SpriteSetPos(&miracle_sync_locked, 2*16, 4*16);
  C2D_SpriteSetPos(&overclock_locked, 5*16, 4*16);
  C2D_SpriteSetPos(&ruthless_locked, 8*16, 4*16);

  C2D_SpriteSetPos(&absolute_accuracy_locked_highlighted, 2*16, 2*16);
  C2D_SpriteSetPos(&celestial_gift_locked_highlighted, 5*16, 2*16);
  C2D_SpriteSetPos(&counter_locked_highlighted, 8*16, 2*16);
  C2D_SpriteSetPos(&divine_intervention_locked_highlighted, 184, 2*16);
  C2D_SpriteSetPos(&double_or_nothing_locked_highlighted, 15*16, 2*16);
  C2D_SpriteSetPos(&eek_run_locked_highlighted, 18*16, 2*16);
  C2D_SpriteSetPos(&healing_mist_locked_highlighted, 21*16, 2*16);
  C2D_SpriteSetPos(&miracle_sync_locked_highlighted, 2*16, 4*16);
  C2D_SpriteSetPos(&overclock_locked_highlighted, 5*16, 4*16);
  C2D_SpriteSetPos(&ruthless_locked_highlighted, 8*16, 4*16);

  Sprite shop_buttons = load_sprite("romfs:/shop_buttons.t3x");
  if (!shop_buttons.valid) {
    C2D_SpriteSheetFree(ability_shop_bg.sprite_sheet);
    C2D_SpriteSheetFree(ability_icons.sprite_sheet);
    C2D_SpriteSheetFree(ability_info.sprite_sheet);
    C2D_SpriteSheetFree(ability_icons_locked.sprite_sheet);
    C2D_SpriteSheetFree(ability_icons_locked_highlighted.sprite_sheet);
    log_error("ERROR: Failed to load romfs:/shop_buttons.t3x");
    return false;
  }

  enum {
    EQUIP,
    UNEQUIP,
    UNLOCK,
    NOTHING
  };

  C2D_Sprite equip = shop_buttons.sprite;
  C2D_Sprite unequip;
  C2D_Sprite unlock;

  C2D_SpriteFromSheet(&unequip, shop_buttons.sprite_sheet, 1);
  C2D_SpriteFromSheet(&unlock, shop_buttons.sprite_sheet, 2);

  C2D_SpriteSetCenter(&equip, 0, 0);
  C2D_SpriteSetCenter(&unequip, 0, 0);
  C2D_SpriteSetCenter(&unlock, 0, 0);

  C2D_SpriteSetPos(&equip, 6*16, 11*16);
  C2D_SpriteSetPos(&unequip, 6*16, 11*16);
  C2D_SpriteSetPos(&unlock, 6*16, 11*16);

  C2D_Sprite los_buttons[NOTHING];
  los_buttons[EQUIP] = equip;
  los_buttons[UNEQUIP] = unequip;
  los_buttons[UNLOCK] = unlock;

  int current_shop_button = NOTHING;

  C2D_Sprite ability_icons_array[RUTHLESS+1];
  C2D_Sprite ability_icons_locked_array[RUTHLESS+1];

  ability_icons_array[ABSOLUTE_ACCURACY] = absolute_accuracy;
  ability_icons_array[CELESTIAL_GIFT] = celestial_gift;
  ability_icons_array[COUNTER] = counter;
  ability_icons_array[DIVINE_INTERVENTION] = divine_intervention;
  ability_icons_array[DOUBLE_OR_NOTHING] = double_or_nothing;
  ability_icons_array[EEK_RUN] = eek_run;
  ability_icons_array[HEALING_MIST] = healing_mist;
  ability_icons_array[MIRACLE_SYNC] = miracle_sync;
  ability_icons_array[OVERCLOCK] = overclock;
  ability_icons_array[RUTHLESS] = ruthless;

  ability_icons_locked_array[ABSOLUTE_ACCURACY] = absolute_accuracy_locked;
  ability_icons_locked_array[CELESTIAL_GIFT] = celestial_gift_locked;
  ability_icons_locked_array[COUNTER] = counter_locked;
  ability_icons_locked_array[DIVINE_INTERVENTION] = divine_intervention_locked;
  ability_icons_locked_array[DOUBLE_OR_NOTHING] = double_or_nothing_locked;
  ability_icons_locked_array[EEK_RUN] = eek_run_locked;
  ability_icons_locked_array[HEALING_MIST] = healing_mist_locked;
  ability_icons_locked_array[MIRACLE_SYNC] = miracle_sync_locked;
  ability_icons_locked_array[OVERCLOCK] = overclock_locked;
  ability_icons_locked_array[RUTHLESS] = ruthless_locked;

  C2D_Sprite user_ability_icons[RUTHLESS+1];
  C2D_Sprite user_ability_icons_highlighted[RUTHLESS+1];

  if (strcmp(save_data->main.current_pilot, "shinji") == 0) {
    if (save_data->pilots.shinji.unlocked_abilities.absolute_accuracy) {
      user_ability_icons[ABSOLUTE_ACCURACY] = absolute_accuracy;
      user_ability_icons_highlighted[ABSOLUTE_ACCURACY] = absolute_accuracy_highlighted;
    } else {
      user_ability_icons[ABSOLUTE_ACCURACY] = absolute_accuracy_locked;
      user_ability_icons_highlighted[ABSOLUTE_ACCURACY] = absolute_accuracy_locked_highlighted;
    }
    if (save_data->pilots.shinji.unlocked_abilities.celestial_gift) { 
      user_ability_icons[CELESTIAL_GIFT] = celestial_gift; 
      user_ability_icons_highlighted[CELESTIAL_GIFT] = celestial_gift_highlighted;
    } else {
      user_ability_icons[CELESTIAL_GIFT] = celestial_gift_locked;
      user_ability_icons_highlighted[CELESTIAL_GIFT] = celestial_gift_locked_highlighted;
    }

    if (save_data->pilots.shinji.unlocked_abilities.counter) { 
      user_ability_icons[COUNTER] = counter; 
      user_ability_icons_highlighted[COUNTER] = counter_highlighted;
    } else {
      user_ability_icons[COUNTER] = counter_locked;
      user_ability_icons_highlighted[COUNTER] = counter_locked_highlighted;
    }
  
    if (save_data->pilots.shinji.unlocked_abilities.divine_intervention) { 
      user_ability_icons[DIVINE_INTERVENTION] = divine_intervention; 
      user_ability_icons_highlighted[DIVINE_INTERVENTION] = divine_intervention_highlighted;
    } else {
      user_ability_icons[DIVINE_INTERVENTION] = divine_intervention_locked;
      user_ability_icons_highlighted[DIVINE_INTERVENTION] = divine_intervention_locked_highlighted;
    }

    if (save_data->pilots.shinji.unlocked_abilities.double_or_nothing) { 
      user_ability_icons[DOUBLE_OR_NOTHING] = double_or_nothing; 
      user_ability_icons_highlighted[DOUBLE_OR_NOTHING] = double_or_nothing_highlighted;
    } else {
      user_ability_icons[DOUBLE_OR_NOTHING] = double_or_nothing_locked;
      user_ability_icons_highlighted[DOUBLE_OR_NOTHING] = double_or_nothing_locked_highlighted;
    }

    if (save_data->pilots.shinji.unlocked_abilities.eek_run) { 
      user_ability_icons[EEK_RUN] = eek_run; 
      user_ability_icons_highlighted[EEK_RUN] = eek_run_highlighted;
    } else {
      user_ability_icons[EEK_RUN] = eek_run_locked;
      user_ability_icons_highlighted[EEK_RUN] = eek_run_locked_highlighted;
    }

    if (save_data->pilots.shinji.unlocked_abilities.healing_mist) { 
      user_ability_icons[HEALING_MIST] = healing_mist; 
      user_ability_icons_highlighted[HEALING_MIST] = healing_mist_highlighted;
    } else {
      user_ability_icons[HEALING_MIST] = healing_mist_locked;
      user_ability_icons_highlighted[HEALING_MIST] = healing_mist_locked_highlighted;
    }

    if (save_data->pilots.shinji.unlocked_abilities.miracle_sync) { 
      user_ability_icons[MIRACLE_SYNC] = miracle_sync; 
      user_ability_icons_highlighted[MIRACLE_SYNC] = miracle_sync_highlighted;
    } else {
      user_ability_icons[MIRACLE_SYNC] = miracle_sync_locked;
      user_ability_icons_highlighted[MIRACLE_SYNC] = miracle_sync_locked_highlighted;
    }

    if (save_data->pilots.shinji.unlocked_abilities.overclock) { 
      user_ability_icons[OVERCLOCK] = overclock; 
      user_ability_icons_highlighted[OVERCLOCK] = overclock_highlighted;
    } else {
      user_ability_icons[OVERCLOCK] = overclock_locked;
      user_ability_icons_highlighted[OVERCLOCK] = overclock_locked_highlighted;
    }

    if (save_data->pilots.shinji.unlocked_abilities.ruthless) { 
      user_ability_icons[RUTHLESS] = ruthless; 
      user_ability_icons_highlighted[RUTHLESS] = ruthless_highlighted;
    } else {
      user_ability_icons[RUTHLESS] = ruthless_locked;
      user_ability_icons_highlighted[RUTHLESS] = ruthless_locked_highlighted;
    }

  }
  else if (strcmp(save_data->main.current_pilot, "asuka") == 0) {
    if (save_data->pilots.asuka.unlocked_abilities.absolute_accuracy) {
      user_ability_icons[ABSOLUTE_ACCURACY] = absolute_accuracy;
      user_ability_icons_highlighted[ABSOLUTE_ACCURACY] = absolute_accuracy_highlighted;
    } else {
      user_ability_icons[ABSOLUTE_ACCURACY] = absolute_accuracy_locked;
      user_ability_icons_highlighted[ABSOLUTE_ACCURACY] = absolute_accuracy_locked_highlighted;
    }
    if (save_data->pilots.asuka.unlocked_abilities.celestial_gift) { 
      user_ability_icons[CELESTIAL_GIFT] = celestial_gift; 
      user_ability_icons_highlighted[CELESTIAL_GIFT] = celestial_gift_highlighted;
    } else {
      user_ability_icons[CELESTIAL_GIFT] = celestial_gift_locked;
      user_ability_icons_highlighted[CELESTIAL_GIFT] = celestial_gift_locked_highlighted;
    }

    if (save_data->pilots.asuka.unlocked_abilities.counter) { 
      user_ability_icons[COUNTER] = counter; 
      user_ability_icons_highlighted[COUNTER] = counter_highlighted;
    } else {
      user_ability_icons[COUNTER] = counter_locked;
      user_ability_icons_highlighted[COUNTER] = counter_locked_highlighted;
    }
  
    if (save_data->pilots.asuka.unlocked_abilities.divine_intervention) { 
      user_ability_icons[DIVINE_INTERVENTION] = divine_intervention; 
      user_ability_icons_highlighted[DIVINE_INTERVENTION] = divine_intervention_highlighted;
    } else {
      user_ability_icons[DIVINE_INTERVENTION] = divine_intervention_locked;
      user_ability_icons_highlighted[DIVINE_INTERVENTION] = divine_intervention_locked_highlighted;
    }

    if (save_data->pilots.asuka.unlocked_abilities.double_or_nothing) { 
      user_ability_icons[DOUBLE_OR_NOTHING] = double_or_nothing; 
      user_ability_icons_highlighted[DOUBLE_OR_NOTHING] = double_or_nothing_highlighted;
    } else {
      user_ability_icons[DOUBLE_OR_NOTHING] = double_or_nothing_locked;
      user_ability_icons_highlighted[DOUBLE_OR_NOTHING] = double_or_nothing_locked_highlighted;
    }

    if (save_data->pilots.asuka.unlocked_abilities.eek_run) { 
      user_ability_icons[EEK_RUN] = eek_run; 
      user_ability_icons_highlighted[EEK_RUN] = eek_run_highlighted;
    } else {
      user_ability_icons[EEK_RUN] = eek_run_locked;
      user_ability_icons_highlighted[EEK_RUN] = eek_run_locked_highlighted;
    }

    if (save_data->pilots.asuka.unlocked_abilities.healing_mist) { 
      user_ability_icons[HEALING_MIST] = healing_mist; 
      user_ability_icons_highlighted[HEALING_MIST] = healing_mist_highlighted;
    } else {
      user_ability_icons[HEALING_MIST] = healing_mist_locked;
      user_ability_icons_highlighted[HEALING_MIST] = healing_mist_locked_highlighted;
    }

    if (save_data->pilots.asuka.unlocked_abilities.miracle_sync) { 
      user_ability_icons[MIRACLE_SYNC] = miracle_sync; 
      user_ability_icons_highlighted[MIRACLE_SYNC] = miracle_sync_highlighted;
    } else {
      user_ability_icons[MIRACLE_SYNC] = miracle_sync_locked;
      user_ability_icons_highlighted[MIRACLE_SYNC] = miracle_sync_locked_highlighted;
    }

    if (save_data->pilots.asuka.unlocked_abilities.overclock) { 
      user_ability_icons[OVERCLOCK] = overclock; 
      user_ability_icons_highlighted[OVERCLOCK] = overclock_highlighted;
    } else {
      user_ability_icons[OVERCLOCK] = overclock_locked;
      user_ability_icons_highlighted[OVERCLOCK] = overclock_locked_highlighted;
    }

    if (save_data->pilots.asuka.unlocked_abilities.ruthless) { 
      user_ability_icons[RUTHLESS] = ruthless; 
      user_ability_icons_highlighted[RUTHLESS] = ruthless_highlighted;
    } else {
      user_ability_icons[RUTHLESS] = ruthless_locked;
      user_ability_icons_highlighted[RUTHLESS] = ruthless_locked_highlighted;
    }
  }
  else if (strcmp(save_data->main.current_pilot, "rei") == 0) {
    if (save_data->pilots.rei.unlocked_abilities.absolute_accuracy) {
      user_ability_icons[ABSOLUTE_ACCURACY] = absolute_accuracy;
      user_ability_icons_highlighted[ABSOLUTE_ACCURACY] = absolute_accuracy_highlighted;
    } else {
      user_ability_icons[ABSOLUTE_ACCURACY] = absolute_accuracy_locked;
      user_ability_icons_highlighted[ABSOLUTE_ACCURACY] = absolute_accuracy_locked_highlighted;
    }
    if (save_data->pilots.rei.unlocked_abilities.celestial_gift) { 
      user_ability_icons[CELESTIAL_GIFT] = celestial_gift; 
      user_ability_icons_highlighted[CELESTIAL_GIFT] = celestial_gift_highlighted;
    } else {
      user_ability_icons[CELESTIAL_GIFT] = celestial_gift_locked;
      user_ability_icons_highlighted[CELESTIAL_GIFT] = celestial_gift_locked_highlighted;
    }

    if (save_data->pilots.rei.unlocked_abilities.counter) { 
      user_ability_icons[COUNTER] = counter; 
      user_ability_icons_highlighted[COUNTER] = counter_highlighted;
    } else {
      user_ability_icons[COUNTER] = counter_locked;
      user_ability_icons_highlighted[COUNTER] = counter_locked_highlighted;
    }
  
    if (save_data->pilots.rei.unlocked_abilities.divine_intervention) { 
      user_ability_icons[DIVINE_INTERVENTION] = divine_intervention; 
      user_ability_icons_highlighted[DIVINE_INTERVENTION] = divine_intervention_highlighted;
    } else {
      user_ability_icons[DIVINE_INTERVENTION] = divine_intervention_locked;
      user_ability_icons_highlighted[DIVINE_INTERVENTION] = divine_intervention_locked_highlighted;
    }

    if (save_data->pilots.rei.unlocked_abilities.double_or_nothing) { 
      user_ability_icons[DOUBLE_OR_NOTHING] = double_or_nothing; 
      user_ability_icons_highlighted[DOUBLE_OR_NOTHING] = double_or_nothing_highlighted;
    } else {
      user_ability_icons[DOUBLE_OR_NOTHING] = double_or_nothing_locked;
      user_ability_icons_highlighted[DOUBLE_OR_NOTHING] = double_or_nothing_locked_highlighted;
    }

    if (save_data->pilots.rei.unlocked_abilities.eek_run) { 
      user_ability_icons[EEK_RUN] = eek_run; 
      user_ability_icons_highlighted[EEK_RUN] = eek_run_highlighted;
    } else {
      user_ability_icons[EEK_RUN] = eek_run_locked;
      user_ability_icons_highlighted[EEK_RUN] = eek_run_locked_highlighted;
    }

    if (save_data->pilots.rei.unlocked_abilities.healing_mist) { 
      user_ability_icons[HEALING_MIST] = healing_mist; 
      user_ability_icons_highlighted[HEALING_MIST] = healing_mist_highlighted;
    } else {
      user_ability_icons[HEALING_MIST] = healing_mist_locked;
      user_ability_icons_highlighted[HEALING_MIST] = healing_mist_locked_highlighted;
    }

    if (save_data->pilots.rei.unlocked_abilities.miracle_sync) { 
      user_ability_icons[MIRACLE_SYNC] = miracle_sync; 
      user_ability_icons_highlighted[MIRACLE_SYNC] = miracle_sync_highlighted;
    } else {
      user_ability_icons[MIRACLE_SYNC] = miracle_sync_locked;
      user_ability_icons_highlighted[MIRACLE_SYNC] = miracle_sync_locked_highlighted;
    }

    if (save_data->pilots.rei.unlocked_abilities.overclock) { 
      user_ability_icons[OVERCLOCK] = overclock; 
      user_ability_icons_highlighted[OVERCLOCK] = overclock_highlighted;
    } else {
      user_ability_icons[OVERCLOCK] = overclock_locked;
      user_ability_icons_highlighted[OVERCLOCK] = overclock_locked_highlighted;
    }

    if (save_data->pilots.rei.unlocked_abilities.ruthless) { 
      user_ability_icons[RUTHLESS] = ruthless; 
      user_ability_icons_highlighted[RUTHLESS] = ruthless_highlighted;
    } else {
      user_ability_icons[RUTHLESS] = ruthless_locked;
      user_ability_icons_highlighted[RUTHLESS] = ruthless_locked_highlighted;
    }
  }

  int current_ability = ABSOLUTE_ACCURACY;
  if (input_button == ABILITIES) {
    if (user_ability_icons[current_ability].image.subtex == ability_icons_array[current_ability].image.subtex) {
      current_shop_button = NOTHING;
    } else {
      current_shop_button = UNLOCK;
    }
  }
  
  Sprite popup = load_sprite("romfs:/popup.t3x");
  if (!popup.valid) {
    C2D_SpriteSheetFree(ability_shop_bg.sprite_sheet);
    C2D_SpriteSheetFree(ability_icons.sprite_sheet);
    C2D_SpriteSheetFree(ability_info.sprite_sheet);
    C2D_SpriteSheetFree(ability_icons_locked.sprite_sheet);
    C2D_SpriteSheetFree(ability_icons_locked_highlighted.sprite_sheet);
    C2D_SpriteSheetFree(shop_buttons.sprite_sheet);
    log_error("ERROR: Failed to load romfs:/popup.t3x");
    return false;
  }

  C2D_SpriteSetCenter(&popup.sprite, 0, 0);
  C2D_SpriteSetPos(&popup.sprite, 7*16, 4*16);

  Sprite insufficiant_funds = load_sprite("romfs:/insufficiant_funds.t3x");
  if (!insufficiant_funds.valid) {
    C2D_SpriteSheetFree(ability_shop_bg.sprite_sheet);
    C2D_SpriteSheetFree(ability_icons.sprite_sheet);
    C2D_SpriteSheetFree(ability_info.sprite_sheet);
    C2D_SpriteSheetFree(ability_icons_locked.sprite_sheet);
    C2D_SpriteSheetFree(ability_icons_locked_highlighted.sprite_sheet);
    C2D_SpriteSheetFree(shop_buttons.sprite_sheet);
    C2D_SpriteSheetFree(popup.sprite_sheet);
    log_error("ERROR: Failed to load romfs:/insufficiant_funds.t3x");
    return false;
  };
  C2D_SpriteSetCenter(&insufficiant_funds.sprite, 0, 0);
  C2D_SpriteSetPos(&insufficiant_funds.sprite, 7*16, 4*16);

  int level_points = save_data->main.level_points;
  bool level_points_changed = false;

  char* buf = malloc(sizeof(char)*15);
  Text level_points_text = load_text(strcat(itoa(level_points, buf, 10), " LP"));
  free(buf);
  float font_scale = 1.0;

  Text unlock_text = load_text("Unlock for");
  Text price_text = load_text("Placeholder");

  while (aptMainLoop()) {
    hidScanInput();
    u32 key_pressed = hidKeysDown();
    if (key_pressed & KEY_DLEFT) {
      if (current_ability != ABSOLUTE_ACCURACY) {
        current_ability--;
        if (input_button == ABILITIES) {
          if (user_ability_icons[current_ability].image.subtex == ability_icons_array[current_ability].image.subtex) {
            current_shop_button = NOTHING;
          } else {
            current_shop_button = UNLOCK;
          }
        } else {
          if (user_ability_icons[current_ability].image.subtex == ability_icons_locked_array[current_ability].image.subtex) {
            current_shop_button = UNLOCK;
          } else if (strcmp(save_data->main.current_pilot, "shinji") == 0) {
            if (strcmp(abilities_string_array[current_ability], save_data->pilots.shinji.active_abilities[input_button]) == 0) {
              current_shop_button = UNEQUIP;
            } else {
              bool equiped = false;
              for (int i = 0; i < 5; i++) {
                if (strcmp(abilities_string_array[current_ability], save_data->pilots.shinji.active_abilities[i]) == 0) {
                  equiped = true;
                  break;
                }
              }
              if (equiped) { current_shop_button = NOTHING; }
              else { current_shop_button = EQUIP; }
            }
          } else if (strcmp(save_data->main.current_pilot, "asuka") == 0) {
            if (strcmp(abilities_string_array[current_ability], save_data->pilots.asuka.active_abilities[input_button]) == 0) {
              current_shop_button = UNEQUIP;
            } else {
              bool equiped = false;
              for (int i = 0; i < 5; i++) {
                if (strcmp(abilities_string_array[current_ability], save_data->pilots.asuka.active_abilities[i]) == 0) {
                  equiped = true;
                  break;
                }
              }
              if (equiped) { current_shop_button = NOTHING; }
              else { current_shop_button = EQUIP; }
            }
          } else if (strcmp(save_data->main.current_pilot, "rei") == 0) {
            if (strcmp(abilities_string_array[current_ability], save_data->pilots.rei.active_abilities[input_button]) == 0) {
              current_shop_button = UNEQUIP;
            } else {
              bool equiped = false;
              for (int i = 0; i < 5; i++) {
                if (strcmp(abilities_string_array[current_ability], save_data->pilots.rei.active_abilities[i]) == 0) {
                  equiped = true;
                  break;
                }
              }
              if (equiped) { current_shop_button = NOTHING; }
              else { current_shop_button = EQUIP; }
            }
          } 
        }
      }
    } else if (key_pressed & KEY_DRIGHT) {
      if (current_ability != RUTHLESS) {
        current_ability++;
        if (input_button == ABILITIES) {
          if (user_ability_icons[current_ability].image.subtex == ability_icons_array[current_ability].image.subtex) {
            current_shop_button = NOTHING;
          } else {
            current_shop_button = UNLOCK;
          }
        } else {
          if (user_ability_icons[current_ability].image.subtex == ability_icons_locked_array[current_ability].image.subtex) {
            current_shop_button = UNLOCK;
          } else if (strcmp(save_data->main.current_pilot, "shinji") == 0) {
            if (strcmp(abilities_string_array[current_ability], save_data->pilots.shinji.active_abilities[input_button]) == 0) {
              current_shop_button = UNEQUIP;
            } else {
              bool equiped = false;
              for (int i = 0; i < 5; i++) {
                if (strcmp(abilities_string_array[current_ability], save_data->pilots.shinji.active_abilities[i]) == 0) {
                  equiped = true;
                  break;
                }
              }
              if (equiped) { current_shop_button = NOTHING; }
              else { current_shop_button = EQUIP; }
            }
          } else if (strcmp(save_data->main.current_pilot, "asuka") == 0) {
            if (strcmp(abilities_string_array[current_ability], save_data->pilots.asuka.active_abilities[input_button]) == 0) {
              current_shop_button = UNEQUIP;
            } else {
              bool equiped = false;
              for (int i = 0; i < 5; i++) {
                if (strcmp(abilities_string_array[current_ability], save_data->pilots.asuka.active_abilities[i]) == 0) {
                  equiped = true;
                  break;
                }
              }
              if (equiped) { current_shop_button = NOTHING; }
              else { current_shop_button = EQUIP; }
            }
          } else if (strcmp(save_data->main.current_pilot, "rei") == 0) {
            if (strcmp(abilities_string_array[current_ability], save_data->pilots.rei.active_abilities[input_button]) == 0) {
              current_shop_button = UNEQUIP;
            } else {
              bool equiped = false;
              for (int i = 0; i < 5; i++) {
                if (strcmp(abilities_string_array[current_ability], save_data->pilots.rei.active_abilities[i]) == 0) {
                  equiped = true;
                  break;
                }
              }
              if (equiped) { current_shop_button = NOTHING; }
              else { current_shop_button = EQUIP; }
            }
          } 
        }
      }
    } else if (key_pressed & KEY_B) {
      break;
    } else if (key_pressed & KEY_A) {
      if (current_shop_button != NOTHING) {
        if (current_shop_button == UNLOCK) {
          int price = 0;
          if (current_ability == ABSOLUTE_ACCURACY) { price = 2500; }
          else if (current_ability == CELESTIAL_GIFT) { price = 2000; }
          else if (current_ability == COUNTER) { price = 1250; }
          else if (current_ability == DIVINE_INTERVENTION) { price = 2500; }
          else if (current_ability == DOUBLE_OR_NOTHING) { price = 750; }
          else if (current_ability == EEK_RUN) { price = 750; }
          else if (current_ability == HEALING_MIST) { price = 1750; }
          else if (current_ability == MIRACLE_SYNC) { price = 1500; }
          else if (current_ability == OVERCLOCK) { price = 1250; }
          else if (current_ability == RUTHLESS) { price = 1750; }
          
          char* buf = malloc(sizeof(char)*15);
          C2D_TextBufClear(price_text.buffer);
          price_text = load_text(strcat(itoa(price, buf, 10), " LP?"));
          free(buf);
          while (true) {
            C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
            C2D_TargetClear(top_screen, colour_palette.background);
            C2D_SceneBegin(top_screen);
            C2D_DrawSprite(&popup.sprite);
            C2D_DrawText(&unlock_text.text, C2D_WithColor, 9*16, 5*16, 0, font_scale, font_scale, colour_palette.white);
            C2D_DrawText(&price_text.text, C2D_WithColor, 9*16, 6*16, 0, font_scale, font_scale, colour_palette.white);

            C3D_FrameEnd(0);
            
            hidScanInput();
            key_pressed = hidKeysDown();

            if (key_pressed & KEY_B) { break; }
            else if (key_pressed & KEY_A) {
              if (level_points - price >= 0) {
                if (strcmp(save_data->main.current_pilot, "shinji") == 0) {
                  if (strcmp(abilities_string_array[current_ability], abilities_string_array[ABSOLUTE_ACCURACY]) == 0) {
                    save_data->pilots.shinji.unlocked_abilities.absolute_accuracy = true;
                    if (input_button == ABILITIES) {
                      current_shop_button = NOTHING;
                    } else { current_shop_button = EQUIP; }
                    user_ability_icons[ABSOLUTE_ACCURACY] = absolute_accuracy;
                    user_ability_icons_highlighted[ABSOLUTE_ACCURACY] = absolute_accuracy_highlighted;
                    save_data->main.level_points -= price;
                    level_points -= price;
                    level_points_changed = true;
                  }
                  else if (strcmp(abilities_string_array[current_ability], abilities_string_array[CELESTIAL_GIFT]) == 0) {
                    save_data->pilots.shinji.unlocked_abilities.celestial_gift = true;
                    if (input_button == ABILITIES) {
                      current_shop_button = NOTHING;
                    } else { current_shop_button = EQUIP; }
                    user_ability_icons[CELESTIAL_GIFT] = celestial_gift;
                    user_ability_icons_highlighted[CELESTIAL_GIFT] = celestial_gift_highlighted;
                    save_data->main.level_points -= price;
                    level_points -= price;
                    level_points_changed = true;
                  }
                  else if (strcmp(abilities_string_array[current_ability], abilities_string_array[COUNTER]) == 0) {
                    save_data->pilots.shinji.unlocked_abilities.counter = true;
                    if (input_button == ABILITIES) {
                      current_shop_button = NOTHING;
                    } else { current_shop_button = EQUIP; }
                    user_ability_icons[COUNTER] = counter;
                    user_ability_icons_highlighted[COUNTER] = counter_highlighted;
                    save_data->main.level_points -= price;
                    level_points -= price;
                    level_points_changed = true;
                  }
                  else if (strcmp(abilities_string_array[current_ability], abilities_string_array[DIVINE_INTERVENTION]) == 0) {
                    save_data->pilots.shinji.unlocked_abilities.divine_intervention = true;
                    if (input_button == ABILITIES) {
                      current_shop_button = NOTHING;
                    } else { current_shop_button = EQUIP; }
                    user_ability_icons[DIVINE_INTERVENTION] = divine_intervention;
                    user_ability_icons_highlighted[DIVINE_INTERVENTION] = divine_intervention_highlighted;
                    save_data->main.level_points -= price;
                    level_points -= price;
                    level_points_changed = true;
                  }
                  else if (strcmp(abilities_string_array[current_ability], abilities_string_array[DOUBLE_OR_NOTHING]) == 0) {
                    save_data->pilots.shinji.unlocked_abilities.double_or_nothing = true;
                    if (input_button == ABILITIES) {
                      current_shop_button = NOTHING;
                    } else { current_shop_button = EQUIP; }
                    user_ability_icons[DOUBLE_OR_NOTHING] = double_or_nothing;
                    user_ability_icons_highlighted[DOUBLE_OR_NOTHING] = double_or_nothing_highlighted;
                    save_data->main.level_points -= price;
                    level_points -= price;
                    level_points_changed = true;
                  }
                  else if (strcmp(abilities_string_array[current_ability], abilities_string_array[EEK_RUN]) == 0) {
                    save_data->pilots.shinji.unlocked_abilities.eek_run = true;
                    if (input_button == ABILITIES) {
                      current_shop_button = NOTHING;
                    } else { current_shop_button = EQUIP; }
                    user_ability_icons[EEK_RUN] = eek_run;
                    user_ability_icons_highlighted[EEK_RUN] = eek_run_highlighted;
                    save_data->main.level_points -= price;
                    level_points -= price;
                    level_points_changed = true;
                  }
                  else if (strcmp(abilities_string_array[current_ability], abilities_string_array[HEALING_MIST]) == 0) {
                    save_data->pilots.shinji.unlocked_abilities.healing_mist = true;
                    if (input_button == ABILITIES) {
                      current_shop_button = NOTHING;
                    } else { current_shop_button = EQUIP; }
                    user_ability_icons[HEALING_MIST] = healing_mist;
                    user_ability_icons_highlighted[HEALING_MIST] = healing_mist_highlighted;
                    save_data->main.level_points -= price;
                    level_points -= price;
                    level_points_changed = true;
                  }
                  else if (strcmp(abilities_string_array[current_ability], abilities_string_array[MIRACLE_SYNC]) == 0) {
                    save_data->pilots.shinji.unlocked_abilities.miracle_sync = true;
                    if (input_button == ABILITIES) {
                      current_shop_button = NOTHING;
                    } else { current_shop_button = EQUIP; }
                    user_ability_icons[MIRACLE_SYNC] = miracle_sync;
                    user_ability_icons_highlighted[MIRACLE_SYNC] = miracle_sync_highlighted;
                    save_data->main.level_points -= price;
                    level_points -= price;
                    level_points_changed = true;
                  }
                  else if (strcmp(abilities_string_array[current_ability], abilities_string_array[OVERCLOCK]) == 0) {
                    save_data->pilots.shinji.unlocked_abilities.overclock = true;
                    if (input_button == ABILITIES) {
                      current_shop_button = NOTHING;
                    } else { current_shop_button = EQUIP; }
                    user_ability_icons[OVERCLOCK] = overclock;
                    user_ability_icons_highlighted[OVERCLOCK] = overclock_highlighted;
                    save_data->main.level_points -= price;
                    level_points -= price;
                    level_points_changed = true;
                  }
                  else if (strcmp(abilities_string_array[current_ability], abilities_string_array[RUTHLESS]) == 0) {
                    save_data->pilots.shinji.unlocked_abilities.ruthless = true;
                    if (input_button == ABILITIES) {
                      current_shop_button = NOTHING;
                    } else { current_shop_button = EQUIP; }
                    user_ability_icons[RUTHLESS] = ruthless;
                    user_ability_icons_highlighted[RUTHLESS] = ruthless_highlighted;
                    save_data->main.level_points -= price;
                    level_points -= price;
                    level_points_changed = true;
                  }
                }
                else if (strcmp(save_data->main.current_pilot, "asuka") == 0) {
                  if (strcmp(abilities_string_array[current_ability], abilities_string_array[ABSOLUTE_ACCURACY]) == 0) {
                    save_data->pilots.asuka.unlocked_abilities.absolute_accuracy = true;
                    if (input_button == ABILITIES) {
                      current_shop_button = NOTHING;
                    } else { current_shop_button = EQUIP; }
                    user_ability_icons[ABSOLUTE_ACCURACY] = absolute_accuracy;
                    user_ability_icons_highlighted[ABSOLUTE_ACCURACY] = absolute_accuracy_highlighted;
                    save_data->main.level_points -= price;
                    level_points -= price;
                    level_points_changed = true;
                  }
                  else if (strcmp(abilities_string_array[current_ability], abilities_string_array[CELESTIAL_GIFT]) == 0) {
                    save_data->pilots.asuka.unlocked_abilities.celestial_gift = true;
                    if (input_button == ABILITIES) {
                      current_shop_button = NOTHING;
                    } else { current_shop_button = EQUIP; }
                    user_ability_icons[CELESTIAL_GIFT] = celestial_gift;
                    user_ability_icons_highlighted[CELESTIAL_GIFT] = celestial_gift_highlighted;
                    save_data->main.level_points -= price;
                    level_points -= price;
                    level_points_changed = true;
                  }
                  else if (strcmp(abilities_string_array[current_ability], abilities_string_array[COUNTER]) == 0) {
                    save_data->pilots.asuka.unlocked_abilities.counter = true;
                    if (input_button == ABILITIES) {
                      current_shop_button = NOTHING;
                    } else { current_shop_button = EQUIP; }
                    user_ability_icons[COUNTER] = counter;
                    user_ability_icons_highlighted[COUNTER] = counter_highlighted;
                    save_data->main.level_points -= price;
                    level_points -= price;
                    level_points_changed = true;
                  }
                  else if (strcmp(abilities_string_array[current_ability], abilities_string_array[DIVINE_INTERVENTION]) == 0) {
                    save_data->pilots.asuka.unlocked_abilities.divine_intervention = true;
                    if (input_button == ABILITIES) {
                      current_shop_button = NOTHING;
                    } else { current_shop_button = EQUIP; }
                    user_ability_icons[DIVINE_INTERVENTION] = divine_intervention;
                    user_ability_icons_highlighted[DIVINE_INTERVENTION] = divine_intervention_highlighted;
                    save_data->main.level_points -= price;
                    level_points -= price;
                    level_points_changed = true;
                  }
                  else if (strcmp(abilities_string_array[current_ability], abilities_string_array[DOUBLE_OR_NOTHING]) == 0) {
                    save_data->pilots.asuka.unlocked_abilities.double_or_nothing = true;
                    if (input_button == ABILITIES) {
                      current_shop_button = NOTHING;
                    } else { current_shop_button = EQUIP; }
                    user_ability_icons[DOUBLE_OR_NOTHING] = double_or_nothing;
                    user_ability_icons_highlighted[DOUBLE_OR_NOTHING] = double_or_nothing_highlighted;
                    save_data->main.level_points -= price;
                    level_points -= price;
                    level_points_changed = true;
                  }
                  else if (strcmp(abilities_string_array[current_ability], abilities_string_array[EEK_RUN]) == 0) {
                    save_data->pilots.asuka.unlocked_abilities.eek_run = true;
                    if (input_button == ABILITIES) {
                      current_shop_button = NOTHING;
                    } else { current_shop_button = EQUIP; }
                    user_ability_icons[EEK_RUN] = eek_run;
                    user_ability_icons_highlighted[EEK_RUN] = eek_run_highlighted;
                    save_data->main.level_points -= price;
                    level_points -= price;
                    level_points_changed = true;
                  }
                  else if (strcmp(abilities_string_array[current_ability], abilities_string_array[HEALING_MIST]) == 0) {
                    save_data->pilots.asuka.unlocked_abilities.healing_mist = true;
                    if (input_button == ABILITIES) {
                      current_shop_button = NOTHING;
                    } else { current_shop_button = EQUIP; }
                    user_ability_icons[HEALING_MIST] = healing_mist;
                    user_ability_icons_highlighted[HEALING_MIST] = healing_mist_highlighted;
                    save_data->main.level_points -= price;
                    level_points -= price;
                    level_points_changed = true;
                  }
                  else if (strcmp(abilities_string_array[current_ability], abilities_string_array[MIRACLE_SYNC]) == 0) {
                    save_data->pilots.asuka.unlocked_abilities.miracle_sync = true;
                    if (input_button == ABILITIES) {
                      current_shop_button = NOTHING;
                    } else { current_shop_button = EQUIP; }
                    user_ability_icons[MIRACLE_SYNC] = miracle_sync;
                    user_ability_icons_highlighted[MIRACLE_SYNC] = miracle_sync_highlighted;
                    save_data->main.level_points -= price;
                    level_points -= price;
                    level_points_changed = true;
                  }
                  else if (strcmp(abilities_string_array[current_ability], abilities_string_array[OVERCLOCK]) == 0) {
                    save_data->pilots.asuka.unlocked_abilities.overclock = true;
                    if (input_button == ABILITIES) {
                      current_shop_button = NOTHING;
                    } else { current_shop_button = EQUIP; }
                    user_ability_icons[OVERCLOCK] = overclock;
                    user_ability_icons_highlighted[OVERCLOCK] = overclock_highlighted;
                    save_data->main.level_points -= price;
                    level_points -= price;
                    level_points_changed = true;
                  }
                  else if (strcmp(abilities_string_array[current_ability], abilities_string_array[RUTHLESS]) == 0) {
                    save_data->pilots.asuka.unlocked_abilities.ruthless = true;
                    if (input_button == ABILITIES) {
                      current_shop_button = NOTHING;
                    } else { current_shop_button = EQUIP; }
                    user_ability_icons[RUTHLESS] = ruthless;
                    user_ability_icons_highlighted[RUTHLESS] = ruthless_highlighted;
                    save_data->main.level_points -= price;
                    level_points -= price;
                    level_points_changed = true;
                  }
                }
                else if (strcmp(save_data->main.current_pilot, "rei") == 0) {
                  if (strcmp(abilities_string_array[current_ability], abilities_string_array[ABSOLUTE_ACCURACY]) == 0) {
                    save_data->pilots.rei.unlocked_abilities.absolute_accuracy = true;
                    if (input_button == ABILITIES) {
                      current_shop_button = NOTHING;
                    } else { current_shop_button = EQUIP; }
                    user_ability_icons[ABSOLUTE_ACCURACY] = absolute_accuracy;
                    user_ability_icons_highlighted[ABSOLUTE_ACCURACY] = absolute_accuracy_highlighted;
                    save_data->main.level_points -= price;
                    level_points -= price;
                    level_points_changed = true;
                  }
                  else if (strcmp(abilities_string_array[current_ability], abilities_string_array[CELESTIAL_GIFT]) == 0) {
                    save_data->pilots.rei.unlocked_abilities.celestial_gift = true;
                    if (input_button == ABILITIES) {
                      current_shop_button = NOTHING;
                    } else { current_shop_button = EQUIP; }
                    user_ability_icons[CELESTIAL_GIFT] = celestial_gift;
                    user_ability_icons_highlighted[CELESTIAL_GIFT] = celestial_gift_highlighted;
                    save_data->main.level_points -= price;
                    level_points -= price;
                    level_points_changed = true;
                  }
                  else if (strcmp(abilities_string_array[current_ability], abilities_string_array[COUNTER]) == 0) {
                    save_data->pilots.rei.unlocked_abilities.counter = true;
                    if (input_button == ABILITIES) {
                      current_shop_button = NOTHING;
                    } else { current_shop_button = EQUIP; }
                    user_ability_icons[COUNTER] = counter;
                    user_ability_icons_highlighted[COUNTER] = counter_highlighted;
                    save_data->main.level_points -= price;
                    level_points -= price;
                    level_points_changed = true;
                  }
                  else if (strcmp(abilities_string_array[current_ability], abilities_string_array[DIVINE_INTERVENTION]) == 0) {
                    save_data->pilots.rei.unlocked_abilities.divine_intervention = true;
                    if (input_button == ABILITIES) {
                      current_shop_button = NOTHING;
                    } else { current_shop_button = EQUIP; }
                    user_ability_icons[DIVINE_INTERVENTION] = divine_intervention;
                    user_ability_icons_highlighted[DIVINE_INTERVENTION] = divine_intervention_highlighted;
                    save_data->main.level_points -= price;
                    level_points -= price;
                    level_points_changed = true;
                  }
                  else if (strcmp(abilities_string_array[current_ability], abilities_string_array[DOUBLE_OR_NOTHING]) == 0) {
                    save_data->pilots.rei.unlocked_abilities.double_or_nothing = true;
                    if (input_button == ABILITIES) {
                      current_shop_button = NOTHING;
                    } else { current_shop_button = EQUIP; }
                    user_ability_icons[DOUBLE_OR_NOTHING] = double_or_nothing;
                    user_ability_icons_highlighted[DOUBLE_OR_NOTHING] = double_or_nothing_highlighted;
                    save_data->main.level_points -= price;
                    level_points -= price;
                    level_points_changed = true;
                  }
                  else if (strcmp(abilities_string_array[current_ability], abilities_string_array[EEK_RUN]) == 0) {
                    save_data->pilots.rei.unlocked_abilities.eek_run = true;
                    if (input_button == ABILITIES) {
                      current_shop_button = NOTHING;
                    } else { current_shop_button = EQUIP; }
                    user_ability_icons[EEK_RUN] = eek_run;
                    user_ability_icons_highlighted[EEK_RUN] = eek_run_highlighted;
                    save_data->main.level_points -= price;
                    level_points -= price;
                    level_points_changed = true;
                  }
                  else if (strcmp(abilities_string_array[current_ability], abilities_string_array[HEALING_MIST]) == 0) {
                    save_data->pilots.rei.unlocked_abilities.healing_mist = true;
                    if (input_button == ABILITIES) {
                      current_shop_button = NOTHING;
                    } else { current_shop_button = EQUIP; }
                    user_ability_icons[HEALING_MIST] = healing_mist;
                    user_ability_icons_highlighted[HEALING_MIST] = healing_mist_highlighted;
                    save_data->main.level_points -= price;
                    level_points -= price;
                    level_points_changed = true;
                  }
                  else if (strcmp(abilities_string_array[current_ability], abilities_string_array[MIRACLE_SYNC]) == 0) {
                    save_data->pilots.rei.unlocked_abilities.miracle_sync = true;
                    if (input_button == ABILITIES) {
                      current_shop_button = NOTHING;
                    } else { current_shop_button = EQUIP; }
                    user_ability_icons[MIRACLE_SYNC] = miracle_sync;
                    user_ability_icons_highlighted[MIRACLE_SYNC] = miracle_sync_highlighted;
                    save_data->main.level_points -= price;
                    level_points -= price;
                    level_points_changed = true;
                  }
                  else if (strcmp(abilities_string_array[current_ability], abilities_string_array[OVERCLOCK]) == 0) {
                    save_data->pilots.rei.unlocked_abilities.overclock = true;
                    if (input_button == ABILITIES) {
                      current_shop_button = NOTHING;
                    } else { current_shop_button = EQUIP; }
                    user_ability_icons[OVERCLOCK] = overclock;
                    user_ability_icons_highlighted[OVERCLOCK] = overclock_highlighted;
                    save_data->main.level_points -= price;
                    level_points -= price;
                    level_points_changed = true;
                  }
                  else if (strcmp(abilities_string_array[current_ability], abilities_string_array[RUTHLESS]) == 0) {
                    save_data->pilots.rei.unlocked_abilities.ruthless = true;
                    if (input_button == ABILITIES) {
                      current_shop_button = NOTHING;
                    } else { current_shop_button = EQUIP; }
                    user_ability_icons[RUTHLESS] = ruthless;
                    user_ability_icons_highlighted[RUTHLESS] = ruthless_highlighted;
                    save_data->main.level_points -= price;
                    level_points -= price;
                    level_points_changed = true;
                  }
                }
              break;
              } else {
                while (true) {
                  C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
                  C2D_TargetClear(top_screen, colour_palette.background);
                  C2D_SceneBegin(top_screen);
                  C2D_DrawSprite(&insufficiant_funds.sprite);
                  C3D_FrameEnd(0);

                  hidScanInput();
                  key_pressed = hidKeysDown();

                  if (key_pressed & KEY_A) { break; }
                }
              break;
              }
            }
          }
        } else if (current_shop_button == EQUIP) {
          if (strcmp(save_data->main.current_pilot, "shinji") == 0) {
            save_data->pilots.shinji.active_abilities[input_button] = abilities_string_array[current_ability];
            current_shop_button = UNEQUIP;
          } else if (strcmp(save_data->main.current_pilot, "asuka") == 0) {
            save_data->pilots.asuka.active_abilities[input_button] = abilities_string_array[current_ability];
            current_shop_button = UNEQUIP;
          } else if (strcmp(save_data->main.current_pilot, "rei") == 0) {
            save_data->pilots.rei.active_abilities[input_button] = abilities_string_array[current_ability];
            current_shop_button = UNEQUIP;
          }
        } else if (current_shop_button == UNEQUIP) {
          if (strcmp(save_data->main.current_pilot, "shinji") == 0) {
            save_data->pilots.shinji.active_abilities[input_button] = "null";
            current_shop_button = EQUIP;
          } else if (strcmp(save_data->main.current_pilot, "asuka") == 0) {
            save_data->pilots.asuka.active_abilities[input_button] = "null";
            current_shop_button = EQUIP;
          } else if (strcmp(save_data->main.current_pilot, "rei") == 0) {
            save_data->pilots.rei.active_abilities[input_button] = "null";
            current_shop_button = EQUIP;
          }
        }
      }
    }

    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    C2D_TargetClear(top_screen, colour_palette.background);
    C2D_TargetClear(bottom_screen, colour_palette.background);

    
    C2D_SceneBegin(top_screen);

    for (int i = 0; i < RUTHLESS+1; i++) {
      if (i == current_ability) {
        C2D_DrawSprite(&user_ability_icons_highlighted[current_ability]);
      } else {
        C2D_DrawSprite(&user_ability_icons[i]);
      }
    }

    C2D_SceneBegin(bottom_screen);

    C2D_DrawSprite(&ability_shop_bg.sprite);

    if (current_shop_button != NOTHING) {
      C2D_DrawSprite(&los_buttons[current_shop_button]);
    }

    if (level_points_changed) {
      C2D_TextBufClear(level_points_text.buffer);
      char* buf = malloc(sizeof(char)*15);
      level_points_text = load_text(strcat(itoa(level_points, buf, 10), " LP"));
      free(buf);
      level_points_changed = false;
    }
    C2D_DrawText(&level_points_text.text, C2D_WithColor, 35, 0, 0, font_scale, font_scale, colour_palette.white);

    C2D_DrawSprite(&info[current_ability]);

    
    C3D_FrameEnd(0);

    
  }

  C2D_SpriteSheetFree(ability_shop_bg.sprite_sheet);
  C2D_SpriteSheetFree(ability_icons.sprite_sheet);
  C2D_SpriteSheetFree(ability_info.sprite_sheet);
  C2D_SpriteSheetFree(ability_icons_locked.sprite_sheet);
  C2D_SpriteSheetFree(ability_icons_locked_highlighted.sprite_sheet);
  C2D_SpriteSheetFree(shop_buttons.sprite_sheet);
  C2D_SpriteSheetFree(popup.sprite_sheet);
  C2D_TextBufDelete(level_points_text.buffer);
  C2D_TextBufDelete(unlock_text.buffer);
  C2D_SpriteSheetFree(insufficiant_funds.sprite_sheet);
  C2D_TextBufDelete(price_text.buffer);
  return true;
}

bool skins_menu(SaveFile* save_data, C3D_RenderTarget* top_screen, C3D_RenderTarget* bottom_screen, Colours colour_palette) {
  Sprite shinji_sheet = load_sprite("romfs:/shinji_selection.t3x");
  if (!shinji_sheet.valid) {
    log_error("ERROR: Failed to load romfs:/shinji_selection.t3x");
    return false;
  }

  Sprite rei_sheet = load_sprite("romfs:/rei_selection.t3x");
  if (!rei_sheet.valid) {
    C2D_SpriteSheetFree(shinji_sheet.sprite_sheet);
    log_error("ERROR: Failed to load romfs:/rei_selection.t3x");
    return false;
  }

  Sprite asuka_sheet = load_sprite("romfs:/asuka_selection.t3x");
  if (!asuka_sheet.valid) {
    C2D_SpriteSheetFree(shinji_sheet.sprite_sheet);
    C2D_SpriteSheetFree(rei_sheet.sprite_sheet);
    log_error("ERROR: Failed to load romfs:/asuka_selection.t3x");
    return false;
  }

  Sprite master_skins = load_sprite("romfs:/characters_selection_master.t3x");
  if (!master_skins.valid) {
    C2D_SpriteSheetFree(shinji_sheet.sprite_sheet);
    C2D_SpriteSheetFree(rei_sheet.sprite_sheet);
    C2D_SpriteSheetFree(asuka_sheet.sprite_sheet);
    log_error("ERROR: Failed to load romfs:/characters_selection_master.t3x");
    return false;
  }

  C2D_Sprite shinji;
  C2D_Sprite shinji_highlighted;
  
  C2D_Sprite rei;
  C2D_Sprite rei_highlighted;

  C2D_Sprite asuka;
  C2D_Sprite asuka_highlighted;


  C2D_Sprite shinji_master;
  C2D_Sprite shinji_highlighted_master;
  
  C2D_Sprite rei_master;
  C2D_Sprite rei_highlighted_master;

  C2D_Sprite asuka_master;
  C2D_Sprite asuka_highlighted_master;

  shinji = shinji_sheet.sprite;
  rei = rei_sheet.sprite;
  asuka = asuka_sheet.sprite;

  C2D_SpriteFromSheet(&shinji_highlighted, shinji_sheet.sprite_sheet, 1);
  C2D_SpriteFromSheet(&rei_highlighted, rei_sheet.sprite_sheet, 1);
  C2D_SpriteFromSheet(&asuka_highlighted, asuka_sheet.sprite_sheet, 1);

  shinji_highlighted_master = master_skins.sprite;
  C2D_SpriteFromSheet(&shinji_master, master_skins.sprite_sheet, 1);
  C2D_SpriteFromSheet(&asuka_highlighted_master, master_skins.sprite_sheet, 2);
  C2D_SpriteFromSheet(&asuka_master, master_skins.sprite_sheet, 3);
  C2D_SpriteFromSheet(&rei_highlighted_master, master_skins.sprite_sheet, 4);
  C2D_SpriteFromSheet(&rei_master, master_skins.sprite_sheet, 5);

  C2D_SpriteSetCenter(&shinji, 0, 0);
  C2D_SpriteSetCenter(&rei, 0, 0);
  C2D_SpriteSetCenter(&asuka, 0, 0);
  C2D_SpriteSetCenter(&shinji_highlighted, 0, 0);
  C2D_SpriteSetCenter(&rei_highlighted, 0, 0);
  C2D_SpriteSetCenter(&asuka_highlighted, 0, 0);
  C2D_SpriteSetCenter(&shinji_master, 0, 0);
  C2D_SpriteSetCenter(&rei_master, 0, 0);
  C2D_SpriteSetCenter(&asuka_master, 0, 0);
  C2D_SpriteSetCenter(&shinji_highlighted_master, 0, 0);
  C2D_SpriteSetCenter(&rei_highlighted_master, 0, 0);
  C2D_SpriteSetCenter(&asuka_highlighted_master, 0, 0);

  C2D_SpriteSetPos(&shinji, 6*16, 3*16);
  C2D_SpriteSetPos(&rei, 6*16, 3*16);
  C2D_SpriteSetPos(&asuka, 6*16, 3*16);
  C2D_SpriteSetPos(&shinji_highlighted, 6*16, 3*16);
  C2D_SpriteSetPos(&rei_highlighted, 6*16, 3*16);
  C2D_SpriteSetPos(&asuka_highlighted, 6*16, 3*16);

  C2D_SpriteSetPos(&shinji_master, 13*16, 3*16);
  C2D_SpriteSetPos(&rei_master, 13*16, 3*16);
  C2D_SpriteSetPos(&asuka_master, 13*16, 3*16);
  C2D_SpriteSetPos(&shinji_highlighted_master, 13*16, 3*16);
  C2D_SpriteSetPos(&rei_highlighted_master, 13*16, 3*16);
  C2D_SpriteSetPos(&asuka_highlighted_master, 13*16, 3*16);

  enum {
    SHINJI,
    REI,
    ASUKA
  };

  enum {
    DEFAULT,
    MASTER
  };

  enum {
    DARK,
    HIGHLIGHTED
  };


  C2D_Sprite skins[ASUKA+1][MASTER+1][HIGHLIGHTED+1];

  skins[SHINJI][DEFAULT][DARK] = shinji;
  skins[SHINJI][DEFAULT][HIGHLIGHTED] = shinji_highlighted;
  skins[SHINJI][MASTER][DARK] = shinji_master;
  skins[SHINJI][MASTER][HIGHLIGHTED] = shinji_highlighted_master;

  skins[REI][DEFAULT][DARK] = rei;
  skins[REI][DEFAULT][HIGHLIGHTED] = rei_highlighted;
  skins[REI][MASTER][DARK] = rei_master;
  skins[REI][MASTER][HIGHLIGHTED] = rei_highlighted_master;

  skins[ASUKA][DEFAULT][DARK] = asuka;
  skins[ASUKA][DEFAULT][HIGHLIGHTED] = asuka_highlighted;
  skins[ASUKA][MASTER][DARK] = asuka_master;
  skins[ASUKA][MASTER][HIGHLIGHTED] = asuka_highlighted_master;

  Sprite bg = load_sprite("romfs:/skins_menu.t3x");
  if (!bg.valid) {
    C2D_SpriteSheetFree(shinji_sheet.sprite_sheet);
    C2D_SpriteSheetFree(rei_sheet.sprite_sheet);
    C2D_SpriteSheetFree(asuka_sheet.sprite_sheet);
    C2D_SpriteSheetFree(master_skins.sprite_sheet);
    log_error("ERROR: Failed to load romfs:/skins_menu.t3x");
    return false;
  }

  C2D_Sprite equip = bg.sprite;
  C2D_Sprite nothing;

  C2D_SpriteFromSheet(&nothing, bg.sprite_sheet, 1);

  C2D_SpriteSetCenter(&equip, 0, 0);
  C2D_SpriteSetCenter(&nothing, 0, 0);

  C2D_SpriteSetPos(&equip, 0, 0);
  C2D_SpriteSetPos(&nothing, 0, 0);


  int character;
  int save_skin_version;
  int skin_version;

  if (strcmp(save_data->main.current_pilot, "shinji") == 0) {
    character = SHINJI;
    if (strcmp(save_data->pilots.shinji.current_skin, "default_skin") == 0) {
      save_skin_version = DEFAULT;
      skin_version = DEFAULT;
    } else {
      save_skin_version = MASTER;
      skin_version = MASTER;
    }
  } else if (strcmp(save_data->main.current_pilot, "rei") == 0) {
    character = REI;
    if (strcmp(save_data->pilots.rei.current_skin, "default_skin") == 0) {
      save_skin_version = DEFAULT;
      skin_version = DEFAULT;
    } else {
      save_skin_version = MASTER;
      skin_version = MASTER;
    }
  } else {
    if (strcmp(save_data->pilots.asuka.current_skin, "default_skin") == 0) {
      skin_version = DEFAULT;
      save_skin_version = DEFAULT;
    } else {
      save_skin_version = MASTER;
      skin_version = MASTER;
    }
    character = ASUKA;
  }

  bool can_equip = false;

  while (aptMainLoop()) {
    hidScanInput();
    u32 key_pressed = hidKeysDown();
    if (key_pressed & KEY_B) {
      break;
    } else if (key_pressed & KEY_DLEFT) {
      skin_version = DEFAULT;
      if (save_skin_version != skin_version) { can_equip = true; } else { can_equip = false; }
      if (can_equip) {
        if (skin_version == MASTER) {
          if (character == SHINJI) {
            if (!save_data->pilots.shinji.mastery_unlocked) { can_equip = false; }
          } else if (character == REI) {
            if (!save_data->pilots.rei.mastery_unlocked) { can_equip = false; }
          } else if (character == ASUKA) {
            if (!save_data->pilots.asuka.mastery_unlocked) { can_equip = false; }
          }
        }
      }
    } else if (key_pressed & KEY_DRIGHT) {
      skin_version = MASTER;
      if (save_skin_version != skin_version) { can_equip = true; } else { can_equip = false; }
      if (can_equip) {
        if (skin_version == MASTER) {
          if (character == SHINJI) {
            if (!save_data->pilots.shinji.mastery_unlocked) { can_equip = false; }
          } else if (character == REI) {
            if (!save_data->pilots.rei.mastery_unlocked) { can_equip = false; }
          } else if (character == ASUKA) {
            if (!save_data->pilots.asuka.mastery_unlocked) { can_equip = false; }
          }
        }
      }
    } else if (key_pressed & KEY_A) {
      if (can_equip) {
        save_skin_version = skin_version;
        can_equip = false;
        if (save_skin_version == DEFAULT) {
          if (character == SHINJI) { save_data->pilots.shinji.current_skin = "default_skin"; }
          else if (character == REI) { save_data->pilots.rei.current_skin = "default_skin"; }
          else if (character == ASUKA) { save_data->pilots.asuka.current_skin = "default_skin"; }
        } else {
          if (character == SHINJI) { save_data->pilots.shinji.current_skin = "master"; }
          else if (character == REI) { save_data->pilots.rei.current_skin = "master"; }
          else if (character == ASUKA) { save_data->pilots.asuka.current_skin = "master"; }
        }
      }
    }

    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    C2D_TargetClear(top_screen, colour_palette.background);
    C2D_TargetClear(bottom_screen, colour_palette.background);
    
    C2D_SceneBegin(top_screen);

    C2D_DrawSprite(&skins[character][skin_version][HIGHLIGHTED]);
    if (skin_version == DEFAULT) { C2D_DrawSprite(&skins[character][MASTER][DARK]); }
    else { C2D_DrawSprite(&skins[character][DEFAULT][DARK]); }
    
    C2D_SceneBegin(bottom_screen);

    if (can_equip) { C2D_DrawSprite(&equip); }
    else { C2D_DrawSprite(&nothing); }
   
    C3D_FrameEnd(0);
  }


  C2D_SpriteSheetFree(shinji_sheet.sprite_sheet);
  C2D_SpriteSheetFree(rei_sheet.sprite_sheet);
  C2D_SpriteSheetFree(asuka_sheet.sprite_sheet);
  C2D_SpriteSheetFree(master_skins.sprite_sheet);
  C2D_SpriteSheetFree(bg.sprite_sheet);
  return true;
}

bool mastery_menu(SaveFile* save_data, C3D_RenderTarget* top_screen, C3D_RenderTarget* bottom_screen, Colours colour_palette) {
  bool previously_unlocked = false;
  bool can_unlock = false;

  if (strcmp(save_data->main.current_pilot, "shinji") == 0) {
    if (!save_data->pilots.shinji.mastery_unlocked) { can_unlock = check_mastery(save_data, "shinji"); }
    else { previously_unlocked = true; }
  } else if (strcmp(save_data->main.current_pilot, "asuka") == 0) {
    if (!save_data->pilots.asuka.mastery_unlocked) { can_unlock = check_mastery(save_data, "asuka"); }
    else { previously_unlocked = true; }
  } else if (strcmp(save_data->main.current_pilot, "rei") == 0) {
    if (!save_data->pilots.rei.mastery_unlocked) { can_unlock = check_mastery(save_data, "rei"); }
    else { previously_unlocked = true; }
  }

  Sprite sprites = load_sprite("romfs:/mastery_menu.t3x");
  if (!sprites.valid) {
    log_error("ERROR: Failed to load romfs:/mastery_menu.t3x");
    return false;
  }

  C2D_Sprite shinji;
  C2D_Sprite shinji_locked;

  C2D_Sprite asuka;
  C2D_Sprite asuka_locked;

  C2D_Sprite rei;
  C2D_Sprite rei_locked;

  C2D_Sprite unlock;

  C2D_Sprite bottom_bg;

  shinji = sprites.sprite;
  C2D_SpriteFromSheet(&shinji_locked, sprites.sprite_sheet, 1);
  C2D_SpriteFromSheet(&asuka, sprites.sprite_sheet, 2);
  C2D_SpriteFromSheet(&asuka_locked, sprites.sprite_sheet, 3);
  C2D_SpriteFromSheet(&rei, sprites.sprite_sheet, 4);
  C2D_SpriteFromSheet(&rei_locked, sprites.sprite_sheet, 5);
  C2D_SpriteFromSheet(&unlock, sprites.sprite_sheet, 6);
  C2D_SpriteFromSheet(&bottom_bg, sprites.sprite_sheet, 7);

  C2D_SpriteSetCenter(&shinji, 0, 0);
  C2D_SpriteSetCenter(&shinji_locked, 0, 0);
  C2D_SpriteSetCenter(&asuka, 0, 0);
  C2D_SpriteSetCenter(&asuka_locked, 0, 0);
  C2D_SpriteSetCenter(&rei, 0, 0);
  C2D_SpriteSetCenter(&rei_locked, 0, 0);
  C2D_SpriteSetCenter(&unlock, 0, 0);
  C2D_SpriteSetCenter(&bottom_bg, 0, 0);

  C2D_SpriteSetPos(&shinji, 10*16, 4*16);
  C2D_SpriteSetPos(&shinji_locked, 10*16, 4*16);
  C2D_SpriteSetPos(&asuka, 10*16, 4*16);
  C2D_SpriteSetPos(&asuka_locked, 10*16, 4*16);
  C2D_SpriteSetPos(&rei, 10*16, 4*16);
  C2D_SpriteSetPos(&rei_locked, 10*16, 4*16);
  C2D_SpriteSetPos(&unlock, 6*16, 10*16);
  C2D_SpriteSetPos(&bottom_bg, 0, 0);

  Text max_stats;
  Text ability_slots;
  Text beat_1_helpless;
  Text angels_beat_hard;
  float font_scale = 1.0;

  if (strcmp(save_data->main.current_pilot, "shinji") == 0) {
    if (save_data->pilots.shinji.mastery.max_stats) {
      max_stats = load_text("true");
    } else { max_stats = load_text("false"); }

    char* buf = malloc(sizeof(char)*15);
    ability_slots = load_text(itoa(save_data->pilots.shinji.mastery.ability_slots, buf, 10));
    free(buf);

    buf = malloc(sizeof(char)*15);
    angels_beat_hard = load_text(itoa(save_data->pilots.shinji.mastery.angels_beat_hard, buf, 10));
    free(buf);

    if (save_data->pilots.shinji.mastery.beat_1_helpless) { beat_1_helpless = load_text("true"); }
    else { beat_1_helpless = load_text("false"); }
  } else if (strcmp(save_data->main.current_pilot, "asuka") == 0) {
    if (save_data->pilots.asuka.mastery.max_stats) {
      max_stats = load_text("true");
    } else { max_stats = load_text("false"); }

    char* buf = malloc(sizeof(char)*15);
    ability_slots = load_text(itoa(save_data->pilots.asuka.mastery.ability_slots, buf, 10));
    free(buf);

    buf = malloc(sizeof(char)*15);
    angels_beat_hard = load_text(itoa(save_data->pilots.asuka.mastery.angels_beat_hard, buf, 10));
    free(buf);

    if (save_data->pilots.asuka.mastery.beat_1_helpless) { beat_1_helpless = load_text("true"); }
    else { beat_1_helpless = load_text("false"); }
  } else if (strcmp(save_data->main.current_pilot, "rei") == 0) {
    if (save_data->pilots.rei.mastery.max_stats) {
      max_stats = load_text("true");
    } else { max_stats = load_text("false"); }

    char* buf = malloc(sizeof(char)*15);
    ability_slots = load_text(itoa(save_data->pilots.rei.mastery.ability_slots, buf, 10));
    free(buf);

    buf = malloc(sizeof(char)*15);
    angels_beat_hard = load_text(itoa(save_data->pilots.rei.mastery.angels_beat_hard, buf, 10));
    free(buf);

    if (save_data->pilots.rei.mastery.beat_1_helpless) { beat_1_helpless = load_text("true"); }
    else { beat_1_helpless = load_text("false"); }
  }

  while (aptMainLoop()) {
    hidScanInput();
    u32 key_pressed = hidKeysDown();
    if (key_pressed & KEY_B) {
      break;
    }
    if (key_pressed & KEY_A) {
      if (can_unlock) {
        if (strcmp(save_data->main.current_pilot, "shinji") == 0) {
          save_data->pilots.shinji.mastery_unlocked = true;
          unlock_cutscene("shinji", colour_palette, top_screen, bottom_screen);
          can_unlock = false;
          previously_unlocked = true;
        } else if (strcmp(save_data->main.current_pilot, "asuka") == 0) {
          save_data->pilots.asuka.mastery_unlocked = true;
          unlock_cutscene("asuka", colour_palette, top_screen, bottom_screen);
          can_unlock = false;
          previously_unlocked = true;
        } else if (strcmp(save_data->main.current_pilot, "rei") == 0) {
          save_data->pilots.rei.mastery_unlocked = true;
          unlock_cutscene("rei", colour_palette, top_screen, bottom_screen);
          can_unlock = false;
          previously_unlocked = true;
        }
      }
    }

    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    C2D_TargetClear(top_screen, colour_palette.background);
    C2D_TargetClear(bottom_screen, colour_palette.background);
    
    C2D_SceneBegin(top_screen);

    if (strcmp(save_data->main.current_pilot, "shinji") == 0) {
      if (previously_unlocked) { C2D_DrawSprite(&shinji); }
      else { C2D_DrawSprite(&shinji_locked); }
    } else if (strcmp(save_data->main.current_pilot, "asuka") == 0) {
      if (previously_unlocked) { C2D_DrawSprite(&asuka); }
      else { C2D_DrawSprite(&asuka_locked); }
    } else if (strcmp(save_data->main.current_pilot, "rei") == 0) {
      if (previously_unlocked) { C2D_DrawSprite(&rei); }
      else { C2D_DrawSprite(&rei_locked); }
    }
    
    C2D_SceneBegin(bottom_screen);

    C2D_DrawSprite(&bottom_bg);
    if (!previously_unlocked && can_unlock) {
      C2D_DrawSprite(&unlock);
    }

    C2D_DrawText(&angels_beat_hard.text, C2D_WithColor, 13*16, 4.5*16, 0, font_scale, font_scale, colour_palette.white);
    C2D_DrawText(&ability_slots.text, C2D_WithColor, 12*16, 2.5*16, 0, font_scale, font_scale, colour_palette.white);
    C2D_DrawText(&max_stats.text, C2D_WithColor, 9*16, 0.5*16, 0, font_scale, font_scale, colour_palette.white);
    C2D_DrawText(&beat_1_helpless.text, C2D_WithColor, 11*16, 6.5*16, 0, font_scale, font_scale, colour_palette.white);
   
    C3D_FrameEnd(0);
  }


  C2D_SpriteSheetFree(sprites.sprite_sheet);
  C2D_TextBufDelete(beat_1_helpless.buffer);
  C2D_TextBufDelete(max_stats.buffer);
  C2D_TextBufDelete(angels_beat_hard.buffer);
  C2D_TextBufDelete(ability_slots.buffer);
  return true;
}

bool unlock_cutscene(char* pilot, Colours colour_palette, C3D_RenderTarget* top_screen, C3D_RenderTarget* bottom_screen) {
  if (strcmp(pilot, "shinji") == 0) {
    Sprite shinji_1 = load_sprite("romfs:/shinji_unlock_animation_1.t3x");
    if (!shinji_1.valid) {
      log_error("ERROR: Failed to load romfs:/shinji_unlock_animation_1.t3x");
      return false;
    }
    Sprite shinji_2 = load_sprite("romfs:/shinji_unlock_animation_2.t3x");
    if (!shinji_2.valid) {
      C2D_SpriteSheetFree(shinji_1.sprite_sheet);
      log_error("ERROR: Failed to load romfs:/shinji_unlock_animation_2.t3x");
      return false;
    }
    Sprite shinji_3 = load_sprite("romfs:/shinji_unlock_animation_3.t3x");
    if (!shinji_3.valid) {
      C2D_SpriteSheetFree(shinji_1.sprite_sheet);
      C2D_SpriteSheetFree(shinji_2.sprite_sheet);
      log_error("ERROR: Failed to load romfs:/shinji_unlock_animation_3.t3x");
      return false;
    }
    Sprite shinji_4 = load_sprite("romfs:/shinji_unlock_animation_4.t3x");
    if (!shinji_4.valid) {
      C2D_SpriteSheetFree(shinji_1.sprite_sheet);
      C2D_SpriteSheetFree(shinji_2.sprite_sheet);
      C2D_SpriteSheetFree(shinji_3.sprite_sheet);
      log_error("ERROR: Failed to load romfs:/shinji_unlock_animation_4.t3x");
      return false;
    }

    C2D_Sprite frames[32+1];

    for (int i = 0; i < 32+1; i++) {
      if (i < 9-1) { C2D_SpriteFromSheet(&frames[i], shinji_1.sprite_sheet, i); }
      else if (i < 17-1) { C2D_SpriteFromSheet(&frames[i], shinji_2.sprite_sheet, i - 8); }
      else if (i < 25-1) { C2D_SpriteFromSheet(&frames[i], shinji_3.sprite_sheet, i - 16); }
      else { C2D_SpriteFromSheet(&frames[i], shinji_4.sprite_sheet, i - 24); }
    }

    for (int i = 0; i < 32+1; i++) {
      C2D_SpriteSetCenter(&frames[i], 0, 0);
      C2D_SpriteSetPos(&frames[i], 0, 0);
    }


    int current_frame = 0;
    u64 prev_frame = osGetTime();

    while (aptMainLoop()) {
      if (current_frame >= 32) { break; }

      u64 now = osGetTime();

      if (now - prev_frame >= 100) {
        current_frame++;
        prev_frame+=100;
      }
      
      C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
      C2D_TargetClear(top_screen, colour_palette.background);
      C2D_TargetClear(bottom_screen, colour_palette.background);

      C2D_SceneBegin(top_screen);
      
      C2D_DrawSprite(&frames[current_frame]);

      C3D_FrameEnd(0);
    }


    C2D_SpriteSheetFree(shinji_1.sprite_sheet);
    C2D_SpriteSheetFree(shinji_2.sprite_sheet);
    C2D_SpriteSheetFree(shinji_3.sprite_sheet);
    C2D_SpriteSheetFree(shinji_4.sprite_sheet);
    return true;
  } else if (strcmp(pilot, "asuka") == 0) {
    Sprite asuka_1 = load_sprite("romfs:/asuka_unlock_animation_1.t3x");
    if (!asuka_1.valid) {
      log_error("ERROR: Failed to load romfs:/asuka_unlock_animation_1.t3x");
      return false;
    }
    Sprite asuka_2 = load_sprite("romfs:/asuka_unlock_animation_2.t3x");
    if (!asuka_2.valid) {
      C2D_SpriteSheetFree(asuka_1.sprite_sheet);
      log_error("ERROR: Failed to load romfs:/asuka_unlock_animation_2.t3x");
      return false;
    }
    Sprite asuka_3 = load_sprite("romfs:/asuka_unlock_animation_3.t3x");
    if (!asuka_3.valid) {
      C2D_SpriteSheetFree(asuka_1.sprite_sheet);
      C2D_SpriteSheetFree(asuka_2.sprite_sheet);
      log_error("ERROR: Failed to load romfs:/asuka_unlock_animation_3.t3x");
      return false;
    }
    Sprite asuka_4 = load_sprite("romfs:/asuka_unlock_animation_4.t3x");
    if (!asuka_4.valid) {
      C2D_SpriteSheetFree(asuka_1.sprite_sheet);
      C2D_SpriteSheetFree(asuka_2.sprite_sheet);
      C2D_SpriteSheetFree(asuka_3.sprite_sheet);
      log_error("ERROR: Failed to load romfs:/asuka_unlock_animation_4.t3x");
      return false;
    }

    C2D_Sprite frames[32+1];

    for (int i = 0; i < 32+1; i++) {
      if (i < 9-1) { C2D_SpriteFromSheet(&frames[i], asuka_1.sprite_sheet, i); }
      else if (i < 17-1) { C2D_SpriteFromSheet(&frames[i], asuka_2.sprite_sheet, i - 8); }
      else if (i < 25-1) { C2D_SpriteFromSheet(&frames[i], asuka_3.sprite_sheet, i - 16); }
      else { C2D_SpriteFromSheet(&frames[i], asuka_4.sprite_sheet, i - 24); }
    }

    for (int i = 0; i < 32+1; i++) {
      C2D_SpriteSetCenter(&frames[i], 0, 0);
      C2D_SpriteSetPos(&frames[i], 0, 0);
    }


    int current_frame = 0;
    u64 prev_frame = osGetTime();

    while (aptMainLoop()) {
      if (current_frame >= 32) { break; }

      u64 now = osGetTime();

      if (now - prev_frame >= 100) {
        current_frame++;
        prev_frame+=100;
      }
      
      C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
      C2D_TargetClear(top_screen, colour_palette.background);
      C2D_TargetClear(bottom_screen, colour_palette.background);

      C2D_SceneBegin(top_screen);
      
      C2D_DrawSprite(&frames[current_frame]);

      C3D_FrameEnd(0);
    }


    C2D_SpriteSheetFree(asuka_1.sprite_sheet);
    C2D_SpriteSheetFree(asuka_2.sprite_sheet);
    C2D_SpriteSheetFree(asuka_3.sprite_sheet);
    C2D_SpriteSheetFree(asuka_4.sprite_sheet);
    return true;
  } else if (strcmp(pilot, "rei") == 0) {
    Sprite rei_1 = load_sprite("romfs:/rei_unlock_animation_1.t3x");
    if (!rei_1.valid) {
      log_error("ERROR: Failed to load romfs:/rei_unlock_animation_1.t3x");
      return false;
    }
    Sprite rei_2 = load_sprite("romfs:/rei_unlock_animation_2.t3x");
    if (!rei_2.valid) {
      C2D_SpriteSheetFree(rei_1.sprite_sheet);
      log_error("ERROR: Failed to load romfs:/rei_unlock_animation_2.t3x");
      return false;
    }
    Sprite rei_3 = load_sprite("romfs:/rei_unlock_animation_3.t3x");
    if (!rei_3.valid) {
      C2D_SpriteSheetFree(rei_1.sprite_sheet);
      C2D_SpriteSheetFree(rei_2.sprite_sheet);
      log_error("ERROR: Failed to load romfs:/rei_unlock_animation_3.t3x");
      return false;
    }
    Sprite rei_4 = load_sprite("romfs:/rei_unlock_animation_4.t3x");
    if (!rei_4.valid) {
      C2D_SpriteSheetFree(rei_1.sprite_sheet);
      C2D_SpriteSheetFree(rei_2.sprite_sheet);
      C2D_SpriteSheetFree(rei_3.sprite_sheet);
      log_error("ERROR: Failed to load romfs:/rei_unlock_animation_4.t3x");
      return false;
    }

    C2D_Sprite frames[32+1];

    for (int i = 0; i < 32+1; i++) {
      if (i < 9-1) { C2D_SpriteFromSheet(&frames[i], rei_1.sprite_sheet, i); }
      else if (i < 17-1) { C2D_SpriteFromSheet(&frames[i], rei_2.sprite_sheet, i - 8); }
      else if (i < 25-1) { C2D_SpriteFromSheet(&frames[i], rei_3.sprite_sheet, i - 16); }
      else { C2D_SpriteFromSheet(&frames[i], rei_4.sprite_sheet, i - 24); }
    }

    for (int i = 0; i < 32+1; i++) {
      C2D_SpriteSetCenter(&frames[i], 0, 0);
      C2D_SpriteSetPos(&frames[i], 0, 0);
    }


    int current_frame = 0;
    u64 prev_frame = osGetTime();

    while (aptMainLoop()) {
      if (current_frame >= 32) { break; }

      u64 now = osGetTime();

      if (now - prev_frame >= 100) {
        current_frame++;
        prev_frame+=100;
      }
      
      C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
      C2D_TargetClear(top_screen, colour_palette.background);
      C2D_TargetClear(bottom_screen, colour_palette.background);

      C2D_SceneBegin(top_screen);
      
      C2D_DrawSprite(&frames[current_frame]);

      C3D_FrameEnd(0);
    }


    C2D_SpriteSheetFree(rei_1.sprite_sheet);
    C2D_SpriteSheetFree(rei_2.sprite_sheet);
    C2D_SpriteSheetFree(rei_3.sprite_sheet);
    C2D_SpriteSheetFree(rei_4.sprite_sheet);
    return true;
  }

  return false;
}

bool stats_menu(SaveFile* save_data, C3D_RenderTarget* top_screen, C3D_RenderTarget* bottom_screen, Colours colour_palette) {
  Sprite labels = load_sprite("romfs:/stats_labels.t3x");
  if (!labels.valid) {
    log_error("ERROR: Failed to load romfs:/stats_labels.t3x");
    return false;
  }
  Sprite info = load_sprite("romfs:/stats_info.t3x");
  if (!info.valid) {
    C2D_SpriteSheetFree(labels.sprite_sheet);
    log_error("ERROR: Failed to load romfs:/stats_info.t3x");
    return false;
  }
  Sprite skins = load_sprite("romfs:/stats_skins.t3x");
  if (!skins.valid) {
    C2D_SpriteSheetFree(labels.sprite_sheet);
    C2D_SpriteSheetFree(info.sprite_sheet);
    log_error("ERROR: Failed to load romfs:/stats_skins.t3x");
    return false;
  }
  Sprite popups = load_sprite("romfs:/stats_popups.t3x");
  if (!popups.valid) {
    C2D_SpriteSheetFree(labels.sprite_sheet);
    C2D_SpriteSheetFree(info.sprite_sheet);
    C2D_SpriteSheetFree(skins.sprite_sheet);
    log_error("ERROR: Failed to load romfs:/stats_popups.t3x");
    return false;
  }
  Sprite bg = load_sprite("romfs:/stats_bg.t3x");
  if (!bg.valid) {
    C2D_SpriteSheetFree(labels.sprite_sheet);
    C2D_SpriteSheetFree(info.sprite_sheet);
    C2D_SpriteSheetFree(skins.sprite_sheet);
    C2D_SpriteSheetFree(popups.sprite_sheet);
    log_error("ERROR: Failed to load romfs:/stats_bg.t3x");
    return false;
  }
  Sprite max = load_sprite("romfs:/stats_max.t3x");
  if (!max.valid) {
    C2D_SpriteSheetFree(labels.sprite_sheet);
    C2D_SpriteSheetFree(info.sprite_sheet);
    C2D_SpriteSheetFree(skins.sprite_sheet);
    C2D_SpriteSheetFree(popups.sprite_sheet);
    C2D_SpriteSheetFree(bg.sprite_sheet);
    log_error("ERROR: Failed to load romfs:/stats_max.t3x");
    return false;
  }
  Sprite highlighted = load_sprite("romfs:/stats_highlighted.t3x");
  if (!highlighted.valid) {
    C2D_SpriteSheetFree(labels.sprite_sheet);
    C2D_SpriteSheetFree(info.sprite_sheet);
    C2D_SpriteSheetFree(skins.sprite_sheet);
    C2D_SpriteSheetFree(popups.sprite_sheet);
    C2D_SpriteSheetFree(bg.sprite_sheet);
    C2D_SpriteSheetFree(max.sprite_sheet);
    log_error("ERROR: Failed to load romfs:/stats_highlighted.t3x");
    return false;
  }

  enum {
    AT_FIELD,
    ATTACK,
    BOND,
    DEFAULT_SYNC_RATE,
    MAX_HEALTH
  };

  enum {
    ASUKA,
    ASUKA_MASTER,
    REI,
    REI_MASTER,
    SHINJI,
    SHINJI_MASTER
  };

  C2D_Sprite labels_array[MAX_HEALTH+1];
  C2D_Sprite info_array[MAX_HEALTH+1];
  C2D_Sprite highlighted_array[MAX_HEALTH+1];

  for (int i = 0; i < MAX_HEALTH+1; i++) {
    C2D_SpriteFromSheet(&labels_array[i], labels.sprite_sheet, i);
    C2D_SpriteFromSheet(&info_array[i], info.sprite_sheet, i);
    C2D_SpriteSetCenter(&info_array[i], 0, 0);
    C2D_SpriteSetPos(&info_array[i], 4*16, 7*16);
    C2D_SpriteSetCenter(&labels_array[i], 0, 0);
    C2D_SpriteFromSheet(&highlighted_array[i], highlighted.sprite_sheet, i);
    C2D_SpriteSetCenter(&highlighted_array[i], 0, 0);
  }

  C2D_Sprite skins_array[SHINJI_MASTER+1];

  for (int i = 0; i <  SHINJI_MASTER+1; i++) {
    C2D_SpriteFromSheet(&skins_array[i], skins.sprite_sheet, i);
    C2D_SpriteSetCenter(&skins_array[i], 0, 0);
    C2D_SpriteSetPos(&skins_array[i], 2*16, 3*16);
  }

  C2D_Sprite max_100 = max.sprite;
  C2D_Sprite max_200;

  C2D_SpriteFromSheet(&max_200, max.sprite_sheet, 1);

  C2D_SpriteSetCenter(&max_100, 0, 0);
  C2D_SpriteSetPos(&max_100, 4*16, 4*16);
  C2D_SpriteSetCenter(&max_200, 0, 0);
  C2D_SpriteSetPos(&max_200, 4*16, 4*16);

  C2D_Sprite bg_sprite = bg.sprite;

  C2D_SpriteSetCenter(&bg_sprite, 0, 0);
  C2D_SpriteSetPos(&bg_sprite, 0, 0);

  C2D_Sprite broke = popups.sprite;
  C2D_Sprite upgrade;

  C2D_SpriteFromSheet(&upgrade, popups.sprite_sheet, 1);

  C2D_SpriteSetCenter(&broke, 0, 0);
  C2D_SpriteSetPos(&broke, 4*16, 4*16);
  C2D_SpriteSetCenter(&upgrade, 0, 0);
  C2D_SpriteSetPos(&upgrade, 4*16, 4*16);


  int player_stats[MAX_HEALTH+1];
  int character = ASUKA;

  if (strcmp(save_data->main.current_pilot, "shinji") == 0) {
    if (strcmp(save_data->pilots.shinji.current_skin, "master") == 0) {
      character = SHINJI_MASTER;
    } else {
      character = SHINJI;
    }
    player_stats[AT_FIELD] = save_data->pilots.shinji.stats.at_field;
    player_stats[ATTACK] = save_data->pilots.shinji.stats.attack;
    player_stats[BOND] = save_data->pilots.shinji.stats.bond;
    player_stats[DEFAULT_SYNC_RATE] = save_data->pilots.shinji.stats.default_sync_rate;
    player_stats[MAX_HEALTH] = save_data->pilots.shinji.stats.max_health;
  } else if (strcmp(save_data->main.current_pilot, "asuka") == 0) {
    if (strcmp(save_data->pilots.asuka.current_skin, "master") == 0) {
      character = ASUKA_MASTER;
    } else {
      character = ASUKA;
    }
    player_stats[AT_FIELD] = save_data->pilots.asuka.stats.at_field;
    player_stats[ATTACK] = save_data->pilots.asuka.stats.attack;
    player_stats[BOND] = save_data->pilots.asuka.stats.bond;
    player_stats[DEFAULT_SYNC_RATE] = save_data->pilots.asuka.stats.default_sync_rate;
    player_stats[MAX_HEALTH] = save_data->pilots.asuka.stats.max_health;
  } else if (strcmp(save_data->main.current_pilot, "rei") == 0) {
    if (strcmp(save_data->pilots.rei.current_skin, "master") == 0) {
      character = REI_MASTER;
    } else {
      character = REI;
    }
    player_stats[AT_FIELD] = save_data->pilots.rei.stats.at_field;
    player_stats[ATTACK] = save_data->pilots.rei.stats.attack;
    player_stats[BOND] = save_data->pilots.rei.stats.bond;
    player_stats[DEFAULT_SYNC_RATE] = save_data->pilots.rei.stats.default_sync_rate;
    player_stats[MAX_HEALTH] = save_data->pilots.rei.stats.max_health;
  }

  
  int current_stat = AT_FIELD;
  int level_points = save_data->main.level_points;
  bool level_points_changed = false;

  struct stat {
    bool changed;
    int stat;
  };

  struct stat stat_changed;
  stat_changed.changed = false;
  stat_changed.stat = current_stat;

  Text stats_text[MAX_HEALTH+1];
  for (int i = 0; i < MAX_HEALTH+1; i++) {
    char* buf = malloc(sizeof(char)*15);
    stats_text[i] = load_text(itoa(player_stats[i], buf, 10));
    free(buf);
  }
  float font_scale = 1.0;

  char* buf = malloc(sizeof(char)*15);
  Text level_points_text = load_text(strcat(itoa(save_data->main.level_points, buf, 10), " LP"));
  free(buf);
  
  while (aptMainLoop()) {
    hidScanInput();
    u32 key_pressed = hidKeysDown();
    if (key_pressed & KEY_B) {
      break;
    } else if (key_pressed & KEY_DDOWN) {
      if (current_stat != MAX_HEALTH) {
        current_stat++;
      } else {
        current_stat = AT_FIELD;
      }
    } else if (key_pressed & KEY_DUP) {
      if (current_stat != AT_FIELD) {
        current_stat--;
      } else {
        current_stat = MAX_HEALTH;
      }
    } else if (key_pressed & KEY_A) {
      if (current_stat == MAX_HEALTH) {
        if (player_stats[current_stat] != 200) {
          while (true) {
            hidScanInput();
            key_pressed = hidKeysDown();

            if (key_pressed & KEY_B) {
              break;
            } else if (key_pressed & KEY_A) {
              if (level_points - 75 >= 0) {
                level_points -= 75;
                save_data->main.level_points = level_points;
                level_points_changed = true;
                player_stats[current_stat] += 5;
                stat_changed.changed = true;
                stat_changed.stat = current_stat;
                break;
              } else {
                while (true) {
                  hidScanInput();
                  key_pressed = hidKeysDown();
                  if (key_pressed & KEY_B) { break; }

                  C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

                  C2D_TargetClear(bottom_screen, colour_palette.background);
                  C2D_SceneBegin(bottom_screen);
                  C2D_DrawSprite(&broke);

                  C3D_FrameEnd(0);
                }
              }
            }

            C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

            C2D_TargetClear(bottom_screen, colour_palette.background);
            C2D_SceneBegin(bottom_screen);
            C2D_DrawSprite(&upgrade);

            C3D_FrameEnd(0);
          }
        }
      } else {
        if (player_stats[current_stat] != 100) {
          while (true) {
            hidScanInput();
            key_pressed = hidKeysDown();

            if (key_pressed & KEY_B) {
              break;
            } else if (key_pressed & KEY_A) {
              if (level_points - 75 >= 0) {
                level_points -= 75;
                save_data->main.level_points = level_points;
                level_points_changed = true;
                player_stats[current_stat] += 5;
                stat_changed.changed = true;
                stat_changed.stat = current_stat;
                break;
              } else {
                while (true) {
                  hidScanInput();
                  key_pressed = hidKeysDown();
                  if (key_pressed & KEY_B) { break; }

                  C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

                  C2D_TargetClear(bottom_screen, colour_palette.background);
                  C2D_SceneBegin(bottom_screen);
                  C2D_DrawSprite(&broke);

                  C3D_FrameEnd(0);
                }
              }
            }

            C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

            C2D_TargetClear(bottom_screen, colour_palette.background);
            C2D_SceneBegin(bottom_screen);
            C2D_DrawSprite(&upgrade);

            C3D_FrameEnd(0);
          }
        }
      }
    }
     

    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    C2D_TargetClear(top_screen, colour_palette.background);
    C2D_TargetClear(bottom_screen, colour_palette.background);
    
    C2D_SceneBegin(top_screen);

    C2D_DrawSprite(&skins_array[character]);
    for (int i = 0; i < 3; i++) {
      int stat = (current_stat + i) % (MAX_HEALTH + 1);
      if (i == 0) {
        C2D_SpriteSetPos(&highlighted_array[stat], 11*16, (4+i*3)*16);
        C2D_DrawSprite(&highlighted_array[stat]);
      } else {
        C2D_SpriteSetPos(&labels_array[stat], 11*16, (4+i*3)*16);
        C2D_DrawSprite(&labels_array[stat]);
      }
      if (stat_changed.changed) {
        C2D_TextBufClear(stats_text[stat_changed.stat].buffer);
        char* buf = malloc(sizeof(char)*15);
        stats_text[stat_changed.stat] = load_text(itoa(player_stats[stat_changed.stat], buf, 10));
        free(buf);
        stat_changed.changed = false;
      }
      if (stat == DEFAULT_SYNC_RATE) {
        C2D_DrawText(&stats_text[stat].text, C2D_WithColor, 20*16, (4+i*3)*16, 0, font_scale, font_scale, colour_palette.white);
      }
      else {
        C2D_DrawText(&stats_text[stat].text, C2D_WithColor, 17*16, (4+i*3)*16, 0, font_scale, font_scale, colour_palette.white);
      }
    }

    
    C2D_SceneBegin(bottom_screen);

    C2D_DrawSprite(&bg_sprite);
    if (current_stat == MAX_HEALTH) {
      C2D_DrawSprite(&max_200);
    } else {
      C2D_DrawSprite(&max_100);
    }
    C2D_DrawSprite(&info_array[current_stat]);

    if (level_points_changed) {
      C2D_TextBufClear(level_points_text.buffer);
      char* buf = malloc(sizeof(char)*15);
      level_points_text = load_text(strcat(itoa(level_points, buf, 10), " LP"));
      free(buf);
      level_points_changed = false;
    }
    C2D_DrawText(&level_points_text.text, C2D_WithColor, 35, 0, 0, font_scale, font_scale, colour_palette.white); 

   
    C3D_FrameEnd(0);
  }


  if (strcmp(save_data->main.current_pilot, "shinji") == 0) {
    save_data->pilots.shinji.stats.at_field = player_stats[AT_FIELD];
    save_data->pilots.shinji.stats.attack = player_stats[ATTACK];
    save_data->pilots.shinji.stats.bond = player_stats[BOND];
    save_data->pilots.shinji.stats.default_sync_rate = player_stats[DEFAULT_SYNC_RATE];
    save_data->pilots.shinji.stats.max_health = player_stats[MAX_HEALTH];
  } else if (strcmp(save_data->main.current_pilot, "asuka") == 0) {
    save_data->pilots.asuka.stats.at_field = player_stats[AT_FIELD];
    save_data->pilots.asuka.stats.attack = player_stats[ATTACK];
    save_data->pilots.asuka.stats.bond = player_stats[BOND];
    save_data->pilots.asuka.stats.default_sync_rate = player_stats[DEFAULT_SYNC_RATE];
    save_data->pilots.asuka.stats.max_health = player_stats[MAX_HEALTH];
  } else if (strcmp(save_data->main.current_pilot, "rei") == 0) {
    save_data->pilots.rei.stats.at_field = player_stats[AT_FIELD];
    save_data->pilots.rei.stats.attack = player_stats[ATTACK];
    save_data->pilots.rei.stats.bond = player_stats[BOND];
    save_data->pilots.rei.stats.default_sync_rate = player_stats[DEFAULT_SYNC_RATE];
    save_data->pilots.rei.stats.max_health = player_stats[MAX_HEALTH];
  }


  C2D_SpriteSheetFree(labels.sprite_sheet);
  C2D_SpriteSheetFree(info.sprite_sheet);
  C2D_SpriteSheetFree(skins.sprite_sheet);
  C2D_SpriteSheetFree(popups.sprite_sheet);
  C2D_SpriteSheetFree(bg.sprite_sheet);
  C2D_SpriteSheetFree(max.sprite_sheet);
  C2D_SpriteSheetFree(highlighted.sprite_sheet);
  for (int i = 0; i < MAX_HEALTH+1; i++) {
    C2D_TextBufDelete(stats_text[i].buffer);
  }
  C2D_TextBufDelete(level_points_text.buffer);
  return true;
}

GameModeOutput game_mode_menu(C3D_RenderTarget* top_screen, C3D_RenderTarget* bottom_screen, Colours colour_palette) {
  /*

    info coords: (4;3)

    prev: (32; 56)
    current: (114+32; 56)
    next: (228+32; 56)
    
    assets:

    game_mode_top.t3x
    game_mode_dark.t3x
    game_mode_info.t3x
    game_mode_bg.t3x

    endless
    marathon
    randomizer
    single_battle
    tutorial

    output:
      valid: false on error else true
      game_mode: -1 on error/nothing else a valid game mode
  */

  GameModeOutput output;
  output.valid = false;
  output.game_mode = -1;

  Sprite top_highlighted = load_sprite("romfs:/game_mode_top.t3x");
  if (!top_highlighted.valid) {
    log_error("ERROR: Failed to load romfs:/game_mode_top.t3x");
    return output;
  }

  Sprite top_dark = load_sprite("romfs:/game_mode_dark.t3x");
  if (!top_dark.valid) {
    C2D_SpriteSheetFree(top_highlighted.sprite_sheet);
    log_error("ERROR: Failed to load romfs:/game_mode_dark.t3x");
    return output;
  }
  
  Sprite info = load_sprite("romfs:/game_mode_info.t3x");
  if (!info.valid) {
    C2D_SpriteSheetFree(top_highlighted.sprite_sheet);
    C2D_SpriteSheetFree(top_dark.sprite_sheet);
    log_error("ERROR: Failed to load romfs:/game_mode_info.t3x");
    return output;
  }

  Sprite bg = load_sprite("romfs:/game_mode_bg.t3x");
  if (!bg.valid) {
    C2D_SpriteSheetFree(top_highlighted.sprite_sheet);
    C2D_SpriteSheetFree(top_dark.sprite_sheet);
    C2D_SpriteSheetFree(info.sprite_sheet);
    log_error("ERROR: Failed to load romfs:/game_mode_bg.t3x");
    return output;
  }

  enum {
    ENDLESS,
    MARATHON,
    RANDOMIZER,
    SINGLE_BATTLE,
    TUTORIAL,
  };

  C2D_Sprite top_highlighted_array[TUTORIAL+1];
  C2D_Sprite top_dark_array[TUTORIAL+1];
  C2D_Sprite info_array[TUTORIAL+1];

  C2D_Sprite bg_sprite = bg.sprite;
  C2D_SpriteSetCenter(&bg_sprite, 0, 0);
  C2D_SpriteSetPos(&bg_sprite, 0, 0);

  for (int i = 0; i < TUTORIAL+1; i++) {
    C2D_SpriteFromSheet(&top_dark_array[i], top_dark.sprite_sheet, i);
    C2D_SpriteFromSheet(&top_highlighted_array[i], top_highlighted.sprite_sheet, i);
    C2D_SpriteFromSheet(&info_array[i], info.sprite_sheet, i);

    C2D_SpriteSetCenter(&top_highlighted_array[i], 0, 0);
    C2D_SpriteSetCenter(&top_dark_array[i], 0, 0);
    C2D_SpriteSetCenter(&info_array[i], 0, 0);
    C2D_SpriteSetPos(&info_array[i], 4*16, 3*16);
  }


  int current_mode = TUTORIAL;

  while (aptMainLoop()) {
    hidScanInput();
    u32 key_pressed = hidKeysDown();
    if (key_pressed & KEY_B) {
      output.game_mode = -1;
      break;
    } else if (key_pressed & KEY_A) {
      output.game_mode = current_mode;
      break;
    } else if (key_pressed & KEY_DLEFT) {
      current_mode = wrap(current_mode - 1, TUTORIAL + 1);
    } else if (key_pressed & KEY_DRIGHT) {
      current_mode = wrap(current_mode + 1, TUTORIAL + 1);
    } 

    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    C2D_TargetClear(top_screen, colour_palette.background);
    C2D_TargetClear(bottom_screen, colour_palette.background);
    
    C2D_SceneBegin(top_screen);

    C2D_SpriteSetPos(&top_dark_array[wrap(current_mode - 1, TUTORIAL + 1)], 32, 56);
    C2D_SpriteSetPos(&top_highlighted_array[current_mode], 114+32, 56);
    C2D_SpriteSetPos(&top_dark_array[wrap(current_mode + 1, TUTORIAL + 1)], 228+32, 56);

    C2D_DrawSprite(&top_dark_array[wrap(current_mode - 1, TUTORIAL + 1)]);
    C2D_DrawSprite(&top_highlighted_array[current_mode]);
    C2D_DrawSprite(&top_dark_array[wrap(current_mode + 1, TUTORIAL + 1)]);
    
    C2D_SceneBegin(bottom_screen);

    C2D_DrawSprite(&bg_sprite);
    C2D_DrawSprite(&info_array[current_mode]);
   
    C3D_FrameEnd(0);
  }

  
  C2D_SpriteSheetFree(top_highlighted.sprite_sheet);
  C2D_SpriteSheetFree(top_dark.sprite_sheet);
  C2D_SpriteSheetFree(info.sprite_sheet);
  C2D_SpriteSheetFree(bg.sprite_sheet);
  output.valid = true;
  return output;
}

// ==== UTILITIES ==== //

int wrap(int index, int range) {
  return (index % range + range) % range;
}

bool check_mastery(SaveFile* save_data, char* pilot) {
  if (strcmp(pilot, "shinji") == 0) {
    if (save_data->pilots.shinji.mastery.max_stats) {
      if (save_data->pilots.shinji.mastery.ability_slots == 4) {
        if (save_data->pilots.shinji.mastery.angels_beat_hard == 16) {
          if (save_data->pilots.shinji.mastery.beat_1_helpless) {
            return true;
          }
        }
      }
    }
  } else if (strcmp(pilot, "asuka") == 0) {
    if (save_data->pilots.asuka.mastery.max_stats) {
      if (save_data->pilots.asuka.mastery.ability_slots == 4) {
        if (save_data->pilots.asuka.mastery.angels_beat_hard == 16) {
          if (save_data->pilots.asuka.mastery.beat_1_helpless) {
            return true;
          }
        }
      }
    }
  } else {
    if (save_data->pilots.rei.mastery.max_stats) {
      if (save_data->pilots.rei.mastery.ability_slots == 4) {
        if (save_data->pilots.rei.mastery.angels_beat_hard == 16) {
          if (save_data->pilots.rei.mastery.beat_1_helpless) {
            return true;
          }
        }
      }
    }
  }

  return false;
}

bool create_backup(C3D_RenderTarget* top_screen, Colours colour_palette) {
  FILE* save_file = fopen("sdmc:/3ds/evangelion_angel_attack/save_file.json", "rb");
  if (save_file == NULL) {
    log_error("ERROR: Failed to open sdmc:/3ds/evangelion_angel_attack/save_file.json");
    return false;
  }
  fseek(save_file, 0, SEEK_END);
  long file_size = ftell(save_file);
  rewind(save_file);
  char* file_buffer = malloc(file_size+1);
  if (file_buffer == NULL) {
    log_error("ERROR: Failed to allocate file buffer");
    return false;
  }
  fread(file_buffer, 1, file_size, save_file);
  file_buffer[file_size] = '\0';
  fclose(save_file);

  FILE* backup_file = fopen("sdmc:/3ds/evangelion_angel_attack/save_file_backup.json", "w");
  if (backup_file == NULL) {
    log_error("ERROR: Failed to create sdmc:/3ds/evangelion_angel_attack/save_file_backup.json");
    free(file_buffer);
    return false;
  }
  fputs(file_buffer, backup_file);
  fclose(backup_file);
  free(file_buffer);

  Sprite popup = load_sprite("romfs:/done_popup.t3x");
  if (!popup.valid) {
    log_error("ERROR: Failed to load romfs:/done_popup.t3x");
    return false;
  }
  C2D_SpriteSetCenter(&popup.sprite, 0, 0);
  C2D_SpriteSetPos(&popup.sprite, 7*16, 4*16);

  while (aptMainLoop()) {
    hidScanInput();
    u32 key_pressed = hidKeysDown();
    if (key_pressed & KEY_A) {
      break;
    } 

    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    C2D_TargetClear(top_screen, colour_palette.background);
    
    C2D_SceneBegin(top_screen);

    C2D_DrawSprite(&popup.sprite);
   
    C3D_FrameEnd(0);
  }

  C2D_SpriteSheetFree(popup.sprite_sheet);
  return true;
}

void close_game(SaveFile save_data) {
  write_to_save_file(save_data);
  system_exit();
}

Text load_text(char* message) {
  Text output;
  output.buffer = C2D_TextBufNew(10);

  C2D_TextFontParse(&output.text, MAIN_FONT, output.buffer, message);
  C2D_TextOptimize(&output.text);

  return output;
}

bool initialize_save_file(void) {
  FILE* save_file = fopen("sdmc:/3ds/evangelion_angel_attack/save_file.json", "w");
  if (save_file == NULL) {
    log_error("ERROR: Failed to create save file");
    return false;
  }
  
  SaveFile save_data;
  
  save_data.main.main_game_beat = false;
  save_data.main.angels_beat = 0;
  save_data.main.current_pilot = "null";
  save_data.main.endless_highscore = 0;
  save_data.main.level_points = 0;

  save_data.pilots.shinji.ability_slots = 1;
  save_data.pilots.shinji.current_skin = "default_skin";
  save_data.pilots.shinji.mastery_unlocked = false;
  save_data.pilots.shinji.unlocked = false;

  save_data.pilots.shinji.mastery.ability_slots = 1;
  save_data.pilots.shinji.mastery.angels_beat_hard = 0;
  save_data.pilots.shinji.mastery.beat_1_helpless = false;
  save_data.pilots.shinji.mastery.max_stats = false;

  save_data.pilots.shinji.active_abilities[0] = "divine_intervention";
  save_data.pilots.shinji.active_abilities[1] = "null";
  save_data.pilots.shinji.active_abilities[2] = "null";
  save_data.pilots.shinji.active_abilities[3] = "null";

  save_data.pilots.shinji.unlocked_abilities.divine_intervention = true;
  save_data.pilots.shinji.unlocked_abilities.miracle_sync = false;
  save_data.pilots.shinji.unlocked_abilities.celestial_gift = false;
  save_data.pilots.shinji.unlocked_abilities.ruthless = false;
  save_data.pilots.shinji.unlocked_abilities.counter = false;
  save_data.pilots.shinji.unlocked_abilities.healing_mist = false;
  save_data.pilots.shinji.unlocked_abilities.absolute_accuracy = false;
  save_data.pilots.shinji.unlocked_abilities.overclock = false;
  save_data.pilots.shinji.unlocked_abilities.double_or_nothing = false;
  save_data.pilots.shinji.unlocked_abilities.eek_run = false;

  save_data.pilots.shinji.stats.at_field = 5;
  save_data.pilots.shinji.stats.attack = 5;
  save_data.pilots.shinji.stats.bond = 15;
  save_data.pilots.shinji.stats.default_sync_rate = 50;
  save_data.pilots.shinji.stats.max_health = 100;

  save_data.pilots.shinji.unlocked_skins.default_skin = true;
  save_data.pilots.shinji.unlocked_skins.master = false;
  

  save_data.pilots.asuka.ability_slots = 1;
  save_data.pilots.asuka.current_skin = "default_skin";
  save_data.pilots.asuka.mastery_unlocked = false;
  save_data.pilots.asuka.unlocked = false;

  save_data.pilots.asuka.mastery.ability_slots = 1;
  save_data.pilots.asuka.mastery.angels_beat_hard = 0;
  save_data.pilots.asuka.mastery.beat_1_helpless = false;
  save_data.pilots.asuka.mastery.max_stats = false;

  save_data.pilots.asuka.active_abilities[0] = "miracle_sync";
  save_data.pilots.asuka.active_abilities[1] = "null";
  save_data.pilots.asuka.active_abilities[2] = "null";
  save_data.pilots.asuka.active_abilities[3] = "null";

  save_data.pilots.asuka.unlocked_abilities.divine_intervention = false;
  save_data.pilots.asuka.unlocked_abilities.miracle_sync = true;
  save_data.pilots.asuka.unlocked_abilities.celestial_gift = false;
  save_data.pilots.asuka.unlocked_abilities.ruthless = false;
  save_data.pilots.asuka.unlocked_abilities.counter = false;
  save_data.pilots.asuka.unlocked_abilities.healing_mist = false;
  save_data.pilots.asuka.unlocked_abilities.absolute_accuracy = false;
  save_data.pilots.asuka.unlocked_abilities.overclock = false;
  save_data.pilots.asuka.unlocked_abilities.double_or_nothing = false;
  save_data.pilots.asuka.unlocked_abilities.eek_run = false;

  save_data.pilots.asuka.stats.at_field = 5;
  save_data.pilots.asuka.stats.attack = 5;
  save_data.pilots.asuka.stats.bond = 5;
  save_data.pilots.asuka.stats.default_sync_rate = 65;
  save_data.pilots.asuka.stats.max_health = 100;

  save_data.pilots.asuka.unlocked_skins.default_skin = true;
  save_data.pilots.asuka.unlocked_skins.master = false;


  save_data.pilots.rei.ability_slots = 1;
  save_data.pilots.rei.current_skin = "default_skin";
  save_data.pilots.rei.mastery_unlocked = false;
  save_data.pilots.rei.unlocked = false;

  save_data.pilots.rei.mastery.ability_slots = 1;
  save_data.pilots.rei.mastery.angels_beat_hard = 0;
  save_data.pilots.rei.mastery.beat_1_helpless = false;
  save_data.pilots.rei.mastery.max_stats = false;

  save_data.pilots.rei.active_abilities[0] = "celestial_gift";
  save_data.pilots.rei.active_abilities[1] = "null";
  save_data.pilots.rei.active_abilities[2] = "null";
  save_data.pilots.rei.active_abilities[3] = "null";

  save_data.pilots.rei.unlocked_abilities.divine_intervention = false;
  save_data.pilots.rei.unlocked_abilities.miracle_sync = false;
  save_data.pilots.rei.unlocked_abilities.celestial_gift = true;
  save_data.pilots.rei.unlocked_abilities.ruthless = false;
  save_data.pilots.rei.unlocked_abilities.counter = false;
  save_data.pilots.rei.unlocked_abilities.healing_mist = false;
  save_data.pilots.rei.unlocked_abilities.absolute_accuracy = false;
  save_data.pilots.rei.unlocked_abilities.overclock = false;
  save_data.pilots.rei.unlocked_abilities.double_or_nothing = false;
  save_data.pilots.rei.unlocked_abilities.eek_run = false;

  save_data.pilots.rei.stats.at_field = 15;
  save_data.pilots.rei.stats.attack = 5;
  save_data.pilots.rei.stats.bond = 5;
  save_data.pilots.rei.stats.default_sync_rate = 50;
  save_data.pilots.rei.stats.max_health = 100;

  save_data.pilots.rei.unlocked_skins.default_skin = true;
  save_data.pilots.rei.unlocked_skins.master = false;


  cJSON* root = cJSON_CreateObject();

  cJSON* main = cJSON_CreateObject();

  cJSON_AddItemToObject(root, "main", main);
  
  cJSON_AddStringToObject(main, "current_pilot", save_data.main.current_pilot);
  cJSON_AddNumberToObject(main, "angels_beat", save_data.main.angels_beat);
  cJSON_AddBoolToObject(main, "main_game_beat", save_data.main.main_game_beat);
  cJSON_AddNumberToObject(main, "level_points", save_data.main.level_points);
  cJSON_AddNumberToObject(main, "endless_highscore", save_data.main.endless_highscore);
  
  cJSON* pilots = cJSON_CreateObject();
  cJSON_AddItemToObject(root, "pilots", pilots);

  cJSON* shinji = cJSON_CreateObject();
  cJSON_AddItemToObject(pilots, "shinji", shinji);

  cJSON_AddBoolToObject(shinji, "mastery_unlocked", save_data.pilots.shinji.mastery_unlocked);
  cJSON_AddNumberToObject(shinji, "ability_slots", save_data.pilots.shinji.ability_slots);
  cJSON_AddBoolToObject(shinji, "unlocked", save_data.pilots.shinji.unlocked);
  cJSON_AddStringToObject(shinji, "current_skin", save_data.pilots.shinji.current_skin);

  cJSON* shinji_stats = cJSON_CreateObject();
  cJSON_AddItemToObject(shinji, "stats", shinji_stats);

  cJSON_AddNumberToObject(shinji_stats, "max_health", save_data.pilots.shinji.stats.max_health);
  cJSON_AddNumberToObject(shinji_stats, "attack", save_data.pilots.shinji.stats.attack);
  cJSON_AddNumberToObject(shinji_stats, "at_field", save_data.pilots.shinji.stats.at_field);
  cJSON_AddNumberToObject(shinji_stats, "default_sync_rate", save_data.pilots.shinji.stats.default_sync_rate);
  cJSON_AddNumberToObject(shinji_stats, "bond", save_data.pilots.shinji.stats.bond);

  cJSON* shinji_active_abilities = cJSON_CreateArray();
  cJSON_AddItemToObject(shinji, "active_abilities", shinji_active_abilities);

  for (int i = 0; i < 4; i++) {
    cJSON_AddItemToArray(shinji_active_abilities, cJSON_CreateString(save_data.pilots.shinji.active_abilities[i]));
  }

  cJSON* shinji_unlocked_abilities = cJSON_CreateObject();
  cJSON_AddItemToObject(shinji, "unlocked_abilities", shinji_unlocked_abilities);

  cJSON_AddBoolToObject(shinji_unlocked_abilities, "divine_intervention", save_data.pilots.shinji.unlocked_abilities.divine_intervention);
  cJSON_AddBoolToObject(shinji_unlocked_abilities, "miracle_sync", save_data.pilots.shinji.unlocked_abilities.miracle_sync);
  cJSON_AddBoolToObject(shinji_unlocked_abilities, "celestial_gift", save_data.pilots.shinji.unlocked_abilities.celestial_gift);
  cJSON_AddBoolToObject(shinji_unlocked_abilities, "ruthless", save_data.pilots.shinji.unlocked_abilities.ruthless);
  cJSON_AddBoolToObject(shinji_unlocked_abilities, "counter", save_data.pilots.shinji.unlocked_abilities.counter);
  cJSON_AddBoolToObject(shinji_unlocked_abilities, "healing_mist", save_data.pilots.shinji.unlocked_abilities.healing_mist);
  cJSON_AddBoolToObject(shinji_unlocked_abilities, "absolute_accuracy", save_data.pilots.shinji.unlocked_abilities.absolute_accuracy);
  cJSON_AddBoolToObject(shinji_unlocked_abilities, "overclock", save_data.pilots.shinji.unlocked_abilities.overclock);
  cJSON_AddBoolToObject(shinji_unlocked_abilities, "double_or_nothing", save_data.pilots.shinji.unlocked_abilities.double_or_nothing);
  cJSON_AddBoolToObject(shinji_unlocked_abilities, "eek_run", save_data.pilots.shinji.unlocked_abilities.eek_run);

  cJSON* shinji_unlocked_skins = cJSON_CreateObject();
  cJSON_AddItemToObject(shinji, "unlocked_skins", shinji_unlocked_skins);

  cJSON_AddBoolToObject(shinji_unlocked_skins, "default_skin", save_data.pilots.shinji.unlocked_skins.default_skin);
  cJSON_AddBoolToObject(shinji_unlocked_skins, "master", save_data.pilots.shinji.unlocked_skins.master);

  cJSON* shinji_mastery = cJSON_CreateObject();
  cJSON_AddItemToObject(shinji, "mastery", shinji_mastery);

  cJSON_AddNumberToObject(shinji_mastery, "angels_beat_hard", save_data.pilots.shinji.mastery.angels_beat_hard);
  cJSON_AddNumberToObject(shinji_mastery, "ability_slots", save_data.pilots.shinji.mastery.ability_slots);
  cJSON_AddBoolToObject(shinji_mastery, "max_stats", save_data.pilots.shinji.mastery.max_stats);
  cJSON_AddBoolToObject(shinji_mastery, "beat_1_helpless", save_data.pilots.shinji.mastery.beat_1_helpless);


  cJSON* asuka = cJSON_CreateObject();
  cJSON_AddItemToObject(pilots, "asuka", asuka);

  cJSON_AddBoolToObject(asuka, "mastery_unlocked", save_data.pilots.asuka.mastery_unlocked);
  cJSON_AddNumberToObject(asuka, "ability_slots", save_data.pilots.asuka.ability_slots);
  cJSON_AddBoolToObject(asuka, "unlocked", save_data.pilots.asuka.unlocked);
  cJSON_AddStringToObject(asuka, "current_skin", save_data.pilots.asuka.current_skin);

  cJSON* asuka_stats = cJSON_CreateObject();
  cJSON_AddItemToObject(asuka, "stats", asuka_stats);

  cJSON_AddNumberToObject(asuka_stats, "max_health", save_data.pilots.asuka.stats.max_health);
  cJSON_AddNumberToObject(asuka_stats, "attack", save_data.pilots.asuka.stats.attack);
  cJSON_AddNumberToObject(asuka_stats, "at_field", save_data.pilots.asuka.stats.at_field);
  cJSON_AddNumberToObject(asuka_stats, "default_sync_rate", save_data.pilots.asuka.stats.default_sync_rate);
  cJSON_AddNumberToObject(asuka_stats, "bond", save_data.pilots.asuka.stats.bond);

  
  cJSON* asuka_active_abilities = cJSON_CreateArray();
  cJSON_AddItemToObject(asuka, "active_abilities", asuka_active_abilities);

  for (int i = 0; i < 4; i++) {
    cJSON_AddItemToArray(asuka_active_abilities, cJSON_CreateString(save_data.pilots.asuka.active_abilities[i]));
  }

  cJSON* asuka_unlocked_abilities = cJSON_CreateObject();
  cJSON_AddItemToObject(asuka, "unlocked_abilities", asuka_unlocked_abilities);

  cJSON_AddBoolToObject(asuka_unlocked_abilities, "divine_intervention", save_data.pilots.asuka.unlocked_abilities.divine_intervention);
  cJSON_AddBoolToObject(asuka_unlocked_abilities, "miracle_sync", save_data.pilots.asuka.unlocked_abilities.miracle_sync);
  cJSON_AddBoolToObject(asuka_unlocked_abilities, "celestial_gift", save_data.pilots.asuka.unlocked_abilities.celestial_gift);
  cJSON_AddBoolToObject(asuka_unlocked_abilities, "ruthless", save_data.pilots.asuka.unlocked_abilities.ruthless);
  cJSON_AddBoolToObject(asuka_unlocked_abilities, "counter", save_data.pilots.asuka.unlocked_abilities.counter);
  cJSON_AddBoolToObject(asuka_unlocked_abilities, "healing_mist", save_data.pilots.asuka.unlocked_abilities.healing_mist);
  cJSON_AddBoolToObject(asuka_unlocked_abilities, "absolute_accuracy", save_data.pilots.asuka.unlocked_abilities.absolute_accuracy);
  cJSON_AddBoolToObject(asuka_unlocked_abilities, "overclock", save_data.pilots.asuka.unlocked_abilities.overclock);
  cJSON_AddBoolToObject(asuka_unlocked_abilities, "double_or_nothing", save_data.pilots.asuka.unlocked_abilities.double_or_nothing);
  cJSON_AddBoolToObject(asuka_unlocked_abilities, "eek_run", save_data.pilots.asuka.unlocked_abilities.eek_run);

  cJSON* asuka_unlocked_skins = cJSON_CreateObject();
  cJSON_AddItemToObject(asuka, "unlocked_skins", asuka_unlocked_skins);

  cJSON_AddBoolToObject(asuka_unlocked_skins, "default_skin", save_data.pilots.asuka.unlocked_skins.default_skin);
  cJSON_AddBoolToObject(asuka_unlocked_skins, "master", save_data.pilots.asuka.unlocked_skins.master);

  cJSON* asuka_mastery = cJSON_CreateObject();
  cJSON_AddItemToObject(asuka, "mastery", asuka_mastery);

  cJSON_AddNumberToObject(asuka_mastery, "angels_beat_hard", save_data.pilots.asuka.mastery.angels_beat_hard);
  cJSON_AddNumberToObject(asuka_mastery, "ability_slots", save_data.pilots.asuka.mastery.ability_slots);
  cJSON_AddBoolToObject(asuka_mastery, "max_stats", save_data.pilots.asuka.mastery.max_stats);
  cJSON_AddBoolToObject(asuka_mastery, "beat_1_helpless", save_data.pilots.asuka.mastery.beat_1_helpless);
  

  cJSON* rei = cJSON_CreateObject();
  cJSON_AddItemToObject(pilots, "rei", rei);

  cJSON_AddBoolToObject(rei, "mastery_unlocked", save_data.pilots.rei.mastery_unlocked);
  cJSON_AddNumberToObject(rei, "ability_slots", save_data.pilots.rei.ability_slots);
  cJSON_AddBoolToObject(rei, "unlocked", save_data.pilots.rei.unlocked);
  cJSON_AddStringToObject(rei, "current_skin", save_data.pilots.rei.current_skin);

  cJSON* rei_stats = cJSON_CreateObject();
  cJSON_AddItemToObject(rei, "stats", rei_stats);

  cJSON_AddNumberToObject(rei_stats, "max_health", save_data.pilots.rei.stats.max_health);
  cJSON_AddNumberToObject(rei_stats, "attack", save_data.pilots.rei.stats.attack);
  cJSON_AddNumberToObject(rei_stats, "at_field", save_data.pilots.rei.stats.at_field);
  cJSON_AddNumberToObject(rei_stats, "default_sync_rate", save_data.pilots.rei.stats.default_sync_rate);
  cJSON_AddNumberToObject(rei_stats, "bond", save_data.pilots.rei.stats.bond);

  cJSON* rei_active_abilities = cJSON_CreateArray();
  cJSON_AddItemToObject(rei, "active_abilities", rei_active_abilities);

  for (int i = 0; i < 4; i++) {
    cJSON_AddItemToArray(rei_active_abilities, cJSON_CreateString(save_data.pilots.rei.active_abilities[i]));
  }

  cJSON* rei_unlocked_abilities = cJSON_CreateObject();
  cJSON_AddItemToObject(rei, "unlocked_abilities", rei_unlocked_abilities);

  cJSON_AddBoolToObject(rei_unlocked_abilities, "divine_intervention", save_data.pilots.rei.unlocked_abilities.divine_intervention);
  cJSON_AddBoolToObject(rei_unlocked_abilities, "miracle_sync", save_data.pilots.rei.unlocked_abilities.miracle_sync);
  cJSON_AddBoolToObject(rei_unlocked_abilities, "celestial_gift", save_data.pilots.rei.unlocked_abilities.celestial_gift);
  cJSON_AddBoolToObject(rei_unlocked_abilities, "ruthless", save_data.pilots.rei.unlocked_abilities.ruthless);
  cJSON_AddBoolToObject(rei_unlocked_abilities, "counter", save_data.pilots.rei.unlocked_abilities.counter);
  cJSON_AddBoolToObject(rei_unlocked_abilities, "healing_mist", save_data.pilots.rei.unlocked_abilities.healing_mist);
  cJSON_AddBoolToObject(rei_unlocked_abilities, "absolute_accuracy", save_data.pilots.rei.unlocked_abilities.absolute_accuracy);
  cJSON_AddBoolToObject(rei_unlocked_abilities, "overclock", save_data.pilots.rei.unlocked_abilities.overclock);
  cJSON_AddBoolToObject(rei_unlocked_abilities, "double_or_nothing", save_data.pilots.rei.unlocked_abilities.double_or_nothing);
  cJSON_AddBoolToObject(rei_unlocked_abilities, "eek_run", save_data.pilots.rei.unlocked_abilities.eek_run);

  cJSON* rei_unlocked_skins = cJSON_CreateObject();
  cJSON_AddItemToObject(rei, "unlocked_skins", rei_unlocked_skins);

  cJSON_AddBoolToObject(rei_unlocked_skins, "default_skin", save_data.pilots.rei.unlocked_skins.default_skin);
  cJSON_AddBoolToObject(rei_unlocked_skins, "master", save_data.pilots.rei.unlocked_skins.master);

  cJSON* rei_mastery = cJSON_CreateObject();
  cJSON_AddItemToObject(rei, "mastery", rei_mastery);

  cJSON_AddNumberToObject(rei_mastery, "angels_beat_hard", save_data.pilots.rei.mastery.angels_beat_hard);
  cJSON_AddNumberToObject(rei_mastery, "ability_slots", save_data.pilots.rei.mastery.ability_slots);
  cJSON_AddBoolToObject(rei_mastery, "max_stats", save_data.pilots.rei.mastery.max_stats);
  cJSON_AddBoolToObject(rei_mastery, "beat_1_helpless", save_data.pilots.rei.mastery.beat_1_helpless);

  char* json_data = cJSON_Print(root);
  fputs(json_data, save_file);
  free(json_data);

  cJSON_Delete(root);
  fclose(save_file);
  return true;
}

bool write_to_save_file(SaveFile save_data) {
  FILE* save_file = fopen("sdmc:/3ds/evangelion_angel_attack/save_file.json", "w");
  if (save_file == NULL) {
    log_error("ERROR: Failed to create save file");
    return false;
  }

  cJSON* root = cJSON_CreateObject();

  cJSON* main = cJSON_CreateObject();

  cJSON_AddItemToObject(root, "main", main);
  
  cJSON_AddStringToObject(main, "current_pilot", save_data.main.current_pilot);
  cJSON_AddNumberToObject(main, "angels_beat", save_data.main.angels_beat);
  cJSON_AddBoolToObject(main, "main_game_beat", save_data.main.main_game_beat);
  cJSON_AddNumberToObject(main, "level_points", save_data.main.level_points);
  cJSON_AddNumberToObject(main, "endless_highscore", save_data.main.endless_highscore);
  
  cJSON* pilots = cJSON_CreateObject();
  cJSON_AddItemToObject(root, "pilots", pilots);

  cJSON* shinji = cJSON_CreateObject();
  cJSON_AddItemToObject(pilots, "shinji", shinji);

  cJSON_AddBoolToObject(shinji, "mastery_unlocked", save_data.pilots.shinji.mastery_unlocked);
  cJSON_AddNumberToObject(shinji, "ability_slots", save_data.pilots.shinji.ability_slots);
  cJSON_AddBoolToObject(shinji, "unlocked", save_data.pilots.shinji.unlocked);
  cJSON_AddStringToObject(shinji, "current_skin", save_data.pilots.shinji.current_skin);

  cJSON* shinji_stats = cJSON_CreateObject();
  cJSON_AddItemToObject(shinji, "stats", shinji_stats);

  cJSON_AddNumberToObject(shinji_stats, "max_health", save_data.pilots.shinji.stats.max_health);
  cJSON_AddNumberToObject(shinji_stats, "attack", save_data.pilots.shinji.stats.attack);
  cJSON_AddNumberToObject(shinji_stats, "at_field", save_data.pilots.shinji.stats.at_field);
  cJSON_AddNumberToObject(shinji_stats, "default_sync_rate", save_data.pilots.shinji.stats.default_sync_rate);
  cJSON_AddNumberToObject(shinji_stats, "bond", save_data.pilots.shinji.stats.bond);

  cJSON* shinji_active_abilities = cJSON_CreateArray();
  cJSON_AddItemToObject(shinji, "active_abilities", shinji_active_abilities);

  for (int i = 0; i < 4; i++) {
    cJSON_AddItemToArray(shinji_active_abilities, cJSON_CreateString(save_data.pilots.shinji.active_abilities[i]));
  }

  cJSON* shinji_unlocked_abilities = cJSON_CreateObject();
  cJSON_AddItemToObject(shinji, "unlocked_abilities", shinji_unlocked_abilities);

  cJSON_AddBoolToObject(shinji_unlocked_abilities, "divine_intervention", save_data.pilots.shinji.unlocked_abilities.divine_intervention);
  cJSON_AddBoolToObject(shinji_unlocked_abilities, "miracle_sync", save_data.pilots.shinji.unlocked_abilities.miracle_sync);
  cJSON_AddBoolToObject(shinji_unlocked_abilities, "celestial_gift", save_data.pilots.shinji.unlocked_abilities.celestial_gift);
  cJSON_AddBoolToObject(shinji_unlocked_abilities, "ruthless", save_data.pilots.shinji.unlocked_abilities.ruthless);
  cJSON_AddBoolToObject(shinji_unlocked_abilities, "counter", save_data.pilots.shinji.unlocked_abilities.counter);
  cJSON_AddBoolToObject(shinji_unlocked_abilities, "healing_mist", save_data.pilots.shinji.unlocked_abilities.healing_mist);
  cJSON_AddBoolToObject(shinji_unlocked_abilities, "absolute_accuracy", save_data.pilots.shinji.unlocked_abilities.absolute_accuracy);
  cJSON_AddBoolToObject(shinji_unlocked_abilities, "overclock", save_data.pilots.shinji.unlocked_abilities.overclock);
  cJSON_AddBoolToObject(shinji_unlocked_abilities, "double_or_nothing", save_data.pilots.shinji.unlocked_abilities.double_or_nothing);
  cJSON_AddBoolToObject(shinji_unlocked_abilities, "eek_run", save_data.pilots.shinji.unlocked_abilities.eek_run);

  cJSON* shinji_unlocked_skins = cJSON_CreateObject();
  cJSON_AddItemToObject(shinji, "unlocked_skins", shinji_unlocked_skins);

  cJSON_AddBoolToObject(shinji_unlocked_skins, "default_skin", save_data.pilots.shinji.unlocked_skins.default_skin);
  cJSON_AddBoolToObject(shinji_unlocked_skins, "master", save_data.pilots.shinji.unlocked_skins.master);

  cJSON* shinji_mastery = cJSON_CreateObject();
  cJSON_AddItemToObject(shinji, "mastery", shinji_mastery);

  cJSON_AddNumberToObject(shinji_mastery, "angels_beat_hard", save_data.pilots.shinji.mastery.angels_beat_hard);
  cJSON_AddNumberToObject(shinji_mastery, "ability_slots", save_data.pilots.shinji.mastery.ability_slots);
  cJSON_AddBoolToObject(shinji_mastery, "max_stats", save_data.pilots.shinji.mastery.max_stats);
  cJSON_AddBoolToObject(shinji_mastery, "beat_1_helpless", save_data.pilots.shinji.mastery.beat_1_helpless);


  cJSON* asuka = cJSON_CreateObject();
  cJSON_AddItemToObject(pilots, "asuka", asuka);

  cJSON_AddBoolToObject(asuka, "mastery_unlocked", save_data.pilots.asuka.mastery_unlocked);
  cJSON_AddNumberToObject(asuka, "ability_slots", save_data.pilots.asuka.ability_slots);
  cJSON_AddBoolToObject(asuka, "unlocked", save_data.pilots.asuka.unlocked);
  cJSON_AddStringToObject(asuka, "current_skin", save_data.pilots.asuka.current_skin);

  cJSON* asuka_stats = cJSON_CreateObject();
  cJSON_AddItemToObject(asuka, "stats", asuka_stats);

  cJSON_AddNumberToObject(asuka_stats, "max_health", save_data.pilots.asuka.stats.max_health);
  cJSON_AddNumberToObject(asuka_stats, "attack", save_data.pilots.asuka.stats.attack);
  cJSON_AddNumberToObject(asuka_stats, "at_field", save_data.pilots.asuka.stats.at_field);
  cJSON_AddNumberToObject(asuka_stats, "default_sync_rate", save_data.pilots.asuka.stats.default_sync_rate);
  cJSON_AddNumberToObject(asuka_stats, "bond", save_data.pilots.asuka.stats.bond);

  
  cJSON* asuka_active_abilities = cJSON_CreateArray();
  cJSON_AddItemToObject(asuka, "active_abilities", asuka_active_abilities);

  for (int i = 0; i < 4; i++) {
    cJSON_AddItemToArray(asuka_active_abilities, cJSON_CreateString(save_data.pilots.asuka.active_abilities[i]));
  }

  cJSON* asuka_unlocked_abilities = cJSON_CreateObject();
  cJSON_AddItemToObject(asuka, "unlocked_abilities", asuka_unlocked_abilities);

  cJSON_AddBoolToObject(asuka_unlocked_abilities, "divine_intervention", save_data.pilots.asuka.unlocked_abilities.divine_intervention);
  cJSON_AddBoolToObject(asuka_unlocked_abilities, "miracle_sync", save_data.pilots.asuka.unlocked_abilities.miracle_sync);
  cJSON_AddBoolToObject(asuka_unlocked_abilities, "celestial_gift", save_data.pilots.asuka.unlocked_abilities.celestial_gift);
  cJSON_AddBoolToObject(asuka_unlocked_abilities, "ruthless", save_data.pilots.asuka.unlocked_abilities.ruthless);
  cJSON_AddBoolToObject(asuka_unlocked_abilities, "counter", save_data.pilots.asuka.unlocked_abilities.counter);
  cJSON_AddBoolToObject(asuka_unlocked_abilities, "healing_mist", save_data.pilots.asuka.unlocked_abilities.healing_mist);
  cJSON_AddBoolToObject(asuka_unlocked_abilities, "absolute_accuracy", save_data.pilots.asuka.unlocked_abilities.absolute_accuracy);
  cJSON_AddBoolToObject(asuka_unlocked_abilities, "overclock", save_data.pilots.asuka.unlocked_abilities.overclock);
  cJSON_AddBoolToObject(asuka_unlocked_abilities, "double_or_nothing", save_data.pilots.asuka.unlocked_abilities.double_or_nothing);
  cJSON_AddBoolToObject(asuka_unlocked_abilities, "eek_run", save_data.pilots.asuka.unlocked_abilities.eek_run);

  cJSON* asuka_unlocked_skins = cJSON_CreateObject();
  cJSON_AddItemToObject(asuka, "unlocked_skins", asuka_unlocked_skins);

  cJSON_AddBoolToObject(asuka_unlocked_skins, "default_skin", save_data.pilots.asuka.unlocked_skins.default_skin);
  cJSON_AddBoolToObject(asuka_unlocked_skins, "master", save_data.pilots.asuka.unlocked_skins.master);

  cJSON* asuka_mastery = cJSON_CreateObject();
  cJSON_AddItemToObject(asuka, "mastery", asuka_mastery);

  cJSON_AddNumberToObject(asuka_mastery, "angels_beat_hard", save_data.pilots.asuka.mastery.angels_beat_hard);
  cJSON_AddNumberToObject(asuka_mastery, "ability_slots", save_data.pilots.asuka.mastery.ability_slots);
  cJSON_AddBoolToObject(asuka_mastery, "max_stats", save_data.pilots.asuka.mastery.max_stats);
  cJSON_AddBoolToObject(asuka_mastery, "beat_1_helpless", save_data.pilots.asuka.mastery.beat_1_helpless);
  

  cJSON* rei = cJSON_CreateObject();
  cJSON_AddItemToObject(pilots, "rei", rei);

  cJSON_AddBoolToObject(rei, "mastery_unlocked", save_data.pilots.rei.mastery_unlocked);
  cJSON_AddNumberToObject(rei, "ability_slots", save_data.pilots.rei.ability_slots);
  cJSON_AddBoolToObject(rei, "unlocked", save_data.pilots.rei.unlocked);
  cJSON_AddStringToObject(rei, "current_skin", save_data.pilots.rei.current_skin);

  cJSON* rei_stats = cJSON_CreateObject();
  cJSON_AddItemToObject(rei, "stats", rei_stats);

  cJSON_AddNumberToObject(rei_stats, "max_health", save_data.pilots.rei.stats.max_health);
  cJSON_AddNumberToObject(rei_stats, "attack", save_data.pilots.rei.stats.attack);
  cJSON_AddNumberToObject(rei_stats, "at_field", save_data.pilots.rei.stats.at_field);
  cJSON_AddNumberToObject(rei_stats, "default_sync_rate", save_data.pilots.rei.stats.default_sync_rate);
  cJSON_AddNumberToObject(rei_stats, "bond", save_data.pilots.rei.stats.bond);

  cJSON* rei_active_abilities = cJSON_CreateArray();
  cJSON_AddItemToObject(rei, "active_abilities", rei_active_abilities);

  for (int i = 0; i < 4; i++) {
    cJSON_AddItemToArray(rei_active_abilities, cJSON_CreateString(save_data.pilots.rei.active_abilities[i]));
  }

  cJSON* rei_unlocked_abilities = cJSON_CreateObject();
  cJSON_AddItemToObject(rei, "unlocked_abilities", rei_unlocked_abilities);

  cJSON_AddBoolToObject(rei_unlocked_abilities, "divine_intervention", save_data.pilots.rei.unlocked_abilities.divine_intervention);
  cJSON_AddBoolToObject(rei_unlocked_abilities, "miracle_sync", save_data.pilots.rei.unlocked_abilities.miracle_sync);
  cJSON_AddBoolToObject(rei_unlocked_abilities, "celestial_gift", save_data.pilots.rei.unlocked_abilities.celestial_gift);
  cJSON_AddBoolToObject(rei_unlocked_abilities, "ruthless", save_data.pilots.rei.unlocked_abilities.ruthless);
  cJSON_AddBoolToObject(rei_unlocked_abilities, "counter", save_data.pilots.rei.unlocked_abilities.counter);
  cJSON_AddBoolToObject(rei_unlocked_abilities, "healing_mist", save_data.pilots.rei.unlocked_abilities.healing_mist);
  cJSON_AddBoolToObject(rei_unlocked_abilities, "absolute_accuracy", save_data.pilots.rei.unlocked_abilities.absolute_accuracy);
  cJSON_AddBoolToObject(rei_unlocked_abilities, "overclock", save_data.pilots.rei.unlocked_abilities.overclock);
  cJSON_AddBoolToObject(rei_unlocked_abilities, "double_or_nothing", save_data.pilots.rei.unlocked_abilities.double_or_nothing);
  cJSON_AddBoolToObject(rei_unlocked_abilities, "eek_run", save_data.pilots.rei.unlocked_abilities.eek_run);

  cJSON* rei_unlocked_skins = cJSON_CreateObject();
  cJSON_AddItemToObject(rei, "unlocked_skins", rei_unlocked_skins);

  cJSON_AddBoolToObject(rei_unlocked_skins, "default_skin", save_data.pilots.rei.unlocked_skins.default_skin);
  cJSON_AddBoolToObject(rei_unlocked_skins, "master", save_data.pilots.rei.unlocked_skins.master);

  cJSON* rei_mastery = cJSON_CreateObject();
  cJSON_AddItemToObject(rei, "mastery", rei_mastery);

  cJSON_AddNumberToObject(rei_mastery, "angels_beat_hard", save_data.pilots.rei.mastery.angels_beat_hard);
  cJSON_AddNumberToObject(rei_mastery, "ability_slots", save_data.pilots.rei.mastery.ability_slots);
  cJSON_AddBoolToObject(rei_mastery, "max_stats", save_data.pilots.rei.mastery.max_stats);
  cJSON_AddBoolToObject(rei_mastery, "beat_1_helpless", save_data.pilots.rei.mastery.beat_1_helpless);

  char* json_data = cJSON_Print(root);
  fputs(json_data, save_file);
  free(json_data);

  cJSON_Delete(root);
  fclose(save_file);
  return true;
}

SaveFile load_save_file(void) {
  SaveFile output;
  output.valid = false;
  FILE* save_file = fopen("sdmc:/3ds/evangelion_angel_attack/save_file.json", "rb");
  if (save_file == NULL) {
    log_error("ERROR: Failed to open sdmc:/3ds/evangelion_angel_attack/save_file.json");
    return output;
  }
  fseek(save_file, 0, SEEK_END);
  long file_size = ftell(save_file);
  rewind(save_file);
  char* file_buffer = malloc(file_size+1);
  if (file_buffer == NULL) {
    log_error("ERROR: Failed to allocate file buffer");
    return output;
  }
  fread(file_buffer, 1, file_size, save_file);
  file_buffer[file_size] = '\0';
  fclose(save_file);


  cJSON* root = cJSON_Parse(file_buffer);
  free(file_buffer);
  if (root == NULL) {
    log_error("ERROR: Failed to parse JSON save file");
    return output;
  }

  cJSON* main = cJSON_GetObjectItemCaseSensitive(root, "main");
  
  output.main.main_game_beat = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(main, "main_game_beat"));
  output.main.angels_beat = cJSON_GetObjectItemCaseSensitive(main, "angels_beat")->valueint;
  output.main.current_pilot = strdup(cJSON_GetObjectItemCaseSensitive(main, "current_pilot")->valuestring);
  output.main.endless_highscore = cJSON_GetObjectItemCaseSensitive(main, "endless_highscore")->valueint;
  output.main.level_points = cJSON_GetObjectItemCaseSensitive(main, "level_points")->valueint;
  if (output.main.level_points < 0) { output.main.level_points = 0; }
  else if (output.main.level_points > 9999) { output.main.level_points = 9999; }

  cJSON* pilots = cJSON_GetObjectItemCaseSensitive(root, "pilots");

  cJSON* shinji = cJSON_GetObjectItemCaseSensitive(pilots, "shinji");

  output.pilots.shinji.mastery_unlocked = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(shinji, "mastery_unlocked"));
  output.pilots.shinji.ability_slots = cJSON_GetObjectItemCaseSensitive(shinji, "ability_slots")->valueint;
  output.pilots.shinji.unlocked = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(shinji, "unlocked"));
  output.pilots.shinji.current_skin = strdup(cJSON_GetObjectItemCaseSensitive(shinji, "current_skin")->valuestring);
  
  cJSON* shinji_active_abilities = cJSON_GetObjectItemCaseSensitive(shinji, "active_abilities");
  int shinji_active_abilities_count = cJSON_GetArraySize(shinji_active_abilities);

  output.pilots.shinji.active_abilities[0] = "null";
  output.pilots.shinji.active_abilities[1] = "null";
  output.pilots.shinji.active_abilities[2] = "null";
  output.pilots.shinji.active_abilities[3] = "null";

  for (int i = 0; i < shinji_active_abilities_count; i++) {
    output.pilots.shinji.active_abilities[i] = strdup(cJSON_GetArrayItem(shinji_active_abilities, i)->valuestring);
  }

  cJSON* shinji_unlocked_abilities = cJSON_GetObjectItemCaseSensitive(shinji, "unlocked_abilities");

  output.pilots.shinji.unlocked_abilities.divine_intervention = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(shinji_unlocked_abilities, "divine_intervention"));
  output.pilots.shinji.unlocked_abilities.miracle_sync = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(shinji_unlocked_abilities, "miracle_sync"));
  output.pilots.shinji.unlocked_abilities.celestial_gift = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(shinji_unlocked_abilities, "celestial_gift"));
  output.pilots.shinji.unlocked_abilities.ruthless = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(shinji_unlocked_abilities, "ruthless"));
  output.pilots.shinji.unlocked_abilities.counter = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(shinji_unlocked_abilities, "counter"));
  output.pilots.shinji.unlocked_abilities.healing_mist = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(shinji_unlocked_abilities, "healing_mist"));
  output.pilots.shinji.unlocked_abilities.absolute_accuracy = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(shinji_unlocked_abilities, "absolute_accuracy"));
  output.pilots.shinji.unlocked_abilities.overclock = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(shinji_unlocked_abilities, "overclock"));
  output.pilots.shinji.unlocked_abilities.double_or_nothing = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(shinji_unlocked_abilities, "double_or_nothing"));
  output.pilots.shinji.unlocked_abilities.eek_run = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(shinji_unlocked_abilities, "eek_run"));

  cJSON* shinji_unlocked_skins = cJSON_GetObjectItemCaseSensitive(shinji, "unlocked_skins");

  output.pilots.shinji.unlocked_skins.default_skin = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(shinji_unlocked_skins, "default_skin"));
  output.pilots.shinji.unlocked_skins.master = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(shinji_unlocked_skins, "master"));

  cJSON* shinji_mastery = cJSON_GetObjectItemCaseSensitive(shinji, "mastery");

  output.pilots.shinji.mastery.ability_slots = cJSON_GetObjectItemCaseSensitive(shinji_mastery, "ability_slots")->valueint;
  output.pilots.shinji.mastery.angels_beat_hard = cJSON_GetObjectItemCaseSensitive(shinji_mastery, "angels_beat_hard")->valueint;
  output.pilots.shinji.mastery.beat_1_helpless = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(shinji_mastery, "beat_1_helpless"));
  output.pilots.shinji.mastery.max_stats = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(shinji_mastery, "max_stats"));

  cJSON* shinji_stats = cJSON_GetObjectItemCaseSensitive(shinji, "stats");

  output.pilots.shinji.stats.at_field = cJSON_GetObjectItemCaseSensitive(shinji_stats, "at_field")->valueint;
  output.pilots.shinji.stats.attack = cJSON_GetObjectItemCaseSensitive(shinji_stats, "attack")->valueint;
  output.pilots.shinji.stats.bond = cJSON_GetObjectItemCaseSensitive(shinji_stats, "bond")->valueint;
  output.pilots.shinji.stats.default_sync_rate = cJSON_GetObjectItemCaseSensitive(shinji_stats, "default_sync_rate")->valueint;
  output.pilots.shinji.stats.max_health = cJSON_GetObjectItemCaseSensitive(shinji_stats, "max_health")->valueint;


  cJSON* asuka = cJSON_GetObjectItemCaseSensitive(pilots, "asuka");

  output.pilots.asuka.mastery_unlocked = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(asuka, "mastery_unlocked"));
  output.pilots.asuka.ability_slots = cJSON_GetObjectItemCaseSensitive(asuka, "ability_slots")->valueint;
  output.pilots.asuka.unlocked = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(asuka, "unlocked"));
  output.pilots.asuka.current_skin = strdup(cJSON_GetObjectItemCaseSensitive(asuka, "current_skin")->valuestring);
  
  cJSON* asuka_active_abilities = cJSON_GetObjectItemCaseSensitive(asuka, "active_abilities");
  int asuka_active_abilities_count = cJSON_GetArraySize(asuka_active_abilities);

  output.pilots.asuka.active_abilities[0] = "null";
  output.pilots.asuka.active_abilities[1] = "null";
  output.pilots.asuka.active_abilities[2] = "null";
  output.pilots.asuka.active_abilities[3] = "null";

  for (int i = 0; i < asuka_active_abilities_count; i++) {
    output.pilots.asuka.active_abilities[i] = strdup(cJSON_GetArrayItem(asuka_active_abilities, i)->valuestring);
  }

  cJSON* asuka_unlocked_abilities = cJSON_GetObjectItemCaseSensitive(asuka, "unlocked_abilities");

  output.pilots.asuka.unlocked_abilities.divine_intervention = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(asuka_unlocked_abilities, "divine_intervention"));
  output.pilots.asuka.unlocked_abilities.miracle_sync = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(asuka_unlocked_abilities, "miracle_sync"));
  output.pilots.asuka.unlocked_abilities.celestial_gift = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(asuka_unlocked_abilities, "celestial_gift"));
  output.pilots.asuka.unlocked_abilities.ruthless = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(asuka_unlocked_abilities, "ruthless"));
  output.pilots.asuka.unlocked_abilities.counter = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(asuka_unlocked_abilities, "counter"));
  output.pilots.asuka.unlocked_abilities.healing_mist = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(asuka_unlocked_abilities, "healing_mist"));
  output.pilots.asuka.unlocked_abilities.absolute_accuracy = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(asuka_unlocked_abilities, "absolute_accuracy"));
  output.pilots.asuka.unlocked_abilities.overclock = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(asuka_unlocked_abilities, "overclock"));
  output.pilots.asuka.unlocked_abilities.double_or_nothing = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(asuka_unlocked_abilities, "double_or_nothing"));
  output.pilots.asuka.unlocked_abilities.eek_run = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(asuka_unlocked_abilities, "eek_run"));

  cJSON* asuka_unlocked_skins = cJSON_GetObjectItemCaseSensitive(asuka, "unlocked_skins");

  output.pilots.asuka.unlocked_skins.default_skin = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(asuka_unlocked_skins, "default_skin"));
  output.pilots.asuka.unlocked_skins.master = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(asuka_unlocked_skins, "master"));

  cJSON* asuka_mastery = cJSON_GetObjectItemCaseSensitive(asuka, "mastery");

  output.pilots.asuka.mastery.ability_slots = cJSON_GetObjectItemCaseSensitive(asuka_mastery, "ability_slots")->valueint;
  output.pilots.asuka.mastery.angels_beat_hard = cJSON_GetObjectItemCaseSensitive(asuka_mastery, "angels_beat_hard")->valueint;
  output.pilots.asuka.mastery.beat_1_helpless = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(asuka_mastery, "beat_1_helpless"));
  output.pilots.asuka.mastery.max_stats = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(asuka_mastery, "max_stats"));

  cJSON* asuka_stats = cJSON_GetObjectItemCaseSensitive(asuka, "stats");

  output.pilots.asuka.stats.at_field = cJSON_GetObjectItemCaseSensitive(asuka_stats, "at_field")->valueint;
  output.pilots.asuka.stats.attack = cJSON_GetObjectItemCaseSensitive(asuka_stats, "attack")->valueint;
  output.pilots.asuka.stats.bond = cJSON_GetObjectItemCaseSensitive(asuka_stats, "bond")->valueint;
  output.pilots.asuka.stats.default_sync_rate = cJSON_GetObjectItemCaseSensitive(asuka_stats, "default_sync_rate")->valueint;
  output.pilots.asuka.stats.max_health = cJSON_GetObjectItemCaseSensitive(asuka_stats, "max_health")->valueint;


  cJSON* rei = cJSON_GetObjectItemCaseSensitive(pilots, "rei");

  output.pilots.rei.mastery_unlocked = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(rei, "mastery_unlocked"));
  output.pilots.rei.ability_slots = cJSON_GetObjectItemCaseSensitive(rei, "ability_slots")->valueint;
  output.pilots.rei.unlocked = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(rei, "unlocked"));
  output.pilots.rei.current_skin = strdup(cJSON_GetObjectItemCaseSensitive(rei, "current_skin")->valuestring);
  
  cJSON* rei_active_abilities = cJSON_GetObjectItemCaseSensitive(rei, "active_abilities");
  int rei_active_abilities_count = cJSON_GetArraySize(rei_active_abilities);

  output.pilots.rei.active_abilities[0] = "null";
  output.pilots.rei.active_abilities[1] = "null";
  output.pilots.rei.active_abilities[2] = "null";
  output.pilots.rei.active_abilities[3] = "null";

  for (int i = 0; i < rei_active_abilities_count; i++) {
    output.pilots.rei.active_abilities[i] = strdup(cJSON_GetArrayItem(rei_active_abilities, i)->valuestring);
  }

  cJSON* rei_unlocked_abilities = cJSON_GetObjectItemCaseSensitive(rei, "unlocked_abilities");

  output.pilots.rei.unlocked_abilities.divine_intervention = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(rei_unlocked_abilities, "divine_intervention"));
  output.pilots.rei.unlocked_abilities.miracle_sync = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(rei_unlocked_abilities, "miracle_sync"));
  output.pilots.rei.unlocked_abilities.celestial_gift = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(rei_unlocked_abilities, "celestial_gift"));
  output.pilots.rei.unlocked_abilities.ruthless = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(rei_unlocked_abilities, "ruthless"));
  output.pilots.rei.unlocked_abilities.counter = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(rei_unlocked_abilities, "counter"));
  output.pilots.rei.unlocked_abilities.healing_mist = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(rei_unlocked_abilities, "healing_mist"));
  output.pilots.rei.unlocked_abilities.absolute_accuracy = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(rei_unlocked_abilities, "absolute_accuracy"));
  output.pilots.rei.unlocked_abilities.overclock = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(rei_unlocked_abilities, "overclock"));
  output.pilots.rei.unlocked_abilities.double_or_nothing = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(rei_unlocked_abilities, "double_or_nothing"));
  output.pilots.rei.unlocked_abilities.eek_run = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(rei_unlocked_abilities, "eek_run"));

  cJSON* rei_unlocked_skins = cJSON_GetObjectItemCaseSensitive(rei, "unlocked_skins");

  output.pilots.rei.unlocked_skins.default_skin = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(rei_unlocked_skins, "default_skin"));
  output.pilots.rei.unlocked_skins.master = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(rei_unlocked_skins, "master"));

  cJSON* rei_mastery = cJSON_GetObjectItemCaseSensitive(rei, "mastery");

  output.pilots.rei.mastery.ability_slots = cJSON_GetObjectItemCaseSensitive(rei_mastery, "ability_slots")->valueint;
  output.pilots.rei.mastery.angels_beat_hard = cJSON_GetObjectItemCaseSensitive(rei_mastery, "angels_beat_hard")->valueint;
  output.pilots.rei.mastery.beat_1_helpless = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(rei_mastery, "beat_1_helpless"));
  output.pilots.rei.mastery.max_stats = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(rei_mastery, "max_stats"));

  cJSON* rei_stats = cJSON_GetObjectItemCaseSensitive(rei, "stats");

  output.pilots.rei.stats.at_field = cJSON_GetObjectItemCaseSensitive(rei_stats, "at_field")->valueint;
  output.pilots.rei.stats.attack = cJSON_GetObjectItemCaseSensitive(rei_stats, "attack")->valueint;
  output.pilots.rei.stats.bond = cJSON_GetObjectItemCaseSensitive(rei_stats, "bond")->valueint;
  output.pilots.rei.stats.default_sync_rate = cJSON_GetObjectItemCaseSensitive(rei_stats, "default_sync_rate")->valueint;
  output.pilots.rei.stats.max_health = cJSON_GetObjectItemCaseSensitive(rei_stats, "max_health")->valueint;

  cJSON_Delete(root);
  output.valid = true;
  return output;
}

void system_init(void) {
  romfsInit();
  gfxInitDefault();
  C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
  C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
  C2D_Prepare();
  consoleClear();
  MAIN_FONT = C2D_FontLoad("romfs:/main_font.bcfnt");
}

bool audio_init(void) {
  if (SDL_Init(SDL_INIT_AUDIO) != 0) {
    log_error("SDL_Init Error");
    log_error(SDL_GetError());
    return false;
  }
  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) == -1) {
    log_error("Mix_OpenAudio Error");
    log_error(SDL_GetError());
    return false;
  }
  return true;
}

Colours colours_init(void) {
  Colours colour_palette;
  colour_palette.white = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);
  colour_palette.black = C2D_Color32(0x00, 0x00, 0x00, 0xFF);
  colour_palette.red = C2D_Color32(0xFF, 0x00, 0x00, 0xFF);
  colour_palette.green = C2D_Color32(0x00, 0xFF, 0x00, 0xFF);
  colour_palette.blue = C2D_Color32(0x00, 0x00, 0xFF, 0xFF);
  colour_palette.yellow = C2D_Color32(0xFF, 0xFF, 0x00, 0xFF);
  colour_palette.background = C2D_Color32(0xB, 0x9, 0x20, 0xFF);
  return colour_palette;
}

void system_exit(void) {
  C2D_Fini();
  C3D_Fini();
  Mix_CloseAudio();
  Mix_Quit();
  SDL_Quit();
  romfsExit();
  gfxExit();
  C2D_FontFree(MAIN_FONT);
}

Sprite load_sprite(char* path) {
  Sprite output;
  output.valid = false;

  output.sprite_sheet = C2D_SpriteSheetLoad(path);
  if (!output.sprite_sheet) {
    log_error("ERROR: Could not load sprite");
    log_error(path);
    return output;
  }

  size_t image_count = C2D_SpriteSheetCount(output.sprite_sheet);
  if (image_count == 0) {
    log_error("ERROR: sprite contains no images");
    log_error(path);
    C2D_SpriteSheetFree(output.sprite_sheet);
    return output;
  }

  C2D_Image sprite_image = C2D_SpriteSheetGetImage(output.sprite_sheet, 0);

  C2D_Sprite sprite;
  C2D_SpriteFromImage(&sprite, sprite_image);
  output.sprite = sprite;
  output.valid = true;

  return output;
}

void log_error(char* error_message) {
  FILE* file = fopen("sdmc:/3ds/evangelion_angel_attack/error_log.txt", "a+");
  if (file != NULL) {
    fprintf(file, "%s\n", error_message);
    fclose(file);
  }
}
