#include <gst/gst.h>
#include <glib.h>



static gboolean bus_call (GstBus  *bus , GstMessage *msg , gpointer  data)
{
  GMainLoop *loop = (GMainLoop *) data;
 static int i=1;
  g_print("bus call is %d::\n",i++);
  switch (GST_MESSAGE_TYPE (msg)) {

    case GST_MESSAGE_EOS:
      g_print ("End of stream\n");
      g_main_loop_quit (loop);
      break;

    case GST_MESSAGE_ERROR: {
      gchar  *debug;
      GError *error;

      gst_message_parse_error (msg, &error, &debug);
      g_free (debug);

      g_printerr ("Error: %s\n", error->message);
      g_error_free (error);

      g_main_loop_quit (loop);
      break;
    }
    default:
      break;
  }

  return TRUE;
}


static void on_pad_added (GstElement *element,  GstPad  *pad, gpointer data)
{
  GstPad *sinkpad;
  GstElement *typefind = (GstElement *) data;

  /* We can now link this pad with the typefind-converter sink pad */
  g_print ("Dynamic pad created, linking typefind//converter\n");

  sinkpad = gst_element_get_static_pad (typefind, "sink");

  gst_pad_link (pad, sinkpad);

  gst_object_unref (sinkpad);
}



int main (int   argc,char *argv[])
{
  GMainLoop *loop;
 
  GstCaps *filtercaps;//
  GstElement *pipeline, *source, *typefind, *conv, *sink,*filter,*que;
  GstBus *bus;
  guint bus_watch_id;
  /* Initialisation */
  gst_init (&argc, &argv);
  
 loop = g_main_loop_new (NULL, FALSE);
/*context a GMainContext (if NULL, the default context will be used).[nullable] 
                                is_running set to TRUE to indicate that the loop is running. This is not very important since calling 					g_main_loop_run() will set this to TRUE anyway.*/
 
  /* Check input arguments */
  if (argc != 2) 
  {
    g_printerr ("Usage: %s <Ogg/Vorbis filename>\n", argv[0]);
    return -1;
  }


  /* Create gstreamer elements ,keep information about gstreamer element*/
  pipeline = gst_pipeline_new ("video-player");
  source   = gst_element_factory_make ("filesrc",       "file-source");
  typefind  = gst_element_factory_make ("typefind",     "typefinder");
  conv     = gst_element_factory_make ("videoconvert",  "converter");
  que     = gst_element_factory_make ("queue", "que_ue");
  sink     = gst_element_factory_make ("ximagesink", "NULL");

  if (!pipeline || !source || !typefind || !conv || !sink || !que) 
  {
    g_printerr ("One element could not be created. Exiting.\n");
    return -1;
  }


 
  g_object_set (G_OBJECT (source), "location", argv[1], NULL);
  
 
 

   	bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
   	bus_watch_id = gst_bus_add_watch (bus, bus_call, loop);
   	gst_object_unref (bus);
  

  
  	gst_bin_add_many (GST_BIN (pipeline),source, typefind, que , conv,  sink ,NULL);
 


  	/* file-source ->typefind ->demux->decoder-> converter -> video-output */
 	 gst_element_link (source, typefind);
  	gst_element_link_many (que,conv,sink, NULL);

  	g_signal_connect (typefind, "have-type", G_CALLBACK (on_pad_added), que);
 
  

	gst_caps_unref (filtercaps);

  
  	/* Set the pipeline to "playing" state*/
  	g_print ("Now playing: %s\n", argv[1]);
  	gst_element_set_state (pipeline, GST_STATE_PLAYING);


  /* Iterate */
  g_print ("Running...\n");
  g_main_loop_run (loop);
  

  /* Out of the main loop, clean up nicely */
  g_print ("Returned, stopping playback\n");
  gst_element_set_state (pipeline, GST_STATE_NULL);
  
  g_print ("Deleting pipeline\n");
  gst_object_unref (GST_OBJECT (pipeline));
  g_source_remove (bus_watch_id);
 g_main_loop_unref (loop);//Decreases the reference count on a GMainLoop object by one. If the result is zero, free the loop and free all associated memory.

  return 0;
}


