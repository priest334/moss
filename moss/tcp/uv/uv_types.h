#pragma once


#define container_of(ptr, type, member) \
  ((type *) ((char *) (ptr) - offsetof(type, member)))

#ifdef _WIN32
#	define MIRABILIS_CHANNEL_TCP_LISTENER "\\\\?\\pipe\\moss\\channel\\tcp-listener"
#else
#	define MIRABILIS_CHANNEL_TCP_LISTENER "/var/run/moss.channel.tcp-listener"
#endif

using worker_id_t = int;
