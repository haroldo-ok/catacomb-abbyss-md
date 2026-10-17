/* Catacomb Abyss for the Sega Genesis
 * Port of the Catacomb Abyss source code (C) 1993-2014 Flat Rock Software,
 * released under the GNU GPL v2.  Genesis port additions are also GPL v2.
 *
 * def.h - global definitions, adapted from DEF.H and the ID_*.H headers.
 */
#ifndef DEF_H
#define DEF_H

#include <genesis.h>
#include "gen/assets.h"
#include "gfxe_abs.h"

typedef u16 boolean;
typedef u8 byte;
typedef u16 word;
typedef u32 longword;
typedef s32 fixed;

#define nil NULL

#ifndef false
#define false 0
#endif

/* Borland random(n): 0..n-1 */
s16 rnd(s16 n);
u16 US_RndT(void);
void US_InitRndT(boolean randomize);



#define FL_QUICK        0x01
#define FL_NOMEMCHECK   0x02
#define FL_CLEAR        (FL_QUICK|FL_NOMEMCHECK)
#define FL_DEAD         0x80

#define MAXBOLTS        10
#define MAXNUKES        10
#define MAXPOTIONS      10

#define RADARX          31
#define RADARY          11
#define RADAR_WIDTH     51
#define RADAR_HEIGHT    51
#define RADAR_XCENTER   ((RADARX*8)+(RADAR_WIDTH/2)+3)
#define RADAR_YCENTER   ((RADARY-8)+(RADAR_HEIGHT/2)+5)
#define MAX_RADAR_BLIPS 60

#define RADAR_RADIUS        17
#define RADAR_RADIUS_NSEW   15
#define RADAR_X_IRADIUS     (113/5)
#define RADAR_Y_IRADIUS     (113/7)
#define RADAR_ICON_CENTER   4

#define NAMESTART           180
#define REMOVED_DOOR_TILE   NAMESTART

#define NEXT_LEVEL_CODE     0xff
#define REMOVE_DOOR_CODE    0xfe
#define CANT_OPEN_CODE      0xfd
#define EXP_WALL_CODE       0xfc

#define EXPWALLSTART    8
#define NUMEXPWALLS     7
#define WALLEXP         15
#define WATEREXP        29
#define NUMFLOORS       71

#define NUMLATCHPICS    (FIRSTWALLPIC-FIRSTLATCHPIC)

#define FLASHCOLOR      12
#define FLASHTICS       4

#define NUMLEVELS       21

#define VIEWX           0
#define VIEWY           0
#define VIEWWIDTH       (40*8)
#define VIEWHEIGHT      (15*8)
#define VIEWXH          (VIEWX+VIEWWIDTH-1)
#define VIEWYH          (VIEWY+VIEWHEIGHT-1)

#define CENTERX         (VIEWX+VIEWWIDTH/2-1)
#define CENTERY         (VIEWY+VIEWHEIGHT/2-1)

#define GLOBAL1         (1l<<16)
#define TILEGLOBAL      GLOBAL1
#define TILESHIFT       16l

#define MINDIST         (2*GLOBAL1/5)
#define FOCALLENGTH     (TILEGLOBAL)

#define ANGLES          360

#define MAPSIZE         64
#define MAPSIZEY        32
#define MAXACTORS       100

#define NORTH   0
#define EAST    1
#define SOUTH   2
#define WEST    3

#define MAXSCALEHEIGHT  (VIEWWIDTH/2)

#define MAXBODY         100
#define MAXSHOTPOWER    56

#define PIXRADIUS       512

#define STATUSLINES     (200-VIEWHEIGHT)

/* status bar: 320x80 surface at screen line 120 */

enum bonusnumbers {B_BOLT,B_NUKE,B_POTION,B_RKEY,B_YKEY,B_GKEY,B_BKEY,B_SCROLL1,
 B_SCROLL2,B_SCROLL3,B_SCROLL4,B_SCROLL5,B_SCROLL6,B_SCROLL7,B_SCROLL8,
 B_GOAL,B_CHEST,B_RGEM,B_GGEM,B_BGEM,B_YGEM,B_PGEM,B_RKEY2};

#define GEM_DELAY_TIME  (120*60)
#define ROTATE_SPEED    (6)
#define WALL_SKELETON_CODE 6
#define MAXREALTICS     (2*60)
#define MAXFREEZETIME   (100*30)
#define MAXTICS         6

enum {BLANKCHAR=9,BOLTCHAR,NUKECHAR,POTIONCHAR,KEYCHARS,SCROLLCHARS=17,
    NUMBERCHARS=25};

/* EGA colours */
#define BLACK       0
#define WHITE       15
#define LT_GREY     7

/* tile info (MAPHEAD tinf) */
#define tf_SOLID                0x01
#define tf_SPECIAL              0x02
#define tf_EMBEDDED_KEY_COLOR   0x04
#define TILE_FLAGS(tile)        (tile_flags[(tile)])
#define GATE_KEY_COLOR(tile)    ((u8)(TILE_FLAGS(tile)>>4))

typedef struct {s16 x,y;} tilept;
typedef struct {fixed x,y;} globpt;

typedef enum
  {nothing,playerobj,bonusobj,orcobj,batobj,skeletonobj,trollobj,demonobj,
  mageobj,pshotobj,bigpshotobj,mshotobj,inertobj,bounceobj,grelmobj,
  gateobj,zombieobj,spookobj,wetobj,expobj,eyeobj,wallskelobj,eshotobj,
  gshotobj,reddemonobj,freezeobj,solidobj} classtype;

typedef enum {north,east,south,west,northeast,southeast,southwest,
          northwest,nodir} dirtype;

struct objstruct;
typedef void (*thinkfn)(struct objstruct *ob);

typedef struct statestruct
{
    u16 shapenum;
    s16 tictime;
    thinkfn think;
    const struct statestruct *next;
} statetype;

#define of_shootable    0x01
#define of_damagedone   0x02

enum {no,noalways,yes,always};

typedef struct objstruct
{
    u8      active;
    u8      obclass;
    u8      flags;
    u8      dir;
    s16     ticcount;
    s16     angle;
    const statetype *state;

    s32     distance;
    fixed   x,y;
    u16     tilex,tiley;
    s16     viewx;
    u16     viewheight;

    s16     hitpoints;
    s16     temp1,temp2;
    s32     speed;

    u32     size;
    fixed   xl,xh,yl,yh;

    struct objstruct *next,*prev;
} objtype;

typedef enum {ex_stillplaying,ex_died,ex_warped,ex_resetgame
    ,ex_loadedgame,ex_victorious,ex_turning,ex_abort} exittype;

typedef enum {S_NONE,     S_HELP,    S_SND,     S_SAVING,   S_RESTORING,
            S_JOYSTICK, S_RETREAT, S_ADVANCE, S_SIDESTEP, S_QTURN,
            S_MISSLE,   S_ZAPPER,  S_XTER,    S_CURING,   S_READ,
            S_VIEWING,  S_ITEMDES,  S_DAMAGE,  S_TURN,    S_TIMESTOP} status_flags;

typedef enum {gd_Continue,gd_Easy,gd_Normal,gd_Hard} GameDiff;

typedef struct
{
    s16     difficulty;
    s16     mapon;
    s16     bolts,nukes,potions,keys[4],scrolls[8];
    s16     gems[5];
    s32     score;
    s16     body,shotpower;
} gametype;

/* ---------------------------------------------------------------- C4_MAIN */
extern char         str[80],str2[20];
extern gametype     gamestate;
extern exittype     playstate;
extern u16          Flags;
extern boolean      EASYMODEON;
extern GameDiff     restartgame;
extern boolean      loadedgame,abortgame,ingame;
extern boolean      quitgame;

void NewGame (void);
boolean SaveTheGame(u16 slot);
boolean LoadTheGame(u16 slot);
void ResetGame(void);
void InitGame (void);
void Quit (const char *error);
boolean DemoLoop (void);

/* ---------------------------------------------------------------- C4_GAME */
void ScanInfoPlane (void);
void SetupGameLevel (void);
void Victory (boolean playsounds);
void NormalScreen (void);
void DrawPlayScreen (void);
void FizzleOut (s16 showlevel);
void FreeUpMemory (void);
void GameLoop (void);
void DrawEnterScreen (void);
void CacheScaleds (void);
void LatchDrawPic (u16 x, u16 y, u16 picnum);
const char *LevelName(s16 n);
extern boolean restore_requested;
extern s16 starting_level;
extern boolean tileneeded[NUMFLOORS];

/* ---------------------------------------------------------------- C4_PLAY */
#define BGF_NIGHT           0x01
#define BGF_NOT_LIGHTNING   0x02

extern byte BGFLAGS,bcolor;
extern const u16 *skycolor,*groundcolor;
extern const u16 sky_colors[NUMLEVELS], gnd_colors[NUMLEVELS];

typedef struct
{
    boolean button0,button1;
    s16     x,y;
    s16     xaxis,yaxis;
    s16     dir;
} ControlInfo;

extern ControlInfo  control;
extern boolean      running;
extern s16          bordertime;

/* tilemap and spotvis live together (the ray caster reaches spotvis at a
 * fixed offset from a tilemap pointer); tiles outside the map are sealed
 * with 0xFF by R_SealMap() so rays and movement always stop. */
typedef struct
{
    byte pad[MAPSIZEY];
    byte tilemap[MAPSIZE][MAPSIZEY];
    byte spotvis[MAPSIZE][MAPSIZEY];
} MapArrays;
extern MapArrays mapa;
#define tilemap (mapa.tilemap)
#define spotvis (mapa.spotvis)
#define SEALTILE 0xFF
void R_SealMap(void);
extern u8       actorat[MAPSIZE][MAPSIZEY];
extern byte     mapplane0[MAPSIZE*MAPSIZEY];
extern byte     mapplane2[MAPSIZE*MAPSIZEY];   /* high byte of plane 2 */
extern u16      mapwidth,mapheight;
#define MAP0(x,y)   mapplane0[(y)*mapwidth+(x)]
#define MAP2(x,y)   mapplane2[(y)*mapwidth+(x)]
#define MAP2FULL(x,y) (map_defs[gamestate.mapon].plane2[(y)*mapwidth+(x)])

/* actorat[][] holds 0, a tile number (<0x80) or ACTOROBJ(index) */
#define ACTOROBJ(o)     ((u8)(0x80 + ((o) - objlist)))
#define ISACTOROBJ(v)   ((v) >= 0x80)
#define ACTORPTR(v)     (&objlist[(v)-0x80])

extern objtype  objlist[MAXACTORS],*new,*obj,*player,*lastobj,*objfreelist;
extern objtype  dummyobj;
extern boolean  godmode;
extern s16      objectcount;
extern status_flags status_flag;
extern s16      status_delay;
extern s16      BeepTime;
extern status_flags status_hint;
extern u16      tics,realtics;

void CenterWindow(u16 w,u16 h);
void CheckKeys (void);
void InitObjList (void);
void GetNewObj (boolean usedummy);
void RemoveObj (objtype *gone);
void PollControls (void);
void PlayLoop (void);
void InitBgChange(s16 stimer, const u16 *scolors, s16 gtimer, const u16 *gcolors, byte flag);
void DisplayStatus (status_flags *stat_flag);
s16  IntSqrt(s32 va);

/* --------------------------------------------------------------- C4_STATE */
void Internal_SpawnNewObj (u16 x, u16 y, const statetype *state, u32 size,boolean UseDummy);
void Internal_SpawnNewObjFrac (s32 x, s32 y, const statetype *state, u32 size,boolean UseDummy);

#define DSpawnNewObj(x, y, state, size)             Internal_SpawnNewObj(x,y,state,size,true)
#define SpawnNewObj(x, y, state, size)              Internal_SpawnNewObj(x,y,state,size,false)
#define SpawnNewObjFrac(x, y, state, size,Dummy)    Internal_SpawnNewObjFrac(x, y, state, size,false)
#define DSpawnNewObjFrac(x, y, state, size)         Internal_SpawnNewObjFrac(x, y, state, size,true)

boolean CheckHandAttack (objtype *ob);
void T_DoDamage (objtype *ob);
boolean Walk (objtype *ob);
void ChaseThink (objtype *obj, boolean diagonal);
void MoveObj (objtype *ob, s32 move);
boolean Chase (objtype *ob, boolean diagonal);
void ShootActor (objtype *ob, u16 damage);
extern const dirtype opposite[9];

/* ---------------------------------------------------------------- C4_DRAW */
extern const fixed *sintable;
extern const fixed *costable;
extern fixed    viewx,viewy,viewsin,viewcos;
extern s16      viewangle;
extern s32      lasttimecount;
extern boolean  fizzlein;
extern s16      zbuffer[VIEWWIDTH/2];

fixed FixedByFrac (fixed a, fixed b);
void ThreeDRefresh (void);
void BuildTables (void);
void CalcTics (void);

/* ---------------------------------------------------------------- C4_WIZ */
#define MAXHANDHEIGHT   72
extern const statetype s_pshot_exp1;
extern const statetype s_pshot_exp2;
extern const statetype s_pshot_exp3;
extern s32      lastnuke;
extern s16      lasttext;
extern s16      handheight;
extern s16      boltsleft,bolttimer;
extern s16      RadarXY[MAX_RADAR_BLIPS][3];
extern s16      FreezeTime;
extern boolean  redraw_gems;
extern boolean  autofire;

char DisplayMsg(const char *text,const char *choices);
char DisplaySMsg(const char *text,const char *choices);
void SpawnExplosion(fixed x, fixed y,s16 Delay);
void T_ExpThink(objtype *obj);
void SpawnBigExplosion(fixed x, fixed y, s16 Delay, fixed Range);
void RedrawStatusWindow (void);
void DrawHealth(void);
void DrawText (boolean force);
void TakeDamage (s16 points);
void GiveBolt (void);
void GiveNuke (void);
void GivePotion (void);
void GiveKey (s16 keytype);
void GiveScroll (s16 scrolltype,boolean show);
void ReadScroll (s16 scroll);
void StopTime(void);
void SpawnPlayer (s16 tilex, s16 tiley, s16 dir);
void Thrust (s16 angle, u32 speed);
void CalcBounds (objtype *ob);
boolean ShotClipMove (objtype *ob, s32 xmove, s32 ymove);
void DrinkPotion (void);
void CastNuke (void);
void CastBolt (void);
void DrawRadar (void);
void DrawNSEWIcons(void);

/* ---------------------------------------------------------------- C4_ACT1 */
s16 EasyHitPoints(s16 NrmHitPts);
s16 EasyDoDamage(s16 Damage);
extern s16 zombie_base_delay;

void SpawnBonus (s16 tilex, s16 tiley, s16 number);
void SpawnTombstone (s16 tilex, s16 tiley, s16 shape);
void SpawnFTime(s16 tilex, s16 tiley);
void ExplodeWall (s16 tilex, s16 tiley);
void SpawnWarp (s16 tilex, s16 tiley, s16 type);
void SpawnTroll (s16 tilex, s16 tiley);
void SpawnWetMan(s16 tilex, s16 tiley);
void SpawnZombie (s16 tilex, s16 tiley);
void SpawnSpook(s16 tilex, s16 tiley);
void SpawnWallSkeleton(s16 tilex, s16 tiley);
void SpawnSkeleton(s16 tilex, s16 tiley);
void SpawnEye(s16 tilex, s16 tiley);
void SpawnOrc (s16 tilex, s16 tiley);
void SpawnDemon (s16 tilex, s16 tiley);
void SpawnMage (s16 tilex, s16 tiley);
void SpawnRedDemon (s16 tilex, s16 tiley);
void SpawnGrelminar (s16 tilex, s16 tiley);
void SpawnBat (s16 tilex, s16 tiley);

extern const statetype s_trollouch, s_trolldie1, s_orcdie1, s_orcouch,
    s_demonouch, s_demondie1, s_mageouch, s_magedie1, s_grelouch, s_greldie1,
    s_batdie1, s_zombie_death1, s_zombie_ouch, s_spookouch, s_spookdie,
    s_skel_ouch, s_skel_die1, s_wet_ouch, s_wet_die1, s_eye_ouch, s_eye_die1,
    s_bonus_die, s_red_demonouch, s_red_demondie1;

/* ---------------------------------------------------------------- misc */
extern volatile u32 TimeCount;      /* 70 Hz, like the original timer */

#endif
