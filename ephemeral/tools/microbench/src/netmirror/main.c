#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

#include "sender.h"
#include "receiver.h"
#include "sink.h"

#include "../lib/optparser.h"
#include "../lib/verbosity.h"

int run_as_sender(struct optbase *ob);
int launch_sink(struct optbase *ob);
int run_as_receiver(struct optbase *ob);
void capture_signal(int signo);

/* Used to check if the user wants to stop */
int g_stop = 0;

/* ========================================================================= */
int main(int argc, char *argv[])
{
   struct optbase *ob;

   char mode[MAX_VALUE_SIZE];

   /* Read in options */
   if(NULL == (ob = ReadOptions(argc, argv)))
      return(1);

   /* Validate that MODE is set (to something) */
   if ( IsInvalidOption(ob, "MODE", IVO_EXISTS) )
   {
      ErrorMessage("ERROR: MODE option is missing or invalid.\n");
      return(1);
   }

   /* Read the MODE value */
   if ( GetSTRValue(ob, mode, "MODE", 4, 8) )
   {
      ErrorMessage("ERROR: Problems parsing MODE value.\n");
      return(1);
   }

   /* Run as sender (and optionally (start) the sink) */
   if ( ( strcmp(mode, "SENDER") == 0 ) || ( strcmp(mode, "SINK") == 0 ) )
      return(run_as_sender(ob));

   /* Run the receiver */
   if ( ( strcmp(mode, "RECEIVER") == 0 ) || ( strcmp(mode, "MIRROR") == 0 ) )
      return(run_as_receiver(ob));

   /* Well... This is a problem. */
   ErrorMessage("ERROR: MODE value not understood.\n");
   return(1);
}



/* ========================================================================= */
int run_as_sender(struct optbase *ob)
{
   /* This used to be char junkbuf[0]; but the compiler complained. junkbuf
      is a pointer to the top of the auto variables. The point is to have
      a pointer to some data - but nothing of particular value. */
   int junk;
   char *junkbuf = (char *)&junk;


   struct roundtrip *rt;

   struct pc_comm *pcomm = NULL;
   int go; /* Used multiple times for "event loops" */
   char child_message[80];

   char mirror_address[MAX_VALUE_SIZE];
   char mirror_protocol[MAX_VALUE_SIZE];
   char mirror_port[MAX_VALUE_SIZE];

   unsigned long multiplier;
   unsigned short optimal_block;

   unsigned long run_seconds;

   char sink_address[MAX_VALUE_SIZE];
   char sink_protocol[MAX_VALUE_SIZE];
   char sink_port[MAX_VALUE_SIZE];

   time_t time_started;  /* Really about reporting. */
   time_t time_now;      /* The most recent time.   */
   time_t time_stop;


   /* Do some basic validation of the expected values */
   if ( IsInvalidOption(ob, "MIRROR_ADDRESS", IVO_EXISTS) )
   {
      ErrorMessage("ERROR: MIRROR_ADDRESS option is missing or invalid.\n");
      return(1);
   }
      
   if ( IsInvalidOption(ob, "MIRROR_PROTOCOL", IVO_EXISTS) )
   {
      ErrorMessage("ERROR: MIRROR_PROTOCOL option is missing or invalid.\n");
      return(1);
   }

   if ( IsInvalidOption(ob, "MIRROR_PORT", IVO_EXISTS) )
   {
      ErrorMessage("ERROR: MIRROR_PORT option is missing or invalid.\n");
      return(1);
   }

   if ( IsInvalidOption(ob, "RETURN_MULTIPLIER", IVO_NNNUMERIC) )
   {
      ErrorMessage("ERROR: RETURN_MULTIPLIER option is missing or invalid.\n");
      return(1);
   }

   if ( IsInvalidOption(ob, "OPTIMAL_BLOCK", IVO_EXISTS) )
   {
      optimal_block = DEFAULT_OPTIMAL_BLOCK;
   }
   else
   {
      if ( GetUSValue(ob, &optimal_block, "OPTIMAL_BLOCK", 0, 65535) )
      {
         ErrorMessage("ERROR: Problems parsing OPTIMAL_BLOCK value.\n");
         return(1);
      }
   }

   if ( IsInvalidOption(ob, "RUN_FOR", IVO_EXISTS) )
   {
      run_seconds = 0;
   }
   else
   {
      if ( GetTimeValue(ob, &run_seconds, "RUN_FOR", 0, ULONG_MAX) )
      {
         ErrorMessage("ERROR: Problems parsing RUN_FOR value.\n");
         return(1);
      }
   }

   if ( GetULValue(ob, &multiplier, "RETURN_MULTIPLIER", 0, ULONG_MAX) )
   {
      ErrorMessage("ERROR: Problems parsing RETURN_MULTIPLIER value.\n");
      return(1);
   }

   if ( multiplier > 0 )
   {
      /* This *could* be derived - it should not be *required* */
      if ( IsInvalidOption(ob, "SINK_ADDRESS", IVO_EXISTS) )
      {
         ErrorMessage("ERROR: SINK_ADDRESS option is missing or invalid.\n");
         return(1);
      }

      if ( IsInvalidOption(ob, "SINK_PORT", IVO_EXISTS) )
      {
         ErrorMessage("ERROR: SINK_PORT option is missing or invalid.\n");
         return(1);
      }
         
      if ( IsInvalidOption(ob, "SINK_PROTOCOL", IVO_EXISTS) )
      {
         ErrorMessage("ERROR: SINK_PROTOCOL option is missing or invalid.\n");
         return(1);
      }
   }
      
   if ( GetStrValue(ob, mirror_address, "MIRROR_ADDRESS", 4, MAX_VALUE_SIZE) )
   {
      ErrorMessage("ERROR: Problems parsing MIRROR_ADDRESS value.\n");
      return(1);
   }

   if ( GetSTRValue(ob, mirror_protocol, "MIRROR_PROTOCOL", 2, 4) )
   {
      ErrorMessage("ERROR: Problems parsing MIRROR_PROTOCOL value.\n");
      return(1);
   }

   if ( GetStrValue(ob, mirror_port, "MIRROR_PORT", 2, 5) )
   {
      ErrorMessage("ERROR: Problems parsing MIRROR_PORT value.\n");
      return(1);
   }

   if ( multiplier > 0 )
   {
      if ( GetStrValue(ob, sink_address, "SINK_ADDRESS", 4, MAX_VALUE_SIZE) )
      {
         ErrorMessage("ERROR: Problems parsing SINK_ADDRESS value.\n");
         return(1);
      }
      
      if ( GetSTRValue(ob, sink_protocol, "SINK_PROTOCOL", 2, 4) )
      {
         ErrorMessage("ERROR: Problems parsing SINK_PROTOCOL value.\n");
         return(1);
      }
         
      if ( GetStrValue(ob, sink_port, "SINK_PORT", 2, 5) )
      {
         ErrorMessage("ERROR: Problems parsing SINK_PORT value.\n");
         return(1);
      }
   }

   ReportStart("netmirror", NULL, "A module to bounce packets across the network.");

   /* Register signal handlers */
   /*
   signal(SIGTERM, capture_signal);
   signal(SIGQUIT, capture_signal);
   signal(SIGINT, capture_signal);
   signal(SIGHUP, NULL);
   */

   if ( NULL == ( rt = InitRoundTrip() ) )
      return(1);

   if ( InitSender(rt, mirror_address, mirror_protocol, mirror_port) )
      return(1);
   
   if ( InitReceiver(rt, sink_address, sink_protocol, sink_port, multiplier) )
      return(1);

   DebugMessage("Round trip multiplier is %lu.\n", multiplier);
   DebugMessage("Run time (in seconds) is: %lu.\n", run_seconds);

   if ( multiplier > 0 )
   {
      if (NULL == (pcomm = LaunchAsSink("ADDR", "PROTO", "PORT")))
         return(1);
   }
   
   /* Check for message from the sink (if we started it). */
   if (pcomm)
   {
      if(CheckForChildMessages(pcomm->outof, 250, child_message) > 0)
         VerboseMessage("Sink: %s\n", child_message);
      /* A sink is present */
   }


   rt->opt_block = optimal_block;
   rt->run_seconds = run_seconds;

   /* Start up the sender and send the opening packets */
   if(StartSender(rt))
      return(1);


   /* Take time here - use it or not */
   time(&time_started);
   time_stop = time_started + run_seconds;

   /* Send packets - handle sink messages */
   go = 1;
   while(go)
   {
      //DebugMessage("Sending data...");
      send(rt->sd, junkbuf, optimal_block, 0);
      //DebugMessage("Done.\n");

      /* NOTE: This is an oppurtunity to put a delay in the sender. It
               can be done here with the CheckForChildMessages() API.
               In cases where there is no sink, then that code will not
               run, so some other blocking mechanisim might be required
               to insert the (optional) delay. */
      if (pcomm)
      {
         /* A sink is present */
         if(CheckForChildMessages(pcomm->outof, 0, child_message) > 0)
            VerboseMessage("Sink: %s\n", child_message);
      }
      
      /* Now time becomes conditional */
      if ( run_seconds > 0 )
      {
         time(&time_now);

         if ( time_now >= time_stop )
            go = 0;
      }
   }

   DebugMessage("Shutting down the connection...");
   close(rt->sd); /* An actual shutdown() is not relevant. */
   DebugMessage("Done.\n");

   /* There should be a several second pause between shutting down
      outbound traffic and sending the stop to the sink. */

   DebugMessage("Sending stop to sink process");
   go = 3;
   while(go)
   {
      DebugMessage(".");
      sleep(1);
      go--;
   }
   SendStopSink(pcomm->into);
   DebugMessage("Done.\n");
      
   go = 10; 
   while ( go )
   {
      if(CheckForChildMessages(pcomm->outof, 500, child_message) > 0)
      {
         if ( child_message[0] == '.' )
         {
            VerboseMessage("Sink exited. Sender exiting now.\n");
            return(0);
         }
      }

      go--;
   }

   ErrorMessage("ERROR: Sink process never exited. Sender exiting.\n");
   return(1);
}

/* ========================================================================= */
int run_as_receiver(struct optbase *ob)
{
   char listen_address[MAX_VALUE_SIZE];
   char listen_protocol[MAX_VALUE_SIZE];
   char listen_port[MAX_VALUE_SIZE];
   char *local_listen;


   /* Do some basic validation of the expected values */
   if ( IsInvalidOption(ob, "MIRROR_PROTOCOL", IVO_EXISTS) )
   {
      /* NOTE: The mirror protocol is not optional. We do not run multiple
               listeners at this time. Either you are UDP or TCP, not both,
               not mixed (sender --> mirror (tcp); mirror --> sink (udp)). */

      ErrorMessage("ERROR: MIRROR_PROTOCOL option is missing or invalid.\n");
      return(1);
   }
   
   if ( IsInvalidOption(ob, "MIRROR_PORT", IVO_EXISTS) ) /* STUB: Check for numeric */
   {
      ErrorMessage("ERROR: MIRROR_PORT option is missing or invalid.\n");
      return(1);
   }

   if ( IsInvalidOption(ob, "MIRROR_ADDRESS", IVO_EXISTS) )
   {
      if ( GetStrValue(ob, listen_address, "MIRROR_ADDRESS", 3, MAX_VALUE_SIZE) )
      {
         ErrorMessage("ERROR: Problems parsing MIRROR_ADDRESS value.\n");
         return(1);
      }
      
      local_listen = listen_address;
   }
   else
      local_listen = NULL;

   if ( GetSTRValue(ob, listen_protocol, "MIRROR_PROTOCOL", 2, 4) )
   {
      ErrorMessage("ERROR: Problems parsing MIRROR_PROTOCOL value.\n");
      return(1);
   }
   
   if ( GetStrValue(ob, listen_port, "MIRROR_PORT", 2, 5) )
   {
      ErrorMessage("ERROR: Problems parsing MIRROR_PORT value.\n");
      return(1);
   }

   /* Register signal handlers */
   /*
   signal(SIGTERM, capture_signal);
   signal(SIGQUIT, capture_signal);
   signal(SIGINT, capture_signal);
   signal(SIGHUP, NULL);
   */

   RunAsReceiver(local_listen, listen_protocol, listen_port);

   return(0);
}

/* ========================================================================= */
void capture_signal(int signo)
{
   g_stop = 1;
}
