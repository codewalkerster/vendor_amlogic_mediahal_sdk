package sharelibAndGitInfo

import (
    "android/soong/android"
    "android/soong/cc"
    "fmt"
)

func androidversionDefaults(ctx android.LoadHookContext) {

    type props struct {
        Cflags []string
    }

    p := &props{}
    p.Cflags = setversion(ctx)
    ctx.AppendProperties(p)
}

func setversion(ctx android.BaseContext) ([]string) {
	var cppflags []string

    sdkVersion := ctx.DeviceConfig().PlatformVndkVersion()

    ver10 := "-DANDROID_PLATFORM_SDK_VERSION=" + sdkVersion
    fmt.Println(string(ver10))
    cppflags = append(cppflags, ver10)

    return cppflags
}

func init() {
    android.RegisterModuleType("androidversion_defaults", androidversionFactory)
}

func androidversionFactory() android.Module {
    module := cc.DefaultsFactory()
    android.AddLoadHook(module, androidversionDefaults)

    return module
}