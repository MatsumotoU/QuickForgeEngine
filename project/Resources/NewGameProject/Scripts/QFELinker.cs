using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace QuickForgeEngine
{
    // C++のログ機能をC#から呼び出すためのDebugクラス
    public static class Debug
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Log(string message);
    }

    // 時間
    public static class Time
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float GetDeltaTime();
    }

    public static class Input
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void GetKeyMoveDir(out Vector2 outDir);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool GetKeyTrigger(string actionName);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool GetKeyPress(string actionName);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool GetKeyRelease(string actionName);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void GetMouseScreenPos(out Vector2 outPos);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void GetMouseMoveDir(out Vector2 outDir);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float GetMouseWheelDir();
    }

    public static class SceneManager
    {
       [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void LoadScene(string sceneName); 
    }

    public static class Audio
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern uint LoadSound(string soundName);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern uint PlaySound(uint soundHandle,bool isLoop,float volume);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void StopSound(uint playHandle);
    }

    public static class Entity
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern uint Create(string entityName,Transform transform);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void ChangeModel(uint entityID,string modelName);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void ChangeMesh(uint entityID,string meshName);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern uint GetEntityFromName(string entityName);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Destroy(uint entityID);
    }

    // C#スクリプト用QuickForgeコンポーネントクラス
    public abstract class QuickForgeComponent
    {
        public uint EntityID { get; internal set; }
        public GameLogicManager Manager { get; internal set; }
        public virtual void Initialize(float deltaTime) { }
        public virtual void Update(float deltaTime) { }

        public bool isDestroyed_ = false;
        public Transform transform_;
        
        public void Destroy()
        {
            isDestroyed_ = true;
            Entity.Destroy(EntityID);
        }

        public Vector3 GetPositionFromEntity(uint entityID)
        {
            return Manager.GetTransform(entityID).translate;
        }
        public Vector3 GetRotationFromEntity(uint entityID){
            return Manager.GetTransform(entityID).rotate;
        }
        public Vector3 GetScaleFromEntity(uint entityID){
            return Manager.GetTransform(entityID).scale;    
        }

        public void SetPositionFromEntity(uint entityID,Vector3 pos){
            Transform transform = Manager.GetTransform(entityID);
            transform.translate = pos;
            Manager.SetTransform(entityID, transform);
        }

        public void SetRotationFromEntity(uint entityID,Vector3 rot){
            Transform transform = Manager.GetTransform(entityID);
            transform.rotate = rot;
            Manager.SetTransform(entityID, transform);
        }

        public void SetScaleFromEntity(uint entityID,Vector3 scale){
            Transform transform = Manager.GetTransform(entityID);
            transform.scale = scale;
            Manager.SetTransform(entityID, transform);
        }
    }

    public class CreateComponentRequest
    {
        public uint EntityID;
        public Type ComponentType;
    }

    // ゲームロジックの初期化や更新を管理するクラス
    public class GameLogicManager
    {
        private Dictionary<uint, QuickForgeComponent> components_ = new Dictionary<uint, QuickForgeComponent>();
        private List<CreateComponentRequest> createRequests_ = new List<CreateComponentRequest>();
        
        private float deltaTime_ = 0.0f;
        private uint transformArraySize_ = 0;
        private uint[] transformEntityIDs_ = new uint[1024]; 
        private Transform[] transforms_ = new Transform[1024];
        public void InitializeAll()// C++側から呼び出される初期化関数
        {
            foreach (var component in components_)            {
                component.Value.Initialize(deltaTime_);
            }
        }
        public void UpdateAll()// C++側から呼び出される更新関数
        {
            foreach (var component in components_)            {
                component.Value.Update(deltaTime_);
            }
        }
        public void CreateInstance(uint entityID, string className)// C++側から呼び出されるインスタンス生成関数
        {
            // classNameからTypeを取得して、QuickForgeComponentを継承しているか確認して、createRequests_に追加する
            // インスタンスの生成はFrameEndで行なっている
            
            // 既に同じエンティティIDでリクエストがあるか確認
            if (createRequests_.Exists(r => r.EntityID == entityID))
            {
                Debug.Log("Instance creation request already exists for EntityID " + entityID);
                return;
            }

            Type type = Type.GetType(className);
            if (type != null && typeof(QuickForgeComponent).IsAssignableFrom(type))
            {
                createRequests_.Add(new CreateComponentRequest { EntityID = entityID, ComponentType = type });
                Debug.Log("Created instance of " + className + " with EntityID " + entityID);
            }
            else
            {
                string name;
                if (className == null)
                {
                    name = "<null>";
                }
                else if (className.Length == 0)
                {
                    name = "<empty>";
                }
                else if (string.IsNullOrWhiteSpace(className))
                {
                    name = "<whitespace>";
                }
                else
                {
                    name = "[" + className + "]";
                }

                Debug.Log("Failed to create instance of " + name + ". Class not found or does not inherit from QuickForgeComponent.");
            }
        }

        public void FrameStart()// C++側から呼び出されるフレーム開始関数
        {
            // =============== C++側からのデータ受け取り =====================
            // フレーム開始時にdeltaTime_をC++側から取得する
            deltaTime_ = Time.GetDeltaTime();

            // C++側からエンティティIDとTransformの配列を取得して、各コンポーネントのTransformを更新する
            InternalProperty.GetTransforms(transformEntityIDs_, transforms_,  ref transformArraySize_);
            for (int i = 0; i < transformArraySize_; i++)
            {
                // sparseからその要素のEntityIDを取得して、components_からそのEntityIDを持つコンポーネントを探す
                uint entityID = transformEntityIDs_[i];
                Transform transform = transforms_[i];

                if (components_.TryGetValue(entityID, out var component))
                {
                    component.transform_ = transform;
                }
            }
        }

        public void FrameEnd()// C++側から呼び出されるフレーム終了関数
        {
            
            // 消滅したコンポーネントをcomponents_から削除する
            foreach (var component in components_)
            {
                if (component.Value.isDestroyed_)
                {
                    components_.Remove(component.Key);
                    Debug.Log($"Removed component with EntityID {component.Key}");
                    break;
                }
            }

            // =============== C++側からのデータ受け取り =====================
            // 各コンポーネントのTransformをC++側に渡すための配列を作成して、C++側に送る
            for(int i = 0;i<transformArraySize_; i++)
            {
                // sparseからその要素のEntityIDを取得して、components_からそのEntityIDを持つコンポーネントを探す
                uint entityID = transformEntityIDs_[i];
                if (components_.TryGetValue(entityID, out var component))
                {
                    transforms_[i] = component.transform_;
                }
            }
            InternalProperty.SetTransforms(transformEntityIDs_, transforms_, transformArraySize_);

            // =============== C#側のデータ処理 =====================
            // createRequests_にあるTypeのインスタンスをAddComponentで作成して、components_に追加する
            foreach (var request in createRequests_)
            {
                AddComponent(request.EntityID, request.ComponentType);
            }
            createRequests_.Clear();
        }

        public Transform GetTransform(uint entityID){
            if (components_.TryGetValue(entityID, out var component))
            {
                return component.transform_;
            }
            return new Transform();
        }
        public void SetTransform(uint entityID,Transform transform){
            if (components_.TryGetValue(entityID, out var component))
            {
                component.transform_ = transform;
            }
        }

        private void AddComponent(uint entityID, Type type)
        {
            // typeがQuickForgeComponentを継承しているか確認
            if (!typeof(QuickForgeComponent).IsAssignableFrom(type))
            {
                Debug.Log("Type " + type.Name + " does not inherit from QuickForgeComponent. Cannot add component.");
                return;
            }

            // typeのインスタンスを作成して、EntityIDとManagerをセットして、components_に追加する
            QuickForgeComponent component = (QuickForgeComponent)Activator.CreateInstance(type);
            component.EntityID = entityID;
            component.Manager = this;
            components_.Add(entityID, component);
            Debug.Log("Added component of type " + type.Name + " to EntityID " + entityID);
        }
    }
}