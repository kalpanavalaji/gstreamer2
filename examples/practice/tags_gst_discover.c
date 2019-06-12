

/* Print a tag in a human-readable format (name: value) */
static void print_tag_foreach (const GstTagList *tags, const gchar *tag, gpointer user_data) {
  GValue val = { 0, };
  gchar *str;
  gint depth = GPOINTER_TO_INT (user_data);

  gst_tag_list_copy_value (&val, tags, tag);

  if (G_VALUE_HOLDS_STRING (&val))
    str = g_value_dup_string (&val);
  else
    str = gst_value_serialize (&val);

  g_print ("\t%*s%s: %s\n", 2 * depth, " ", gst_tag_get_nick (tag), str);
  g_free (str);

  g_value_unset (&val);
}

/* Print information regarding a stream */
static void print_stream_info (GstDiscovererStreamInfo *info, gint depth) {
  gchar *desc = NULL;
  GstCaps *caps;
  const GstTagList *tags;

  caps = gst_discoverer_stream_info_get_caps (info);

  if (caps) {
    if (gst_caps_is_fixed (caps))
      desc = gst_pb_utils_get_codec_description (caps);
    else
      desc = gst_caps_to_string (caps);
    gst_caps_unref (caps);
  }

  g_print ("%*s%s: %s\n", 2 * depth, " ", gst_discoverer_stream_info_get_stream_type_nick (info), (desc ? desc : ""));

  if (desc) {
    g_free (desc);
    desc = NULL;
  }
}

/* Print information regarding a stream and its substreams, if any */
static void print_topology (GstDiscovererStreamInfo *info, gint depth) {
  GstDiscovererStreamInfo *next;

  if (!info)
    return;

  print_stream_info (info, depth);

  next = gst_discoverer_stream_info_get_next (info);
  if (next) {
    print_topology (next, depth + 1);
    gst_discoverer_stream_info_unref (next);
  } else if (GST_IS_DISCOVERER_CONTAINER_INFO (info)) {
    GList *tmp, *streams;

    streams = gst_discoverer_container_info_get_streams (GST_DISCOVERER_CONTAINER_INFO (info));
    for (tmp = streams; tmp; tmp = tmp->next) {
      GstDiscovererStreamInfo *tmpinf = (GstDiscovererStreamInfo *) tmp->data;
      print_topology (tmpinf, depth + 1);
    }
    gst_discoverer_stream_info_list_free (streams);
  }
}

/* This function is called every time the discoverer has information regarding
 * one of the URIs we provided.*/


/* This function is called when the discoverer has finished examining
 * all the URIs we provided.*/
static void on_finished_cb (GstDiscoverer *discoverer, CustomData *data) {
  g_main_loop_quit (data->loop);
}

int main (int argc, char **argv) {




 




  /* Instantiate the Discoverer */

  if (!data.discoverer) {
    g_print ("Error creating discoverer instance: %s\n", err->message);
    g_clear_error (&err);
    return -1;
  }

  /* Connect to the interesting signals */

  g_signal_connect (data.discoverer, "finished", G_CALLBACK (on_finished_cb), &data);

  /* Start the discoverer process (nothing to do yet) */


