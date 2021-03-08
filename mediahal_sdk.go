package mediahal_sdk

import (
    "fmt"
    "android/soong/android"
    "android/soong/cc"
    "github.com/google/blueprint/proptools"
)

func init() {
    android.RegisterModuleType("mediahal_sdk_go_defaults", mediahal_sdk_go_DefaultsFactory)
}

func mediahal_sdk_go_DefaultsFactory() (android.Module) {
    module := cc.DefaultsFactory()
    android.AddLoadHook(module, func(ctx android.LoadHookContext) {
        type props struct {
            Enabled *bool
        }
        p := &props{}

        mediahalSrcPath := "media_hal"
        hardmediahalSrcPath := "hardware/amlogic/media_hal"
        if android.ExistentPathForSource(ctx, mediahalSrcPath).Valid() == true {
            p.Enabled = proptools.BoolPtr(false)
            fmt.Printf("mediahalSrcPath:%s exist, use medial source to build\n", mediahalSrcPath)
        } else if android.ExistentPathForSource(ctx, hardmediahalSrcPath).Valid() == true {
            p.Enabled = proptools.BoolPtr(false)
            fmt.Printf("dvb:%s not exist, use hardwaremedial source to build\n", hardmediahalSrcPath)
        }else {
            fmt.Println("mediahalSrcPath:%s not exist, use mediahal_sdk to build",mediahalSrcPath)
        }
        ctx.AppendProperties(p)
    })
    return module
}
