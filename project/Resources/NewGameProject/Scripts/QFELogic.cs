namespace QuickForgeEngine
{
    // QuickForgeEngineのC#側のロジックを記述するクラス
    static class Logic
    {
        public static uint CreateEntity(string entityName,Vector3 pos,Vector3 rot,Vector3 scale)
    {
        Transform transform = new Transform();
        transform.translate = pos;
        transform.rotate = rot;
        transform.scale = scale;
        uint entityID = Entity.Create(entityName, transform);
        return entityID;
    }
        public static uint CreateEntity(string entityName,Vector3 pos,Vector3 rot)
    {
        Transform transform = new Transform();
        transform.translate = pos;
        transform.rotate = rot;
        uint entityID = Entity.Create(entityName, transform);
        return entityID;
    }
        public static uint CreateEntity(string entityName,Vector3 pos)
    {
        Transform transform = new Transform();
        transform.translate = pos;
        uint entityID = Entity.Create(entityName, transform);
        return entityID;
    }

    public static float ExponentialDecay(float currentValue,float targetValue,float decayRate)
    {
        return currentValue + (targetValue - currentValue) * decayRate;
    }

    public static Vector3 ExponentialDecay(Vector3 currentValue,Vector3 targetValue,float decayRate)
    {
        return currentValue + (targetValue - currentValue) * decayRate;
    }
    }
}