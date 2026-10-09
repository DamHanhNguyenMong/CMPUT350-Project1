#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <string>

namespace CMPUT350 {

class GameContext;

class GameObject {
public:
    virtual void Initialize(GameContext *context);
    virtual void Update(GameContext *context);
    virtual void LateUpdate(GameContext *context);
    virtual void RenderUI(GameContext *context);
    virtual bool IsAlive() const;
    virtual void Kill();
    virtual void ReceiveNotification(const std::string& key);

protected:
    bool mAlive = true; // need a way to make the object die
};

}  // namespace CMPUT350

#endif  // GAMEOBJECT_H
