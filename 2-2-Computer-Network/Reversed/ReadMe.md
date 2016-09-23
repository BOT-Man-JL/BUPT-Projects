#Motivation
Since the original Bupt Computer Network experiment simulator is a bit nasty
(in which the library calls <code>exit (0);</code> to kill the caller :-( ), <br>
I suppose that I need to do something to make it easier to debug our programe.<br><br>

This code is reversely extracted from the orignal code (written by jiangy@public3.bta.net.cn, without any copyright/copyleft 😂)<br>

#How to Use
Just add "event.c, Impl.c, Impl.h, Init.c, log.c, network.c, physical.c, protocol.h, timer.c" to your current project, 
and everything will work in the same way as using dll/lib.

#Code Layout
##Impl.h/Impl.c
Store the shared global constaints and variables.
##Init.c
Impl function <code>protocol_init ()</code>
##event.c
Impl function <code>wait_for_event ()</code>
##network.c
Impl function <code>enable_network_layer ()</code>, <code>disable_network_layer ()</code>, 
<code>get_packe ()t</code>, <code>put_packet ()</code>
##physical.c
Impl function <code>recv_frame ()</code>, <code>send_frame ()</code>, <code>phl_sq_len ()</code>, <code>crc32 ()</code>
##timer.c
Impl function <code>get_ms ()</code>, <code>start_timer ()</code>, <code>stop_timer ()</code>, 
<code>start_ack_timer ()</code>, <code>stop_ack_timer ()</code>
##log.c
Impl function <code>log_printf ()</code>, <code>lprintf ()</code>, <code>station_name ()</code>, <code>dbg_* ()</code>

#By the way
Just for fun. No Offense. Enjoy it :)
