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
      fprintf (stderr, "Could not find input format.\n");
      return 1;
    }
  printf ("Found input format '%s'.\n", in_fmt->long_name);

  /* 打开设备 */
  ret = avformat_open_input (&fmt_ctx, DEVICE_NAME, in_fmt, NULL);
  if (ret != 0)
    {
      av_strerror (ret, errbuf, sizeof (errbuf));
      fprintf (stderr, "Could not open audio device: %s\n", errbuf);
      return 1;
    }
  printf ("Successfully opened audio device.\n");

  double time_base = 0;
  int audio_stream_idx = -1;
  AVStream *audio_stream = NULL;
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
      AVStream *stream = fmt_ctx->streams[i];
      AVCodecParameters *params = stream->codecpar;
      if (params && params->codec_type == AVMEDIA_TYPE_AUDIO)
	{
	  time_base = av_q2d (stream->time_base);
	  codec_params = params;
	  audio_stream = stream;
	  audio_stream_idx = i;
	  break;
	}
    }
  if (audio_stream_idx == -1 || audio_stream == NULL || codec_params == NULL)
    {
      fprintf (stderr, "No audio stream detected.\n");
      return 1;
    }

  /* 打开输出文件 */
  FILE *out = fopen ("out.pcm", "wb");
  if (out == NULL)
    {
      fprintf (stderr, "Could not create pcm file.\n");
      avformat_close_input (&fmt_ctx);
      return 1;
    }

  /* 读取音频包 */
  double start_time = AV_NOPTS_VALUE;
  AVPacket *pkt = av_packet_alloc ();
  for (int i = 0; (ret = av_read_frame (fmt_ctx, pkt)) == 0; i++)
    {
      /* 过滤音频包 */
      if (pkt->stream_index != audio_stream_idx)
	continue;

      /* 初始化 start_time */
      if (start_time == AV_NOPTS_VALUE)
	start_time = pkt->pts;

      /* 计算录制时间 */
      double time = (pkt->pts - start_time) * time_base;
      fwrite (pkt->data, 1, pkt->size, out);
      /* fflush (out); */

      printf ("[%lfs] No.%d packet with %d bytes.\n", time, i, pkt->size);
      av_packet_unref (pkt);

      if (time >= 3)
	break;
    }

  fclose (out);
  av_packet_free (&pkt);
  avformat_close_input (&fmt_ctx);
}
