#import <CoreGraphics/CoreGraphics.h>
#import <Metal/Metal.h>

#include "metaltexture.h"

MetalTexture::MetalTexture(const char* filepath) {
    @autoreleasepool{
        texture = nullptr;

        CFURLRef url = CFURLCreateWithFileSystemPath(
            kCFAllocatorDefault, 
            CFStringCreateWithCString(kCFAllocatorDefault, filepath, kCFStringEncodingUTF8),
            kCFURLPOSIXPathStyle, 
            false
        );
        
        if (!url) {
            fprintf(stderr, "Failed to create URL from path: %s\n", filepath);
            return;
        }
        
        CGImageSourceRef imageSource = CGImageSourceCreateWithURL(url, nullptr);
        CFRelease(url);
        
        if (!imageSource) {
            fprintf(stderr, "Failed to create image source from path: %s\n", filepath);
            return;
        }
        
        CGImageRef cgImage = CGImageSourceCreateImageAtIndex(imageSource, 0, nullptr);
        CFRelease(imageSource);
        
        if (!cgImage) {
            fprintf(stderr, "Failed to create CGImage from path: %s\n", filepath);
            return;
        }
        
        size_t width = CGImageGetWidth(cgImage);
        size_t height = CGImageGetHeight(cgImage);
        
        CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
        NSUInteger bytesPerPixel = 4;
        NSUInteger bytesPerRow = bytesPerPixel * width;
        NSUInteger bitsPerComponent = 8;
        
        unsigned char* rawData = (unsigned char*)calloc(width * height * bytesPerPixel, sizeof(unsigned char));
        
        if (!rawData) {
            fprintf(stderr, "Failed to allocate memory for image data\n");
            CGImageRelease(cgImage);
            CGColorSpaceRelease(colorSpace);
            return;
        }
        
        CGContextRef context = CGBitmapContextCreate(rawData, 
                                                    width, 
                                                    height, 
                                                    bitsPerComponent, 
                                                    bytesPerRow, 
                                                    colorSpace, 
                                                    kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
        
        if (!context) {
            fprintf(stderr, "Failed to create bitmap context\n");
            free(rawData);
            CGImageRelease(cgImage);
            CGColorSpaceRelease(colorSpace);
            return;
        }
        
        CGColorSpaceRelease(colorSpace);
        CGContextTranslateCTM(context, 0, height);
        CGContextScaleCTM(context, 1.0, -1.0);
        CGContextDrawImage(context, CGRectMake(0, 0, width, height), cgImage);
        CGContextRelease(context);
        CGColorSpaceRelease(colorSpace);
        CGImageRelease(cgImage);
        
        id<MTLDevice> device = MTLCreateSystemDefaultDevice();
        
        if (!device) {
            fprintf(stderr, "Failed to create Metal device\n");
            free(rawData);
            return;
        }

        MTLTextureDescriptor* textureDescriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                                                                    width:width
                                                                                                    height:height
                                                                                                mipmapped:NO];
        
        texture = [device newTextureWithDescriptor:textureDescriptor];
        
        if (!texture) {
            fprintf(stderr, "Failed to create Metal texture\n");
            free(rawData);
            return;
        }
        
        MTLRegion region = MTLRegionMake2D(0, 0, width, height);
        [texture replaceRegion:region
                mipmapLevel:0
                    withBytes:rawData
                bytesPerRow:bytesPerRow];
        
        
        free(rawData);

        MTLSamplerDescriptor *samplerDescriptor = [[MTLSamplerDescriptor alloc] init];
        samplerDescriptor.sAddressMode = MTLSamplerAddressModeRepeat;
        samplerDescriptor.tAddressMode = MTLSamplerAddressModeRepeat;
        //samplerDescriptor.minFilter = MTLSamplerMinMagFilterLinear;
        //samplerDescriptor.magFilter = MTLSamplerMinMagFilterLinear;

        samplerDescriptor.minFilter = MTLSamplerMinMagFilterNearest;
        samplerDescriptor.magFilter = MTLSamplerMinMagFilterNearest;

        sampler = [device newSamplerStateWithDescriptor:samplerDescriptor];
    }
}

MetalTexture::MetalTexture(id<MTLTexture> tex,id<MTLSamplerState> samp) {
    texture = [tex retain];
    sampler = [samp retain];
}

MetalTexture::MetalTexture() {
    texture = nil;
    sampler = nil;
}

MetalTexture::~MetalTexture() {
    [texture release];
    [sampler release];
}

void* MetalTexture::getTexture() {
    @autoreleasepool{
        return texture ? (__bridge void*)texture : nil;
    }
}
id<MTLSamplerState> MetalTexture::getSampler() {
    @autoreleasepool{
        return sampler ? sampler : nil;
    }
}