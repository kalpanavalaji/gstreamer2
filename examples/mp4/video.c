#include<gst/gst.h>
#include <glib.h>

static gboolean bus_call (GstBus  *bus , GstMessage *msg , gpointer  data)
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



static void on_pad_added (GstElement *element,  GstPad  *pad, gpointer data)
{
	 GstPad *sinkpad;
  	 GstElement *decoder = (GstElement *) data;
	 g_print ("Dynamic pad created, linking demuxer//decoder\n");
	 
	 sinkpad = gst_element_get_static_pad (decoder, "sink");
	
	 gst_pad_link(pad,sinkpad);
	
	 gst_object_unref(sinkpad);
}




int main(int argc ,char *argv[])
{
	GstElement *pipeline,*source,*demux,*decoder,*converter,*sink;
	
	GMainLoop *loop;
	GstBus *bus;
	guint bus_watch_id;


	gst_init (&argc, &argv);//inisialize the gstreamer libraries
        loop = g_main_loop_new (NULL, FALSE);
	if (argc != 2) 
   	{
   	  	g_printerr ("Usage: %s <mp4: filename>\n", argv[0]);
   	  	return -1;
   	}
	//create pipeline	
	pipeline = gst_pipeline_new("Video-player");
	//create a type of elements 
	//we need our own element by using type of element typedefined by the given element factory.
	source = gst_element_factory_make("filesrc","file_src");
	demux  = gst_element_factory_make("qtdemux","qt_demux");
	decoder = gst_element_factory_make("avdec_h264","av_dec_h264");
	converter =gst_element_factory_make("autovideoconvert","video_converter");
	sink = gst_element_factory_make("autovideosink","video_sink");

	//check if elements are created or not
	g_assert("source");
	g_assert("demux");
	g_assert("decoder");
	g_assert("converter");
	g_assert("sink");

	/* we set the input filename to the source element  */
	//using source element property
	g_object_set (G_OBJECT (source), "location", argv[1], NULL);
	

	bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
        bus_watch_id = gst_bus_add_watch (bus, bus_call, loop);
        gst_object_unref (bus);
a	//add all elements to bin(pipeline)
	 gst_bin_add_many (GST_BIN (pipeline),source,demux, decoder, converter, sink, NULL);
	

	//link the elements inside the bin
	
	gst_element_link(source,demux);
	gst_element_link_many(decoder,converter,sink,NULL);//we can add number of elements upto to we reach NULL (internally it calls 									gst_element_link)
	
	g_signal_connect(demux, "pad-added", G_CALLBACK (on_pad_added), decoder);//dynamically created a src pad in demux then ,linking with decoder.
	 /* Set the pipeline to "playing" state*/
 	 g_print ("Now playing: %s\n", argv[1]);

  	 gst_element_set_state (pipeline, GST_STATE_PLAYING);
		/* You can only move between adjacent ones, this is, you can't go from NULL to PLAYING, you have to go through the 			intermediate READY and PAUSED states. If you set the pipeline to PLAYING, though, GStreamer will make the intermediate 			transitions for you.*/
	
	
	/* Iterate */
	 g_print ("Running...\n");
	 g_main_loop_run (loop);
  	//Runs a main loop until g_main_loop_quit() is called on the loop. If this is called for the thread of the loop's GMainContext, it 		 will process events from the loop, otherwise it will simply wait.
	
	
  	/* Out of the main loop, clean up nicely */
  	g_print ("Returned, stopping playback\n");
  	gst_element_set_state (pipeline, GST_STATE_NULL);
  	
	g_print("Deleting the Pipeline\n");
	gst_object_unref(GST_OBJECT(pipeline));
	
	return 0;
}
	

	


