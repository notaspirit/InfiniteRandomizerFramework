public class InfiniteRandomizerFramework extends ScriptableService {
    private cb func OnInitialize() {
        GameInstance.GetCallbackSystem()
            .RegisterCallback(n"Resource/PostLoad", this, n"OnSectorLoad")
            .AddTarget(ResourceTarget.Type(NameOf(worldStreamingSector)));
    }

    public cb func OnSectorLoad(event: ref<ResourceEvent>) {
    }
}