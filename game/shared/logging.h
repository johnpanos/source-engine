#ifndef PORTAL2_COMPAT_LOGGING_H
#define PORTAL2_COMPAT_LOGGING_H

typedef int LoggingChannelID_t;
typedef int LoggingFileHandle_t;

#define DECLARE_LOGGING_CHANNEL( name ) extern LoggingChannelID_t name
#define BEGIN_DEFINE_LOGGING_CHANNEL( name, ... ) LoggingChannelID_t name = 0
#define ADD_LOGGING_CHANNEL_TAG( ... )
#define END_DEFINE_LOGGING_CHANNEL()
#define Log_Msg( channel, ... ) Msg( __VA_ARGS__ )
#define Log_Warning( channel, ... ) Warning( __VA_ARGS__ )
#define Log_Error( channel, ... ) Warning( __VA_ARGS__ )

#endif
