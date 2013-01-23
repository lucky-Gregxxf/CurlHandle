//
//  CURLMulti.h
//
//  Created by Sam Deane on 20/09/2012.
//  Copyright (c) 2012 Karelia Software. All rights reserved.
//

#import <Foundation/Foundation.h>

#import <curl/curl.h>

@class CURLHandle;

/**
 * Wrapper for a curl_multi handle.
 * In general you shouldn't use this class directly - use the extensions in NSURLRequest+CURLHandle
 * instead, and work with normal NSURLConnections.
 *
 * CURLProtocol uses the global sharedInstance to implement the NSURLRequest/NSURLConnection 
 * integration.
 *
 * There's nothing to stop you making other instances if you want to - it's just not really necessary, particularly
 * as we don't expose the curl multi externally.
 *
 * This class works by setting up a serial GCD queue to process all events associated with the multi. We add
 * gcd dispatch sources for each socket that the multi makes, and use them to notify curl when something
 * happens that needs attention.
 */

@interface CURLMulti : NSObject
{
    NSMutableArray* _handles;
    CURLM* _multi;
    dispatch_queue_t _queue;
    dispatch_source_t _timer;
}

/**
 * Return a default instance. 
 * Don't call startup or shutdown on this instance - startup will already have been called, and shutting it down
 * will be terminal since it's shared by everything.
 *
 * @return The shared instance.
 */

+ (CURLMulti*)sharedInstance;


/** Prepare the multi for work. Needs to be called once before addHandle is called. Should be matched with a call to shutdown
 * before the multi is destroyed.
 */

- (void)startup;

/** 
 * Shut down the multi and clean up all resources that it was using.
 */

- (void)shutdown;

/**
 * Assign a CURLHandle to the multi to manage.
 * CURLHandle uses this method internally when you call loadRequest:withMulti: on a handle,
 * so generally you don't need to call it directly.
 * The multi will retain the handle for as long as it needs it, but will silently release it once
 * the handle's upload/download has completed or failed.
 *
 * @param handle The handle to manage. Will be retained by the multi.
 */

- (void)manageHandle:(CURLHandle*)handle;

/** 
 * Cancel a handle, and remove it from the multi.
 * Cancelling the handle will cause the multi to release it, will stop any progress on it, will call
 * the cancel method on the handle, and will report to the delegate that it was cancelled.
 *
 * It is safe to call this method for a handle that has already been cancelled, or has completed,
 * (or indeed was never managed by the multi). Doing so will simply do nothing.
 *
 * @param handle The handle to cancel. Should have previously been added with manageHandle:.
 */

- (void)cancelHandle:(CURLHandle*)handle;

- (dispatch_source_t)updateSource:(dispatch_source_t)source type:(dispatch_source_type_t)type socket:(int)socket required:(BOOL)required;

@end