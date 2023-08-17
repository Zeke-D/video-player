#include "video.h"

  
enum AVPixelFormat correct_for_deprecated_pixel_format(enum AVPixelFormat pix_fmt) {
    // Fix swscaler deprecated pixel format warning
    // (YUVJ has been deprecated, change pixel format to regular YUV)
    switch (pix_fmt) {
        case AV_PIX_FMT_YUVJ420P: return AV_PIX_FMT_YUV420P;
        case AV_PIX_FMT_YUVJ422P: return AV_PIX_FMT_YUV422P;
        case AV_PIX_FMT_YUVJ444P: return AV_PIX_FMT_YUV444P;
        case AV_PIX_FMT_YUVJ440P: return AV_PIX_FMT_YUV440P;
        default:                  return pix_fmt;
    }
}


void cleanup_frame_context(FrameContext frame_context) {
  printf("Cleaning up");
  free(frame_context.frame_data);
  av_packet_unref(frame_context.video_packet);
  av_frame_free(&frame_context.video_frame);
  av_packet_free(&frame_context.video_packet);
  avformat_close_input(&frame_context.format_context);
  return;  
}

FrameContext init_frame_context(const char* path_to_input) {
  FrameContext frame_ctx;
  frame_ctx.format_context = avformat_alloc_context();
  frame_ctx.video_stream_ind = -1;
  frame_ctx.audio_stream_ind = -1;
  
  avformat_open_input(&frame_ctx.format_context, path_to_input, NULL, NULL);
  avformat_find_stream_info(frame_ctx.format_context, NULL); // populates stream headers in format ctx struct  
  
  // duration is in AV_TIME_BASE fractional seconds
  int64_t hours, mins, secs, us;
  int64_t duration = frame_ctx.format_context->duration + (frame_ctx.format_context->duration <= INT64_MAX - 5000 ? 5000 : 0);
  secs  = duration / AV_TIME_BASE;
  us    = duration % AV_TIME_BASE;
  mins  = secs / 60;
  secs %= 60;
  hours = mins / 60;
  mins %= 60;
  printf("Format %s\n", frame_ctx.format_context->iformat->long_name);
  printf("Duration: %02lldh : %02lldm : %02llds : %02lldms\n", hours, mins, secs, (100 * us) / AV_TIME_BASE);

  // av_dump_format(frame_ctx.format_context, 0, path_to_input, 0);

  // Classify the streams
  for (uint8_t i = 0; i < frame_ctx.format_context->nb_streams && (frame_ctx.video_stream_ind == -1 || frame_ctx.audio_stream_ind == -1); i++) {
  
    // CODEC = Coder/Decoder, like MODEM (modulator/demodulator)
    AVCodecParameters* local_codec_parms = frame_ctx.format_context->streams[i]->codecpar;
    const AVCodec* local_codec = avcodec_find_decoder(local_codec_parms->codec_id);
    
    if (frame_ctx.video_stream_ind == -1 && local_codec_parms->codec_type == AVMEDIA_TYPE_VIDEO) {
      frame_ctx.width = local_codec_parms->width;
      frame_ctx.height = local_codec_parms->height;
      frame_ctx.video_codec = local_codec;
      frame_ctx.video_stream_ind = i;
      frame_ctx.video_codec_context = avcodec_alloc_context3(frame_ctx.video_codec);
      avcodec_parameters_to_context(frame_ctx.video_codec_context, local_codec_parms);
      avcodec_open2(frame_ctx.video_codec_context, frame_ctx.video_codec, NULL);
      printf("Video Codec: %dx%d\n", frame_ctx.width, frame_ctx.height);
    }
    else if (frame_ctx.audio_stream_ind == -1 && local_codec_parms->codec_type == AVMEDIA_TYPE_AUDIO) {
      printf("Audio Codec: %d channel(s), sample rate %d\n", local_codec_parms->ch_layout.nb_channels, local_codec_parms->sample_rate);
      frame_ctx.audio_codec = local_codec;
      frame_ctx.audio_stream_ind = i;
      /*
      TODO: audio?
      audio_codec_context = avcodec_alloc_context3(audio_codec);
      avcodec_parameters_to_context(audio_codec_context, local_codec_parms);
      avcodec_open2(audio_codec_context, audio_codec, NULL);
      */
    }
  }
  
  int alloc_size = frame_ctx.width * frame_ctx.height * 4; // RGBA per pixel
  frame_ctx.frame_data = calloc(alloc_size, sizeof(uint8_t));
  

  printf("Allocated %d bytes.\n", alloc_size);
  frame_ctx.video_frame  = av_frame_alloc();
  frame_ctx.video_packet = av_packet_alloc();


  // TODO: check to ensure all allocations were successful
  return frame_ctx;
}


int get_one_video_frame(FrameContext* frame_context) {
  
  int res = -1;
  do { 
    res = av_read_frame(frame_context->format_context, frame_context->video_packet); 
  } while (frame_context->video_packet->stream_index != frame_context->video_stream_ind || res < 0);

  // decode the packets
  res = avcodec_send_packet(frame_context->video_codec_context, frame_context->video_packet);
  if (res < 0) {
    fprintf(stderr, "Error sending packet.\n");
    return res;
  }

  res = avcodec_receive_frame(frame_context->video_codec_context, frame_context->video_frame);

  if (res < 0) {
    return res;
  }

  printf("Frame %lld (type=%c, size=%d bytes)\n", 
    frame_context->video_codec_context->frame_num, 
    av_get_picture_type_char(frame_context->video_frame->pict_type),
    frame_context->video_packet->size
  );

  // initialize software scaler to guarantee we get data from frame into RGB from YUV
  struct SwsContext* scaler_context = sws_getContext(
    frame_context->width, frame_context->height, correct_for_deprecated_pixel_format(frame_context->video_codec_context->pix_fmt),
    frame_context->width, frame_context->height, AV_PIX_FMT_RGB0,
    SWS_BILINEAR, NULL, NULL, NULL
  );

  uint8_t* dest[4] = {frame_context->frame_data, NULL, NULL, NULL};
  int dest_linesize[4] = {frame_context->width * 4, 0, 0, 0};
  sws_scale(scaler_context, (const uint8_t* const*)frame_context->video_frame->data, frame_context->video_frame->linesize, 0, 
            frame_context->video_frame->height, dest, dest_linesize);

  return res;
  
}
