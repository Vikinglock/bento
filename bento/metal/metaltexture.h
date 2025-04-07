#ifndef METALTEXTURE_H
#define METALTEXTURE_H

#ifdef __OBJC__
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <Cocoa/Cocoa.h>
#include "metalcommon.h"
#endif

#include <string>

class MetalTexture {//i touched (:
public:
    MetalTexture();
    MetalTexture(const char* filepath);
#ifdef __OBJC__
    MetalTexture(id<MTLTexture> tex,id<MTLSamplerState> sampler);
#endif
    ~MetalTexture();
    void* getTexture();
    
#ifdef __OBJC__
    id<MTLSamplerState> getSampler();
    id<MTLTexture> texture;
    id<MTLSamplerState> sampler;
#endif
};


#endif