#include<gst/gst.h>
#include <glib.h>

typedef struct _CustomData {
    GstElement *pipeline;
    GstElement *source;
    GstElement *demuxer;
    GstElement *audioqueue;
    GstElement *videoqueue;
    GstElement *audio_decoder;
    GstElement *video_decoder;
    GstElement *video_convert;
    GstElement *audio_convert;
    GstElement *video_sink;
    GstElement *audio_sink;
    GstElement *Are_sam;
    } CustomData;


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
static void on_pad_added (GstElement *element,  GstPad  *pad, CustomData *data)
{
 
 	 GstPad *sinkpad;
  	 GstCaps *caps; 
  	 GstStructure *str; 
  	 const gchar *new_pad_type = NULL;
  	 g_print (" Starting link (type '%s').\n", new_pad_type );
  	 caps =  gst_pad_get_current_caps (pad); //get current capbalities of demuxer pads
 
  	 str = gst_caps_get_structure (caps, 0); 
  
  	 new_pad_type = gst_structure_get_name (str);
  	//g_print (" Dynamic pad created  (type '%s').\n", new_pad_type );
  		// We can now link this pad with the decoder-converter sink pad 
  	
	 if (g_strrstr (gst_structure_get_name (str), "audio/mpeg"))
	 { 
		g_print (" Link succeeded (type '%s').\n", new_pad_type );
		sinkpad = gst_element_get_static_pad(data->audioqueue, "sink"); 
		gst_pad_link (pad, sinkpad);
		g_print ("linking decoder//Aconverter\n");
		gst_object_unref (sinkpad);
		
		
	 }
	 else if(g_strrstr(gst_structure_get_name (str),"video/x-h264"))
	 {
		g_print (" Link succeeded (type '%s').\n", new_pad_type );
		sinkpad = gst_element_get_static_pad(data->videoqueue,"sink");
		gst_pad_link(pad,sinkpad);
		g_print ("linking decoder//Vconverter\n");
		gst_object_unref (sinkpad);
	 }	
	
 	 gst_caps_unref (caps);
}






int main(int argc ,char *argv[])
{
	
	
	guint bus_watch_id;
	GMainLoop *loop;
	GstBus *bus;
	GstMessage *msg;
	CustomData data;

	gst_init (&argc, &argv);//inisialize the gstreamer libraries
        loop = g_main_loop_new (NULL, FALSE);
	if (argc != 2) 
   	{
   	  	g_printerr ("Usage: %s <mp4: filename>\n", argv[0]);
   	  	return -1;
   	}
	//create pipeline	
	data.pipeline = gst_pipeline_new ("test-mp4-pipeline");
	//create a type of elements 
	//we need our own element by using type of element typedefined by the given element factory.
	  data.source = gst_element_factory_make ("filesrc", "source");
    	  data.demuxer = gst_element_factory_make ("qtdemux", "demuxer");
          data.audioqueue = gst_element_factory_make("queue","audioqueue");
   	  data.videoqueue = gst_element_factory_make("queue","videoqueue");
    	  data.audio_decoder = gst_element_factory_make ("aacparse", "audio_decoder");
    	  data.audio_convert = gst_element_factory_make ("audioconvert", "audio_convert");
     	  data.audio_sink = gst_element_factory_make ("autoaudiosink", "audio_sink");
    	  data.video_decoder = gst_element_factory_make("avdec_h264","video_decoder");
    	  data.video_convert = gst_element_factory_make("autovideoconvert","video_convert");
    	  data.video_sink = gst_element_factory_make("autovideosink","video_sink");
	  data.Are_sam =gst_element_factory_make("audioresample","audio_resample");
	//check if elements are created or not
	
    if (!data.pipeline || !data.source || !data.demuxer || !data.audioqueue ||!data.audio_decoder ||!data.audio_convert ||
    !data.audio_sink || !data.videoqueue || !data.video_decoder || !data.video_convert || !data.video_sink) {
    g_printerr ("Not all elements could be created.\n");
    return -1;
    }
	/* we set the input filename to the source element  */
	// using source element property
	g_object_set (G_OBJECT (data.source), "location", argv[1], NULL);
	

	bus = gst_pipeline_get_bus (GST_PIPELINE (data.pipeline));//if this 2 apis not used,then is won't come out of the loop.
        bus_watch_id = gst_bus_add_watch (bus, bus_call, loop);
        gst_object_unref (bus);

	//add all elements to bin(pipeline)
	gst_bin_add_many (GST_BIN(data.pipeline),data.source,data.demuxer,data.audioqueue,data.audio_decoder,data.audio_convert,data.audio_sink
,data.Are_sam,data.videoqueue,data.video_decoder,data.video_convert,data.video_sink, NULL);
	

	//link the elements inside the bin
	
	gst_element_link(data.source,data.demuxer);
	gst_element_link_many (data.audioqueue,data.audio_decoder,data.Are_sam,data.audio_convert, data.audio_sink,NULL);//we can add number of elements upto to we reach NULL (internally it calls 									gst_element_link)
	gst_element_link_many(data.videoqueue,data.video_decoder,data.video_convert, data.video_sink,NULL);
   		 
	g_signal_connect(data.demuxer, "pad-added", G_CALLBACK (on_pad_added), &data);
	
	
	//dynamically created a src pad in demux then ,linking with queue.
	
	 /* Set the pipeline to "playing" state*/
 	 g_print ("Now playing: %s\n", argv[1]);

  	 gst_element_set_state (data.pipeline, GST_STATE_PLAYING);
		/* You can only move between adjacent ones, this is, you can't go from NULL to PLAYING, you have to go through the 			intermediate READY and PAUSED states. If you set the pipeline to PLAYING, though, GStreamer will make the intermediate 			transitions for you.*/
	
	
	/* Iterate */
	 g_print ("Running...\n");
	 g_main_loop_run (loop);
  	//Runs a main loop until g_main_loop_quit() is called on the loop. If this is called for the thread of the loop's GMainContext, it 		 will process events from the loop, otherwise it will simply wait.
	
	
  	/* Out of the main loop, clean up nicely */
  	g_print ("Returned, stopping playback\n");
  	gst_element_set_state (data.pipeline, GST_STATE_NULL);
  	
	g_print("Deleting the Pipeline\n");
	gst_object_unref(GST_OBJECT(data.pipeline));
	
	return 0;
}
	

	


