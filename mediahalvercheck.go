package mediahalvercheck

import (
    "android/soong/android"
    "android/soong/cc"
    "fmt"
    "github.com/google/blueprint/proptools"
)

func MediahalVerCheckDefaults(ctx android.LoadHookContext) {
    sdkVersion := ctx.AConfig().PlatformSdkVersionInt()
    fmt.Println("sdkVersion:", sdkVersion)
    if sdkVersion >= 30 {
        type props struct {
            System_ext_specific  *bool
        }
        p := &props{}
        p.System_ext_specific = proptools.BoolPtr(true)
        ctx.AppendProperties(p)
    }
}

func init() {
    android.RegisterModuleType("mediahalvercheck_defaults", MediahalVerCheckDefaultsFactory)
}

func MediahalVerCheckDefaultsFactory() android.Module {
    module := cc.DefaultsFactory()
    android.AddLoadHook(module, MediahalVerCheckDefaults)

    return module
}
