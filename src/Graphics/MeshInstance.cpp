#include <MeshInstance.hpp>

MeshInstance::MeshInstance(const Mesh& mesh, const Transformation& t) :
	_mesh(&mesh),
	_material(mesh.getMaterial()),
	_transformation(t)
{
}

bool MeshInstance::isVisible(const glm::mat4& ProjectionMatrix, const glm::mat4& ViewMatrix) const
{
	const BoundingBox& bbox = _mesh->getBoundingBox();
	const glm::vec4 a = _transformation.getModelMatrix() * glm::vec4(bbox.min, 1.0);
	const glm::vec4 b = _transformation.getModelMatrix() * glm::vec4(bbox.max, 1.0);

	std::array<glm::vec4, 8> p = {glm::vec4{a.x, a.y, a.z, 1.0},
									glm::vec4{a.x, a.y, b.z, 1.0},
									glm::vec4{a.x, b.y, a.z, 1.0},
									glm::vec4{a.x, b.y, b.z, 1.0},
									glm::vec4{b.x, a.y, a.z, 1.0},
									glm::vec4{b.x, a.y, b.z, 1.0},
									glm::vec4{b.x, b.y, a.z, 1.0},
									glm::vec4{b.x, b.y, b.z, 1.0}};
						
	bool front = false;
	for(auto& t : p)
	{
		t = ViewMatrix * t;
		front = front || t.z < 0.0;
	}

	if(!front) return false;

	glm::vec2 min = glm::vec2(2.0, 2.0);
	glm::vec2 max = glm::vec2(-2.0, -2.0);
						
	for(auto& t : p)
	{
		t = ProjectionMatrix * t;
		if(t.w > 0.0) t /= t.w;
		min.x = std::min(min.x, t.x);
		min.y = std::min(min.y, t.y);
		max.x = std::max(max.x, t.x);
		max.y = std::max(max.y, t.y);
	}

	return !(max.x < -1.0 || max.y < -1.0 ||
			 min.x >  1.0 || min.y >  1.0);
}
