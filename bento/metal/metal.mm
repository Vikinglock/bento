#include "../lib/glm/glm.hpp"
#include "../lib/glm/gtc/matrix_transform.hpp"
#include "../lib/glm/gtc/type_ptr.hpp"
#include "../lib/hidapi/hidapi.h"
#include <vector>
#import "metal.h"
#import "metalcommon.h"

#import <Cocoa/Cocoa.h>
#import <GameController/GameController.h>


id<MTLDevice> device = nil;
bool fullscreenable = true;

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
@property (nonatomic) MTLViewport viewport;
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

        [self.window setTitle:@(title)];//"ベント"];

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


        NSString *metalFilePath = @"./bento/shaders/shader.metal";
        NSError *error = nil;

        NSString *shaderSource = [NSString stringWithContentsOfFile:metalFilePath encoding:NSUTF8StringEncoding error:&error];
        if (error) {
            NSLog(@"could not load shader file: %@", error);
            return;
        }

        id<MTLLibrary> library = [self.device newLibraryWithSource:shaderSource options:nil error:&error];
        if (!library) {
            NSLog(@"could not create metal shader library: %@", error);
            return;
        }

        id<MTLFunction> vertexFunction = [library newFunctionWithName:@"vertex_main"];
        id<MTLFunction> fragmentFunction = [library newFunctionWithName:@"fragment_main"];

        MTLVertexDescriptor *vertexDescriptor = [[MTLVertexDescriptor alloc] init];

        vertexDescriptor.attributes[0].format = MTLVertexFormatFloat3;
        vertexDescriptor.attributes[0].offset = 0;
        vertexDescriptor.attributes[0].bufferIndex = 0;
        vertexDescriptor.attributes[1].format = MTLVertexFormatFloat3;
        vertexDescriptor.attributes[1].offset = 0;
        vertexDescriptor.attributes[1].bufferIndex = 2;
        vertexDescriptor.attributes[2].format = MTLVertexFormatFloat2;
        vertexDescriptor.attributes[2].offset = 0;
        vertexDescriptor.attributes[2].bufferIndex = 3;
        vertexDescriptor.layouts[0].stride = sizeof(float) * 3;
        vertexDescriptor.layouts[2].stride = sizeof(float) * 3;
        vertexDescriptor.layouts[3].stride = sizeof(float) * 2;
        vertexDescriptor.layouts[0].stepRate = 1;
        vertexDescriptor.layouts[2].stepRate = 1;
        vertexDescriptor.layouts[3].stepRate = 1;
        vertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
        vertexDescriptor.layouts[2].stepFunction = MTLVertexStepFunctionPerVertex;
        vertexDescriptor.layouts[3].stepFunction = MTLVertexStepFunctionPerVertex;


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
        self.passDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.0, 0.0, 0.0, 1.0);
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
        self.passDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.0, 0.0, 0.0, 1.0);
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
        [self.commandEncoder setRenderPipelineState:self.pipelineState];
        [self.commandEncoder setDepthStencilState:self.depthStencilState];
        [self.commandEncoder setVertexBytes:self.model length:sizeof(float) * 16 atIndex:1];
        [self.commandEncoder setVertexBytes:self.view length:sizeof(float) * 16 atIndex:2];
        [self.commandEncoder setVertexBytes:self.projection length:sizeof(float) * 16 atIndex:3];
        [self.commandEncoder setVertexBuffer:self.vertexBuffer offset:0 atIndex:0];
        [self.commandEncoder setVertexBuffer:self.normalBuffer offset:0 atIndex:2];
        [self.commandEncoder setVertexBuffer:self.uvBuffer offset:0 atIndex:3];

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
                sourceOrigin:MTLOriginMake(1, 2, 0)
                  sourceSize:MTLSizeMake(self.appTexture.width, self.appTexture.height, 1)//self.window.frame.size.width*self.window.backingScaleFactor
                   toTexture:self.drawable.texture
            destinationSlice:0
            destinationLevel:0
           destinationOrigin:MTLOriginMake(0, 0, 0)];
        [blitEncoder endEncoding];
        [self.commandBuffer presentDrawable:self.drawable];
        [self.commandBuffer commit];
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
    - (void)setVerticesDirect:(const float *)vertices count:(NSUInteger)count {
        [self.vertexBuffer release];
        self.vertexBuffer = [self.device newBufferWithBytes:vertices
                                                    length:sizeof(float) * count
                                                    options:MTLResourceStorageModeShared];
        [self.vertexBuffer retain];
        self.vertCount = count / 3;
    }

    - (void)setNormalsDirect:(const float *)normals count:(NSUInteger)count {
        [self.normalBuffer release];
        self.normalBuffer = [self.device newBufferWithBytes:normals
                                                    length:sizeof(float) * count
                                                    options:MTLResourceStorageModeShared];
        [self.normalBuffer retain];
        self.normCount = count / 3;
    }

    - (void)setUvsDirect:(const float *)uvs count:(NSUInteger)count {
        [self.uvBuffer release];
        self.uvBuffer = [self.device newBufferWithBytes:uvs
                                                length:sizeof(float) * count
                                                options:MTLResourceStorageModeShared];
        [self.uvBuffer retain];
        self.uvCount = count / 3;
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
    - (void)setViewMatrix:(const float*)matrix {
        memcpy(self.view, matrix, sizeof(float) * 16);
    }
    - (void)setProjectionMatrix:(const float*)matrix {
        memcpy(self.projection, matrix, sizeof(float) * 16);
    }
    - (void)bindTexture:(id<MTLTexture>)tex samp:(id<MTLSamplerState>)samp {
        self.texture = tex;
        self.sampler = samp;
    }


// #### INPUT ####
    - (void)updateInput:(NSEvent *)event {
        if (event.modifierFlags & NSEventModifierFlagCommand) {
            self.keyStates[@(55)] = @(1);
        }else{
            self.keyStates[@(55)] = @(0);
        }
        if (event.modifierFlags & NSEventModifierFlagControl) {
            self.keyStates[@(59)] = @(1);
        } else {
            self.keyStates[@(59)] = @(0);
        }
        if (event.type == NSEventTypeKeyDown || event.type == NSEventTypeKeyUp) {
            NSUInteger keyCode = event.keyCode;
            if (event.type == NSEventTypeKeyDown) {
                self.keyStates[@(keyCode)] = @(1);
            } else if (event.type == NSEventTypeKeyUp) {
                self.keyStates[@(keyCode)] = @(0);
            }
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
                
                NSEventModifierFlags modifierFlags = event.modifierFlags;
                io.AddKeyEvent(ImGuiMod_Shift, (modifierFlags & NSEventModifierFlagShift) != 0);
                io.AddKeyEvent(ImGuiMod_Ctrl, (modifierFlags & NSEventModifierFlagControl) != 0);
                io.AddKeyEvent(ImGuiMod_Alt, (modifierFlags & NSEventModifierFlagOption) != 0);
                io.AddKeyEvent(ImGuiMod_Super, (modifierFlags & NSEventModifierFlagCommand) != 0);
                break;
            }

            case NSEventTypeFlagsChanged: {
                NSEventModifierFlags modifierFlags = event.modifierFlags;
                io.AddKeyEvent(ImGuiMod_Shift, (modifierFlags & NSEventModifierFlagShift) != 0);
                io.AddKeyEvent(ImGuiMod_Ctrl, (modifierFlags & NSEventModifierFlagControl) != 0);
                io.AddKeyEvent(ImGuiMod_Alt, (modifierFlags & NSEventModifierFlagOption) != 0);
                io.AddKeyEvent(ImGuiMod_Super, (modifierFlags & NSEventModifierFlagCommand) != 0);
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

    void MetalBento::predraw() {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            [renderer predraw];
        }
    }

    void MetalBento::draw() {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            [renderer draw];
            
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

    void MetalBento::exit() {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            ImGui_ImplMetal_Shutdown();
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
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            [renderer setVerticesDirect:(float *)vertices.data() count:vertices.size() * 3];
        }
    }

    void MetalBento::setNormalsDirect(const std::vector<glm::vec3>& normals) {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            [renderer setNormalsDirect:(float *)normals.data() count:normals.size() * 3];
        }
    }

    void MetalBento::setUvsDirect(const std::vector<glm::vec2>& uvs) {
        @autoreleasepool {
            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            [renderer setUvsDirect:(float *)uvs.data() count:uvs.size() * 2];
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


    void MetalBento::setModelMatrix(const glm::mat4& m) {
        @autoreleasepool {
            MetalRendererObjC* renderer = (__bridge MetalRendererObjC*)this->rendererObjC;
            [renderer setModelMatrix:(float*)&m];
        }
    }

    void MetalBento::setViewMatrix(const glm::mat4& v) {
        @autoreleasepool {
            MetalRendererObjC* renderer = (__bridge MetalRendererObjC*)this->rendererObjC;
            [renderer setViewMatrix:(float*)&v];
        }
    }

    void MetalBento::setProjectionMatrix(const glm::mat4& p) {
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
            
            switch ([renderer.keyStates[@(key)] integerValue]) {
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
            buffer = [device newBufferWithBytes:(float *)buf.data() length:sizeof(glm::vec3) * buf.size() options:MTLResourceStorageModeShared];
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

    
    void MetalBento::imgui() {
        @autoreleasepool {
            ImGuiIO& io = ImGui::GetIO();
            glm::vec2 size = getWindowSize();
            io.DisplaySize = ImVec2(size.x,size.y);

            MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
            ImGui_ImplMetal_NewFrame([renderer getRenderPass]);

            ImGui::NewFrame();
            static bool show_demo_window = true;
            ImGui::ShowDemoWindow(&show_demo_window);
            ImGui::Begin("Example Window");
            ImGui::Text("metal");
            ImGui::End();
            
            ImGui::Render();
            ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(),[renderer getCommandBuffer],[renderer getCommandEncoder]);
        }
    }

    



