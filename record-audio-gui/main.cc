#include "ui_main.h"
#include <QThread>

extern "C"
{
#include <libavdevice/avdevice.h>
}

class audio_recorder
{
public:
  audio_recorder (const char *in_fmt, const char *dev_name)
  {
    m_in_fmt = av_find_input_format (in_fmt);
    if (m_in_fmt == nullptr)
      throw std::runtime_error ("Could not find input format.");

    if (avformat_open_input (&m_fmt_ctx, dev_name, m_in_fmt, nullptr) != 0)
      throw std::runtime_error ("Could not open audio device.");
  }

  ~audio_recorder ()
  {
    if (m_pkt != nullptr)
      av_packet_free (&m_pkt);
    if (m_fmt_ctx != nullptr)
      avformat_close_input (&m_fmt_ctx);
  }

  const AVPacket *
  read_frame ()
  {
    if (m_pkt != nullptr)
      av_packet_unref (m_pkt);
    else
      m_pkt = av_packet_alloc ();

    if (av_read_frame (m_fmt_ctx, m_pkt) != 0)
      return nullptr;
    return m_pkt;
  }

public:
  static void
  init ()
  {
    avdevice_register_all ();
  }

private:
  AVPacket *m_pkt = nullptr;
  AVFormatContext *m_fmt_ctx = nullptr;
  const AVInputFormat *m_in_fmt = nullptr;
};

int
main (int argc, char **argv)
{
  auto app = QApplication (argc, argv);
  auto window = QMainWindow ();
  auto ui = Ui::Window ();
  ui.setupUi (&window);

  audio_recorder::init ();

  std::thread worker;
  FILE *out = nullptr;
  std::atomic<bool> recording = false;
  auto recorder = audio_recorder ("alsa", "default");

  auto record = [&] () {
    for (;;)
      {
	if (!recording)
	  break;

	auto pkt = recorder.read_frame ();
	if (pkt != nullptr)
	  fwrite (pkt->data, 1, pkt->size, out);
      }
  };

  QObject::connect (ui.btn, &QPushButton::clicked, [&] () {
    recording = !recording;

    // 停止录制
    if (!recording)
      {
	worker.join ();
	fclose (out);
	out = nullptr;

	ui.btn->setText ("start");
	return;
      }

    // 开始录制
    out = fopen ("out.pcm", "wb");
    if (out == nullptr)
      throw std::runtime_error ("Could not create pcm file.");

    worker = std::thread (record);
    ui.btn->setText ("stop");
  });

  QObject::connect (&app, &QCoreApplication::aboutToQuit, [&] () {
    recording = false;
    if (worker.joinable ())
      worker.join ();
    if (out != nullptr)
      fclose (out);
  });

  window.show ();
  return app.exec ();
}
