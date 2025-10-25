public class InfiniteRandomizerFrameworkService extends ScriptableService {

    private cb func OnInitialize() {
        InfiniteRandomizerFrameworkNative.Initialize();

        GameInstance.GetCallbackSystem()
            .RegisterCallback(n"Resource/PostLoad", this, n"OnSectorLoad")
            .AddTarget(ResourceTarget.Type(NameOf<worldStreamingSector>()));
    }

    private cb func OnSectorLoad(event: ref<ResourceEvent>) {
        let sector: ref<worldStreamingSector> = event.GetResource() as worldStreamingSector;
        InfiniteRandomizerFrameworkNative.OnSectorPostLoad(sector);
    }
}

public static native class InfiniteRandomizerFrameworkNative extends IScriptable {
    public static native func Initialize() -> Void;
    public static native func OnSectorPostLoad(sector: ref<worldStreamingSector>) -> Void;

}
