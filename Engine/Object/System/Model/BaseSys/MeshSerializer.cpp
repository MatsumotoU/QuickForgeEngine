#include "MeshSerializer.h"
#include "Base/EngineCore.h"

nlohmann::json MeshSerializer::Serialize(const MeshComponent& meshComponent) {
    nlohmann::json j;

    // 頂点バッファのシリアライズ
    nlohmann::json jVertices = nlohmann::json::array();

    // vertexBuffers_の中身を取得
    const auto& vertices = meshComponent.vertices; // ここをvertexBuffers_から取得する場合は適宜修正
    for (const auto& v : vertices) {
        jVertices.push_back({
            {"position", {v.position.x, v.position.y, v.position.z}},
            {"normal",   {v.normal.x, v.normal.y, v.normal.z}},
            {"uv",       {v.texcoord.x, v.texcoord.y}}
            });
    }
    j["vertexBuffer"] = jVertices;
    j["indexBuffer"] = meshComponent.indices;
    return j;
}

void MeshSerializer::Deserialize(EngineCore* engineCore, MeshComponent& meshComponent, const nlohmann::json& j) {
    // 初期化とリソースの生成
	meshComponent.indices.clear();
	meshComponent.vertices.clear();
	meshComponent.vertexBuffers_.CreateResource(
		engineCore->GetDirectXCommon()->GetDevice(),
		static_cast<uint32_t>(j.contains("vertexBuffer") ? j["vertexBuffer"].size() : 0)
	);

	// 頂点データのデシリアライズ
    if (j.contains("vertexBuffer")) {
        const auto& jVertices = j["vertexBuffer"];
        meshComponent.vertices.clear();
        for (const auto& v : jVertices) {
            VertexData vertex;
            vertex.position = Vector4(v["position"][0].get<float>(), v["position"][1].get<float>(), v["position"][2].get<float>(), 1.0f);
            vertex.normal = Vector3(v["normal"][0].get<float>(), v["normal"][1].get<float>(), v["normal"][2].get<float>());
            vertex.texcoord = Vector2(v["uv"][0].get<float>(), v["uv"][1].get<float>());
            meshComponent.vertices.push_back(vertex);
        }
    }
    if (j.contains("indexBuffer")) {
        meshComponent.indices = j["indexBuffer"].get<std::vector<uint32_t>>();
    }
}