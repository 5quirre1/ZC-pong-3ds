#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202300L
#define ZC_AUTO auto
#else
#define ZC_AUTO __auto_type
#endif
#define ZC_CAST(T, x) ((T)(x))
#ifdef __TINYC__
#ifndef __auto_type
#define __auto_type __typeof__
#endif

#ifndef __builtin_expect
#define __builtin_expect(x, v) (x)
#endif

#ifndef __builtin_unreachable
#define __builtin_unreachable()
#endif
#endif
static inline const char* _z_bool_str(_Bool b) { return b ? "true" : "false"; }
#ifdef __OBJC__
#define _z_objc_map ,id: "%s", Class: "%s", SEL: "%s"
#define _z_objc_arg_map(x) ,id: [(id)(x) description].UTF8String, Class: class_getName((Class)(x)), SEL: sel_getName((SEL)(x))
#else
#define _z_objc_map
#define _z_objc_arg_map(x)
#endif

#define _z_str(x) _Generic((x), _Bool: "%s", char: "%c", signed char: "%c", unsigned char: "%u", short: "%d", unsigned short: "%u", int: "%d", unsigned int: "%u", long: "%ld", unsigned long: "%lu", long long: "%lld", unsigned long long: "%llu", float: "%f", double: "%f", char*: "%s", void*: "%p" _z_objc_map)
#define _z_arg(x) _Generic((x), _Bool: _z_bool_str(x) _z_objc_arg_map(x), default: (x))
typedef size_t usize;
typedef char* string;
typedef struct { void *func; void *ctx; } z_closure_T;
typedef void U0;
typedef int8_t I8;
typedef uint8_t U8;
typedef int16_t I16;
typedef uint16_t U16;
typedef int32_t I32;
typedef uint32_t U32;
typedef int64_t I64;
typedef uint64_t U64;
#define F32 float
#define F64 double
#define z_malloc malloc
#define z_realloc realloc
#define z_free free
#define z_print printf
void z_panic(const char* msg) { fprintf(stderr, "Panic: %s\n", msg); exit(1); }
void _z_autofree_impl(void *p) { void **pp = (void**)p; if(*pp) { z_free(*pp); *pp = NULL; } }
#define assert(cond, ...) if (!(cond)) { fprintf(stderr, "Assertion failed: " __VA_ARGS__); exit(1); }
string _z_readln_raw() { size_t cap = 64; size_t len = 0; char *line = z_malloc(cap); if(!line) return NULL; int c; while((c = fgetc(stdin)) != EOF) { if(c == '\n') break; if(len + 1 >= cap) { cap *= 2; char *n = z_realloc(line, cap); if(!n) { z_free(line); return NULL; } line = n; } line[len++] = c; } if(len == 0 && c == EOF) { z_free(line); return NULL; } line[len] = 0; return line; }
int _z_scan_helper(const char *fmt, ...) { char *l = _z_readln_raw(); if(!l) return 0; va_list ap; va_start(ap, fmt); int r = vsscanf(l, fmt, ap); va_end(ap); z_free(l); return r; }
int _z_orig_stdout = -1;
void _z_suppress_stdout() {
    fflush(stdout);
    if (_z_orig_stdout == -1) _z_orig_stdout = dup(STDOUT_FILENO);
    int nullfd = open("/dev/null", O_WRONLY);
    dup2(nullfd, STDOUT_FILENO);
    close(nullfd);
}
void _z_restore_stdout() {
    fflush(stdout);
    if (_z_orig_stdout != -1) {
        dup2(_z_orig_stdout, STDOUT_FILENO);
        close(_z_orig_stdout);
        _z_orig_stdout = -1;
    }
}
#include "3ds.h"
#include "citro2d.h"
typedef char* string;
typedef struct { void **data; int len; int cap; } Vec;
#define Vec_new() (Vec){.data=0, .len=0, .cap=0}
void _z_vec_push(Vec *v, void *item) { if(v->len >= v->cap) { v->cap = v->cap?v->cap*2:8; v->data = z_realloc(v->data, v->cap * sizeof(void*)); } v->data[v->len++] = item; }
static inline Vec _z_make_vec(int count, ...) { Vec v = {0}; v.cap = count > 8 ? count : 8; v.data = z_malloc(v.cap * sizeof(void*)); v.len = 0; va_list args; va_start(args, count); for(int i=0; i<count; i++) { v.data[v.len++] = va_arg(args, void*); } va_end(args); return v; }
#define Vec_push(v, i) _z_vec_push(&(v), (void*)(long)(i))
#define _z_check_bounds(index, limit) ({ ZC_AUTO _i = (index); if(_i < 0 || _i >= (limit)) { fprintf(stderr, "Index out of bounds: %ld (limit %d)\n", (long)_i, (int)(limit)); exit(1); } _i; })

const int SCREEN_WIDTH = 400;
const int SCREEN_HEIGHT = 240;
const int PADDLE_W = 10;
const int PADDLE_H = 50;
const int BALL_SIZE = 8;
const int PADDLE_SPEED = 3;
const int AI_SPEED = 2;
const int AI_DEAD_ZONE = 10;
const int PADDLE_START_Y = 95;
const int BALL_START_X = 200;
const int BALL_START_Y = 120;
const int BALL_START_VX = 2;
const int BALL_START_VY = 2;
const int P1_X = 10;
const int P2_X = 380;
int main(void);
bool draw_score(int score1, int score2, int last_score1, int last_score2);
void draw_game(C3D_RenderTarget* top, int p1_y, int p2_y, int ball_x, int ball_y, uint32_t white, uint32_t black);
int calculate_bounce_angle(int ball_y, int paddle_y);
bool check_paddle_collision(int ball_x, int ball_y, int ball_dx, int paddle_y, bool is_left);
int update_ai_paddle(int p2_y, int ball_y, bool should_update);
int update_player_paddle(int p1_y, uint32_t kHeld);

int main(void)
{
    {
gfxInitDefault();
C3D_Init(524288);
C2D_Init(4096);
C2D_Prepare();
    ZC_AUTO top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
consoleInit(GFX_BOTTOM, NULL);
    int p1_y = 95;
    int p2_y = 95;
    int ball_x = 200;
    int ball_y = 120;
    int ball_dx = 2;
    int ball_dy = 2;
    int score1 = 0;
    int score2 = 0;
    int ai_frame = 0;
    int last_score1 = (-1);
    int last_score2 = (-1);
    ZC_AUTO white = C2D_Color32(255, 255, 255, 255);
    ZC_AUTO black = C2D_Color32(0, 0, 0, 255);
while (aptMainLoop())     {
hidScanInput();
    ZC_AUTO kHeld = hidKeysHeld();
if ((kHeld & KEY_START))     {
break;
    }
(p1_y = update_player_paddle(p1_y, kHeld));
(ai_frame = (ai_frame + 1));
    int should_update_ai = (ai_frame >= 2);
if (should_update_ai)     {
(ai_frame = 0);
    }
(p2_y = update_ai_paddle(p2_y, ball_y, should_update_ai));
(ball_x = (ball_x + ball_dx));
(ball_y = (ball_y + ball_dy));
    int ball_max_y = (240 - 8);
if ((ball_y <= 0))     {
(ball_y = 1);
(ball_dy = (-ball_dy));
    }
 else if ((ball_y >= ball_max_y))     {
(ball_y = (ball_max_y - 1));
(ball_dy = (-ball_dy));
    }
if (check_paddle_collision(ball_x, ball_y, ball_dx, p1_y, true))     {
(ball_dx = (-ball_dx));
(ball_x = ((10 + 10) + 1));
(ball_dy = calculate_bounce_angle(ball_y, p1_y));
    }
if (check_paddle_collision(ball_x, ball_y, ball_dx, p2_y, false))     {
(ball_dx = (-ball_dx));
(ball_x = ((380 - 8) - 1));
(ball_dy = calculate_bounce_angle(ball_y, p2_y));
    }
if ((ball_x < 0))     {
(score2 = (score2 + 1));
(ball_x = 200);
(ball_y = 120);
(ball_dx = 2);
(ball_dy = 2);
(p1_y = 95);
(p2_y = 95);
    }
 else if ((ball_x > 400))     {
(score1 = (score1 + 1));
(ball_x = 200);
(ball_y = 120);
(ball_dx = (-2));
(ball_dy = (-2));
(p1_y = 95);
(p2_y = 95);
    }
C3D_FrameBegin(2);
draw_game(top, p1_y, p2_y, ball_x, ball_y, white, black);
C3D_FrameEnd(0);
if (draw_score(score1, score2, last_score1, last_score2))     {
(last_score1 = score1);
(last_score2 = score2);
    }
gspWaitForVBlank();
    }
C2D_Fini();
C3D_Fini();
gfxExit();
    }
}

bool draw_score(int score1, int score2, int last_score1, int last_score2)
{
    {
if (((score1 != last_score1) || (score2 != last_score2)))     {
printf("\x1b[2J\x1b[10;8HPlayer 1: %d    Player 2: %d", score1, score2);
    return true;
    }
    return false;
    }
}

void draw_game(C3D_RenderTarget* top, int p1_y, int p2_y, int ball_x, int ball_y, uint32_t white, uint32_t black)
{
    {
C2D_SceneBegin(top);
C2D_TargetClear(top, black);
C2D_DrawRectSolid(10, p1_y, 0.500000, 10, 50, white);
C2D_DrawRectSolid(380, p2_y, 0.500000, 10, 50, white);
C2D_DrawRectSolid(ball_x, ball_y, 0.500000, 8, 8, white);
    }
}

int calculate_bounce_angle(int ball_y, int paddle_y)
{
    {
    int hit_pos = ((ball_y + (8 / 2)) - paddle_y);
if ((hit_pos < 15))     {
    return (-2);
    }
 else if ((hit_pos > 35))     {
    return 2;
    }
    return 2;
    }
}

bool check_paddle_collision(int ball_x, int ball_y, int ball_dx, int paddle_y, bool is_left)
{
    {
    int ball_right = (ball_x + 8);
    int ball_bottom = (ball_y + 8);
if (is_left)     {
    int p1_right = (10 + 10);
if ((((ball_x <= p1_right) && (ball_x > 10)) && (ball_dx < 0)))     {
if (((ball_bottom > paddle_y) && (ball_y < (paddle_y + 50))))     {
    return true;
    }
    }
    }
 else     {
if ((((ball_right >= 380) && (ball_x < 380)) && (ball_dx > 0)))     {
if (((ball_bottom > paddle_y) && (ball_y < (paddle_y + 50))))     {
    return true;
    }
    }
    }
    return false;
    }
}

int update_ai_paddle(int p2_y, int ball_y, bool should_update)
{
    {
if ((!should_update))     {
    return p2_y;
    }
    int p2_max = (240 - 50);
    int ball_center_y = (ball_y + (8 / 2));
    int p2_center = (p2_y + (50 / 2));
    int diff = (ball_center_y - p2_center);
if (((diff > 10) && (p2_y < p2_max)))     {
    return (p2_y + 2);
    }
 else if (((diff < (-10)) && (p2_y > 0)))     {
    return (p2_y - 2);
    }
    return p2_y;
    }
}

int update_player_paddle(int p1_y, uint32_t kHeld)
{
    {
    int p1_max = (240 - 50);
if (((kHeld & KEY_DUP) && (p1_y > 0)))     {
    return (p1_y - 3);
    }
if (((kHeld & KEY_DDOWN) && (p1_y < p1_max)))     {
    return (p1_y + 3);
    }
    return p1_y;
    }
}
