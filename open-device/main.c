#include <libavdevice/avdevice.h>

#define INPUT_FORMAT "alsa"
#define DEVICE_NAME "default"

char err_buff[AV_ERROR_MAX_STRING_SIZE];

int
main (int argc, char **argv)
{
  int ret = 0;
  AVFormatContext *fmt_ctx = NULL;
  const AVInputFormat *in_fmt = NULL;

  avdevice_register_all ();

  in_fmt = av_find_input_format (INPUT_FORMAT);
  if (!in_fmt)
    {
      fprintf (stderr, "Could not find input format '%s'.\n", INPUT_FORMAT);
      return 1;
    }
  printf ("Found input format '%s'.\n", in_fmt->long_name);

  ret = avformat_open_input (&fmt_ctx, DEVICE_NAME, in_fmt, NULL);
  if (ret < 0)
    {
      av_strerror (ret, err_buff, sizeof (err_buff));
      fprintf (stderr, "Could not open audio device: %s.\n", err_buff);
      return 1;
    }
  printf ("Successfully opened audio device.\n");

  int audio_stream_idx = -1;
  AVCodecParameters *codec_params = NULL;
  ret = avformat_find_stream_info (fmt_ctx, NULL);
  if (ret < 0)
    {
      av_strerror (ret, err_buff, sizeof (err_buff));
      fprintf (stderr, "Failed to find stream information: %s.\n", err_buff);
    }

  for (unsigned i = 0; i < fmt_ctx->nb_streams; i++)
    {
      AVCodecParameters *params = fmt_ctx->streams[i]->codecpar;
      if (params && params->codec_type == AVMEDIA_TYPE_AUDIO)
	{
	  codec_params = params;
	  audio_stream_idx = i;
	  break;
	}
    }

  if (audio_stream_idx == -1 || codec_params == NULL)
    {
      fprintf (stderr, "No audio stream detected.\n");
      return 1;
    }
  printf ("Audio Stream Found (Index: %d):\n", audio_stream_idx);

  if (av_channel_layout_check (&codec_params->ch_layout))
    {
      int chans = codec_params->ch_layout.nb_channels;
      if (chans < 1)
	printf ("  Channels: %d\n", chans);
      else
	printf ("  Channels: %s\n", chans == 1 ? "mono" : "stereo");
    }

  const char *sample_fmt_name = av_get_sample_fmt_name (codec_params->format);
  if (sample_fmt_name)
    printf ("  Sample Format: %s\n", sample_fmt_name);
  else
    printf ("  Sample Format: Unknown (%d)\n", codec_params->format);

  printf ("  Sample Rate: %dHz\n", codec_params->sample_rate);

  avformat_close_input (&fmt_ctx);
}
