
#include "mq_client.h"

#include <stdio.h>
#include <unistd.h>
#include <string>

#ifdef __cplusplus
    //extern "C" {
#endif
///////////////////////////////////////////////////////////////////////////////

//static int gs_taskid = 0;
void test_mq_msg_cb(mq_client_t mq, int qos, const char* topic, const void* payload, unsigned int payloadlen, void* userata)
{
    printf("[MSG CBK] topic[%s], qos[%d], msg[%s]\n",topic,qos,(char*)payload);
}


void xx_reconnect_cb(mq_client_t client, void* userdata)
{
    printf("[RECON CBK] !!!!!!!!!\n");
}

int main(int argc, char **argv)
{
    //服务器地址改成自己的服务器地址，例如我测试用的百度智能云的物联网核心套件IoT Core生产的MQTT 连接信息
    mq_client_t *cln = (mq_client_t *)mq_new_client("Broker 地址", 1883, "MQTT 用户名", "MQTT 密码", "任意字符，但是每个客户端好像要唯一");
    if(!cln)
    {
        printf("new client fail!\n");
        return 0;
    }
    if(0!=mq_set_message_cb(cln, test_mq_msg_cb,NULL))
    {
        printf("mq_set_message_cb fail!\n");
        mq_free(cln);
        return 0;
    }
    if(0!=mq_start(cln,100))
    {
        printf("mq_start fail!\n");
        mq_free(cln);
        return 0;
    }

    #if 1
    sleep(1);
    int ret = mq_subscribe(cln, 1, "/ant/topic1");
    printf("+++ mq_subscribe ret:%d\n",ret);
    #else
    int ret = 0;
    #endif


    while(1)
    {
        if(!mq_is_connect(cln))
        {
            printf("--- not connect ---\n");
            sleep(1);
            continue;
        }

        sleep(3);
        std::string msg {"hello, your are so beautiful!"};
        ret = mq_public(cln, 1, "/ant/topic1", msg.c_str(), msg.size());
        printf("+++ mq_public ret:%d\n",ret);
        //sleep(3);
    }
}
///////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
    //}
#endif
///////////////////////////////////////////////////////////////////////////////


