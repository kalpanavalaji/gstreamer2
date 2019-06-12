#include <gst/gst.h>
#include <glib.h>
static gboolean bus_call (GstBus  *bus , GstMessage *msg , gpointer  data)
{
  GMainLoop *loop = (GMainLoop *) data;
 
  switch (GST_MESSAGE_TYPE (msg)) 
 {

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
  GstCaps *filtercaps;//
  GstElement *filter = (GstElement *) data;

  /* We can now link this pad with the decoder-converter sink pad */
  g_print ("Dynamic pad created, linking decoder//converter\n");

  sinkpad = gst_element_get_static_pad (filter, "sink");
gst_pad_link (pad, sinkpad);

 
  
  gst_object_unref (sinkpad);
}

gint
main (gint   argc,
      gchar *argv[])
{
  GMainLoop *loop;
  GstElement *pipeline, *src, *sink, *filter, *csp,*decoder,*que;
  GstCaps *filtercaps;
  GstBus *bus;
  guint bus_watch_id;
 GstPad *sinkpad,*pad;
  /* init GStreamer */
  gst_init (&argc, &argv);
  loop = g_main_loop_new (NULL, FALSE);

  /* build */
  pipeline = gst_pipeline_new ("my-pipeline");
  src = gst_element_factory_make ("filesrc", "src");
  
  filter = gst_element_factory_make ("capsfilter", "filter");
  decoder  = gst_element_factory_make ("decodebin",     "decoder");
  csp = gst_element_factory_make ("autovideoconvert", "csp");
 
  que     = gst_element_factory_make ("queue", "que_ue");
  sink = gst_element_factory_make ("xvimagesink", "sink");
  
  g_object_set (G_OBJECT (src), "location", argv[1], NULL);
bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
   bus_watch_id = gst_bus_add_watch (bus, bus_call, loop);
   
  gst_bin_add_many (GST_BIN (pipeline), src, decoder,que,filter, csp, sink, NULL);
  if(gst_element_link (src, decoder)!= TRUE)
		g_printerr ("Elements 1 could not be linked.\n"); 

 if( gst_element_link_many (que, csp,filter, sink, NULL)!= TRUE)
		g_printerr ("Elements 2 could not be linked.\n"); 

   
 /* filtercaps = gst_caps_new_simple ("video/x-raw",
               "format", G_TYPE_STRING, "YUV",
               "width", G_TYPE_INT, 800,
               "height", G_TYPE_INT, 720,
               "framerate", GST_TYPE_FRACTION, 25, 1,
               NULL);*/
if (!gst_pad_set_caps (pad, filtercaps)) 
      GST_ELEMENT_ERROR (filtercaps, CORE, NEGOTIATION, (NULL),  ("Some debug information here"));
 g_assert(filtercaps!=NULL);
	//if( g_signal_connect (decoder, "pad-added", G_CALLBACK (on_pad_added), que)!= TRUE)
		//g_printerr ("Elements pads not be linked.\n");

// g_object_set (G_OBJECT (filter), "caps", filtercaps, NULL);
sinkpad = gst_element_get_static_pad (filter, "sink");
gst_pad_link (pad, sinkpad);

    //gst_caps_unref (filtercaps);
  /* run */
  gst_element_set_state (pipeline, GST_STATE_PLAYING);

  /* wait until it's up and running or failed */
  //if (gst_element_get_state (pipeline, NULL, NULL, -1) == GST_STATE_CHANGE_FAILURE) {
 //   g_error ("Failed to go into PLAYING state");
 // }

  g_print ("Running ...\n");
  g_main_loop_run (loop);
  gst_object_unref (bus);
  
  /* exit */
  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (pipeline);

  return 0;
}
