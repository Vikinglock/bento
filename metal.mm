#include "lib/glm/glm.hpp"
#include "lib/glm/gtc/matrix_transform.hpp"
#include "lib/glm/gtc/type_ptr.hpp"
#import "metal.h"
#import "metalcommon.h"

id<MTLDevice> device = nil;


typedef NS_ENUM(NSInteger, KeyState) {
    KeyStateNone,
    KeyStatePressed,
    KeyStateReleased
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
@property (nonatomic, strong) NSMutableDictionary<NSNumber *, NSNumber *> *keyStates;
@property (nonatomic, strong) id<MTLTexture> texture;
@property (nonatomic, strong) id<MTLSamplerState> sampler;
@property (nonatomic, strong) id<CAMetalDrawable> drawable;
@property (nonatomic, strong) id<MTLCommandBuffer> commandBuffer;
@property (nonatomic, strong) id<MTLRenderCommandEncoder> commandEncoder;
@property (nonatomic) NSInteger vertCount;
@property (nonatomic) NSInteger normCount;
@property (nonatomic) NSInteger uvCount;
@property (nonatomic) float *model;
@property (nonatomic) float *view;
@property (nonatomic) float *projection;
- (void)initRenderer:(const char *)title width:(int)width height:(int)height;
- (void)render;
- (BOOL)isRunning;
- (NSSize)getWindowSize;
- (KeyState)getKey:(NSEvent *)event forKey:(int)key;
@end

@implementation MetalRendererObjC


// #### MAIN ####

- (void)initRenderer:(const char *)title width:(int)width height:(int)height {
        self.device = MTLCreateSystemDefaultDevice();
        device = MTLCreateSystemDefaultDevice();
        self.commandQueue = [self.device newCommandQueue];
        self.shouldClose = NO;
        self.model = (float *)malloc(sizeof(float) * 16);
        self.view = (float *)malloc(sizeof(float) * 16);
        self.projection = (float *)malloc(sizeof(float) * 16);

        self.app = [NSApplication sharedApplication];
        [self.app setActivationPolicy:NSApplicationActivationPolicyRegular];

        NSRect frame = NSMakeRect(0, 0, width, height);
        NSUInteger windowStyle = (NSWindowStyleMaskTitled |
                                NSWindowStyleMaskClosable |
                                NSWindowStyleMaskResizable |
                                NSWindowStyleMaskMiniaturizable);
        self.window = [[NSWindow alloc] initWithContentRect:frame
                                                styleMask:windowStyle
                                                    backing:NSBackingStoreBuffered
                                                    defer:NO];
        [self.window setTitle:@(title)];//"ベント"];
        [self.window makeKeyAndOrderFront:nil];
        [self.window makeFirstResponder:self];

        [self.window setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];

        [[NSNotificationCenter defaultCenter] addObserver:self
                                                selector:@selector(windowDidResize:)
                                                    name:NSWindowDidResizeNotification
                                                object:self.window];
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                selector:@selector(windowWillClose:)
                                                    name:NSWindowWillCloseNotification
                                                object:self.window];
        self.keyStates = [NSMutableDictionary dictionary];
        self.metalLayer = [CAMetalLayer layer];
        self.metalLayer.device = self.device;
        self.metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
        self.metalLayer.contentsScale = [NSScreen mainScreen].backingScaleFactor;
        self.metalLayer.framebufferOnly = YES;
        [self.metalLayer setFrame:frame];
        [self.window.contentView setLayer:self.metalLayer];
        [self.window.contentView setWantsLayer:YES];
        //[self toggleFullScreen];



        NSString *metalFilePath = @"./shader.metal";
        NSError *error = nil;

        NSString *shaderSource = [NSString stringWithContentsOfFile:metalFilePath encoding:NSUTF8StringEncoding error:&error];
        if (error) {
            NSLog(@"Error loading shader file: %@", error);
            return;
        }

        id<MTLLibrary> library = [self.device newLibraryWithSource:shaderSource options:nil error:&error];
        if (!library) {
            NSLog(@"Error creating Metal shader library: %@", error);
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
            NSLog(@"Error creating pipeline state: %@", error);
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
        



        id<CAMetalDrawable> drawable = [self.metalLayer nextDrawable];
        if (!drawable) {
            return;
        }
        

        MTLRenderPassDescriptor *passDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
        passDescriptor.colorAttachments[0].texture = drawable.texture;
        passDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
        passDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.0, 0.0, 0.0, 1.0);
        passDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;

        passDescriptor.depthAttachment.texture = self.depthTexture;
        passDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
        passDescriptor.depthAttachment.clearDepth = 1.0;
        passDescriptor.depthAttachment.storeAction = MTLStoreActionDontCare;


        id<MTLCommandBuffer> commandBuffer = [self.commandQueue commandBuffer];
        self.commandEncoder = [commandBuffer renderCommandEncoderWithDescriptor:passDescriptor]; 

        [self.commandEncoder endEncoding];
        [commandBuffer commit];
        




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

    - (void)predraw {
        self.commandBuffer = [self.commandQueue commandBuffer];
        self.drawable = [self.metalLayer nextDrawable];
        if (!self.drawable) {
            return;
        }
        MTLRenderPassDescriptor *passDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
        passDescriptor.colorAttachments[0].texture = self.drawable.texture;
        passDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.0, 0.0, 0.0, 1.0);
        passDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
        passDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;

        passDescriptor.depthAttachment.texture = self.depthTexture;
        passDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
        passDescriptor.depthAttachment.clearDepth = 1.0;
        passDescriptor.depthAttachment.storeAction = MTLStoreActionStore;
        self.commandEncoder = [self.commandBuffer renderCommandEncoderWithDescriptor:passDescriptor];
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
        [self.commandBuffer presentDrawable:self.drawable];
        [self.commandBuffer commit];
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
        }
    }

// #### UNIFORMS ####
    - (void)setVertices:(const float *)vertices count:(NSUInteger)count {
        self.vertexBuffer = [self.device newBufferWithBytes:vertices length:sizeof(float) * count options:MTLResourceStorageModeShared];
        self.vertCount = count/3;
    }
    - (void)setNormals:(const float *)normals count:(NSUInteger)count {
        self.normalBuffer = [self.device newBufferWithBytes:normals length:sizeof(float) * count options:MTLResourceStorageModeShared];
        self.normCount = count/3;
    }
    - (void)setUvs:(const float *)uvs count:(NSUInteger)count {
        self.uvBuffer = [self.device newBufferWithBytes:uvs length:sizeof(float) * count options:MTLResourceStorageModeShared];
        self.uvCount = count/3;
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
    - (void)bindTextures:(NSArray<id<MTLTexture>> *)textures {
        for (NSUInteger i = 0; i < textures.count; ++i) {
            [self.commandEncoder setFragmentTexture:textures[i] atIndex:i];
        }
    }
    - (void)bindTexture:(id<MTLTexture>)tex samp:(id<MTLSamplerState>)samp slot:(NSUInteger)slot {
        self.texture = tex;
        self.sampler = samp;
        //[self.commandEncoder setFragmentTexture:tex atIndex:slot];
    }


// #### INPUT ####
    - (KeyState)getKey:(NSEvent *)event forKey:(int)key {
        if (event.type == NSEventTypeKeyDown) {
            NSUInteger keyCode = event.keyCode;
            if (keyCode == key) {
                return KeyStatePressed;
            }
        } else if (event.type == NSEventTypeKeyUp) {
            NSUInteger keyCode = event.keyCode;
            if (keyCode == key) {
                return KeyStateReleased;
            }
        }
        return KeyStateNone;
    }


    - (void)updateKeyStates:(NSEvent *)event {
        if (event.modifierFlags & NSEventModifierFlagCommand) {
            self.keyStates[@(55)] = @(KeyStatePressed);
        }else{
            self.keyStates[@(55)] = @(KeyStateReleased);
        }
        if (event.modifierFlags & NSEventModifierFlagControl) {
            self.keyStates[@(59)] = @(KeyStatePressed);
        } else {
            self.keyStates[@(59)] = @(KeyStateReleased);
        }
        if (event.type == NSEventTypeKeyDown || event.type == NSEventTypeKeyUp) {
            NSUInteger keyCode = event.keyCode;
            if (event.type == NSEventTypeKeyDown) {
                self.keyStates[@(keyCode)] = @(KeyStatePressed);
            } else if (event.type == NSEventTypeKeyUp) {
                self.keyStates[@(keyCode)] = @(KeyStateReleased);
            }
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
        return [self.window frame].origin;
    }

    - (void)setWindowPos:(NSPoint)pos {
        [self.window setFrameOrigin:pos];
    }

    - (NSSize)getDisplaySize {
        NSScreen *mainScreen = [NSScreen mainScreen];
        return [mainScreen frame].size;
    }
    
    

@end

// #### MAIN ####

    void MetalBento::init(const char *title, int w, int h) {
        MetalRendererObjC *renderer = [[MetalRendererObjC alloc] init];
        [renderer initRenderer:title width:w height:h];
        this->rendererObjC = (__bridge void*)renderer;
    }

    void MetalBento::predraw() {
        MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;

        @autoreleasepool {
            [renderer predraw];
        }
    }

    void MetalBento::draw() {
        MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;

        @autoreleasepool {
            [renderer draw];
        }
    }

    void MetalBento::render() {
        MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;

        @autoreleasepool {
            [renderer render];
            NSEvent *event = nil;
            while ((event = [renderer.app nextEventMatchingMask:NSEventMaskAny
                                                    untilDate:nil
                                                        inMode:NSDefaultRunLoopMode
                                                        dequeue:YES])) {
                [renderer.app sendEvent:event];
                [renderer.app updateWindows];
                [renderer updateKeyStates:event];
            }
            [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:1.0 / 144.0]];
        }
    }

    void MetalBento::exit() {
        [[NSApplication sharedApplication] terminate:nil];
    }

    bool MetalBento::isRunning() {
        MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
        return [renderer isRunning];
    }

    void MetalBento::toggleFullscreen(){
        MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
        [renderer toggleFullScreen];
    }

    bool MetalBento::isWindowFocused() {
        MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
        return [renderer isWindowFocused];
    }

// #### UNIFORMS ####

    void MetalBento::setVertices(const std::vector<glm::vec3>& vertices) {
        MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
        [renderer setVertices:(float *)vertices.data() count:vertices.size() * 3];
    }

    void MetalBento::setNormals(const std::vector<glm::vec3>& normals) {
        MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
        [renderer setNormals:(float *)normals.data() count:normals.size() * 3];
    }

    void MetalBento::setUvs(const std::vector<glm::vec2>& uvs) {
        MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
        [renderer setUvs:(float *)uvs.data() count:uvs.size() * 2];
    }


    void MetalBento::setModelMatrix(const glm::mat4& m) {
        MetalRendererObjC* renderer = (__bridge MetalRendererObjC*)this->rendererObjC;
        [renderer setModelMatrix:(float*)&m];
    }

    void MetalBento::setViewMatrix(const glm::mat4& v) {
        MetalRendererObjC* renderer = (__bridge MetalRendererObjC*)this->rendererObjC;
        [renderer setViewMatrix:(float*)&v];
    }

    void MetalBento::setProjectionMatrix(const glm::mat4& p) {
        MetalRendererObjC* renderer = (__bridge MetalRendererObjC*)this->rendererObjC;
        [renderer setProjectionMatrix:(float*)&p];
    }

    void MetalBento::bindTexture(Texture *tex, int slot) {
        MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
        [renderer bindTexture:tex->getTexture() samp:tex->getSampler() slot:0];
    }

    void MetalBento::unbindTexture() {

    }
// #### INPUT ####
    bool MetalBento::getKey(int key) {
        MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
        KeyState state = static_cast<KeyState>([renderer.keyStates[@(key)] integerValue] ?: KeyStateNone);
        
        switch (state) {
            case KeyStatePressed:
                return true;
            break;

            case KeyStateReleased:
                return false;
            break;

            default:
                break;
        }
        return false;
    }
// #### MOUSE AND WINDOWS ####
    glm::vec2 MetalBento::getWindowSize() {
        MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
        NSSize size = [renderer getWindowSize];
        return glm::vec2(size.width,size.height);
    }

    glm::vec2 MetalBento::getWindowPos() {
        MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
        NSPoint pos = [renderer getWindowPos];
        glm::vec2 wSize = getWindowSize();
        glm::vec2 dSize = getDisplaySize();
        return glm::vec2(pos.x,dSize.y-(pos.y+wSize.y));
    }

    void MetalBento::setMouseCursor(bool hide, int cursor) {
        MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
        [renderer setMouseCursor:cursor hidden:hide];
    }

    glm::vec2 MetalBento::getMousePosition() {
        CGPoint mouseLocation = [NSEvent mouseLocation];
        return glm::vec2(mouseLocation.x, mouseLocation.y);
    }

    void MetalBento::setMousePosition(glm::vec2 pos, bool needsFocus) {
        MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
        if ([renderer isWindowFocused]) {
            CGPoint newPosition;
            newPosition.x = pos.x;
            glm::vec2 dSize = getDisplaySize();
            newPosition.y = dSize.y - pos.y;
            CGEventRef moveEvent = CGEventCreateMouseEvent(NULL, kCGEventMouseMoved, newPosition, kCGMouseButtonLeft);
            CGEventPost(kCGHIDEventTap, moveEvent);
            CFRelease(moveEvent);
        }
    }


    void MetalBento::setWindowPos(glm::vec2 pos) {
        MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
        glm::vec2 wSize = getWindowSize();
        glm::vec2 dSize = getDisplaySize();
        [renderer setWindowPos:NSMakePoint(pos.x,dSize.y-(pos.y+wSize.y))];
    }

    glm::vec2 MetalBento::getDisplaySize() {
        MetalRendererObjC *renderer = (__bridge MetalRendererObjC *)this->rendererObjC;
        NSSize size = [renderer getDisplaySize];
        return glm::vec2(size.width,size.height);
    }

