#include "apngmanager.h"
#include <sstream>
#include <fstream>
#include <iostream>

ApngManager::ApngManager() {

}

ApngManager::~ApngManager() {
    for (auto &i : this->img)
    {
        i->free();
        delete i;
    }
    this->img.clear();
}

api_result ApngManager::SetSize(int width, int height) {
    if (width <= 0 || height <= 0)
        return ApiResult(false, "size should be larger than zero");
    this->width = width;
    this->height = height;
    return ApiResult(true, "ok");
}

api_result ApngManager::SetLoops(int loops) {
    this->loops = loops;
    return ApiResult(true, "ok");
}

api_result ApngManager::SetSkipFirst(int first) {
    if (first >= 2 || first < 0)
        return ApiResult(false, "value should be 0 or 1");
    this->first = first;
    return ApiResult(true, "ok");
}

api_result ApngManager::SetFps(int num, int den) {
    if (num <= 0 || den <= 0)
        return ApiResult(false, "value should be positive");
    this->delay_num = num;
    this->delay_den = den;
    return ApiResult(true, "ok");
}

api_result ApngManager::SetOptimizationOptions(bool keep_palette, bool keep_coltype) {
    if (keep_palette >= 2 || keep_palette < 0 || keep_coltype >= 2 || keep_coltype < 0)
        return ApiResult(false, "value should be 0 or 1");
    this->keep_palette = keep_palette;
    this->keep_coltype = keep_coltype;
    return ApiResult(true, "ok");
}

api_result ApngManager::SetCompressionOptions(int deflate_method, int iter) {
    if (deflate_method > 2 || deflate_method < 0)
        return ApiResult(false, "deflate_method should be 0, 1 or 2");
    if (iter < 0)
        return ApiResult(false, "iter should be positive");
    this->deflate_method = deflate_method;
    this->iter = iter;
    return ApiResult(true, "ok");
}

api_result ApngManager::AddFrame(unsigned char* data, int count) {
    std::stringstream buffer;
    std::streampos lastpos = buffer.tellp();
    buffer.write((char*)data, count);

    Image* img = new Image();
    try {
        load_png_from_stream(&buffer, img);
    }
    catch(const std::exception &ex) {
        char err[256];
        sprintf_s(err, "load image failed: %s", ex.what());
        return ApiResult(false, err);
    }
    
    if (img->w != this->width || img->h != this->height)
    {
        return ApiResult(false, "different image size");
    }
    img->delay_num = this->delay_num;
    img->delay_den = this->delay_den;
    this->img.push_back(img);
    return ApiResult(true, "ok");
}

api_result ApngManager::AddFrameFromFile(const char* filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        return ApiResult(false, "Unable to open file!\n");
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    Image* img = new Image();
    try {
        load_png_from_stream(&buffer, img);
    }
    catch(const std::exception &ex) {
        char err[256];
        sprintf_s(err, "load image failed: %s", ex.what());
        return ApiResult(false, err);
    }

    if (img->w != this->width || img->h != this->height)
    {
      return ApiResult(false, "different image size (maybe you have loaded a non-image file)\n");
    }
    img->delay_num = this->delay_num;
    img->delay_den = this->delay_den;
    this->img.push_back(img);
    return ApiResult(true, "ok");
}

api_result ApngManager::Optimize() {
    common_coltype = find_common_coltype(this->img);

    for (i=0; i<img.size(); i++)
    {
        if (img[i]->type != common_coltype)
        optim_upconvert(img[i], common_coltype);
    }

    if (common_coltype == 6 || common_coltype == 4)
    {
        for (i=0; i<img.size(); i++)
        optim_dirty_transp(img[i]);
    }

    optim_duplicates(img, first);

    if (!keep_coltype)
        optim_downconvert(img);

    common_coltype = img[0]->type;

    if (common_coltype == 3 && !keep_palette)
        optim_palette(img);

    if (common_coltype == 2 || common_coltype == 0)
        optim_add_transp(img);
    return ApiResult(true, "ok");
}

api_result ApngManager::Save(char* filename, int& cur, int& total) {
    int res = save_apng_to_file(filename, this->img, this->loops, this->first, this->deflate_method, this->iter, cur, total);
    if (res == 0)
        return ApiResult(true, "ok");
    else
        return ApiResult(false, "save error");
}

/////////////////////////////////////////////////////////////////////////

APNGASM_API ApngManager* CreateManager() {
    return new ApngManager();
}

APNGASM_API api_result DestroyManager(ApngManager* inst) {
    if (inst) {
        delete inst;
        return ApiResult(true, "ok");
    }
    else {
        return ApiResult(false, "pointer is null");
    }
}

APNGASM_API api_result SetSize(ApngManager* inst, int width, int height) {
    return inst->SetSize(width, height);
}

APNGASM_API api_result SetLoops(ApngManager* inst, int loops) {
    return inst->SetLoops(loops);
}

APNGASM_API api_result SetSkipFirst(ApngManager* inst, int first) {
    return inst->SetSkipFirst(first);
}

APNGASM_API api_result SetFps(ApngManager* inst, int num, int den) {
    return inst->SetFps(num, den);
}

APNGASM_API api_result SetOptimizationOptions(ApngManager* inst, bool keep_palette, bool keep_coltype) {
    return inst->SetOptimizationOptions(keep_palette, keep_coltype);
}

APNGASM_API api_result SetCompressionOptions(ApngManager* inst, int deflate_method, int iter) {
    return inst->SetCompressionOptions(deflate_method, iter);
}
    
APNGASM_API api_result AddFrame(ApngManager* inst, unsigned char* data, int count) {
    return inst->AddFrame(data, count);
}

APNGASM_API api_result AddFrameFromFile(ApngManager* inst, char* filename) {
    return inst->AddFrameFromFile(filename);
}

APNGASM_API api_result Optimize(ApngManager* inst) {
    return inst->Optimize();
}

APNGASM_API api_result Save(ApngManager* inst, char* filename, int& cur, int& total) {
    return inst->Save(filename, cur, total);
}