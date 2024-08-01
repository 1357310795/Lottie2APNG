#pragma once

#include "export_helper.h"
#include "api_result.h"
#include <vector>
#include "image.h"
#include <sstream>

class APNGASM_API ApngManager {
private:
    std::vector<Image*> img;
    unsigned int i;
    unsigned int first = 0;
    unsigned int loops = 0;
    int delay_num = 1;
    int delay_den = 10;
    int hs = 0;
    int vs = 0;
    int keep_palette = 0;
    int keep_coltype = 0;
    int deflate_method = 0;
    int iter = 15;
    int res = 0;
    int width = 0;
    int height = 0;
    int common_coltype = 6;
public:
    ApngManager();
    ~ApngManager();
    api_result SetSize(int width, int height);
    api_result SetLoops(int loops);
    api_result SetSkipFirst(int first);
    api_result SetFps(int num, int den);
    api_result SetOptimizationOptions(bool keep_palette, bool keep_coltype);
    api_result SetCompressionOptions(int deflate_method, int iter);
    api_result AddFrame(unsigned char* data, int count);
    api_result AddFrameFromFile(const char* filename);
    api_result Optimize();
    api_result Save(char* filename, int& cur, int& total);
    api_result SaveStream(std::stringstream ss);
};

extern "C" {
    APNGASM_API ApngManager* CreateManager();
    APNGASM_API api_result DestroyManager(ApngManager* inst);
    APNGASM_API api_result SetSize(ApngManager* inst, int width, int height);
    APNGASM_API api_result SetLoops(ApngManager* inst, int loops);
    APNGASM_API api_result SetSkipFirst(ApngManager* inst, int first);
    APNGASM_API api_result SetFps(ApngManager* inst, int num, int den);
    APNGASM_API api_result SetOptimizationOptions(ApngManager* inst, bool keep_palette, bool keep_coltype);
    APNGASM_API api_result SetCompressionOptions(ApngManager* inst, int deflate_method, int iter);
    APNGASM_API api_result AddFrame(ApngManager* inst, unsigned char* data, int count);
    APNGASM_API api_result AddFrameFromFile(ApngManager* inst, char* filename);
    APNGASM_API api_result Optimize(ApngManager* inst);
    APNGASM_API api_result Save(ApngManager* inst, char* filename, int& cur, int& total);
}