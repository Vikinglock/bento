#include "../lib/glm/glm.hpp"
#include "../lib/glm/gtc/matrix_transform.hpp"
#include "../lib/glm/gtc/type_ptr.hpp"
#include "../lib/hidapi/hidapi.h"
#import "metal.h"
#import "metalcommon.h"

#import <Cocoa/Cocoa.h>
#import <GameController/GameController.h>


ALCdevice* aldevice = nullptr;
ALCcontext* context = nullptr;
std::vector<ALuint> sounds;
std::vector<ALuint> buffers;

id<MTLDevice> device = nil;
bool fullscreenable = true;

simd::float3 positions[MAX_LIGHTS];
simd::float4 constants[MAX_LIGHTS];
simd::float4 linears[MAX_LIGHTS];
simd::float4 quads[MAX_LIGHTS];
simd::float3 ambients[MAX_LIGHTS];
simd::float3 diffuses[MAX_LIGHTS];
simd::float3 speculars[MAX_LIGHTS];

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
@property (nonatomic, strong) id<MTLTexture> depthTexture;
@property (nonatomic, strong) id<MTLTexture> appTexture;
@property (nonatomic, strong) NSMutableDictionary<NSNumber *, NSNumber *> *keyStates;
@property (nonatomic, strong) NSMutableDictionary<NSNumber *, NSNumber *> *mouseStates;
@property (nonatomic, strong) id<MTLTexture> texture;
@property (nonatomic, strong) id<MTLSamplerState> sampler;
@property (nonatomic, strong) id<CAMetalDrawable> drawable;
@property (nonatomic, strong) id<MTLCommandBuffer> commandBuffer;
@property (nonatomic, strong) id<MTLRenderCommandEncoder> commandEncoder;
@property (nonatomic, strong) NSMutableArray *controllers;
@property (nonatomic, strong) MTLRenderPassDescriptor *passDescriptor;
@property (nonatomic, strong) id<MTLTexture> renderTexture;
@property (nonatomic, strong) id<MTLTexture> depthTTexture;
@property (nonatomic) int numLights;
@property (nonatomic) glm::vec3 pos;
@property (nonatomic) double wheelX;
@property (nonatomic) double wheelY;
@property (nonatomic) NSInteger vertCount;
@property (nonatomic) NSInteger normCount;
@property (nonatomic) NSInteger uvCount;
@property (nonatomic) float *model;
@property (nonatomic) float *view;
@property (nonatomic) float *projection;
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

        self.model = (float *)malloc(sizeof(float) * 16);
        self.view = (float *)malloc(sizeof(float) * 16);
        self.projection = (float *)malloc(sizeof(float) * 16);
        self.numLights = 0;

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
        self.metalLayer = [CAMetalLayer layer];
        self.metalLayer.device = self.device;
        self.metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
        self.metalLayer.contentsScale = [NSScreen mainScreen].backingScaleFactor;
        self.metalLayer.framebufferOnly = YES;
        [self.metalLayer setFrame:frame];
        [self.window.contentView setLayer:self.metalLayer];
        [self.window.contentView setWantsLayer:YES];

        //[self toggleFullScreen];

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
        pipelineDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
        pipelineDescriptor.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;
        pipelineDescriptor.vertexDescriptor = vertexDescriptor;

        self.pipelineState = [self.device newRenderPipelineStateWithDescriptor:pipelineDescriptor error:&error];
        if (!self.pipelineState) {
            NSLog(@"could not create pipeline state: %@", error);
            return;
        }

        MTLTextureDescriptor *depthTextureDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
                                                                                                width:width
                                                                                                height:height
                                                                                            mipmapped:NO];
                                                                                            
        depthTextureDesc.usage = MTLTextureUsageRenderTarget;
        self.depthTexture = [self.device newTextureWithDescriptor:depthTextureDesc];
        
        MTLDepthStencilDescriptor *depthStencilDesc = [[MTLDepthStencilDescriptor alloc] init];
        depthStencilDesc.depthCompareFunction = MTLCompareFunctionLess;
        depthStencilDesc.depthWriteEnabled = YES;
        self.depthStencilState = [self.device newDepthStencilStateWithDescriptor:depthStencilDesc];

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
        

        self.passDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
        self.passDescriptor.colorAttachments[0].texture = self.appTexture;
        self.passDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
        self.passDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(clearColor.x,clearColor.y,clearColor.z,clearColor.w);
        self.passDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;

        self.passDescriptor.depthAttachment.texture = self.depthTexture;
        self.passDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
        self.passDescriptor.depthAttachment.clearDepth = 1.0;
        self.passDescriptor.depthAttachment.storeAction = MTLStoreActionDontCare;


        id<MTLCommandBuffer> commandBuffer = [self.commandQueue commandBuffer];
        self.commandEncoder = [commandBuffer renderCommandEncoderWithDescriptor:self.passDescriptor]; 

        [self.commandEncoder endEncoding];
        [commandBuffer commit];
        
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


        [self.app activateIgnoringOtherApps:YES];
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
        [self.window makeKeyAndOrderFront:nil];
        [self.window makeFirstResponder:self.window.contentView];
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

        int numLights = self.numLights;//HOLY HELL IT WORKS
        [self.commandEncoder setFragmentBytes:&numLights length:sizeof(int) atIndex:0];
        [self.commandEncoder setFragmentBytes:positions length:sizeof(positions) atIndex:1];
        [self.commandEncoder setFragmentBytes:constants length:sizeof(constants) atIndex:2];
        [self.commandEncoder setFragmentBytes:linears length:sizeof(linears) atIndex:3];
        [self.commandEncoder setFragmentBytes:quads length:sizeof(quads) atIndex:4];
        [self.commandEncoder setFragmentBytes:ambients length:sizeof(ambients) atIndex:5];
        [self.commandEncoder setFragmentBytes:diffuses length:sizeof(diffuses) atIndex:6];
        [self.commandEncoder setFragmentBytes:speculars length:sizeof(speculars) atIndex:7];
        [self.commandEncoder setVertexBytes:self.model length:sizeof(float) * 16 atIndex:3];
        [self.commandEncoder setVertexBytes:self.view length:sizeof(float) * 16 atIndex:4];
        [self.commandEncoder setVertexBytes:self.projection length:sizeof(float) * 16 atIndex:5];
        [self.commandEncoder setVertexBytes:&self.pos[0] length:sizeof(float) * 3 atIndex:6];
    }

    - (void)draw {
        [self.commandEncoder setCullMode:MTLCullModeFront];
        [self.commandEncoder setRenderPipelineState:self.pipelineState];
        [self.commandEncoder setDepthStencilState:self.depthStencilState];
        
        [self.commandEncoder setVertexBuffer:self.vertexBuffer offset:0 atIndex:0];
        [self.commandEncoder setVertexBuffer:self.normalBuffer offset:0 atIndex:1];
        [self.commandEncoder setVertexBuffer:self.uvBuffer offset:0 atIndex:2];
        [self.commandEncoder setVertexBytes:self.model length:sizeof(float) * 16 atIndex:3];
        [self.commandEncoder setVertexBytes:self.view length:sizeof(float) * 16 atIndex:4];
        [self.commandEncoder setVertexBytes:self.projection length:sizeof(float) * 16 atIndex:5];
        [self.commandEncoder setVertexBytes:&self.pos[0] length:sizeof(float) * 3 atIndex:6];

        [self.commandEncoder setFragmentTexture:self.texture atIndex:0];
        [self.commandEncoder setFragmentSamplerState:self.sampler atIndex:0];

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

        self.wheelY = 0;
        self.wheelX = 0;
    }
    - (void)predrawTex:(int)width height:(int)height {
        if (!self.renderTexture || 
            self.renderTexture.width != width || 
            self.renderTexture.height != height) {
            
            MTLTextureDescriptor *texDesc = [[MTLTextureDescriptor alloc] init];
            texDesc.pixelFormat = MTLPixelFormatBGRA8Unorm;
            texDesc.width = width;
            texDesc.height = height;
            texDesc.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
            self.renderTexture = [self.device newTextureWithDescriptor:texDesc];
            
            MTLTextureDescriptor *depthDesc = [[MTLTextureDescriptor alloc] init];
            depthDesc.pixelFormat = MTLPixelFormatDepth32Float;
            depthDesc.width = width;
            depthDesc.height = height;
            depthDesc.usage = MTLTextureUsageRenderTarget;
            self.depthTTexture = [self.device newTextureWithDescriptor:depthDesc];
        }
        
        self.commandBuffer = [self.commandQueue commandBuffer];
        

        self.passDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
        self.passDescriptor.colorAttachments[0].texture = self.renderTexture;
        self.passDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(clearColor.x,clearColor.y,clearColor.z,1.0);
        self.passDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
        self.passDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;

        self.passDescriptor.depthAttachment.texture = self.depthTTexture;
        self.passDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
        self.passDescriptor.depthAttachment.clearDepth = 1.0;
        self.passDescriptor.depthAttachment.storeAction = MTLStoreActionStore;
        self.commandEncoder = [self.commandBuffer renderCommandEncoderWithDescriptor:self.passDescriptor];


        int numLights = self.numLights;
        [self.commandEncoder setFragmentBytes:&numLights length:sizeof(int) atIndex:0];
        [self.commandEncoder setFragmentBytes:positions length:sizeof(positions) atIndex:1];
        [self.commandEncoder setFragmentBytes:constants length:sizeof(constants) atIndex:2];
        [self.commandEncoder setFragmentBytes:linears length:sizeof(linears) atIndex:3];
        [self.commandEncoder setFragmentBytes:quads length:sizeof(quads) atIndex:4];
        [self.commandEncoder setFragmentBytes:ambients length:sizeof(ambients) atIndex:5];
        [self.commandEncoder setFragmentBytes:diffuses length:sizeof(diffuses) atIndex:6];
        [self.commandEncoder setFragmentBytes:speculars length:sizeof(speculars) atIndex:7];
        [self.commandEncoder setVertexBytes:self.model length:sizeof(float) * 16 atIndex:3];
        [self.commandEncoder setVertexBytes:self.view length:sizeof(float) * 16 atIndex:4];
        [self.commandEncoder setVertexBytes:self.projection length:sizeof(float) * 16 atIndex:5];
        [self.commandEncoder setVertexBytes:&self.pos[0] length:sizeof(float) * 3 atIndex:6];
    }

    - (void)drawTex {
        [self.commandEncoder setCullMode:MTLCullModeFront];
        [self.commandEncoder setRenderPipelineState:self.pipelineState];
        [self.commandEncoder setDepthStencilState:self.depthStencilState];
        [self.commandEncoder setVertexBuffer:self.vertexBuffer offset:0 atIndex:0];
        [self.commandEncoder setVertexBuffer:self.normalBuffer offset:0 atIndex:1];
        [self.commandEncoder setVertexBuffer:self.uvBuffer offset:0 atIndex:2];
        [self.commandEncoder setVertexBytes:self.model length:sizeof(float) * 16 atIndex:3];
        [self.commandEncoder setVertexBytes:self.view length:sizeof(float) * 16 atIndex:4];
        [self.commandEncoder setVertexBytes:self.projection length:sizeof(float) * 16 atIndex:5];
        [self.commandEncoder setVertexBytes:&self.pos[0] length:sizeof(float) * 3 atIndex:6];
        [self.commandEncoder setFragmentTexture:self.texture atIndex:0];
        [self.commandEncoder setFragmentSamplerState:self.sampler atIndex:0];
        [self.commandEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:self.vertCount];
    }

    - (id<MTLTexture>)renderTex {
        [self.commandEncoder endEncoding];
        [self.commandBuffer commit];
        return self.renderTexture;
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

        free(self.model);
        free(self.view);
        free(self.projection);
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
        self.vertCount = vertices.size();
    }

    - (void)setNormalsDirect:(const std::vector<glm::vec3>&)normals {
        self.normalBuffer = [self.device newBufferWithBytes:normals.data()
                                                length:normals.size() * sizeof(glm::vec3)
                                            options:MTLResourceStorageModeShared];
        self.normCount = normals.size();
    }

    - (void)setUvsDirect:(const std::vector<glm::vec2>&)uvs {
        self.uvBuffer = [self.device newBufferWithBytes:uvs.data()
                                            length:uvs.size() * sizeof(glm::vec2)
                                        options:MTLResourceStorageModeShared];
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
    - (void)setModelMatrix:(const float*)matrix {
        memcpy(self.model, matrix, sizeof(float) * 16);
    }
    - (void)setViewMatrix:(const float*)matrix pos:(glm::vec3)pos {
        memcpy(self.view, matrix, sizeof(float) * 16);
        self.pos = pos;
    }
    - (void)setProjectionMatrix:(const float*)matrix {
        memcpy(self.projection, matrix, sizeof(float) * 16);
    }
    - (void)bindTexture:(void*)tex samp:(id<MTLSamplerState>)samp {
        self.texture = (__bridge id<MTLTexture>)tex;
        self.sampler = samp;
    }
    //idk at what point it went from pos amb dif spec const lin quad to pos const lin quad amb dif spec but whatever
    - (simd::float3)glmtosimd3:(glm::vec3)v3{return simd::float3{v3.x,v3.y,v3.z};}
    - (void)addLight:(glm::vec3)pos 
            ambient:(glm::vec3)ambient 
            diffuse:(glm::vec3)diffuse 
            specular:(glm::vec3)specular 
            constant:(float)constant 
            linear:(float)linear 
        quadratic:(float)quadratic {
        
        //self.positions[self.numLights] = pos;
        if (self.numLights >= MAX_LIGHTS) return;
        positions[self.numLights] = [self glmtosimd3:pos];
        constants[self.numLights] = simd::float4{constant,0.0,0.0,0.0};
        linears[self.numLights] = simd::float4{linear,0.0,0.0,0.0};
        quads[self.numLights] = simd::float4{quadratic,0.0,0.0,0.0};
        ambients[self.numLights] = [self glmtosimd3:ambient];
        diffuses[self.numLights] = [self glmtosimd3:diffuse];
        speculars[self.numLights] = [self glmtosimd3:specular];
        self.numLights++;
    }

    
    - (void) setLightPos:(int)index position:(glm::vec3)position      {positions[index] = [self glmtosimd3:position];}
    - (void) setLightAmbients:(int)index ambient:(glm::vec3)ambient   {ambients[index] = [self glmtosimd3:ambient];}
    - (void) setLightDiffuses:(int)index diffuse:(glm::vec3)diffuse   {diffuses[index] = [self glmtosimd3:diffuse];}
    - (void) setLightSpeculars:(int)index specular:(glm::vec3)specular{speculars[index] = [self glmtosimd3:specular];}
    - (void) setLightConstants:(int)index constant:(float)constant    {constants[index] = simd::float4{constant,0.0,0.0,0.0};}
    - (void) setLightLinears:(int)index linear:(float)linear          {linears[index] = simd::float4{linear,0.0,0.0,0.0};}
    - (void) setLightQuads:(int)index quad:(float)quad                {quads[index] = simd::float4{quad,0.0,0.0,0.0};}



// #### INPUT ####
    - (void)updateInput:(NSEvent *)event {
        if (event.type == NSEventTypeKeyDown || event.type == NSEventTypeKeyUp) {
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
                self.mouseStates[@(0)] = @(event.type == NSEventTypeLeftMouseDown ? 1 : 0);
            } 
            else if (event.type == NSEventTypeRightMouseDown || event.type == NSEventTypeRightMouseUp) {
                self.mouseStates[@(1)] = @(event.type == NSEventTypeRightMouseDown ? 1 : 0);
            }
            else if (event.type == NSEventTypeOtherMouseDown || event.type == NSEventTypeOtherMouseUp) {
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
        NSRect frame = self.window.frame;
        return NSMakeSize(frame.size.width, frame.size.height);
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

        return NSMakePoint(windowFrame.origin.x,windowFrame.origin.y - (windowFrame.size.height - contentViewFrame.size.height));
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

@end

// #### MAIN ####

    void MetalBento::init(const char *title, int w, int h, int x, int y) {
        MetalRendererObjC *renderer = [[MetalRendererObjC alloc] init];
        @autoreleasepool {
            [renderer initRenderer:title width:w height:h x:x y:y];
        }
        this->rendererObjC = (__bridge void*)renderer;
    }

    void MetalBento::initSound(){
        aldevice = alcOpenDevice(nullptr);
        context = alcCreateContext(aldevice, nullptr);
        if (!context)alcCloseDevice(aldevice);
        alcMakeContextCurrent(context);
        ALenum error = alGetError();
    }

    void MetalBento::setClearColor(glm::vec4 col){
        @autoreleasepool {
            clearColor = col;
        }
    }

    void MetalBento::predraw() {
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

    void MetalBento::draw() {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            [renderer draw];
        }
    }

    void MetalBento::render() {
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
                [renderer.app updateWindows];
                [renderer updateInput:event];
            }
            [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:1.0 / 144.0]];
        }
    }
    void MetalBento::predrawTex(int width,int height) {
        @autoreleasepool {
            MetalRendererObjC* renderer = (__bridge MetalRendererObjC*)this->rendererObjC;
            [renderer predrawTex:width height:height];
        }
    }

    void MetalBento::drawTex() {
        @autoreleasepool {
            MetalRendererObjC* renderer = (__bridge MetalRendererObjC*)this->rendererObjC;
            [renderer drawTex];
        }
    }

    Texture* MetalBento::renderTex() {
        @autoreleasepool {
            MetalRendererObjC* renderer = (__bridge MetalRendererObjC*)this->rendererObjC;

            MTLSamplerDescriptor *samplerDescriptor = [[MTLSamplerDescriptor alloc] init];
            samplerDescriptor.sAddressMode = MTLSamplerAddressModeRepeat;
            samplerDescriptor.tAddressMode = MTLSamplerAddressModeRepeat;
            samplerDescriptor.minFilter = MTLSamplerMinMagFilterNearest;
            samplerDescriptor.magFilter = MTLSamplerMinMagFilterNearest;
            id<MTLSamplerState> sampler = [device newSamplerStateWithDescriptor:samplerDescriptor];
            
            return new Texture([renderer renderTex],sampler);
        }
    }

    void MetalBento::exit() {
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

    bool MetalBento::isRunning() {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
          return [renderer isRunning];
        }
    }

    void MetalBento::toggleFullscreen(){
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            [renderer toggleFullScreen];
        }
    }

    void MetalBento::focus(){
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            [renderer focus];
        }
    }

    bool MetalBento::isWindowFocused() {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            return [renderer isWindowFocused];
        }
    }

// #### UNIFORMS ####

    void MetalBento::setVerticesDirect(const std::vector<glm::vec3>& vertices) {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)rendererObjC;
            [renderer setVerticesDirect:vertices];
        }
    }
    
    void MetalBento::setNormalsDirect(const std::vector<glm::vec3>& normals) {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)rendererObjC;
            [renderer setNormalsDirect:normals];
        }
    }
    
    void MetalBento::setUvsDirect(const std::vector<glm::vec2>& uvs) {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)rendererObjC;
            [renderer setUvsDirect:uvs];
        }
    }

    void MetalBento::setVertices(class vertexBuffer vs) {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            [renderer setVertices:(__bridge id<MTLBuffer>)vs.getBuffer() count:vs.size() * 3];
        }
    }

    void MetalBento::setNormals(class normalBuffer ns) {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            [renderer setNormals:(__bridge id<MTLBuffer>)ns.getBuffer() count:ns.size() * 3];
        }
    }

    void MetalBento::setUvs(class uvBuffer uvs) {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            [renderer setUvs:(__bridge id<MTLBuffer>)uvs.getBuffer() count:uvs.size() * 2];
        }
    }


    void MetalBento::setModelMatrix(const glm::mat4 m) {
        @autoreleasepool {
            MetalRendererObjC* renderer = (__bridge MetalRendererObjC*)this->rendererObjC;
            [renderer setModelMatrix:(float*)&m];
        }
    }

    void MetalBento::setViewMatrix(const glm::mat4 v,const glm::vec3 p) {
        @autoreleasepool {
            MetalRendererObjC* renderer = (__bridge MetalRendererObjC*)this->rendererObjC;
            [renderer setViewMatrix:(float*)&v pos:p];
        }
    }

    void MetalBento::setProjectionMatrix(const glm::mat4 p) {
        @autoreleasepool {
            MetalRendererObjC* renderer = (__bridge MetalRendererObjC*)this->rendererObjC;
            [renderer setProjectionMatrix:(float*)&p];
        }
    }

    void MetalBento::bindTexture(Texture *tex) {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            [renderer bindTexture:tex->getTexture() samp:tex->getSampler()];
        }
    }

    void MetalBento::unbindTexture() {

    }
// #### INPUT ####
    bool MetalBento::getKey(int key) {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            return [renderer.keyStates[@(key)] integerValue];
        }
    }

    bool MetalBento::getMouse(int mouse) {
        @autoreleasepool {
            MetalRendererObjC* renderer = (__bridge MetalRendererObjC*)this->rendererObjC;
            NSNumber* mouseStateNumber = renderer.mouseStates[@(mouse)];
            
            switch ([mouseStateNumber integerValue]) {
                case 1:
                    return true;
                case 0:
                    return false;
                default:
                    break;
            }
            return false;
        }
    }

    double MetalBento::getScroll(int wheel){
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            return [renderer getScroll:wheel];
        }
    }

// #### MOUSE AND WINDOWS ####
    glm::vec2 MetalBento::getWindowSize() {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            NSSize size = [renderer getWindowSize];
            return glm::vec2(size.width,size.height);
        }
    }

    glm::vec2 MetalBento::getWindowPos() {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            NSPoint pos = [renderer getWindowPos];
            glm::vec2 wSize = getWindowSize();
            glm::vec2 dSize = getDisplaySize();
            return glm::vec2(pos.x,dSize.y-(pos.y+wSize.y));
        }
    }

    void MetalBento::setMouseCursor(bool hide, int cursor) {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            [renderer setMouseCursor:cursor hidden:hide];
        }
    }

    glm::vec2 MetalBento::getMousePosition() {
        @autoreleasepool {
            glm::vec2 dsize = getDisplaySize();
            CGPoint mouseLocation = [NSEvent mouseLocation];
            return glm::vec2(mouseLocation.x, dsize.y-mouseLocation.y);
        }
    }

    void MetalBento::setMousePosition(glm::vec2 pos, bool needsFocus) {
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


    void MetalBento::setWindowPos(glm::vec2 pos) {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            glm::vec2 wSize = getWindowSize();
            glm::vec2 dSize = getDisplaySize();
            [renderer setWindowPos:NSMakePoint(pos.x,dSize.y-(pos.y+wSize.y))];
        }
    }


    glm::vec2 MetalBento::getControllerAxis(int controller, JoystickType joystick) {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            NSPoint point = [renderer getControllerAxis:controller type:(joystick==GAMEPAD_JOYSTICK_LEFT?1:joystick==GAMEPAD_JOYSTICK_RIGHT?0:-1)];
            return glm::vec2(point.x/127.5,point.y/127.5);
        }
    }
    
    bool MetalBento::getControllerButton(int controller, ButtonType button){
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            return [renderer getControllerButton:controller button:button];
        }
    }

    glm::vec2 MetalBento::getDisplaySize() {
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



    void MetalBento::initImgui() {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            IMGUI_CHECKVERSION();
            ImGuiContext* imguiContext = ImGui::CreateContext();
            ImGui::SetCurrentContext(imguiContext);
            ImGui_ImplMetal_Init(device);

            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
        }
    }

    
    void MetalBento::imguiNewFrame() {
        @autoreleasepool {
            ImGuiIO& io = ImGui::GetIO();
            glm::vec2 size = getWindowSize();
            io.DisplaySize = ImVec2(size.x,size.y);
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            ImGui_ImplMetal_NewFrame([renderer getRenderPass]);
            ImGui::NewFrame();
        }
    }


    void MetalBento::imguiRender() {
        @autoreleasepool {
            ImGui::Render();
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(),[renderer getCommandBuffer],[renderer getCommandEncoder]);
        }
    }

// #### LIGHTING ####
    //FUYDAOSJDWOAKLAWFHIUOSNFOBEFDOUWNKLDWASNIOFAWNDLSA
    void MetalBento::addLight(const glm::vec3& pos,const glm::vec3& ambient,const glm::vec3& diffuse,const glm::vec3& specular,float constant,float linear,float quadratic) {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            [renderer addLight:pos
                       ambient:ambient
                       diffuse:diffuse
                      specular:specular
                      constant:constant
                        linear:linear
                     quadratic:quadratic];
        }
    }
    void MetalBento::setLightPos(int index, glm::vec3& position){
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            [renderer setLightPos:index position:position];
        }
    }
    void MetalBento::setLightConstants(int index, float constant){
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            [renderer setLightConstants:index constant:constant];
        }
    }
    void MetalBento::setLightLinears(int index, float linear){
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            [renderer setLightLinears:index linear:linear];
        }
    }
    void MetalBento::setLightQuads(int index, float quad){
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            [renderer setLightQuads:index quad:quad];
        }
    }
    void MetalBento::setLightAmbients(int index, glm::vec3& ambient){
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            [renderer setLightAmbients:index ambient:ambient];
        }
    }
    void MetalBento::setLightDiffuses(int index, glm::vec3& diffuse){
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            [renderer setLightDiffuses:index diffuse:diffuse];
        }
    }
    void MetalBento::setLightSpeculars(int index, glm::vec3& specular){
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            [renderer setLightSpeculars:index specular:specular];
        }
    }

    std::string MetalBento::getFramework(){
        @autoreleasepool {
            return "Metal";
        }
    }