#include <gst/gst.h>
  GstElement *pipeline, *filesrc, *typefind, *Vfakesink,*Afakesink,*Vdecoder,*Adecoder,*demux,*videoqueue,*audioqueue;
static gboolean my_bus_callback (GstBus  *bus , GstMessage *msg , gpointer  data)
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

static gboolean
idle_exit_loop (gpointer data)
{
  g_main_loop_quit ((GMainLoop *) data);

  /* once */
  return FALSE;
}


static void on_pad_added (GstElement *element,  GstPad  *pad, gpointer *data)
{
 
 	 GstPad *sinkpad;
  	 GstCaps *caps; 
  	 GstStructure *str; 
  	 const gchar *new_pad_type = NULL;
 // 	 g_print ("\t\t\t Starting link (type '%s').\n\n", new_pad_type );
  	 caps =  gst_pad_get_current_caps (pad); //get current capbalities of demuxer pads
 
  	 str = gst_caps_get_structure (caps, 0); 
  
  	 new_pad_type = gst_structure_get_name (str);
  	
	 if (g_strrstr (gst_structure_get_name (str), "audio"))
	 { 
		g_print ("\t\t\t Link succeeded (type '%s').\n", new_pad_type );
		sinkpad = gst_element_get_static_pad(audioqueue, "sink"); 
		gst_pad_link (pad, sinkpad);
		g_print ("\t\t\tlinking decoder//Aconverter\n");
		gst_object_unref (sinkpad);
		
		
	 }
	 else if(g_strrstr(gst_structure_get_name (str),"video"))
	 {
		g_print ("\t\t\tLink succeeded (type '%s').\n", new_pad_type );
		sinkpad = gst_element_get_static_pad(videoqueue,"sink");
		gst_pad_link(pad,sinkpad);
		g_print ("\t\t\tlinking decoder//Vconverter\n");
		gst_object_unref (sinkpad);
	 }	
	
 	 gst_caps_unref (caps);
}
/*webm:
gst-launch-1.0 filesrc location=small.webm ! matroskademux name=demux demux. ! queue ! vorbisdec ! audioconvert ! audioresample ! autoaudiosink demux. ! queue ! vp8dec ! videoconvert ! autovideosink*/
static void cb_typefound (GstElement *typefind, guint  probability, GstCaps    *caps, gpointer    data)
{
 	 GMainLoop *loop = data;
 	 gchar *type;
	 GstPad *pad;

 	 type = gst_caps_to_string (caps);

 	 g_print ("Media type %s found, probability %d%%\n", type, probability);
	 g_free (type);

		pad = gst_element_get_static_pad (typefind, "src");

			  demux = gst_element_factory_make ("matroskademux", "matroskademux1");
			  Adecoder = gst_element_factory_make ("vorbisdec", "vorbisdec1");
			  Vdecoder = gst_element_factory_make ("vp8dec", "vp8dec1");
		
		  g_signal_connect (demux, "pad-added", G_CALLBACK(on_pad_added),NULL);//link pads


  
  //g_idle_add (idle_exit_loop, loop);
}

////////////main
gint main (gint   argc,gchar *argv[])
{
  GMainLoop *loop;

  GstBus *bus;

  /* init GStreamer */
  gst_init (&argc, &argv);
  loop = g_main_loop_new (NULL, FALSE);

  /* check args */
  if (argc != 2) {
    g_print ("Usage: %s <filename>\n", argv[0]);
    return -1;
  }

  /* create a new pipeline to hold the elements */
  pipeline = gst_pipeline_new ("pipe");

  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  gst_bus_add_watch (bus, my_bus_callback, NULL);
  gst_object_unref (bus);

  /* create file source and typefind element */
  filesrc = gst_element_factory_make ("filesrc", "source");
  g_object_set (G_OBJECT (filesrc), "location", argv[1], NULL);
  typefind = gst_element_factory_make ("typefind", "typefinder");
  g_signal_connect (typefind, "have-type", G_CALLBACK (cb_typefound), loop);
  Vfakesink = gst_element_factory_make ("xvideosink", "sink");
  Afakesink = gst_element_factory_make ("autovideosink", "sink");
  audioqueue = gst_element_factory_make ("audioqueue", "queue");
  videoqueue = gst_element_factory_make ("videoqueue", "vqueue");
  /* setup */
  gst_bin_add_many (GST_BIN (pipeline), filesrc, typefind,demux,Adecoder,Vdecoder,audioqueue,videoqueue, Afakesink, Vfakesink,NULL);
  gst_element_link_many (filesrc, typefind, NULL);
  gst_element_link_many (videoqueue,Vdecoder,Vfakesink, NULL);
  gst_element_link_many (audioqueue,Adecoder,Afakesink, NULL);



  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PLAYING);
  g_main_loop_run (loop);

  /* unset */
  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_NULL);
  gst_object_unref (GST_OBJECT (pipeline));

  return 0;
}

