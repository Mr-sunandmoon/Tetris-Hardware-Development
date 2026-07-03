#include"global_variable.h"

extern int tetrisLevel;
extern int tetrisScore;

void *score_thread(void *arg)
{
    (void)arg;
    printf("-- 计分系统启动成功 --\n");

    char scoreBuff[8] = "00000000";
    dev_zlg7290_fd = open(DEV_ZLG7290, O_RDWR);
    if (dev_zlg7290_fd < 0)
    {
        perror("打开数码管设备失败");
        return NULL;
    }

    while (1)
    {
        itoa(tetrisScore, scoreBuff, 10);
        itoa(tetrisLevel, scoreBuff + 4, 10);
        ioctl(dev_zlg7290_fd, SET_VAL, scoreBuff);
        usleep(100000);
    }
    return NULL;
}

// int 转字符串
void itoa(int num, char *str, int radix)
{
    char index[] = "0123456789ABCDEF";
    unsigned unum;
    int i = 0, j = 0, k = 0;

    if (radix == 10 && num < 0)
    {
        unum = (unsigned)-num;
        str[i++] = '-';
    }
    else
    {
        unum = (unsigned)num;
    }

    do
    {
        str[i++] = index[unum % (unsigned)radix];
        unum /= radix;
    } while (unum);
    str[i] = '\0';

    if (str[0] == '-')
        k = 1;
    char temp;
    for (j = k; j <= (i - 1) / 2; j++)
    {
        temp = str[j];
        str[j] = str[i - 1 + k - j];
        str[i - 1 + k - j] = temp;
    }
}