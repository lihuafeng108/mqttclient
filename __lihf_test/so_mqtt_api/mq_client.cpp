#include "mqttclient.h"

#include "mq_client.h"

#include <errno.h>

///////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C" {
#endif
///////////////////////////////////////////////////////////////////////////////

/*
** MQTT 客户端定义
*/
typedef struct _mq_client
{
    mqtt_client_t *client;
    char client_id[128];
    char username[128];
    char pws[128];
    char ip[128];
    char port[8];
    void *userdata;
    mq_msg_cb msg_cb;
    mq_reconnect_cb recon_cb;
    void *recon_userdata;
    //线程相关
    unsigned int looptime;
    pthread_t thread_id;
    int thread_run;

} stu_mq_client_t;

static u32 gs_mqclient_inited = 0; // 只能初始化一次
static pthread_mutex_t gs_mutex = PTHREAD_MUTEX_INITIALIZER;

//-----------------------------------------------------------------------------
static void x_msg_handle(void *client, message_data_t *msg)
{
    stu_mq_client_t *mc = NULL;
    mqtt_client_t *c = (mqtt_client_t *)client;
    c ? (mc = (stu_mq_client_t *)mqtt_get_userdata(c)) : (mc = NULL);
    if (c && mc && msg)
    {
        printf("<--[%p:%s] <RECV> topic[%s], qos[%d], msg[%s]",
                mc, mc->client_id, msg->topic_name, msg->message->qos, (char *)(msg->message->payload));
        if (mc->msg_cb)
        {
            mc->msg_cb(mc, msg->message->qos, msg->topic_name, msg->message->payload, msg->message->payloadlen, mc->userdata);
        }
    }
}

static void x_reconnect_cb(void* client, void* userdata)
{
    stu_mq_client_t *mc = (stu_mq_client_t *)userdata;
    if(mc)
    {
        printf("***[%p:%s] reconnect ...\n",mc, mc->client_id);
        if(mc->recon_cb)
        {
            mc->recon_cb(mc, userdata);
        }
    }
}

//-----------------------------------------------------------------------------
static int mq_init(void)
{
    if (!gs_mqclient_inited)
    {
        // mqtt_log_init();
        // mqtt_set_log(print_log);
    }
    gs_mqclient_inited = 1;
    return 0;
}

//-----------------------------------------------------------------------------
mq_client_t mq_new_client(const char *host, int port,
                          const char *username, const char *pws,
                          const char *clientid)
{
     mq_init();
     if (!host || port < 1 || port > 65535)
    {
        printf("param invalid or null.\n");
        return NULL;
    }
    stu_mq_client_t *mc = (stu_mq_client_t *)calloc(1, sizeof(stu_mq_client_t));
    if (!mc)
    {
        printf("MM error(%d:%s)\n", errno, strerror(errno));
        return NULL;
    }
    mc->client = mqtt_lease();
    if (!mc->client)
    {
        printf("create client error(%d:%s)\n", errno, strerror(errno));
        free(mc);
        return NULL;
    }

    snprintf(mc->ip, 127, "%s", host);
    snprintf(mc->port, 127, "%d", port);

    if (username && username[0] != 0)
    {
        snprintf(mc->username, 127, "%s", username);
        mqtt_set_user_name(mc->client, mc->username);
    }
    if (pws && pws[0] != 0)
    {
        snprintf(mc->pws, 127, "%s", pws);
        mqtt_set_password(mc->client, mc->pws);
    }
    if (clientid && clientid[0] != 0)
    {
        snprintf(mc->client_id, 127, "%s", clientid);
        mqtt_set_client_id(mc->client, mc->client_id); //"ANT|A01|myid"
    }
    mqtt_set_host(mc->client, mc->ip);
    mqtt_set_port(mc->client, mc->port);
    mqtt_set_clean_session(mc->client, 1);
    mqtt_set_userdata(mc->client, mc); // 将自身传入，回调中使用

    mqtt_set_reconnect_handler(mc->client, x_reconnect_cb);
    mqtt_set_reconnect_data(mc->client, mc);

    printf("[%p:%s] create client success, host:port=%s:%d\n", mc, clientid ? clientid : "", host, port);
    return (mq_client_t)mc;
}

//-----------------------------------------------------------------------------
int mq_set_message_cb(mq_client_t mq, mq_msg_cb cb, void *userdata)
{
     stu_mq_client_t *mc = (stu_mq_client_t *)mq;
    if (mc && cb)
    {
        mc->msg_cb = cb;
        mc->userdata = userdata;
        return 0;
    }
    return -1;
}
//-----------------------------------------------------------------------------
int mq_set_reconnect_cb(mq_client_t mq, mq_reconnect_cb cb, void* userdata)
{
     stu_mq_client_t *mc = (stu_mq_client_t *)mq;
    if (mc && cb)
    {
        mc->recon_cb = cb;
        mc->recon_userdata = userdata;
        return 0;
    }
    return -1;
}
//-----------------------------------------------------------------------------
int mq_subscribe(mq_client_t mq, int qos, const char *topic)
{
    int ret = 0;
    stu_mq_client_t *mc = (stu_mq_client_t *)mq;
    if (!mc || !topic)
    {
        printf("<SUB> param invalid or null.\n");
        return -1;
    }
    if ((ret = mqtt_is_connect_success(mc->client)) != MQTT_SUCCESS_ERROR)
    {
        printf("[%p:%s] <SUB> failed (%d:%s)\n", mc, mc->client_id, ret, mqtt_errormsg(ret));
        return -2;
    }
    ret = mqtt_subscribe(mc->client, topic, (mqtt_qos_t)qos, x_msg_handle);
    printf("[%p:%s] <SUB> +++ topic[%s], qos[%d] ret[%d:%s] +++\n", mc, mc->client_id, topic, qos, ret, mqtt_errormsg(ret));
    return ret == MQTT_SUCCESS_ERROR ? 0 : -3;
}
//-----------------------------------------------------------------------------
int mq_unsubscribe(mq_client_t mq, const char *topic)
{
    int ret = 0;
    stu_mq_client_t *mc = (stu_mq_client_t *)mq;
    if (!mc || !topic)
    {
        printf("<UNSUB> param invalid or null.\n");
        return -1;
    }
    if ((ret = mqtt_is_connect_success(mc->client)) != MQTT_SUCCESS_ERROR)
    {
        printf("[%p:%s] <UNSUB> failed (%d:%s)\n", mc, mc->client_id, ret, mqtt_errormsg(ret));
        return -2;
    }
    ret = mqtt_unsubscribe(mc->client, topic);
    printf("[%p:%s] <UNSUB> --- topic[%s], ret[%d:%s] ---\n", mc, mc->client_id, topic, ret, mqtt_errormsg(ret));
    return ret == MQTT_SUCCESS_ERROR ? 0 : -3;
}

//-----------------------------------------------------------------------------
int mq_public(mq_client_t mq, int qos, const char *topic, const void *payload, unsigned int paylaodlen)
{
    int ret = 0;
    stu_mq_client_t *mc = (stu_mq_client_t *)mq;
    if (!mc || !topic)
    {
        printf("<PUB> param invalid or null.\n");
        return -1;
    }
    if ((ret = mqtt_is_connect_success(mc->client)) != MQTT_SUCCESS_ERROR)
    {
        printf("[%p:%s] <PUB> not connected (%d:%s)\n", mc, mc->client_id, ret, mqtt_errormsg(ret));
        return -2;
    }
    mqtt_message_t msg;
    memset((char *)&msg, 0, sizeof(msg));
    msg.payload = (void*)payload;
    msg.payloadlen = paylaodlen;
    msg.qos = (mqtt_qos_t)qos;
    ret = mqtt_publish(mc->client, topic, &msg);
    if (ret != MQTT_SUCCESS_ERROR)
    {
        printf("[%p:%s] <PUB> failed (%d:%s)\n", mc, mc->client_id, ret, mqtt_errormsg(ret));
    }
    else
    {
        printf("-->[%p:%s] <PUB> topic[%s], qos[%d], msg[%s]\n",
                mc, mc->client_id, topic, qos, (char *)payload);
    }
    return ret == MQTT_SUCCESS_ERROR ? 0 : -3;
}

//-----------------------------------------------------------------------------

static void *x_thread_func(void *arg)
{
    pthread_t thid = pthread_self();
    pthread_detach(thid);
    stu_mq_client_t *mc = (stu_mq_client_t *)arg;
    if (!mc)
    {
        return 0;
    }
    //
    printf("TH[%x]MC[%p:%s] to connect server[%s:%s], user[%s], pws[%s] ......\n",
            (unsigned int)thid, mc, mc->client_id, mc->ip, mc->port, mc->username, mc->pws);
    unsigned int count = 0;
    while (mc->thread_run)
    {
        int ret = mqtt_connect(mc->client);
        if (ret != MQTT_SUCCESS_ERROR)
        {
            count++;
            printf("TH[%x]MC[%p:%s](%u) connect fail (%d:%s)\n",
                    (unsigned int)thid, mc, mc->client_id, count, ret, mqtt_errormsg(ret));
            //delay 1s
            int tick = 0;
            while(mc->thread_run && tick++ <10 )
            {
                usleep(100000); //100ms
            }
            if (mc->thread_run)
            {
                printf("TH[%x]MC[%p:%s](%u) retry after 1s ...\n", (unsigned int)thid, mc, mc->client_id,count);
                continue;
            }
        }
        else
        {
            printf("TH[%x]MC[%p:%s](%u) connect server success, exit thread!\n", (unsigned int)thid, mc, mc->client_id, count);
            break;
        }
    }
    printf("TH[%x]MC[%p:%s](%u) ---------- thread(%x) exit ----------\n",
            (unsigned int)thid, mc, mc->client_id, count, (unsigned int)mc->thread_id);
    mc->thread_id = 0;
    mc->thread_run = 0;
    return 0;
}
//-----------------------------------------------------------------------------
int mq_start(mq_client_t mq, unsigned int looptime)
{
    stu_mq_client_t *mc = (stu_mq_client_t *)mq;
    if (!mc)
    {
        printf("start: param invalid or null.\n");
        return -1;
    }
    if (mc->thread_id != 0)
    {
        printf("[%p:%s](thid:%x) exist and runing.\n", mc, mc->client_id, (unsigned int)mc->thread_id);
        return 0;
    }
    // 创建线程去连接服务器！
    mc->looptime = looptime;
    mc->thread_run = 1;
    int ret = pthread_create(&mc->thread_id, 0, x_thread_func, mc);
    if (0 != ret || mc->thread_id < (pthread_t)1)
    {
        printf("[%p:%s] pthread_create error(%d:%s), ret:%d.\n", mc, mc->client_id, errno, strerror(errno), ret);
        mc->thread_id = 0;
        mc->thread_run = 0;
        return -2;
    }
    printf("[%p:%s] create thread(%x) success.\n", mc, mc->client_id, (unsigned int)mc->thread_id);
    return 0;
}

//-----------------------------------------------------------------------------
int mq_stop(mq_client_t mq, int force)
{
    stu_mq_client_t *mc = (stu_mq_client_t *)mq;
    if (!mc)
    {
        printf("stop: param invalid or null.\n");
        return -1;
    }
    mc->thread_run = 0;
    int ret = mqtt_is_connect_success(mc->client);
    if (ret != MQTT_CLEAN_SESSION_ERROR && ret != MQTT_NULL_VALUE_ERROR)
    {
        ret = mqtt_disconnect(mc->client);
        printf("[%p:%s] stop, do disconnect ret[%d:%s]...\n",mc,mc->client_id, ret, mqtt_errormsg(ret));
        sleep(2);
    }
    printf("[%p:%s] stop, do disconnect ret[%d:%s]...\n",mc,mc->client_id, ret, mqtt_errormsg(ret));
    mc->thread_id = 0;
    return 0;
}

//-----------------------------------------------------------------------------
int mq_free(mq_client_t mq)
{
    stu_mq_client_t *mc = (stu_mq_client_t *)mq;
    if (!mc)
    {
        printf("free, param invalid or null.\n");
        return -1;
    }
    int ret1 = mqtt_disconnect(mc->client);
    int ret2 = mqtt_release(mc->client);
    printf("[%p:%s] free, do disconnect ret[%d:%s]...\n", mc, mc->client_id, ret1, mqtt_errormsg(ret1));
    printf("[%p:%s] free, do release ret[%d:%s]...\n", mc, mc->client_id, ret2, mqtt_errormsg(ret2));
    //
    mc->client; // 没有释放内存的操作，会有泄露，需要修改mqtt库内部接口（待完成）！！！
    //
    free(mc);
    mc = NULL;
    return 0;
}

//-----------------------------------------------------------------------------
int mq_is_connect(mq_client_t mq)
{
    stu_mq_client_t *mc = (stu_mq_client_t *)mq;
    if (!mc || !mc->client)
    {
        return 0;
    }
    int ret = mqtt_is_connect_success(mc->client);
    return ret == MQTT_SUCCESS_ERROR ? 1 : 0;
}

///////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
}
#endif
///////////////////////////////////////////////////////////////////////////////

