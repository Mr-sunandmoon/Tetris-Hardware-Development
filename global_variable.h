#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<pthread.h>
#include<signal.h>
#include<fcntl.h>
#include<termios.h>
#include<time.h>
#include<sys/types.h>
#include<linux/input.h>

// 游戏地图尺寸 16*16
#define MAPLENGTH 16
#define MAPWIDTH 16
#define DEBUG 1       // 开启按键调试打印

// 开发板设备节点 FS6818
#define DEV_LED1616  "/dev/led1616"
#define DEV_ZLG7290  "/dev/zlg7290"
#define DEV_IR       "/dev/input/event1"
#define DEV_BEEP     "/sys/devices/platform/pwm/pwm.2"

#define LED_DISPLAY  _IOW('L', 0, struct led1616Data)
#define SET_VAL      _IO('Z', 0)

// 红外遥控器键值定义
#define KEY_1     2
#define KEY_2     3
#define KEY_3     4
#define KEY_4     5
#define KEY_5     6
#define KEY_6     7
#define KEY_7     8
#define KEY_8     9
#define KEY_9     10
#define KEY_UP    103
#define KEY_DOWN  108
#define KEY_LEFT  105
#define KEY_RIGHT 106
#define KEY_ENTER 28

// 俄罗斯方块配置
#define BLOCK_TYPE_COUNT    7
#define BLOCK_ROTATE_COUNT  4
#define INIT_LEVEL          1
#define SCORE_PER_LINE      100

// 全局游戏变量
char tetrisKey;
int tetrisLevel;
int tetrisScore;
int tetrisSpeed;

// 设备句柄
int dev_ir_fd;
int dev_led1616_fd;
int dev_zlg7290_fd;
int dev_beep_fd;

// 线程ID
pthread_t id_infrared_control_thread;
pthread_t id_score_thread;
pthread_t id_tetris_thread;

// 16*16点阵传输结构体
struct led1616Data{
    unsigned char code[33];
};

// 方块结构体
typedef struct {
    int x;
    int y;
    int shape[4][4];
    int type;
    int rotate;
} TetrisBlock;

// 游戏地图 & 当前活动方块（全局，所有函数共享）
extern int gameMap[MAPLENGTH][MAPWIDTH];
extern TetrisBlock currentBlock;

// 函数声明
void ReleaseResource();
void *infrared_control_thread(void *arg);
void *score_thread(void *arg);
void itoa(int num, char *str, int radix);

void *tetris_thread(void *arg);
void tetris_init();
TetrisBlock create_block();
int check_collision(TetrisBlock block);
void merge_block_to_map(TetrisBlock block);
int clear_full_lines();
void tetris_print();
void tetris_move_sound();
void tetris_clear_sound();
void tetris_game_over();

// UI界面函数
void start_interface1();
void start_interface2();
void start_interface3();
void end_interface();
void game_end();