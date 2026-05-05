1\. Design Philosophy

This engine is built on a Data-Oriented Entity Paradigm, heavily emphasizing the separation of blueprint data (Memory) from spatial data (Instances). The architecture is designed to minimize CPU overhead through memory caching, lazy mathematical evaluation, and multi-stage physics validation.



2\. Core Object Architecture

The fundamental building block of the engine is the strict separation between Mesh and Object.



Mesh (The Blueprint): Represents pure structural data. It holds OpenGL buffer IDs (VAO, VBO), material properties, and CPU-side arrays (std::vector<Vertex>). It calculates and stores its localAABB exactly once upon creation. It has zero knowledge of the world.



Object (The Instance): Represents a physical entity in the game world. It holds a Transform struct (Position, Rotation, Scale) and a shared pointer to a Mesh. Multiple Objects can point to the same Mesh to achieve hardware instancing and memory efficiency.



Lazy Transform Evaluation

To prevent redundant matrix multiplication, Object utilizes a "Dirty Flag" pattern:



Any changes to Position/Rotation/Scale trigger transformDirtyFlag = true.



When GetModelMatrix() is called (by the Physics or Render pipelines), the matrix is mathematically rebuilt only if the flag is true. Otherwise, it instantly returns a const cached matrix.



3\. Resource Management (MeshManager)

The engine prevents RAM and VRAM duplication through a centralized MeshManager.



Models are loaded via .obj parsing and immediately stored in a hashed map (std::unordered\_map<std::string, std::shared\_ptr<Mesh>>).



Upon loading, the manager guarantees the dual-dispatch of geometry: vertices are sent to the GPU for the ForwardRendererPass, and simultaneously preserved in standard CPU memory (std::vector) to feed the Physics pipeline.



4\. The Physics \& Collision Pipeline

The engine uses a custom continuous-collision-detection (CCD) capable pipeline, executed sequentially during the Player::Update() step.



Phase 1: Broadphase (AABB vs AABB)



The engine dynamically calculates the WorldAABB of an Object using its cached ModelMatrix and its pre-calculated localAABB.



It tests this against the Player's AABB. Objects failing the overlap test are discarded instantly, bypassing matrix math for hidden geometry.



Phase 2: Narrowphase (Sphere vs Triangle)



Objects passing the Broadphase have their individual triangles extracted, transformed into World Space, and tested mathematically against the Player's collision Sphere.



Phase 3: Collision Resolution \& Slide



Upon a positive hit, the exact penetration depth is calculated. The player's position is immediately pushed along the collision normal (Depenetration).



Residual velocity directed into the wall is zeroed out, creating a smooth "wall-slide" effect.



The collision normal's Y-axis is analyzed (pushNormal.y > 0.5f) to determine if the surface is walkable, dynamically setting the isGrounded state to allow jumping.



5\. Event System

The engine communicates across discrete systems using a Publisher/Subscriber paradigm via a MessageBus.



Example: UI commands trigger an ObjectSpawnedMessage. The Bus alerts the active Scene, which calls the MeshManager, generates the Object, and pushes it to the renderer and physics loop without hard-coupling the UI to the Scene logic.

