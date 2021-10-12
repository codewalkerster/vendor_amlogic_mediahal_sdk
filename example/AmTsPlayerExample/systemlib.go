package systemCheck

import (
    "android/soong/android"
    "android/soong/cc"
    "fmt"
    "strconv"
)

func systemlibDefaults(ctx android.LoadHookContext) {
    sdkVersion := ctx.DeviceConfig().PlatformVndkVersion()
    sdkVersionInt,err := strconv.Atoi(sdkVersion)
    if err != nil {
        fmt.Printf("---------------------------->%v fail to convert", sdkVersionInt)
    } else {
        fmt.Println("sdkVersion:", sdkVersionInt)
    }
    if sdkVersionInt == 30 {
        type props struct {
            Shared_libs []string
			Include_dirs []string
        }
        p := &props{}

        var sharedlib []string
        sharedlib = append(sharedlib,"//hardware/amlogic:libamgralloc_ext")
        p.Shared_libs = sharedlib

        var includedirs []string
		includedirs = append(includedirs,"vendor/amlogic/common/frameworks/services/systemcontrol","vendor/amlogic/common/frameworks/services/systemcontrol/PQ/include","hardware/amlogic/gralloc")
		p.Include_dirs = includedirs
		
        ctx.AppendProperties(p)
    }
    if sdkVersionInt == 28 {
        type props struct {
            Shared_libs []string
			Include_dirs []string
        }
        p := &props{}

        var sharedlib []string
        sharedlib = append(sharedlib,"//hardware/amlogic:libamgralloc_ext@2")
        p.Shared_libs = sharedlib

        var includedirs []string
        includedirs = append(includedirs,"hardware/amlogic/gralloc")
        p.Include_dirs = includedirs

        ctx.AppendProperties(p)
    }
}

func init() {
    android.RegisterModuleType("systemlib_defaults", systemlibDefaultsFactory)
}

func systemlibDefaultsFactory() android.Module {
    module := cc.DefaultsFactory()
    android.AddLoadHook(module, systemlibDefaults)

    return module
}