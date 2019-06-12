#include <gst/gst.h>

  GstElement *pipeline,*decode, *filesrc, *typefind, *fakesink,*Vconv,*demux,*Afakesink,*Aconv;

static void on_pad_added (GstPadTemplate * templ, GstPad * newpad, gpointer user_data)
{
  GstPad *target = GST_PAD (user_data);//Pads are created from Pad Templates

  gst_pad_link (newpad, target);
  gst_object_unref (target);
}

static gboolean  my_bus_callback (GstBus  *bus , GstMessage *msg , gpointer  data)
{
  GMainLoop *loop = (GMainLoop *) data;

  switch (GST_MESSAGE_TYPE (msg)) {

    case GST_MESSAGE_EOS:{
      g_print ("End of stream\n");
      g_main_loop_quit (loop);
      break;
	}
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

static void cb_typefound (GstElement *typefind,guint probability,GstCaps *caps, gpointer data)
{

 GMainLoop *loop = data;
  gchar *type;
  type = gst_caps_to_string (caps);
  g_print ("Media type %s found, probability %d%%\n", type, probability);
  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PAUSED);
	if(!strcmp("video/webm",type))//webm
	{
				demux=  gst_element_factory_make ("matroskademux", "demuxeer");
		  		decode =  gst_element_factory_make ("vp9dec", "decoderbin");
		
	}
	else if(!strcmp("video/x-matroska",type))//webm
	{
				demux=  gst_element_factory_make ("matroskademux", "demuxeer");
		  		decode =  gst_element_factory_make ("vp8dec", "decoderbin");

	}
	else if(!strncmp("application/x-3gp",type,17))//3gp
	{
				demux=  gst_element_factory_make ("qtdemux", "demuxeer");
		  		decode =  gst_element_factory_make ("avdec_h263", "decoderbin");

	}
	else if(!strncmp("video/quicktime",type))//mov
	{
				demux=  gst_element_factory_make ("qtdemux", "demuxeer");
		  		decode =  gst_element_factory_make ("avdec_h264", "decoderbin");

	}
	else 
			g_print("type not found\n");

			Vconv     = gst_element_factory_make ("videoconvert",  "Vconverter");
  			fakesink = gst_element_factory_make ("autovideosink", "sink");
 			gst_bin_add_many (GST_BIN (pipeline), demux,decode ,Vconv, fakesink,NULL);

  			if(!gst_element_link( typefind, demux))
					g_print("linking error\n");

 			 if(!gst_element_link_many (decode,Vconv,fakesink,NULL))
					g_print("linking error\n");	
			if(!g_signal_connect (G_OBJECT (demux), "pad-added",G_CALLBACK (on_pad_added), gst_element_get_static_pad (decode, "sink")))
					g_print("Signal errror\n");
		
		
///////
 gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PLAYING);

  g_free (type);

 
}
//pad added

gint main (gint   argc,gchar *argv[])
{
	GMainLoop *loop;
	GstBus *bus;


  gst_init (&argc, &argv);
  loop = g_main_loop_new (NULL, FALSE);


  if (argc != 2) {
    g_print ("Usage: %s <filename>\n", argv[0]);
    return -1;
  }


  pipeline = gst_pipeline_new ("pipe");

  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  gst_bus_add_watch (bus, my_bus_callback, NULL);
  gst_object_unref (bus);


  filesrc = gst_element_factory_make ("filesrc", "source");//matroskademux

  g_object_set (G_OBJECT (filesrc), "location", argv[1], NULL);
  typefind = gst_element_factory_make ("typefind", "typefinder");

 /* demux=  gst_element_factory_make ("matroskademux", "demuxeer");
  decode=  gst_element_factory_make ("vp9dec", "decoderbin");
  

   Vconv     = gst_element_factory_make ("videoconvert",  "Vconverter");
  fakesink = gst_element_factory_make ("autovideosink", "sink");*/


  gst_bin_add_many (GST_BIN (pipeline), filesrc,typefind,NULL);
  gst_element_link (filesrc, typefind);
  g_signal_connect (typefind, "have-type", G_CALLBACK (cb_typefound), loop);


  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PLAYING);

  g_main_loop_run (loop);


  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_NULL);
  gst_object_unref (GST_OBJECT (pipeline));

  return 0;
}

