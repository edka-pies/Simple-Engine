#include "Player.h"
#include "Mesh.h"
void Player::Update(float dt,
    glm::vec3 inputDir,
    bool wantJump,
    const Terrain& terrain,
    const std::vector<Object*>& sceneObjects) {
    // Horizontal Movement 
    if (glm::length(inputDir) > 0.1f) {
        velocity.x += inputDir.x * acceleration * dt;
        velocity.z += inputDir.z * acceleration * dt;
    }
    else {
        // Apply friction
        float decay = glm::exp(-friction * dt);
        velocity.x *= decay;
        velocity.z *= decay;
    }

    // Clamp horizontal speed
    float speed = glm::length(glm::vec2(velocity.x, velocity.z));
    if (speed > maxSpeed) {
        velocity.x = (velocity.x / speed) * maxSpeed;
        velocity.z = (velocity.z / speed) * maxSpeed;
    }

    // Gravity & Jumping
    if (isGrounded) {
        velocity.y = 0;
        if (wantJump) {
            velocity.y = jumpForce;
            isGrounded = false;
        }
    }
    else {
        velocity.y += gravity * dt;
    }

    // Integration (Moving the player)
    position += velocity * dt;

    // Custom Collision (Terrain snap)
    float floorY = terrain.GetTriangleHeightAt(position.x, position.z);
    if (position.y <= floorY) {
        position.y = floorY;
        velocity.y = 0;
        isGrounded = true;
    }
    else {
        isGrounded = false;
    }

    // Object Collision (The Physics Loop)
    // Setup Player Physics Shapes
    Sphere playerSphere = { position + glm::vec3(0.0f, 1.0f, 0.0f), 1.0f };
    AABB playerBox = GetPlayerAABB();

    static float debugTimer = 0.0f;
    debugTimer += dt;
    bool shouldPrint = false;
    if (debugTimer > 1.0f) { 
        shouldPrint = true;
        debugTimer = 0.0f;
        std::cout << "\n=== PHYSICS FRAME START ===\n";
    }

    for (Object* obj : sceneObjects) {

        if (!obj || !obj->GetMesh()) continue;
        
        if (visualObject != nullptr && obj == visualObject) {
            continue;
        }
        
        std::shared_ptr<Mesh> mesh = obj->GetMesh();

        // Check the Local AABB 
        AABB localBox = mesh->GetLocalAABB();

        // Check the World AABB
        AABB worldBox = GetWorldAABB(localBox, obj->GetModelMatrix());

        // The Broadphase Check
        if (!TestAABBAABB(playerBox, worldBox)) {
            continue;
        }

        // Broadphase Check
        if (!TestAABBAABB(playerBox, worldBox)) {
            continue;
        }

        // --- NARROWPHASE ---
        glm::mat4 modelMat = obj->GetModelMatrix();
        const auto& vertices = mesh->GetVertices();
        const auto& indices = mesh->GetIndices();

        // Loop through every triangle in the mesh
        for (size_t i = 0; i < indices.size(); i += 3) {
            Triangle tri;

            // Convert the local mesh vertices into actual world coordinates
            tri.a = glm::vec3(modelMat * glm::vec4(vertices[indices[i]].position, 1.0f));
            tri.b = glm::vec3(modelMat * glm::vec4(vertices[indices[i + 1]].position, 1.0f));
            tri.c = glm::vec3(modelMat * glm::vec4(vertices[indices[i + 2]].position, 1.0f));

            glm::vec3 hitPoint;
            if (TestSphereTriangle(playerSphere, tri, hitPoint)) {

                // --- COLLISION RESOLUTION ---
                // Calculate how deep the player is inside the triangle
                glm::vec3 pushDir = playerSphere.center - hitPoint;
                float distance = glm::length(pushDir);

                // Prevent division by zero
                if (distance > 0.0001f && distance < playerSphere.radius) {
                    glm::vec3 pushNormal = pushDir / distance; // Normalize
                    float penetrationDepth = playerSphere.radius - distance;

                    position += pushNormal * penetrationDepth;
                    playerSphere.center += pushNormal * penetrationDepth;

                    if (pushNormal.y > 0.5f) {
                        isGrounded = true;

                        // Stop gravity
                        if (velocity.y < 0.0f) {
                            velocity.y = 0.0f;
                        }
                    }
                    // Velocity Slide
                    float velAlongNormal = glm::dot(velocity, pushNormal);
                    if (velAlongNormal < 0.0f) {
                        // Subtract the forward momentum going into the wall
                        velocity -= pushNormal * velAlongNormal;
                    }
                }
            }
        }
    }
    // Sync the puppet (Visual Mesh Update)
    if (visualObject && visualObject->GetMesh()) {
        visualObject->SetPosition(position);
    }
}

glm::mat4 Player::GetModelMatrix() const {
    return visualObject->GetModelMatrix();
}