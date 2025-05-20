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

  /* 注册设备 */
  avdevice_register_all ();

  /* 确定输入格式 */
  in_fmt = av_find_input_format (INPUT_FORMAT);
  if (in_fmt == NULL)
    {
      fprintf (stderr, "Could not find input format '%s'.\n", INPUT_FORMAT);
      return 1;
    }
  printf ("Found input format '%s'.\n", in_fmt->long_name);

  /* 打开设备 */
  ret = avformat_open_input (&fmt_ctx, DEVICE_NAME, in_fmt, NULL);
  if (ret != 0)
    {
      av_strerror (ret, errbuf, sizeof (errbuf));
      fprintf (stderr, "Could not open audio device: %s.\n", errbuf);
      return 1;
    }
  printf ("Successfully opened audio device.\n");

  int audio_stream_idx = -1;
  AVCodecParameters *codec_params = NULL;

  /* 补充流信息 */
  ret = avformat_find_stream_info (fmt_ctx, NULL);
  if (ret < 0)
    {
      av_strerror (ret, errbuf, sizeof (errbuf));
      fprintf (stderr, "Failed to find stream information: %s.\n", errbuf);
    }

  /* 找到第一个音频流及其编码参数 */
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

  /* 通道数 */
  if (av_channel_layout_check (&codec_params->ch_layout))
    {
      int chans = codec_params->ch_layout.nb_channels;
      if (chans < 1)
	printf ("  Channels: %d\n", chans);
      else
	printf ("  Channels: %s\n", chans == 1 ? "mono" : "stereo");
    }

  /* 采样格式 */
  const char *sample_fmt_name = av_get_sample_fmt_name (codec_params->format);
  if (sample_fmt_name)
    printf ("  Sample Format: %s\n", sample_fmt_name);
  else
    printf ("  Sample Format: Unknown (%d)\n", codec_params->format);

  /* 采样率 */
  printf ("  Sample Rate: %dHz\n", codec_params->sample_rate);

  avformat_close_input (&fmt_ctx);
}
