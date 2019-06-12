#include<gst/gst.h>
#include <glib.h>
#include<string.h>

typedef struct play
{
	GstElement *pipeline, *source,*source1, *decodebinn1,*decodebinn, *Vsink,*Are_sa,*Asink,*Aconv1,*Vconv,*Aconv,*adderr;
	GstState state;
	GMainLoop *loop;
	GstElement *bin,*mixedqueue ,*audioqueue,*videoqueue;
	GstPad *audio_queue_src_pad,*audio_conv1_src_pad,*adder_conv1_sink_pad,*adder_queue_sink_pad;

}Playdata;
static void on_pad_added (GstElement *element,  GstPad  *pad, Playdata *data)
{
 
 	 GstPad *sinkpad;
  	 GstCaps *caps; 
  	 GstStructure *str; 
  	 const gchar *new_pad_type = NULL;
  	 caps =  gst_pad_get_current_caps (pad); //get current capbalities of demuxer pads
 
  	 str = gst_caps_get_structure (caps, 0); 
  
  	 new_pad_type = gst_structure_get_name (str);
  	
	 if (g_strrstr (gst_structure_get_name (str), "audio"))
	 { 
		g_print (" Link succeeded (type '%s').\n", new_pad_type );
		sinkpad = gst_element_get_static_pad(data->audioqueue, "sink"); 
		gst_pad_link (pad, sinkpad);
		g_print ("linking decoder//Aconverter\n");
		gst_object_unref (sinkpad);
		
		
	 }
	 else if(g_strrstr(gst_structure_get_name (str),"video"))
	 {
				g_print (" Link succeeded (type '%s').\n", new_pad_type );
		sinkpad = gst_element_get_static_pad(data->videoqueue,"sink");
		gst_pad_link(pad,sinkpad);
		gst_object_unref (sinkpad);
				g_print ("linking decoder//Vconverter\n");
	 }	
	
 	 gst_caps_unref (caps);
}
static void on_pad_added_1 (GstElement *element,  GstPad  *pad, Playdata *data)
{
  GstPad *sinkpad;
  GstElement *decoder = (GstElement *) data;

  /* We can now link this pad with the decoder-converter sink pad */
  g_print ("111111111...Dynamic pad created, linking decoder//converter\n");

  sinkpad = gst_element_get_static_pad (data->Aconv1, "sink");

  gst_pad_link (pad, sinkpad);

  gst_object_unref (sinkpad);
}

//Error
static void error_cb (GstBus *bus, GstMessage *msg,Playdata *data) 
{
  	GError *err;
  	gchar *debug_info;
  	gst_message_parse_error (msg, &err, &debug_info);
  	g_printerr ("Error received from element %s: %s\n", GST_OBJECT_NAME (msg->src), err->message);
  	g_printerr ("Debugging information: %s\n", debug_info ? debug_info : "none");
  	g_clear_error (&err);
  	g_free (debug_info);
  	gst_element_set_state (data->pipeline, GST_STATE_READY);
}
//State change
static void state_changed_cb (GstBus *bus, GstMessage *msg,Playdata *data)
{
  	GstState old_state, new_state, pending_state;
  	gst_message_parse_state_changed (msg, &old_state, &new_state, &pending_state);
  	if (GST_MESSAGE_SRC (msg) == GST_OBJECT (data->pipeline)) 
	{
	    data->state = new_state;
	    g_print ("State set to %s\n", gst_element_state_get_name (new_state));
   

   	}

}
//EOS
static void eos_cb (GstBus *bus, GstMessage *msg, Playdata *data) 
{
	  g_print ("End-Of-Stream reached.\n");
	  gst_element_set_state (data->pipeline, GST_STATE_READY);
}

gint main(int argc ,char *argv[])
{
  	GstBus *bus;
	Playdata data;
	GMainLoop *loop;

 	 gst_init (&argc, &argv);
	loop = g_main_loop_new (NULL, FALSE);
	memset (&data, 0, sizeof (data));
	data.pipeline = gst_pipeline_new ("1Video-2Audio-mixed");
	data.source   = gst_element_factory_make ("filesrc","file-source");
  	g_object_set (G_OBJECT (data.source), "location", argv[1], NULL);//set source property

  	data.source1   = gst_element_factory_make ("filesrc","file-source-1");
  	g_object_set (G_OBJECT (data.source1), "location", argv[2], NULL);//set source property

 	data.decodebinn  = gst_element_factory_make ("decodebin","decoderr");
	data.decodebinn1  = gst_element_factory_make ("decodebin","decoderr-1");
	
	data.videoqueue = gst_element_factory_make("queue","videoqueue");
  	data.Vconv     = gst_element_factory_make ("videoconvert",  "Vconverter");
  	data.Vsink     = gst_element_factory_make ("autovideosink", "Video-output");
	
	data.audioqueue = gst_element_factory_make("queue","audioqueue");
	data.Aconv     = gst_element_factory_make ("audioconvert",  "Aconverter");
  	data.Asink     = gst_element_factory_make ("alsasink", "Audio-output");

	data.Aconv1     = gst_element_factory_make ("audioconvert",  "data-Aconverter-1");
  	//data.mixedqueue = gst_element_factory_make("queue","mixedqueue");
	
data.adderr    = gst_element_factory_make ("adder", "data.Aadderr");

	bus = gst_element_get_bus (data.pipeline);
  	gst_bus_add_signal_watch (bus);
  	g_signal_connect (G_OBJECT (bus), "message::error", (GCallback)error_cb, &data);
   	g_signal_connect (G_OBJECT (bus), "message::eos", (GCallback)eos_cb, &data);
  	g_signal_connect (G_OBJECT (bus), "message::state-changed", (GCallback)state_changed_cb, &data);
  	gst_object_unref (bus);

	 gst_bin_add_many (GST_BIN                                                                                                 (data.pipeline),data.source,data.decodebinn,data.videoqueue,data.Vconv,data.Vsink,data.audioqueue, data.adderr,data.Aconv,data.Asink,data.source1,data.decodebinn1,data.Aconv1,NULL);//add ele in bin

 	gst_element_link(data.source, data.decodebinn);
  	gst_element_link_many (data.videoqueue,data.Vconv, data.Vsink, NULL);
  	gst_element_link_many (data.adderr,data.Aconv,data.Asink, NULL);
	g_signal_connect (data.decodebinn, "pad-added", G_CALLBACK(on_pad_added),&data);//link pads

 	gst_element_link(data.source1, data.decodebinn1);

	g_signal_connect (data.decodebinn1, "pad-added", G_CALLBACK(on_pad_added_1),&data);//link pads
//	GstPad *audio_queue_src_pad,*audio_conv1_src_pad,*adder_conv1_sink_pad,*adder_queue_sink_pad;

	data.adder_queue_sink_pad = gst_element_get_request_pad (data.adderr, "sink_%u");
	data.audio_queue_src_pad  = gst_element_get_static_pad (data.audioqueue, "src");

	data.adder_conv1_sink_pad = gst_element_get_request_pad (data.adderr, "sink_%u");
        data.audio_conv1_src_pad = gst_element_get_static_pad (data.Aconv1, "src");
	
	gst_pad_link (data.audio_queue_src_pad,data.adder_queue_sink_pad);
	sleep(1);
	gst_pad_link (data.audio_conv1_src_pad,data.adder_conv1_sink_pad);//linking adder

	g_print ("Now playing: %s\n", argv[1]);
  	gst_element_set_state (data.pipeline, GST_STATE_PLAYING);


 	g_main_loop_run (loop);
	

  	// Free resources 
  	gst_element_set_state (data.pipeline, GST_STATE_NULL);
  	gst_object_unref (data.pipeline);
	g_main_loop_unref (loop);
  	return 0;
}





