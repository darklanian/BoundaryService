#ifndef _OPENGLESCAMERATEST_LOGGER_H_
#define _OPENGLESCAMERATEST_LOGGER_H_

#include <android/log.h>

#define TAG "[LANIAN]"
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__))

#endif //_OPENGLESCAMERATEST_LOGGER_H_