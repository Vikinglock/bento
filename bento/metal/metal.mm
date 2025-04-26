#include "../lib/glm/glm.hpp"
#include "../lib/glm/gtc/matrix_transform.hpp"
#include "../lib/glm/gtc/type_ptr.hpp"
#include "../lib/hidapi/hidapi.h"
#import "metal.h"
#import "metalcommon.h"

#import <Cocoa/Cocoa.h>
#import <GameController/GameController.h>

//ay yo i'm gonna leave blending as it is
//because for my own use it's gonna be about as primitive as it is here
//improve upon it if you wish but i'm gonna fix it later

ALCdevice* aldevice = nullptr;
ALCcontext* context = nullptr;
std::vector<ALuint> sounds;
std::vector<ALuint> buffers;

id<MTLDevice> device = nil;
bool fullscreenable = true;

bool useDefShader = true;

std::string vertShaderSourceC = "";
std::string fragShaderSourceC = "";
id<MTLRenderPipelineState> defaultPipelineState = nil;
Shader* shader;
Shader* defaultShader;

int startAtt;
int endAtt;
int startTex;
int endTex;
int depthInd;

std::vector<id<MTLTexture>> outTexture;

std::vector<id<MTLTexture>> texture;
std::vector<id<MTLSamplerState>> sampler;

std::vector<id<MTLTexture>> depthTextures;

glm::vec4 clearColor = glm::vec4(0.0,0.0,0.0,1.0);

struct controller{
    hid_device *hiddevice;
    std::vector<unsigned char> buffer;
    controller(hid_device* device) 
        : hiddevice(device), buffer() {}
};

@interface MetalRendererObjC : NSResponder
@property (nonatomic, strong) id<MTLDevice> device;
@property (nonatomic, strong) id<MTLCommandQueue> commandQueue;
@property (nonatomic, strong) id<MTLRenderPipelineState> pipelineState;
@property (nonatomic, strong) id<MTLBuffer> vertexBuffer;
@property (nonatomic, strong) id<MTLBuffer> normalBuffer;
@property (nonatomic, strong) id<MTLBuffer> uvBuffer;
@property (nonatomic, strong) CAMetalLayer *metalLayer;
@property (nonatomic, strong) NSWindow *window;
@property (nonatomic, strong) NSApplication *app;
@property (nonatomic, assign) BOOL shouldClose;
@property (nonatomic, strong) id<MTLDepthStencilState> depthStencilState;
@property (nonatomic, strong) id<MTLDepthStencilState> noDepthStencilState;
@property (nonatomic, strong) id<MTLDepthStencilState> yesDepthStencilState;
@property (nonatomic, strong) id<MTLTexture> appTexture;
@property (nonatomic, strong) NSMutableDictionary<NSNumber *, NSNumber *> *keyStates;
@property (nonatomic, strong) NSMutableDictionary<NSNumber *, NSNumber *> *mouseStates;
@property (nonatomic, strong) NSMutableDictionary<NSNumber *, NSNumber *> *prevKeyStates;
@property (nonatomic, strong) NSMutableDictionary<NSNumber *, NSNumber *> *prevMouseStates;
@property (nonatomic, strong) id<CAMetalDrawable> drawable;
@property (nonatomic, strong) id<MTLCommandBuffer> commandBuffer;
@property (nonatomic, strong) id<MTLRenderCommandEncoder> commandEncoder;
@property (nonatomic, strong) NSMutableArray *controllers;
@property (nonatomic, strong) MTLRenderPassDescriptor *passDescriptor;
@property (nonatomic, strong) id<MTLTexture> depthTexture;
@property (nonatomic, strong) id<MTLTexture> depthTTexture;
@property (nonatomic, strong) id<MTLSamplerState> rTSampler;
@property (nonatomic) int lastWidth;
@property (nonatomic) int lastHeight;
@property (nonatomic) glm::vec3 pos;
@property (nonatomic) double wheelX;
@property (nonatomic) double wheelY;
@property (nonatomic) NSInteger vertCount;
@property (nonatomic) NSInteger normCount;
@property (nonatomic) NSInteger uvCount;
@property (nonatomic) bool fullscreenable;
- (void)initRenderer:(const char *)title width:(int)width height:(int)height x:(int)x y:(int)y;
- (void)render;
- (BOOL)isRunning;
@end

@implementation MetalRendererObjC

//they're all indented for vsc

// #### MAIN ####

    - (void)initRenderer:(const char *)title width:(int)width height:(int)height x:(int)x y:(int)y {
        self.device = MTLCreateSystemDefaultDevice();
        device = MTLCreateSystemDefaultDevice();
        self.commandQueue = [self.device newCommandQueue];
        self.shouldClose = NO;

        MTLSamplerDescriptor *samplerDescriptor = [[MTLSamplerDescriptor alloc] init];
        samplerDescriptor.sAddressMode = MTLSamplerAddressModeRepeat;
        samplerDescriptor.tAddressMode = MTLSamplerAddressModeRepeat;
        samplerDescriptor.minFilter = MTLSamplerMinMagFilterNearest;
        samplerDescriptor.magFilter = MTLSamplerMinMagFilterNearest;
        self.rTSampler = [device newSamplerStateWithDescriptor:samplerDescriptor];

        self.app = [NSApplication sharedApplication];
        [self.app setActivationPolicy:NSApplicationActivationPolicyRegular];

        NSRect frame = NSMakeRect(0,0, width, height);
        NSUInteger windowStyle = (NSWindowStyleMaskTitled |
                                NSWindowStyleMaskClosable |
                                NSWindowStyleMaskResizable |
                                NSWindowStyleMaskMiniaturizable);
        self.window = [[NSWindow alloc] initWithContentRect:frame
                                                styleMask:windowStyle
                                                    backing:NSBackingStoreBuffered
                                                    defer:NO];


        [self.window setFrameOrigin:NSMakePoint(x,([NSScreen mainScreen].frame.size.height-self.window.frame.size.height)-y)];

        [self.window setTitle:@(title)];

        [self.window setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];


        [[NSNotificationCenter defaultCenter] addObserver:self
                                                selector:@selector(windowDidResize:)
                                                    name:NSWindowDidResizeNotification
                                                object:self.window];//why did i get rid of this???
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                selector:@selector(windowWillClose:)
                                                    name:NSWindowWillCloseNotification
                                                object:self.window];


        self.keyStates = [NSMutableDictionary dictionary];
        self.mouseStates = [NSMutableDictionary dictionary];
        self.prevKeyStates = [NSMutableDictionary dictionary];
        self.prevMouseStates = [NSMutableDictionary dictionary];
        self.metalLayer = [CAMetalLayer layer];
        self.metalLayer.device = self.device;
        self.metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
        self.metalLayer.contentsScale = [NSScreen mainScreen].backingScaleFactor;
        self.metalLayer.framebufferOnly = YES;
        [self.metalLayer setFrame:frame];
        [self.window.contentView setLayer:self.metalLayer];
        [self.window.contentView setWantsLayer:YES];
        
        NSString *vertMetalPath = @"./bento/shaders/shader.vsmetal";
        NSString *fragMetalPath = @"./bento/shaders/shader.fsmetal";
        NSError *error = nil;

        NSString *vertShaderSource = [NSString stringWithContentsOfFile:vertMetalPath encoding:NSUTF8StringEncoding error:&error];
        NSString *fragShaderSource = [NSString stringWithContentsOfFile:fragMetalPath encoding:NSUTF8StringEncoding error:&error];
        if (error) {
            NSLog(@"could not load shader file: %@", error);
            return;
        }

        id<MTLLibrary> vertLibrary = [self.device newLibraryWithSource:vertShaderSource options:nil error:&error];
        if (!vertLibrary) {
            NSLog(@"could not create vertex metal shader library: %@", error);
            return;
        }

        id<MTLLibrary> fragLibrary = [self.device newLibraryWithSource:fragShaderSource options:nil error:&error];
        if (!fragLibrary) {
            NSLog(@"could not create fragment metal shader library: %@", error);
            return;
        }
        
        vertShaderSourceC = std::string([vertShaderSource UTF8String]);
        fragShaderSourceC = std::string([fragShaderSource UTF8String]);

        id<MTLFunction> vertexFunction = [vertLibrary newFunctionWithName:@"main0"];
        id<MTLFunction> fragmentFunction = [fragLibrary newFunctionWithName:@"main0"];

        MTLVertexDescriptor *vertexDescriptor = [[MTLVertexDescriptor alloc] init];

        vertexDescriptor.attributes[0].format = MTLVertexFormatFloat3;
        vertexDescriptor.attributes[0].offset = 0;
        vertexDescriptor.attributes[0].bufferIndex = 0;
        vertexDescriptor.attributes[1].format = MTLVertexFormatFloat3;
        vertexDescriptor.attributes[1].offset = 0;
        vertexDescriptor.attributes[1].bufferIndex = 1;
        vertexDescriptor.attributes[2].format = MTLVertexFormatFloat2;
        vertexDescriptor.attributes[2].offset = 0;
        vertexDescriptor.attributes[2].bufferIndex = 2;
        vertexDescriptor.layouts[0].stride = sizeof(float) * 3;
        vertexDescriptor.layouts[1].stride = sizeof(float) * 3;
        vertexDescriptor.layouts[2].stride = sizeof(float) * 2;
        vertexDescriptor.layouts[0].stepRate = 1;
        vertexDescriptor.layouts[1].stepRate = 1;
        vertexDescriptor.layouts[2].stepRate = 1;
        vertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
        vertexDescriptor.layouts[1].stepFunction = MTLVertexStepFunctionPerVertex;
        vertexDescriptor.layouts[2].stepFunction = MTLVertexStepFunctionPerVertex;


        MTLRenderPipelineDescriptor* pipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
        pipelineDescriptor.vertexFunction = vertexFunction;
        pipelineDescriptor.fragmentFunction = fragmentFunction;

        std::regex colorRegex("\\[\\[color\\((\\d+)\\)\\]\\]");
        std::smatch match;
        int highest = 0;
        std::string::const_iterator start(fragShaderSourceC.cbegin());
        while (std::regex_search(start, fragShaderSourceC.cend(), match, colorRegex)) {
            if (match.size()>1)highest = std::max(highest,std::stoi(match[1].str()));
            start = match.suffix().first;
        }        
        pipelineDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatR32Float;
        for(int i = 0; i <= highest; i++)pipelineDescriptor.colorAttachments[i+1].pixelFormat = MTLPixelFormatRGBA32Float;

        pipelineDescriptor.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;
        pipelineDescriptor.vertexDescriptor = vertexDescriptor;

        self.pipelineState = [self.device newRenderPipelineStateWithDescriptor:pipelineDescriptor error:&error];
        if (!self.pipelineState) {
            NSLog(@"could not create pipeline state: %@", error);
            return;
        }
        defaultPipelineState = self.pipelineState;
        defaultShader = new Shader(defaultPipelineState,vertShaderSourceC,fragShaderSourceC);
        shader = defaultShader;

        MTLTextureDescriptor *depthTextureDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
                                                                                                width:width
                                                                                                height:height
                                                                                            mipmapped:NO];
                                                                                            
        depthTextureDesc.usage = MTLTextureUsageRenderTarget;
        self.depthTexture = [self.device newTextureWithDescriptor:depthTextureDesc];
        
        MTLDepthStencilDescriptor *depthStencilDesc = [[MTLDepthStencilDescriptor alloc] init];
        depthStencilDesc.depthCompareFunction = MTLCompareFunctionLess;
        depthStencilDesc.depthWriteEnabled = YES;
        self.yesDepthStencilState = [self.device newDepthStencilStateWithDescriptor:depthStencilDesc];
        depthStencilDesc = [[MTLDepthStencilDescriptor alloc] init];
        depthStencilDesc.depthCompareFunction = MTLCompareFunctionLess;
        depthStencilDesc.depthWriteEnabled = NO;
        self.noDepthStencilState = [self.device newDepthStencilStateWithDescriptor:depthStencilDesc];

        self.depthStencilState = self.noDepthStencilState;

        MTLTextureDescriptor *appTextureDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm
                                                                                                    width:width
                                                                                                height:height
                                                                                                mipmapped:NO];
        appTextureDesc.storageMode = MTLStorageModePrivate;
        appTextureDesc.usage = MTLTextureUsageRenderTarget;

        self.appTexture = [self.device newTextureWithDescriptor:appTextureDesc];



        id<CAMetalDrawable> drawable = [self.metalLayer nextDrawable];
        if (!drawable) {
            return;
        }
        
        self.fullscreenable = true;
        
        self.controllers = [NSMutableArray array];

        //the based method (literally only works with the controller i coded it for)

        if (hid_init()) {
            std::cerr << "couldn't enable hid (it's fine if you're not planning on using a controller)" << std::endl;
            return;
        }
        struct hid_device_info* devices = hid_enumerate(0x0, 0x0);
        struct hid_device_info* current = devices;
        while (current) {
            if (current->manufacturer_string && current->product_string) {
                int usage = current->usage;
                int usage_page = current->usage_page;

                if (usage == 0x05 && usage_page == 0x01) {
                    hid_device* device = hid_open_path(current->path);

                    if (device) {
                        controller *foundController = new controller(device);
                        [self.controllers addObject:[NSValue valueWithPointer:foundController]];
                    }
                }
            }
            current = current->next;
        }
        hid_free_enumeration(devices);
        if (self.controllers.count == 0) hid_exit();
        [self controllerInput];

        [self.app finishLaunching];
        
        
        [NSEvent addLocalMonitorForEventsMatchingMask:NSEventMaskKeyDown handler:^NSEvent * _Nullable(NSEvent *event) {
            return nil;
        }];

        NSMenu *mainMenu = [[NSMenu alloc] init];
        NSMenuItem *appMenuItem = [[NSMenuItem alloc] initWithTitle:@"App" action:nil keyEquivalent:@""];
        [mainMenu addItem:appMenuItem];
        NSMenu *appMenu = [[NSMenu alloc] initWithTitle:@"App"];
        [appMenu addItemWithTitle:@"Quit" action:@selector(terminate:) keyEquivalent:@"q"];
        [appMenu addItemWithTitle:@"Fullscreen" action:@selector(toggleFullScreen:) keyEquivalent:@""];
        [appMenuItem setSubmenu:appMenu];

        [NSApp setMainMenu:mainMenu];
    }

    - (void)focus {
        [self.window setIsVisible:YES];
        [self.window orderFront:nil];
        dispatch_async(dispatch_get_main_queue(), ^{
            [self.window makeKeyAndOrderFront:nil];
            [NSApp activateIgnoringOtherApps:YES];
            [self.window makeKeyWindow];
            dispatch_async(dispatch_get_main_queue(), ^{
                [NSApp activateIgnoringOtherApps:YES];
                [self.window makeFirstResponder:self.window.contentView];
            });
        });
    }

    - (void)predraw {
        self.commandBuffer = [self.commandQueue commandBuffer];
        self.drawable = [self.metalLayer nextDrawable];
        if (!self.drawable) {
            return;
        }

        self.passDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
        self.passDescriptor.colorAttachments[0].texture = self.appTexture;
        self.passDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(clearColor.x,clearColor.y,clearColor.z,clearColor.w);
        self.passDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
        self.passDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;

        self.passDescriptor.depthAttachment.texture = self.depthTexture;
        self.passDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
        self.passDescriptor.depthAttachment.clearDepth = 1.0;
        self.passDescriptor.depthAttachment.storeAction = MTLStoreActionStore;
        self.commandEncoder = [self.commandBuffer renderCommandEncoderWithDescriptor:self.passDescriptor];
    }

    - (void)draw {
        [self.commandEncoder setCullMode:MTLCullModeFront];
        [self.commandEncoder setDepthStencilState:self.depthStencilState];
        [self.commandEncoder setVertexBuffer:self.vertexBuffer offset:0 atIndex:0];
        [self.commandEncoder setVertexBuffer:self.normalBuffer offset:0 atIndex:1];
        [self.commandEncoder setVertexBuffer:self.uvBuffer offset:0 atIndex:2];

        for(int i=0;i<texture.size();i++){
            [self.commandEncoder setFragmentTexture:texture[i] atIndex:i];
            [self.commandEncoder setFragmentSamplerState:sampler[i] atIndex:i];
        }

        [self.commandEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:self.vertCount];
    }
    - (void)render {
        [self.commandEncoder endEncoding];
        id<MTLBlitCommandEncoder> blitEncoder = [self.commandBuffer blitCommandEncoder];
        [blitEncoder copyFromTexture:self.appTexture
                 sourceSlice:0
                 sourceLevel:0
                sourceOrigin:MTLOriginMake(0, 0, 0)
                  sourceSize:MTLSizeMake(self.appTexture.width, self.appTexture.height, 1)//self.window.frame.size.width*self.window.backingScaleFactor
                   toTexture:self.drawable.texture
            destinationSlice:0
            destinationLevel:0
           destinationOrigin:MTLOriginMake(0, 0, 0)];
        [blitEncoder endEncoding];
        [self.commandBuffer presentDrawable:self.drawable];
        [self.commandBuffer commit];

        [self.prevKeyStates setDictionary:self.keyStates];
        [self.prevMouseStates setDictionary:self.mouseStates];

        self.wheelY = 0;
        self.wheelX = 0;
    }
    - (void)predrawTex:(int)width height:(int)height {
        self.commandBuffer = [self.commandQueue commandBuffer];
        self.passDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
        if (!depthTextures[depthInd] || depthTextures[depthInd].width != width || depthTextures[depthInd].height != height) {
            [depthTextures[depthInd] release];
            MTLTextureDescriptor *depthTextureDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatR32Float
                                                                                                        width:width
                                                                                                    height:height
                                                                                                    mipmapped:NO];
            depthTextureDesc.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
            depthTextures[depthInd] = [self.device newTextureWithDescriptor:depthTextureDesc];
        }
        self.passDescriptor.colorAttachments[0].texture = depthTextures[depthInd];
        self.passDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.0, 0.0, 0.0, 1.0);
        self.passDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
        self.passDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;

        for (int i = startTex; i < endTex; i++) {
            if (!outTexture[i] || outTexture[i].width != width || outTexture[i].height != height){
                @autoreleasepool {
                    [outTexture[i] release];
                    MTLTextureDescriptor *textureDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA32Float
                                                                                                        width:width
                                                                                                        height:height
                                                                                                    mipmapped:NO];
                    textureDesc.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
                    outTexture[i] = [self.device newTextureWithDescriptor:textureDesc];
                }
            }
            self.passDescriptor.colorAttachments[startAtt+(i-startTex)+1].texture = outTexture[i];
            self.passDescriptor.colorAttachments[startAtt+(i-startTex)+1].clearColor = MTLClearColorMake(clearColor.x, clearColor.y, clearColor.z, 1.0);
            self.passDescriptor.colorAttachments[startAtt+(i-startTex)+1].loadAction = MTLLoadActionClear;
            self.passDescriptor.colorAttachments[startAtt+(i-startTex)+1].storeAction = MTLStoreActionStore;
        }
        if (!self.depthTTexture || self.depthTTexture.width != width || self.depthTTexture.height != height) {
            [self.depthTTexture release];
            MTLTextureDescriptor *depthTextureDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
                                                                                                    width:width
                                                                                                    height:height
                                                                                                mipmapped:NO];
            depthTextureDesc.usage = MTLTextureUsageRenderTarget;
            depthTextureDesc.storageMode = MTLStorageModePrivate;
            self.depthTTexture = [self.device newTextureWithDescriptor:depthTextureDesc];
        }
        self.passDescriptor.depthAttachment.texture = self.depthTTexture;
        self.passDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
        self.passDescriptor.depthAttachment.clearDepth = 1.0;
        self.passDescriptor.depthAttachment.storeAction = MTLStoreActionStore;

        self.commandEncoder = [self.commandBuffer renderCommandEncoderWithDescriptor:self.passDescriptor];
    }


    - (void)setShader:(Shader*)shd{
        shader = shd;
        [self.commandEncoder setRenderPipelineState:(__bridge id<MTLRenderPipelineState>)shd->pipelineState];
        useDefShader = shd==defaultShader;
    }

    - (void)setUniform:(float*)data name:(std::string)name{
        memcpy((uint8_t*)((__bridge id<MTLBuffer>)shader->fragBuffer).contents+shader->uniformMapFrag[name],data,shader->sizeMapFrag[name]);
    }

    - (void)setVertexUniform:(float*)data name:(std::string)name{
        memcpy((uint8_t*)((__bridge id<MTLBuffer>)shader->vertBuffer).contents+shader->uniformMapVert[name],data,shader->sizeMapVert[name]);
    }

    - (void)setUniformInt:(int*)data name:(std::string)name{
        memcpy((uint8_t*)((__bridge id<MTLBuffer>)shader->fragBuffer).contents+shader->uniformMapFrag[name],data,shader->sizeMapFrag[name]);
    }
    - (void)setVertexUniformInt:(int*)data name:(std::string)name{
        memcpy((uint8_t*)((__bridge id<MTLBuffer>)shader->vertBuffer).contents+shader->uniformMapVert[name],data,shader->sizeMapVert[name]);
    }

    - (void)drawTex {
        /*if(useDefShader){
            uint8_t* fragPtr = (uint8_t*)((__bridge id<MTLBuffer>)shader->vertBuffer).contents;
            for (size_t i = 0; i < ((__bridge id<MTLBuffer>)shader->vertBuffer).length; i++){
                printf("%02X ",fragPtr[i]);
                if((i+1)%16==0)printf("\n");
            }
            printf("\n");
        }*/
        [self.commandEncoder setCullMode:MTLCullModeFront];
        [self.commandEncoder setDepthStencilState:self.depthStencilState];

        [self.commandEncoder setVertexBuffer:self.vertexBuffer offset:0 atIndex:0];
        [self.commandEncoder setVertexBuffer:self.normalBuffer offset:0 atIndex:1];
        [self.commandEncoder setVertexBuffer:self.uvBuffer offset:0 atIndex:2];
        
        [self.commandEncoder setFragmentBuffer:(__bridge id<MTLBuffer>)shader->fragBuffer offset:0 atIndex:0];
        //[self.commandEncoder setFragmentBytes:[(__bridge id<MTLBuffer>)shader->fragBuffer contents] length:[(__bridge id<MTLBuffer>)shader->fragBuffer length] atIndex:0];
        //use this if you wanna switch inbetween idk
        [self.commandEncoder setVertexBytes:[(__bridge id<MTLBuffer>)shader->vertBuffer contents] length:[(__bridge id<MTLBuffer>)shader->vertBuffer length] atIndex:3];

        for(int i=0;i<texture.size();i++){
            [self.commandEncoder setFragmentTexture:texture[i] atIndex:i];
            [self.commandEncoder setFragmentSamplerState:sampler[i] atIndex:i];
        }
        [self.commandEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:self.vertCount];
    }

    - (void)renderTex {
        [self.commandEncoder endEncoding];
        [self.commandBuffer commit];
    }

    - (void)renderTex:(Texture*)tex ind:(int)ind{
        [tex->texture release];
        [tex->sampler release];
        tex->texture = [outTexture[ind] retain];
        tex->sampler = [self.rTSampler retain];
    }

    - (void)renderDepthTex:(Texture*)tex ind:(int)ind{
        [tex->texture release];
        [tex->sampler release];
        tex->texture = [depthTextures[ind] retain];
        tex->sampler = [self.rTSampler retain];
    }

    - (void)renderTex:(Texture*)tex1 tex2:(Texture*)tex2 ind:(int)ind{
        [tex1->texture release];
        [tex1->sampler release];
        tex1->texture = [outTexture[ind] retain];
        tex1->sampler = [self.rTSampler retain];
        [tex2->texture release];
        [tex2->sampler release];
        tex2->texture = [outTexture[ind+1] retain];
        tex2->sampler = [self.rTSampler retain];
    } 

    - (void)renderTex:(Texture*)tex1 tex2:(Texture*)tex2 tex3:(Texture*)tex3 ind:(int)ind{
        [tex1->texture release];
        [tex1->sampler release];
        tex1->texture = [outTexture[ind] retain];
        tex1->sampler = [self.rTSampler retain];
        [tex2->texture release];
        [tex2->sampler release];
        tex2->texture = [outTexture[ind+1] retain];
        tex2->sampler = [self.rTSampler retain];
        [tex3->texture release];
        [tex3->sampler release];
        tex3->texture = [outTexture[ind+2] retain];
        tex3->sampler = [self.rTSampler retain];
    }

    - (void)windowDidResize:(NSNotification *)notification {
        NSWindow *window = notification.object;
        NSView *view = window.contentView;

        if (view.layer) {
            CAMetalLayer *metalLayer = (CAMetalLayer *)view.layer;

            CGSize drawableSize = [view convertSizeToBacking:view.frame.size];
            metalLayer.drawableSize = drawableSize;

            MTLTextureDescriptor *depthTextureDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
                                                                                                        width:drawableSize.width
                                                                                                    height:drawableSize.height
                                                                                                    mipmapped:NO];
            depthTextureDesc.storageMode = MTLStorageModePrivate;
            depthTextureDesc.usage = MTLTextureUsageRenderTarget;

            self.depthTexture = [self.device newTextureWithDescriptor:depthTextureDesc];

            MTLTextureDescriptor *appTextureDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm
                                                                                                        width:drawableSize.width
                                                                                                    height:drawableSize.height
                                                                                                    mipmapped:NO];
            appTextureDesc.storageMode = MTLStorageModePrivate;
            appTextureDesc.usage = MTLTextureUsageRenderTarget;

            self.appTexture = [self.device newTextureWithDescriptor:appTextureDesc];
        }
    }
    - (void)controllerInput {
        for (NSValue *controllerValue in self.controllers) {
            controller *ctrl = (controller *)[controllerValue pointerValue];
            unsigned char buffer[65];
            int bytesRead = hid_read(ctrl->hiddevice, buffer, sizeof(buffer));

            if (bytesRead < 0) {
                NSLog(@"Error reading from controller.");
                continue;
            }

            switch(buffer[3]){
                case 0x08: buffer[3] = 0x00; break;
                case 0x07: buffer[3] = 0x09; break;
                case 0x06: buffer[3] = 0x08; break;
                case 0x05: buffer[3] = 0x12; break;
                case 0x04: buffer[3] = 0x04; break;
                case 0x03: buffer[3] = 0x06; break;
                case 0x02: buffer[3] = 0x02; break;
                case 0x01: buffer[3] = 0x03; break;
                case 0x00: buffer[3] = 0x01; break;
            }

            ctrl->buffer.assign(buffer, buffer + bytesRead);
        }
    }
    - (void)exit {
        for (NSValue *controllerValue in self.controllers) {
            controller *ctrl = (controller *)[controllerValue pointerValue];
            if (ctrl->hiddevice) {
                hid_close(ctrl->hiddevice);
            }
            delete ctrl;
        }
        [self.controllers removeAllObjects];
        hid_exit();
    }

    - (BOOL)isRunning {
        return !self.shouldClose;
    }

    - (void)windowWillClose:(NSNotification *)notification {
        self.shouldClose = YES;
        [NSApp terminate:nil];
    }

    - (void)toggleFullScreen {
        if (([self.window styleMask] & NSWindowStyleMaskFullScreen) == NSWindowStyleMaskFullScreen) {
            [self.window toggleFullScreen:nil];
        } else {
            [self.window toggleFullScreen:nil];
        }
    }

// #### UNIFORMS ####
    - (void)setVerticesDirect:(const std::vector<glm::vec3>&)vertices {
        self.vertexBuffer = [self.device newBufferWithBytes:vertices.data()
                                                length:vertices.size() * sizeof(glm::vec3)
                                            options:MTLResourceStorageModeShared];
        [self.vertexBuffer release];
        self.vertCount = vertices.size();
    }

    - (void)setNormalsDirect:(const std::vector<glm::vec3>&)normals {
        self.normalBuffer = [self.device newBufferWithBytes:normals.data()
                                                length:normals.size() * sizeof(glm::vec3)
                                            options:MTLResourceStorageModeShared];
        [self.normalBuffer release];
        self.normCount = normals.size();
    }

    - (void)setUvsDirect:(const std::vector<glm::vec2>&)uvs {
        self.uvBuffer = [self.device newBufferWithBytes:uvs.data()
                                            length:uvs.size() * sizeof(glm::vec2)
                                        options:MTLResourceStorageModeShared];
        [self.uvBuffer release];
        self.uvCount = uvs.size();
    }
    - (void)setVertices:(id<MTLBuffer>)vertices count:(NSUInteger)count {
        self.vertexBuffer = vertices;
        self.vertCount = count;
    }
    - (void)setNormals:(id<MTLBuffer>)normals count:(NSUInteger)count {
        self.normalBuffer = normals;
        self.normCount = count;
    }
    - (void)setUvs:(id<MTLBuffer>)uvs count:(NSUInteger)count {
        self.uvBuffer = uvs;
        self.uvCount = count;
    }
    //idk at what point it went from pos amb dif spec const lin quad to pos const lin quad amb dif spec but whatever
    - (simd::float3)glmtosimd3:(glm::vec3)v3{return simd::float3{v3.x,v3.y,v3.z};}

// #### INPUT ####
    - (void)updateInput:(NSEvent *)event {
        if (event.type == NSEventTypeKeyDown || event.type == NSEventTypeKeyUp) {
            self.prevKeyStates[@(event.keyCode)] = self.keyStates[@(event.keyCode)] ?: @(NO);
            self.keyStates[@(event.keyCode)] = (event.type == NSEventTypeKeyDown)?@(YES):@(NO);
        }
        //command  0x10
        //alt/option  0x08
        //control  0x04
        //shift  0x02

        if (event.type == NSEventTypeFlagsChanged) {
            self.keyStates[@(0x38)] = ((event.modifierFlags & 0x2) != 0)?@(YES):@(NO);
            self.keyStates[@(0x39)] = ((event.modifierFlags & 0x4) != 0)?@(YES):@(NO);
            self.keyStates[@(0x3B)] = ((event.modifierFlags & 0x1) != 0)?@(YES):@(NO);
            self.keyStates[@(0x3C)] = ((event.modifierFlags & 0x2000) != 0)?@(YES):@(NO);
            self.keyStates[@(0x3A)] = ((event.modifierFlags & 0x20) != 0)?@(YES):@(NO);
            self.keyStates[@(0x3D)] = ((event.modifierFlags & 0x40) != 0)?@(YES):@(NO);
            self.keyStates[@(0x37)] = ((event.modifierFlags & 0x8) != 0)?@(YES):@(NO);
            self.keyStates[@(0x36)] = ((event.modifierFlags & 0x10) != 0)?@(YES):@(NO);
        }

        if (NSPointInRect([event locationInWindow], [self.window contentView].frame) || (event.type == NSEventTypeLeftMouseUp || event.type == NSEventTypeRightMouseUp || event.type == NSEventTypeOtherMouseUp)) {
            if (event.type == NSEventTypeLeftMouseDown || event.type == NSEventTypeLeftMouseUp) {
                self.prevMouseStates[@(0)] = self.mouseStates[@(0)] ?: @(0);
                self.mouseStates[@(0)] = @(event.type == NSEventTypeLeftMouseDown ? 1 : 0);
            } 
            else if (event.type == NSEventTypeRightMouseDown || event.type == NSEventTypeRightMouseUp) {
                self.prevMouseStates[@(0)] = self.mouseStates[@(0)] ?: @(0);
                self.mouseStates[@(1)] = @(event.type == NSEventTypeRightMouseDown ? 1 : 0);
            }
            else if (event.type == NSEventTypeOtherMouseDown || event.type == NSEventTypeOtherMouseUp) {
                self.prevMouseStates[@(0)] = self.mouseStates[@(0)] ?: @(0);
                self.mouseStates[@(event.buttonNumber)] = @(event.type == NSEventTypeOtherMouseDown ? 1 : 0);
            }
        }
        
        ImGuiIO &io = ImGui::GetIO();
        switch (event.type) {
            case NSEventTypeKeyUp:
            case NSEventTypeKeyDown: {
                bool isKeyDown = (event.type == NSEventTypeKeyDown);
                
                auto it = KeytoDearImguiKey.find(event.keyCode);
                if (it != KeytoDearImguiKey.end()) {
                    io.AddKeyEvent(it->second, isKeyDown);
                }

                //printf("%02x\n",event.keyCode);
                if (isKeyDown) {
                    NSString *characters = event.characters;
                    if (characters.length > 0 && event.type != NSEventTypeFlagsChanged) {
                        unichar c = [characters characterAtIndex:0];
                        if (c >= 33 && c != 127) {
                            NSEventModifierFlags flags = event.modifierFlags & NSEventModifierFlagDeviceIndependentFlagsMask;
                            if (!(flags & NSEventModifierFlagFunction)) {
                              io.AddInputCharactersUTF8(characters.UTF8String);
                            }
                        }
                    }
                }
                
                io.AddKeyEvent(ImGuiMod_Shift, (event.modifierFlags & NSEventModifierFlagShift) != 0);
                io.AddKeyEvent(ImGuiMod_Ctrl,  (event.modifierFlags & NSEventModifierFlagControl) != 0);
                io.AddKeyEvent(ImGuiMod_Alt,   (event.modifierFlags & NSEventModifierFlagOption) != 0);
                io.AddKeyEvent(ImGuiMod_Super, (event.modifierFlags & NSEventModifierFlagCommand) != 0);
                break;
            }

            case NSEventTypeFlagsChanged: {
                io.AddKeyEvent(ImGuiMod_Shift, (event.modifierFlags & NSEventModifierFlagShift) != 0);
                io.AddKeyEvent(ImGuiMod_Ctrl,  (event.modifierFlags & NSEventModifierFlagControl) != 0);
                io.AddKeyEvent(ImGuiMod_Alt,   (event.modifierFlags & NSEventModifierFlagOption) != 0);
                io.AddKeyEvent(ImGuiMod_Super, (event.modifierFlags & NSEventModifierFlagCommand) != 0);
                break;
            }

            case NSEventTypeLeftMouseDown:
            case NSEventTypeLeftMouseUp: {
                io.MouseDown[0] = (event.type == NSEventTypeLeftMouseDown);
                break;
            }
            case NSEventTypeRightMouseDown:
            case NSEventTypeRightMouseUp: {
                io.MouseDown[1] = (event.type == NSEventTypeRightMouseDown);
                break;
            }
            case NSEventTypeOtherMouseDown:
            case NSEventTypeOtherMouseUp: {
                io.MouseDown[2] = (event.type == NSEventTypeOtherMouseDown);
                break;
            }
            case NSEventTypeScrollWheel: {
                io.MouseWheel += event.scrollingDeltaY;
                io.MouseWheelH += event.scrollingDeltaX;
                self.wheelY = event.scrollingDeltaY;
                self.wheelX = event.scrollingDeltaX;
                break;
            }
            case NSEventTypeMouseMoved:
            case NSEventTypeLeftMouseDragged:
            case NSEventTypeRightMouseDragged:
            case NSEventTypeOtherMouseDragged: {
                NSPoint locationInWindow = [event locationInWindow];
                NSRect contentRect = [[[NSApp keyWindow] contentView] frame];
                io.MousePos = ImVec2(locationInWindow.x, contentRect.size.height - locationInWindow.y);
                break;
            }
            default:
                break;
        }
    }
    
    - (BOOL)isWindowFocused {
        return [self.window isKeyWindow];
    }
// #### MOUSE AND WINDOWS ####
    - (NSSize)getWindowSize {
        return NSMakeSize(self.window.contentView.frame.size.width, self.window.contentView.frame.size.height);
    }

    - (void)setMouseCursor:(int)cursorType hidden:(BOOL)hide {
        if (hide) {
            [NSCursor hide];
        } else {
            [NSCursor unhide];
            switch (cursorType) {
                case 0:
                    [[NSCursor arrowCursor] set];
                    break;
                case 1:
                    [[NSCursor crosshairCursor] set];
                    break;
                case 2:
                    [[NSCursor IBeamCursor] set];
                    break;
                case 3:
                    [[NSCursor pointingHandCursor] set];
                    break;
                default:
                    [[NSCursor arrowCursor] set];
                    break;
            }
        }
    }

    - (NSPoint)getWindowPos {
        NSRect contentViewFrame = [self.window contentView].frame;
        NSRect windowFrame = [self.window frame];

        return NSMakePoint(windowFrame.origin.x,windowFrame.origin.y);
    }

    - (void)setWindowPos:(NSPoint)pos {
        [self.window setFrameOrigin:pos];
    }

    - (NSSize)getDisplaySize {
        NSScreen *mainScreen = [NSScreen mainScreen];
        return [mainScreen frame].size;
    }

    - (NSPoint)getControllerAxis:(NSInteger)contId type:(NSInteger)type {
        if (contId >= self.controllers.count){return NSMakePoint(0,0);}
        controller *cont = (controller *)[self.controllers[contId] pointerValue];

        std::vector<unsigned char> buffer = cont->buffer;
        int x;
        int y;
        if(type==1){
            x = buffer[5];
            y = buffer[7];
        }else{
            x = buffer[9];
            y = buffer[11];
        }
        return NSMakePoint(x,y);
    }

    - (bool)getControllerButton:(NSInteger)contId button:(ButtonType)button{
        if (contId >= self.controllers.count){return false;}
        controller *cont = (controller *)[self.controllers[contId] pointerValue];

        std::vector<unsigned char> buffer = cont->buffer;
        switch (button) {
            case GAMEPAD_KEY_A:
                return buffer[1] & 0x02;
            case GAMEPAD_KEY_B:
                return buffer[1] & 0x01;
            case GAMEPAD_KEY_X:
                return buffer[1] & 0x08;
            case GAMEPAD_KEY_Y:
                return buffer[1] & 0x04;

            case GAMEPAD_KEY_R:
                return buffer[1] & 0x20;
            case GAMEPAD_KEY_R2:
                return buffer[1] & 0x80;
            case GAMEPAD_KEY_R3:
                return buffer[2] & 0x08;

            case GAMEPAD_KEY_L:
                return buffer[1] & 0x10;
            case GAMEPAD_KEY_L2:
                return buffer[1] & 0x40;
            case GAMEPAD_KEY_L3:
                return buffer[2] & 0x04;

            case GAMEPAD_KEY_START:
                return buffer[2] & 0x02;
            case GAMEPAD_KEY_SELECT:
                return buffer[2] & 0x01;
            case GAMEPAD_KEY_HOME:
                return buffer[2] & 0x10;
            case GAMEPAD_KEY_SCREENSHOT:
                return buffer[2] & 0x20;

            case GAMEPAD_KEY_UP:
                return buffer[3] & 0x01;
            case GAMEPAD_KEY_DOWN:
                return buffer[3] & 0x04;
            case GAMEPAD_KEY_LEFT:
                return buffer[3] & 0x08;
            case GAMEPAD_KEY_RIGHT:
                return buffer[3] & 0x02;
        }
        return false;
    }
    - (double) getScroll:(int)wheel {
        if(wheel == 0)return self.wheelY;
        if(wheel == 1)return self.wheelX;
        return 0;
    }
    - (id<MTLCommandBuffer>) getCommandBuffer{
        return self.commandBuffer;
    }
    - (id<MTLRenderCommandEncoder>) getCommandEncoder{
        return self.commandEncoder;
    }
    - (MTLRenderPassDescriptor *) getRenderPass{
        return self.passDescriptor;
    }
    - (id<MTLSamplerState>) getSampler{
        return self.rTSampler;
    }


    - (void) enableVSync:(bool)enabled{
        self.metalLayer.displaySyncEnabled = enabled?YES:NO;
    }

    - (void) enableDepthTesting:(bool)enabled{
        self.depthStencilState = enabled?self.yesDepthStencilState:self.noDepthStencilState;
    }

    - (void) hideTitle:(bool)enabled{
        if(enabled){
            NSWindowStyleMask mask = [self.window styleMask];
            mask |= NSWindowStyleMaskFullSizeContentView;
            [self.window setStyleMask:mask];

            [self.window setTitleVisibility:NSWindowTitleHidden];
            [self.window setTitlebarAppearsTransparent:YES];
        }else{
            NSWindowStyleMask mask = [self.window styleMask];
            mask |= NSWindowStyleMaskTitled;
            [self.window setStyleMask:mask];

            [self.window setTitleVisibility:NSWindowTitleVisible];
            [self.window setTitlebarAppearsTransparent:NO];
        }
    }

    - (void) hideBar:(bool)enabled{
        if(enabled){
            NSWindowStyleMask mask = [self.window styleMask];
            mask &= ~NSWindowStyleMaskTitled;
            mask |= NSWindowStyleMaskFullSizeContentView;

            [self.window setStyleMask:mask];
            [self.window setTitleVisibility:NSWindowTitleHidden];
            [self.window setTitlebarAppearsTransparent:YES];
        }else{
            NSWindowStyleMask mask = [self.window styleMask];
            mask |= NSWindowStyleMaskTitled;
            [self.window setStyleMask:mask];

            [self.window setTitleVisibility:NSWindowTitleVisible];
            [self.window setTitlebarAppearsTransparent:NO];
        }
    }

@end

// #### MAIN ####

    void Bento::init(const char *title, int w, int h, int x, int y) {
        MetalRendererObjC *renderer = [[MetalRendererObjC alloc] init];
        @autoreleasepool {
            [renderer initRenderer:title width:w height:h x:x y:y];
        }
        this->rendererObjC = (__bridge void*)renderer;
    }

    void Bento::initSound(){
        aldevice = alcOpenDevice(nullptr);
        context = alcCreateContext(aldevice, nullptr);
        if (!context)alcCloseDevice(aldevice);
        alcMakeContextCurrent(context);
        ALenum error = alGetError();
    }

    void Bento::setClearColor(glm::vec4 col){
        @autoreleasepool {
            clearColor = col;
        }
    }

    void Bento::predraw() {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            [renderer predraw];
            if(getKey(KEY_LEFT_CONTROL)&&getKey(KEY_LEFT_COMMAND)&&getKey(KEY_F)&&fullscreenable){
                toggleFullscreen();
                fullscreenable = false;
            }
            if(!getKey(KEY_F))fullscreenable = true;

            if(getKey(KEY_LEFT_COMMAND)&&getKey(KEY_Q)){
                exit();
            }
        }
    }

    void Bento::draw() {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            [renderer draw];
        }
    }

    void Bento::render() {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            [renderer render];
            [renderer controllerInput];
            NSEvent *event = nil;
            while ((event = [renderer.app nextEventMatchingMask:NSEventMaskAny
                                                    untilDate:nil
                                                        inMode:NSDefaultRunLoopMode
                                                        dequeue:YES])) {
                [renderer.app sendEvent:event];
                [renderer updateInput:event];
            }
            [renderer.app updateWindows];
            [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:1.0 / 144.0]];
        }
    }
    void Bento::predrawTex(int width,int height) {
        @autoreleasepool {
            MetalRendererObjC* renderer = (__bridge MetalRendererObjC*)this->rendererObjC;
            [renderer predrawTex:width height:height];
        }
    }

    void Bento::drawTex() {
        @autoreleasepool {
            MetalRendererObjC* renderer = (__bridge MetalRendererObjC*)this->rendererObjC;
            [renderer drawTex];
        }
    }

    void Bento::renderTex() {
        @autoreleasepool {
            MetalRendererObjC* renderer = (__bridge MetalRendererObjC*)this->rendererObjC;
            [renderer renderTex];
        }
    }

    void Bento::setActiveTextures(int start, int end){
        startTex = start;
        endTex = end+1;
        if(outTexture.size()<end+1)outTexture.resize(end+1);
    }
    void Bento::setActiveTextures(int ind){
        startTex = ind;
        endTex = ind+1;
        if(outTexture.size()<ind+1)outTexture.resize(ind+1);
    }

    void Bento::setActiveDepthTexture(int ind){
        depthInd = ind;
        if(depthTextures.size()<ind+1)depthTextures.resize(ind+1);
    }

    void Bento::setActiveAttachments(int start, int end){
        startAtt = start;
        endAtt = end+1;
    }
    void Bento::setActiveAttachments(int ind){
        startAtt = ind;
        endAtt = ind+1;
    }

    void Bento::renderToTex(Texture*& tex,int ind) {
        @autoreleasepool {
            if(!tex){
                tex = new Texture();
            }
            MetalRendererObjC* renderer = (__bridge MetalRendererObjC*)this->rendererObjC;
            [renderer renderTex:tex ind:ind];
        }
    }

    void Bento::renderDepthToTex(Texture*& tex,int ind){
        @autoreleasepool {
            if(!tex){
                tex = new Texture();
            }
            MetalRendererObjC* renderer = (__bridge MetalRendererObjC*)this->rendererObjC;
            [renderer renderDepthTex:tex ind:ind];
        }
    }

    void Bento::renderToTex(Texture*& tex1, Texture*& tex2,int ind) {
        @autoreleasepool {
            MetalRendererObjC* renderer = (__bridge MetalRendererObjC*)this->rendererObjC;
            [renderer renderTex:tex1 tex2:tex2 ind:ind];
        }
    }
    void Bento::renderToTex(Texture*& tex1, Texture*& tex2, Texture*& tex3,int ind) {
        @autoreleasepool {
            MetalRendererObjC* renderer = (__bridge MetalRendererObjC*)this->rendererObjC;
            [renderer renderTex:tex1 tex2:tex2 tex3:tex3 ind:ind];
        }
    }
    void Bento::normalizeTexture(int index,bool normalized){
        //metal textures aren't normalized
        //i made it like that and if isn't like that then metal blows up
        //i could fix it but i just wanna make games man i don't like this no more ):
    }
//return new Texture([renderer renderTex],[renderer getSampler]);
    void Bento::exit() {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            ImGui_ImplMetal_Shutdown();

            for(int i = 0; i < sounds.size(); i++){
                alSourceStop(sounds[i]);
                alDeleteSources(1, &sounds[i]);
                alDeleteBuffers(1, &buffers[i]);
            }

            alcMakeContextCurrent(nullptr);
            alcDestroyContext(context);
            alcCloseDevice(aldevice);

            [renderer exit];
            [[NSApplication sharedApplication] terminate:nil];
        }
    }

    bool Bento::isRunning() {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
          return [renderer isRunning];
        }
    }

    void Bento::toggleFullscreen(){
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            [renderer toggleFullScreen];
        }
    }

    void Bento::focus(){
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            [renderer focus];
        }
    }

    bool Bento::isWindowFocused() {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            return [renderer isWindowFocused];
        }
    }

// #### UNIFORMS ####

    void Bento::setVerticesDirect(const std::vector<glm::vec3>& vertices) {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)rendererObjC;
            [renderer setVerticesDirect:vertices];
        }
    }
    
    void Bento::setNormalsDirect(const std::vector<glm::vec3>& normals) {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)rendererObjC;
            [renderer setNormalsDirect:normals];
        }
    }
    
    void Bento::setUvsDirect(const std::vector<glm::vec2>& uvs) {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)rendererObjC;
            [renderer setUvsDirect:uvs];
        }
    }

    void Bento::setVertices(class vertexBuffer vs) {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            [renderer setVertices:(__bridge id<MTLBuffer>)vs.getBuffer() count:vs.size() * 3];
        }
    }

    void Bento::setNormals(class normalBuffer ns) {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            [renderer setNormals:(__bridge id<MTLBuffer>)ns.getBuffer() count:ns.size() * 3];
        }
    }

    void Bento::setUvs(class uvBuffer uvs) {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            [renderer setUvs:(__bridge id<MTLBuffer>)uvs.getBuffer() count:uvs.size() * 2];
        }
    }

    void Bento::bindTexture(Texture *tex, int index) {
        @autoreleasepool {
            if (texture.size() <= index) {
                texture.resize(index + 1);
                sampler.resize(index + 1);
            }
            if (tex) {
                texture[index] = (__bridge id<MTLTexture>)tex->getTexture();
                sampler[index] = (__bridge id<MTLSamplerState>)tex->getSampler();
            } else {
                texture[index] = nil;
                sampler[index] = nil;
            }
        }
    }

    void Bento::unbindTexture() {

    }
// #### INPUT ####
    bool Bento::getKey(int key) {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            return [renderer.keyStates[@(key)] integerValue];
        }
    }

    bool Bento::getMouse(int mouse) {
        @autoreleasepool {
            MetalRendererObjC* renderer = (__bridge MetalRendererObjC*)this->rendererObjC;
            return ([renderer.mouseStates[@(mouse)] integerValue]==1);
        }
    }

    bool Bento::getKeyDown(int key) {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            return ([renderer.keyStates[@(key)] integerValue] == 1 && [renderer.prevKeyStates[@(key)] integerValue]==0);
        }
    }

    bool Bento::getKeyUp(int key) {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            return ([renderer.keyStates[@(key)] integerValue] == 0 && [renderer.prevKeyStates[@(key)] integerValue] == 1);
        }
    }

    bool Bento::getMouseDown(int button) {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            return ([renderer.mouseStates[@(button)] integerValue] == 1 && [renderer.prevMouseStates[@(button)] integerValue] == 0);
        }
    }

    bool Bento::getMouseUp(int button) {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            return ([renderer.mouseStates[@(button)] integerValue] == 0 && [renderer.prevMouseStates[@(button)] integerValue] == 1);
        }
    }

    double Bento::getScroll(int wheel){
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            return [renderer getScroll:wheel];
        }
    }

// #### MOUSE AND WINDOWS ####
    glm::vec2 Bento::getWindowSize() {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            NSSize size = [renderer getWindowSize];
            return glm::vec2(size.width,size.height);
        }
    }

    glm::vec2 Bento::getWindowPos() {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            NSPoint pos = [renderer getWindowPos];
            glm::vec2 wSize = getWindowSize();
            glm::vec2 dSize = getDisplaySize();
            return glm::vec2(pos.x,dSize.y-(pos.y+wSize.y));
        }
    }

    void Bento::setMouseCursor(bool hide, int cursor) {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            [renderer setMouseCursor:cursor hidden:hide];
        }
    }

    glm::vec2 Bento::getMousePosition() {
        @autoreleasepool {
            glm::vec2 dsize = getDisplaySize();
            CGPoint mouseLocation = [NSEvent mouseLocation];
            return glm::vec2(mouseLocation.x, dsize.y-mouseLocation.y);
        }
    }

    void Bento::setMousePosition(glm::vec2 pos, bool needsFocus) {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            if ([renderer isWindowFocused]) {
                CGPoint position = NSMakePoint(pos.x,pos.y);
                CGEventRef moveEvent = CGEventCreateMouseEvent(NULL, kCGEventMouseMoved, position, kCGMouseButtonLeft);
                CGEventPost(kCGHIDEventTap, moveEvent);
                CFRelease(moveEvent);
            }
        }
    }


    void Bento::setWindowPos(glm::vec2 pos) {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            glm::vec2 wSize = getWindowSize();
            glm::vec2 dSize = getDisplaySize();
            [renderer setWindowPos:NSMakePoint(pos.x,dSize.y-(pos.y+wSize.y))];
        }
    }


    glm::vec2 Bento::getControllerAxis(int controller, JoystickType joystick) {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            NSPoint point = [renderer getControllerAxis:controller type:(joystick==GAMEPAD_JOYSTICK_LEFT?1:joystick==GAMEPAD_JOYSTICK_RIGHT?0:-1)];
            return glm::vec2(point.x/127.5,point.y/127.5);
        }
    }
    
    bool Bento::getControllerButton(int controller, ButtonType button){
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            return [renderer getControllerButton:controller button:button];
        }
    }

    glm::vec2 Bento::getDisplaySize() {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            NSSize size = [renderer getDisplaySize];
            return glm::vec2(size.width,size.height);
        }
    }



// #### BUFFERS ####

    void vertexBuffer::setBuffer(const std::vector<glm::vec3>& buf){
        @autoreleasepool {
            buffer = [device newBufferWithBytes:(float *)buf.data() length:sizeof(glm::vec3) * buf.size() options:MTLResourceStorageModeShared];
            count = buf.size()/3;
        }
    }
    void* vertexBuffer::getBuffer() {
        @autoreleasepool {
            return (__bridge void*)buffer;
        }
    }

    void normalBuffer::setBuffer(const std::vector<glm::vec3>& buf){
        @autoreleasepool {
            buffer = [device newBufferWithBytes:(float *)buf.data() length:sizeof(glm::vec3) * buf.size() options:MTLResourceStorageModeShared];
            count = buf.size()/3;
        }
    }
    void* normalBuffer::getBuffer() {
        @autoreleasepool {
            return (__bridge void*)buffer;
        }
    }

    void uvBuffer::setBuffer(const std::vector<glm::vec2>& buf){
        @autoreleasepool {
            buffer = [device newBufferWithBytes:(float *)buf.data() length:sizeof(glm::vec2) * buf.size() options:MTLResourceStorageModeShared];
            count = buf.size()/2;
        }
    }
    void* uvBuffer::getBuffer() {
        @autoreleasepool {
            return (__bridge void*)buffer;
        }
    }


// #### DEARIMGUI ####



    void Bento::initImgui() {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            IMGUI_CHECKVERSION();
            ImGuiContext* imguiContext = ImGui::CreateContext();
            ImGui::SetCurrentContext(imguiContext);
            ImGui_ImplMetal_Init(device);

            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        }
    }

    
    void Bento::imguiNewFrame() {
        @autoreleasepool {
            ImGuiIO& io = ImGui::GetIO();
            glm::vec2 size = getWindowSize();
            io.DisplaySize = ImVec2(size.x,size.y);
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            ImGui_ImplMetal_NewFrame([renderer getRenderPass]);
            ImGui::NewFrame();
        }
    }


    void Bento::imguiRender() {
        @autoreleasepool {
            ImGui::Render();
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(),[renderer getCommandBuffer],[renderer getCommandEncoder]);
        }
    }

// #### LIGHTING is gone ): ####
    

    std::string Bento::getFramework(){
        @autoreleasepool {
            return "Metal";
        }
    }

    void Bento::setShader(Shader* shd){
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            [renderer setShader:shd];
            shader = shd;
        }
    }

    Shader* Bento::getDefaultShader(){
        return defaultShader;
    }
    struct uniformData {
        std::unordered_map<std::string, int> uniformMap;
        std::unordered_map<std::string, int> sizeMap;
        int size;
    };
    uniformData extractUniforms(const std::string& shaderSource, bool isVertex = false) {
        uniformData uniforms;
        int currentOffset = 0;
        int totalSize = 0;
        std::regex structRegex(R"(struct\s+Uniforms\s*\{([^}]*)\};)");
        std::regex uniformRegex(R"(\s*(\w+)\s+(\w+)(\[\d+\])?;)");
        std::smatch structMatch;
        if (std::regex_search(shaderSource, structMatch, structRegex)) {
            std::string structBody = structMatch[1].str();
            std::sregex_iterator begin(structBody.begin(), structBody.end(), uniformRegex);
            std::sregex_iterator end;
            for (auto it = begin; it != end; ++it) {
                std::string type = (*it)[1].str();
                std::string name = (*it)[2].str();
                std::string arraySize = (*it)[3].str();
                int typeSize = 0;
                if(type=="int")typeSize=sizeof(int)*4;
                else if(type=="float")typeSize=sizeof(float);
                else if(type=="float2")typeSize=sizeof(float)*2;
                else if(type=="float3")typeSize=sizeof(float)*4;
                else if(type=="float4")typeSize=sizeof(float)*4;
                else if(type=="mat4")typeSize=sizeof(float)*16;
                else if(type=="double")typeSize=sizeof(double);
                else if(type=="double2")typeSize=sizeof(double)*4;
                else if(type=="double3")typeSize=sizeof(double)*4;
                else if(type=="double4")typeSize=sizeof(double)*4;
                int arrayCount = 1;
                if (!arraySize.empty()) {
                    std::regex arraySizeRegex(R"(\[(\d+)\])");
                    std::smatch arraySizeMatch;
                    if (std::regex_match(arraySize, arraySizeMatch, arraySizeRegex)) {
                        arrayCount = std::stoi(arraySizeMatch[1].str());
                    }
                }
                uniforms.uniformMap[name] = currentOffset;
                currentOffset += typeSize * arrayCount;
                uniforms.sizeMap[name] = typeSize * arrayCount;
                totalSize += typeSize * arrayCount;
            }
        }
        uniforms.size = totalSize;
        return uniforms;
    }

    std::string Bento::getUni(){
        @autoreleasepool {
            std::string out;
            int size = 0;
            uniformData uniforms = extractUniforms(vertShaderSourceC,true);

            out.append("VERTEX : ");
            out.append(std::to_string(uniforms.size));
            out.append("\n");
            for (const auto& [name, index] : uniforms.uniformMap) {
                out.append(name);
                out.append(" : ");
                out.append(std::to_string(index));
                out.append(" : ");
                out.append(std::to_string(uniforms.sizeMap[name]));
                out.append("\n");
            }
            uniforms = extractUniforms(fragShaderSourceC);
            out.append("FRAGMENT : ");
            out.append(std::to_string(uniforms.size));
            out.append("\n");
            for (const auto& [name, index] : uniforms.uniformMap) {
                out.append(name);
                out.append(" : ");
                out.append(std::to_string(index));
                out.append(" : ");
                out.append(std::to_string(uniforms.sizeMap[name]));
                out.append("\n");
            }
            return out;
        }
    }

    void Bento::setUniform(std::string uniformName, glm::vec3 value, bool onvertex) {
        @autoreleasepool {
            MetalRendererObjC* renderer = (__bridge MetalRendererObjC*)this->rendererObjC;
            if(onvertex){
                [renderer setVertexUniform:glm::value_ptr(value) name:uniformName];
            }else{
                [renderer setUniform:glm::value_ptr(value) name:uniformName];
            }
        }
    }

    void Bento::setUniform(std::string uniformName, glm::vec4 value, bool onvertex) {
        @autoreleasepool {
            MetalRendererObjC* renderer = (__bridge MetalRendererObjC*)this->rendererObjC;
            if(onvertex){
                [renderer setVertexUniform:glm::value_ptr(value) name:uniformName];
            }else{
                [renderer setUniform:glm::value_ptr(value) name:uniformName];
            }
        }
    }
    void Bento::setUniform(std::string uniformName, float value, bool onvertex){
        @autoreleasepool {
            float v = static_cast<float>(value);
            MetalRendererObjC* renderer = (__bridge MetalRendererObjC*)this->rendererObjC;
            if(onvertex){
                [renderer setVertexUniform:&v name:uniformName];
            }else{
                [renderer setUniform:&v name:uniformName];
            }
        }
    }
    void Bento::setUniform(std::string uniformName, int value, bool onvertex){
        @autoreleasepool {
            int v = static_cast<int>(value);
            MetalRendererObjC* renderer = (__bridge MetalRendererObjC*)this->rendererObjC;
            if(onvertex){
                [renderer setVertexUniformInt:&v name:uniformName];
            }else{
                [renderer setUniformInt:&v name:uniformName];
            }
        }
    }
    void Bento::setUniform(std::string uniformName, glm::vec2 value, bool onvertex){
        @autoreleasepool {
            MetalRendererObjC* renderer = (__bridge MetalRendererObjC*)this->rendererObjC;
            if(onvertex){
                [renderer setVertexUniform:glm::value_ptr(value) name:uniformName];
            }else{
                [renderer setUniform:glm::value_ptr(value) name:uniformName];
            }
        }
    }
    void Bento::setUniform(std::string uniformName, glm::mat4 value, bool onvertex){
        @autoreleasepool {
            MetalRendererObjC* renderer = (__bridge MetalRendererObjC*)this->rendererObjC;
            if(onvertex){
                [renderer setVertexUniform:glm::value_ptr(value) name:uniformName];
            }else{
                [renderer setUniform:glm::value_ptr(value) name:uniformName];
            }
        }
    }
    void Bento::setUniform(std::string uniformName, std::vector<glm::vec4>& value, bool onvertex) {
        @autoreleasepool {
            MetalRendererObjC* renderer = (__bridge MetalRendererObjC*)this->rendererObjC;
            if(onvertex){
                [renderer setVertexUniform:glm::value_ptr(value[0]) name:uniformName];
            }else{
                [renderer setUniform:glm::value_ptr(value[0]) name:uniformName];
            }
        }
    }
    void Bento::setUniform(std::string uniformName, std::vector<glm::vec3>& value, bool onvertex) {
        @autoreleasepool {
            glm::vec4 paddedValue[value.size()];
            for (int i = 0; i < value.size(); i++) {
                paddedValue[i] = glm::vec4(value[i],0.0);
            }
            MetalRendererObjC* renderer = (__bridge MetalRendererObjC*)this->rendererObjC;
            if(onvertex){
                [renderer setVertexUniform:glm::value_ptr(*paddedValue) name:uniformName];
            }else{
                [renderer setUniform:glm::value_ptr(*paddedValue) name:uniformName];
            }
        }

    }
    void Bento::setUniform(std::string uniformName, std::vector<float>& value, bool onvertex) {
        @autoreleasepool {
            glm::vec4 paddedValue[value.size()];
            for (int i = 0; i < value.size(); i++) {
                paddedValue[i] = glm::vec4(value[i],0.0,0.0,0.0);
            }
            MetalRendererObjC* renderer = (__bridge MetalRendererObjC*)this->rendererObjC;
            if(onvertex){
                [renderer setVertexUniform:glm::value_ptr(*paddedValue) name:uniformName];
            }else{
                [renderer setUniform:glm::value_ptr(*paddedValue) name:uniformName];
            }
        }
    }
    void Bento::setUniform(std::string uniformName, std::vector<int>& value, bool onvertex) {
        @autoreleasepool {
            MetalRendererObjC* renderer = (__bridge MetalRendererObjC*)this->rendererObjC;
            if(onvertex){
                [renderer setVertexUniformInt:value.data() name:uniformName];
            }else{
                [renderer setUniformInt:value.data() name:uniformName];
            }
        }
    }

    void Bento::setUniform(std::string uniformName, std::vector<glm::vec2>& value, bool onvertex) {
        @autoreleasepool {
            MetalRendererObjC* renderer = (__bridge MetalRendererObjC*)this->rendererObjC;
            if(onvertex){
                [renderer setVertexUniform:glm::value_ptr(value[0]) name:uniformName];
            }else{
                [renderer setUniform:glm::value_ptr(value[0]) name:uniformName];
            }
        }
    }

    void Bento::setUniform(std::string uniformName, std::vector<glm::mat4>& value, bool onvertex) {
        @autoreleasepool {
            MetalRendererObjC* renderer = (__bridge MetalRendererObjC*)this->rendererObjC;
            if(onvertex){
                [renderer setVertexUniform:glm::value_ptr(value[0]) name:uniformName];
            }else{
                [renderer setUniform:glm::value_ptr(value[0]) name:uniformName];
            }
        }
    }

    void Bento::setUniform(std::string uniformName, glm::vec4* value, int count, bool onvertex) {
        @autoreleasepool {
            MetalRendererObjC* renderer = (__bridge MetalRendererObjC*)this->rendererObjC;
            if(onvertex){
                [renderer setVertexUniform:glm::value_ptr(*value) name:uniformName];
            }else{
                [renderer setUniform:glm::value_ptr(*value) name:uniformName];
            }
        }
    }
    void Bento::setUniform(std::string uniformName, glm::vec3* value, int count, bool onvertex) {
        @autoreleasepool {
            glm::vec4 paddedValue[count];
            for (int i = 0; i < count; i++) {
                paddedValue[i] = glm::vec4(value[i],0.0);
            }
            MetalRendererObjC* renderer = (__bridge MetalRendererObjC*)this->rendererObjC;
            if(onvertex){
                [renderer setVertexUniform:glm::value_ptr(*paddedValue) name:uniformName];
            }else{
                [renderer setUniform:glm::value_ptr(*paddedValue) name:uniformName];
            }
        }

    }
    void Bento::setUniform(std::string uniformName, float* value, int count, bool onvertex) {
        @autoreleasepool {
            glm::vec4 paddedValue[count];
            for (int i = 0; i < count; i++) {
                paddedValue[i] = glm::vec4(value[i],0.0,0.0,0.0);
            }
            MetalRendererObjC* renderer = (__bridge MetalRendererObjC*)this->rendererObjC;
            if(onvertex){
                [renderer setVertexUniform:glm::value_ptr(*paddedValue) name:uniformName];
            }else{
                [renderer setUniform:glm::value_ptr(*paddedValue) name:uniformName];
            }
        }
    }

    void Bento::setUniform(std::string uniformName, int* value, int count, bool onvertex) {
        @autoreleasepool {
            MetalRendererObjC* renderer = (__bridge MetalRendererObjC*)this->rendererObjC;
            if(onvertex){
                [renderer setVertexUniformInt:value name:uniformName];
            }else{
                [renderer setUniformInt:value name:uniformName];
            }
        }
    }

    void Bento::setUniform(std::string uniformName, glm::vec2* value, int count, bool onvertex) {
        @autoreleasepool {
            MetalRendererObjC* renderer = (__bridge MetalRendererObjC*)this->rendererObjC;
            if(onvertex){
                [renderer setVertexUniform:glm::value_ptr(*value) name:uniformName];
            }else{
                [renderer setUniform:glm::value_ptr(*value) name:uniformName];
            }
        }
    }

    void Bento::setUniform(std::string uniformName, glm::mat4* value, int count, bool onvertex) {
        @autoreleasepool {
            MetalRendererObjC* renderer = (__bridge MetalRendererObjC*)this->rendererObjC;
            if(onvertex){
                [renderer setVertexUniform:glm::value_ptr(*value) name:uniformName];
            }else{
                [renderer setUniform:glm::value_ptr(*value) name:uniformName];
            }
        }
    }

    class ShaderImpl {
    public:

        id<MTLRenderPipelineState> pipelineState;
        id<MTLBuffer> fragBuffer;
        id<MTLBuffer> vertBuffer;
        std::string vertSource = "";
        std::string fragSource = "";
        std::unordered_map<std::string, int> uniformMapVert;
        std::unordered_map<std::string, int> sizeMapVert;
        std::unordered_map<std::string, int> uniformMapFrag;
        std::unordered_map<std::string, int> sizeMapFrag;
        
        ShaderImpl(std::string vertPath, std::string fragPath) {
            @autoreleasepool {
                std::cout << "compiling "+vertPath+" and "+fragPath;
                size_t lastSlash = vertPath.find_last_of("/\\");
                std::string vertDir = "";
                std::string vertFilename = "";
                if (lastSlash != std::string::npos) {
                    vertDir = vertPath.substr(0, lastSlash);
                    vertFilename = vertPath.substr(lastSlash, vertPath.rfind('.') - lastSlash);
                }
                lastSlash = fragPath.find_last_of("/\\");
                std::string fragDir = "";
                std::string fragFilename = "";
                if (lastSlash != std::string::npos) {
                    fragDir = fragPath.substr(0, lastSlash);
                    fragFilename = fragPath.substr(lastSlash, fragPath.rfind('.') - lastSlash);
                }
                #ifdef CONVERT // you need glslangvalidator and spirv-cross
                    system(("glslangValidator -V --quiet " + vertPath + " -o "+vertDir+vertFilename+".vert.spv").c_str());
                    system(("glslangValidator -V --quiet " + fragPath + " -o "+fragDir+fragFilename+".frag.spv").c_str());
                    system(("spirv-cross "+vertDir+vertFilename+".vert.spv --msl --output " +vertDir+"/cache"+vertFilename + ".vsmetal").c_str());
                    system(("spirv-cross "+fragDir+fragFilename+".frag.spv --msl --output " +fragDir+"/cache"+fragFilename + ".fsmetal").c_str());
                    system(("./bento/shaders/bindfix "+vertPath+" "+vertDir+"/cache"+vertFilename+".vsmetal ").c_str());
                    system(("rm "+vertDir+vertFilename+".vert.spv "+fragDir+fragFilename+".frag.spv").c_str());
                    std::cout << " to "+vertDir+"/cache"+vertFilename+".vsmetal and "+fragDir+"/cache"+fragFilename+".fsmetal\n";
                #endif

                NSString *vertMetalPath = [NSString stringWithUTF8String:(vertDir+"/cache"+vertFilename+".vsmetal").c_str()];
                NSString *fragMetalPath = [NSString stringWithUTF8String:(fragDir+"/cache"+fragFilename+".fsmetal").c_str()];
                NSError *error = nil;

                NSString *vertShaderSource = [NSString stringWithContentsOfFile:vertMetalPath encoding:NSUTF8StringEncoding error:&error];
                NSString *fragShaderSource = [NSString stringWithContentsOfFile:fragMetalPath encoding:NSUTF8StringEncoding error:&error];
                if (error) {
                    NSLog(@"could not load shader file: %@", error);
                    return;
                }

                id<MTLLibrary> vertLibrary = [device newLibraryWithSource:vertShaderSource options:nil error:&error];
                if (!vertLibrary) {
                    NSLog(@"could not create vertex metal shader library: %@", error);
                    return;
                }

                id<MTLLibrary> fragLibrary = [device newLibraryWithSource:fragShaderSource options:nil error:&error];
                if (!fragLibrary) {
                    NSLog(@"could not create fragment metal shader library: %@", error);
                    return;
                }
                
                
                vertSource = std::string([vertShaderSource UTF8String]);
                fragSource = std::string([fragShaderSource UTF8String]);

                id<MTLFunction> vertexFunction = [vertLibrary newFunctionWithName:@"main0"];
                id<MTLFunction> fragmentFunction = [fragLibrary newFunctionWithName:@"main0"];

                MTLVertexDescriptor *vertexDescriptor = [[MTLVertexDescriptor alloc] init];

                vertexDescriptor.attributes[0].format = MTLVertexFormatFloat3;
                vertexDescriptor.attributes[0].offset = 0;
                vertexDescriptor.attributes[0].bufferIndex = 0;
                vertexDescriptor.attributes[1].format = MTLVertexFormatFloat3;
                vertexDescriptor.attributes[1].offset = 0;
                vertexDescriptor.attributes[1].bufferIndex = 1;
                vertexDescriptor.attributes[2].format = MTLVertexFormatFloat2;
                vertexDescriptor.attributes[2].offset = 0;
                vertexDescriptor.attributes[2].bufferIndex = 2;
                vertexDescriptor.layouts[0].stride = sizeof(float) * 3;
                vertexDescriptor.layouts[1].stride = sizeof(float) * 3;
                vertexDescriptor.layouts[2].stride = sizeof(float) * 2;
                vertexDescriptor.layouts[0].stepRate = 1;
                vertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;


                MTLRenderPipelineDescriptor* pipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
                pipelineDescriptor.vertexFunction = vertexFunction;
                pipelineDescriptor.fragmentFunction = fragmentFunction;

                std::regex colorRegex("\\[\\[color\\((\\d+)\\)\\]\\]");
                std::smatch match;
                int highest = 0;
                std::string::const_iterator start(fragSource.cbegin());
                while (std::regex_search(start, fragSource.cend(), match, colorRegex)) {
                    if (match.size()>1)highest = std::max(highest,std::stoi(match[1].str()));
                    start = match.suffix().first;
                }
                pipelineDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatR32Float;
                
                for(int i = 0; i <= highest; i++){
                    pipelineDescriptor.colorAttachments[i+1].pixelFormat = MTLPixelFormatRGBA32Float;
                    pipelineDescriptor.colorAttachments[i+1].blendingEnabled = YES;
                    pipelineDescriptor.colorAttachments[i+1].rgbBlendOperation = MTLBlendOperationAdd;
                    pipelineDescriptor.colorAttachments[i+1].alphaBlendOperation = MTLBlendOperationAdd;
                    pipelineDescriptor.colorAttachments[i+1].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
                    pipelineDescriptor.colorAttachments[i+1].sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
                    pipelineDescriptor.colorAttachments[i+1].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
                    pipelineDescriptor.colorAttachments[i+1].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;

                }

                pipelineDescriptor.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;
                pipelineDescriptor.vertexDescriptor = vertexDescriptor;

                pipelineState = [device newRenderPipelineStateWithDescriptor:pipelineDescriptor error:&error];
                if (!pipelineState) {
                    NSLog(@"could not create pipeline state: %@", error);
                    return;
                }

                int currentOffset = 0;
                int totalSize = 0;
                std::regex inputStructRegex(R"(struct\s+main0_in\s*\{([^}]*)\};)");
                std::regex inputRegex(R"(\s*(\w+\d*)\s+(\w+)\s*\[\[.*?\]\];)");
                std::smatch inputStructMatch;
                if (std::regex_search(vertSource, inputStructMatch, inputStructRegex)) {
                    std::string inputStructBody = inputStructMatch[1].str();
                    std::sregex_iterator begin(inputStructBody.begin(), inputStructBody.end(), inputRegex);
                    std::sregex_iterator end;
                    for (auto it = begin; it != end; ++it) {
                        std::string type = (*it)[1].str();
                        std::string name = (*it)[2].str();
                    }
                }
                std::regex structRegex(R"(struct\s+Uniforms\s*\{([^}]*)\};)");
                std::regex uniformRegex(R"(\s*(\w+)\s+(\w+)(\[\d+\])?;)");
                std::smatch structMatch;
                if (std::regex_search(vertSource, structMatch, structRegex)) {
                    std::string structBody = structMatch[1].str();
                    std::sregex_iterator begin(structBody.begin(), structBody.end(), uniformRegex);
                    std::sregex_iterator end;
                    for (auto it = begin; it != end; ++it) {
                        std::string type = (*it)[1].str();
                        std::string name = (*it)[2].str();
                        std::string arraySize = (*it)[3].str();
                        int typeSize = 0;
                        if(type=="int")typeSize=sizeof(int);
                        else if(type=="float")typeSize=sizeof(float);
                        else if(type=="float2")typeSize=sizeof(float)*2;
                        else if(type=="float3")typeSize=sizeof(float)*4;
                        else if(type=="float4")typeSize=sizeof(float)*4;
                        else if(type=="packed_float")typeSize=sizeof(float);
                        else if(type=="packed_float2")typeSize=sizeof(float)*2;
                        else if(type=="packed_float3")typeSize=sizeof(float)*3;
                        else if(type=="packed_float4")typeSize=sizeof(float)*4;
                        else if(type=="float4x4")typeSize=sizeof(float)*16;
                        else if(type=="double")typeSize=sizeof(double);
                        else if(type=="double2")typeSize=sizeof(double)*4;
                        else if(type=="double3")typeSize=sizeof(double)*4;
                        else if(type=="double4")typeSize=sizeof(double)*4;
                        int arrayCount = 1;
                        if (!arraySize.empty()) {
                            std::regex arraySizeRegex(R"(\[(\d+)\])");
                            std::smatch arraySizeMatch;
                            if (std::regex_match(arraySize, arraySizeMatch, arraySizeRegex)) {
                                arrayCount = std::stoi(arraySizeMatch[1].str());
                            }
                        }
                        uniformMapVert[name] = currentOffset;
                        currentOffset += typeSize * arrayCount;
                        sizeMapVert[name] = typeSize * arrayCount;
                        totalSize += typeSize * arrayCount;
                    }
                }
                vertBuffer = [device newBufferWithLength:totalSize options:MTLResourceStorageModeShared];
                currentOffset = 0;
                totalSize = 0;
                if (std::regex_search(fragSource, structMatch, structRegex)) {
                    std::string structBody = structMatch[1].str();
                    std::sregex_iterator begin(structBody.begin(), structBody.end(), uniformRegex);
                    std::sregex_iterator end;
                    for (auto it = begin; it != end; ++it) {
                        std::string type = (*it)[1].str();
                        std::string name = (*it)[2].str();
                        std::string arraySize = (*it)[3].str();
                        int typeSize = 0;
                        if(type=="int")typeSize=sizeof(int);
                        else if(type=="float")typeSize=sizeof(float);
                        else if(type=="float2")typeSize=sizeof(float)*2;
                        else if(type=="float3")typeSize=sizeof(float)*4;
                        else if(type=="float4")typeSize=sizeof(float)*4;
                        else if(type=="packed_float")typeSize=sizeof(float);
                        else if(type=="packed_float2")typeSize=sizeof(float)*2;
                        else if(type=="packed_float3")typeSize=sizeof(float)*3;
                        else if(type=="packed_float4")typeSize=sizeof(float)*4;
                        else if(type=="float4x4")typeSize=sizeof(float)*16;
                        else if(type=="double")typeSize=sizeof(double);
                        else if(type=="double2")typeSize=sizeof(double)*4;
                        else if(type=="double3")typeSize=sizeof(double)*4;
                        else if(type=="double4")typeSize=sizeof(double)*4;
                        int arrayCount = 1;
                        if (!arraySize.empty()) {
                            std::regex arraySizeRegex(R"(\[(\d+)\])");
                            std::smatch arraySizeMatch;
                            if (std::regex_match(arraySize, arraySizeMatch, arraySizeRegex)) {
                                arrayCount = std::stoi(arraySizeMatch[1].str());
                            }
                        }
                        uniformMapFrag[name] = currentOffset;
                        currentOffset += typeSize * arrayCount;
                        sizeMapFrag[name] = typeSize * arrayCount;
                        totalSize += typeSize * arrayCount;
                    }
                }
                fragBuffer = [device newBufferWithLength:totalSize options:MTLResourceStorageModeShared];
            }
        }
    };


    Shader::Shader(const char* vertPath, const char* fragPath) {
        impl = new ShaderImpl(vertPath, fragPath);

        pipelineState = impl->pipelineState;
        fragBuffer = impl->fragBuffer;
        vertBuffer = impl->vertBuffer;
        vertSource = impl->vertSource;
        fragSource = impl->fragSource;
        uniformMapVert = impl->uniformMapVert;
        sizeMapVert = impl->sizeMapVert;
        uniformMapFrag = impl->uniformMapFrag;
        sizeMapFrag = impl->sizeMapFrag;
    }

    void Bento::enable(Feature f, bool enabled){
        @autoreleasepool {
            MetalRendererObjC* renderer = (__bridge MetalRendererObjC*)this->rendererObjC;
            switch(f){
                case 0:[renderer enableVSync:enabled];break;
                case 1:[renderer enableDepthTesting:enabled];break;
                case 2:[renderer hideTitle:enabled];break;
                case 3:[renderer hideBar:enabled];break;
            }
        }
    }