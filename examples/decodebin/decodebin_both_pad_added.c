#include <gst/gst.h>
#include <glib.h>
#include <gtk/gtk.h>
GstElement *audioqueue,*videoqueue;

static gboolean bus_call (GstBus *bus, GstMessage *msg, gpointer data)
{
  GMainLoop *loop = (GMainLoop *) data;

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
  GstCaps *caps; 
  GstStructure *str; 
  const gchar *new_pad_type = NULL;
  g_print (" Link succeeded (type '%s').\n", new_pad_type );
  caps =  gst_pad_get_current_caps (pad); //get current capbalities of demuxer pads
 
  str = gst_caps_get_structure (caps, 0); 
  
  new_pad_type = gst_structure_get_name (str);
  g_print (" Link succeeded (type '%s').\n", new_pad_type );
  /* We can now link this pad with the decoder-converter sink pad */
  g_print ("Dynamic pad created, linking decoder//converter\n");
	 if (g_strrstr (gst_structure_get_name (str), "audio"))
	 { 
		g_print (" Link succeeded (type '%s').\n", new_pad_type );
		sinkpad = gst_element_get_static_pad(audioqueue, "sink"); 
		gst_pad_link (pad, sinkpad);
		gst_object_unref (sinkpad);
		
	 }
	 else if(g_strrstr(gst_structure_get_name (str),"video"))
	 {
		g_print (" Link succeeded (type '%s').\n", new_pad_type );
		sinkpad = gst_element_get_static_pad(videoqueue,"sink");
		gst_pad_link(pad,sinkpad);
		gst_object_unref (sinkpad);
	}	
 	 gst_caps_unref (caps);
}



int main (int   argc,char *argv[])
{
	int a=0;
  GMainLoop *loop;

  GstElement *pipeline, *source, *decodebinn, *Vsink,*Are_sa,*Asink,*Vconv,*Aconv;
  GstBus *bus;
  guint bus_watch_id;

  /* Initialisation */
  gst_init (&argc, &argv);

  loop = g_main_loop_new (NULL, FALSE);/*context a GMainContext (if NULL, the default context will be used).[nullable] 
                                is_running set to TRUE to indicate that the loop is running. This is not very important since calling 					g_main_loop_run() will set this to TRUE anyway.*/


  /* Check input arguments */
  if (argc != 2) 
  {
    g_printerr ("Usage: %s  filename>\n", argv[0]);
    return -1;
  }


  /* Create gstreamer elements */
  pipeline = gst_pipeline_new ("video-audio-player");
  source   = gst_element_factory_make ("filesrc",       "file-source");
  
  decodebinn  = gst_element_factory_make ("decodebin",     "decoderr");
  audioqueue = gst_element_factory_make("queue","audioqueue");
  videoqueue = gst_element_factory_make("queue","videoqueue");
  Vconv     = gst_element_factory_make ("videoconvert",  "Vconverter");
  Vsink     = gst_element_factory_make ("autovideosink", "Video-output");


  Aconv     = gst_element_factory_make ("audioconvert",  "Aconverter");
  Are_sa    = gst_element_factory_make ("audioresample", "Are_sample");
  Asink     = gst_element_factory_make ("autoaudiosink", "Audio-output");


  if (!pipeline || !source || !decodebinn || !Vconv || !Vsink || !Are_sa || !Aconv || !Asink || !audioqueue || !videoqueue) 
  {
    g_printerr ("One element could not be created. Exiting.\n");
    return -1;
  }

  /* Set up the pipeline */

  /* we set the input filename to the source element */
  g_object_set (G_OBJECT (source), "location", argv[1], NULL);

  /* we add a message handler */
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  bus_watch_id = gst_bus_add_watch (bus, bus_call, loop);
  gst_object_unref (bus);//

  /* we add all elements into the pipeline */
  //file-source | decoder | 
  gst_bin_add_many (GST_BIN (pipeline),source, decodebinn,videoqueue, Vconv, Vsink,audioqueue , Aconv , Asink, Are_sa, NULL);

  /* we link the elements together */
  /* file-source ->decoder -> converter -> video-output */
  gst_element_link (source, decodebinn);
  gst_element_link_many (videoqueue,Vconv, Vsink, NULL);
  gst_element_link_many (audioqueue, Aconv,Are_sa, Asink, NULL);
  g_signal_connect (decodebinn, "pad-added", G_CALLBACK(on_pad_added), NULL);
  /* note that the decoder will be linked to the converter dynamically.
     The source pad(s) will be created at run time,by the decoder when it detects the amount and nature of streams.
     Therefore we connect a callback function which will be executed
      when the "pad-added" is emitted.*/


  /* Set the pipeline to "playing" state*/
  g_print ("Now playing: %s\n", argv[1]);
  gst_element_set_state (pipeline, GST_STATE_PLAYING);


  /* Iterate */
  g_print ("Running...\n");
  g_main_loop_run (loop);
  g_printf("hiii\n");
//Runs a main loop until g_main_loop_quit() is called on the loop. If this is called for the thread of the loop's GMainContext, it will process events from the loop, otherwise it will simply wait.


  /* Out of the main loop, clean up nicely */
  g_print ("Returned, stopping playback\n");
  gst_element_set_state (pipeline, GST_STATE_NULL);

  g_print ("Deleting pipeline\n");
  gst_object_unref (GST_OBJECT (pipeline));
  g_source_remove (bus_watch_id);
  g_main_loop_unref (loop);//Decreases the reference count on a GMainLoop object by one. If the result is zero, free the loop and free all associated memory.

  return 0;
}


