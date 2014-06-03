include_directories($(PROJECT_SOURCE_DIR)../graph $(PROJECT_SOURCE_DIR)../sources $(PROJECT_SOURCE_DIR)../utils)
add_library(decoders AC3Decoder.cpp BitmapDecoder.cpp DefaultDecoderFactory.cpp MPEGAudioDecoder.cpp MPEGVideoDecoder.cpp PCMDecoder.cpp)
