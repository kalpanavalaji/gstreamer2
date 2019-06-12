#include <gst/gst.h>
#include <glib.h>
#include<string.h>
typedef struct play{

	GstElement *pipeline, *source, *demux,*Vdec,*Adec, *Vsink,*Are_sa,*Asink,*Vconv,*Aconv,*Vsink1,*Aparser,*Vparser;
	GstElement *audioqueue,*videoqueue,*typefind;
	GstState state;


}Playdata;
	GMainLoop *loop;
static void on_pad_added (GstElement *element,  GstPad  *pad, Playdata *data);

static void cb_typefound (GstElement *typefinder, guint  probability, GstCaps *caps,Playdata *data);

static gboolean bus_call (GstBus  *bus , GstMessage *msg , gpointer  data);


int main (int   argc,char *argv[])
{
  
  	GstBus *bus;
  	guint bus_watch_id;gint ret;
	Playdata data;


  	gst_init (&argc, &argv);
  
 	loop = g_main_loop_new (NULL, FALSE);
 	memset (&data, 0, sizeof (data));
 	 if (argc != 2) 
 	 {
 		   g_printerr ("Usage: %s <Mp4 filename>\n", argv[0]);
 		   return -1;
 	 }


 	data.pipeline = gst_pipeline_new ("Video-Audio-VT-Player");
  	data.source   = gst_element_factory_make ("filesrc","file-source");
	g_object_set (G_OBJECT (data.source), "location", argv[1], NULL);
	
 	data.typefind  = gst_element_factory_make ("typefind","typefinder");
  	g_signal_connect (data.typefind, "have-type", G_CALLBACK (cb_typefound), &data);

   	bus = gst_pipeline_get_bus (GST_PIPELINE (data.pipeline));
   	bus_watch_id = gst_bus_add_watch (bus, bus_call,loop);
   	gst_object_unref (bus);
  	gst_bin_add_many (GST_BIN (data.pipeline),data.source,data.typefind,NULL);

	gst_element_link (data.source, data.typefind);

  	
  	g_print ("Now playing: %s\n", argv[1]);
  	ret=gst_element_set_state (data.pipeline, GST_STATE_PLAYING);
	if (ret == GST_STATE_CHANGE_FAILURE) {
   		g_printerr ("Unable to set the pipeline to the playing state.\n");
    		gst_object_unref (data.pipeline);
    		return -1;
  	 }


  	g_print ("Running...\n");
  	g_main_loop_run (loop);
  


  	g_print ("Returned, stopping playback\n");
 	gst_element_set_state (data.pipeline, GST_STATE_NULL);
  
  	g_print ("Deleting pipeline\n");
  	gst_object_unref (GST_OBJECT (data.pipeline));
  	g_source_remove (bus_watch_id);
 	g_main_loop_unref (loop);
  return 0;
}
/*
static void on_pad_added (GstPadTemplate * templ, GstPad * newpad, gpointer user_data)
{
  GstPad *target = GST_PAD (user_data);
  gst_pad_link (newpad, target);
  gst_object_unref (target);
}
*/
static void on_pad_added (GstElement *element,  GstPad  *pad, Playdata *data)
{
 
 	 GstPad *sinkpad;
  	 GstCaps *caps; 
  	 GstStructure *str; 
  	 const gchar *new_pad_type = NULL;
  	 g_print (" Starting link (type '%s').\n", new_pad_type );
  	 caps =  gst_pad_get_current_caps (pad); //get current capbalities of demuxer pads
 
  	 str = gst_caps_get_structure (caps, 0); 
  
  	 new_pad_type = gst_structure_get_name (str);
  	
	 if (g_strrstr (gst_structure_get_name (str), "audio"))
	 { 
		g_print (" Link succeeded (type '%s').\n", new_pad_type );
		sinkpad = gst_element_get_static_pad(data->Adec, "sink"); 
		if(gst_pad_link (pad, sinkpad))
				g_print ("linking decoder//Aconverter\n");
		gst_object_unref (sinkpad);
		
		
	 }
	 else if(g_strrstr(gst_structure_get_name (str),"video"))
	 {
		g_print (" Link succeeded (type '%s').\n", new_pad_type );		
		sinkpad = gst_element_get_static_pad(data->Vdec,"sink");
		gst_pad_link(pad,sinkpad);
		gst_object_unref (sinkpad);
	 }	

 	 gst_caps_unref (caps);
}
static void cb_typefound (GstElement *typefinder, guint  probability, GstCaps *caps, Playdata *data)
{


 	// GMainLoop *loop =  (GMainLoop *)data;
 	 gchar *type;
	 GstPad *pad;


 	 type = gst_caps_to_string (caps);
	 g_print ("Media type %s found, probability %d%%\n", type, probability);
	 gst_element_set_state (data->pipeline, GST_STATE_PAUSED);//
		// gst_element_set_state (data->pipeline, GST_STATE_READY);//
  	 data->demux = gst_element_factory_make("qtdemux","qtdemuxer");

  	data->Vdec = gst_element_factory_make("avdec_h264","videodecoder");
  	data->Adec = gst_element_factory_make("avdec_aac","audiodecoder");
	//data.Vparser=gst_element_factory_make("h264parse","videoparser");
	//data.Aparser=gst_element_factory_make("aacparse","audioparser");

	data->audioqueue = gst_element_factory_make("queue","audioqueue");
  	data->videoqueue = gst_element_factory_make("queue","videoqueue");
  	data->Vconv     = gst_element_factory_make ("videoconvert",  "Vconverter");
  	data->Vsink     = gst_element_factory_make ("ximagesink", "Video-output");


  	data->Aconv     = gst_element_factory_make ("audioconvert",  "data.Aconverter");
  	data->Are_sa    = gst_element_factory_make ("audioresample", "data.Are_sample");
  	data->Asink     = gst_element_factory_make ("autoaudiosink", "Audio-output");


  	gst_bin_add_many (GST_BIN (data->pipeline),data->demux,data->Vdec,data->Adec,data->videoqueue, data->Vconv,data->Vsink,data->audioqueue, data->Aconv,data->Asink, data->Are_sa,NULL);//add ele in bin//data.Vparser,data.Aparser ,


	if(!gst_element_link (data->typefind,data->demux))//data.Vparser,
 					g_print("linking error\n");
	gst_element_link_many (data->Vdec,data->videoqueue,data->Vconv, data->Vsink, NULL);
  	gst_element_link_many (data->Adec,data->audioqueue, data->Aconv,data->Are_sa, data->Asink, NULL);//data.Aparser,



	if(!g_signal_connect (data->demux, "pad-added", G_CALLBACK(on_pad_added),data))//link pads
								g_print("signal error\n");

  	gst_element_set_state (data->pipeline, GST_STATE_PLAYING);
	g_free (type);  

}
static gboolean bus_call (GstBus  *bus , GstMessage *msg , gpointer  data)
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



