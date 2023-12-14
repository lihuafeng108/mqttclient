#pragma once

///////////////////////////////////////////////////////////////////////////////
//#include "mqtt_client.h" //lib all inc file !

#ifndef u8
    typedef unsigned char        u8;
#endif
#ifndef u16
    typedef unsigned short      u16;
#endif
#ifndef u32
    typedef unsigned int        u32;
#endif
#ifndef unlong
    typedef unsigned long        unlong;
#endif

#ifndef u64
    typedef unsigned long long  u64;
#endif

#ifndef s8
    typedef char      ss8;
#endif
#ifndef s16
    typedef short     ss16;
#endif
#ifndef s32
    typedef int       s32;
#endif
#ifndef s64
    typedef long long s64;
#endif
#ifndef f32
    typedef float     f32;
#endif
#ifndef f64
    typedef double    f64;
#endif

///////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C" {
#endif
///////////////////////////////////////////////////////////////////////////////

//
//客户端句柄
//
typedef void* mq_client_t;

/**************************************************************************
 * @brief mqtt 消息回调函数（当接收到订阅的消息时触发此回调）
 * @param mq :[in] 客户端句柄
 * @param qos :[in] 主题质量
 * @param topic :[in] 主题
 * @param payload :[in] 接收的数据（可能为NULL）
 * @param payloadlen :[in] 数据大小（字节）
 * @param userata :[in] 用户数据
**************************************************************************/
typedef void (*mq_msg_cb)(mq_client_t mq, int qos, const char* topic, const void* payload, unsigned int payloadlen, void* userata);

/**************************************************************************
 * @brief 【可选接口】mqtt重连时触发此回调
 * @param mq :[in] 客户端句柄
 * @param userdata :[in] 用户数据
**************************************************************************/
typedef void (*mq_reconnect_cb)(mq_client_t mq, void* userdata);

int mq_set_reconnect_cb(mq_client_t mq, mq_reconnect_cb cb, void* userdata);


/**************************************************************************
 * @brief  新建客户端
 * @param host :[in] 服务器地址
 * @param port :[in] 服务器端口
 * @param username :[in] 用户名
 * @param pws :[in] 密码
 * @param clientid :[in] 客户端编号
 * @return mq_client_t : 客户端句柄
**************************************************************************/
mq_client_t mq_new_client(const char* host, int port,
                     const char * username, const char * pws,
                     const char *clientid);

/**************************************************************************
 * @brief 设置消息回调函数，当收到订阅的主题时触发此回调
 * @param mq :[in] 客户端句柄（mq_new_client返回的）
 * @param cb :[in] 回调函数
 * @param userdata :[in] 用户数据，将传给回调函数
 * @return int : 0-成功，1-失败
**************************************************************************/
int mq_set_message_cb(mq_client_t mq, mq_msg_cb cb, void* userdata);


/**************************************************************************
 * @brief 订阅主题
 * @param mq :[in] 客户端句柄（mq_new_client返回的）
 * @param qos :[in] 主题质量：[0,1,2]
 * @param topic :[in] 主题，支持通配符“#”和“+”
 * @return int : 0-成功，其它-失败
**************************************************************************/
int mq_subscribe(mq_client_t mq, int qos, const char* topic);

/**************************************************************************
 * @brief 取消订阅
 * @param mq :[in] 客户端句柄（mq_new_client返回的）
 * @param topic :[in] 主题，支持通配符“#”和“+”
 * @return int : 0-成功，其它-失败
**************************************************************************/
int mq_unsubscribe(mq_client_t mq, const char* topic);

/**************************************************************************
 * @brief 发布主题消息
 * @param mq :[in] 客户端句柄（mq_new_client返回的）
 * @param qos :[in] 主题质量：[0,1,2]
 * @param topic :[in] 主题
 * @param payload :[in] 需要发布的数据
 * @param paylaodlen :[in] 数据大小（字节）
 * @return int :  0-成功，其它-失败
**************************************************************************/
int mq_public(mq_client_t mq, int qos, const char* topic, const void* payload, unsigned int paylaodlen);


/**************************************************************************
 * @brief 开始运行（非阻塞，内部起线程loop）；
 * @param mq :[in] 客户端句柄（mq_new_client返回的）
 * @param looptime :[in] loop间隔（毫秒）0--则使用默认的500ms
 * @return int : 0-成功，其它-失败
 * @note
**************************************************************************/
int mq_start(mq_client_t mq, unsigned int looptime);

/**************************************************************************
 * @brief 停止运行，此时 订阅和发布 接口均无效；
 * @param mq :[in] 客户端句柄（mq_new_client返回的）
 * @param force :[in]
 * @return int :  0-成功，其它-失败
**************************************************************************/
int mq_stop(mq_client_t mq, int force);

/**************************************************************************
 * @brief 释放 mqtt 客户端资源 *
 * @param mq :[in] 客户端句柄（mq_new_client返回的）
 * @return int :  0-成功，其它-失败
**************************************************************************/
int mq_free(mq_client_t mq);

/**************************************************************************
 * @brief 客户端是否已连接成功
 * @param mq :[in] 客户端句柄（mq_new_client返回的）
 * @return int :  1-已连接，0-未连接
**************************************************************************/
int mq_is_connect(mq_client_t mq);

///////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
}
#endif


