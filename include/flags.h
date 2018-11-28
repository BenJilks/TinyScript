#ifndef FLAGS_H
#define FLAGS_H

#define DEBUG 0

#if DEBUG
#define LOG(...) printf(__VA_ARGS__)
#else
#define LOG(...) ;
#endif

#endif // FLAGS_H
