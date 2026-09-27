#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

namespace CMPUT350 {

class GameContext;

class GameObject {
public:
    virtual ~GameObject() = default;
    virtual void Initialize(GameContext *context);
    virtual void Update(GameContext *context);
    virtual void LateUpdate(GameContext *context);
    virtual void RenderUI(GameContext *context);
    virtual bool HandleKeyEvent(GameContext *context, char key);
    virtual bool IsAlive() const;
    virtual void Kill();
protected:
    bool mAlive = true; // need a way to make the object die
};

}  // namespace CMPUT350

#endif  // GAMEOBJECT_H
