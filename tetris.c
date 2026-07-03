#include "global_variable.h"

// 全局地图 + 当前下落方块
int gameMap[MAPLENGTH][MAPWIDTH] = {0};
TetrisBlock currentBlock;

// 7种方块 4种旋转形态 标准4*4矩阵
int blockShapes[BLOCK_TYPE_COUNT][BLOCK_ROTATE_COUNT][4][4] = {
    // 0 I型
    {
        {{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}},
        {{0,1,0,0},{0,1,0,0},{0,1,0,0},{0,1,0,0}},
        {{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}},
        {{0,1,0,0},{0,1,0,0},{0,1,0,0},{0,1,0,0}}
    },
    // 1 O型
    {
        {{0,1,1,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,1,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,1,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,1,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}}
    },
    // 2 T型
    {
        {{0,1,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,0,0},{0,1,1,0},{0,1,0,0},{0,0,0,0}},
        {{0,0,0,0},{1,1,1,0},{0,1,0,0},{0,0,0,0}},
        {{0,1,0,0},{1,1,0,0},{0,1,0,0},{0,0,0,0}}
    },
    // 3 L型
    {
        {{0,0,1,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,0,0},{0,1,0,0},{0,1,1,0},{0,0,0,0}},
        {{0,0,0,0},{1,1,1,0},{1,0,0,0},{0,0,0,0}},
        {{1,1,0,0},{0,1,0,0},{0,1,0,0},{0,0,0,0}}
    },
    // 4 J型
    {
        {{1,0,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,1,0},{0,1,0,0},{0,1,0,0},{0,0,0,0}},
        {{0,0,0,0},{1,1,1,0},{0,0,1,0},{0,0,0,0}},
        {{0,1,0,0},{0,1,0,0},{1,1,0,0},{0,0,0,0}}
    },
    // 5 S型
    {
        {{0,1,1,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,0,0},{0,1,1,0},{0,0,1,0},{0,0,0,0}},
        {{0,1,1,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}},
        {{0,1,0,0},{0,1,1,0},{0,0,1,0},{0,0,0,0}}
    },
    // 6 Z型
    {
        {{1,1,0,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,0,1,0},{0,1,1,0},{0,1,0,0},{0,0,0,0}},
        {{1,1,0,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},
        {{0,0,1,0},{0,1,1,0},{0,1,0,0},{0,0,0,0}}
    }
};

// ==================== 居中优化字模与无限循环检测逻辑 ====================
// 16行高，每个字母固定 8 列宽 (上下各留空4行，实现绝对纵向居中)
static const unsigned char font_T[16] = {0,0,0,0, 0x7C,0x10,0x10,0x10,0x10,0x10,0x10,0x00, 0,0,0,0}; // ■■■■■ 顶部，■ 中心竖柱
static const unsigned char font_E[16] = {0,0,0,0, 0x7C,0x40,0x40,0x78,0x40,0x40,0x7C,0x00, 0,0,0,0};
static const unsigned char font_R[16] = {0,0,0,0, 0x78,0x44,0x44,0x78,0x4C,0x44,0x44,0x00, 0,0,0,0};
static const unsigned char font_I[16] = {0,0,0,0, 0x7C,0x10,0x10,0x10,0x10,0x10,0x7C,0x00, 0,0,0,0};
static const unsigned char font_S[16] = {0,0,0,0, 0x78,0x44,0x40,0x38,0x04,0x44,0x78,0x00, 0,0,0,0};

void tetris_opening_animation()
{
    struct led1616Data data;
    int total_columns = 80; 
    int offset = 0;

    printf("[动画提示] 开场动画已启动，等待按键选择难度...\n");

    while (1)
    {
        // 实时检测红外按键
        if (tetrisKey >= '1' && tetrisKey <= '9')
        {
            break; 
        }

        memset(data.code, 0, sizeof(data.code));

        // 渲染 16×16 逻辑视窗
        for (int y = 0; y < 16; y++)
        {
            for (int x = 0; x < 16; x++)
            {
                int global_x = offset + x - 16; 
                int pixel = 0;

                if (global_x >= 0 && global_x < 48)
                {
                    int char_idx = global_x / 8;     
                    int bit_offset = global_x % 8;   // 当前在当前字符的第几列 (0 ~ 7)

                    // 【裁剪保护】每个字符的第 7 列 (bit_offset == 7) 强制作为字间距，不读取任何像素
                    if (bit_offset < 7)
                    {
                        int bit_idx = 7 - bit_offset; 

                        switch(char_idx)
                        {
                            case 0: pixel = (font_T[y] >> bit_idx) & 1; break; 
                            case 1: pixel = (font_E[y] >> bit_idx) & 1; break; 
                            case 2: pixel = (font_T[y] >> bit_idx) & 1; break; 
                            case 3: pixel = (font_R[y] >> bit_idx) & 1; break; 
                            case 4: pixel = (font_I[y] >> bit_idx) & 1; break; 
                            case 5: pixel = (font_S[y] >> bit_idx) & 1; break; 
                        }
                    }
                }

                // 转换硬件映射逻辑
                if (pixel == 1)
                {
                    int real_x = (16 - 1) - x; 
                    int pos;
                    if (real_x < 8) {
                        pos = 2 * y + 1;         
                        data.code[pos] |= (1 << real_x);
                    } else {
                        pos = 2 * y;             
                        data.code[pos] |= (1 << (real_x - 8));
                    }
                }
            }
        }

        ioctl(dev_led1616_fd, LED_DISPLAY, &data);
        usleep(50000); 

        offset++;
        if (offset >= total_columns)
        {
            offset = 0; 
        }
    }

    // 清空点阵显存
    memset(data.code, 0, sizeof(data.code));
    ioctl(dev_led1616_fd, LED_DISPLAY, &data);
    printf("[动画提示] 检测到选择难度，开场动画已关闭。\n");
}
// ====================================================================

// 游戏主线程
void *tetris_thread(void *arg)
{
    (void)arg;
    printf("-- 俄罗斯方块线程启动成功 --\n");

    dev_led1616_fd = open(DEV_LED1616, O_RDWR);
    dev_beep_fd = open(DEV_BEEP, O_RDWR);
    if (dev_led1616_fd < 0 || dev_beep_fd < 0)
    {
        perror("打开外设失败");
        return NULL;
    }

    // 默认基础初始化
    tetrisScore = 0;
    tetrisLevel = INIT_LEVEL;
    tetrisSpeed = (10 - tetrisLevel) * 100000;
    tetrisKey = 0;

    // 显示选关终端 UI 
    start_interface1();

    // 【核心联动位置】调用无限循环动画，它会一边滚动，一边帮你盯住 `tetrisKey` 
    // 直到玩家按下 '1'~'9' 难度键，它才会退出并保持 `tetrisKey` 供下方主逻辑截获
    tetris_opening_animation();

    // 主逻辑直接捕获刚才退出动画时的红外难度键值，无缝初始化参数
    while (1)
    {
        if (tetrisKey >= '1' && tetrisKey <= '9')
        {
            tetrisLevel = tetrisKey - '0';
            tetrisSpeed = (10 - tetrisLevel) * 100000;
            tetrisKey = 0;
            break;
        }
        usleep(10000);
    }

    // 后续原版游戏启动与下落主逻辑，完全不修改保持原样
    start_interface2();
    while (tetrisKey != 'O')
    {
        usleep(10000);
    }
    tetrisKey = 0;

    start_interface3();

    tetris_init();
    currentBlock = create_block();
    tetris_print();

    while (1)
    {
        if (tetrisKey != 0)
        {
            TetrisBlock tempBlock = currentBlock;
            int doMove = 0;

            switch (tetrisKey)
            {
                case 'L':
                    tempBlock.x -= 1;
                    break;
                case 'R':
                    tempBlock.x += 1;
                    break;
                case 'D':
                    tempBlock.y += 1;
                    break;
                case 'U':
                    tempBlock.rotate = (tempBlock.rotate + 1) % BLOCK_ROTATE_COUNT;
                    memcpy(tempBlock.shape, blockShapes[tempBlock.type][tempBlock.rotate], sizeof(tempBlock.shape));
                    break;
                default:
                    break;
            }

            if (!check_collision(tempBlock))
            {
                currentBlock = tempBlock;
                doMove = 1;
            }

            if (doMove)
            {
                tetris_move_sound();
            }
            tetrisKey = 0;
        }

        usleep(tetrisSpeed);
        TetrisBlock downBlock = currentBlock;
        downBlock.y += 1;

        if (check_collision(downBlock))
        {
            if (currentBlock.y < 0)
            {
                tetris_game_over();
                return NULL;
            }

            merge_block_to_map(currentBlock);

            int lines = clear_full_lines();
            if (lines > 0)
            {
                tetrisScore += lines * SCORE_PER_LINE;
                tetris_clear_sound();
                if ((tetrisScore / 1000) >= tetrisLevel && tetrisLevel < 9)
                {
                    tetrisLevel++;
                    tetrisSpeed = (10 - tetrisLevel) * 100000;
                }
            }

            currentBlock = create_block();
            if (check_collision(currentBlock))
            {
                tetris_game_over();
                return NULL;
            }
        }
        else
        {
            currentBlock = downBlock;
        }

        tetris_print();
    }
    return NULL;
}

void tetris_init()
{
    memset(gameMap, 0, sizeof(gameMap));
    srand((unsigned)time(NULL));
}

TetrisBlock create_block()
{
    TetrisBlock block;
    block.type = rand() % BLOCK_TYPE_COUNT;
    block.rotate = rand() % BLOCK_ROTATE_COUNT;
    memcpy(block.shape, blockShapes[block.type][block.rotate], sizeof(block.shape));

    block.x = (MAPWIDTH - 4) / 2;
    block.y = -3; 
    return block;
}

int check_collision(TetrisBlock block)
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (block.shape[i][j] == 1)
            {
                int x = block.x + j;
                int y = block.y + i;

                if (x < 0 || x >= MAPWIDTH)
                    return 1;
                if (y >= MAPLENGTH)
                    return 1;
                if (y >= 0 && gameMap[y][x] == 1)
                    return 1;
            }
        }
    }
    return 0;
}

void merge_block_to_map(TetrisBlock block)
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (block.shape[i][j] == 1)
            {
                int x = block.x + j;
                int y = block.y + i;
                if (y >= 0 && y < MAPLENGTH && x >= 0 && x < MAPWIDTH)
                {
                    gameMap[y][x] = 1;
                }
            }
        }
    }
}

int clear_full_lines()
{
    int clearCount = 0;
    for (int i = MAPLENGTH - 1; i >= 0; i--)
    {
        int isFull = 1;
        for (int j = 0; j < MAPWIDTH; j++)
        {
            if (gameMap[i][j] == 0)
            {
                isFull = 0;
                break;
            }
        }
        if (isFull)
        {
            clearCount++;
            for (int k = i; k > 0; k--)
            {
                memcpy(gameMap[k], gameMap[k-1], sizeof(gameMap[k]));
            }
            memset(gameMap[0], 0, sizeof(gameMap[0]));
            i++;
        }
    }
    return clearCount;
}

void tetris_print()
{
    struct led1616Data data;
    memset(data.code, 0, sizeof(data.code));
    int real_x; 

    for (int y = 0; y < MAPLENGTH; y++)
    {
        for (int logic_x = 0; logic_x < MAPWIDTH; logic_x++)
        {
            if (gameMap[y][logic_x] == 1)
            {
                real_x = (MAPWIDTH - 1) - logic_x; 
                int pos;
                if (real_x < 8) {
                    pos = 2 * y + 1;         
                    data.code[pos] |= (1 << real_x);
                } else {
                    pos = 2 * y;             
                    data.code[pos] |= (1 << (real_x - 8));
                }
            }
        }
    }

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (currentBlock.shape[i][j] == 1)
            {
                int logic_x = currentBlock.x + j;
                int y = currentBlock.y + i;
                
                if (logic_x >= 0 && logic_x < MAPWIDTH && y >= 0 && y < MAPLENGTH)
                {
                    real_x = (MAPWIDTH - 1) - logic_x;
                    int pos;
                    if (real_x < 8) {
                        pos = 2 * y + 1;
                        data.code[pos] |= (1 << real_x);
                    } else {
                        pos = 2 * y;
                        data.code[pos] |= (1 << (real_x - 8));
                    }
                }
            }
        }
    }

    ioctl(dev_led1616_fd, LED_DISPLAY, &data);
}

void tetris_move_sound()
{
    const char on[] = "2000,50";
    const char off[] = "0,0";
    write(dev_beep_fd, on, strlen(on));
    usleep(50000);
    write(dev_beep_fd, off, strlen(off));
}

void tetris_clear_sound()
{
    const char on[] = "1500,80";
    const char off[] = "0,0";
    write(dev_beep_fd, on, strlen(on));
    usleep(100000);
    write(dev_beep_fd, off, strlen(off));
}

typedef struct {
    int freq;        
    int duration_ms; 
} OverNote;

void tetris_game_over()
{
    pthread_cancel(id_score_thread);
    pthread_cancel(id_infrared_control_thread);
    usleep(50000); 

    struct led1616Data current_scene_data;
    struct led1616Data full_off_data;
    
    int real_x;
    memset(current_scene_data.code, 0, sizeof(current_scene_data.code));
    memset(full_off_data.code, 0, sizeof(full_off_data.code));

    for (int y = 0; y < MAPLENGTH; y++)
    {
        for (int logic_x = 0; logic_x < MAPWIDTH; logic_x++)
        {
            if (gameMap[y][logic_x] == 1)
            {
                real_x = (MAPWIDTH - 1) - logic_x; 
                int pos;
                if (real_x < 8) {
                    pos = 2 * y + 1;         
                    current_scene_data.code[pos] |= (1 << real_x);
                } else {
                    pos = 2 * y;             
                    current_scene_data.code[pos] |= (1 << (real_x - 8));
                }
            }
        }
    }

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (currentBlock.shape[i][j] == 1)
            {
                int logic_x = currentBlock.x + j;
                int y = currentBlock.y + i;
                
                if (logic_x >= 0 && logic_x < MAPWIDTH && y >= 0 && y < MAPLENGTH)
                {
                    real_x = (MAPWIDTH - 1) - logic_x;
                    int pos;
                    if (real_x < 8) {
                        pos = 2 * y + 1;
                        current_scene_data.code[pos] |= (1 << real_x);
                    } else {
                        pos = 2 * y;
                        current_scene_data.code[pos] |= (1 << (real_x - 8));
                    }
                }
            }
        }
    }

    printf("\n!!!!! GAME OVER !!!!!\n");

    OverNote go_melody[] = {
        {880, 250},   
        {698, 250},   
        {587, 300},   
        {494, 350},   
        {349, 600}    
    };
    int count = sizeof(go_melody) / sizeof(OverNote);
    char buf[20] = { 0 };

    for (int i = 0; i < count; i++) {
        snprintf(buf, sizeof(buf), "%d,50", go_melody[i].freq);
        write(dev_beep_fd, buf, strlen(buf));

        if (i % 2 == 0) {
            ioctl(dev_led1616_fd, LED_DISPLAY, &current_scene_data);
        } else {
            ioctl(dev_led1616_fd, LED_DISPLAY, &full_off_data);
        }

        usleep(go_melody[i].duration_ms * 1000);
        write(dev_beep_fd, "0,0", 3);
        usleep(20000);
    }

    ioctl(dev_led1616_fd, LED_DISPLAY, &full_off_data);
    usleep(100000); 

    end_interface();
    printf("\n[游戏提示] 正在通知主系统安全释放资源...\n");
    
    int temp_fd = dev_beep_fd;
    dev_beep_fd = -1;       
    close(temp_fd);         
    
    int temp_led_fd = dev_led1616_fd;
    dev_led1616_fd = -1;    
    close(temp_led_fd);

    raise(SIGINT); 
}