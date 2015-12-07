#ifndef SINK_H
#define SINK_H

struct pc_comm
{
   int into;
   int outof;

   int ctl_pipe[2];
   int msg_pipe[2];

};


/* ========================================================================= */
struct pc_comm *LaunchAsSink(char *listen_address,
                             char *listen_protocol,
                             char *listen_port);


/* ========================================================================= */
int SendStopSink(int into);


/* ========================================================================= */
int CheckForChildMessages(int outof,
                          unsigned long mswait,
                          char *cp);





#endif
