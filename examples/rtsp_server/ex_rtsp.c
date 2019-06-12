#include<gtk/gtk.h>
#include<glib.h>
#include<gtk/gtk.h>
static void on_pad_added(GstElement *element, GstPad *pad, gpointer data) { 
        gchar *name; 
        name = gst_pad_get_name(pad); 
        g_print("A new pad %s was created.\n", name); 
        g_free(name); 

        GstPad *sinkpad; 
        GstElement *downstream = (GstElement *) data; 

        sinkpad = gst_element_get_static_pad(downstream, "sink"); 

        gst_pad_link (pad, sinkpad); 

        gst_object_unref(sinkpad); 
} 

int main(int argc, char *argv[])
{ 
	        GstCaps *caps; 
		GMainLoop *loop;
        GstElement *pipeline;
	 GstElement *source, *dec_bin, *v_conv, *sink; 
        GstBus *bus; 
        guint bus_watch_id; 
	gtk_init(&argc,&argv);//initize the gstreamer library
  	gst_init(&argc,&argv);//initize the gstreamer library

        pipeline = gst_pipeline_new("my_pipeline"); 
        bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline)); 
        bus_watch_id = gst_bus_add_watch(bus, bus_callback, NULL); 
        gst_object_unref(bus); 

        
        source = gst_element_factory_make("rtspsrc", "source"); 
        dec_bin = gst_element_factory_make("decodebin", "dec_bin"); 
        v_conv = gst_element_factory_make("videoconvert", "v_con"); 
        sink = gst_element_factory_make("autovideosink", "sink"); 
        gst_bin_add_many(GST_BIN(pipeline), source, dec_bin, v_conv, sink, NULL); 

        // Set element properties. 
        g_object_set(G_OBJECT(source), "port-range", "5000-5100", NULL); 
        g_object_set(G_OBJECT(source), "location", vs_location, NULL); 
        g_object_set(G_OBJECT(source), "latency", 0, NULL); 

        g_signal_connect(source, "pad-added", G_CALLBACK(on_pad_added), dec_bin); 
        g_signal_connect(dec_bin, "pad-added", G_CALLBACK(on_pad_added), v_conv); 


        caps = gst_caps_new_simple( 
                        "video/x-raw", 
                        "width", G_TYPE_INT, 640, 
                        "height", G_TYPE_INT, 480, 
                        "format", G_TYPE_STRING, "YUY2", 
                        NULL); 
        if(gst_element_link_filtered(v_conv, sink, caps) == FALSE) { 
                g_printerr("Error: gst_element_link_filtered() failed.\n"); 

                gst_element_set_state(pipeline, GST_STATE_NULL); 
                gst_object_unref(pipeline); 
                g_source_remove(bus_watch_id); 

                return -1; 
        } 
        gst_caps_unref(caps); 

        gst_element_set_state(pipeline, GST_STATE_PLAYING); 

        loop = g_main_loop_new(NULL, FALSE); 
        g_main_loop_run(loop); 

        // Clean up. 
        gst_element_set_state(pipeline, GST_STATE_NULL); 
        gst_object_unref(pipeline); 
        g_source_remove(bus_watch_id); 
        g_main_loop_unref(loop); 

        return 0; 
}

