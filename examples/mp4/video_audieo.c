#include<gst/gst.h>
#include <glib.h>

GstElement *pipeline,*source,*demux,*Adecoder,*Aconverter,*Asink,*Vdecoder,*Vconverter,*Vsink,*audioqueue,*videoqueue,*Are_sam;
//type_find fun
static void cb_typefound (GstElement *typefinder, guint  probability, GstCaps *caps, gpointer data)
{

	g_print("Hello......................\n");
 	 GMainLoop *loop =  (GMainLoop *)data;
 	 gchar *type;
	 GstPad *pad;
	 g_print ("Media type %s found, probability %d%%\n", type, probability);
 	 type = gst_caps_to_string (caps);


	 g_free (type);  

}


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



void on_pad_added (GstElement *element, GstPad *pad) 
{ 
        g_debug ("Signal: pad-added"); 
        GstCaps *caps; 
        GstStructure *str; 
	GstPad *targetsink=NULL; 
	const gchar *new_pad_type = NULL;
        caps =  gst_pad_get_current_caps (pad); 
        g_assert (caps != NULL); 
        str = gst_caps_get_structure (caps, 0); 
        g_assert (str != NULL); 
	
                // Link it actually 
		g_print (" before (type '%s').\n", new_pad_type );
	new_pad_type = gst_structure_get_name (str);
                // Link it actually 
		g_print ("after  (type '%s').\n", new_pad_type );
	 if (g_strrstr (gst_structure_get_name (str), "audio/mpeg")) { 
                g_debug ("Linking audio pad to dec_ad"); 
                // Link it actually 
		g_print (" Link succeeded (type '%s').\n", new_pad_type );
                targetsink = gst_element_get_static_pad(audioqueue, "sink"); 
		g_print ("Dynamic pad created, linking demuxer//audioqueue\n");
                g_assert (targetsink != NULL); 
               
        	} 
        if (g_strrstr (gst_structure_get_name (str), "video/x-h264")) { 
                g_debug ("Linking video pad to dec_vd"); 
                // Link it actually 
		g_print (" Link succeeded (type '%s').\n", new_pad_type );
                targetsink = gst_element_get_static_pad(videoqueue, "sink"); 
		g_print ("Dynamic pad created, linking demuxer//videoqueue\n");
                g_assert (targetsink != NULL); 
              
        } 
	 if (!gst_pad_is_linked(targetsink))
	 {
	  		gst_pad_link (pad, targetsink); 
   	 }

       
	  gst_pad_link (pad, targetsink); 
          gst_object_unref (targetsink); 

        gst_caps_unref (caps); 
} 



int main(int argc ,char *argv[])
{
	
	
	
	GMainLoop *loop;
	GstBus *bus;
	guint bus_watch_id;
	
	        gst_init (&argc, &argv);//inisialize the gstreamer libraries

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

	Vdecoder = gst_element_factory_make("avdec_h264","av_dec_h264");
	Vconverter =gst_element_factory_make("autovideoconvert","video_converter");
	Vsink = gst_element_factory_make("autovideosink","video_sink");
	
	Adecoder = gst_element_factory_make("faad","av_dec_aac");//avdec_aac
	Aconverter =gst_element_factory_make("autoaudioconvert","audio_converter");
	Are_sam =gst_element_factory_make("audioresample","audio_resample");
	Asink = gst_element_factory_make("autoaudiosink","audio_sink");
	
	audioqueue = gst_element_factory_make("queue","audio_queue");
        videoqueue = gst_element_factory_make("queue","video_queue");





	//check if elements are created or not
	g_assert("source");
	g_assert("demux");
	g_assert("Vdecoder");
	g_assert("Vconverter");
	g_assert("Vsink");
	g_assert("Adecoder");
	g_assert("Aconverter");
	g_assert("Asink");
	g_assert("Are_sam");

	/* we set the input filename to the source element  */
	// using source element property
	g_object_set (G_OBJECT (source), "location", argv[1], NULL);
	

	bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));//if this 2 apis not used,then is won't come out of the loop.
        bus_watch_id = gst_bus_add_watch (bus, bus_call, loop);
        gst_object_unref (bus);

	//add all elements to bin(pipeline)
	 gst_bin_add_many (GST_BIN (pipeline),source,demux,audioqueue,Vdecoder, Vconverter, Vsink, videoqueue , Adecoder, Aconverter, Are_sam , Asink,NULL);
	

	//link the elements inside the bin
	
	gst_element_link(source,demux);
	gst_element_link_many(videoqueue,Vdecoder,Vconverter,Vsink,NULL);//we can add number of elements upto to we reach NULL (internally it calls 									gst_element_link)
	if(gst_element_link_many(audioqueue,Adecoder,Aconverter,Are_sam,Asink,NULL) != TRUE) 
	{
   		 g_printerr ("Elements could not be linked.\n");
    		 gst_object_unref (pipeline);
   		 return -1;
 	}
	
	  g_signal_connect (demux, "pad-added", G_CALLBACK (on_pad_added), NULL); 

	
	//dynamically created a src pad in demux then ,linking with decoder.
	
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
	

	


