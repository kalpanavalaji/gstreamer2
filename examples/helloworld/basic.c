#include<gst/gst.h>
int main (int   argc,
      char *argv[])
{
  GstElement *player;
	GMainLoop *loop;

  /* init */
  gst_init (&argc, &argv);
  loop = g_main_loop_new (NULL, FALSE);/*add:context a GMainContext (if NULL, the default context will be used).[nullable] is_running set to TRUE to indicate that the loop is running. This is not very important since calling  g_main_loop_run() will set this to TRUE anyway.*/

  /* create player */
  player = gst_element_factory_make ("playbin", "player");

  /* set the source audio file */
  g_object_set (player, "uri", "https://www.freedesktop.org/software/gstreamer-sdk/data/media/sintel_trailer-480p.webm", NULL);

  /* start playback */
  gst_element_set_state (GST_ELEMENT (player), GST_STATE_PLAYING);
		g_main_loop_run (loop);//add
}

