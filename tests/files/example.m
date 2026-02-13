// Objective-C sample
#import <Foundation/Foundation.h>
#import "AppConfig.h"

#define MAX_RETRIES 3
#define CLAMP(x, lo, hi) ((x) < (lo) ? (lo) : ((x) > (hi) ? (hi) : (x)))

@class NetworkManager;

typedef NS_ENUM(NSInteger, Priority) {
    PriorityLow = 0,
    PriorityMedium,
    PriorityHigh
};

typedef NS_OPTIONS(NSUInteger, Permission) {
    PermissionRead  = 1 << 0,
    PermissionWrite = 1 << 1
};

typedef struct { CGFloat x; CGFloat y; } Point2D;
typedef void (^CompletionBlock)(NSData *data, NSError *error);

@protocol TaskDelegate <NSObject>
@required
- (void)taskDidFinish:(NSString *)taskName;
- (void)task:(NSString *)name didFailWithError:(NSError *)error;
@optional
- (BOOL)shouldRetryTask:(NSString *)taskName;
@end

@interface TaskManager : NSObject <TaskDelegate, NSCoding>
@property (nonatomic, strong) NSString *name;
@property (nonatomic, weak) id<TaskDelegate> delegate;
@property (nonatomic, assign, readonly) NSInteger taskCount;
@property (nonatomic, copy, nullable) CompletionBlock completion;
@property (class, nonatomic, readonly) TaskManager *sharedInstance;
+ (instancetype)sharedManager;
- (void)addTask:(NSString *)task withPriority:(Priority)priority;
- (nullable NSString *)taskAtIndex:(NSInteger)index;
- (void)runWithCompletion:(void (^)(BOOL success))block;
@end

@interface NSString (Validation)
- (BOOL)isValidEmail;
@end

@interface TaskManager ()
@property (nonatomic, strong) NSMutableArray<NSDictionary *> *tasks;
@end

@implementation TaskManager {
    dispatch_queue_t _queue;
}
@synthesize name = _name;
@dynamic taskCount;

+ (instancetype)sharedManager {
    static TaskManager *instance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [[TaskManager alloc] init];
    });
    return instance;
}

- (instancetype)init {
    self = [super init];
    if (self) {
        _tasks = [NSMutableArray new];
        _name = @"Default";
        _queue = dispatch_queue_create("com.app.tasks", DISPATCH_QUEUE_SERIAL);
    }
    return self;
}

- (void)addTask:(NSString *)task withPriority:(Priority)priority {
    @synchronized(self) {
        NSDictionary *entry = @{
            @"name": task,
            @"priority": @(priority),
            @"timestamp": [NSDate date]
        };
        [_tasks addObject:entry];
        NSLog(@"Added: %@ (priority=%ld)", task, (long)priority);
    }
}

- (nullable NSString *)taskAtIndex:(NSInteger)index {
    if (index < 0 || index >= (NSInteger)_tasks.count) return nil;
    return _tasks[index][@"name"];
}

- (void)runWithCompletion:(void (^)(BOOL success))block {
    __weak typeof(self) weakSelf = self;
    dispatch_async(_queue, ^{
        __strong typeof(weakSelf) strongSelf = weakSelf;
        if (!strongSelf) return;
        BOOL result = [strongSelf processTasks];
        dispatch_async(dispatch_get_main_queue(), ^{
            if (block) block(result);
        });
    });
}

- (BOOL)processTasks {
    @try {
        for (NSDictionary *task in _tasks) {
            NSString *name = task[@"name"];
            Priority p = [task[@"priority"] integerValue];
            if (p == PriorityHigh) NSLog(@"[HIGH] %@", name);
        }
        return YES;
    } @catch (NSException *exception) {
        NSLog(@"Error: %@", exception.reason);
        return NO;
    } @finally {
        NSLog(@"Processed %lu tasks", (unsigned long)_tasks.count);
    }
}

- (void)taskDidFinish:(NSString *)taskName {
    NSLog(@"Finished: %@", taskName);
}

- (void)task:(NSString *)name didFailWithError:(NSError *)error {
    NSLog(@"Failed: %@ - %@", name, error.localizedDescription);
}

- (void)encodeWithCoder:(NSCoder *)coder {
    [coder encodeObject:_name forKey:@"name"];
    [coder encodeObject:_tasks forKey:@"tasks"];
}

- (instancetype)initWithCoder:(NSCoder *)coder {
    self = [super init];
    if (self) {
        _name = [coder decodeObjectForKey:@"name"];
        _tasks = [[coder decodeObjectForKey:@"tasks"] mutableCopy];
    }
    return self;
}

@end

@implementation NSString (Validation)
- (BOOL)isValidEmail {
    NSString *pattern = @"[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,}";
    NSRegularExpression *regex =
        [NSRegularExpression regularExpressionWithPattern:pattern
                                                 options:NSRegularExpressionCaseInsensitive
                                                   error:nil];
    return [regex numberOfMatchesInString:self options:0 range:NSMakeRange(0, self.length)] > 0;
}
@end

int main(int argc, const char *argv[]) {
    @autoreleasepool {
        TaskManager *manager = [TaskManager sharedManager];
        manager.name = @"Production";

        NSArray *items = @[@"Build", @"Test", @"Deploy"];
        for (NSString *item in items) {
            [manager addTask:item withPriority:PriorityHigh];
        }

        [manager runWithCompletion:^(BOOL success) {
            NSLog(@"Done: %@", success ? @"YES" : @"NO");
        }];

        int hex = 0xFF;
        float pi = 3.14f;
        double sci = 1.5e10;
        NSNumber *boxed = @42;
        NSDictionary *map = @{@"key": @"value", @"count": @(items.count)};

        SEL sel = @selector(addTask:withPriority:);
        BOOL responds = [manager respondsToSelector:sel];
        const char *type = @encode(NSInteger);

        Point2D origin = {0.0, 0.0};
        CGFloat clamped = CLAMP(origin.x, -1.0, 1.0);
        NSLog(@"hex=%d pi=%f sci=%e clamped=%f enc=%s resp=%d", hex, pi, sci, clamped, type, responds);
    }
    return 0;
}
