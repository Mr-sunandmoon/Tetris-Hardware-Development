#include"global_variable.h"

extern char tetrisKey;

// 红外键值映射表
typedef struct key {
    unsigned char key;
    char key_desc;
}key1_t;

key1_t table[] = {
    {KEY_1,     '1'},
    {KEY_2,     '2'},
    {KEY_3,     '3'},
    {KEY_4,     '4'},
    {KEY_5,     '5'},
    {KEY_6,     '6'},
    {KEY_7,     '7'},
    {KEY_8,     '8'},
    {KEY_9,     '9'},
    {KEY_UP,    'U'},
    {KEY_DOWN,  'D'},
    {KEY_LEFT,  'L'},
    {KEY_RIGHT, 'R'},
    {KEY_ENTER, 'O'},
};

void *infrared_control_thread(void *arg)
{
    (void)arg;
    printf("-- 红外线程启动成功 --\n");

    struct input_event ev;
    int ret;

    // 非阻塞打开红外设备
    dev_ir_fd = open(DEV_IR, O_RDONLY | O_NONBLOCK);
    if (dev_ir_fd < 0)
    {
        perror("打开红外设备失败");
        return NULL;
    }

    while (1)
    {
        ret = read(dev_ir_fd, &ev, sizeof(struct input_event));
        if (ret <= 0)
        {
            usleep(2000);
            continue;
        }

        // 只处理按键按下事件
        if (ev.type == EV_KEY && ev.value == 1)
        {
            for (int i = 0; i < sizeof(table) / sizeof(table[0]); i++)
            {
                if (table[i].key == ev.code)
                {
                    tetrisKey = table[i].key_desc;
#if DEBUG
                    printf("检测到按键: %c\n", tetrisKey);
#endif
                    break;
                }
            }
        }
    }
    return NULL;
}