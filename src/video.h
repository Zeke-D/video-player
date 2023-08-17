#ifndef libav_imports
#define libav_imports

#include <stdio.h>
#include <stdbool.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>

#endif

typedef struct FrameContext_s {
  AVFormatContext* format_context;
  int video_stream_ind;
  int audio_stream_ind;
  const AVCodec* video_codec;
  const AVCodec* audio_codec;
  AVCodecContext* video_codec_context;
  int width;
  int height;
  AVPacket* video_packet;
  AVFrame*  video_frame;
  uint8_t* frame_data;
} FrameContext;


// get one frame from the given format, 
// on success, returns 0 and fills the FrameContext frame_data with the RGBA pixel values
// on failure, returns AV error code (< 0 for all errors)
int get_one_video_frame(FrameContext* frame_context);

// initializes a FrameContext to read from a video stream accessible via input path
FrameContext init_frame_context(const char* path_to_input);

// frees allocated resources part of the frame context
void cleanup_frame_context(FrameContext frame_context);

// FFmpeg deprecated YUVJ, just YUV now
// from https://github.com/bmewj/video-app
enum AVPixelFormat correct_for_deprecated_pixel_format(enum AVPixelFormat pix_fmt);