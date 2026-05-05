#pragma once
#pragma once
#include <vector>
#include <memory>
#include <iostream>
#include "Camera.h" 

class CameraManager {
private:
    std::vector<std::shared_ptr<Camera>> m_Cameras;
    size_t m_ActiveIndex = 0;

public:
    // Add a camera and return its index
    size_t AddCamera(std::shared_ptr<Camera> camera) {
        m_Cameras.push_back(camera);
        return m_Cameras.size() - 1;
    }

    // Cycle to the next camera
    void CycleCamera() {
        if (m_Cameras.empty()) return;
        m_ActiveIndex = (m_ActiveIndex + 1) % m_Cameras.size();
    }

    // Get the currently active camera
    std::shared_ptr<Camera> GetActiveCamera() const {
        if (m_Cameras.empty()) return nullptr;
        return m_Cameras[m_ActiveIndex];
    }

    bool IsActive(size_t index) const {
        return m_ActiveIndex == index;
    }

    void SetActiveCamera(size_t index) {
        if (index < m_Cameras.size()) {
            m_ActiveIndex = index;
        }
        else {
            std::cout << "WARNING: Attempted to switch to invalid camera index: " << index << "\n";
        }
    }
};