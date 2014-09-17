#import "Cheminot.h"
#import <Cordova/CDVPlugin.h>
#import "Adapter.h"

@implementation Cheminot

-(const char*)dbPath {
    NSString* path = [[NSBundle mainBundle] pathForResource:@"cheminot" ofType:@"db"];
    return [path UTF8String];
}

- (void)init:(CDVInvokedUrlCommand*)command
{
    CDVPluginResult* pluginResult = nil;
    NSString* version = [NSString stringWithUTF8String:getVersion(self.dbPath)];
    pluginResult = [CDVPluginResult resultWithStatus:CDVCommandStatus_OK messageAsString:version];
    [self.commandDelegate sendPluginResult:pluginResult callbackId:command.callbackId];
}

@end
