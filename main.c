#include"global_variable.h"

extern pthread_t id_infrared_control_thread;
extern pthread_t id_score_thread;
extern pthread_t id_tetris_thread;

int main()
{
    start_interface1();

    // 创建红外线程
    if (pthread_create(&id_infrared_control_thread, 0, infrared_control_thread, NULL) != 0)
    {
        printf("红外线程创建失败\n");
        return -1;
    }
    // 创建计分线程
    if (pthread_create(&id_score_thread, 0, score_thread, NULL) != 0)
    {
        printf("计分线程创建失败\n");
        return -1;
    }
    // 创建游戏主线程
    if (pthread_create(&id_tetris_thread, 0, tetris_thread, NULL) != 0)
    {
        printf("游戏线程创建失败\n");
        return -1;
    }

    // 注册Ctrl+C退出回调
    signal(SIGINT, ReleaseResource);

    // 等待所有线程
    pthread_join(id_infrared_control_thread, NULL);
    pthread_join(id_score_thread, NULL);
    pthread_join(id_tetris_thread, NULL);

    return 0;
}

// 释放线程、设备资源
void ReleaseResource()
{
    pthread_cancel(id_infrared_control_thread);
    pthread_cancel(id_score_thread);
    pthread_cancel(id_tetris_thread);

    close(dev_led1616_fd);
    close(dev_zlg7290_fd);
    close(dev_ir_fd);
    close(dev_beep_fd);

    printf("\n所有资源已释放，程序退出\n");
    exit(0);
}