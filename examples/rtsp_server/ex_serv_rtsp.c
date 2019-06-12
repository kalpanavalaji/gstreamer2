#include <gst/gst.h>  
#include <gst/rtsp-server/rtsp-server.h>
#include <gst/rtsp-server/rtsp-session-pool.h> 

static gboolean timeout (GstRTSPServer * server)
{
	GstRTSPSessionPool *pool;
 
   	pool = gst_rtsp_server_get_session_pool (server);
    gst_rtsp_session_pool_cleanup (pool);
    g_object_unref (pool);
 
    return TRUE;
}
 
 
int main (int argc, char *argv[])
{
    GMainLoop *loop;
    GstRTSPServer *server;
    GstRTSPMountPoints *mounts;
    //GstRTSPMediaFactory *factory;
    GstRTSPMediaFactory *factory1;
    GstRTSPSessionPool *session;
     
    gst_init (&argc, &argv);

   	loop = g_main_loop_new (NULL, FALSE);
   
    session = gst_rtsp_session_pool_new();
    gst_rtsp_session_pool_set_max_sessions  (session, 255);
    /*  */
    server = gst_rtsp_server_new ();
 

    mounts = gst_rtsp_server_get_mount_points (server);
 

    //factory = gst_rtsp_media_factory_new ();
    factory1 = gst_rtsp_media_factory_new ();
    //gst_rtsp_media_factory_set_launch (factory,
    //   "( videotestsrc is-live=1 ! x264enc ! rtph264pay name=pay0 pt=96 )");
  
    gst_rtsp_media_factory_set_launch (factory1,
		"( "  
       	"filesrc location=/home/nvidia/Videos/iptest.mp4 ! qtdemux name=d "
       	"d. ! queue ! rtph264pay pt=96 name=pay0 "
       	"d. ! queue ! rtpmp4apay pt=97 name=pay1 " ")");
   
   	//gst_rtsp_media_factory_set_shared (factory, TRUE);
   	gst_rtsp_media_factory_set_shared (factory1, TRUE);


   	//gst_rtsp_mount_points_add_factory (mounts, "/test", factory);
    gst_rtsp_mount_points_add_factory (mounts, "/test1", factory1);
   
   	g_object_unref (mounts);
 
   	gst_rtsp_server_attach (server, NULL); 
     
   	g_timeout_add_seconds (2, (GSourceFunc) timeout, server);
 
   	//g_print ("stream ready at rtsp://127.0.0.1:8554/test\n");
   	g_print ("stream ready at rtsp://127.0.0.1:8554/test1\n");
   	g_main_loop_run (loop);
 
   	return 0;
}


