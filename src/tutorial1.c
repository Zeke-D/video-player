#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
// #include <libswscale/swscale.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define INBUF_SIZE 4096


// exits the program with an error msg if the passed condition is true
void exit_if(bool cond, const char* message) {
  if (cond) {
    fprintf(stderr, message);
    exit(1);
  }
}

int main(int argc, char* argv[]) {
  
  AVFormatContext* p_format_ctx = NULL;
  int video_stream_ind = -1;
  const AVCodec* p_codec;
  AVStream* video;
  AVCodecContext* p_codec_ctx = NULL;
  AVCodecParserContext* p_parser;
  FILE*     p_file;
  AVFrame*  p_frame;
  uint8_t   inbuf[INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
  uint8_t*  p_data;
  size_t    data_size;
  int       eof;  
  AVPacket* p_pkt;
  
  
  printf("Starting program.\n");


  p_pkt = av_packet_alloc();
  exit_if(p_pkt == NULL, "Unable to alloc packet.\n");
  
  // clear end of buffer to prevent no overreading on damaged streams
  memset(inbuf + INBUF_SIZE, 0, AV_INPUT_BUFFER_PADDING_SIZE);
  
  exit_if(argc < 2,
    "Please supply a filepath as the second argument.\neg: ./tutorial [filename]\n");
  
  exit_if(avformat_open_input( &p_format_ctx, argv[1], NULL, NULL ) != 0,
    "Could not open the file.\n");

  exit_if( avformat_find_stream_info(p_format_ctx, NULL) < 0,
    "Could not stream the file.\n");
  
  av_dump_format(p_format_ctx, 0, argv[1], 0);

  // find video stream
  for(int i = 0; i < p_format_ctx->nb_streams; i++) {
    if (p_format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      video_stream_ind = i;
      break;
    }
  }

  video = p_format_ctx->streams[video_stream_ind];
  exit_if(video_stream_ind == -1, "Didn't find video stream.\n");
 
  exit_if(avcodec_parameters_to_context(p_codec_ctx, video->codecpar) < 0,
    "Unable to convert to codec context.\n");
  
  p_codec = avcodec_find_decoder(p_codec_ctx->codec_id);
  exit_if(p_codec == NULL, "Unsupported codec!\n");

  p_parser = av_parser_init(p_codec->id);
  exit_if(p_parser == NULL, "Unable to initialize parser.\n");

  // TODO: open file and do stuff...
  
  
  
  printf("Finished program.\n");
  return 0;
}
