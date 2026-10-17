/* states.c - registry of every actor state, used to save games */
#include "def.h"

extern const statetype s_boltbonus;
extern const statetype s_boltbonus2;
extern const statetype s_boltbonus3;
extern const statetype s_nukebonus;
extern const statetype s_nukebonus2;
extern const statetype s_nukebonus3;
extern const statetype s_potionbonus;
extern const statetype s_rkey2bonus;
extern const statetype s_rkeybonus;
extern const statetype s_ykeybonus;
extern const statetype s_gkeybonus;
extern const statetype s_bkeybonus;
extern const statetype s_scrollbonus;
extern const statetype s_chestbonus;
extern const statetype s_waterchestbonus1;
extern const statetype s_waterchestbonus2;
extern const statetype s_rgem1bonus;
extern const statetype s_ygem1bonus;
extern const statetype s_ggem1bonus;
extern const statetype s_bgem1bonus;
extern const statetype s_pgem1bonus;
extern const statetype s_rgem2bonus;
extern const statetype s_ygem2bonus;
extern const statetype s_ggem2bonus;
extern const statetype s_bgem2bonus;
extern const statetype s_pgem2bonus;
extern const statetype s_bonus_die;
extern const statetype s_tombs[3];
extern const statetype s_ftimebonus;
extern const statetype s_ftimebonus2;
extern const statetype s_walldie1;
extern const statetype s_walldie2;
extern const statetype s_walldie3;
extern const statetype s_walldie4;
extern const statetype s_walldie5;
extern const statetype s_walldie6;
extern const statetype s_obj_gate1;
extern const statetype s_obj_gate2;
extern const statetype s_obj_gate3;
extern const statetype s_obj_gate4;
extern const statetype s_pit;
extern const statetype s_trollpause;
extern const statetype s_troll1;
extern const statetype s_troll2;
extern const statetype s_troll3;
extern const statetype s_troll4;
extern const statetype s_trollattack1;
extern const statetype s_trollattack2;
extern const statetype s_trollattack3;
extern const statetype s_trollouch;
extern const statetype s_trolldie1;
extern const statetype s_trolldie2;
extern const statetype s_trolldie3;
extern const statetype s_wet_bubbles1;
extern const statetype s_wet_bubbles2;
extern const statetype s_wet_bubbles3;
extern const statetype s_wet_peek;
extern const statetype s_wet_rise1;
extern const statetype s_wet_rise3;
extern const statetype s_wet_rise4;
extern const statetype s_wet_rise5;
extern const statetype s_wet_sink1;
extern const statetype s_wet_sink2;
extern const statetype s_wet_sink3;
extern const statetype s_wet_walk1;
extern const statetype s_wet_walk2;
extern const statetype s_wet_walk3;
extern const statetype s_wet_walk4;
extern const statetype s_wet_attack1;
extern const statetype s_wet_attack2;
extern const statetype s_wet_attack3;
extern const statetype s_wet_attack4;
extern const statetype s_wet_ouch;
extern const statetype s_wet_die1;
extern const statetype s_wet_die2;
extern const statetype s_wet_die3;
extern const statetype s_wet_die4;
extern const statetype s_wet_die5;
extern const statetype s_zombie_risen;
extern const statetype s_zombie_pause;
extern const statetype s_zombie_inground;
extern const statetype s_zombie_rise1;
extern const statetype s_zombie_rise2;
extern const statetype s_zombie_rise3;
extern const statetype s_zombie_rise4;
extern const statetype s_zombie_alive1;
extern const statetype s_zombie_alive2;
extern const statetype s_zombie_alive3;
extern const statetype s_zombie_death1;
extern const statetype s_zombie_death2;
extern const statetype s_zombie_death3;
extern const statetype s_zombie_attack;
extern const statetype s_zombie_ouch;
extern const statetype s_spook_wait;
extern const statetype s_spook0;
extern const statetype s_spook0_1;
extern const statetype s_spook0_2;
extern const statetype s_spook1;
extern const statetype s_spook2;
extern const statetype s_spook3;
extern const statetype s_spook4;
extern const statetype s_spook5;
extern const statetype s_spook6;
extern const statetype s_spook_attack1;
extern const statetype s_spook_attack3;
extern const statetype s_spook_pause;
extern const statetype s_spookouch;
extern const statetype s_spookdie;
extern const statetype s_spookdie1;
extern const statetype s_spookdie2;
extern const statetype s_spookdie3;
extern const statetype s_spookdie4;
extern const statetype s_spookdie5;
extern const statetype s_wallskel;
extern const statetype s_wallskel2;
extern const statetype s_skel_pause;
extern const statetype s_skel_1;
extern const statetype s_skel_2;
extern const statetype s_skel_3;
extern const statetype s_skel_4;
extern const statetype s_skel_attack1;
extern const statetype s_skel_attack2;
extern const statetype s_skel_attack3;
extern const statetype s_skel_ouch;
extern const statetype s_skel_die1;
extern const statetype s_skel_die2;
extern const statetype s_skel_die3;
extern const statetype s_eye_pause;
extern const statetype s_eye_1;
extern const statetype s_eye_2;
extern const statetype s_eye_3;
extern const statetype s_eye_4;
extern const statetype s_eye_ouch;
extern const statetype s_eye_ouch2;
extern const statetype s_eye_die1;
extern const statetype s_eye_die2;
extern const statetype s_eye_die3;
extern const statetype s_eshot1;
extern const statetype s_eshot2;
extern const statetype s_orcpause;
extern const statetype s_orc1;
extern const statetype s_orc2;
extern const statetype s_orc3;
extern const statetype s_orc4;
extern const statetype s_orcattack1;
extern const statetype s_orcattack2;
extern const statetype s_orcattack3;
extern const statetype s_orcouch;
extern const statetype s_orcdie1;
extern const statetype s_orcdie2;
extern const statetype s_orcdie3;
extern const statetype s_demonpause;
extern const statetype s_demon1;
extern const statetype s_demon2;
extern const statetype s_demon3;
extern const statetype s_demon4;
extern const statetype s_demonattack1;
extern const statetype s_demonattack2;
extern const statetype s_demonattack3;
extern const statetype s_demonouch;
extern const statetype s_demondie1;
extern const statetype s_demondie2;
extern const statetype s_demondie3;
extern const statetype s_magepause;
extern const statetype s_mage1;
extern const statetype s_mage2;
extern const statetype s_mageattack3;
extern const statetype s_mageouch;
extern const statetype s_magedie1;
extern const statetype s_magedie2;
extern const statetype s_mshot1;
extern const statetype s_mshot2;
extern const statetype s_red_demonpause;
extern const statetype s_red_demon1;
extern const statetype s_red_demon2;
extern const statetype s_red_demon3;
extern const statetype s_red_demon4;
extern const statetype s_red_demonattack1;
extern const statetype s_red_demonattack2;
extern const statetype s_red_demonattack3;
extern const statetype s_red_demonouch;
extern const statetype s_red_demondie1;
extern const statetype s_red_demondie2;
extern const statetype s_red_demondie3;
extern const statetype s_grelpause;
extern const statetype s_grel1;
extern const statetype s_grel2;
extern const statetype s_grelattack3;
extern const statetype s_grelouch;
extern const statetype s_greldie1;
extern const statetype s_greldie2;
extern const statetype s_greldie3;
extern const statetype s_greldie4;
extern const statetype s_greldie5;
extern const statetype s_greldie5a;
extern const statetype s_greldie6;
extern const statetype s_gshot1;
extern const statetype s_bat1;
extern const statetype s_bat2;
extern const statetype s_bat3;
extern const statetype s_bat4;
extern const statetype s_batpast;
extern const statetype s_batdie1;
extern const statetype s_batdie2;
extern const statetype s_pshot1;
extern const statetype s_pshot2;
extern const statetype s_pshot_exp1;
extern const statetype s_pshot_exp2;
extern const statetype s_pshot_exp3;
extern const statetype s_player;
extern const statetype s_explode;

const statetype * const all_states[] = {
  &s_boltbonus,
  &s_boltbonus2,
  &s_boltbonus3,
  &s_nukebonus,
  &s_nukebonus2,
  &s_nukebonus3,
  &s_potionbonus,
  &s_rkey2bonus,
  &s_rkeybonus,
  &s_ykeybonus,
  &s_gkeybonus,
  &s_bkeybonus,
  &s_scrollbonus,
  &s_chestbonus,
  &s_waterchestbonus1,
  &s_waterchestbonus2,
  &s_rgem1bonus,
  &s_ygem1bonus,
  &s_ggem1bonus,
  &s_bgem1bonus,
  &s_pgem1bonus,
  &s_rgem2bonus,
  &s_ygem2bonus,
  &s_ggem2bonus,
  &s_bgem2bonus,
  &s_pgem2bonus,
  &s_bonus_die,
  &s_tombs[0],
  &s_tombs[1],
  &s_tombs[2],
  &s_ftimebonus,
  &s_ftimebonus2,
  &s_walldie1,
  &s_walldie2,
  &s_walldie3,
  &s_walldie4,
  &s_walldie5,
  &s_walldie6,
  &s_obj_gate1,
  &s_obj_gate2,
  &s_obj_gate3,
  &s_obj_gate4,
  &s_pit,
  &s_trollpause,
  &s_troll1,
  &s_troll2,
  &s_troll3,
  &s_troll4,
  &s_trollattack1,
  &s_trollattack2,
  &s_trollattack3,
  &s_trollouch,
  &s_trolldie1,
  &s_trolldie2,
  &s_trolldie3,
  &s_wet_bubbles1,
  &s_wet_bubbles2,
  &s_wet_bubbles3,
  &s_wet_peek,
  &s_wet_rise1,
  &s_wet_rise3,
  &s_wet_rise4,
  &s_wet_rise5,
  &s_wet_sink1,
  &s_wet_sink2,
  &s_wet_sink3,
  &s_wet_walk1,
  &s_wet_walk2,
  &s_wet_walk3,
  &s_wet_walk4,
  &s_wet_attack1,
  &s_wet_attack2,
  &s_wet_attack3,
  &s_wet_attack4,
  &s_wet_ouch,
  &s_wet_die1,
  &s_wet_die2,
  &s_wet_die3,
  &s_wet_die4,
  &s_wet_die5,
  &s_zombie_risen,
  &s_zombie_pause,
  &s_zombie_inground,
  &s_zombie_rise1,
  &s_zombie_rise2,
  &s_zombie_rise3,
  &s_zombie_rise4,
  &s_zombie_alive1,
  &s_zombie_alive2,
  &s_zombie_alive3,
  &s_zombie_death1,
  &s_zombie_death2,
  &s_zombie_death3,
  &s_zombie_attack,
  &s_zombie_ouch,
  &s_spook_wait,
  &s_spook0,
  &s_spook0_1,
  &s_spook0_2,
  &s_spook1,
  &s_spook2,
  &s_spook3,
  &s_spook4,
  &s_spook5,
  &s_spook6,
  &s_spook_attack1,
  &s_spook_attack3,
  &s_spook_pause,
  &s_spookouch,
  &s_spookdie,
  &s_spookdie1,
  &s_spookdie2,
  &s_spookdie3,
  &s_spookdie4,
  &s_spookdie5,
  &s_wallskel,
  &s_wallskel2,
  &s_skel_pause,
  &s_skel_1,
  &s_skel_2,
  &s_skel_3,
  &s_skel_4,
  &s_skel_attack1,
  &s_skel_attack2,
  &s_skel_attack3,
  &s_skel_ouch,
  &s_skel_die1,
  &s_skel_die2,
  &s_skel_die3,
  &s_eye_pause,
  &s_eye_1,
  &s_eye_2,
  &s_eye_3,
  &s_eye_4,
  &s_eye_ouch,
  &s_eye_ouch2,
  &s_eye_die1,
  &s_eye_die2,
  &s_eye_die3,
  &s_eshot1,
  &s_eshot2,
  &s_orcpause,
  &s_orc1,
  &s_orc2,
  &s_orc3,
  &s_orc4,
  &s_orcattack1,
  &s_orcattack2,
  &s_orcattack3,
  &s_orcouch,
  &s_orcdie1,
  &s_orcdie2,
  &s_orcdie3,
  &s_demonpause,
  &s_demon1,
  &s_demon2,
  &s_demon3,
  &s_demon4,
  &s_demonattack1,
  &s_demonattack2,
  &s_demonattack3,
  &s_demonouch,
  &s_demondie1,
  &s_demondie2,
  &s_demondie3,
  &s_magepause,
  &s_mage1,
  &s_mage2,
  &s_mageattack3,
  &s_mageouch,
  &s_magedie1,
  &s_magedie2,
  &s_mshot1,
  &s_mshot2,
  &s_red_demonpause,
  &s_red_demon1,
  &s_red_demon2,
  &s_red_demon3,
  &s_red_demon4,
  &s_red_demonattack1,
  &s_red_demonattack2,
  &s_red_demonattack3,
  &s_red_demonouch,
  &s_red_demondie1,
  &s_red_demondie2,
  &s_red_demondie3,
  &s_grelpause,
  &s_grel1,
  &s_grel2,
  &s_grelattack3,
  &s_grelouch,
  &s_greldie1,
  &s_greldie2,
  &s_greldie3,
  &s_greldie4,
  &s_greldie5,
  &s_greldie5a,
  &s_greldie6,
  &s_gshot1,
  &s_bat1,
  &s_bat2,
  &s_bat3,
  &s_bat4,
  &s_batpast,
  &s_batdie1,
  &s_batdie2,
  &s_pshot1,
  &s_pshot2,
  &s_pshot_exp1,
  &s_pshot_exp2,
  &s_pshot_exp3,
  &s_player,
  &s_explode
};
const u16 num_states = sizeof(all_states)/sizeof(all_states[0]);
