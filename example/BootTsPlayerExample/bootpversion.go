package sharelibAndGitInfo

import (
    "android/soong/android"
    "android/soong/cc"
    "fmt"
)

func bootpversionDefaults(ctx android.LoadHookContext) {

    type props struct {
        Cflags []string
    }

    p := &props{}
    p.Cflags = setversion(ctx)
    ctx.AppendProperties(p)
}

func setversion(ctx android.BaseContext) ([]string) {
    var cppflags []string

    sdkVersion := ctx.Config().PlatformSdkVersion().String()

    ver10 := "-DANDROID_PLATFORM_SDK_VERSION=" + sdkVersion
    fmt.Println(string(ver10))
    cppflags = append(cppflags, ver10)

    return cppflags
}

func init() {
    android.RegisterModuleType("bootpversion_defaults", bootpversionFactory)
}

func bootpversionFactory() android.Module {
    module := cc.DefaultsFactory()
    android.AddLoadHook(module, bootpversionDefaults)

    return module
}