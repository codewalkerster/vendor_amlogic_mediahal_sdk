package mediahalvercheck

import (
    "android/soong/android"
    "android/soong/cc"
    //"fmt"
    "github.com/google/blueprint/proptools"
    "strconv"
)

func MediahalVerCheckDefaults(ctx android.LoadHookContext) {
    sdkVersion := ctx.Config().PlatformSdkVersion().String()
    sdkVersionInt,err := strconv.Atoi(sdkVersion)
    if err != nil {
        //fmt.Printf("%v fail to convert", sdkVersionInt)
    } else {
        //fmt.Println("CheckDefaults sdkVersion:", sdkVersionInt)
    }
    if sdkVersionInt >= 30 {
        type props struct {
            System_ext_specific  *bool
        }
        p := &props{}
        p.System_ext_specific = proptools.BoolPtr(true)
        ctx.AppendProperties(p)
    }
}

func MediahalVerCheckMediahalPassthroughDefaults(ctx android.LoadHookContext) {
    type props struct {
    Enabled  *bool
    }
    p := &props{}
    // Enabled by default, when sdkVersion >= 30 or sdkVersion is not a valid number string
    p.Enabled = proptools.BoolPtr(true)
    sdkVersion := ctx.Config().PlatformSdkVersion().String()
    sdkVersionInt,err := strconv.Atoi(sdkVersion)
    if err != nil {
        //fmt.Printf("%v fail to convert", sdkVersionInt)
    } else {
        //fmt.Println("PassthroughDefaults sdkVersion:", sdkVersionInt)
    if sdkVersionInt < 30 {
        p.Enabled = proptools.BoolPtr(false)
    }
    }
    ctx.AppendProperties(p)
}

func init() {
    android.RegisterModuleType("mediahalvercheck_defaults", MediahalVerCheckDefaultsFactory)
    android.RegisterModuleType("mediahalvercheck_mediahal_passthrough_defaults", MediahalVerCheckMediahalPassthroughDefaultsFactory)
}

func MediahalVerCheckDefaultsFactory() android.Module {
    module := cc.DefaultsFactory()
    android.AddLoadHook(module, MediahalVerCheckDefaults)

    return module
}

func MediahalVerCheckMediahalPassthroughDefaultsFactory() android.Module {
    module := cc.DefaultsFactory()
    android.AddLoadHook(module, MediahalVerCheckMediahalPassthroughDefaults)

    return module
}
