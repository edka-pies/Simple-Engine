#include "Enemy.h"
#include "PhysicsUtils.h"
#include "MeshComponent.h"
#include <iostream>

void Enemy::Update(float dt, glm::vec3 playerPos, Object* playerObj, const Terrain& terrain, const std::vector<Object*>& sceneObjects) {

    // 1. STATE MACHINE LOGIC (The "AI")
    float distToPlayer = glm::distance(position, playerPos);

    if (distToPlayer < aggroRadius) {
        state = EnemyState::Chase;
    }
    else {
        state = EnemyState::Idle;
    }

    // 2. MOVEMENT LOGIC
    if (state == EnemyState::Chase) {
        // Find the vector pointing from the enemy to the player
        glm::vec3 dir = playerPos - position;
        dir.y = 0.0f; // Ignore Y so the enemy doesn't try to fly up into the sky!

        if (glm::length(dir) > 0.1f) {
            dir = glm::normalize(dir);
            velocity.x = dir.x * speed;
            velocity.z = dir.z * speed;

            // BONUS: Make the enemy rotate to face the player!
            // atan2 gives us the angle in radians, we convert to degrees for your transform
            float angle = glm::degrees(atan2(dir.x, dir.z));
            visualObject->SetRotation(glm::vec3(0.0f, angle, 0.0f));
        }
    }
    else {
        // Friction / Stopping when idle
        velocity.x *= 0.9f;
        velocity.z *= 0.9f;
    }

    // 3. GRAVITY & INTEGRATION
    if (!isGrounded) {
        velocity.y += gravity * dt;
    }
    position += velocity * dt;

    // 4. TERRAIN COLLISION (Keep them on the ground)
    float floorY = terrain.GetTriangleHeightAt(position.x, position.z);
    if (position.y <= floorY) {
        position.y = floorY;
        velocity.y = 0.0f;
        isGrounded = true;
    }
    else {
        isGrounded = false;
    }

    AABB enemyBox;
    enemyBox.minBounds = position + glm::vec3(-0.5f, 0.0f, -0.5f);
    enemyBox.maxBounds = position + glm::vec3(0.5f, 2.0f, 0.5f);

    // --- ENVIRONMENT COLLISION ---
    for (Object* obj : sceneObjects) {
        if (!obj) continue;
        auto meshComp = obj->GetComponent<MeshComponent>();
        if (!meshComp || !meshComp->mesh || obj == visualObject || obj == playerObj) continue;

        AABB localBox = meshComp->mesh->GetLocalAABB();
        AABB worldBox = GetWorldAABB(localBox, obj->GetModelMatrix()); // Using your existing helper!

        if (TestAABBAABB(enemyBox, worldBox)) {
            // We hit a wall/crate! Find the shallowest penetration axis to push them out of
            float overlapX = (std::min)(enemyBox.maxBounds.x - worldBox.minBounds.x, worldBox.maxBounds.x - enemyBox.minBounds.x);
            float overlapZ = (std::min)(enemyBox.maxBounds.z - worldBox.minBounds.z, worldBox.maxBounds.z - enemyBox.minBounds.z);

            // Push out along the X axis
            if (overlapX < overlapZ) {
                if (position.x < worldBox.minBounds.x + (worldBox.maxBounds.x - worldBox.minBounds.x) * 0.5f)
                    position.x -= overlapX;
                else
                    position.x += overlapX;
            }
            // Push out along the Z axis
            else {
                if (position.z < worldBox.minBounds.z + (worldBox.maxBounds.z - worldBox.minBounds.z) * 0.5f)
                    position.z -= overlapZ;
                else
                    position.z += overlapZ;
            }

            // Update the box for the next object check so we don't get stuck in corners!
            enemyBox.minBounds = position + glm::vec3(-0.5f, 0.0f, -0.5f);
            enemyBox.maxBounds = position + glm::vec3(0.5f, 2.0f, 0.5f);
        }
    }

    // --- PLAYER HIT DETECTION ---
    AABB playerBox;
    playerBox.minBounds = playerPos + glm::vec3(-0.5f, 0.0f, -0.5f);
    playerBox.maxBounds = playerPos + glm::vec3(0.5f, 2.0f, 0.5f);

    if (TestAABBAABB(enemyBox, playerBox)) {
        std::cout << "PLAYER TOOK DAMAGE!\n";

        // Quick Reset for testing:
        // You can either enqueue a MessageBus event here like "PlayerDiedMessage"
        // Or simply bump the player away by modifying playerPos (if you passed it by reference)
    }

    // 5. UPDATE VISUAL MESH
    visualObject->SetPosition(position);
}