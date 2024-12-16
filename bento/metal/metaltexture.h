#ifndef METALTEXTURE_H
#define METALTEXTURE_H

#ifdef __OBJC__
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <Cocoa/Cocoa.h>
#include "metalcommon.h"
#endif

#include <string>

class MetalTexture {//it worked like this; never touch again
public:
    MetalTexture(const char* filepath);
    ~MetalTexture();
    
#ifdef __OBJC__
    id<MTLTexture> getTexture();
    id<MTLSamplerState> getSampler();
private:
    id<MTLTexture> texture;
    id<MTLSamplerState> sampler;
#endif
};


#endif