#include<gst/gst.h>

int main(int argc,char *argv[])
{
	GstElement *pipeline;
	GstBus *bus;
	GstMessage *msg;
	gst_init(&argc,&argv);//before the GStreamer libraries can be used,gst_init has to be called from the main application. This call will perform the necessary initialization of the library as well as parse the GStreamer-specific command line options.

	pipeline=gst_parse_launch("playbin uri=https://www.freedesktop.org/software/gstreamer-sdk/data/media/sintel_trailer-480p.webm", NULL);//Build the Pipeline.
	
	gst_element_set_state(pipeline,GST_STATE_PLAYING);//start playing

	bus=gst_element_get_bus(pipeline);
	msg=gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);
	
	//if (msg != NULL)
	//    gst_message_unref (msg);
	 gst_object_unref (bus);
	 gst_element_set_state (pipeline, GST_STATE_NULL);
	 gst_object_unref (pipeline);
}

	
