#include <gst/gst.h>
#include <glib.h>
#include<string.h>
typedef struct play{

	GstElement *pipeline, *source, *demux,*Vdec,*Adec, *Vsink,*Are_sa,*Asink,*Vconv,*Aconv,*Vsink1,*Aparser,*Vparser;
	GstElement *audioqueue,*videoqueue,*typefind,*parseA,*parseV;
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
		sinkpad = gst_element_get_static_pad(data->audioqueue, "sink"); 
		if(gst_pad_link (pad, sinkpad))
				g_print ("linking decoder//Aconverter\n");
		gst_object_unref (sinkpad);
		
		
	 }
	 else if(g_strrstr(gst_structure_get_name (str),"video"))
	 {
		g_print (" Link succeeded (type '%s').\n", new_pad_type );		
		sinkpad = gst_element_get_static_pad(data->videoqueue,"sink");
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
	gint i=0;
		data->audioqueue = gst_element_factory_make("queue","audioqueue");
  		data->videoqueue = gst_element_factory_make("queue","videoqueue");
  		data->Vconv     = gst_element_factory_make ("videoconvert",  "Vconverter");
  		data->Vsink     = gst_element_factory_make ("ximagesink", "Video-output");
  		data->Aconv     = gst_element_factory_make ("audioconvert",  "data.Aconverter");
  		data->Are_sa    = gst_element_factory_make ("audioresample", "data.Are_sample");
  		data->Asink     = gst_element_factory_make ("autoaudiosink", "Audio-output");

 	 type = gst_caps_to_string (caps);
	 g_print ("Media type %s found, probability %d%%\n", type, probability);
	 gst_element_set_state (data->pipeline, GST_STATE_PAUSED);//

	 if (!strncmp ("video/quicktime", type,15))//mp4 ,mov
	 {
  		data->demux = gst_element_factory_make("qtdemux","qtdemuxer");
		data->Vdec = gst_element_factory_make("avdec_h264","videodecoder");
  		data->Adec = gst_element_factory_make("avdec_aac","audiodecoder");
	 
		//data.Vparser=gst_element_factory_make("h264parse","videoparser");
		//data.Aparser=gst_element_factory_make("aacparse","audioparser");
	 }
	 else if (!strcmp ("video/webm", type))//webm
	 {
  		data->demux = gst_element_factory_make("matroskademux","matroskademuxer");
		data->Vdec = gst_element_factory_make("vp8dec","videodecoder");
  		data->Adec = gst_element_factory_make("vorbisdec","audiodecoder");
	 }
	else if(!strncmp("application/x-3gp",type,17))//3gp 
	 {
		 	g_print("linking error\n");
		data->parseV    = gst_element_factory_make ("h263parse",  "data.parse2");
  		data->demux = gst_element_factory_make("qtdemux","qtdemuxer");
		data->Vdec = gst_element_factory_make("avdec_h263","videodecoder");//h263parse
  		data->Adec = gst_element_factory_make("avdec_amrnb","audiodecoder");
		gst_bin_add (GST_BIN (data->pipeline),data->parseV);
	 }
	 else  if(!strcmp("video/x-matroska",type))//mkv mka, mk3d, webm
	 {

  		data->demux = gst_element_factory_make("matroskademux","qtdemuxer");
		data->Vdec = gst_element_factory_make("avdec_h264","videodecoder"); //h264parse
  		data->Adec = gst_element_factory_make("vorbisdec","audiodecoder"); 

	 }
	 else if(!strcmp("video/ogg",type))//ogg
	 {

  		data->demux = gst_element_factory_make("oggdemux","qtdemuxer");
		data->Vdec = gst_element_factory_make("theoradec","videodecoder");//ogmvideoparse: OGM video stream parser
  		data->Adec = gst_element_factory_make("vorbisdec","audiodecoder");//ogmaudioparse: OGM audio stream parser
	 }
	 else if(!strcmp("application/x-id3",type))//MP3
	 {
  		data->demux = gst_element_factory_make("id3demux","qtdemuxer");
		data->Adec = gst_element_factory_make("flump3dec","audiodecoder");
	 }
 	 else  if(!strcmp("video/x-flv",type))//mkv mka, mk3d, webm
	 {

  		data->demux = gst_element_factory_make("flvdemux","qtdemuxer");
		data->Vdec = gst_element_factory_make("avdec_flv","videodecoder"); //h264parse
  		data->Adec = gst_element_factory_make("faad","audiodecoder"); 

	 }
	else  if(!strncmp("video/mpegts",type,12))//mts
	 {
 					g_print("linking erro11111111111111111111r\n");
		

		data->parseA    = gst_element_factory_make ("ac3parse",  "data.parse1");
		data->parseV    = gst_element_factory_make ("h264parse",  "data.parse2");
  		data->demux = gst_element_factory_make("tsdemux","qtdemuxer");
		data->Vdec = gst_element_factory_make("avdec_h264","videodecoder"); //h264parse
  		data->Adec = gst_element_factory_make("a52dec","audiodecoder"); 
		gst_bin_add_many (GST_BIN (data->pipeline),data->parseV,data->parseA,NULL);

	 }
	 else 
		g_print("Did not find the type :::%s\n",type);
	
  		gst_bin_add_many (GST_BIN (data->pipeline),data->demux,data->Vdec,data->Adec,data->videoqueue, data->Vconv,data->Vsink,data->audioqueue, data->Aconv,data->Asink, data->Are_sa,NULL);//add ele in bin//data.Vparser,data.Aparser ,

		

	if(!gst_element_link (data->typefind,data->demux))//data.Vparser,
 					g_print("linking error2\n");
	
	if(data->parseA!=NULL){
		 					g_print("linking error parser1\n");
  		gst_element_link_many (data->audioqueue,data->parseA,data->Adec,data->Aconv,data->Are_sa, data->Asink, NULL);//data.Aparser,
	}
	else {
					 					g_print("linking error parser11111111111\n");
  		gst_element_link_many (data->audioqueue,data->Adec,data->Aconv,data->Are_sa, data->Asink, NULL);//data.Aparser,
	}
	
	if(data->parseV!=NULL){
				 					g_print("linking error parser2\n");
		gst_element_link_many (data->videoqueue,data->parseV,data->Vdec,data->Vconv, data->Vsink, NULL);
	}
	else{
								g_print("linking error parser22222222222222\n");
		gst_element_link_many (data->videoqueue,data->Vdec,data->Vconv, data->Vsink, NULL);

	}

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



