#pragma once

class Camera;

class ICameraBehavior {
public:
    explicit ICameraBehavior(Camera& camera) : _camera(camera) {}

    virtual ~ICameraBehavior() = default;

    virtual void update(float dt) = 0;

protected:
    Camera& _camera;
};
