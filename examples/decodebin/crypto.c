#include<gst/gst.h>
#include <glib.h>
#include<gtk/gtk.h>
#include<string.h>
#include <gst/pbutils/pbutils.h>//for gstdiscover utility object ,Pbutils — General Application and Plugin Utility Library

#include <gdk/gdkx.h>//linux platform supported library
#include <gst/video/videooverlay.h>//interface b//w gstreamer and window supported library
gint w=720,h=540;
gchar *filename1,*filename2;
static gint i=0;

typedef struct play{
	GstElement *pipeline, *source, *decodebinn, *Vsink,*Are_sa,*Asink,*Vconv,*Aconv,*playbin,*Vsink1;
	GstElement *audioqueue,*videoqueue,*queue1,*cryptoo;
	GstState state;
	GMainLoop *loop;
  	GstDiscoverer *discoverer;//
	GtkWidget *slider;            
  	gulong slider_update_signal_id; 
	gint64 duration,current,position; 
}Playdata;


static void on_pad_added (GstElement *element,  GstPad  *pad,Playdata *data);
static void error_cb (GstBus *bus, GstMessage *msg, Playdata *data); 
static void state_changed_cb (GstBus *bus, GstMessage *msg, Playdata *data);
static void eos_cb (GstBus *bus, GstMessage *msg, Playdata *data);



int main(int argc, char *argv[])
{

	GMainLoop *loop;
  	GstBus *bus;
	GstStateChangeReturn ret;
	Playdata data;
	GError *err = NULL;//
	gchar *uri;
	gchar *dec=dec;
  	gst_init(&argc,&argv);//initize the gstreamer library
	gtk_init(&argc,&argv);//initize the gstreamer library

	if(argc==3)
		w=atoi(argv[2]);//char to integer convertion
	if(argc==4)
		h=atoi(argv[3]);

	loop = g_main_loop_new (NULL, FALSE);
	memset (&data, 0, sizeof (data));
	data.duration = GST_CLOCK_TIME_NONE;

	/* Create gstreamer elements */
  	data.pipeline = gst_pipeline_new ("Video-Audio-VT-Player");
  	data.source   = gst_element_factory_make ("filesrc","file-source");


  	data.cryptoo= gst_element_factory_make ("crypto", "cry_pto");
    	data.queue1= gst_element_factory_make ("queue", "cryqueue");
  	g_object_set (G_OBJECT (data.cryptoo), "mode", "dec", NULL);//set source property

	if(data.cryptoo==NULL)
		printf("element not created\n");
 	data.decodebinn  = gst_element_factory_make ("decodebin","decoderr");
  	data.audioqueue = gst_element_factory_make("queue","audioqueue");
  	data.videoqueue = gst_element_factory_make("queue","videoqueue");
  	data.Vconv     = gst_element_factory_make ("videoconvert",  "Vconverter");
  	data.Vsink     = gst_element_factory_make ("xvimagesink", "Video-output");


  	data.Aconv     = gst_element_factory_make ("audioconvert",  "data.Aconverter");
  	data.Are_sa    = gst_element_factory_make ("audioresample", "data.Are_sample");
  	data.Asink     = gst_element_factory_make ("alsasink", "Audio-output");

	
			 

	
 	bus = gst_element_get_bus (data.pipeline);
  	gst_bus_add_signal_watch (bus);
  	g_signal_connect (G_OBJECT (bus), "message::error", (GCallback)error_cb, &data);
   	g_signal_connect (G_OBJECT (bus), "message::eos", (GCallback)eos_cb, &data);
  	g_signal_connect (G_OBJECT (bus), "message::state-changed", (GCallback)state_changed_cb, &data);

  	 gst_object_unref (bus);
	
 	 gst_bin_add_many (GST_BIN (data.pipeline),data.source,data.cryptoo,
data.queue1,data.decodebinn,data.videoqueue, data.Vconv,data.Vsink,data.audioqueue,data.Aconv,data.Asink, data.Are_sa, NULL);//add ele in bin  //
	 gst_element_link_many (data.source,data.cryptoo,data.queue1,data.decodebinn,NULL);// 
  	 gst_element_link_many (data.videoqueue,data.Vconv, data.Vsink, NULL);
  	 gst_element_link_many (data.audioqueue, data.Aconv,data.Are_sa, data.Asink, NULL);

   	 g_signal_connect (data.decodebinn, "pad-added", G_CALLBACK(on_pad_added),&data);//link pads

	 if(argc>=2){
		uri=gst_filename_to_uri (argv[1],NULL);
  		g_object_set (G_OBJECT (data.source), "location", argv[1], NULL);//set source property
	 }
	 else{
		uri="file:///~/Pictures/sample.png";//
		g_object_set (G_OBJECT (data.source), "location",uri, NULL);
	 }
	 
	

	 g_print ("Now playing: %s\n", argv[1]);
  	 ret = gst_element_set_state (data.pipeline, GST_STATE_PLAYING);
  	 if (ret == GST_STATE_CHANGE_FAILURE) {
   		g_printerr ("Unable to set the pipeline to the playing state.\n");
    		gst_object_unref (data.pipeline);
    		return -1;
  	 }



	 g_main_loop_run (loop);
	

  	// Free resources 
  	gst_element_set_state (data.pipeline, GST_STATE_NULL);
  	gst_object_unref (data.pipeline);

	g_main_loop_unref (loop);
  	return 0;
}
//
static void on_pad_added (GstElement *element,  GstPad  *pad, Playdata *data)
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
		//g_print ("\t\t\t Link succeeded (type '%s').\n", new_pad_type );
		sinkpad = gst_element_get_static_pad(data->audioqueue, "sink"); 
		gst_pad_link (pad, sinkpad);
		//g_print ("\t\t\tlinking decoder//Aconverter\n");
		gst_object_unref (sinkpad);
		
		
	 }
	 else if(g_strrstr(gst_structure_get_name (str),"video"))
	 {
		sinkpad = gst_element_get_static_pad(data->videoqueue,"sink");
		gst_pad_link(pad,sinkpad);
		gst_object_unref (sinkpad);
	 }	
	
 	 gst_caps_unref (caps);
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
   

	    if (old_state == GST_STATE_READY && new_state == GST_STATE_PAUSED); 

   	}

}
//EOS
static void eos_cb (GstBus *bus, GstMessage *msg, Playdata *data) 
{
	  g_print ("End-Of-Stream reached.\n");
	  gst_element_set_state (data->pipeline, GST_STATE_READY);
}

