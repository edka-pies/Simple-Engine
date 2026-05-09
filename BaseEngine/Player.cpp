#include "Player.h"
#include "Mesh.h"
void Player::Update(float dt,
    glm::vec3 inputDir,
    bool wantJump,
    bool wantDash,
    glm::vec3 cameraForward,
    const Terrain& terrain,
    const std::vector<Object*>& sceneObjects) {

    // 1. DASH TIMERS
    if (dashCooldownTimer > 0.0f) dashCooldownTimer -= dt;

    if (wantDash && dashCooldownTimer <= 0.0f) {
        isDashing = true;
        dashTimer = DASH_DURATION;
        dashCooldownTimer = DASH_COOLDOWN;

        if (glm::length(inputDir) > 0.1f) {
            dashDirection = inputDir;
        }
        else {
            // FIX #1: Check length BEFORE normalizing to prevent the NaN crash!
            glm::vec3 flatCamDir(cameraForward.x, 0.0f, cameraForward.z);
            if (glm::length(flatCamDir) < 0.01f) {
                dashDirection = glm::vec3(0.0f, 0.0f, -1.0f);
            }
            else {
                dashDirection = glm::normalize(flatCamDir);
            }
        }
    }

    // 2. APPLY VELOCITY
    if (isDashing) {
        dashTimer -= dt;
        if (dashTimer <= 0.0f) {
            isDashing = false;
            velocity.x = 0.0f;
            velocity.z = 0.0f;
        }
        else {
            velocity.x = dashDirection.x * dashSpeed;
            velocity.z = dashDirection.z * dashSpeed;
            velocity.y = 0.0f; // Freeze gravity during dash
        }
    }
    else {
        // Normal Walking
        if (glm::length(inputDir) > 0.1f) {
            velocity.x += inputDir.x * acceleration * dt;
            velocity.z += inputDir.z * acceleration * dt;
        }
        else {
            float decay = glm::exp(-friction * dt);
            velocity.x *= decay;
            velocity.z *= decay;
        }
    }

    // 3. CLAMP SPEED (Make sure maxSpeed in your header > 0!)
    if (!isDashing) {
        float speed = glm::length(glm::vec2(velocity.x, velocity.z));
        if (speed > maxSpeed && maxSpeed > 0.0f) {
            velocity.x = (velocity.x / speed) * maxSpeed;
            velocity.z = (velocity.z / speed) * maxSpeed;
        }
    }

    // 4. GRAVITY & JUMPING
    if (isGrounded) {
        jumpCount = 0;
        velocity.y = 0;
    }
    else {
        velocity.y += gravity * dt; // Make sure gravity is negative! e.g., -15.0f
    }

    if (wantJump) {
        if (isGrounded || jumpCount < MAX_JUMPS) {
            velocity.y = jumpForce;
            jumpCount++;
            isGrounded = false;
            isDashing = false;
        }
    }

    // 5. INTEGRATION
    position += velocity * dt;

    // 6. TERRAIN COLLISION
    float floorY = terrain.GetTriangleHeightAt(position.x, position.z);
    if (position.y <= floorY) {
        position.y = floorY;
        velocity.y = 0;
        isGrounded = true;
    }
    else {
        isGrounded = false;
    }

    // 7. OBJECT COLLISION LOOP
    Sphere playerSphere = { position + glm::vec3(0.0f, 1.0f, 0.0f), 1.0f };
    AABB playerBox = GetPlayerAABB();

    for (Object* obj : sceneObjects) {

        if (!obj || !obj->GetMesh()) continue;
        if (visualObject != nullptr && obj == visualObject) continue;

        std::shared_ptr<Mesh> mesh = obj->GetMesh();
        AABB localBox = mesh->GetLocalAABB();
        AABB worldBox = GetWorldAABB(localBox, obj->GetModelMatrix());

        if (!TestAABBAABB(playerBox, worldBox)) continue;

        glm::mat4 modelMat = obj->GetModelMatrix();
        const auto& vertices = mesh->GetVertices();
        const auto& indices = mesh->GetIndices();

        for (size_t i = 0; i < indices.size(); i += 3) {
            Triangle tri;
            tri.a = glm::vec3(modelMat * glm::vec4(vertices[indices[i]].position, 1.0f));
            tri.b = glm::vec3(modelMat * glm::vec4(vertices[indices[i + 1]].position, 1.0f));
            tri.c = glm::vec3(modelMat * glm::vec4(vertices[indices[i + 2]].position, 1.0f));

            glm::vec3 hitPoint;
            if (TestSphereTriangle(playerSphere, tri, hitPoint)) {
                glm::vec3 pushDir = playerSphere.center - hitPoint;
                float distance = glm::length(pushDir);

                if (distance > 0.0001f && distance < playerSphere.radius) {
                    glm::vec3 pushNormal = pushDir / distance;
                    float penetrationDepth = playerSphere.radius - distance;

                    position += pushNormal * penetrationDepth;
                    playerSphere.center += pushNormal * penetrationDepth;

                    if (pushNormal.y > 0.5f) {
                        isGrounded = true;
                        if (velocity.y < 0.0f) {
                            velocity.y = 0.0f;
                        }
                    }

                    float velAlongNormal = glm::dot(velocity, pushNormal);
                    if (velAlongNormal < 0.0f) {
                        velocity -= pushNormal * velAlongNormal;
                    }
                }
            }
        }
    } // <--- FIX #2: Object loop strictly ends here!

    // 8. SYNC THE PUPPET (Safely outside the physics loops!)
    if (visualObject && visualObject->GetMesh()) {
        visualObject->SetPosition(position);
    }
}

glm::mat4 Player::GetModelMatrix() const {
    return visualObject->GetModelMatrix();
}