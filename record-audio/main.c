#include <libavdevice/avdevice.h>

#define INPUT_FORMAT "alsa"
#define DEVICE_NAME "default"

char errbuf[AV_ERROR_MAX_STRING_SIZE];

int
main (int argc, char **argv)
{
  int ret = 0;
  AVFormatContext *fmt_ctx = NULL;
  const AVInputFormat *in_fmt = NULL;

  avdevice_register_all ();

  in_fmt = av_find_input_format (INPUT_FORMAT);
  if (in_fmt == NULL)
    {
      fprintf (stderr, "Could not find input format.\n");
      return 1;
    }
  printf ("Found input format '%s'.\n", in_fmt->long_name);

  ret = avformat_open_input (&fmt_ctx, DEVICE_NAME, in_fmt, NULL);
  if (ret != 0)
    {
      av_strerror (ret, errbuf, sizeof (errbuf));
      fprintf (stderr, "Could not open audio device: %s\n", errbuf);
      return 1;
    }
  printf ("Successfully opened audio device.\n");

  AVPacket *pkt = av_packet_alloc ();
  FILE *out = fopen ("out.pcm", "wb");
  if (out == NULL)
    {
      fprintf (stderr, "Could not create pcm file.\n");
      avformat_close_input (&fmt_ctx);
      return 1;
    }
  for (int i = 1; (ret = av_read_frame (fmt_ctx, pkt)) == 0 && i <= 250; i++)
    {
      fwrite (pkt->data, 1, pkt->size, out);
      fflush (out);
      printf ("No.%d packet with %d bytes.\n", i, pkt->size);
      av_packet_unref (pkt);
    }
  fclose (out);
  av_packet_free (&pkt);

  avformat_close_input (&fmt_ctx);
}
