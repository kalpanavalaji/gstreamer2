#include <gst/gst.h>

  GstElement *pipeline,*decode, *filesrc, *typefind, *fakesink,*Vconv,*demux,*Afakesink,*Aconv,*queue;
	GstBus *bus;
/*static void on_pad_added (GstPadTemplate * templ, GstPad * newpad, gpointer user_data)
{
  GstPad *target = GST_PAD (user_data);

  gst_pad_link (newpad, target);
  gst_object_unref (target);
}*/
static void on_pad_added (GstPadTemplate * templ,  GstPad  *pad, GstElement *decoder)
{
 
// GstElement *decoder = (GstElement *) data;


  g_print ("Dynamic pad created, linking decoder//converter\n");

 GstPad *sinkpad;
	sinkpad = gst_element_get_static_pad (decoder, "sink");
  
gint ret;
 if(!(ret=gst_pad_link (pad, sinkpad)))
			g_print("pad link error::::::::::ret:%d\n",ret);

  gst_object_unref (sinkpad);
}


static void message_received (GstBus * bus, GstMessage * message, GstPipeline * pipeline)
{
  const GstStructure *s;

  s = gst_message_get_structure (message);
  g_print ("message from \"%s\" (%s): ",
      GST_STR_NULL (GST_ELEMENT_NAME (GST_MESSAGE_SRC (message))),
      gst_message_type_get_name (GST_MESSAGE_TYPE (message)));
  if (s) {
    gchar *sstr;

    sstr = gst_structure_to_string (s);
    g_print ("%s\n", sstr);
    g_free (sstr);
  } else {
    g_print ("no message details\n");
  }
}

static void eos_message_received (GstBus * bus, GstMessage * message,gpointer  loop)
{

  message_received (bus, message,(GstPipeline *) pipeline);
  gtk_main_quit ((GMainLoop *)loop);
}

static gboolean idle_exit_loop (gpointer data)
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
/*	if(!strcmp("video/webm",type))//webm
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
  			fakesink = gst_element_factory_make ("ximagesink", "sink");
 			queue	= gst_element_factory_make ("queue",  "vqueue");

 			gst_bin_add_many (GST_BIN (pipeline), demux,queue,decode ,Vconv, fakesink,NULL);

  			if(!gst_element_link( typefind, demux))
					g_print("linking error2\n");
			 if(!gst_element_link_many (queue,decode,Vconv,fakesink,NULL))
					g_print("linking error1\n");	

  g_signal_connect (G_OBJECT (demux), "pad-added", G_CALLBACK (on_pad_added), queue);
			//if(!g_signal_connect (G_OBJECT (demux), "pad-added",G_CALLBACK (on_pad_added), gst_element_get_static_pad (decode, "sink")))
				//	g_print("Signal errror\n");
		
		
///////
 gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PLAYING);*/

  g_free (type);
 //idle_exit_loop (loop);
 
}
//pad added

gint main (gint argc,gchar *argv[])
{
	GMainLoop *loop;



  gst_init (&argc, &argv);
  loop = g_main_loop_new (NULL, FALSE);


  if (argc != 2) {
    g_print ("Usage: %s <filename>\n", argv[0]);
    return -1;
  }


  pipeline = gst_pipeline_new ("pipe");

  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  gst_bus_add_signal_watch_full (bus, G_PRIORITY_HIGH);
  g_signal_connect (bus, "message::error", (GCallback) message_received,
      pipeline);
  g_signal_connect (bus, "message::warning", (GCallback) message_received,
      pipeline);
  g_signal_connect (bus, "message::eos", (GCallback) eos_message_received,
      loop);


  filesrc = gst_element_factory_make ("filesrc", "source");//matroskademux

  g_object_set (G_OBJECT (filesrc), "location", argv[1], NULL);
  typefind = gst_element_factory_make ("typefind", "typefinder");
  g_signal_connect (typefind, "have-type", G_CALLBACK (cb_typefound), loop);
  


  gst_bin_add_many (GST_BIN (pipeline), filesrc,typefind,NULL);
  gst_element_link (filesrc, typefind);
  

  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PLAYING);

  g_main_loop_run (loop);


  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_NULL);
  gst_object_unref (GST_OBJECT (pipeline));
	g_main_loop_unref (loop);
  return 0;
}

